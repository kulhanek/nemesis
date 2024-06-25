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

#include "OpenBabelOptimizer.hpp"
#include "OpenBabelOptimizerSetup.hpp"
#include "OpenBabelModule.hpp"
#include <OpenBabelUtils.hpp>

#include <CategoryUUID.hpp>
#include <ErrorSystem.hpp>
#include <Structure.hpp>
#include <RestraintList.hpp>
#include <Restraint.hpp>
#include <Project.hpp>
#include <AtomList.hpp>
#include <Atom.hpp>
#include <PhysicalQuantities.hpp>
#include <PhysicalQuantity.hpp>

#include "openbabel/obconversion.h"

using namespace std;
using namespace OpenBabel;

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

QObject* OpenBabelOptimizerCB(void* p_data);

CExtUUID        OpenBabelOptimizerID(
                    "{OPEN_BABEL_OPTIMIZER:1703e001-988a-4c9d-a6d8-ea7559ea4661}",
                    "OpenBabel Optimizer");

CPluginObject   OpenBabelOptimizerObject(&OpenBabelPlugin,
                    OpenBabelOptimizerID,OPTIMIZER_CAT,
                    ":/images/OpenBabel/Openbabel.png",
                    OpenBabelOptimizerCB);

// -----------------------------------------------------------------------------

QObject* OpenBabelOptimizerCB(void* p_data)
{
    CProject* p_proj = static_cast<CProject*>(p_data);
    QObject* p_object = new COpenBabelOptimizer(p_proj);
    return(p_object);
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

COpenBabelOptimizer::COpenBabelOptimizer(CProject* p_project)
    : COptimizer(&OpenBabelOptimizerObject,p_project)
{
    OBForceField = NULL;
}

//------------------------------------------------------------------------------

COpenBabelOptimizer::~COpenBabelOptimizer(void)
{
    //TODO: DOES NOT WORK - WHY?
    //if( OBForceField != NULL ) delete OBForceField;
    OBForceField = NULL;
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

bool COpenBabelOptimizer::InitializationStep(void)
{
    if( Structure->GetAtoms()->children().count() < 2 ) {
        Structure->GetProject()->TextNotification(ETNT_ERROR,"only two and more atoms can be optimized",ETNT_ERROR_DELAY);
        return(false);
    }

    // convert to open babel ------------------------
    COpenBabelUtils::Nemesis2OpenBabel(Structure,OBMol);

    // create force field ---------------------------
    OpenBabel::OBForceField* p_global = OBForceField::FindForceField(CSmallString(GetSetup()->ForceFieldName));
    if( p_global == NULL ) {
        CSmallString error;
        error = "unable to open force field file named: ";
        error += CSmallString(GetSetup()->ForceFieldName);
        error += ".";
        ES_ERROR(error);
        return(false);
    }
    OBForceField = p_global->MakeNewInstance();
    if( OBForceField == NULL ) {
        ES_ERROR("unable to duplicate field file");
        return(false);
    }

    // set some force field variables
    OBForceField->SetLogFile(NULL);
    OBForceField->SetLogLevel(OBFF_LOGLVL_NONE);

    OBForceField->SetUpdateFrequency(GetSetup()->NBUpdate);
    OBForceField->EnableCutOff(GetSetup()->EnableCutoff);
    OBForceField->SetVDWCutOff(GetSetup()->vdwCutoff);
    OBForceField->SetElectrostaticCutOff(GetSetup()->eleCutoff);


    // prepare constraints
    OBFFConstraints con;

    // fix requested atoms
    int i = 1;
    foreach(QObject* p_qobj,Structure->GetAtoms()->children()) {
        CAtom* p_atom = static_cast<CAtom*>(p_qobj);
        if( p_atom->IsFlagSet(EPOF_FREEZED) ){
            con.AddAtomConstraint(i);
        }
        i++;
    }

    if( OBForceField->Setup(OBMol,con) == false) {
        ES_ERROR("unable to setup molecule");
        return(false);
    }

    // prepare restraints
    RestraintGradients.CreateVector(Structure->GetAtoms()->GetNumberOfAtoms() * 3);
    // create map hash
    int index = 0;
    foreach(QObject* p_qobj, Structure->GetAtoms()->children()){
        CAtom* p_atom = static_cast<CAtom*>(p_qobj);
        AtomMap[p_atom] = index;
        index++;
    }

    int maxstep = GetSetup()->MaxSteps;
    if( MaxOptSteps > 0 ){
        maxstep = MaxOptSteps;
    }
    if( MaxOptSteps < 0 ){
        maxstep = -1;
    }

    // optimize
    switch(GetSetup()->OptimizationMethod) {
    case EOBOM_STEEPEST_DESCENT:
        OBForceField->SteepestDescentInitializeNemesis(maxstep,GetSetup()->TermCrit, OBFF_ANALYTICAL_GRADIENT);
        break;
    case EOBOM_CONJUGATE_GRADIENTS:
        OBForceField->ConjugateGradientsInitializeNemesis(maxstep,GetSetup()->TermCrit, OBFF_ANALYTICAL_GRADIENT);
        break;
    }

    Step = 0;

    return(true);
}

//------------------------------------------------------------------------------

bool COpenBabelOptimizer::OptimizationStep(void)
{
    Step++;

    bool done;

    // prepare restraints
    RestraintEnergy = Structure->GetRestraints()->GetEnergy(RestraintGradients,AtomMap);

    switch(GetSetup()->OptimizationMethod) {
    case EOBOM_STEEPEST_DESCENT:
        done = OBForceField->SteepestDescentNemesis(RestraintGradients.GetRawDataField(),
                                                       RestraintEnergy);
        break;
    case EOBOM_CONJUGATE_GRADIENTS:
        done = OBForceField->ConjugateGradientsNemesis(RestraintGradients.GetRawDataField(),
                                                          RestraintEnergy);
        break;
    }

    if( OBForceField->DetectExplosion() ) return(false);

    OBForceField->GetCoordinates(OBMol);

    COpenBabelUtils::OpenBabelPos2NemesisPos(OBMol,Structure);

    return(done);
}

//------------------------------------------------------------------------------

bool COpenBabelOptimizer::FinalizationStep(void)
{
    OBForceField->GetCoordinates(OBMol);

    COpenBabelUtils::OpenBabelPos2NemesisPos(OBMol,Structure);
    return(true);
}

//------------------------------------------------------------------------------

const QString COpenBabelOptimizer::GetStepDescription(bool final)
{
    QString text;

    if( ! final ){
        text  = tr("Step: ");
        text += QString().setNum(Step);
    }

    if( final ){
        text  += tr("Final E(");
    } else {
        text  += tr(" E(");
    }
    text += GetSetup()->ForceFieldName + QString(")=");

    if( OBForceField ){
        text += PQ_ENERGY->GetRealValueText(OBForceField->GetLastEnergy());
    } else {
        text += tr("n.d.");
    }
    text += QString(" " ) + PQ_ENERGY->GetUnitName();

    if( final ){
        text += tr("; Number of steps: ");
        if( OBForceField ){
            text += QString().setNum(Step);
        } else {
            text += QString().setNum(0);
        }
    }

    return(text);
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

COpenBabelOptimizerSetup* COpenBabelOptimizer::GetSetup(void)
{
    return(static_cast<COpenBabelOptimizerSetup*>(Setup));
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================



