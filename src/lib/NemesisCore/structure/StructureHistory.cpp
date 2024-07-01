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

#include <StructureHistory.hpp>
#include <Structure.hpp>
#include <ProObjectHistory.hpp>
#include <Project.hpp>
#include <StructureList.hpp>
#include <ErrorSystem.hpp>
#include <XMLElement.hpp>
#include <NemesisCoreModule.hpp>
#include <CategoryUUID.hpp>

//------------------------------------------------------------------------------

REGISTER_HISTORY_OBJECT(NemesisCorePlugin,StructureHI,
                        "{STR:0041b85d-6c1b-4d5d-8c8e-bd6366d7e9c9}")
REGISTER_HISTORY_OBJECT(NemesisCorePlugin,StructureBoxHI,
                        "{STRB:f46247e4-fda9-4dfe-bc4c-9e416ce2eac0}")
REGISTER_HISTORY_OBJECT(NemesisCorePlugin,StructureUpdateHI,
                        "{STRU:31a676a2-1737-4fc1-baa3-65f3fad242b1}")
REGISTER_HISTORY_OBJECT(NemesisCorePlugin,StructureSeqIndexHI,
                        "{STRSI:1881c6cc-1dc8-4c47-8176-c1ad165f78c4}")
REGISTER_HISTORY_OBJECT(NemesisCorePlugin,StructureTotalChargeHI,
                        "{STRTC:027cb53d-71fb-47c4-ba41-2cfed39a9474}")

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

CStructureHI::CStructureHI(CStructure* p_str,EHistoryItemDirection change)
    : CHistoryItem(&StructureHIObject,p_str->GetProject(),change)
{
    MoleculeIndex = p_str->GetIndex();
    CXMLElement* p_ele = StructureData.CreateChildElement("structure");
    p_str->SaveData(p_ele);
}

//------------------------------------------------------------------------------

void CStructureHI::Forward(void)
{
    CXMLElement* p_ele = StructureData.GetFirstChildElement("structure");
    GetProject()->GetStructures()->CreateStructure(p_ele);
}

//------------------------------------------------------------------------------

void CStructureHI::Backward(void)
{
    CXMLElement* p_ele = StructureData.GetFirstChildElement("structure");
    if( p_ele == NULL ) return;
    int str_index = -1;
    p_ele->GetAttribute("id",str_index);

    CStructure* p_str = dynamic_cast<CStructure*>(GetProject()->FindObject(str_index));
    if(p_str == NULL) return;

    // save designer setup
    CXMLElement* p_sdele = p_str->GetDesignerData();
    if( p_sdele ){
        CXMLElement* p_tdele = StructureData.GetChildElementByPath("structure/designer",true);
        p_tdele->RemoveAllChildNodes();
        p_tdele->RemoveAllAttributes();
        p_tdele->CopyContentsFrom(p_sdele);
    }

    p_str->RemoveFromBaseList();
}


//------------------------------------------------------------------------------

void CStructureHI::LoadData(CXMLElement* p_ele)
{
    if( p_ele == NULL ){
        INVALID_ARGUMENT("p_ele is NULL");
    }

    // load core data ----------------------------
    CHistoryItem::LoadData(p_ele);

    // load local data ---------------------------
    CXMLElement* p_sele = p_ele->GetFirstChildElement("structure");
    if( p_sele == NULL ) return;
    StructureData.RemoveAllChildNodes();
    CXMLElement* p_dele = StructureData.CreateChildElement("structure");
    p_dele->CopyContentsFrom(p_sele);
}

//------------------------------------------------------------------------------

