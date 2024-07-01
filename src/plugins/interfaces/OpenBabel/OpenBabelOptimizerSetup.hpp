#ifndef OpenBabelOptimizerSetupH
#define OpenBabelOptimizerSetupH
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

#include <OptimizerSetup.hpp>
#include <QString>

//------------------------------------------------------------------------------

enum EOBOptimizationMethod {
    EOBOM_STEEPEST_DESCENT,
    EOBOM_CONJUGATE_GRADIENTS
};

//------------------------------------------------------------------------------

class COpenBabelOptimizerSetup : public COptimizerSetup {
public:
// constructor and destructor -------------------------------------------------
    COpenBabelOptimizerSetup(CExtComObject* p_parent);
    virtual ~COpenBabelOptimizerSetup(void);

// information methods ---------------------------------------------------------
    /// return UUID of associated optimizer
    virtual const CUUID& GetOptimizerUUID(void);

    /// get notify tick interval
    virtual int GetNotifyTickInterval(void);

// main executive method ------------------------------------------------------
    /// load optimizer setup
    virtual void LoadData(CXMLElement* p_ele);

    /// save optimizer setup
    virtual void SaveData(CXMLElement* p_ele);

// section of private data ----------------------------------------------------
private:
    QString                 ForceFieldName;
    QString                 ChargeMethod;
    EOBOptimizationMethod   OptimizationMethod;
    int                     MaxSteps;
    int                     StepsPerUpdate;
    double                  TermCrit;

    // long-range interactions
    int                     NBUpdate;
    bool                    EnableCutoff;
    double                  vdwCutoff;
    double                  eleCutoff;

    friend class COpenBabelOptimizer;
    friend class COpenBabelOptimizerSetupDesigner;
};

//------------------------------------------------------------------------------

#endif
