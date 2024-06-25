// =============================================================================
// NEMESIS - Molecular Modelling Package
// -----------------------------------------------------------------------------
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

#include <QtGui>

#include <CategoryUUID.hpp>
#include <DesktopSystem.hpp>
#include <ProjectList.hpp>
#include <Project.hpp>
#include <ContainerModel.hpp>
#include <ErrorSystem.hpp>
#include <WorkPanelList.hpp>
#include <SelectionList.hpp>

#include "SelectionWorkPanel.hpp"
#include "StandardWPModule.hpp"

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

QObject* SelectionWorkPanelCB(void* p_data);

CExtUUID        SelectionWorkPanelID(
                    "{SELECTION_WORK_PANEL:afec4a64-48c0-4b7e-84d0-e6af678fb3df}",
                    "Selection list");

CPluginObject   SelectionWorkPanelObject(&StandardWPPlugin,
                    SelectionWorkPanelID,WORK_PANEL_CAT,
                    ":/images/StandardWP/SelectionList.svg",
                    SelectionWorkPanelCB);

// -----------------------------------------------------------------------------

QObject* SelectionWorkPanelCB(void* p_data)
{
    CProject* p_project = static_cast<CProject*>(p_data);
    if( p_project == NULL ){
        ES_ERROR("Active project required.");
        return(NULL);
    }

    // only one instance is allowed
    QObject* p_wp = WorkPanels->Find(SelectionWorkPanelID,p_project);
    if( p_wp == NULL ) {
        p_wp = new CSelectionWorkPanel(p_project);
    }

    return(p_wp);
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

CSelectionWorkPanel::CSelectionWorkPanel(CProject* p_project)
    : CWorkPanel(&SelectionWorkPanelObject,p_project,EWPR_TOOL)
{
    Model = NULL;
    Selection = NULL;

    // set up ui file
    WidgetUI.setupUi(this);

    // events
    connect(Projects,SIGNAL(OnProjectNameChanged(CProject*)),
            this,SLOT(ProjectNameChanged(CProject*)));
    //--------------
    connect(WidgetUI.projectPB,SIGNAL(clicked(bool)),
            this,SLOT(OpenProjectDesigner(void)));
    //--------------
    connect(WidgetUI.listView,SIGNAL(doubleClicked(const QModelIndex&)),
            this,SLOT(ListViewDoubleClicked(const QModelIndex&)));

    // set active project
    SetProject(p_project);

    // load work panel setup
    LoadWorkPanelSetup();
}

// -----------------------------------------------------------------------------

CSelectionWorkPanel::~CSelectionWorkPanel(void)
{
    SaveWorkPanelSetup();
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

void CSelectionWorkPanel::WindowDockStatusChanged(bool docked)
{
    if( docked ){
        WidgetUI.projectNameW->hide();
    } else {
        WidgetUI.projectNameW->show();
    }
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

void CSelectionWorkPanel::SetProject(CProject* p_project)
{
    Selection = p_project->GetSelection();
    Model = Selection->GetContainerModel(this);
    connect(Model,SIGNAL(modelReset(void)),
            this,SLOT(UpdateNumberOfSelectedItems(void)));
    UpdateNumberOfSelectedItems();
    WidgetUI.listView->setModel(Model);

    for(int i=0; i < Model->columnCount(); i++) {
        WidgetUI.listView->resizeColumnToContents(i);
    }

    connect(Selection,SIGNAL(OnSelectionRequestChanged()),
            this,SLOT(SelectionRequestChanged()));

    ProjectNameChanged(p_project);
    SelectionRequestChanged();
}

//------------------------------------------------------------------------------

void CSelectionWorkPanel::ProjectNameChanged(CProject* p_project)
{
    if( p_project != GetProject() ) return;
    WidgetUI.projectPB->setText(p_project->GetName());
}

//------------------------------------------------------------------------------

void CSelectionWorkPanel::SelectionRequestChanged(void)
{
    WidgetUI.requestLE->setText(Selection->GetRequestTypeString());
    WidgetUI.requestLE->setCursorPosition(0);
    WidgetUI.requestLE->setToolTip(Selection->GetRequestTypeString());

    WidgetUI.reasonLE->setText(Selection->GetRequestReason());
    WidgetUI.reasonLE->setCursorPosition(0);
    WidgetUI.reasonLE->setToolTip(Selection->GetRequestReason());

    WidgetUI.requestorLE->setText(Selection->GetRequestorName());
    WidgetUI.requestorLE->setCursorPosition(0);
    WidgetUI.requestorLE->setToolTip(Selection->GetRequestorName());
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

void CSelectionWorkPanel::OpenProjectDesigner(void)
{
    if( Selection == NULL ) return;
    CProject* p_proj = Selection->GetProject();
    p_proj->OpenObjectDesigner();
}

//------------------------------------------------------------------------------

void CSelectionWorkPanel::ListViewDoubleClicked(const QModelIndex& index)
{
    if( Model == NULL ) return;

    QObject* p_obj = Model->GetItem(index);
    if( p_obj == NULL ) return;

    // is it project object?
    CProObject* p_proobj = dynamic_cast<CProObject*>(p_obj);
    if( p_proobj != NULL ) {
        p_proobj->OpenObjectDesigner();
    }
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

void CSelectionWorkPanel::UpdateNumberOfSelectedItems(void)
{
    QString text = tr("Number of selected items: %1");
    int num = 0;
    if( Model != NULL ) {
        num = Model->rowCount();
    }
    text = text.arg(num);
    WidgetUI.labelNumOfItems->setText(text);
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================




