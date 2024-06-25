// =============================================================================
// NEMESIS - Molecular Modelling Package
// -----------------------------------------------------------------------------
//    Copyright (C) 2010 Petr Kulhanek, kulhanek@chemi.muni.cz
//    Copyright (C) 2008 Petr Kulhanek, kulhanek@enzim.hu,
//                       Jakub Stepan, xstepan3@chemi.muni.cz
//    Copyright (C) 1998-2004 Petr Kulhanek, kulhanek@chemi.muni.cz
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

#include <Bond.hpp>
#include <BondList.hpp>
#include <NemesisCoreModule.hpp>
#include <CategoryUUID.hpp>
#include <Atom.hpp>
#include <ErrorSystem.hpp>
#include <XMLElement.hpp>
#include <Structure.hpp>
#include <BondHistory.hpp>
#include <AtomList.hpp>
#include <Residue.hpp>
#include <Project.hpp>
#include <HistoryNode.hpp>

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

CExtUUID        BondID(
                    "{BOND:4faa274e-70b5-4dd9-94d2-8e15552c598e}",
                    "Bond");

CPluginObject   BondObject(&NemesisCorePlugin,
                    BondID,STRUCTURE_CAT,
                    ":/images/NemesisCore/structure/Bond.svg",
                    NULL);

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

CBond::CBond(CBondList* p_bl)
    : CProObject(&BondObject,p_bl,p_bl->GetProject())
{
    A1 = NULL;
    A2 = NULL;

    Order = BO_NONE;
    PBCIndex = 0;

    // keep it here!!!, it calls GetName() that depends on A1 and A2
    // default name is empty
    // if the name is empty then it is derived from individual atoms
    // see CBond::GetName()
    SetName("");
}

//------------------------------------------------------------------------------

/* constructor order */
/* CProObject calls GetName that depends on initialized A1 and A2 pointers */

CBond::CBond(CBondList* p_bl,bool no_index)
    : CProObject(&BondObject,p_bl,p_bl->GetProject(),no_index)
{
    A1 = NULL;
    A2 = NULL;
    Order = BO_NONE;
    PBCIndex = 0;

    // keep it here!!!, it calls GetName() that depends on A1 and A2
    // default name is empty
    // if the name is empty then it is derived from individual atoms
    // see CBond::GetName()
    SetName("");
}

//------------------------------------------------------------------------------

CBond::~CBond(void)
{
    CBondList* p_list = GetBonds();
    if( p_list ){
        // this significantly speedup destruction time if the whole structure is destructed
        // see CStructure::~CStructure(void)
        if( p_list->GetStructure() ) setParent(NULL);    // remove object from the list
        p_list->ListSizeChanged();
    }
}

//------------------------------------------------------------------------------

