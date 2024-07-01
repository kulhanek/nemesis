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

#include <Structure.hpp>
#include <NemesisCoreModule.hpp>
#include <CategoryUUID.hpp>
#include <Project.hpp>
#include <ErrorSystem.hpp>
#include <XMLElement.hpp>
#include <PeriodicTable.hpp>
#include <AtomList.hpp>
#include <BondList.hpp>
#include <ResidueList.hpp>
#include <RestraintList.hpp>
#include <Atom.hpp>
#include <Residue.hpp>
#include <StructureHistory.hpp>
#include <OpenBabelUtils.hpp>
#include <StructureList.hpp>
#include <HistoryNode.hpp>
#include <vector>
#include <Trajectory.hpp>

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

CExtUUID        StructureID(
                    "{STRUCTURE:89533ab0-0244-4a03-8b5c-7d82ebbe2254}",
                    "Structure");

CPluginObject   StructureObject(&NemesisCorePlugin,
                    StructureID,STRUCTURE_CAT,
                    ":/images/NemesisCore/structure/Structure.svg",
                    NULL);

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

CStructure::CStructure(CStructureList* p_list)
    :   CProObject(&StructureObject,p_list,p_list->GetProject())
{
    ConFlags |= EECOF_SUB_CONTAINERS;

    Atoms = new CAtomList(this);
    Bonds = new CBondList(this);
    Residues = new CResidueList(this);
    Restraints = new CRestraintList(this);
    Trajectory = NULL;

    GeometryUpdateLevel=0;
    SeqIndex = 1;
    TotalCharge = 0;
}

//------------------------------------------------------------------------------

CStructure::CStructure(CProject* p_project)
    :   CProObject(&StructureObject,NULL,p_project)
{
    ConFlags |= EECOF_SUB_CONTAINERS;

    Atoms = new CAtomList(this);
    Bonds = new CBondList(this);
    Residues = new CResidueList(this);
    Restraints = new CRestraintList(this);
    Trajectory = NULL;

    GeometryUpdateLevel=0;
    SeqIndex = 1;
    TotalCharge = 0;
}

//------------------------------------------------------------------------------

CStructure::~CStructure(void)
{
    CStructureList* p_list = GetStructures();
    if( p_list ){
        if( p_list->GetActiveStructure() == this ){
            p_list->SetActiveStructure(NULL);
        }
    }
    // detach from trajectory
    if( Trajectory ){
        SetTrajectory(NULL);
    }

    setParent(NULL);    // remove object from the list

    if( p_list ){
        p_list->EmitChildContainerRemoved(this);
        p_list->ListSizeChanged();
    }

    // destroy subobjects
    BeginUpdate();
        // destruction time optimization for large structures
        Restraints->setParent(NULL);
        delete Restraints;
        Restraints = NULL;

        Bonds->setParent(NULL);
        delete Bonds;
        Bonds = NULL;

        Residues->setParent(NULL);
        delete Residues;
        Residues = NULL;

        Atoms->setParent(NULL);
        delete Atoms;
        Atoms = NULL;
    EndUpdate();
}

// ----------------------------------------------------------------------------

