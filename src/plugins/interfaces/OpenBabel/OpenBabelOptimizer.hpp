#ifndef OpenBabelOptimizerH
#define OpenBabelOptimizerH
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

#include <Optimizer.hpp>
#include <openbabel/mol.h>
#include <openbabel/forcefield.h>
#include <openbabel/chargemodel.h>
#include <Vector.hpp>
#include <QHash>
#include <Atom.hpp>

//------------------------------------------------------------------------------

class COpenBabelOptimizerSetup;

//------------------------------------------------------------------------------

class COpenBabelOptimizer : public COptimizer {
public:
// constructor and destructor -------------------------------------------------
    COpenBabelOptimizer(CProject* p_project);
    virtual ~COpenBabelOptimizer(void);

// main executive method ------------------------------------------------------
    /// initialize optimizer for given molecule
    virtual bool InitializationStep(void);

    /// make optimization step
    virtual bool OptimizationStep(void);

    /// finalize optimization
    virtual bool FinalizationStep(void);

    /// get step description
    virtual const QString GetStepDescription(bool final);

// section of private data ----------------------------------------------------
private:
    OpenBabel::OBMol            OBMol;
    OpenBabel::OBForceField*    OBForceField;
    OpenBabel::OBChargeModel*   ChargeMethod;
    int                         Step;
    double                      RestraintEnergy;
    CVector                     RestraintGradients;
    QHash<CAtom*,int>           AtomMap;
    double                      TotalCharge;

    /// get optimizer setup
    COpenBabelOptimizerSetup*   GetSetup(void);
};

//------------------------------------------------------------------------------

extern CExtUUID        OpenBabelOptimizerID;

//------------------------------------------------------------------------------

#endif
