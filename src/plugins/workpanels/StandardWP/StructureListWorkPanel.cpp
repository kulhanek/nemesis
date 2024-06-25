// =============================================================================
// NEMESIS - Molecular Modelling Package
// -----------------------------------------------------------------------------
//    Copyright (C) 2024 Petr Kulhanek, kulhanek@chemi.muni.cz
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

#include <QtGui>

#include <StructureList.hpp>
#include <PluginObject.hpp>
#include <ExtUUID.hpp>
#include <CategoryUUID.hpp>
#include <Project.hpp>
#include <ProjectList.hpp>
#include <StructureList.hpp>
#include <WorkPanelList.hpp>
#include <ContainerModel.hpp>
#include <ErrorSystem.hpp>

#include <PhysicalQuantities.hpp>
#include <PhysicalQuantity.hpp>

#include <Graphics.hpp>
#include <GraphicsView.hpp>

#include "StandardWPModule.hpp"
#include "StructureListWorkPanel.hpp"

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

QObject* StructureListWorkPanelCB(void* p_data);

CExtUUID        StructureListWorkPanelID(
                    "{STRUCTURE_LIST_WORK_PANEL:a48ddecd-4a95-4dfe-96e1-dd2de8a0fd5e}",
                    "Manage Structures");

CPluginObject   StructureListWorkPanelObject(&StandardWPPlugin,
                    StructureListWorkPanelID,WORK_PANEL_CAT,
                    ":/images/StandardWP/StructureManageStructures.svg",
                    StructureListWorkPanelCB);

// -----------------------------------------------------------------------------

