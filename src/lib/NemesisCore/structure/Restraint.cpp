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

#include <Restraint.hpp>
#include <RestraintList.hpp>
#include <NemesisCoreModule.hpp>
#include <CategoryUUID.hpp>
#include <Atom.hpp>
#include <ErrorSystem.hpp>
#include <XMLElement.hpp>
#include <Structure.hpp>
#include <RestraintHistory.hpp>
#include <Property.hpp>
#include <FCPhysicalQuantity.hpp>
#include <Project.hpp>


//------------------------------------------------------------------------------

CExtUUID        RestraintID(
                    "{RESTRAINT:ab8eeb7e-8a41-4c9e-83dd-f70a78a658e0}",
                    "Restraint");

CPluginObject   RestraintObject(&NemesisCorePlugin,
                    RestraintID,STRUCTURE_CAT,
                    ":/images/NemesisCore/structure/Restraint.svg",
                    NULL);

//------------------------------------------------------------------------------

#define DEFAULT_FORCE_CONSTANT 5

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

CRestraint::CRestraint(CRestraintList* p_bl)
    : CProObject(&RestraintObject,p_bl,p_bl->GetProject())
{
    ForceConstantPQ = NULL;
    Property = NULL;
    TargetValue = 0.0;
    ForceConstant = 0.0;
    SetFlag(EPOF_ENABLED,true);
}

//------------------------------------------------------------------------------

CRestraint::~CRestraint(void)
{
    CRestraintList* p_list = GetRestraints();
    if( p_list ){
        // this significantly speedup destruction time if the whole structure is destructed
        // see CStructure::~CStructure(void)
        if( p_list->GetStructure() ) setParent(NULL);    // remove object from the list
        p_list->ListSizeChanged();
    }
}

//------------------------------------------------------------------------------

