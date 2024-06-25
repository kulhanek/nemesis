#ifndef OptimizerH
#define OptimizerH
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

#include <NemesisCoreMainHeader.hpp>
#include <Job.hpp>
#include <HistoryList.hpp>

//------------------------------------------------------------------------------

class CProject;
class CStructure;
class COptimizerSetup;

//------------------------------------------------------------------------------

/// base class for molecule geometry optimizer

class NEMESIS_CORE_PACKAGE COptimizer : public CJob {
    Q_OBJECT
public:
// constructor and destructor --------------------------------------------------
    COptimizer(CPluginObject* p_objectinfo,CProject* p_project);

// setup methods ---------------------------------------------------------------
    /// set optimizer setup
    void SetOptimizerSetup(COptimizerSetup* p_setup);

    /// set optimized molecule
    void SetOptimizedMolecule(CStructure* o_mol);

    /// set number of optimization steps
    void SetMaxOptSteps(int num);

// section of private data -----------------------------------------------------
protected:
    COptimizerSetup*    Setup;
    CStructure*         Structure;
    CLockLevels         BackupLockLevels;
    int                 MaxOptSteps;    // 0 - number from setup
                                        // -1 - infinite
                                        // > 0 - given number of steps

// executive methods -----------------------------------------------------------
    /// initialize job - executed from main thread
    virtual bool InitializeJob(void);

    /// job main execution point - executed from job thread
    virtual bool ExecuteJob(void);

    /// initialize job - executed from main thread
    virtual bool FinalizeJob(void);

// this should be redefined
    /// initialize optimizer for given molecule
    virtual bool InitializationStep(void);

    /// make optimization step
    virtual bool OptimizationStep(void);

    /// finalize optimization
    virtual bool FinalizationStep(void);

    /// get optimization report text
    virtual const QString GetStepDescription(bool final=false);

signals:
    // this signal is used due to thread safety - all GUI is done by master thread
    void OnNotifyTick(const QString& text);

public slots:
    // executed from main thread
    void NotifyTick(const QString& text);
};

//------------------------------------------------------------------------------

#endif
