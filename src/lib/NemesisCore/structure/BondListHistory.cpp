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

#include <BondListHistory.hpp>
#include <ProObjectHistory.hpp>
#include <Project.hpp>
#include <Structure.hpp>
#include <BondList.hpp>
#include <Bond.hpp>
#include <NemesisCoreModule.hpp>

//------------------------------------------------------------------------------

REGISTER_HISTORY_OBJECT(NemesisCorePlugin,BondListChangeParentHI,
                        "{BOND_L_CHP:6b13d3a2-6e25-4f53-bdb4-b1cb642bfc2e}")

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

CBondListChangeParentHI::CBondListChangeParentHI(CStructure* p_master,
                                                CStructure* p_source,
                                                const QVector<int>& indexes)
    : CHistoryItem(&BondListChangeParentHIObject,p_master->GetProject(),EHID_FORWARD)
{
    MasterIndex = p_master->GetIndex();
    SourceIndex = p_source->GetIndex();
    Bonds.CreateVector(indexes.count());
    for(size_t i=0; i < Bonds.GetLength(); i++) {
        Bonds[i] = indexes.at(i);
    }
}

//------------------------------------------------------------------------------

void CBondListChangeParentHI::Forward(void)
{
    CStructure* p_master = dynamic_cast<CStructure*>(GetProject()->FindObject(MasterIndex));
    if( p_master == NULL ) return;
    CStructure* p_source = dynamic_cast<CStructure*>(GetProject()->FindObject(SourceIndex));
    if( p_source == NULL ) return;

    p_master->BeginUpdate();
    p_source->BeginUpdate();
    for(size_t i=0; i < Bonds.GetLength(); i++) {
        CBond* p_bond = dynamic_cast<CBond*>(GetProject()->FindObject(Bonds[i]));
        if( p_bond == NULL ) return;
        p_bond->ChangeParent(p_master->GetBonds());
    }
    p_source->EndUpdate();
    p_master->EndUpdate();
}

//------------------------------------------------------------------------------

void CBondListChangeParentHI::Backward(void)
{
    CStructure* p_master = dynamic_cast<CStructure*>(GetProject()->FindObject(MasterIndex));
    if( p_master == NULL ) return;
    CStructure* p_source = dynamic_cast<CStructure*>(GetProject()->FindObject(SourceIndex));
    if( p_source == NULL ) return;

    p_master->BeginUpdate();
    p_source->BeginUpdate();
    for(size_t i=0; i < Bonds.GetLength(); i++) {
        CBond* p_bond = dynamic_cast<CBond*>(GetProject()->FindObject(Bonds[i]));
        if( p_bond == NULL ) return;
        p_bond->ChangeParent(p_source->GetBonds());
    }
    p_source->EndUpdate();
    p_master->EndUpdate();
}

//------------------------------------------------------------------------------

void CBondListChangeParentHI::LoadData(CXMLElement* p_ele)
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
        Bonds.Load(p_bele);
    }
}

//------------------------------------------------------------------------------

void CBondListChangeParentHI::SaveData(CXMLElement* p_ele)
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
    Bonds.Save(p_bele);
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================