void CRestraint::RemoveFromBaseList(CHistoryNode* p_history)
{
    // create history record if necessary
    if( p_history != NULL) {
        CRestraintHI* p_data = new CRestraintHI(this,EHID_BACKWARD);
        p_history->Register(p_data);
    }

    // set property to NULL
    SetProperty(NULL);

    // unregister all registered objects
    UnregisterAllRegisteredObjects(p_history);

    delete this;
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

bool CRestraint::SetPropertyWH(CProperty* p_prop)
{
    if( Property == p_prop ) return(true);

    CHistoryNode* p_history = BeginChangeWH(EHCL_TOPOLOGY,tr("set restraint associated property"));
    if( p_history == NULL ) return (false);

    SetProperty(p_prop,p_history);

    EndChangeWH();
    return(true);
}

//------------------------------------------------------------------------------

bool CRestraint::SetTargetValueWH(double target)
{
    if( TargetValue == target ) return(true);

    CHistoryNode* p_history = BeginChangeWH(EHCL_TOPOLOGY,tr("set restraint target value"));
    if( p_history == NULL ) return (false);

    SetTargetValue(target,p_history);

    EndChangeWH();
    return(true);
}

//------------------------------------------------------------------------------

bool CRestraint::SetForceConstantWH(double force)
{
    if( ForceConstant == force ) return(true);

    CHistoryNode* p_history = BeginChangeWH(EHCL_TOPOLOGY,tr("set restraint force constant"));
    if( p_history == NULL ) return (false);

    SetForceConstant(force,p_history);

    EndChangeWH();
    return(true);
}

//------------------------------------------------------------------------------

bool CRestraint::ToggleEnabledDisabledWH(void)
{
    return( SetEnabledWH(! IsEnabled()) );
}

//------------------------------------------------------------------------------

bool CRestraint::SetEnabledWH(bool set)
{
    if( IsEnabled() == set ) return(true);

    CHistoryNode* p_history = BeginChangeWH(EHCL_TOPOLOGY,tr("disable/enable restraint"));
    if( p_history == NULL ) return (false);

    SetEnabled(set,p_history);

    EndChangeWH();
    return(true);
}

//------------------------------------------------------------------------------

bool CRestraint::DeleteRestraintWH(void)
{
    // object will be destroyed - use list instead
    CRestraintList* p_list = GetRestraints();

    CHistoryNode*  p_history = p_list->BeginChangeWH(EHCL_GEOMETRY,tr("delete restraint"));
    if( p_history == NULL ) return (false);

    RemoveFromBaseList(p_history);

    p_list->EndChangeWH();
    return(true);
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

CStructure* CRestraint::GetStructure(void) const
{
    if( GetRestraints() == NULL ) return(NULL);
    return( GetRestraints()->GetStructure() );
}

//------------------------------------------------------------------------------

CRestraintList* CRestraint::GetRestraints(void) const
{
    return(static_cast<CRestraintList*>(parent()));
}

//------------------------------------------------------------------------------

CProperty* CRestraint::GetProperty(void) const
{
    return(Property);
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

void CRestraint::SetName(const QString& name,CHistoryNode* p_history)
{
    CProObject::SetName(name,p_history);
    GetRestraints()->EmitOnRestraintListChanged();
}

//------------------------------------------------------------------------------

void CRestraint::SetDescription(const QString& descrip,CHistoryNode* p_history)
{
    CProObject::SetDescription(descrip,p_history);
    GetRestraints()->EmitOnRestraintListChanged();
}

//------------------------------------------------------------------------------

void CRestraint::SetProperty(CProperty* p_prop,CHistoryNode* p_history)
{
    if( p_prop == Property ) return;

    if( p_history ){
        CHistoryItem* p_hi = new CRestraintPropertyHI(this,p_prop);
        p_history->Register(p_hi);
    }

    if( Property ){
        Property->UnregisterObject(this);
        Property->disconnect(GetRestraints());
        delete ForceConstantPQ;
        ForceConstantPQ = NULL;
    }

    Property = p_prop;

    if( Property ){
        Property->RegisterObject(this);
        connect(Property,SIGNAL(OnStatusChanged(EStatusChanged)),
                GetRestraints(),SLOT(EmitOnRestraintListChanged()));
        ForceConstantPQ = new CFCPhysicalQuantity(this,tr("force"),Property->GetPropertyUnit());
        if( Property->IsReady() ){
            SetTargetValue(Property->GetScalarValue(),p_history);
        }
        if( ForceConstant == 0 ){
            SetForceConstant(DEFAULT_FORCE_CONSTANT,p_history);
        }
    }

    emit OnStatusChanged(ESC_OTHER);

    GetRestraints()->EmitOnRestraintListChanged();
}

//------------------------------------------------------------------------------

void CRestraint::SetTargetValue(double target,CHistoryNode* p_history)
{
    if( target == TargetValue ) return;

    if( p_history ){
        CHistoryItem* p_hi = new CRestraintTargetHI(this,target);
        p_history->Register(p_hi);
    }

    TargetValue = target;
    emit OnStatusChanged(ESC_OTHER);

    GetRestraints()->EmitOnRestraintListChanged();
}

//------------------------------------------------------------------------------

void CRestraint::SetForceConstant(double force,CHistoryNode* p_history)
{
    if( force == ForceConstant ) return;

    if( p_history ){
        CHistoryItem* p_hi = new CRestraintForceHI(this,force);
        p_history->Register(p_hi);
    }

    ForceConstant = force;
    emit OnStatusChanged(ESC_OTHER);

    GetRestraints()->EmitOnRestraintListChanged();
}

//------------------------------------------------------------------------------

void CRestraint::SetEnabled(bool set,CHistoryNode* p_history)
{
    if( set == IsEnabled() ) return;
    CProObjectFlags flags = GetFlags();
    SET_FLAG(flags,EPOF_ENABLED,set);
    SetFlags(flags,p_history);
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

double CRestraint::GetTargetValue(void)
{
    return(TargetValue);
}

//------------------------------------------------------------------------------

double CRestraint::GetCurrentValue(void)
{
    if( Property == NULL ){
        return(0.0);
    }
    return(Property->GetScalarValue());
}

//------------------------------------------------------------------------------

double CRestraint::GetDeviationValue(void)
{
    if( Property == NULL ){
        return(0.0);
    }
    return(Property->GetScalarDeviation(TargetValue));
}

//------------------------------------------------------------------------------

double CRestraint::GetForceConstant(void)
{
    return(ForceConstant);
}

//------------------------------------------------------------------------------

CPhysicalQuantity* CRestraint::GetValueUnit(void)
{
    if( Property == NULL ) return(NULL);
    return(Property->GetPropertyUnit());
}

//------------------------------------------------------------------------------

CPhysicalQuantity* CRestraint::GetForceConstantUnit(void)
{
    return(ForceConstantPQ);
}

//------------------------------------------------------------------------------

bool CRestraint::IsEnabled(void)
{
    return(IsFlagSet(EPOF_ENABLED));
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

double CRestraint::GetEnergy(QVector<CAtomGrad>& grads)
{
    if( (Property == NULL) || (Property->IsReady() == false) ){
        grads.resize(0);
        return(0.0);
    }
    double dv = Property->GetDeviationAndGradient(grads,TargetValue);

    // value
    double ene = 0.5*ForceConstant*dv*dv;
    double deri = ForceConstant*dv;

    // and gradiend
    for(int i=0; i < grads.size(); i++){
        grads[i].Grad = grads.at(i).Grad * deri;
    }

    return(ene);
}

//------------------------------------------------------------------------------

double CRestraint::GetEnergy(void)
{
    double dv = GetDeviationValue();
    return( 0.5*ForceConstant*dv*dv );
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

void CRestraint::RemoveFromRegistered(CProObject* p_object,CHistoryNode* p_history)
{
    if( p_object == Property ){
        RemoveFromBaseList(p_history);
    }
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

void CRestraint::LoadData(CXMLElement* p_ele)
{
    // check input data -----------------------------
    if( p_ele == NULL ) {
        INVALID_ARGUMENT("p_ele is NULL");
    }

    if( p_ele->GetName() != "restraint" ) {
        LOGIC_ERROR("element is not restraint");
    }

    // load object info -----------------------------
    CProObject::LoadData(p_ele);

    // load restraint data --------------------------
    p_ele->GetAttribute("target",TargetValue);
    p_ele->GetAttribute("force",ForceConstant);
    GetProject()->RequestToLinkObject(this,p_ele,"pid");
}

//------------------------------------------------------------------------------

void CRestraint::LinkObject(const QString& role,CProObject* p_object)
{
    if( role == "pid" ){
        CProperty* p_prop = dynamic_cast<CProperty*>(p_object);
        SetProperty(p_prop);
    }
}

//------------------------------------------------------------------------------

void CRestraint::SaveData(CXMLElement* p_ele)
{
    // check input data -----------------------------
    if( p_ele == NULL ) {
        INVALID_ARGUMENT("p_ele is NULL");
    }

    if( p_ele->GetName() != "restraint" ) {
        LOGIC_ERROR("element is not restraint");
    }

    // save object info -----------------------------
    CProObject::SaveData(p_ele);

    // restraint data -------------------------------
    if( Property ){
        p_ele->SetAttribute("pid",Property->GetIndex());
    }
    p_ele->SetAttribute("target",TargetValue);
    p_ele->SetAttribute("force",ForceConstant);
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

void CRestraint::ChangeParent(CRestraintList* p_newparent)
{
    // inform old parent
    GetRestraints()->EmitOnRestraintListChanged();
    // set new parent
    setParent(p_newparent);
    // inform new parent
    GetRestraints()->EmitOnRestraintListChanged();
    // inform object designers
    emit OnStatusChanged(ESC_PARENT);
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================