QObject* StructureListWorkPanelCB(void* p_data)
{
    CProject* p_project = static_cast<CProject*>(p_data);
    if( p_project == NULL ){
        ES_ERROR("CStructureListWorkPanel requires active project");
        return(NULL);
    }

    // only one instance is allowed
    QObject* p_wp = WorkPanels->Find(StructureListWorkPanelID,p_project);
    if( p_wp == NULL ) {
        p_wp = new CStructureListWorkPanel(p_project);
    }

    return(p_wp);
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

CStructureListWorkPanel::CStructureListWorkPanel(CProject* p_owner)
    : CWorkPanel(&StructureListWorkPanelObject,p_owner,EWPR_TOOL)
{   
    List = p_owner->GetStructures();

    WidgetUI.setupUi(this);

    // init model --------------------------------
    ObjectsModel = List->GetContainerModel(this);
    WidgetUI.objectsTV->setModel(ObjectsModel);

    WidgetUI.objectsTV->setDragDropMode(QAbstractItemView::DragOnly);
    WidgetUI.objectsTV->setDragEnabled(true);

    if( ObjectsModel != NULL ){
        for(int i=0; i < ObjectsModel->columnCount(); i++){
            WidgetUI.objectsTV->resizeColumnToContents(i);
        }
        // connect signals ---------------------------
        connect(ObjectsModel,SIGNAL(modelReset()),
                this,SLOT(ObjectsTVSelectionChanged(void)));
    }

    MergeObjectsModel = List->GetContainerModel(this);
    WidgetUI.mergeTV->setModel(MergeObjectsModel);

    WidgetUI.mergeTV->setDragDropMode(QAbstractItemView::DragOnly);
    WidgetUI.mergeTV->setDragEnabled(true);

    if( MergeObjectsModel != NULL ){
        for(int i=0; i < MergeObjectsModel->columnCount(); i++){
            WidgetUI.mergeTV->resizeColumnToContents(i);
        }
        // connect signals ---------------------------
        connect(MergeObjectsModel,SIGNAL(modelReset()),
                this,SLOT(MergeTVSelectionChanged(void)));
    }

    // suppport for trajectories
    if( GetProject()->GetTrajectories() ){
        WidgetUI.tabWidget->removeTab(1);
        WidgetUI.tabWidget->removeTab(1); // indexes are shifted
    }

    // signals -----------------------------------
    connect(Projects,SIGNAL(OnProjectNameChanged(CProject*)),
            this,SLOT(ProjectNameChanged(CProject*)));
    //--------------
    connect(WidgetUI.projectPB,SIGNAL(clicked(bool)),
            this,SLOT(OpenProjectDesigner(void)));

    //------------------
    connect(WidgetUI.objectsTV->selectionModel(), SIGNAL(selectionChanged(const QItemSelection&, const QItemSelection&)),
            this,SLOT(ObjectsTVSelectionChanged(void)));
    //------------------
    connect(WidgetUI.objectsTV,SIGNAL(doubleClicked(const QModelIndex&)),
            this,SLOT(ObjectsTVDblClicked(const QModelIndex&)));
    //------------------
    connect(WidgetUI.newStructurePB,SIGNAL(clicked(bool)),
            this,SLOT(NewStructure(void)));
    //------------------
    connect(WidgetUI.activateStructurePB,SIGNAL(clicked(bool)),
            this,SLOT(ActivateStructure(void)));
    //------------------
    connect(WidgetUI.removeStructurePB,SIGNAL(clicked(bool)),
            this,SLOT(RemoveStructure(void)));
    //------------------
    connect(WidgetUI.duplicateStructurePB,SIGNAL(clicked(bool)),
            this,SLOT(DuplicateStructure(void)));

    WidgetUI.duplicateOffsetSB->setPhysicalQuantity(PQ_DISTANCE);
    WidgetUI.duplicateMoveXCB->setCheckState(Qt::Unchecked);
    WidgetUI.duplicateMoveYCB->setCheckState(Qt::Checked);
    WidgetUI.duplicateMoveZCB->setCheckState(Qt::Unchecked);
    WidgetUI.duplicateOffsetSB->setInternalValue(1.5);

    //------------------
    connect(WidgetUI.infoStructurePB,SIGNAL(clicked(bool)),
            this,SLOT(StructureInfo(void)));

    //------------------
    connect(WidgetUI.deleteSelectedStructuresPB,SIGNAL(clicked(bool)),
            this,SLOT(DeleteEmptyStructures(void)));
    //------------------
    connect(WidgetUI.deleteEmptyStructuresPB,SIGNAL(clicked(bool)),
            this,SLOT(DeleteSelectedStructures(void)));
    //------------------
    connect(WidgetUI.deleteAllStructuresPB,SIGNAL(clicked(bool)),
            this,SLOT(DeleteAllStructures(void)));

    //------------------
    connect(WidgetUI.newFromSelectedResiduesPB,SIGNAL(clicked(bool)),
            this,SLOT(NewFromSelectedResidues(void)));
    //------------------
    connect(WidgetUI.deactivateActiveStructurePB,SIGNAL(clicked(bool)),
            this,SLOT(DeactivateActiveStructure(void)));
    //------------------
    connect(WidgetUI.seqIndexByOrderPB,SIGNAL(clicked(bool)),
            this,SLOT(SeqIndexByOrder(void)));
    //------------------
    connect(WidgetUI.sortStructuresPB,SIGNAL(clicked(bool)),
            this,SLOT(SortStructures(void)));

    //------------------
    connect(WidgetUI.mergeTV->selectionModel(), SIGNAL(selectionChanged(const QItemSelection&, const QItemSelection&)),
            this,SLOT(MergeTVSelectionChanged(void)));
    //------------------
    connect(WidgetUI.mergeTV,SIGNAL(doubleClicked(const QModelIndex&)),
            this,SLOT(MergeTVDblClicked(const QModelIndex&)));
    //------------------
    connect(WidgetUI.mergeAllStructuresPB,SIGNAL(clicked(bool)),
            this,SLOT(MergeAllStructures(void)));
    //------------------
    connect(WidgetUI.mergeSelectedStructuresPB,SIGNAL(clicked(bool)),
            this,SLOT(MergeSelectedStructures(void)));

    //--------------
    connect(GetProject()->GetHistory(),SIGNAL(OnHistoryChanged(EHistoryChangeMessage)),
            this,SLOT(ProjectLockChanged(EHistoryChangeMessage)));

    // finalization
    ProjectLockChanged(EHCM_LOCK_LEVEL);
    ObjectsTVSelectionChanged();
    ProjectNameChanged(GetProject());
    LoadWorkPanelSetup();
}

//------------------------------------------------------------------------------

CStructureListWorkPanel::~CStructureListWorkPanel(void)
{
    SaveWorkPanelSetup();
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

void CStructureListWorkPanel::LoadWorkPanelSpecificData(CXMLElement* p_ele)
{
    if( p_ele == NULL ){
        INVALID_ARGUMENT("p_ele == NULL");
    }

    bool bdummy = false;
    if( p_ele->GetAttribute("dup_move_all",bdummy) ){
        WidgetUI.duplicateMoveToAllCB->setChecked(bdummy);
    }
    if( p_ele->GetAttribute("dup_move_x",bdummy) ){
        WidgetUI.duplicateMoveXCB->setChecked(bdummy);
    }

    if( p_ele->GetAttribute("dup_move_y",bdummy) ){
        WidgetUI.duplicateMoveYCB->setChecked(bdummy);
    }
    if( p_ele->GetAttribute("dup_move_z",bdummy) ){
        WidgetUI.duplicateMoveZCB->setChecked(bdummy);
    }
    if( p_ele->GetAttribute("dup_autofit",bdummy) ){
        WidgetUI.autofitSceneCB->setChecked(bdummy);
    }

    double ddummy = 0.0;
    if( p_ele->GetAttribute("dup_offset",ddummy) ){
        WidgetUI.duplicateOffsetSB->setInternalValue(ddummy);
    }

    CWorkPanel::LoadWorkPanelSpecificData(p_ele);
}

//------------------------------------------------------------------------------

void CStructureListWorkPanel::SaveWorkPanelSpecificData(CXMLElement* p_ele)
{
    if( p_ele == NULL ){
        INVALID_ARGUMENT("p_ele == NULL");
    }

    p_ele->SetAttribute("dup_move_all",WidgetUI.duplicateMoveToAllCB->isChecked());
    p_ele->SetAttribute("dup_move_x",WidgetUI.duplicateMoveXCB->isChecked());
    p_ele->SetAttribute("dup_move_y",WidgetUI.duplicateMoveYCB->isChecked());
    p_ele->SetAttribute("dup_move_z",WidgetUI.duplicateMoveZCB->isChecked());
    p_ele->SetAttribute("dup_autofit",WidgetUI.autofitSceneCB->isChecked());
    p_ele->SetAttribute("dup_offset",WidgetUI.duplicateOffsetSB->getInternalValue());

    CWorkPanel::SaveWorkPanelSpecificData(p_ele);
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

void CStructureListWorkPanel::ProjectNameChanged(CProject* p_project)
{
    if( p_project != GetProject() ) return;
    WidgetUI.projectPB->setText(p_project->GetName());
}

//------------------------------------------------------------------------------

void CStructureListWorkPanel::OpenProjectDesigner(void)
{
    if( GetProject() == NULL ) return;
    GetProject()->OpenObjectDesigner();
}

//------------------------------------------------------------------------------

void CStructureListWorkPanel::WindowDockStatusChanged(bool docked)
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

void CStructureListWorkPanel::ObjectsTVSelectionChanged(void)
{
    QModelIndexList selected_rows = WidgetUI.objectsTV->selectionModel()->selectedRows();

    WidgetUI.activateStructurePB->setEnabled(selected_rows.count() == 1);
    WidgetUI.removeStructurePB->setEnabled(selected_rows.count() > 0);
    WidgetUI.duplicateStructurePB->setEnabled(selected_rows.count() == 1);
    WidgetUI.infoStructurePB->setEnabled(selected_rows.count() > 0);

    int num = 0;
    if( ObjectsModel != NULL ) {
        num = ObjectsModel->rowCount();
    }
    QString text("%1");
    text = text.arg(num);
    WidgetUI.numOfObjectsLE->setText(text);
}

//------------------------------------------------------------------------------

void CStructureListWorkPanel::ObjectsTVDblClicked(const QModelIndex& index)
{
    CProObject* p_obj = dynamic_cast<CProObject*>(ObjectsModel->GetItem(index));
    if( p_obj == NULL ) return;
    p_obj->OpenObjectDesigner();
}

//------------------------------------------------------------------------------

void CStructureListWorkPanel::NewStructure(void)
{
    List->CreateStructureWH(true);
}

//------------------------------------------------------------------------------

void CStructureListWorkPanel::ActivateStructure(void)
{
    QModelIndexList selected_rows = WidgetUI.objectsTV->selectionModel()->selectedRows();
    if(selected_rows.count() != 1 ) return;
    CStructure* p_str = dynamic_cast<CStructure*>(ObjectsModel->GetItem(selected_rows[0]));
    if( p_str == NULL ) return;
    List->SetActiveStructureWH(p_str);
}

//------------------------------------------------------------------------------

void CStructureListWorkPanel::RemoveStructure(void)
{
    QModelIndexList selected_rows = WidgetUI.objectsTV->selectionModel()->selectedRows();
    if( selected_rows.count() > 1 ){
        CHistoryNode* p_history = List->BeginChangeWH(EHCL_TOPOLOGY,"delete structures");
        if( p_history == NULL ) return;
        for(int i=0; i < selected_rows.count(); i++){
            CStructure* p_str = dynamic_cast<CStructure*>(ObjectsModel->GetItem(selected_rows[i]));
            if( p_str != NULL ){
                p_str->RemoveFromBaseList(p_history);
            }
        }
        List->EndChangeWH();
    } else {
        CStructure* p_str = dynamic_cast<CStructure*>(ObjectsModel->GetItem(selected_rows[0]));
        if( p_str == NULL ) return;
        p_str->DeleteWH();
    }
}

//------------------------------------------------------------------------------

void CStructureListWorkPanel::DuplicateStructure(void)
{
    QModelIndexList selected_rows = WidgetUI.objectsTV->selectionModel()->selectedRows();
    if(selected_rows.count() != 1 ) return;
    CStructure* p_str = dynamic_cast<CStructure*>(ObjectsModel->GetItem(selected_rows[0]));
    if( p_str == NULL ) return;

    bool move_to_all = WidgetUI.duplicateMoveToAllCB->checkState() == Qt::Checked;
    bool move_x = WidgetUI.duplicateMoveXCB->checkState() == Qt::Checked;
    bool move_y = WidgetUI.duplicateMoveYCB->checkState() == Qt::Checked;
    bool move_z = WidgetUI.duplicateMoveZCB->checkState() == Qt::Checked;
    double offset = WidgetUI.duplicateOffsetSB->getInternalValue();

    List->DuplicateStructureWH(p_str,move_to_all,move_x,move_y,move_z,offset);

    if( WidgetUI.autofitSceneCB->checkState() == Qt::Checked ){
        CGraphicsView* p_view = GetProject()->GetGraphics()->GetPrimaryView();
        if( p_view ){
            p_view->FitScene();
        }
    }
}

//------------------------------------------------------------------------------

void CStructureListWorkPanel::StructureInfo(void)
{
    QModelIndexList selected_rows = WidgetUI.objectsTV->selectionModel()->selectedRows();
    for(int i=0; i < selected_rows.count(); i++){
        CStructure* p_str = dynamic_cast<CStructure*>(ObjectsModel->GetItem(selected_rows[i]));
        if( p_str != NULL ){
            p_str->OpenObjectDesigner();
        }
    }
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

void CStructureListWorkPanel::DeleteEmptyStructures(void)
{
    List->DeleteEmptyStructuresWH();
}

//------------------------------------------------------------------------------

void CStructureListWorkPanel::DeleteSelectedStructures(void)
{
    List->DeleteSelectedStructuresWH();
}

//------------------------------------------------------------------------------

void CStructureListWorkPanel::DeleteAllStructures(void)
{
    List->DeleteAllStructuresWH();
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

void CStructureListWorkPanel::NewFromSelectedResidues(void)
{
    List->CreateStructureFromSelectedResiduesWH();
}

//------------------------------------------------------------------------------

void CStructureListWorkPanel::DeactivateActiveStructure(void)
{
    List->SetActiveStructureWH(NULL);
}

//------------------------------------------------------------------------------

void CStructureListWorkPanel::SeqIndexByOrder(void)
{
    List->SequenceIndexByOrderWH();
}

//------------------------------------------------------------------------------

void CStructureListWorkPanel::SortStructures(void)
{
    List->SortStructures();
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

void CStructureListWorkPanel::MergeTVSelectionChanged(void)
{
    int num = 0;
    if( MergeObjectsModel != NULL ) {
        num = MergeObjectsModel->rowCount();
    }
    WidgetUI.mergeAllStructuresPB->setEnabled(num > 1);

    QModelIndexList selected_rows = WidgetUI.mergeTV->selectionModel()->selectedRows();
    WidgetUI.mergeSelectedStructuresPB->setEnabled(selected_rows.count() > 1);
}

//------------------------------------------------------------------------------

void CStructureListWorkPanel::MergeTVDblClicked(const QModelIndex& index)
{
    CProObject* p_obj = dynamic_cast<CProObject*>(MergeObjectsModel->GetItem(index));
    if( p_obj == NULL ) return;
    p_obj->OpenObjectDesigner();
}

//------------------------------------------------------------------------------

void CStructureListWorkPanel::MergeAllStructures(void)
{
    List->MergeAllStructuresWH();
}

//------------------------------------------------------------------------------

void CStructureListWorkPanel::MergeSelectedStructures(void)
{
    QModelIndexList selected_rows = WidgetUI.mergeTV->selectionModel()->selectedRows();
    if( selected_rows.count() <= 1 ) return;

    QList<CStructure*> structures;
    for(int i=0; i < selected_rows.count(); i++){
        CStructure* p_str = dynamic_cast<CStructure*>(MergeObjectsModel->GetItem(selected_rows[i]));
        if( p_str != NULL ){
            structures.push_back(p_str);
        }
    }

    List->MergeStructuresWH(structures);
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

void CStructureListWorkPanel::ProjectLockChanged(EHistoryChangeMessage message)
{
    if( message != EHCM_LOCK_LEVEL ) return;
    bool set1,set2;
    set1 = ! List->IsHistoryLocked(EHCL_STRUCTURES);
    set2 = ! List->IsHistoryLocked(EHCL_TOPOLOGY);

    WidgetUI.tools1W->setEnabled(set1);
    WidgetUI.toolsW2->setEnabled(set2);
    WidgetUI.deleteTab->setEnabled(set1);
    WidgetUI.miscTab->setEnabled(set1);
    WidgetUI.newFromSelectedResiduesPB->setEnabled(set2);
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================