void CStructureHI::SaveData(CXMLElement* p_ele)
{
    if( p_ele == NULL ){
        INVALID_ARGUMENT("p_ele is NULL");
    }

    // save core data ----------------------------
    CHistoryItem::SaveData(p_ele);

    // save local data ---------------------------
    CXMLElement* p_sele = StructureData.GetFirstChildElement("structure");
    if( p_sele == NULL ) return;
    CXMLElement* p_dele = p_ele->CreateChildElement("structure");
    p_dele->CopyContentsFrom(p_sele);
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

CStructureBoxHI::CStructureBoxHI(CStructure* p_mol,
                                    bool newpa,bool newpb,bool newpc,
                                    const CPoint& newsizes,const CPoint& newangles)
    : CHistoryItem(&StructureBoxHIObject,p_mol->GetProject(),EHID_FORWARD)
{
    MoleculeIndex = p_mol->GetIndex();

    OldPA = p_mol->PBCInfo.IsPeriodicAlongA();
    OldPB = p_mol->PBCInfo.IsPeriodicAlongB();
    OldPC = p_mol->PBCInfo.IsPeriodicAlongC();
    OldSizes = p_mol->PBCInfo.GetSizes();
    OldAngles = p_mol->PBCInfo.GetAngles();

    NewPA = newpa;
    NewPB = newpb;
    NewPC = newpc;
    NewSizes = newsizes;
    NewAngles = newangles;
}

//------------------------------------------------------------------------------

void CStructureBoxHI::Forward(void)
{
    CStructure *p_mol = dynamic_cast<CStructure*>(GetProject()->FindObject(MoleculeIndex));
    if( p_mol == NULL ) return;

    p_mol->PBCInfo.SetDimmensions(NewSizes,NewAngles);
    p_mol->PBCInfo.SetPeriodicity(NewPA,NewPB,NewPC);
}

//------------------------------------------------------------------------------

void CStructureBoxHI::Backward(void)
{
    CStructure *p_mol = dynamic_cast<CStructure*>(GetProject()->FindObject(MoleculeIndex));
    if( p_mol == NULL ) return;

    p_mol->PBCInfo.SetDimmensions(OldSizes,OldAngles);
    p_mol->PBCInfo.SetPeriodicity(OldPA,OldPB,OldPC);
}

//------------------------------------------------------------------------------

void CStructureBoxHI::LoadData(CXMLElement* p_ele)
{
    if( p_ele == NULL ){
        INVALID_ARGUMENT("p_ele is NULL");
    }

    // load core data ----------------------------
    CHistoryItem::LoadData(p_ele);

    // load local data ---------------------------
    p_ele->GetAttribute("mi",MoleculeIndex);
    p_ele->GetAttribute("nsx",NewSizes.x);
    p_ele->GetAttribute("nsy",NewSizes.y);
    p_ele->GetAttribute("nsz",NewSizes.z);
    p_ele->GetAttribute("nax",NewAngles.x);
    p_ele->GetAttribute("nay",NewAngles.y);
    p_ele->GetAttribute("naz",NewAngles.z);
    p_ele->GetAttribute("npa",NewPA);
    p_ele->GetAttribute("npb",NewPB);
    p_ele->GetAttribute("npc",NewPC);

    p_ele->GetAttribute("osx",OldSizes.x);
    p_ele->GetAttribute("osy",OldSizes.y);
    p_ele->GetAttribute("osz",OldSizes.z);
    p_ele->GetAttribute("oax",OldAngles.x);
    p_ele->GetAttribute("oay",OldAngles.y);
    p_ele->GetAttribute("oaz",OldAngles.z);
    p_ele->GetAttribute("opa",OldPA);
    p_ele->GetAttribute("opb",OldPB);
    p_ele->GetAttribute("opc",OldPC);
}

//------------------------------------------------------------------------------

void CStructureBoxHI::SaveData(CXMLElement* p_ele)
{
    if( p_ele == NULL ){
        INVALID_ARGUMENT("p_ele is NULL");
    }

    // save core data ----------------------------
    CHistoryItem::SaveData(p_ele);

    // save local data ---------------------------
    p_ele->SetAttribute("mi",MoleculeIndex);
    p_ele->SetAttribute("nsx",NewSizes.x);
    p_ele->SetAttribute("nsy",NewSizes.y);
    p_ele->SetAttribute("nsz",NewSizes.z);
    p_ele->SetAttribute("nax",NewAngles.x);
    p_ele->SetAttribute("nay",NewAngles.y);
    p_ele->SetAttribute("naz",NewAngles.z);
    p_ele->SetAttribute("npa",NewPA);
    p_ele->SetAttribute("npb",NewPB);
    p_ele->SetAttribute("npc",NewPC);

    p_ele->SetAttribute("osx",OldSizes.x);
    p_ele->SetAttribute("osy",OldSizes.y);
    p_ele->SetAttribute("osz",OldSizes.z);
    p_ele->SetAttribute("oax",OldAngles.x);
    p_ele->SetAttribute("oay",OldAngles.y);
    p_ele->SetAttribute("oaz",OldAngles.z);
    p_ele->SetAttribute("opa",OldPA);
    p_ele->SetAttribute("opb",OldPB);
    p_ele->SetAttribute("opc",OldPC);
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

CStructureUpdateHI::CStructureUpdateHI(CStructure* p_mol,
                                                 EHistoryItemDirection change)
    : CHistoryItem(&StructureUpdateHIObject,p_mol->GetProject(),change)
{
    MoleculeIndex = p_mol->GetIndex();
}

//------------------------------------------------------------------------------

void CStructureUpdateHI::Forward(void)
{
    CStructure *p_mol = dynamic_cast<CStructure*>(GetProject()->FindObject(MoleculeIndex));
    if( p_mol == NULL ) return;

    p_mol->BeginUpdate();
}

//------------------------------------------------------------------------------

void CStructureUpdateHI::Backward(void)
{
    CStructure *p_mol = dynamic_cast<CStructure*>(GetProject()->FindObject(MoleculeIndex));
    if( p_mol == NULL ) return;

    p_mol->EndUpdate();
}

//------------------------------------------------------------------------------

void CStructureUpdateHI::LoadData(CXMLElement* p_ele)
{
    if( p_ele == NULL ){
        INVALID_ARGUMENT("p_ele is NULL");
    }

    // load core data ----------------------------
    CHistoryItem::LoadData(p_ele);

    // load local data ---------------------------
    p_ele->GetAttribute("mi",MoleculeIndex);
}

//------------------------------------------------------------------------------

void CStructureUpdateHI::SaveData(CXMLElement* p_ele)
{
    if( p_ele == NULL ){
        INVALID_ARGUMENT("p_ele is NULL");
    }

    // save core data ----------------------------
    CHistoryItem::SaveData(p_ele);

    // save local data ---------------------------
    p_ele->SetAttribute("mi",MoleculeIndex);
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

CStructureSeqIndexHI::CStructureSeqIndexHI(CStructure* p_res,int newseqidx)
    : CHistoryItem(&StructureSeqIndexHIObject,p_res->GetProject(),EHID_FORWARD)
{
    StructureIndex = p_res->GetIndex();
    OldSeqIndex = p_res->GetSeqIndex();
    NewSeqIndex = newseqidx;
}

//------------------------------------------------------------------------------

void CStructureSeqIndexHI::Forward(void)
{
    CStructure* p_res = dynamic_cast<CStructure*>(GetProject()->FindObject(StructureIndex));
    if(p_res == NULL) return;

    p_res->SetSeqIndex(NewSeqIndex);
}

//------------------------------------------------------------------------------

void CStructureSeqIndexHI::Backward(void)
{
    CStructure* p_res = dynamic_cast<CStructure*>(GetProject()->FindObject(StructureIndex));
    if(p_res == NULL) return;

    p_res->SetSeqIndex(OldSeqIndex);
}

//------------------------------------------------------------------------------

void CStructureSeqIndexHI::LoadData(CXMLElement* p_ele)
{
    if( p_ele == NULL ){
        INVALID_ARGUMENT("p_ele is NULL");
    }

    // load core data ----------------------------
    CHistoryItem::LoadData(p_ele);

    // load local data ---------------------------
    p_ele->GetAttribute("ri",StructureIndex);
    p_ele->GetAttribute("ns",NewSeqIndex);
    p_ele->GetAttribute("os",OldSeqIndex);
}

//------------------------------------------------------------------------------

void CStructureSeqIndexHI::SaveData(CXMLElement* p_ele)
{
    if( p_ele == NULL ){
        INVALID_ARGUMENT("p_ele is NULL");
    }

    // save core data ----------------------------
    CHistoryItem::SaveData(p_ele);

    // save local data ---------------------------
    p_ele->SetAttribute("ri",StructureIndex);
    p_ele->SetAttribute("ns",NewSeqIndex);
    p_ele->SetAttribute("os",OldSeqIndex);
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

CStructureTotalChargeHI::CStructureTotalChargeHI(CStructure* p_res,int newTotalCharge)
    : CHistoryItem(&StructureTotalChargeHIObject,p_res->GetProject(),EHID_FORWARD)
{
    StructureIndex = p_res->GetIndex();
    OldTotalCharge = p_res->GetTotalCharge();
    NewTotalCharge = newTotalCharge;
}

//------------------------------------------------------------------------------

void CStructureTotalChargeHI::Forward(void)
{
    CStructure* p_res = dynamic_cast<CStructure*>(GetProject()->FindObject(StructureIndex));
    if(p_res == NULL) return;

    p_res->SetTotalCharge(NewTotalCharge);
}

//------------------------------------------------------------------------------

void CStructureTotalChargeHI::Backward(void)
{
    CStructure* p_res = dynamic_cast<CStructure*>(GetProject()->FindObject(StructureIndex));
    if(p_res == NULL) return;

    p_res->SetTotalCharge(OldTotalCharge);
}

//------------------------------------------------------------------------------

void CStructureTotalChargeHI::LoadData(CXMLElement* p_ele)
{
    if( p_ele == NULL ){
        INVALID_ARGUMENT("p_ele is NULL");
    }

    // load core data ----------------------------
    CHistoryItem::LoadData(p_ele);

    // load local data ---------------------------
    p_ele->GetAttribute("ri",StructureIndex);
    p_ele->GetAttribute("nc",NewTotalCharge);
    p_ele->GetAttribute("oc",OldTotalCharge);
}

//------------------------------------------------------------------------------

void CStructureTotalChargeHI::SaveData(CXMLElement* p_ele)
{
    if( p_ele == NULL ){
        INVALID_ARGUMENT("p_ele is NULL");
    }

    // save core data ----------------------------
    CHistoryItem::SaveData(p_ele);

    // save local data ---------------------------
    p_ele->SetAttribute("ri",StructureIndex);
    p_ele->SetAttribute("nc",NewTotalCharge);
    p_ele->SetAttribute("oc",OldTotalCharge);
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

