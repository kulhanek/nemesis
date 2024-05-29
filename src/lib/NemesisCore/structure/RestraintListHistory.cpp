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

#include <RestraintListHistory.hpp>
#include <Structure.hpp>
#include <RestraintList.hpp>
#include <ProObjectHistory.hpp>
#include <Project.hpp>
#include <ErrorSystem.hpp>
#include <XMLElement.hpp>
#include <NemesisCoreModule.hpp>
#include <CategoryUUID.hpp>

//------------------------------------------------------------------------------

REGISTER_HISTORY_OBJECT(NemesisCorePlugin,RestraintListEnabledHI,
                        "{RSTLST_EN:50c89c64-6a08-4648-bf9d-b0940e66226f}")
REGISTER_HISTORY_OBJECT(NemesisCorePlugin,RestraintListChangeParentHI,
                        "{RSTLST_CHP:a807433f-0649-438f-81bb-3946d8b93f4c}")

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

CRestraintListEnabledHI::CRestraintListEnabledHI(CRestraintList* p_rl,bool set)
    : CHistoryItem(&RestraintListEnabledHIObject,p_rl->GetProject(),EHID_FORWARD)
{
    MoleculeIndex = p_rl->GetStructure()->GetIndex();
    OldEnabled = p_rl->IsEnabled();
    NewEnabled = set;
}

//------------------------------------------------------------------------------

void CRestraintListEnabledHI::Forward(void)
{
    CStructure* p_str = dynamic_cast<CStructure*>(GetProject()->FindObject(MoleculeIndex));
    if(p_str == NULL) return;
    p_str->GetRestraints()->EnableRestraints(NewEnabled);
}

//------------------------------------------------------------------------------

void CRestraintListEnabledHI::Backward(void)
{
    CStructure* p_str = dynamic_cast<CStructure*>(GetProject()->FindObject(MoleculeIndex));
    if(p_str == NULL) return;
    p_str->GetRestraints()->EnableRestraints(OldEnabled);
}

//------------------------------------------------------------------------------

void CRestraintListEnabledHI::LoadData(CXMLElement* p_ele)
{
    if( p_ele == NULL ){
        INVALID_ARGUMENT("p_ele is NULL");
    }

    // load core data ----------------------------
    CHistoryItem::LoadData(p_ele);

    // load local data ---------------------------
    p_ele->GetAttribute("mi",MoleculeIndex);
    p_ele->GetAttribute("ne",NewEnabled);
    p_ele->GetAttribute("oe",OldEnabled);
}

//------------------------------------------------------------------------------

void CRestraintListEnabledHI::SaveData(CXMLElement* p_ele)
{
    if( p_ele == NULL ){
        INVALID_ARGUMENT("p_ele is NULL");
    }

    // save core data ----------------------------
    CHistoryItem::SaveData(p_ele);

    // save local data ---------------------------
    p_ele->SetAttribute("mi",MoleculeIndex);
    p_ele->SetAttribute("ne",NewEnabled);
    p_ele->SetAttribute("oe",OldEnabled);
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

CRestraintListChangeParentHI::CRestraintListChangeParentHI(CStructure* p_master,
                                                CStructure* p_source,
                                                const QVector<int>& indexes)
    : CHistoryItem(&RestraintListChangeParentHIObject,p_master->GetProject(),EHID_FORWARD)
{
    MasterIndex = p_master->GetIndex();
    SourceIndex = p_source->GetIndex();
    Restraints.CreateVector(indexes.count());
    for(size_t i=0; i < Restraints.GetLength(); i++) {
        Restraints[i] = indexes.at(i);
    }
}

//------------------------------------------------------------------------------

void CRestraintListChangeParentHI::Forward(void)
{
    CStructure* p_master = dynamic_cast<CStructure*>(GetProject()->FindObject(MasterIndex));
    if( p_master == NULL ) return;
    CStructure* p_source = dynamic_cast<CStructure*>(GetProject()->FindObject(SourceIndex));
    if( p_source == NULL ) return;

    p_master->BeginUpdate();
    p_source->BeginUpdate();
    for(size_t i=0; i < Restraints.GetLength(); i++) {
        CRestraint* p_bond = dynamic_cast<CRestraint*>(GetProject()->FindObject(Restraints[i]));
        if( p_bond == NULL ) return;
        p_bond->ChangeParent(p_master->GetRestraints());
    }
    p_source->EndUpdate();
    p_master->EndUpdate();
}

//------------------------------------------------------------------------------

void CRestraintListChangeParentHI::Backward(void)
{
    CStructure* p_master = dynamic_cast<CStructure*>(GetProject()->FindObject(MasterIndex));
    if( p_master == NULL ) return;
    CStructure* p_source = dynamic_cast<CStructure*>(GetProject()->FindObject(SourceIndex));
    if( p_source == NULL ) return;

    p_master->BeginUpdate();
    p_source->BeginUpdate();
    for(size_t i=0; i < Restraints.GetLength(); i++) {
        CRestraint* p_bond = dynamic_cast<CRestraint*>(GetProject()->FindObject(Restraints[i]));
        if( p_bond == NULL ) return;
        p_bond->ChangeParent(p_source->GetRestraints());
    }
    p_source->EndUpdate();
    p_master->EndUpdate();
}

//------------------------------------------------------------------------------

void CRestraintListChangeParentHI::LoadData(CXMLElement* p_ele)
{
    if( p_ele == NULL ){
        INVALID_ARGUMENT("p_ele is NULL");
    }

    // load core data ----------------------------
    CHistoryItem::LoadData(p_ele);

    // load local data ---------------------------
    p_ele->GetAttribute("mi",MasterIndex);
    p_ele->GetAttribute("si",SourceIndex);

    CXMLBinData* p_bele;
    p_bele = p_ele->GetFirstChildBinData("bi");
    if( p_bele ){
        Restraints.Load(p_bele);
    }
}

//------------------------------------------------------------------------------

void CRestraintListChangeParentHI::SaveData(CXMLElement* p_ele)
{
    if( p_ele == NULL ){
        INVALID_ARGUMENT("p_ele is NULL");
    }

    // save core data ----------------------------
    CHistoryItem::SaveData(p_ele);

    // save local data ---------------------------
    p_ele->SetAttribute("mi",MasterIndex);
    p_ele->SetAttribute("si",SourceIndex);

    CXMLBinData* p_bele;
    p_bele = p_ele->CreateChildBinData("bi");
    Restraints.Save(p_bele);
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

