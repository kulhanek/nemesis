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

#include <AtomListHistory.hpp>
#include <CategoryUUID.hpp>
#include <Project.hpp>
#include <Structure.hpp>
#include <XMLElement.hpp>
#include <AtomList.hpp>
#include <ProObjectHistory.hpp>
#include <Structure.hpp>
#include <Atom.hpp>
#include <NemesisCoreModule.hpp>

//------------------------------------------------------------------------------

REGISTER_HISTORY_OBJECT(NemesisCorePlugin,AtomListTransHI,
                        "{ATOM_L_TRA:cd68649d-bad5-4821-a061-e9a891a4dfd3}")
REGISTER_HISTORY_OBJECT(NemesisCorePlugin,AtomListCoordinatesHI,
                        "{ATOM_L_CRD:d41e853d-e363-4183-a0c3-34e68f15badc}")
REGISTER_HISTORY_OBJECT(NemesisCorePlugin,AtomListChangeParentHI,
                        "{ATOM_L_CHP:b0313f21-b004-430d-8161-3a660006f0df}")

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

CAtomListTransHI::CAtomListTransHI(CStructure* p_mol,const CTransformation& trans)
    : CHistoryItem(&AtomListTransHIObject,p_mol->GetProject(),EHID_FORWARD)
{
    MoleculeIndex = p_mol->GetIndex();

    Indexes.CreateVector(p_mol->GetAtoms()->children().count());

    int i=0;
    foreach(QObject* p_qobj,p_mol->GetAtoms()->children()) {
        CAtom* p_atom = static_cast<CAtom*>(p_qobj);
        if( p_atom->IsFlagSet(EPOF_MANIP_FLAG) ){
            Indexes[i] = p_atom->GetIndex();
        } else {
            Indexes[i] = -1;
        }
        i++;
    }

    NewTrans = trans;
    OldTrans = trans;
    OldTrans.Invert();
}

//------------------------------------------------------------------------------

void CAtomListTransHI::Forward(void)
{
    CStructure* p_mol = dynamic_cast<CStructure*>(GetProject()->FindObject(MoleculeIndex));
    if(p_mol == NULL) return;

    for(size_t i=0; i < Indexes.GetLength(); i++) {
        if( Indexes[i] < 0 ) continue;
        CAtom* p_atom = static_cast<CAtom*>(p_mol->GetProject()->FindObject(Indexes[i]));
        if( p_atom == NULL ) continue;
        p_atom->SetPos(NewTrans.GetTransform(p_atom->GetPos()));
    }
}

//------------------------------------------------------------------------------

void CAtomListTransHI::Backward(void)
{
    CStructure* p_mol = dynamic_cast<CStructure*>(GetProject()->FindObject(MoleculeIndex));
    if(p_mol == NULL) return;

    for(size_t i=0; i < Indexes.GetLength(); i++) {
        if( Indexes[i] < 0 ) continue;
        CAtom* p_atom = static_cast<CAtom*>(p_mol->GetProject()->FindObject(Indexes[i]));
        if( p_atom == NULL ) continue;
        p_atom->SetPos(OldTrans.GetTransform(p_atom->GetPos()));
    }
}

//------------------------------------------------------------------------------

void CAtomListTransHI::LoadData(CXMLElement* p_ele)
{
    if( p_ele == NULL ){
        INVALID_ARGUMENT("p_ele is NULL");
    }

    // load core data ----------------------------
    CHistoryItem::LoadData(p_ele);

    // load local data ---------------------------
    p_ele->GetAttribute("mi",MoleculeIndex);

    CXMLElement* p_sele;
    p_sele = p_ele->GetFirstChildElement("nt");
    if( p_sele ) NewTrans.Load(p_sele);
    p_sele = p_ele->GetFirstChildElement("ot");
    if( p_sele ) OldTrans.Load(p_sele);

    CXMLBinData* p_bele;
    p_bele = p_ele->GetFirstChildBinData("ai");
    if( p_bele ){
        Indexes.Load(p_bele);
    }
}

//------------------------------------------------------------------------------

