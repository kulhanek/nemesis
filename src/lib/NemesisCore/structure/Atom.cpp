// =============================================================================
// NEMESIS - Molecular Modelling Package
// -----------------------------------------------------------------------------
//    Copyright (C) 2024 Petr Kulhanek, kulhanek@chemi.muni.cz
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

#include <Atom.hpp>
#include <AtomList.hpp>
#include <NemesisCoreModule.hpp>
#include <CategoryUUID.hpp>
#include <XMLElement.hpp>
#include <ErrorSystem.hpp>
#include <AtomHistory.hpp>
#include <Structure.hpp>
#include <BondList.hpp>
#include <AtomBondsModel.hpp>
#include <Residue.hpp>
#include <PeriodicTable.hpp>
#include <Project.hpp>
#include <HistoryNode.hpp>
#include <Snapshot.hpp>
#include <random>

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

CExtUUID        AtomID(
                    "{ATOM:4ac377be-76d0-4b3b-9045-6f6b865350f3}",
                    "Atom");

CPluginObject   AtomObject(&NemesisCorePlugin,
                    AtomID,STRUCTURE_CAT,
                    ":/images/NemesisCore/structure/Atom.svg",
                    NULL);

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

CAtom::CAtom(CAtomList* p_bl)
    : CProObject(&AtomObject,p_bl,p_bl->GetProject())
{
    SerIndex = 0;
    LocIndex = 0;
    Z = 0;
    Charge = 0;
    Residue = NULL;
    TrajIndex = -1;
}

//------------------------------------------------------------------------------

CAtom::CAtom(CAtomList *p_bl,bool no_index)
    : CProObject(&AtomObject,p_bl,p_bl->GetProject(),no_index)
{
    SerIndex = 0;
    LocIndex = 0;
    Z = 0;
    Charge = 0;
    Residue = NULL;
    TrajIndex = -1;
}

//------------------------------------------------------------------------------

CAtom::~CAtom(void)
{
    CAtomList* p_list = GetAtoms();
    if( p_list ){
        // this significantly speedup destruction time if the whole structure is destructed
        // see CStructure::~CStructure(void)
        if( p_list->GetStructure() ) setParent(NULL);    // remove object from the list
        p_list->ListSizeChanged();
    }
}

// ----------------------------------------------------------------------------

