// =============================================================================
// NEMESIS - Molecular Modelling Package
// -----------------------------------------------------------------------------
//    Copyright (C) 2009 Petr Kulhanek, kulhanek@chemi.muni.cz,
//                       Jakub Stepan, xstepan3@chemi.muni.cz
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

#include <Optimizer.hpp>
#include <Structure.hpp>
#include <Project.hpp>
#include <AtomListHistory.hpp>
#include <AtomList.hpp>
#include <OptimizerSetup.hpp>

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

COptimizer::COptimizer(CPluginObject* p_objectinfo,CProject* p_project)
    : CJob(p_objectinfo,p_project)
{
    Setup = NULL;
    Structure = NULL;
    MaxOptSteps = 0;
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

void COptimizer::SetOptimizerSetup(COptimizerSetup* p_setup)
{
    Setup = p_setup;
}

//------------------------------------------------------------------------------

void COptimizer::SetOptimizedMolecule(CStructure* p_mol)
{
    Structure = p_mol;
}

//------------------------------------------------------------------------------

void COptimizer::SetMaxOptSteps(int num)
{
    MaxOptSteps = num;
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

bool COptimizer::InitializeJob(void)
{
    if( (Structure == NULL) || (Setup == NULL) ) return(false);

    CHistoryNode* p_node = BeginChangeWH(EHCL_GEOMETRY,tr("optimization"));
    if( p_node == NULL ) return(false);
        // record current molecule coordinates
        CAtomListCoordinatesHI* p_history = new CAtomListCoordinatesHI(Structure);
        p_node->Register(p_history);
    EndChangeWH();

    CProject* p_project = GetProject();
    BackupLockLevels = p_project->GetHistory()->GetLockModeLevels();
    CLockLevels super_lock = ~CLockLevels();
    p_project->GetHistory()->SetLockModeLevels(super_lock);

    connect(this,SIGNAL(OnNotifyTick(const QString&)),
            this,SLOT(NotifyTick(const QString&)));

    Structure->BeginGeometryUpdate();

    return(true);
}

//------------------------------------------------------------------------------

bool COptimizer::ExecuteJob(void)
{
    if( (Structure == NULL) || (Setup == NULL) ) return(false);

    // initialize
    Structure->BeginUpdate();
    if( InitializationStep() == false ){
        Structure->EndUpdate();
        return(false);
    }

    // optimize
    int step = 1;
    while( (Terminated == false) && OptimizationStep() ) {
        if( step % Setup->GetNotifyTickInterval() == 0 ){
            QString text = GetStepDescription();
            // this is due to thread safety
            emit OnNotifyTick(text);
        }
        step++;
    }

    // finish optimization
    FinalizationStep();
    Structure->EndUpdate();

    return(true);
}

//------------------------------------------------------------------------------

bool COptimizer::FinalizeJob(void)
{
    if( (Structure == NULL) || (Setup == NULL) ) return(false);

    CProject* p_project = GetProject();
    p_project->GetHistory()->SetLockModeLevels(BackupLockLevels);

    Structure->EndGeometryUpdate();
    QString text = GetStepDescription(true);
    GetProject()->TextNotification(ETNT_TEXT,text,3000);
    GetProject()->RepaintProject();

    return(true);
}

//------------------------------------------------------------------------------

void COptimizer::NotifyTick(const QString& text)
{
    Structure->NotifyGeometryChangeTick();
    GetProject()->TextNotification(ETNT_TEXT,text);
    GetProject()->RepaintProject();
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

bool COptimizer::InitializationStep(void)
{
    return(true);
}

//------------------------------------------------------------------------------

bool COptimizer::OptimizationStep(void)
{
    return(true);
}

//------------------------------------------------------------------------------

bool COptimizer::FinalizationStep(void)
{
    return(true);
}

//------------------------------------------------------------------------------

const QString COptimizer::GetStepDescription(bool final)
{
    return(QString());
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================