void CAtomListTransHI::SaveData(CXMLElement* p_ele)
{
    if( p_ele == NULL ){
        INVALID_ARGUMENT("p_ele is NULL");
    }

    // save core data ----------------------------
    CHistoryItem::SaveData(p_ele);

    // save local data ---------------------------
    p_ele->SetAttribute("mi",MoleculeIndex);

    CXMLElement* p_sele;
    p_sele = p_ele->CreateChildElement("nt");
    NewTrans.Save(p_sele);
    p_sele = p_ele->CreateChildElement("ot");
    OldTrans.Save(p_sele);

    CXMLBinData* p_bele;
    p_bele = p_ele->CreateChildBinData("ai");
    Indexes.Save(p_bele);
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

CAtomListCoordinatesHI::CAtomListCoordinatesHI(CStructure* p_mol)
    : CHistoryItem(&AtomListCoordinatesHIObject,p_mol->GetProject(),EHID_FORWARD)
{
    MoleculeIndex = p_mol->GetIndex();

    int num_of_atoms = p_mol->GetAtoms()->children().count();

    Coordinates.CreateVector(num_of_atoms);
    Indexes.CreateVector(num_of_atoms);

    int i=0;
    foreach(QObject* p_qobj,p_mol->GetAtoms()->children()) {
        CAtom* p_atom = static_cast<CAtom*>(p_qobj);
        Coordinates[i] = p_atom->GetPos();
        Indexes[i] = p_atom->GetIndex();
        i++;
    }
}

//------------------------------------------------------------------------------

CAtomListCoordinatesHI::CAtomListCoordinatesHI(CStructure* p_mol,bool selected)
    : CHistoryItem(&AtomListCoordinatesHIObject,p_mol->GetProject(),EHID_FORWARD)
{
    MoleculeIndex = p_mol->GetIndex();

    int num_of_atoms = 0;

    if( selected ){
        foreach(QObject* p_qobj,p_mol->GetAtoms()->children()) {
            CAtom* p_atom = static_cast<CAtom*>(p_qobj);
            if( p_atom->IsFlagSet(EPOF_SELECTED) ) {
                num_of_atoms++;
            }
        }
        if( num_of_atoms == 0 ) return;
    } else {
        num_of_atoms = p_mol->GetAtoms()->children().count();
    }

    Coordinates.CreateVector(num_of_atoms);
    Indexes.CreateVector(num_of_atoms);

    int i=0;
    foreach(QObject* p_qobj,p_mol->GetAtoms()->children()) {
        CAtom* p_atom = static_cast<CAtom*>(p_qobj);
        if( selected ){
            if( p_atom->IsFlagSet(EPOF_SELECTED) ) {
                Coordinates[i] = p_atom->GetPos();
                Indexes[i] = p_atom->GetIndex();
                i++;
            }
        } else {
            Coordinates[i] = p_atom->GetPos();
            Indexes[i] = p_atom->GetIndex();
            i++;
        }
    }
}

//------------------------------------------------------------------------------

void CAtomListCoordinatesHI::Forward(void)
{
    CStructure* p_mol = dynamic_cast<CStructure*>(GetProject()->FindObject(MoleculeIndex));
    if(p_mol == NULL) return;

    for(size_t i=0; i < Coordinates.GetLength(); i++) {
        CAtom* p_atom = static_cast<CAtom*>(p_mol->GetProject()->FindObject(Indexes[i]));
        if( p_atom == NULL ) continue;
        CPoint pos = p_atom->GetPos();
        p_atom->SetPos(Coordinates[i]);
        Coordinates[i] = pos;
    }
}

//------------------------------------------------------------------------------

void CAtomListCoordinatesHI::Backward(void)
{
    return(Forward());
}

//------------------------------------------------------------------------------

void CAtomListCoordinatesHI::LoadData(CXMLElement* p_ele)
{
    if( p_ele == NULL ){
        INVALID_ARGUMENT("p_ele is NULL");
    }

    // load core data ----------------------------
    CHistoryItem::LoadData(p_ele);

    // load local data ---------------------------
    p_ele->GetAttribute("mi",MoleculeIndex);

    CXMLBinData* p_bele;
    p_bele = p_ele->GetFirstChildBinData("ai");
    if( p_bele ){
        Indexes.Load(p_bele);
    }
    p_bele = p_ele->GetFirstChildBinData("po");
    if( p_bele ){
        Coordinates.Load(p_bele);
    }
}

//------------------------------------------------------------------------------

void CAtomListCoordinatesHI::SaveData(CXMLElement* p_ele)
{
    if( p_ele == NULL ){
        INVALID_ARGUMENT("p_ele is NULL");
    }

    // save core data ----------------------------
    CHistoryItem::SaveData(p_ele);

    // save local data ---------------------------
    p_ele->SetAttribute("mi",MoleculeIndex);

    CXMLBinData* p_bele;
    p_bele = p_ele->CreateChildBinData("ai");
    Indexes.Save(p_bele);
    p_bele = p_ele->CreateChildBinData("po");
    Coordinates.Save(p_bele);
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

CAtomListChangeParentHI::CAtomListChangeParentHI(CStructure* p_master,
                                                CStructure* p_source,
                                                const QVector<int>& indexes,
                                                int topmindex,int lowsindex)
    : CHistoryItem(&AtomListChangeParentHIObject,p_master->GetProject(),EHID_FORWARD)
{
    MasterIndex = p_master->GetIndex();
    SourceIndex = p_source->GetIndex();
    Atoms.CreateVector(indexes.count());
    for(size_t i=0; i < Atoms.GetLength(); i++) {
        Atoms[i] = indexes.at(i);
    }
    TopMasterSerIndex = topmindex;
    LowSourceSerIndex = lowsindex;
}

//------------------------------------------------------------------------------

void CAtomListChangeParentHI::Forward(void)
{
    CStructure* p_master = dynamic_cast<CStructure*>(GetProject()->FindObject(MasterIndex));
    if( p_master == NULL ) return;
    CStructure* p_source = dynamic_cast<CStructure*>(GetProject()->FindObject(SourceIndex));
    if( p_source == NULL ) return;

    p_master->BeginUpdate();
    p_source->BeginUpdate();
    for(size_t i=0; i < Atoms.GetLength(); i++) {
        CAtom* p_atom = dynamic_cast<CAtom*>(GetProject()->FindObject(Atoms[i]));
        if( p_atom == NULL ) return;
        p_atom->ChangeParent(p_master->GetAtoms());
        if( TopMasterSerIndex >= LowSourceSerIndex ){
            int si = TopMasterSerIndex - LowSourceSerIndex + 1 + p_atom->GetSerIndex();
            p_atom->SetSerIndex(si);
        }
    }
    p_source->EndUpdate();
    p_master->EndUpdate();
}

//------------------------------------------------------------------------------

void CAtomListChangeParentHI::Backward(void)
{
    CStructure* p_master = dynamic_cast<CStructure*>(GetProject()->FindObject(MasterIndex));
    if( p_master == NULL ) return;
    CStructure* p_source = dynamic_cast<CStructure*>(GetProject()->FindObject(SourceIndex));
    if( p_source == NULL ) return;

    p_master->BeginUpdate();
    p_source->BeginUpdate();
    for(size_t i=0; i < Atoms.GetLength(); i++) {
        CAtom* p_atom = dynamic_cast<CAtom*>(GetProject()->FindObject(Atoms[i]));
        if( p_atom == NULL ) return;
        p_atom->ChangeParent(p_source->GetAtoms());
        if( TopMasterSerIndex >= LowSourceSerIndex ){
            int si = p_atom->GetSerIndex() - (TopMasterSerIndex - LowSourceSerIndex + 1);
            p_atom->SetSerIndex(si);
        }
    }
    p_source->EndUpdate();
    p_master->EndUpdate();
}

//------------------------------------------------------------------------------

void CAtomListChangeParentHI::LoadData(CXMLElement* p_ele)
{
    if( p_ele == NULL ){
        INVALID_ARGUMENT("p_ele is NULL");
    }

    // load core data ----------------------------
    CHistoryItem::LoadData(p_ele);

    // load local data ---------------------------
    p_ele->GetAttribute("mi",MasterIndex);
    p_ele->GetAttribute("si",SourceIndex);
    p_ele->GetAttribute("ti",TopMasterSerIndex);
    p_ele->GetAttribute("li",LowSourceSerIndex);

    CXMLBinData* p_bele;
    p_bele = p_ele->GetFirstChildBinData("ai");
    if( p_bele ){
        Atoms.Load(p_bele);
    }
}

//------------------------------------------------------------------------------

void CAtomListChangeParentHI::SaveData(CXMLElement* p_ele)
{
    if( p_ele == NULL ){
        INVALID_ARGUMENT("p_ele is NULL");
    }

    // save core data ----------------------------
    CHistoryItem::SaveData(p_ele);

    // save local data ---------------------------
    p_ele->SetAttribute("mi",MasterIndex);
    p_ele->SetAttribute("si",SourceIndex);
    p_ele->SetAttribute("ti",TopMasterSerIndex);
    p_ele->SetAttribute("li",LowSourceSerIndex);

    CXMLBinData* p_bele;
    p_bele = p_ele->CreateChildBinData("ai");
    Atoms.Save(p_bele);
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

