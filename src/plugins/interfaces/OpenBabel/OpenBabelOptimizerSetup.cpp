// =============================================================================
// NEMESIS - Molecular Modelling Package
// -----------------------------------------------------------------------------
//    Copyright (C) 2010 Petr Kulhanek, kulhanek@chemi.muni.cz
//    Copyright (C) 2009 Petr Kulhanek, kulhanek@chemi.muni.cz,
//                       Jakub Stepan, xstepan3@chemi.muni.cz
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

#include "OpenBabelOptimizerSetup.hpp"
#include "OpenBabelOptimizer.hpp"
#include "OpenBabelModule.hpp"

#include <CategoryUUID.hpp>
#include <ErrorSystem.hpp>

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

QObject* OpenBabelOptimizerSetupCB(void* p_data);

CExtUUID        OpenBabelOptimizerSetupID(
                    "{OPEN_BABEL_OPTIMIZER_SETUP:0af7c638-b469-496a-839f-e8a1c67f4f79}",
                    "OpenBabel Optimizer");

CPluginObject   OpenBabelOptimizerSetupObject(&OpenBabelPlugin,
                    OpenBabelOptimizerSetupID,OPTIMIZER_SETUP_CAT,
                    ":/images/OpenBabel/Openbabel.png",
                    OpenBabelOptimizerSetupCB);

// -----------------------------------------------------------------------------

QObject* OpenBabelOptimizerSetupCB(void* p_data)
{
    CExtComObject* p_owner = static_cast<CExtComObject*>(p_data);
    COpenBabelOptimizerSetup* p_object = new COpenBabelOptimizerSetup(p_owner);
    return(p_object);
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

COpenBabelOptimizerSetup::COpenBabelOptimizerSetup(CExtComObject* p_parent)
    : COptimizerSetup(&OpenBabelOptimizerSetupObject,p_parent)
{
    ForceFieldName = "UFF";
    ChargeMethod = "gasteiger";
    MaxSteps = 5000;
    StepsPerUpdate = 10;
    TermCrit = 1e-4;

    OptimizationMethod = EOBOM_STEEPEST_DESCENT;

    NBUpdate = 10;
    EnableCutoff = false;
    vdwCutoff = 6.0;
    eleCutoff = 10.0;
}

//------------------------------------------------------------------------------

COpenBabelOptimizerSetup::~COpenBabelOptimizerSetup(void)
{
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

const CUUID& COpenBabelOptimizerSetup::GetOptimizerUUID(void)
{
    return(OpenBabelOptimizerID);
}

//------------------------------------------------------------------------------

int COpenBabelOptimizerSetup::GetNotifyTickInterval(void)
{
    return(StepsPerUpdate);
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

void COpenBabelOptimizerSetup::LoadData(CXMLElement* p_ele)
{
    if( p_ele == NULL ){
        INVALID_ARGUMENT("p_ele is NULL");
    }

    // load header
    COptimizerSetup::LoadData(p_ele);

    // load custom setup
    p_ele->GetAttribute("ForceFieldName",ForceFieldName);
    p_ele->GetAttribute("ChargeMethod",ChargeMethod);
    p_ele->GetAttribute("MaxSteps",MaxSteps);
    p_ele->GetAttribute("StepsPerUpdate",StepsPerUpdate);
    p_ele->GetAttribute("TermCrit",TermCrit);
    p_ele->GetAttribute<EOBOptimizationMethod>("OptimizationMethod",OptimizationMethod);
    p_ele->GetAttribute("NBUpdate",NBUpdate);
    p_ele->GetAttribute("EnableCutoff",EnableCutoff);
    p_ele->GetAttribute("vdwCutoff",vdwCutoff);
    p_ele->GetAttribute("eleCutoff",eleCutoff);
}

//------------------------------------------------------------------------------

void COpenBabelOptimizerSetup::SaveData(CXMLElement* p_ele)
{
    if( p_ele == NULL ){
        INVALID_ARGUMENT("p_ele is NULL");
    }

    // save header
    COptimizerSetup::SaveData(p_ele);

    // save custom setup
    p_ele->SetAttribute("ForceFieldName",ForceFieldName);
    p_ele->SetAttribute("ChargeMethod",ChargeMethod);
    p_ele->SetAttribute("MaxSteps",MaxSteps);
    p_ele->SetAttribute("StepsPerUpdate",StepsPerUpdate);
    p_ele->SetAttribute("TermCrit",TermCrit);
    p_ele->SetAttribute("OptimizationMethod",OptimizationMethod);
    p_ele->SetAttribute("NBUpdate",NBUpdate);
    p_ele->SetAttribute("EnableCutoff",EnableCutoff);
    p_ele->SetAttribute("vdwCutoff",vdwCutoff);
    p_ele->SetAttribute("eleCutoff",eleCutoff);
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================