void CStructure::RemoveFromBaseList(CHistoryNode* p_history)
{
    CProObject* p_obj;

    // detach trajectory
    if( Trajectory ){
        SetTrajectory(NULL,p_history);
    }

    // unregister all sub-components
    Restraints->UnregisterAllRegisteredRestraints(p_history);
    Restraints->UnregisterAllRegisteredObjects(p_history);

    Bonds->UnregisterAllRegisteredBonds(p_history);
    Bonds->UnregisterAllRegisteredObjects(p_history);

    Residues->UnregisterAllRegisteredResidues(p_history);
    Residues->UnregisterAllRegisteredObjects(p_history);

    Atoms->UnregisterAllRegisteredAtoms(p_history);
    Atoms->UnregisterAllRegisteredObjects(p_history);

    // unregister other objects
    while( (p_obj = RemoveFirstFromROList()) !=NULL ) {
        p_obj->RemoveFromRegistered(this,p_history);
    }

    // deactivate structure if it was active
    CStructureList* p_list = GetStructures();
    if( p_list ){
        if( p_list->GetActiveStructure() == this ){
            p_list->SetActiveStructure(NULL,p_history);
        }
    }

    // register elementary change to history list
    if( p_history != NULL ) {
        CStructureHI* p_strdata = new CStructureHI(this,EHID_BACKWARD);
        p_history->Register(p_strdata);
    }

    // --------------------------------  
    delete this; // destroy object
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

CStructureList* CStructure::GetStructures(void) const
{
    return(static_cast<CStructureList*>(parent()));
}

//------------------------------------------------------------------------------

CAtomList* CStructure::GetAtoms(void)
{
    return(Atoms);
}

//------------------------------------------------------------------------------

CBondList* CStructure::GetBonds(void)
{
    return(Bonds);
}

//------------------------------------------------------------------------------

CResidueList*   CStructure::GetResidues(void)
{
    return(Residues);
}

//------------------------------------------------------------------------------

CRestraintList* CStructure::GetRestraints(void)
{
    return(Restraints);
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

bool CStructure::DeleteWH(void)
{
    // object will be deleted - use parent for all operation
    CStructureList* p_sl = GetStructures();

    CHistoryNode* p_history = p_sl->BeginChangeWH(EHCL_STRUCTURES,tr("delete structure"));
    if( p_history == NULL ) return (false);

    RemoveFromBaseList(p_history);

    p_sl->EndChangeWH();
    return(true);
}

//------------------------------------------------------------------------------

bool CStructure::InsertSMILESWH(const QString& smiles)
{
    CHistoryNode* p_history = BeginChangeWH(EHCL_TOPOLOGY,tr("insert SMILES structure"));
    if( p_history == NULL ) return (false);

    COpenBabelUtils::FromSMILES(smiles,this,p_history);

    EndChangeWH();
    return(true);
}

//------------------------------------------------------------------------------

bool CStructure::InsertInChIWH(const QString& inchi)
{
    CHistoryNode* p_history = BeginChangeWH(EHCL_TOPOLOGY,tr("insert InChI structure"));
    if( p_history == NULL ) return (false);

    COpenBabelUtils::FromInChI(inchi,this,p_history);

    EndChangeWH();
    return(true);
}

//------------------------------------------------------------------------------

bool CStructure::DeleteAllContentsWH(void)
{
    CHistoryNode* p_history = BeginChangeWH(EHCL_TOPOLOGY,tr("delete molecule"));
    if( p_history == NULL ) return (false);

    DeleteAllContents(p_history);

    EndChangeWH();
    return(true);
}

//------------------------------------------------------------------------------

bool CStructure::DeleteSelectedContentsWH(void)
{
    CHistoryNode* p_history = BeginChangeWH(EHCL_TOPOLOGY,tr("delete selected part of molecule"));
    if( p_history == NULL ) return (false);

    DeleteSelectedContents(p_history);

    EndChangeWH();
    return(true);
}

//------------------------------------------------------------------------------

bool CStructure::PasteWH(CXMLElement* p_ele,bool avoid_overlap)
{
    if( p_ele == NULL ) {
        INVALID_ARGUMENT("p_ele is NULL");
    }

    CHistoryNode* p_history = BeginChangeWH(EHCL_TOPOLOGY, tr("paste molecule"));
    if( p_history == NULL ) return (false);

    Paste(p_ele,false,avoid_overlap,p_history);

    EndChangeWH();
    return(true);
}

//------------------------------------------------------------------------------

bool CStructure::SetBoxWH(bool pa,bool pb,bool pc,const CPoint& sizes,const CPoint& angles)
{
    CHistoryNode* p_history = BeginChangeWH(EHCL_GEOMETRY,tr("set box"));
    if( p_history == NULL ) return (false);

    SetBox(pa,pb,pc,sizes,angles,p_history);

    EndChangeWH();
    return(true);
}

//------------------------------------------------------------------------------

bool CStructure::BuildSuperCellWH(int ka,int kb,int kc)
{
    if( (ka == 1) && (kb == 1) && (kc == 1) ) return(false);
    if( ka < 1 ) return(false);
    if( kb < 1 ) return(false);
    if( kc < 1 ) return(false);
    if( PBCInfo.IsPBCEnabled() == false ) return(false);

    CHistoryNode* p_history = BeginChangeWH(EHCL_GEOMETRY,tr("build super cell"));
    if( p_history == NULL ) return (false);

    // assign PBC bond indexes
    int pbcbonds = GetBonds()->AssignPBCBondIndexes();

    // save current structure
    CXMLDocument xml_doc;
    CXMLElement* p_str = xml_doc.CreateChildElement("structure");
    SaveData(p_str);

    BeginUpdate();

    // duplicate structure
    for(int i=0; i < ka; i++){
        for(int j=0; j < kb; j++){
            for(int k=0; k < kc; k++){
                if( (i == 0) && (j == 0) && (k == 0) ) continue;
                CPoint offset = PBCInfo.GetAVector()*i + PBCInfo.GetBVector()*j + PBCInfo.GetCVector()*k;
                Paste(p_str,true,offset,p_history);
            }
        }
    }

    double minlen = PBCInfo.GetLargestSphereRadius();
    // solve PBC bonds
    for(int pbcbtype = 1; pbcbtype <= pbcbonds; pbcbtype++ ){
        // find all bonds of given type
        CSimpleList<CBond> pbc_bonds;
        foreach(QObject* p_qobj,GetBonds()->children()){
            CBond* p_bond = static_cast<CBond*>(p_qobj);
            if( p_bond->GetPBCIndex() == pbcbtype ){
                pbc_bonds.InsertToEnd(p_bond,0,false);
            }
        }

        bool changed;
        do{
            changed = false;
            // interconnect those PBC bonds that are close to each other
            CSimpleIterator<CBond>  I1(pbc_bonds);
            while( I1.Current() != NULL ){
                CBond* p_b1 = I1.Current();
                CAtom* p_b1a1 = p_b1->GetFirstAtom();
                CAtom* p_b1a2 = p_b1->GetSecondAtom();
                I1++;
                CSimpleIterator<CBond>  I2 = I1;
                while(  I2.Current() != NULL ){
                    CBond* p_b2 = I2.Current();
                    CAtom* p_b2a1 = p_b2->GetFirstAtom();
                    CAtom* p_b2a2 = p_b2->GetSecondAtom();
                    bool found = false;
                    CBond* p_nbond = NULL;
                    if( (p_b1a1->IsBondedWith(p_b2a1) == NULL) &&
                        (Size(p_b1a1->GetPos()-p_b2a1->GetPos()) <= minlen) ){
                        GetBonds()->CreateBond(p_b1a1,p_b2a1,p_b1->GetBondOrder(),p_history);
                        p_nbond = GetBonds()->CreateBond(p_b1a2,p_b2a2,p_b1->GetBondOrder(),p_history);
                        found = true;
                    }
                    if( (p_b1a1->IsBondedWith(p_b2a2) == NULL) &&
                        (Size(p_b1a1->GetPos()-p_b2a2->GetPos()) <= minlen) ){
                        GetBonds()->CreateBond(p_b1a1,p_b2a2,p_b1->GetBondOrder(),p_history);
                        p_nbond = GetBonds()->CreateBond(p_b1a2,p_b2a1,p_b1->GetBondOrder(),p_history);
                        found = true;
                    }
                    if( (p_b1a2->IsBondedWith(p_b2a1) == NULL) &&
                        (Size(p_b1a2->GetPos()-p_b2a1->GetPos()) <= minlen) ){
                        GetBonds()->CreateBond(p_b1a2,p_b2a1,p_b1->GetBondOrder(),p_history);
                        p_nbond = GetBonds()->CreateBond(p_b1a1,p_b2a2,p_b1->GetBondOrder(),p_history);
                        found = true;
                    }
                    if( (p_b1a2->IsBondedWith(p_b2a2) == NULL) &&
                        (Size(p_b1a2->GetPos()-p_b2a2->GetPos()) <= minlen) ){
                        GetBonds()->CreateBond(p_b1a2,p_b2a2,p_b1->GetBondOrder(),p_history);
                        p_nbond = GetBonds()->CreateBond(p_b1a1,p_b2a1,p_b1->GetBondOrder(),p_history);
                        found = true;
                    }
                    I2++;
                    if( found ){
                        changed = true;
                        pbc_bonds.InsertToEnd(p_nbond,0,false);
                        pbc_bonds.Remove(p_b1);
                        pbc_bonds.Remove(p_b2);
                        p_b1->RemoveFromBaseList(p_history);
                        p_b2->RemoveFromBaseList(p_history);
                        break;
                    }
                }
                // we must quit the loop as object in I1 can be invalid
                // I1 points to I2 which is destroyed
                if( changed == true ) break;
            }
        } while( changed );
    }

    EndUpdate(false);

    // fix box sizes
    CPoint sizes = PBCInfo.GetSizes();
    sizes.x = sizes.x*ka;
    sizes.y = sizes.y*kb;
    sizes.z = sizes.z*kc;
    CPoint angles = PBCInfo.GetAngles();
    bool pa = PBCInfo.IsPeriodicAlongA();
    bool pb = PBCInfo.IsPeriodicAlongB();
    bool pc = PBCInfo.IsPeriodicAlongC();

    SetBox(pa,pb,pc,sizes,angles,p_history);

    EndChangeWH();
    return(true);
}

//------------------------------------------------------------------------------

bool CStructure::BuildSuperCellWH(int va,int vb,int vc,int n)
{
    if( (va == 0) && (vb == 0) && (vc == 0) ) return(false);
    if( n <= 1 ) return(false);
    if( PBCInfo.IsPBCEnabled() == false ) return(false);

    CHistoryNode* p_history = BeginChangeWH(EHCL_GEOMETRY,tr("build super cell"));
    if( p_history == NULL ) return (false);

    // assign PBC bond indexes
    int pbcbonds = GetBonds()->AssignPBCBondIndexes();

    // save current structure
    CXMLDocument xml_doc;
    CXMLElement* p_str = xml_doc.CreateChildElement("structure");
    SaveData(p_str);

    BeginUpdate();

    // duplicate structure
    for(int i=1; i < n; i++){
        CPoint offset = PBCInfo.GetAVector()*va*i + PBCInfo.GetBVector()*vb*i + PBCInfo.GetCVector()*vc*i;
        Paste(p_str,true,offset,p_history);
    }

    double minlen = PBCInfo.GetLargestSphereRadius();
    // solve PBC bonds
    for(int pbcbtype = 1; pbcbtype <= pbcbonds; pbcbtype++ ){
        // find all bonds of given type
        CSimpleList<CBond> pbc_bonds;
        foreach(QObject* p_qobj,GetBonds()->children()){
            CBond* p_bond = static_cast<CBond*>(p_qobj);
            if( p_bond->GetPBCIndex() == pbcbtype ){
                pbc_bonds.InsertToEnd(p_bond,0,false);
            }
        }

        bool changed;
        do{
            changed = false;
            // interconnect those PBC bonds that are close to each other
            CSimpleIterator<CBond>  I1(pbc_bonds);
            while( I1.Current() != NULL ){
                CBond* p_b1 = I1.Current();
                CAtom* p_b1a1 = p_b1->GetFirstAtom();
                CAtom* p_b1a2 = p_b1->GetSecondAtom();
                I1++;
                CSimpleIterator<CBond>  I2 = I1;
                while(  I2.Current() != NULL ){
                    CBond* p_b2 = I2.Current();
                    CAtom* p_b2a1 = p_b2->GetFirstAtom();
                    CAtom* p_b2a2 = p_b2->GetSecondAtom();
                    bool found = false;
                    CBond* p_nbond = NULL;
                    if( (p_b1a1->IsBondedWith(p_b2a1) == NULL) &&
                        (Size(p_b1a1->GetPos()-p_b2a1->GetPos()) <= minlen) ){
                        GetBonds()->CreateBond(p_b1a1,p_b2a1,p_b1->GetBondOrder(),p_history);
                        p_nbond = GetBonds()->CreateBond(p_b1a2,p_b2a2,p_b1->GetBondOrder(),p_history);
                        found = true;
                    }
                    if( (p_b1a1->IsBondedWith(p_b2a2) == NULL) &&
                        (Size(p_b1a1->GetPos()-p_b2a2->GetPos()) <= minlen) ){
                        GetBonds()->CreateBond(p_b1a1,p_b2a2,p_b1->GetBondOrder(),p_history);
                        p_nbond = GetBonds()->CreateBond(p_b1a2,p_b2a1,p_b1->GetBondOrder(),p_history);
                        found = true;
                    }
                    if( (p_b1a2->IsBondedWith(p_b2a1) == NULL) &&
                        (Size(p_b1a2->GetPos()-p_b2a1->GetPos()) <= minlen) ){
                        GetBonds()->CreateBond(p_b1a2,p_b2a1,p_b1->GetBondOrder(),p_history);
                        p_nbond = GetBonds()->CreateBond(p_b1a1,p_b2a2,p_b1->GetBondOrder(),p_history);
                        found = true;
                    }
                    if( (p_b1a2->IsBondedWith(p_b2a2) == NULL) &&
                        (Size(p_b1a2->GetPos()-p_b2a2->GetPos()) <= minlen) ){
                        GetBonds()->CreateBond(p_b1a2,p_b2a2,p_b1->GetBondOrder(),p_history);
                        p_nbond = GetBonds()->CreateBond(p_b1a1,p_b2a1,p_b1->GetBondOrder(),p_history);
                        found = true;
                    }
                    I2++;
                    if( found ){
                        changed = true;
                        pbc_bonds.InsertToEnd(p_nbond,0,false);
                        pbc_bonds.Remove(p_b1);
                        pbc_bonds.Remove(p_b2);
                        p_b1->RemoveFromBaseList(p_history);
                        p_b2->RemoveFromBaseList(p_history);
                        break;
                    }
                }
                // we must quit the loop as object in I1 can be invalid
                // I1 points to I2 which is destroyed
                if( changed == true ) break;
            }
        } while( changed );
    }

    EndUpdate(false);

//    // fix box sizes
//    CPoint sizes = PBCInfo.GetSizes();
//    sizes.x = sizes.x + sizes.x*abs(va)*(n-1);
//    sizes.y = sizes.y + sizes.y*abs(vb)*(n-1);
//    sizes.z = sizes.z + sizes.z*abs(vc)*(n-1);
//    CPoint angles = PBCInfo.GetAngles();
//    bool pa = PBCInfo.IsPeriodicAlongA();
//    bool pb = PBCInfo.IsPeriodicAlongB();
//    bool pc = PBCInfo.IsPeriodicAlongC();

//    SetBox(pa,pb,pc,sizes,angles,p_history);

    EndChangeWH();
    return(true);
}

//------------------------------------------------------------------------------

bool CStructure::SetSeqIndexWH(int seqidx)
{
    if( GetSeqIndex() == seqidx ) return(true);

    CHistoryNode* p_history = BeginChangeWH(EHCL_TOPOLOGY,"structure sequence index");
    if( p_history == NULL ) return (false);

    SetSeqIndex(seqidx,p_history);

    EndChangeWH();
    return(true);
}

//------------------------------------------------------------------------------

bool CStructure::SetTotalChargeWH(int charge)
{
    if( GetTotalCharge() == charge ) return(true);

    CHistoryNode* p_history = BeginChangeWH(EHCL_TOPOLOGY,"structure total charge");
    if( p_history == NULL ) return (false);

    SetTotalCharge(charge,p_history);

    EndChangeWH();
    return(true);
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

void CStructure::GetObjectMetrics(CObjMetrics& metrics)
{
    foreach(QObject* p_qobj,Atoms->children()) {
        CAtom* p_atom = static_cast<CAtom*>(p_qobj);
        metrics.CompareWith(p_atom->GetPos());
    }
}

//------------------------------------------------------------------------------

bool CStructure::IsEmpty(void) const
{
    return(Atoms->children().count() == 0);
}

//------------------------------------------------------------------------------

const QString CStructure::GetSMILES(void)
{
    QString smiles;
    COpenBabelUtils::GetSMILES(this,smiles);
    return(smiles);
}

//------------------------------------------------------------------------------

int CStructure::GetSeqIndex(void) const
{
    return(SeqIndex);
}

//------------------------------------------------------------------------------

int CStructure::GetTotalCharge(void) const
{
    return(TotalCharge);
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

void CStructure::SetBox(bool pa,bool pb,bool pc,const CPoint& sizes,const CPoint& angles,
                       CHistoryNode* p_history)
{
    if( p_history ) {
        CStructureBoxHI* p_hnode = new CStructureBoxHI(this,pa,pb,pc,sizes,angles);
        p_history->Register(p_hnode);
    }
    PBCInfo.SetDimmensions(sizes,angles);
    PBCInfo.SetPeriodicity(pa,pb,pc);
    // see void CMainWindow::UpdateGeometryMenu(void)
    if(  GetStructures() ) GetStructures()->EmitOnStructureListChanged();
}

//------------------------------------------------------------------------------

void CStructure::DeleteAllContents(CHistoryNode* p_history)
{
    // lock lists
    BeginUpdate();

    // remove all atoms
    foreach(QObject* p_qobj,Atoms->children()) {
        CAtom* p_atom = static_cast<CAtom*>(p_qobj);
        p_atom->RemoveFromBaseList(p_history);
    }

    // remove remaining bonds - should be automatically deleted with atoms

    // remove all residues
    foreach(QObject* p_qobj,Residues->children()) {
        CResidue* p_res = static_cast<CResidue*>(p_qobj);
        p_res->RemoveFromBaseList(p_history);
    }

    // unlock lists
    EndUpdate();
}

//------------------------------------------------------------------------------

void CStructure::DeleteSelectedContents(CHistoryNode* p_history)
{
    // lock lists
    BeginUpdate();

    // remove selected atoms
    foreach(QObject* p_qobj,Atoms->children()) {
        CAtom* p_atom = static_cast<CAtom*>(p_qobj);
        if( p_atom->IsFlagSet(EPOF_SELECTED) ) {
            p_atom->RemoveFromBaseList(p_history);
        }
    }

    // remove selected bonds
    foreach(QObject* p_qobj,Bonds->children()) {
        CBond* p_bond = static_cast<CBond*>(p_qobj);
        if( p_bond->IsFlagSet(EPOF_SELECTED) ) {
            p_bond->RemoveFromBaseList(p_history);
        }
    }

    // unlock lists
    EndUpdate();
}

//------------------------------------------------------------------------------

void CStructure::SetSeqIndex(int seqidx,CHistoryNode* p_history)
{
    if( SeqIndex == seqidx ) return;

    GetStructures()->BeginUpdate();
    if( p_history ){
        CStructureSeqIndexHI* p_item = new CStructureSeqIndexHI(this,seqidx);
        p_history->Register(p_item);
    }

    SeqIndex = seqidx;
    emit OnStatusChanged(ESC_OTHER);
    GetStructures()->ForceSorting = true;
    GetStructures()->EmitOnStructureListChanged();
    GetStructures()->EndUpdate();
}

//------------------------------------------------------------------------------

void CStructure::SetTotalCharge(int charge,CHistoryNode* p_history)
{
    if( TotalCharge == charge ) return;

    GetStructures()->BeginUpdate();
    if( p_history ){
        CStructureTotalChargeHI* p_item = new CStructureTotalChargeHI(this,charge);
        p_history->Register(p_item);
    }

    TotalCharge = charge;
    emit OnStatusChanged(ESC_OTHER);
}

//------------------------------------------------------------------------------

void CStructure::InsertSMILES(const QString& smiles)
{
    COpenBabelUtils::FromSMILES(smiles,this);
}

//------------------------------------------------------------------------------

void CStructure::InsertInChI(const QString& inchi)
{
    COpenBabelUtils::FromInChI(inchi,this);
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

void CStructure::Copy(CXMLElement* p_ele,bool selected_only)
{
    if( p_ele->GetName() != "structure" ) {
        LOGIC_ERROR("p_ele is not 'structure'");
    }

    if( selected_only == false ) {
        SaveStructureData(p_ele,false);
    } else {
        // clear the EPOF_MANIP_FLAG
        foreach(QObject* p_qobj,GetAtoms()->children()) {
            CAtom* p_atom = static_cast<CAtom*>(p_qobj);
            bool set = p_atom->IsFlagSet(EPOF_SELECTED);
            set |= p_atom->GetResidue()->IsFlagSet(EPOF_SELECTED);
            set |= IsFlagSet(EPOF_SELECTED);
            p_atom->SetFlag(EPOF_MANIP_FLAG,set);
        }
        foreach(QObject* p_qobj,GetBonds()->children()) {
            CBond* p_bond = static_cast<CBond*>(p_qobj);
            bool set = p_bond->IsFlagSet(EPOF_SELECTED);
            set |= IsFlagSet(EPOF_SELECTED);
            if( (p_bond->GetFirstAtom() != NULL) && (p_bond->GetSecondAtom() != NULL) ){
                set |= (p_bond->GetFirstAtom()->IsFlagSet(EPOF_SELECTED) && p_bond->GetSecondAtom()->IsFlagSet(EPOF_SELECTED));
                set |= (p_bond->GetFirstAtom()->IsFlagSet(EPOF_MANIP_FLAG) && p_bond->GetSecondAtom()->IsFlagSet(EPOF_MANIP_FLAG));
            }
            p_bond->SetFlag(EPOF_MANIP_FLAG,set);
        }
        foreach(QObject* p_qobj,GetResidues()->children()) {
            CResidue* p_res = static_cast<CResidue*>(p_qobj);
            bool set = p_res->IsFlagSet(EPOF_SELECTED);
            set |= IsFlagSet(EPOF_SELECTED);
            set |= p_res->IsAnyAtomSelected(true);
            p_res->SetFlag(EPOF_MANIP_FLAG,set);
        }

        // select temporarily those atoms that are part of selected bonds
        foreach(QObject* p_qobj,GetBonds()->children()) {
            CBond* p_bond = static_cast<CBond*>(p_qobj);
            if( p_bond->IsFlagSet(EPOF_MANIP_FLAG) == true ) {
                bool selected;
                if( p_bond->GetFirstAtom() ){
                    selected = p_bond->GetFirstAtom()->IsFlagSet(EPOF_SELECTED);
                    p_bond->GetFirstAtom()->SetFlag(EPOF_MANIP_FLAG,selected);
                }
                if( p_bond->GetSecondAtom() ){
                    selected = p_bond->GetSecondAtom()->IsFlagSet(EPOF_SELECTED);
                    p_bond->GetSecondAtom()->SetFlag(EPOF_MANIP_FLAG,selected);
                }
            }
        }

        // copy selected part of molecule
        SaveStructureData(p_ele,true);
    }
}

//------------------------------------------------------------------------------

void CStructure::Paste(CXMLElement* p_ele,bool donotoverride,bool avoid_overlap,CHistoryNode* p_history)
{
    // check input data -----------------------------
    if( p_ele == NULL ) {
        INVALID_ARGUMENT("p_ele is NULL");
    }

    if( p_ele->GetName() != "structure" ) {
        LOGIC_ERROR("p_ele is not 'structure'");
    }

    if( donotoverride ){
        p_ele = static_cast<CXMLElement*>(p_ele->DuplicateNode(NULL));
    }

    CXMLElement* p_aele = p_ele->GetFirstChildElement("atoms");
    CXMLElement* p_rele = p_ele->GetFirstChildElement("residues");

    // reindex structure - get sequence indexes
    int topseqidx = GetResidues()->GetTopSeqIndex();
    int lowseqidx = 0;
    if( p_rele ){
        CXMLElement* p_res = p_rele->GetFirstChildElement();
        if( p_res ){
            p_res->GetAttribute("seqid",lowseqidx);
        }
        while(p_res != NULL){
            int seqid;
            p_res->GetAttribute("seqid",seqid);
            if( seqid < lowseqidx ){
                lowseqidx = seqid;
            }
            p_res = p_res->GetNextSiblingElement();
        }

        p_res = p_rele->GetFirstChildElement();
        while(p_res != NULL){
            int seqid = 0;
            p_res->GetAttribute("seqid",seqid);
            p_res->SetAttribute("seqid",seqid+topseqidx-lowseqidx+1);
            p_res = p_res->GetNextSiblingElement();
        }
    }

    // reindex structure - get serial indexes and eventually offset
    int topseridx = GetAtoms()->GetTopSerIndex();

    if( avoid_overlap == true ) {
        if( p_aele ){
            // extract some parameters from the stream
            CObjMetrics nbox;
            int lowseridx = 0;

            // new bounding box
            CXMLElement* p_at = p_aele->GetFirstChildElement();
            if( p_at ){
                p_at->GetAttribute("ai",lowseridx);
            }
            while(p_at != NULL){
                CPoint pos;
                p_at->GetAttribute("px",pos.x);
                p_at->GetAttribute("py",pos.y);
                p_at->GetAttribute("pz",pos.z);
                nbox.CompareWith(pos);
                int serid;
                p_at->GetAttribute("ai",serid);
                if( serid < lowseridx ){
                    lowseridx = serid;
                }
                p_at = p_at->GetNextSiblingElement();
            }

            // current bounding box
            CObjMetrics cbox;
            GetObjectMetrics(cbox);

            // shift data
            CPoint  pos_offset;
            pos_offset = cbox.GetHighPoint() - nbox.GetLowPoint();

            // update position
            p_at = p_aele->GetFirstChildElement();
            while(p_at != NULL){
                CPoint pos;
                p_at->GetAttribute("px",pos.x);
                p_at->GetAttribute("py",pos.y);
                p_at->GetAttribute("pz",pos.z);
                pos += pos_offset;
                p_at->SetAttribute("px",pos.x);
                p_at->SetAttribute("py",pos.y);
                p_at->SetAttribute("pz",pos.z);

                int serid=0;
                p_at->GetAttribute("ai",serid);
                p_at->SetAttribute("ai",serid+topseridx-lowseridx+1);

                p_at = p_at->GetNextSiblingElement();
            }
        }
    } else {
        int lowseridx = 0;

        if( p_aele ){
            CXMLElement* p_at = p_aele->GetFirstChildElement();
            if( p_at ){
                p_at->GetAttribute("ai",lowseridx);
            }
            while(p_at != NULL){
                int serid;
                p_at->GetAttribute("ai",serid);
                if( serid < lowseridx ){
                    lowseridx = serid;
                }
                p_at = p_at->GetNextSiblingElement();
            }
            p_at = p_aele->GetFirstChildElement();
            while(p_at != NULL){
                int serid=0;
                p_at->GetAttribute("ai",serid);
                p_at->SetAttribute("ai",serid+topseridx-lowseridx+1);

                p_at = p_at->GetNextSiblingElement();
            }
        }
    }

    // we do not use link objects but we need to set correct ID offset
    GetProject()->BeginLinkProcedure();
        LoadStructureData(p_ele,p_history);
    GetProject()->EndLinkProcedure();

    if( donotoverride ){
        delete p_ele;
    }
}

//------------------------------------------------------------------------------

void CStructure::Paste(CXMLElement* p_ele,bool donotoverride,const CPoint& offset,CHistoryNode* p_history)
{
    // check input data -----------------------------
    if( p_ele == NULL ) {
        INVALID_ARGUMENT("p_ele is NULL");
    }

    if( p_ele->GetName() != "structure" ) {
        LOGIC_ERROR("p_ele is not 'structure'");
    }

    if( donotoverride ){
        p_ele = static_cast<CXMLElement*>(p_ele->DuplicateNode(NULL));
    }

    CXMLElement* p_aele = p_ele->GetFirstChildElement("atoms");
    CXMLElement* p_rele = p_ele->GetFirstChildElement("residues");

    // reindex structure - get sequence indexes
    int topseqidx = GetResidues()->GetTopSeqIndex();
    int lowseqidx = 0;
    if( p_rele ){
        CXMLElement* p_res = p_rele->GetFirstChildElement();
        if( p_res ){
            p_res->GetAttribute("seqid",lowseqidx);
        }
        while(p_res != NULL){
            int seqid;
            p_res->GetAttribute("seqid",seqid);
            if( seqid < lowseqidx ){
                lowseqidx = seqid;
            }
            p_res = p_res->GetNextSiblingElement();
        }

        p_res = p_rele->GetFirstChildElement();
        while(p_res != NULL){
            int seqid = 0;
            p_res->GetAttribute("seqid",seqid);
            p_res->SetAttribute("seqid",seqid+topseqidx-lowseqidx+1);
            p_res = p_res->GetNextSiblingElement();
        }
    }

    // reindex structure - get serial indexes and eventually offset
    int topseridx = GetAtoms()->GetTopSerIndex();
    int lowseridx = 0;

    if( p_aele ){
        // low ser index
        CXMLElement* p_at = p_aele->GetFirstChildElement();
        if( p_at ){
            p_at->GetAttribute("ai",lowseridx);
        }
        while(p_at != NULL){
            int serid;
            p_at->GetAttribute("ai",serid);
            if( serid < lowseridx ){
                lowseridx = serid;
            }
            p_at = p_at->GetNextSiblingElement();
        }

        // update position
        p_at = p_aele->GetFirstChildElement();
        while(p_at != NULL){
            CPoint pos;
            p_at->GetAttribute("px",pos.x);
            p_at->GetAttribute("py",pos.y);
            p_at->GetAttribute("pz",pos.z);
            pos += offset;
            p_at->SetAttribute("px",pos.x);
            p_at->SetAttribute("py",pos.y);
            p_at->SetAttribute("pz",pos.z);

            int serid=0;
            p_at->GetAttribute("ai",serid);
            p_at->SetAttribute("ai",serid+topseridx-lowseridx+1);

            p_at = p_at->GetNextSiblingElement();
        }
    }

    // we do not use link objects but we need to set correct ID offset
    GetProject()->BeginLinkProcedure();
        LoadStructureData(p_ele,p_history);
    GetProject()->EndLinkProcedure();

    if( donotoverride ){
        delete p_ele;
    }
}

//------------------------------------------------------------------------------

void CStructure::SetName(const QString& name,CHistoryNode* p_history)
{
    CProObject::SetName(name,p_history);
    if( GetStructures() ) GetStructures()->EmitOnStructureListChanged();
}

//------------------------------------------------------------------------------

void CStructure::SetDescription(const QString& descrip,CHistoryNode* p_history)
{
    CProObject::SetDescription(descrip,p_history);
    if( GetStructures() ) GetStructures()->EmitOnStructureListChanged();
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

void CStructure::LoadData(CXMLElement* p_ele)
{
    // check input data -----------------------------
    if( p_ele == NULL ) {
        INVALID_ARGUMENT("p_ele is NULL");
    }

    if( p_ele->GetName() != "structure" ) {
        LOGIC_ERROR("p_ele is not structure");
    }

    BeginUpdate();

    // load object info -----------------------------
    CProObject::LoadData(p_ele);

    // total charge
    p_ele->GetAttribute("totchrg",TotalCharge);

    // get seq index
    p_ele->GetAttribute("seqidx",SeqIndex);

    CXMLElement* p_sele;
    // load data ------------------------------------
    p_sele = p_ele->GetFirstChildElement("residues");
    if( p_sele != NULL ) {
        Residues->LoadData(p_sele);
    }
    p_sele = p_ele->GetFirstChildElement("atoms");
    if( p_sele != NULL ) {
        Atoms->LoadData(p_sele);
    }
    p_sele = p_ele->GetFirstChildElement("bonds");
    if( p_sele != NULL ) {
        Bonds->LoadData(p_sele);
    }
    p_sele = p_ele->GetFirstChildElement("restraints");
    if( p_sele != NULL ) {
        Restraints->LoadData(p_sele);
    }
    p_sele = p_ele->GetFirstChildElement("pbc");
    if( p_sele != NULL ) {
        PBCInfo.LoadData(p_sele);
    }

    EndUpdate();
}

//------------------------------------------------------------------------------

void CStructure::SaveData(CXMLElement* p_ele)
{
    // check input data -----------------------------
    if( p_ele == NULL ) {
        INVALID_ARGUMENT("p_ele is NULL");
    }

    if( p_ele->GetName() != "structure" ) {
        LOGIC_ERROR("p_ele is not structure");
    }

    // save object info -----------------------------
    CProObject::SaveData(p_ele);

    // total charge
    p_ele->GetAttribute("totchrg",TotalCharge);

    // total charge
    p_ele->SetAttribute("totchrg",TotalCharge);

    // get seq index
    p_ele->SetAttribute("seqidx",SeqIndex);

    CXMLElement* p_sele;

    // save data ------------------------------------     
    p_sele = p_ele->CreateChildElement("residues");
    Residues->SaveData(p_sele,false) ;

    p_sele = p_ele->CreateChildElement("atoms");
    Atoms->SaveData(p_sele,false);

    p_sele = p_ele->CreateChildElement("bonds");
    Bonds->SaveData(p_sele,false);

    p_sele = p_ele->CreateChildElement("restraints");
    Restraints->SaveData(p_sele);

    p_sele = p_ele->CreateChildElement("pbc");
    PBCInfo.SaveData(p_sele);
}

//------------------------------------------------------------------------------

void CStructure::LoadStructureData(CXMLElement* p_ele,CHistoryNode* p_history)
{
    // check input data -----------------------------
    if( p_ele == NULL ) {
        INVALID_ARGUMENT("p_ele is NULL");
    }

    if( p_ele->GetName() != "structure" ) {
        LOGIC_ERROR("p_ele is not structure");
    }

    BeginUpdate();

    // load object info -----------------------------
    // DO NOT CALL THIS - structure element is not usually complete
    // for create purposes
    // CProObject::LoadData(p_ele);

    // get seq index
    p_ele->GetAttribute("seqidx",SeqIndex);

    CXMLElement* p_sele;
    // load data ------------------------------------
    p_sele = p_ele->GetFirstChildElement("residues");
    if( p_sele != NULL ) {
        Residues->LoadData(p_sele,p_history);
    }
    p_sele = p_ele->GetFirstChildElement("atoms");
    if( p_sele != NULL ) {
        Atoms->LoadData(p_sele,p_history);
    }
    p_sele = p_ele->GetFirstChildElement("bonds");
    if( p_sele != NULL ) {
        Bonds->LoadData(p_sele,p_history);
    }

    EndUpdate();
}

//------------------------------------------------------------------------------

void CStructure::SaveStructureData(CXMLElement* p_ele,bool selected)
{
    // check input data -----------------------------
    if( p_ele == NULL ) {
        INVALID_ARGUMENT("p_ele is NULL");
    }

    if( p_ele->GetName() != "structure" ) {
        LOGIC_ERROR("p_ele is not structure");
    }

    // save object info -----------------------------
    CProObject::SaveData(p_ele);

    // total charge
    p_ele->SetAttribute("totchrg",TotalCharge);

    // save sequential index
    p_ele->SetAttribute("seqidx",SeqIndex);

    CXMLElement* p_sele;

    // save data ------------------------------------
    // DO NOT CHANGE ORDER of sections
    p_sele = p_ele->CreateChildElement("residues");
    Residues->SaveData(p_sele,selected) ;

    p_sele = p_ele->CreateChildElement("atoms");
    Atoms->SaveData(p_sele,selected);

    p_sele = p_ele->CreateChildElement("bonds");
    Bonds->SaveData(p_sele,selected);
}

//------------------------------------------------------------------------------

void CStructure::SaveSelectedResidues(CXMLElement* p_ele)
{
    // check input data -----------------------------
    if( p_ele == NULL ) {
        INVALID_ARGUMENT("p_ele is NULL");
    }

    if( p_ele->GetName() != "structure" ) {
        LOGIC_ERROR("p_ele is not structure");
    }

    // residues
    foreach(QObject* p_qobj,GetResidues()->children()) {
        CResidue* p_res = static_cast<CResidue*>(p_qobj);
        bool set = p_res->IsFlagSet(EPOF_SELECTED);
        p_res->SetFlag(EPOF_MANIP_FLAG,set);
    }

    // atoms
    foreach(QObject* p_qobj,GetAtoms()->children()) {
        bool set = false;
        CAtom* p_atm = static_cast<CAtom*>(p_qobj);
        if( p_atm->GetResidue() ){
            set = p_atm->GetResidue()->IsFlagSet(EPOF_SELECTED);
        }
        p_atm->SetFlag(EPOF_MANIP_FLAG,set);
    }

    // bonds
    foreach(QObject* p_qobj,GetBonds()->children()) {
        CBond* p_bond = static_cast<CBond*>(p_qobj);
        bool set = false;
        if( (p_bond->GetFirstAtom() != NULL) && (p_bond->GetSecondAtom() != NULL) ){
            set |= (p_bond->GetFirstAtom()->IsFlagSet(EPOF_MANIP_FLAG) && p_bond->GetSecondAtom()->IsFlagSet(EPOF_MANIP_FLAG));
        }
        p_bond->SetFlag(EPOF_MANIP_FLAG,set);
    }

    CXMLElement* p_sele;

    // save data ------------------------------------
    // DO NOT CHANGE ORDER of sections
    p_sele = p_ele->GetFirstChildElement("residues");
    if( p_sele == NULL ) {
        INVALID_ARGUMENT("p_sele is not residues");
    }
    Residues->SaveData(p_sele,true) ;

    p_sele = p_ele->GetFirstChildElement("atoms");
    if( p_sele == NULL ) {
        INVALID_ARGUMENT("p_sele is not atoms");
    }
    Atoms->SaveData(p_sele,true);

    p_sele = p_ele->GetFirstChildElement("bonds");
    if( p_sele == NULL ) {
        INVALID_ARGUMENT("p_sele is not bonds");
    }
    Bonds->SaveData(p_sele,true);
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

void CStructure::BeginUpdate(CHistoryNode* p_history)
{
    if( p_history ){
        CStructureUpdateHI* p_node = new CStructureUpdateHI(this,EHID_FORWARD);
        p_history->Register(p_node);
    }

    if( GetAtoms() ) GetAtoms()->BeginUpdate();
    if( GetBonds() ) GetBonds()->BeginUpdate();
    if( GetResidues() ) GetResidues()->BeginUpdate();
    if( GetRestraints() ) GetRestraints()->BeginUpdate();
}

//------------------------------------------------------------------------------

void CStructure::EndUpdate(bool do_not_sort,CHistoryNode* p_history)
{
    if( p_history ){
        CStructureUpdateHI* p_node = new CStructureUpdateHI(this,EHID_BACKWARD);
        p_history->Register(p_node);
    }

    if( GetAtoms() ) GetAtoms()->EndUpdate(do_not_sort);
    if( GetBonds() ) GetBonds()->EndUpdate();
    if( GetResidues() ) GetResidues()->EndUpdate(do_not_sort);
    if( GetRestraints() ) GetRestraints()->EndUpdate();
}

//------------------------------------------------------------------------------

void CStructure::BeginGeometryUpdate(void)
{
    GeometryUpdateLevel++;
}

//------------------------------------------------------------------------------

void CStructure::EndGeometryUpdate(void)
{
    if( GeometryUpdateLevel == 0 ) return;
    GeometryUpdateLevel--;
    if( GeometryUpdateLevel == 0 ){
        NotifyGeometryChangeTick();
    }
}

//------------------------------------------------------------------------------

void CStructure::NotifyGeometryChangeTick(void)
{
    if( GetStructures() ){
        GetStructures()->NotifyGeometryChangeTick();
    }
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

CTrajectory* CStructure::GetTrajectory(void)
{
    return(Trajectory);
}

//------------------------------------------------------------------------------

bool CStructure::SetTrajectoryWH(CTrajectory* p_trajectory)
{
    if( Trajectory == p_trajectory ) return(true);

    CHistoryNode* p_history = BeginChangeWH(EHCL_TRAJECTORIES,tr("associate trajectory with structure"));
    if( p_history == NULL ) return (false);

    SetTrajectory(p_trajectory,p_history);

    EndChangeWH();
    return(true);
}

//------------------------------------------------------------------------------

void CStructure::SetTrajectory(CTrajectory* p_trajectory,CHistoryNode* p_history)
{
    if( Trajectory == p_trajectory ) return;

    if( Trajectory ){
        Trajectory->SetStructure(NULL,p_history);
    }
    Trajectory = p_trajectory;
    if( Trajectory ){
        GetAtoms()->SetSnapshot(Trajectory->GetCurrentSnapshot());
        Trajectory->SetStructure(this,p_history);       
    } else {
        GetAtoms()->SetSnapshot(NULL);
    }
    emit OnStatusChanged(ESC_OTHER);
}

//------------------------------------------------------------------------------

void CStructure::SetTrajIndexes(void)
{
    TrajIndexMap.clear();

    int i=0;
    foreach(CAtom* p_atom,Atoms->findChildren<CAtom*>()){
        p_atom->SetTrajIndex(i);
        TrajIndexMap[i] = p_atom;
        i++;
    }
}

//------------------------------------------------------------------------------

CAtom* CStructure::GetAtomByTrajIndex(int idx)
{
    if( (idx < 0) || (idx >= TrajIndexMap.count()) ) return(NULL);
    return(TrajIndexMap[idx]);
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================



