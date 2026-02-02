// =============================================================================
// NEMESIS - Molecular Modelling Package
// -----------------------------------------------------------------------------
//    Copyright (C) 2011 Petr Kulhanek, kulhanek@chemi.muni.cz
//
//     This program is free software; you can redistribute it and/or modify
//     it under the terms of the GNU General Public License as published by
//     the Free Software Foundation; either version 2 of the License, or
//     (at your option) any later version.
//
//     This program is distributed in the hope that it will be useful,
//     but WITHOUT ANY WARRANTY; without even the implied warranty of
//     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//     GNU General Public License for more details.
//
//     You should have received a copy of the GNU General Public License along
//     with this program; if not, write to the Free Software Foundation, Inc.,
//     51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
// =============================================================================

#include <Property.hpp>
#include <PropertyList.hpp>
#include <PropertyHistory.hpp>
#include <ErrorSystem.hpp>
#include <XMLElement.hpp>
#include <Project.hpp>
#include <Structure.hpp>
#include <RestraintList.hpp>

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

CProperty::CProperty(CPluginObject* p_po,CPropertyList *p_bl)
    : CProObject(p_po,p_bl,p_bl->GetProject())
{
    PropUnit = NULL;
}

//------------------------------------------------------------------------------

CProperty::~CProperty(void)
{
    CPropertyList* p_list = GetPropertyList();
    setParent(NULL);    // remove object from the list
    if( p_list ) p_list->EmitOnPropertyListChanged();
}

// ----------------------------------------------------------------------------

void CProperty::RemoveFromBaseList(CHistoryNode* p_history)
{
    CProObject* p_obj;

    // unregister other objects
    while( (p_obj = RemoveFirstFromROList()) !=NULL ) {
        p_obj->RemoveFromRegistered(this,p_history);
    }

    // register elementary change to history list
    if( p_history != NULL ) {
        CPropertyHI* p_atomdata = new CPropertyHI(this,EHID_BACKWARD);
        p_history->Register(p_atomdata);
    }

    // --------------------------------
    delete this;  // destroy object
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

bool CProperty::DeletePropertyWH(void)
{
    // object will be destroyed - use list
    CPropertyList* p_list = GetPropertyList();

    // with history
    CHistoryNode* p_history = p_list->BeginChangeWH(EHCL_PROPERTY,"delete property");
    if( p_history == NULL ) return(false);
    RemoveFromBaseList(p_history);
    p_list->EndChangeWH();

    return(true);
}

//------------------------------------------------------------------------------

bool CProperty::RestrainPropertyWH(void)
{
    CStructure* p_str = GetProject()->GetActiveStructure();
    if( p_str == NULL ){
        GetProject()->TextNotification(ETNT_ERROR,tr("no active structure"),3000);
        return(false);
    }

    // is already restrained in the active structure?
    if( p_str->GetRestraints()->IsRestrained(this) ){
        GetProject()->TextNotification(ETNT_ERROR,tr("property is already restrained"),3000);
        return(false);
    }

    // is already restrained in the active structure?
    if( ! HasGradient(p_str) ){
        GetProject()->TextNotification(ETNT_ERROR,tr("property does not have gradient for the active structure"),3000);
        return(false);
    }

    // with history
    CHistoryNode* p_history = BeginChangeWH(EHCL_PROPERTY,"restrain property");
    if( p_history == NULL ) return(false);

    RestrainProperty(p_history);

    EndChangeWH();

    return(true);
}

//------------------------------------------------------------------------------

void CProperty::SetName(const QString& name,CHistoryNode* p_history)
{
    CProObject::SetName(name,p_history);
    if( GetPropertyList() ) GetPropertyList()->EmitOnPropertyListChanged();
}

//------------------------------------------------------------------------------

void CProperty::SetDescription(const QString& descrip,CHistoryNode* p_history)
{
    CProObject::SetDescription(descrip,p_history);
    if( GetPropertyList() ) GetPropertyList()->EmitOnPropertyListChanged();
}

//------------------------------------------------------------------------------

void CProperty::RestrainProperty(CHistoryNode* p_history)
{
    CStructure* p_str = GetProject()->GetActiveStructure();
    if( p_str == NULL ) return;

    // is already restrained in the active structure?
    if( p_str->GetRestraints()->IsRestrained(this) ) return;

    // create new restrain
    CRestraint* p_rst = p_str->GetRestraints()->CreateRestraint(GetName(),"",p_history);

    // set property
    p_rst->SetProperty(this,p_history);
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

CPropertyList* CProperty::GetPropertyList(void) const
{
    return(static_cast<CPropertyList*>(parent()));
}

//------------------------------------------------------------------------------

const QString& CProperty::GetPropertyType(void) const
{
    return(GetType().GetName());
}

//------------------------------------------------------------------------------

CPhysicalQuantity* CProperty::GetPropertyUnit(void) const
{
    return(PropUnit);
}

//------------------------------------------------------------------------------

bool CProperty::IsReady(void)
{
    return(false);
}

//------------------------------------------------------------------------------

bool CProperty::IsFromStructure(CStructure* p_str)
{
    return(false);
}

//------------------------------------------------------------------------------

bool CProperty::ComposedBySingleAtomGroups(void)
{
    return(false);
}

//------------------------------------------------------------------------------

bool CProperty::HasGradient(CStructure* p_structure)
{
    return(PropFlags.testFlag(EPF_CARTESIAN_GRADIENT));
}

//------------------------------------------------------------------------------

double CProperty::GetScalarValue(void)
{
    return(0.0);
}

//------------------------------------------------------------------------------

double CProperty::GetScalarDeviation(double target_value)
{
    return(GetScalarValue() - target_value);
}

//------------------------------------------------------------------------------

void CProperty::SetScalarValue(double value)
{
    // nothing to be here
}

//------------------------------------------------------------------------------

double CProperty::GetGradient(QVector<CAtomGrad>& grads)
{
    grads.resize(0);    // no gradients by default
    return(0.0);
}

//------------------------------------------------------------------------------

double CProperty::GetDeviationAndGradient(QVector<CAtomGrad>& grads,double target_value)
{
    return(GetGradient(grads) - target_value);
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

void CProperty::LoadData(CXMLElement* p_ele)
{
    // check input data -----------------------------
    if( p_ele == NULL ) {
        INVALID_ARGUMENT("p_ele is NULL");
    }

    if( p_ele->GetName() != "property" ) {
        LOGIC_ERROR("p_ele is not property");
    }

    // load object info -----------------------------
    CProObject::LoadData(p_ele);
}

// -----------------------------------------------------------------------------

void CProperty::SaveData(CXMLElement* p_ele)
{
    // check input data -----------------------------
    if( p_ele == NULL ) {
        INVALID_ARGUMENT("p_ele is NULL");
    }

    if( p_ele->GetName() != "property" ) {
        LOGIC_ERROR("p_ele is not property");
    }

    // save object info -----------------------------
    CProObject::SaveData(p_ele);

    // save data ------------------------------------
    p_ele->SetAttribute("uuid", GetType().GetFullStringForm());
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================


