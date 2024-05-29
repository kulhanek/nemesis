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

#include <ResidueListHistory.hpp>
#include <ProObjectHistory.hpp>
#include <Project.hpp>
#include <Structure.hpp>
#include <ResidueList.hpp>
#include <Residue.hpp>
#include <NemesisCoreModule.hpp>

//------------------------------------------------------------------------------

REGISTER_HISTORY_OBJECT(NemesisCorePlugin,ResidueListChangeParentHI,
                        "{RES_L_CHP:5d11ec93-5b20-4481-ac62-3dfc2b998a72}")

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

CResidueListChangeParentHI::CResidueListChangeParentHI(CStructure* p_master,
                                                CStructure* p_source,
                                                const QVector<int>& indexes,
                                                int topmindex,int lowsindex)
    : CHistoryItem(&ResidueListChangeParentHIObject,p_master->GetProject(),EHID_FORWARD)
{
    MasterIndex = p_master->GetIndex();
    SourceIndex = p_source->GetIndex();
    Residues.CreateVector(indexes.count());
    for(size_t i=0; i < Residues.GetLength(); i++) {
        Residues[i] = indexes.at(i);
    }
    TopMasterSerIndex = topmindex;
    LowSourceSerIndex = lowsindex;
}

//------------------------------------------------------------------------------

void CResidueListChangeParentHI::Forward(void)
{
    CStructure* p_master = dynamic_cast<CStructure*>(GetProject()->FindObject(MasterIndex));
    if( p_master == NULL ) return;
    CStructure* p_source = dynamic_cast<CStructure*>(GetProject()->FindObject(SourceIndex));
    if( p_source == NULL ) return;

    p_master->BeginUpdate();
    p_source->BeginUpdate();
    for(size_t i=0; i < Residues.GetLength(); i++) {
        CResidue* p_res = dynamic_cast<CResidue*>(GetProject()->FindObject(Residues[i]));
        if( p_res == NULL ) return;
        p_res->ChangeParent(p_master->GetResidues());
        if( TopMasterSerIndex >= LowSourceSerIndex ){
            int si = TopMasterSerIndex - LowSourceSerIndex + 1 + p_res->GetSeqIndex();
            p_res->SetSeqIndex(si);
        }
    }
    p_source->EndUpdate();
    p_master->EndUpdate();
}

//------------------------------------------------------------------------------

void CResidueListChangeParentHI::Backward(void)
{
    CStructure* p_master = dynamic_cast<CStructure*>(GetProject()->FindObject(MasterIndex));
    if( p_master == NULL ) return;
    CStructure* p_source = dynamic_cast<CStructure*>(GetProject()->FindObject(SourceIndex));
    if( p_source == NULL ) return;

    p_master->BeginUpdate();
    p_source->BeginUpdate();
    for(size_t i=0; i < Residues.GetLength(); i++) {
        CResidue* p_res = dynamic_cast<CResidue*>(GetProject()->FindObject(Residues[i]));
        if( p_res == NULL ) return;
        p_res->ChangeParent(p_source->GetResidues());
        if( TopMasterSerIndex >= LowSourceSerIndex ){
            int si = p_res->GetSeqIndex() - (TopMasterSerIndex - LowSourceSerIndex + 1);
            p_res->SetSeqIndex(si);
        }
    }
    p_source->EndUpdate();
    p_master->EndUpdate();
}

//------------------------------------------------------------------------------

void CResidueListChangeParentHI::LoadData(CXMLElement* p_ele)
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
        Residues.Load(p_bele);
    }
}

//------------------------------------------------------------------------------

void CResidueListChangeParentHI::SaveData(CXMLElement* p_ele)
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
    Residues.Save(p_bele);
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================
