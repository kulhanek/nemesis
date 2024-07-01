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

#include <PluginObject.hpp>
#include <ExtUUID.hpp>
#include <CategoryUUID.hpp>
#include <ErrorSystem.hpp>

#include <PhysicalQuantity.hpp>
#include <PhysicalQuantities.hpp>

#include <openbabel/plugin.h>
#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string.hpp>

#include "OpenBabelModule.hpp"
#include "OpenBabelOptimizerSetup.hpp"
#include "OpenBabelOptimizerSetupDesigner.hpp"

//------------------------------------------------------------------------------

using namespace std;
using namespace OpenBabel;
using namespace boost;
using namespace boost::algorithm;

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

QObject* OpenBabelOptimizerSetupDesignerCB(void* p_data);

CExtUUID        OpenBabelOptimizerSetupDesignerID(
                    "{OPENBABEL_OPTIMIZER_SETUP_DESIGNER:c253d6ef-faef-4857-bdf6-d90ccde7d07a}",
                    "OpenBabel Optimizer");

CPluginObject   OpenBabelOptimizerSetupDesignerObject(&OpenBabelPlugin,
                    OpenBabelOptimizerSetupDesignerID,DESIGNER_CAT,
                    ":/images/OpenBabel/Openbabel.png",
                    OpenBabelOptimizerSetupDesignerCB);

// -----------------------------------------------------------------------------

