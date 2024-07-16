// =============================================================================
// NEMESIS - Molecular Modelling Package
// -----------------------------------------------------------------------------
//    Copyright (C) 2024 Petr Kulhanek, kulhanek@chemi.muni.cz
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

#include <QtGui>

#include <PluginObject.hpp>
#include <ProjectList.hpp>
#include <ExtUUID.hpp>
#include <CategoryUUID.hpp>
#include <ErrorSystem.hpp>

#include <StructureList.hpp>
#include <Structure.hpp>
#include <Project.hpp>
#include <PhysicalQuantities.hpp>
#include <PhysicalQuantity.hpp>
#include <AtomList.hpp>
#include <Atom.hpp>
#include <StructureSelection.hpp>
#include <HistoryList.hpp>

#include "ExtraWPModule.hpp"
#include "TotalChargeWorkPanel.hpp"

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

QObject* TotalChargeWorkPanelCB(void* p_data);

CExtUUID        TotalChargeWorkPanelID(
                    "{TOTAL_CHARGE_WP:079979e4-d9f1-4163-909d-2183a1971d21}",
                    "Total Charge");

CPluginObject   TotalChargeWorkPanelObject(&ExtraWPPlugin,
                    TotalChargeWorkPanelID,WORK_PANEL_CAT,
                    ":/images/ExtraWP/TotalChargeWP.svg",
                    TotalChargeWorkPanelCB);

// -----------------------------------------------------------------------------

QObject* TotalChargeWorkPanelCB(void* p_data)
{
    CProject* p_project = static_cast<CProject*>(p_data);
    if( p_project == NULL ){
        ES_ERROR("CTotalChargeWorkPanel requires active project");
        return(NULL);
    }

    QObject* p_build_wp = new CTotalChargeWorkPanel(p_project);
    return(p_build_wp);
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

CTotalChargeWorkPanel::CTotalChargeWorkPanel(CProject* p_project)
    : CWorkPanel(&TotalChargeWorkPanelObject,p_project,EWPR_TOOL)
{
    WidgetUI.setupUi(this);

    // set initial structure
    WidgetUI.structureW->setProject(p_project);
    WidgetUI.structureW->setObjectBaseMIMEType("structure.indexes");
    WidgetUI.structureW->setSelectionHandler(&SH_Structure);
    WidgetUI.structureW->setObject(p_project->GetStructures()->GetActiveStructure());

    WidgetUI.totalChargeLE->setPhysicalQuantity(PQ_CHARGE);

    // signals
    connect(PQ_CHARGE,SIGNAL(OnUnitChanged(void)),
            this,SLOT(ClearTotalCharge(void)));
    // -------------
    connect(WidgetUI.calculatePB,SIGNAL(clicked(bool)),
        this,SLOT(CalculateTotalCharge(void)));
    // -------------
    connect(WidgetUI.removeResidualsPB,SIGNAL(clicked(bool)),
        this,SLOT(RemoveResidules(void)));
    // -------------
    connect(WidgetUI.structureW,SIGNAL(OnObjectChanged(void)),
        this,SLOT(CalculateTotalCharge(void)));
    // -------------
    connect(p_project->GetHistory(),SIGNAL(OnHistoryChanged(EHistoryChangeMessage)),
        this,SLOT(ClearTotalCharge(void)));

    // load work panel setup
    LoadWorkPanelSetup();

    // calc total charge
    CalculateTotalCharge();
}

//------------------------------------------------------------------------------

CTotalChargeWorkPanel::~CTotalChargeWorkPanel()
{
    SaveWorkPanelSetup();
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

void CTotalChargeWorkPanel::CalculateTotalCharge(void)
{
    CStructure* p_str = dynamic_cast<CStructure*>(WidgetUI.structureW->getObject());
    if( p_str != NULL ){
        int i = 0;
        double total_q = 0.0;
        foreach(QObject* p_qobj,p_str->GetAtoms()->children()) {
            CAtom* p_atom = static_cast<CAtom*>(p_qobj);
            total_q += p_atom->GetCharge();
            i++;
        }
        if( i > 0 ){
            WidgetUI.totalChargeLE->setInternalValue(total_q);
        } else {
            WidgetUI.totalChargeLE->setText("");
        }
    } else {
        WidgetUI.totalChargeLE->setText("");
    }
}

//------------------------------------------------------------------------------

void CTotalChargeWorkPanel::ClearTotalCharge(void)
{
    WidgetUI.totalChargeLE->setText("");
}

//------------------------------------------------------------------------------

void CTotalChargeWorkPanel::RemoveResidules(void)
{
    CStructure* p_str = dynamic_cast<CStructure*>(WidgetUI.structureW->getObject());
    if( p_str != NULL ){
        int i = 0;
        double total_q_real = 0.0;
        foreach(QObject* p_qobj,p_str->GetAtoms()->children()) {
            CAtom* p_atom = static_cast<CAtom*>(p_qobj);
            total_q_real += p_atom->GetCharge();
            i++;
        }
        if( i > 0 ){
            CHistoryNode* p_history = p_str->BeginChangeWH(EHCL_GEOMETRY,"remove partial atomic charge residuals");
            if( p_history == NULL ) return;
            double total_q = round(total_q_real);
            double corr = (total_q - total_q_real) / (double)i;
            foreach(QObject* p_qobj,p_str->GetAtoms()->children()) {
                CAtom* p_atom = static_cast<CAtom*>(p_qobj);
                double at_q = p_atom->GetCharge();
                at_q += corr;
                p_atom->SetCharge(at_q,p_history);
            }
            p_str->EndChangeWH();
        }
    }
    CalculateTotalCharge();
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================