void CBond::RemoveFromBaseList(CHistoryNode* p_history)
{
    // unregister all registered objects
    UnregisterAllRegisteredObjects(p_history);

    // unregister bond for both atoms
    if( A1 != NULL) A1->UnregisterBond(this);
    if( A2 != NULL) A2->UnregisterBond(this);

    // create history record if necessary
    if( p_history != NULL) {
        CBondHI* p_data = new CBondHI(this,EHID_BACKWARD);
        p_history->Register(p_data);
    }

    delete this;
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

bool CBond::DeleteWH(void)
{
    CStructure*       p_str = GetStructure(); // object will be destroyed - use molecule instead
    CHistoryNode*    p_history = NULL;

    p_history = p_str->BeginChangeWH(EHCL_TOPOLOGY,"delete bond");
    if( p_history == NULL ) return (false);

//    UnregisterAllRegisteredObjects(p_history); called in RemoveFromBaseList

    bool destroyed = false;

// keep always atoms
//    if( GetBondOrder() != BO_WEAK ) {
//        if( A1->IsVirtual() ) {
//            A1->RemoveFromBaseList(p_history);
//            destroyed = true;
//        }

//        if( A2->IsVirtual()==true ) {
//            A2->RemoveFromBaseList(p_history);
//            destroyed = true;
//        }
//    }

    if( destroyed == false ) RemoveFromBaseList(p_history);

    p_str->EndChangeWH();

    return(true);
}

//------------------------------------------------------------------------------

bool CBond::BreakWH(void)
{
    if( GetBondOrder() == BO_WEAK ) return(false);    // weak bond cannot be broken
    if( (A1->IsVirtual() == true) || (A2->IsVirtual()== true) ) {
        return(false);  // bond including a hydrogen atom cannot be broken too
    }

    CStructure*       p_str = GetStructure(); // object will be destroyed - use molecule instead
    CHistoryNode*    p_history;

    p_history = p_str->BeginChangeWH(EHCL_TOPOLOGY,"break bond");
    if( p_history == NULL ) return (false);

    Break(p_history);

    p_str->EndChangeWH();

    return(true);
}

//------------------------------------------------------------------------------

bool CBond::SetOrderWH(EBondOrder order)
{
    if( GetBondOrder() == order ) return(true);

    // with history
    CHistoryNode*    p_history;

    p_history = BeginChangeWH(EHCL_TOPOLOGY,"set bond order");
    if( p_history == NULL ) return (false);

    SetBondOrder(order,p_history);

    EndChangeWH();
    return(true);
}

//------------------------------------------------------------------------------

bool CBond::SetBondTypeWH(const QString& type)
{
    if( GetBondType() == type ) return(true);

    // with history
    CHistoryNode*    p_history;

    p_history = BeginChangeWH(EHCL_TOPOLOGY,"set bond type");
    if( p_history == NULL ) return (false);

    SetBondType(type,p_history);

    EndChangeWH();
    return(true);
}

//------------------------------------------------------------------------------

bool CBond::SetFirstAtomWH(CAtom* p_atom)
{
    if( GetFirstAtom() == p_atom ) return(true);

    // with history
    CHistoryNode*    p_history;

    p_history = BeginChangeWH(EHCL_TOPOLOGY,"set bond first atom");
    if( p_history == NULL ) return (false);

    SetFirstAtom(p_atom,p_history);

    EndChangeWH();
    return(true);
}

//------------------------------------------------------------------------------

bool CBond::SetSecondAtomWH(CAtom* p_atom)
{
    if( GetSecondAtom() == p_atom ) return(true);

    // with history
    CHistoryNode*    p_history;

    p_history = BeginChangeWH(EHCL_TOPOLOGY,"set bond second atom");
    if( p_history == NULL ) return (false);

    SetSecondAtom(p_atom,p_history);

    EndChangeWH();
    return(true);
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

CStructure* CBond::GetStructure(void) const
{
    if( GetBonds() == NULL ) return(NULL);
    return( GetBonds()->GetStructure() );
}

//------------------------------------------------------------------------------

CBondList* CBond::GetBonds(void) const
{
    return(static_cast<CBondList*>(parent()));
}

//------------------------------------------------------------------------------

const QString CBond::GetName(void) const
{
    QString name = CProObject::GetName();
    if( name.isEmpty() && (A1 != NULL) && (A2 != NULL) ){
        name = A1->GetName() + "-" + A2->GetName();
    }
    if( (A1 == NULL) || (A2 == NULL) ){
        name = tr("orphaned");
    }
    return(name);
}

//------------------------------------------------------------------------------

EBondOrder CBond::GetBondOrder(void) const
{
    return(Order);
}

//------------------------------------------------------------------------------

QString CBond::GetBondOrderInStringForm(void) const
{
    switch(Order) {
        case BO_NONE:
            return(tr("none"));
        case BO_WEAK:
            return(tr("weak"));
        case BO_SINGLE:
            return(tr("single"));
        case BO_SINGLE_H:
            return(tr("single and half"));
        case BO_DOUBLE:
            return(tr("double"));
        case BO_DOUBLE_H:
            return(tr("double and half"));
        case BO_TRIPLE:
            return(tr("triple"));
    }

    return("");
}

//------------------------------------------------------------------------------

const QString&  CBond::GetBondType(void) const
{
    return(Type);
}

//------------------------------------------------------------------------------

double CBond::GetLength(void) const
{
    if( (A1 == NULL) || (A2 == NULL) ) return(0.0);
    return( Size(A1->GetPos() - A2->GetPos()) );
}

//------------------------------------------------------------------------------

CAtom* CBond::GetFirstAtom(void) const
{
    return(A1);
}

//------------------------------------------------------------------------------

CAtom* CBond::GetSecondAtom(void) const
{
    return(A2);
}

//------------------------------------------------------------------------------

CAtom* CBond::GetOppositeAtom(CAtom* p_a) const
{
    if(p_a == A1) {
        return(A2);
    } else {
        return(A1);
    }
}

//------------------------------------------------------------------------------

CAtom* CBond::GetConnectAtom(void) const
{
    if( (A1 != NULL) && (A1->IsVirtual() == true) ) return(A1);
    if( (A2 != NULL) && (A2->IsVirtual() == true) ) return(A2);
    return(NULL);
}

//------------------------------------------------------------------------------

const CPoint CBond::GetMainVector(void) const
{
    if( (A1 == NULL) || (A2 == NULL) ){
        CPoint mainvector(1.0,0.0,0.0);
        return(mainvector);
    }

    if( (Order == BO_SINGLE) || (Order == BO_TRIPLE) || (Order == BO_WEAK) || (Order == BO_NONE) ){
        // any perpendicular vector does the job
        CPoint thisvector = A1->GetPos() - A2->GetPos();
        CPoint mainvector;
        if( thisvector.x != 0.0 ){
            mainvector.y = thisvector.x;
            mainvector.x = -thisvector.y;
        } else if (thisvector.y != 0.0) {
            mainvector.x = thisvector.y;
            mainvector.y = -thisvector.x;
        } else if (thisvector.z != 0.0) {
            mainvector.y = thisvector.z;
            mainvector.z = -thisvector.y;
        } else {
            mainvector.x = 1.0;
        }
        mainvector = CrossDot(thisvector,mainvector);
        mainvector.Normalize();
        return(mainvector);
    }

    // get main atom
    CAtom* p_matom = A1;
    if( A2->GetNumberOfBonds() > A1->GetNumberOfBonds() ) p_matom = A2;

    CPoint thisvector = A1->GetPos() - A2->GetPos();
    CPoint mainvector;

    if( p_matom->GetNumberOfBonds() <= 2 ){
        // cumullene
        // FIXME - can be done better
        // any perpendicular vector does the job
        mainvector.SetZero();
        if( thisvector.x != 0.0 ){
            mainvector.y = thisvector.x;
            mainvector.x = -thisvector.y;
        } else if (thisvector.y != 0.0) {
            mainvector.x = thisvector.y;
            mainvector.y = -thisvector.x;
        } else if (thisvector.z != 0.0) {
            mainvector.y = thisvector.z;
            mainvector.z = -thisvector.y;
        } else {
            mainvector.x = 1.0;
        }
    } else {
        foreach(CBond* p_b,p_matom->GetBonds() ) {
            if( p_b != this ) {
                CPoint vector = p_b->A1->GetPos() - p_b->A2->GetPos();
                CPoint cv = CrossDot(thisvector,vector);
                if( VectDot(mainvector,cv) < 0.0 ){
                    cv *= -1.0;
                }
                mainvector += cv;
            }
        }
    }

    if( Size(mainvector) > 0.0 ) {
        mainvector.Normalize();
    } else {
        // any perpendicular vector does the job
        mainvector.SetZero();
        if( thisvector.x != 0.0 ){
            mainvector.y = thisvector.x;
            mainvector.x = -thisvector.y;
        } else if (thisvector.y != 0.0) {
            mainvector.x = thisvector.y;
            mainvector.y = -thisvector.x;
        } else if (thisvector.z != 0.0) {
            mainvector.y = thisvector.z;
            mainvector.z = -thisvector.y;
        } else {
            mainvector.x = 1.0;
        }
    }

    // and finaly to the main bond vector
    mainvector = CrossDot(thisvector,mainvector);
    if( Size(mainvector) > 0.0 ) {
        mainvector.Normalize();
    }
    return(mainvector);
}

//------------------------------------------------------------------------------

bool CBond::IsSelectedByResidues(void)
{
    CResidue* p_res1 = NULL;
    CResidue* p_res2 = NULL;
    if( A1 ){
        p_res1 = A1->GetResidue();
    }
    if( A2 ){
        p_res2 = A2->GetResidue();
    }

    bool r1 = false;
    bool r2 = false;
    if( p_res1 ){
        r1 = p_res1->IsFlagSet(EPOF_SELECTED);
    }
    if( p_res2 ){
        r2 = p_res2->IsFlagSet(EPOF_SELECTED);
    }
    return(r1 && r2);
}

//------------------------------------------------------------------------------

int CBond::GetPBCIndex(void)
{
    return(PBCIndex);
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

void CBond::Break(CHistoryNode* p_history)
{
    if( GetBondOrder() == BO_WEAK ) return;    // weak bond cannot be broken
    if( (A1->IsVirtual() == true) || (A2->IsVirtual()== true) ) {
        return;  // bond including a hydrogen atom cannot be broken too
    }

    CStructure*       p_str = GetStructure(); // object will be destroyed - use molecule instead

    CAtom* p_a1;
    CAtom* p_a2;
    CAtom* p_oa1 = A1;
    CAtom* p_oa2 = A2;

    EBondOrder order = GetBondOrder();

    p_a1= p_str->GetAtoms()->CreateAtom(1,(A2->GetPos()   + A1->GetPos()*2)/3,p_history);
    if( p_oa1->GetResidue() ){
        p_a1->SetLocIndex(-1); // no history - to ensure that AddAtom will set correct LocIndex
        p_oa1->GetResidue()->AddAtom(p_a1,p_history);
    }
    p_a2= p_str->GetAtoms()->CreateAtom(1,(A2->GetPos()*2 + A1->GetPos())/3,p_history);
    if( p_oa2->GetResidue() ){
        p_a2->SetLocIndex(-1);
        p_oa2->GetResidue()->AddAtom(p_a2,p_history);
    }

    // remove bond
    RemoveFromBaseList(p_history);

    // create new bonds
    p_str->GetBonds()->CreateBond(p_oa1,p_a1,order,p_history);
    p_str->GetBonds()->CreateBond(p_oa2,p_a2,order,p_history);
}

//------------------------------------------------------------------------------

void CBond::SetName(const QString& name,CHistoryNode* p_history)
{
    CProObject::SetName(name,p_history);
    GetBonds()->EmitOnBondListChanged();
}

//------------------------------------------------------------------------------

void CBond::SetDescription(const QString& descrip,CHistoryNode* p_history)
{
    CProObject::SetDescription(descrip,p_history);
    GetBonds()->EmitOnBondListChanged();
}

//------------------------------------------------------------------------------

void CBond::SetBondOrder(EBondOrder order,CHistoryNode* p_history)
{
    if( Order == order ) return;

    if( p_history ){
        CBondOrderHI* p_hnode = new CBondOrderHI(this,order);
        p_history->Register(p_hnode);
    }

    Order = order;
    emit OnStatusChanged(ESC_OTHER);
    GetBonds()->EmitOnBondListChanged();
}

//------------------------------------------------------------------------------

void CBond::SetBondType(const QString& type,CHistoryNode* p_history)
{
    if( Type == type ) return;

    if( p_history ){
        CBondTypeHI* p_hnode = new CBondTypeHI(this,type);
        p_history->Register(p_hnode);
    }

    Type = type;
    emit OnStatusChanged(ESC_OTHER);
    GetBonds()->EmitOnBondListChanged();
}

//------------------------------------------------------------------------------

void CBond::SetFirstAtom(CAtom* p_a,CHistoryNode* p_history)
{
    if( p_history ){
        CHistoryItem* p_hi = new CBondSetFirstAtomHI(this,p_a);
        p_history->Register(p_hi);
    }

    if( A1 != NULL ) {
        A1->UnregisterBond(this);
    }    
    A1 = p_a;
    if( A1 != NULL ) {
        A1->RegisterBond(this);
    }
    emit OnStatusChanged(ESC_OTHER);
    GetBonds()->EmitOnBondListChanged();
}

//------------------------------------------------------------------------------

void CBond::SetSecondAtom(CAtom* p_a,CHistoryNode* p_history)
{
    if( p_history ){
        CHistoryItem* p_hi = new CBondSetSecondAtomHI(this,p_a);
        p_history->Register(p_hi);
    }

    if( A2 != NULL ) {
        A2->UnregisterBond(this);
    }    
    A2 = p_a;
    if( A2 != NULL ) {
        A2->RegisterBond(this);
    }
    emit OnStatusChanged(ESC_OTHER);
    GetBonds()->EmitOnBondListChanged();
}

//------------------------------------------------------------------------------

void CBond::SetPBCIndex(int index)
{
    PBCIndex = index;
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

void CBond::LoadData(CXMLElement* p_ele)
{
    // check input data -----------------------------
    if( p_ele == NULL ) {
        INVALID_ARGUMENT("p_ele is NULL");
    }

    if( p_ele->GetName() != "bond" ) {
        LOGIC_ERROR("element is not bond");
    }

    if( GetProject() == NULL ) {
        LOGIC_ERROR("GetProject() == NULL");
    }

    // load object info -----------------------------
    CProObject::LoadData(p_ele);

    // read bond specific data (all of them are optional)
    p_ele->GetAttribute<EBondOrder>("ord",Order);
    p_ele->GetAttribute("type",Type);

    // link atoms --------------------------------
    int index;
    p_ele->GetAttribute("a1",index);
    if( index > 0 ){
        index += GetProject()->GetBaseObjectIndex();
        if( A1 != NULL ) {
            A1->UnregisterBond(this);
        }
        A1 = static_cast<CAtom*>(GetProject()->FindObject(index));
        if( A1 ) A1->RegisterBond(this);
    }

    p_ele->GetAttribute("a2",index);
    if( index > 0 ){
        index += GetProject()->GetBaseObjectIndex();
        if( A2 != NULL ) {
            A2->UnregisterBond(this);
        }
        A2 = static_cast<CAtom*>(GetProject()->FindObject(index));
        if( A2 ) A2->RegisterBond(this);
    }
    PBCIndex = 0;
    p_ele->GetAttribute("pbc",PBCIndex);
}

//void CBond::LinkObject(const QString& role,CProObject* p_object)
//{
//    WARNING: do not link atoms via LinkObject since CBond::LoadData already have done
//}

//------------------------------------------------------------------------------

void CBond::SaveData(CXMLElement* p_ele)
{
    // check input data -----------------------------
    if( p_ele == NULL ) {
        INVALID_ARGUMENT("p_ele is NULL");
    }

    if( p_ele->GetName() != "bond" ) {
        LOGIC_ERROR("element is not bond");
    }

    // save object info -----------------------------
    CProObject::SaveData(p_ele);

    if( A1 != NULL ) {
        p_ele->SetAttribute("a1",A1->GetIndex());
    }
    if( A2 != NULL ) {
        p_ele->SetAttribute("a2",A2->GetIndex());
    }
    p_ele->SetAttribute("ord",GetBondOrder());
    if( ! Type.isEmpty() ) p_ele->SetAttribute("type",Type);

    if( PBCIndex > 0 ){
        p_ele->SetAttribute("pbc",PBCIndex);
    }
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

CGeoDescriptor  CBond::GetGeoDescriptor(void)
{
    CGeoDescriptor descrip;

    if( (A1 != NULL) && (A2 != NULL) ) {
        descrip.SetType(EGDT_TWO_POINTS);
        descrip.SetPoint1(A1->GetPos());
        descrip.SetPoint2(A2->GetPos());
    }

    return(descrip);
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

void CBond::RemoveFromRegistered(CProObject* p_object,
                                 CHistoryNode* p_history)
{
    // is it atom that compose bond?
    if( (p_object == A1) || (p_object == A2) ){
        RemoveFromBaseList(p_history);
    }
}

//------------------------------------------------------------------------------

void CBond::ChangeParent(CBondList* p_newparent)
{
    // inform old parent
    GetBonds()->EmitOnBondListChanged();
    // set new parent
    setParent(p_newparent);
    // inform new parent
    GetBonds()->EmitOnBondListChanged();
    // inform object designers
    emit OnStatusChanged(ESC_PARENT);
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================