QObject* OpenBabelOptimizerSetupDesignerCB(void* p_data)
{
    COpenBabelOptimizerSetup* p_fmo = static_cast<COpenBabelOptimizerSetup*>(p_data);
    QObject* p_object = new COpenBabelOptimizerSetupDesigner(p_fmo);
    return(p_object);
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

COpenBabelOptimizerSetupDesigner::COpenBabelOptimizerSetupDesigner(COpenBabelOptimizerSetup* p_fmo)
    : CExtComObjectDesigner(&OpenBabelOptimizerSetupDesignerObject, p_fmo)
{
    Object = p_fmo;
    WidgetUI.setupUi(this);

    // list available force fields ---------------
    vector<string> ffs;
    OBPlugin::ListAsVector("forcefields",NULL,ffs);

    vector<string>::iterator    it = ffs.begin();
    vector<string>::iterator    ie = ffs.end();

    while( it != ie ){
        vector<string> keys;
        split(keys,*it, is_any_of(" "), token_compress_on);
        if( keys.size() > 0 ){
            WidgetUI.forceFieldCB->addItem(keys[0].c_str());
        }
        it++;
    }

    // list available charge methods ---------------
    vector<string> chrgmethods;
    OBPlugin::ListAsVector("charges",NULL,chrgmethods);

    it = chrgmethods.begin();
    ie = chrgmethods.end();

    while( it != ie ){
        vector<string> keys;
        split(keys,*it, is_any_of(" "), token_compress_on);
        it++;
        if( keys.size() > 0 ){
            if( keys[0] == "fromfile" ) continue;
            if( keys[0] == "none" ) continue;
            WidgetUI.chargeMethodCB->addItem(keys[0].c_str());
        }
    }

    // units -------------------------------------
    WidgetUI.eelCutoffSB->setPhysicalQuantity(PQ_DISTANCE);
    WidgetUI.evdwCutoffSB->setPhysicalQuantity(PQ_DISTANCE);

    // connect slots -----------------------------
    connect(WidgetUI.nameLE, SIGNAL(textChanged(const QString&)),
            this,SLOT(SetChangedFlagTrue()));
    // ----------------
    connect(WidgetUI.forceFieldCB, SIGNAL(currentIndexChanged(int)),
            this,SLOT(SetChangedFlagTrue()));
    // ----------------
    connect(WidgetUI.chargeMethodCB, SIGNAL(currentIndexChanged(int)),
            this,SLOT(SetChangedFlagTrue()));
    // ----------------
    connect(WidgetUI.maxStepsSB, SIGNAL(valueChanged(int)),
            this,SLOT(SetChangedFlagTrue()));
    // ----------------
    connect(WidgetUI.stepsPerUpdateSB, SIGNAL(valueChanged(int)),
            this,SLOT(SetChangedFlagTrue()));
    // ----------------
    connect(WidgetUI.termCriteriumSB, SIGNAL(valueChanged(double)),
            this,SLOT(SetChangedFlagTrue()));
    // ----------------
    connect(WidgetUI.nbUpdateSB, SIGNAL(valueChanged(int)),
            this,SLOT(SetChangedFlagTrue()));
    // ----------------
    connect(WidgetUI.eelCutoffSB, SIGNAL(valueChanged(double)),
            this,SLOT(SetChangedFlagTrue()));
    // ----------------
    connect(WidgetUI.evdwCutoffSB, SIGNAL(valueChanged(double)),
            this,SLOT(SetChangedFlagTrue()));
    // ----------------
    connect(Object, SIGNAL(OnSetupChanged(void)),
            this,SLOT(InitValues()));
    // ----------------
    connect(WidgetUI.buttonBox, SIGNAL(clicked(QAbstractButton *)),
            this,SLOT(ButtonBoxClicked(QAbstractButton *)));

    // init all values ---------------------------
    InitValues();
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

void COpenBabelOptimizerSetupDesigner::InitValues(void)
{
    if( IsItChangingContent() == true ) return;

    WidgetUI.nameLE->setText(Object->GetName());

    WidgetUI.forceFieldCB->setCurrentIndex(-1);
    for(int i=0; i < WidgetUI.forceFieldCB->count(); i++ ){
        if( Object->ForceFieldName == WidgetUI.forceFieldCB->itemText(i) ){
            WidgetUI.forceFieldCB->setCurrentIndex(i);
            break;
        }
    }

    WidgetUI.chargeMethodCB->setCurrentIndex(-1);
    for(int i=0; i < WidgetUI.chargeMethodCB->count(); i++ ){
        if( Object->ChargeMethod == WidgetUI.chargeMethodCB->itemText(i) ){
            WidgetUI.chargeMethodCB->setCurrentIndex(i);
            break;
        }
    }
    if( WidgetUI.chargeMethodCB->currentIndex() == -1 ){
        // try again with gasteiger as default
        Object->ChargeMethod = "gasteiger";
        for(int i=0; i < WidgetUI.chargeMethodCB->count(); i++ ){
            if( Object->ChargeMethod == WidgetUI.chargeMethodCB->itemText(i) ){
                WidgetUI.chargeMethodCB->setCurrentIndex(i);
                break;
            }
        }
    }

    WidgetUI.maxStepsSB->setValue(Object->MaxSteps);
    WidgetUI.stepsPerUpdateSB->setValue(Object->StepsPerUpdate);
    WidgetUI.termCriteriumSB->setValue(Object->TermCrit);

    WidgetUI.nbUpdateSB->setValue(Object->NBUpdate);
    WidgetUI.enableCutoffCB->setChecked(Object->EnableCutoff);
    WidgetUI.eelCutoffSB->setInternalValue(Object->vdwCutoff);
    WidgetUI.evdwCutoffSB->setInternalValue(Object->eleCutoff);

    SetChangedFlag(false);
}

//------------------------------------------------------------------------------

void COpenBabelOptimizerSetupDesigner::ApplyValues(void)
{
    if( IsChangedFlagSet() == false ) return;

    Changing = true;

    Object->SetName(WidgetUI.nameLE->text());
    Object->ForceFieldName = WidgetUI.forceFieldCB->currentText();
    Object->ChargeMethod = WidgetUI.chargeMethodCB->currentText();
    Object->MaxSteps = WidgetUI.maxStepsSB->value();
    Object->StepsPerUpdate = WidgetUI.stepsPerUpdateSB->value();
    Object->TermCrit = WidgetUI.termCriteriumSB->value();

    Object->NBUpdate = WidgetUI.nbUpdateSB->value();
    Object->EnableCutoff = WidgetUI.enableCutoffCB->isChecked();
    Object->vdwCutoff = WidgetUI.eelCutoffSB->getInternalValue();
    Object->eleCutoff = WidgetUI.evdwCutoffSB->getInternalValue();

    Changing = false;

    SetChangedFlag(false);
}

//------------------------------------------------------------------------------

void COpenBabelOptimizerSetupDesigner::ButtonBoxClicked(QAbstractButton *button)
{
    if( WidgetUI.buttonBox->standardButton(button) == QDialogButtonBox::Reset ) {
        InitValues();
        return;
    }

    if( WidgetUI.buttonBox->standardButton(button) == QDialogButtonBox::Apply ) {
        ApplyValues();
        return;
    }

    if( WidgetUI.buttonBox->standardButton(button) == QDialogButtonBox::Close ) {
        close();
        return;
    }
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

