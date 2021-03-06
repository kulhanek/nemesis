// =============================================================================
// NEMESIS - Molecular Modelling Package
// -----------------------------------------------------------------------------
//    Copyright (C) 2012 Petr Kulhanek, kulhanek@chemi.muni.cz
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

#include <GeoPropertyHistory.hpp>
#include <GeoProperty.hpp>
#include <ProObjectHistory.hpp>
#include <Project.hpp>
#include <NemesisCoreModule.hpp>
#include <CategoryUUID.hpp>
#include <XMLElement.hpp>

//------------------------------------------------------------------------------

REGISTER_HISTORY_OBJECT(NemesisCorePlugin,GeoPropertyChangePositionHI,
                        "{GEOPR_POS:f73adf8e-8a31-4622-a309-afbeeaf1355c}")

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

CGeoPropertyChangePositionHI::CGeoPropertyChangePositionHI(
                        CGeoProperty* p_obj, const CPoint& newPosition)
    : CHistoryItem(&GeoPropertyChangePositionHIObject,p_obj->GetProject(),EHID_FORWARD)
{
    ObjectID = p_obj->GetIndex();
    OldPosition = p_obj->GetLabelPosition();
    NewPosition = newPosition;
}

//------------------------------------------------------------------------------

void CGeoPropertyChangePositionHI::Forward(void)
{
    CGeoProperty* p_go = dynamic_cast<CGeoProperty*>(GetProject()->FindObject(ObjectID));
    if(p_go == NULL) return;
    p_go->SetLabelPosition(NewPosition);
}

//------------------------------------------------------------------------------

void CGeoPropertyChangePositionHI::Backward(void)
{
    CGeoProperty* p_go = dynamic_cast<CGeoProperty*>(GetProject()->FindObject(ObjectID));
    if(p_go == NULL) return;
    p_go->SetLabelPosition(OldPosition);
}

//------------------------------------------------------------------------------

void CGeoPropertyChangePositionHI::LoadData(CXMLElement* p_ele)
{
    if( p_ele == NULL ){
        INVALID_ARGUMENT("p_ele is NULL");
    }

    // load core data ----------------------------
    CHistoryItem::LoadData(p_ele);

    // load local data ---------------------------
    p_ele->GetAttribute("ai",ObjectID);
    p_ele->GetAttribute("npx",NewPosition.x);
    p_ele->GetAttribute("npy",NewPosition.y);
    p_ele->GetAttribute("npz",NewPosition.z);
    p_ele->GetAttribute("opx",OldPosition.x);
    p_ele->GetAttribute("opy",OldPosition.y);
    p_ele->GetAttribute("opz",OldPosition.z);
}

//------------------------------------------------------------------------------

void CGeoPropertyChangePositionHI::SaveData(CXMLElement* p_ele)
{
    if( p_ele == NULL ){
        INVALID_ARGUMENT("p_ele is NULL");
    }

    // save core data ----------------------------
    CHistoryItem::SaveData(p_ele);

    // save local data ---------------------------
    p_ele->SetAttribute("ai",ObjectID);
    p_ele->SetAttribute("npx",NewPosition.x);
    p_ele->SetAttribute("npy",NewPosition.y);
    p_ele->SetAttribute("npz",NewPosition.z);
    p_ele->SetAttribute("opx",OldPosition.x);
    p_ele->SetAttribute("opy",OldPosition.y);
    p_ele->SetAttribute("opz",OldPosition.z);
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================