void CAtom::RemoveFromBaseList(CHistoryNode* p_history)
{
    CProObject* p_obj;

    // unregister residue
    if( Residue ){
        Residue->RemoveAtom(this,p_history);
    }

    // unregister bonds
    while( (p_obj = RemoveBondFromBegin()) != NULL ) {
        p_obj->RemoveFromRegistered(this,p_history);
    }

    // unregister other objects
    while( (p_obj = RemoveFirstFromROList()) !=NULL ) {
        p_obj->RemoveFromRegistered(this,p_history);
    }

    // register elementary change to history list
    if( p_history != NULL ) {
        CAtomHI* p_atomdata = new CAtomHI(this,EHID_BACKWARD);
        p_history->Register(p_atomdata);
    }

    // --------------------------------
    delete this;  // destroy object
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

bool CAtom::SetSerIndexWH(int ser_idx)
{
    if( GetSerIndex() == ser_idx ) return(true);

    // with history
    CHistoryNode* p_history = BeginChangeWH(EHCL_TOPOLOGY,"atom serial index");
    if( p_history == NULL ) return(false);

    SetSerIndex(ser_idx,p_history);

    EndChangeWH();
    return(true);
}

//------------------------------------------------------------------------------

bool CAtom::SetLocIndexWH(int loc_idx)
{
    if( GetSerIndex() == loc_idx ) return(true);

    // with history
    CHistoryNode* p_history = BeginChangeWH(EHCL_TOPOLOGY,"atom local index");
    if( p_history == NULL ) return(false);

    SetLocIndex(loc_idx,p_history);

    EndChangeWH();
    return(true);
}

//------------------------------------------------------------------------------

bool CAtom::SetTypeWH(const QString& atom_type)
{
    if( GetType() == atom_type ) return(true);

    // with history
    CHistoryNode* p_history = BeginChangeWH(EHCL_TOPOLOGY,"atom type");
    if( p_history == NULL ) return(false);

    SetType(atom_type,p_history);

    EndChangeWH();
    return(true);
}

//------------------------------------------------------------------------------

bool CAtom::SetZWH(int newz)
{
    if( GetZ() == newz ) return(true);

    // with history
    CHistoryNode* p_history = BeginChangeWH(EHCL_TOPOLOGY,"atom proton number");
    if( p_history == NULL ) return(false);

    SetZ(newz,p_history);

    EndChangeWH();
    return(true);
}

//------------------------------------------------------------------------------

bool CAtom::SetChargeWH(double charge)
{
    if( GetCharge() == charge ) return(true);

    // with history
    CHistoryNode* p_history = BeginChangeWH(EHCL_TOPOLOGY,"atom charge");
    if( p_history == NULL ) return(false);

    SetCharge(charge,p_history);

    EndChangeWH();
    return(true);
}

//------------------------------------------------------------------------------

bool CAtom::SetPosWH(const CPoint& pos)
{
    if( Pos == pos ) return(true);

    // with history
    CHistoryNode* p_history = BeginChangeWH(EHCL_GEOMETRY,"atom position");
    if( p_history == NULL ) return(false);

    SetPos(pos,p_history);

    EndChangeWH();
    return(true);
}

//------------------------------------------------------------------------------

bool CAtom::SetVelWH(const CPoint& vel)
{
    if( Vel == vel ) return(true);

    // with history
    CHistoryNode* p_history = BeginChangeWH(EHCL_GEOMETRY,"atom velocity");
    if( p_history == NULL ) return(false);

    SetVel(vel,p_history);

    EndChangeWH();
    return(true);
}

//------------------------------------------------------------------------------

bool CAtom::DeleteWH(void)
{
    CStructure*       p_str = GetStructure(); // object will be destroyed - use molecule instead
    CHistoryNode*    p_history = NULL;

    p_history = p_str->BeginChangeWH(EHCL_TOPOLOGY,"delete atom");
    if( p_history == NULL ) return (false);

    CBond*         p_b;
    CAtom*         p_na;
    CAtom*         p_a1;
    CPoint         P;

    if( IsVirtual() == false ) {
        p_b = RemoveBondFromBegin();
        while( p_b != NULL ) {
            if( p_b->GetBondOrder() == BO_WEAK ) {
                p_b->RemoveFromRegistered(this,p_history);
            } else {
                p_a1 = p_b->GetOppositeAtom(this);

                if( p_a1->IsVirtual() == true ) {
                    p_a1->RemoveFromBaseList(p_history);
                } else {
                    P = (GetPos() + p_a1->GetPos())/2.0;
                    p_na = p_str->GetAtoms()->CreateAtom(1,P,p_history);
                    if( GetResidue() ){
                        GetResidue()->AddAtom(p_na,p_history);
                    }
                    p_b->RemoveFromRegistered(this,p_history);
                    p_str->GetBonds()->CreateBond(p_na,p_a1,BO_SINGLE,p_history);
                }
            }
            p_b = RemoveBondFromBegin();
        }
    }

    RemoveFromBaseList(p_history);

    p_str->EndChangeWH();

    return(true);
}

//------------------------------------------------------------------------------

bool CAtom::MakeBondWithWH(CAtom* p_atom,EBondOrder order)
{
    CStructure*     p_str = GetStructure(); // object might be destroyed - use molecule instead
    CHistoryNode*   p_history = NULL;

    // atom must be from the same structure
    if( p_atom->GetStructure() != GetStructure() ){
        GetProject()->TextNotification(ETNT_ERROR,tr("atoms must be from the same structure"),ETNT_WARNING_DELAY);
        return(false);
    }

    // virtual atoms cannot be connected to the same atom
    if( IsVirtual() && p_atom->IsVirtual() ){
        CBond* p_b1 = GetConnectBond();
        CBond* p_b2 = p_atom->GetConnectBond();
        if( (p_b1 != NULL) && (p_b2 != NULL) ){
            if( p_b1->GetOppositeAtom(this) == p_b2->GetOppositeAtom(p_atom) ){
                GetProject()->TextNotification(ETNT_WARNING,tr("valence atoms are connected to the same atom"),ETNT_WARNING_DELAY);
                return(false);
            }
        }
    }

    // there is already a bond between master atoms
    if( IsVirtual() && p_atom->IsVirtual() ){
        CBond* p_b1 = GetConnectBond();
        CBond* p_b2 = p_atom->GetConnectBond();
        if( (p_b1 != NULL) && (p_b2 != NULL) ){
            CAtom* p_a1 = p_b1->GetOppositeAtom(this);
            CAtom* p_a2 = p_b2->GetOppositeAtom(p_atom);
            if( (p_a1 != NULL) && (p_a1->IsBondedWith(p_a2)) ){
                GetProject()->TextNotification(ETNT_WARNING,tr("valence atoms are already connected through their master atoms"),ETNT_WARNING_DELAY);
                return(false);
            }
        }
    }

    // there is already a bond
    if( IsBondedWith(p_atom) ){
        GetProject()->TextNotification(ETNT_WARNING,tr("atoms are already bonded"),ETNT_WARNING_DELAY);
        return(false);
    }

    p_history = p_str->BeginChangeWH(EHCL_TOPOLOGY,"make bond");
    if( p_history == NULL ) return (false);

    CAtom*     p_a1;
    CAtom*     p_a2;
    CBond*     p_bond;

    if( (p_bond = IsBondedWith(p_atom)) == NULL ) {
        // bond does not exist
        if( p_atom->IsVirtual() == true) {
            if( (p_bond = p_atom->GetConnectBond()) != NULL ) {
                p_a2 = p_bond->GetOppositeAtom(p_atom);
                p_atom->RemoveFromBaseList(p_history);
            } else {
                p_a2 = p_atom;
            }
        } else {
            p_a2 = p_atom;
        }

        if( IsVirtual() == true ) {
            if( (p_bond = GetConnectBond()) != NULL ) {
                p_a1 = p_bond->GetOppositeAtom(this);
                RemoveFromBaseList(p_history);
            } else {
                p_a1 = this;
            }
        } else {
            p_a1 = this;
        }
    } else {
        // bond exist
        p_a1 = p_atom;
        p_a2 = this;
    }

    if( (p_bond = p_a1->IsBondedWith(p_a2)) == NULL ) {
        p_bond = p_str->GetBonds()->CreateBond(p_a1,p_a2,order,p_history);
    } else {
        // bond already exists - change order only
        p_bond->SetOrderWH(order);
    }

    p_str->EndChangeWH();

    return(true);
}

//------------------------------------------------------------------------------

bool CAtom::SetResidueWH(CResidue* p_res)
{
    if( GetResidue() == p_res ) return(true);

    CStructure* p_str = GetStructure();
    CHistoryNode* p_history = p_str->BeginChangeWH(EHCL_TOPOLOGY,"set atom residue");
    if( p_history == NULL ) return (false);

    CResidue* p_ores = GetResidue();
    if( p_ores ){
        p_ores->RemoveAtom(this,p_history);
    }
    if( p_res ){
        p_res->AddAtom(this,p_history);
    }

    emit OnStatusChanged(ESC_OTHER);
    GetAtoms()->EmitOnAtomListChanged();

    p_str->EndChangeWH();
    return(true);
}

//------------------------------------------------------------------------------

bool CAtom::AddValenceWH(void)
{
    // with history
    CHistoryNode* p_history = BeginChangeWH(EHCL_GEOMETRY,"add a new valence");
    if( p_history == NULL ) return(false);

    // calculate the COG of all other atoms connected to this one
    CPoint ocog;

    int natoms = 0;
    foreach(CBond *p_b,Bonds){
        CAtom* p_oatm = p_b->GetOppositeAtom(this);
        ocog += p_oatm->GetPos();
        natoms++;
    }

    CPoint dir; // direction of new valence
    if( natoms > 0.0 ){
        ocog /= (double)natoms;
        dir  = GetPos() - ocog;
    }

    if( (natoms == 0) || (Size(dir) < 0.001) ){
        // random vector - if no suitable direction
        double lower_bound = 0.0;
        double upper_bound = 1.0;
        std::uniform_real_distribution<double> unif(lower_bound,upper_bound);
        std::default_random_engine re;
        re.seed(std::chrono::system_clock::now().time_since_epoch().count());
        dir.x = unif(re);
        dir.y = unif(re);
        dir.z = unif(re);
    }

    // normalize
    dir.Normalize();

    // position of new atom
    ocog = dir + GetPos();

    // add atom
    CAtom* p_natm = GetAtoms()->CreateAtom(1,ocog,p_history);

    // and bond
    GetStructure()->GetBonds()->CreateBond(this,p_natm,BO_SINGLE,p_history);

    EndChangeWH();
    return(true);
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

CStructure* CAtom::GetStructure(void) const
{
    if( GetAtoms() == NULL ) return(NULL);
    return( GetAtoms()->GetStructure() );
}

//------------------------------------------------------------------------------

CAtomList* CAtom::GetAtoms(void) const
{
    return(static_cast<CAtomList*>(parent()));
}

//------------------------------------------------------------------------------

CResidue* CAtom::GetResidue(void) const
{
    return(Residue);
}

//------------------------------------------------------------------------------

int CAtom::GetSerIndex(void) const
{
    return(SerIndex);
}

//------------------------------------------------------------------------------

int CAtom::GetLocIndex(void) const
{
    return(LocIndex);
}

//------------------------------------------------------------------------------

int CAtom::GetTrajIndex(void) const
{
    return(TrajIndex);
}

//------------------------------------------------------------------------------

int CAtom::GetZ(void) const
{
    return(Z);
}

//------------------------------------------------------------------------------

double CAtom::GetCharge(void) const
{
    return(Charge);
}

//------------------------------------------------------------------------------

double CAtom::GetMass(void) const
{
    return(PeriodicTable.GetMass(Z));
}

//------------------------------------------------------------------------------

const QString& CAtom::GetType(void) const
{
    return(AtomType);
}

//------------------------------------------------------------------------------

const CPoint& CAtom::GetPos(void) const
{
    if( TrajIndex < 0 ){
        return(Pos);
    }
    CSnapshot* p_snap = GetSnapshot();
    if( p_snap ){
        return( p_snap->GetPos(TrajIndex) );
    }
    return(Pos);
}

//------------------------------------------------------------------------------

const CPoint& CAtom::GetBasePos(void) const
{
    return(Pos);
}

//------------------------------------------------------------------------------

const CPoint& CAtom::GetVel(void) const
{
    if( TrajIndex < 0 ){
        return(Vel);
    }
    CSnapshot* p_snap = GetSnapshot();
    if( p_snap ){
        return( p_snap->GetVel(TrajIndex) );
    }
    return(Vel);
}

//------------------------------------------------------------------------------

const CPoint& CAtom::GetBaseVel(void) const
{
    return(Vel);
}

//------------------------------------------------------------------------------

bool CAtom::IsVirtual(void)
{
    return( (Z==0) || (Z==1) );
}

//------------------------------------------------------------------------------

CBond* CAtom::IsBondedWith(const CAtom *p_a2)
{
    foreach(CBond *p_b,Bonds){
        if( p_a2 == p_b->GetOppositeAtom(this) ) return(p_b);
    }

    return(NULL);
}

//------------------------------------------------------------------------------

CBond* CAtom::GetConnectBond(void)
{
    if( IsVirtual() == false ) return(NULL);

    foreach(CBond *p_b,Bonds){
        if( p_b->GetBondOrder() != BO_WEAK ) return(p_b);
    }

    return(NULL);
}

//------------------------------------------------------------------------------

CGeoDescriptor CAtom::GetGeoDescriptor(void)
{
    CGeoDescriptor descrip;

    descrip.SetType(EGDT_ONE_POINT);
    descrip.SetPoint1(GetPos());

    return(descrip);
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

void CAtom::SetName(const QString& name,CHistoryNode* p_history)
{
    CProObject::SetName(name,p_history);
    GetAtoms()->EmitOnAtomListChanged();
    if( GetResidue() ){
        GetResidue()->EmitOnAtomListChanged();
    }
}

//------------------------------------------------------------------------------

void CAtom::SetDescription(const QString& descrip,CHistoryNode* p_history)
{
    CProObject::SetDescription(descrip,p_history);
    GetAtoms()->EmitOnAtomListChanged();
    if( GetResidue() ){
        GetResidue()->EmitOnAtomListChanged();
    }
}

//------------------------------------------------------------------------------

void CAtom::SetSerIndex(int ser_idx,CHistoryNode* p_history)
{
    if( SerIndex == ser_idx ) return;

    if( p_history ){
        CAtomSerIndexHI* p_hnode = new CAtomSerIndexHI(this,ser_idx);
        p_history->Register(p_hnode);
    }

    SerIndex = ser_idx;
    emit OnStatusChanged(ESC_OTHER);
    GetAtoms()->EmitOnAtomListChanged();
    if( GetResidue() ){
        GetResidue()->EmitOnAtomListChanged();
    }
}

//------------------------------------------------------------------------------

void CAtom::SetLocIndex(int loc_idx,CHistoryNode* p_history)
{
    if( LocIndex == loc_idx ) return;

    if( p_history ){
        CAtomLocIndexHI* p_hnode = new CAtomLocIndexHI(this,loc_idx);
        p_history->Register(p_hnode);
    }

    LocIndex = loc_idx;
    emit OnStatusChanged(ESC_OTHER);
    GetAtoms()->EmitOnAtomListChanged();
    if( GetResidue() ){
        GetResidue()->EmitOnAtomListChanged();
    }
}

//------------------------------------------------------------------------------

void CAtom::SetZ(int z,CHistoryNode* p_history)
{
    if( Z == z ) return;

    if( p_history ){
        CAtomZHI* p_hnode = new CAtomZHI(this,z);
        p_history->Register(p_hnode);
    }

    Z = z;
    emit OnStatusChanged(ESC_OTHER);

    // set name
    if( GetName().isEmpty() ){
        QString name = "%1%2";
        name = name.arg(PeriodicTable.GetSymbol(Z)).arg(GetIndex());
        SetName(name,p_history);
    }

    GetAtoms()->EmitOnAtomListChanged();
    if( GetResidue() ){
        GetResidue()->EmitOnAtomListChanged();
    }
}

//------------------------------------------------------------------------------

void CAtom::SetCharge(double charge,CHistoryNode* p_history)
{
    if( Charge == charge ) return;

    if( p_history ){
        CAtomChargeHI* p_hnode = new CAtomChargeHI(this,charge);
        p_history->Register(p_hnode);
    }

    Charge = charge;
    emit OnStatusChanged(ESC_OTHER);
    GetAtoms()->EmitOnAtomListChanged();
    if( GetResidue() ){
        GetResidue()->EmitOnAtomListChanged();
    }
}

//------------------------------------------------------------------------------

void CAtom::SetType(const QString& atomtype,CHistoryNode* p_history)
{
    if( AtomType == atomtype ) return;

    if( p_history ){
        CAtomTypeHI* p_hnode = new CAtomTypeHI(this,atomtype);
        p_history->Register(p_hnode);
    }

    AtomType = atomtype;
    emit OnStatusChanged(ESC_OTHER);
    GetAtoms()->EmitOnAtomListChanged();
    if( GetResidue() ){
        GetResidue()->EmitOnAtomListChanged();
    }
}

//------------------------------------------------------------------------------

void CAtom::SetPos(const CPoint& pos,CHistoryNode* p_history)
{
    if( Pos == pos ) return;

    if( p_history ){
        CAtomPosHI* p_hnode = new CAtomPosHI(this,pos);
        p_history->Register(p_hnode);
    }

    Pos = pos;
    if( GetStructure()->GeometryUpdateLevel == 0 ){
        emit OnStatusChanged(ESC_OTHER);
        GetAtoms()->EmitOnAtomListChanged();
        if( GetResidue() ){
            GetResidue()->EmitOnAtomListChanged();
        }
        if( GetStructure() ){
            GetStructure()->NotifyGeometryChangeTick();
        }
    }
}

//------------------------------------------------------------------------------

void CAtom::SetVel(const CPoint& vel,CHistoryNode* p_history)
{
    if( Vel == vel ) return;

    if( p_history ){
        CAtomVelHI* p_hnode = new CAtomVelHI(this,vel);
        p_history->Register(p_hnode);
    }

    Vel = vel;
    if( GetStructure()->GeometryUpdateLevel == 0 ){
        emit OnStatusChanged(ESC_OTHER);
        GetAtoms()->EmitOnAtomListChanged();
        if( GetResidue() ){
            GetResidue()->EmitOnAtomListChanged();
        }
        if( GetStructure() ){
            GetStructure()->NotifyGeometryChangeTick();
        }
    }
}

//------------------------------------------------------------------------------

void CAtom::SetTrajIndex(int manip_idx)
{
    if( TrajIndex == manip_idx ) return;
    TrajIndex = manip_idx;
}

//------------------------------------------------------------------------------

bool CAtom::RegisterBond(CBond *p_bond)
{
    if( Bonds.contains(p_bond) ) return(false);
    Bonds.append(p_bond);
    emit OnBondRegistered(p_bond);
    return(true);
}

//------------------------------------------------------------------------------

bool CAtom::UnregisterBond(CBond *p_bond)
{
    if( ! Bonds.contains(p_bond) ) return(false);
    Bonds.removeOne(p_bond);
    emit OnBondUnregistered(p_bond);
    return(true);
}

//------------------------------------------------------------------------------

int CAtom::GetNumberOfBonds(void) const
{
    return(Bonds.count());
}

//------------------------------------------------------------------------------

const QList<CBond*>& CAtom::GetBonds(void) const
{
    return(Bonds);
}

//------------------------------------------------------------------------------

CContainerModel* CAtom::GetRegBondsContainerModel(QObject* p_parent)
{
    // data sent to CreateObject points to enumerated object
    // reason is that only object itself known how to use this information
    // model ownership is then solved via setParent() method
    CAtomBondsModel* p_obj = new CAtomBondsModel(NULL);
    p_obj->SetRootObject(this);
    p_obj->setParent(p_parent);
    return(p_obj);
}

//------------------------------------------------------------------------------

CSnapshot* CAtom::GetSnapshot(void) const
{
    CAtomList* p_list = GetAtoms();
    if( p_list == NULL ) return(NULL);
    return( p_list->GetSnapshot() );
}

//------------------------------------------------------------------------------

void CAtom::EmitOnStatusChanged(void)
{
    emit OnStatusChanged(ESC_OTHER);
    GetAtoms()->EmitOnAtomListChanged();
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

void CAtom::LoadData(CXMLElement* p_ele)
{
    // check input data -----------------------------
    if( p_ele == NULL ) {
        INVALID_ARGUMENT("p_ele is NULL");
    }

    if( p_ele->GetName() != "atom" ) {
        LOGIC_ERROR("element is not atom");
    }

    if( GetProject() == NULL ) {
        LOGIC_ERROR("GetProject() == NULL");
    }

    // load object info -----------------------------
    CProObject::LoadData(p_ele);

    // read atom specific data (all of them are optional)
    p_ele->GetAttribute("ai",SerIndex);
    p_ele->GetAttribute("li",LocIndex);

    int rid = 0;
    if( p_ele->GetAttribute("rid",rid) == true ){
        rid += GetProject()->GetBaseObjectIndex();
        CResidue* p_res = static_cast<CResidue*>(GetProject()->FindObject(rid));
        if( p_res ) p_res->AddAtom(this);
    }

    p_ele->GetAttribute("at",AtomType);
    p_ele->GetAttribute("z",Z);
    p_ele->GetAttribute("charge",Charge);

    p_ele->GetAttribute("px",Pos.x);
    p_ele->GetAttribute("py",Pos.y);
    p_ele->GetAttribute("pz",Pos.z);

    Vel.SetZero();
    p_ele->GetAttribute("vx",Vel.x);
    p_ele->GetAttribute("vy",Vel.y);
    p_ele->GetAttribute("vz",Vel.z);
}

//------------------------------------------------------------------------------

//void CAtom::LinkObject(const QString& role,CProObject* p_object)
//{
//    WARNING: do not link residues via LinkObject since CAtom::LoadData already have done
//}

//------------------------------------------------------------------------------

void CAtom::SaveData(CXMLElement* p_ele)
{
    // check input data -----------------------------
    if( p_ele == NULL ) {
        INVALID_ARGUMENT("p_ele is NULL");
    }

    if( p_ele->GetName() != "atom" ) {
        LOGIC_ERROR("element is not atom");
    }

    // save object info -----------------------------
    CProObject::SaveData(p_ele);

    // save atom specific data
    p_ele->SetAttribute("ai",SerIndex);
    p_ele->SetAttribute("li",LocIndex);
    if( GetResidue() ){
        p_ele->SetAttribute("rid",GetResidue()->GetIndex());
    } else {
        // we can save atom without residue, this is for example used during
        // structure building and registering changes to the history list
    }
    if( ! AtomType.isEmpty() ) p_ele->SetAttribute("at",AtomType);
    p_ele->SetAttribute("z",Z);
    if( Charge != 0 ) p_ele->SetAttribute("charge",Charge);

    CPoint pos = GetPos();  // get prosition via GetPos - it also consider trajectory if it is attached
    p_ele->SetAttribute("px",pos.x);
    p_ele->SetAttribute("py",pos.y);
    p_ele->SetAttribute("pz",pos.z);

    CPoint vel = GetVel(); // get prosition via GetVel - it also consider trajectory if it is attached
    if( (vel.x > 0) || (vel.y > 0) || (vel.z > 0) ){
        p_ele->SetAttribute("vx",vel.x);
        p_ele->SetAttribute("vy",vel.y);
        p_ele->SetAttribute("vz",vel.z);
    }
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

CBond* CAtom::RemoveBondFromBegin(void)
{
    if( Bonds.isEmpty() ) return(NULL);
    return( Bonds.takeFirst() );
}

//------------------------------------------------------------------------------

void CAtom::ChangeParent(CAtomList* p_newparent)
{
    // inform old parent
    GetAtoms()->EmitOnAtomListChanged();
    // set new parent
    setParent(p_newparent);
    // inform new parent
    GetAtoms()->EmitOnAtomListChanged();
    // inform object designers
    emit OnStatusChanged(ESC_PARENT);
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================


