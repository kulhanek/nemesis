// =============================================================================
// NEMESIS - Molecular Modelling Package
// -----------------------------------------------------------------------------
//    Copyright (C) 2011 Petr Kulhanek, kulhanek@chemi.muni.cz
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

#include <Structure.hpp>
#include <Project.hpp>
#include <AtomList.hpp>
#include <ResidueList.hpp>
#include <ContainerModel.hpp>
#include <SelectionRequest.hpp>
#include <SelectionList.hpp>
#include <StructureSelection.hpp>
#include <Atom.hpp>
#include <Residue.hpp>
#include <PhysicalQuantities.hpp>
#include <PhysicalQuantity.hpp>
#include <PluginDatabase.hpp>

#include "ExtraWPModule.hpp"
#include "ResidueListWorkPanel.hpp"

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

QObject* ResidueListWorkPanelCB(void* p_data);

CExtUUID        ResidueListWorkPanelID(
                    "{RESIDUE_LIST_WORK_PANEL:9a26a97a-7ac0-4d64-bbc7-8aa627b5d6b4}",
                    "Edit Residues");

CPluginObject   ResidueListWorkPanelObject(&ExtraWPPlugin,
                    ResidueListWorkPanelID,WORK_PANEL_CAT,
                    ":/images/ExtraWP/ResidueListEditor.svg",
                    ResidueListWorkPanelCB);

// -----------------------------------------------------------------------------

QObject* ResidueListWorkPanelCB(void* p_data)
{
    CProObject* p_obj = static_cast<CProObject*>(p_data);
    if( p_obj == NULL ){
        ES_ERROR("p_obj == NULL");
        return(NULL);
    }
    CStructure* p_str = dynamic_cast<CStructure*>(p_obj);
    CProject* p_pro;
    if( p_str ){
        p_pro = p_str->GetProject();
    } else {
        p_pro = dynamic_cast<CProject*>(p_obj);
        p_str = p_pro->GetActiveStructure();
    }
    if( p_pro == NULL ){
        ES_ERROR("p_pro == NULL");
        return(NULL);
    }
    QObject* p_object = new CResidueListWorkPanel(p_pro,p_str);
    return(p_object);
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

CResidueListWorkPanel::CResidueListWorkPanel(CProject* p_pro,CStructure* p_str)
    : CWorkPanel(&ResidueListWorkPanelObject,p_pro,EWPR_TOOL)
{
    WidgetUI.setupUi(this);

    ResiduesModel = NULL;
    AtomsModel = NULL;

    WidgetUI.buildTab->setEnabled(false);
    WidgetUI.orderTab->setEnabled(false);
    WidgetUI.chainsTab->setEnabled(false);
    WidgetUI.showTab->setEnabled(false);

    // extra setup -------------------------------
    WidgetUI.structureW->setProject(GetProject());
    WidgetUI.structureW->setObjectBaseMIMEType("structure.indexes");
    WidgetUI.structureW->setSelectionHandler(&SH_Structure);

    // edit table header
    QStringList header_captions;
    header_captions << tr("Order") << tr("Sequence index") << tr("Name") << tr("Chain") << tr("Type");
    WidgetUI.editTW->setColumnCount(5);
    WidgetUI.editTW->setHorizontalHeaderLabels(header_captions);

    for(int i=0; i < 5; i++){
        WidgetUI.editTW->resizeColumnToContents(i);
    }

    UpdatingItems = false;

    // local events ------------------------------
    connect(WidgetUI.structureW,SIGNAL(OnObjectChanged()),
            this,SLOT(StructureChanged()));
    connect(WidgetUI.editTW, SIGNAL(itemChanged(QTableWidgetItem*)),
            this, SLOT(EditTWChanged(QTableWidgetItem*)));
    connect(WidgetUI.editTW, SIGNAL(itemSelectionChanged(void)),
            this, SLOT(ItemSelectionChanged(void)));

    connect(WidgetUI.residuesTV, SIGNAL(doubleClicked(const QModelIndex&)),
            this, SLOT(ListModelTVDblClicked(const QModelIndex&)));
    connect(WidgetUI.atomsTV, SIGNAL(doubleClicked(const QModelIndex&)),
            this, SLOT(ListModelTVDblClicked(const QModelIndex&)));

    // order -------------------------------------
    connect(WidgetUI.seqIndxByOrderPB,SIGNAL(clicked(bool)),
            this, SLOT(SeqIndxByOrder(void)));
    connect(WidgetUI.seqIndxByReverseOrderPB,SIGNAL(clicked(bool)),
            this, SLOT(SeqIndxByReverseOrder(void)));
    connect(WidgetUI.seqIndxByChainsPB,SIGNAL(clicked(bool)),
            this, SLOT(SeqIndxByChains(void)));
    connect(WidgetUI.reorderBySeqIndxPB,SIGNAL(clicked(bool)),
            this, SLOT(SortResidues(void)));

    // builds
    connect(WidgetUI.editResiduesPB,SIGNAL(clicked(bool)),
            this, SLOT(EditResidues(void)));
    connect(WidgetUI.deleteResiduesPB,SIGNAL(clicked(bool)),
            this, SLOT(DeleteResidues(void)));
    connect(WidgetUI.deleteEmptyStructuresPB,SIGNAL(clicked(bool)),
            this, SLOT(DeleteEmptyStructures(void)));
    connect(WidgetUI.mergeResiduesPB,SIGNAL(clicked(bool)),
            this, SLOT(MergeResidues(void)));

    // show
    connect(WidgetUI.highlightResiduesCB,SIGNAL(clicked(bool)),
            this,SLOT(UpdateHighlightedResidues()));

    // atoms
    connect(WidgetUI.serIndxBySeqLocPB,SIGNAL(clicked(bool)),
            this,SLOT(SerialIndexBySeqAndLocIndexes()));

    // chains
    connect(WidgetUI.setChainNamesPB,SIGNAL(clicked(bool)),
            this,SLOT(SetChainNames()));

    if( GetProject() ){
        connect(GetProject()->GetHistory(), SIGNAL(OnHistoryChanged(EHistoryChangeMessage)),
                this, SLOT(ProjectLockChanged(EHistoryChangeMessage)));
    }

    // eventually attach Structure
    Structure = NULL;
    WidgetUI.structureW->setObject(p_str);

    // load work panel setup
    LoadWorkPanelSetup();
}

//------------------------------------------------------------------------------

CResidueListWorkPanel::~CResidueListWorkPanel(void)
{
    if( Structure ){
        if( WidgetUI.highlightResiduesCB->isChecked() ){
            WidgetUI.highlightResiduesCB->setChecked(false);
            UpdateHighlightedResidues();
        }
    }
    SaveWorkPanelSetup();
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

void CResidueListWorkPanel::StructureChanged(void)
{
    Structure = WidgetUI.structureW->getObject<CStructure*>();
    if( Structure ){
        StructureRegistered();
    } else {
        StructureUnregistered();
    }
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

void CResidueListWorkPanel::StructureRegistered(void)
{
    // connect signals
    connect(Structure->GetResidues(),SIGNAL(OnResidueListChanged(void)),
            this,SLOT(UpdateEditTable()));

    // create residues model
    ResiduesModel = Structure->GetResidues()->GetContainerModel(this);
    WidgetUI.residuesTV->setModel(ResiduesModel);

    if( ResiduesModel != NULL ){
        for(int i=0; i < ResiduesModel->columnCount(); i++){
            WidgetUI.residuesTV->resizeColumnToContents(i);
        }
    }

    // create atoms model
    AtomsModel = Structure->GetAtoms()->GetContainerModel(this);
    WidgetUI.atomsTV->setModel(AtomsModel);

    if( AtomsModel != NULL ){
        for(int i=0; i < AtomsModel->columnCount(); i++){
            WidgetUI.atomsTV->resizeColumnToContents(i);
        }
    }

    // enable edit tools
    WidgetUI.buildTab->setEnabled(true);
    WidgetUI.orderTab->setEnabled(true);
    WidgetUI.chainsTab->setEnabled(true);
    WidgetUI.showTab->setEnabled(true);

    UpdateEditTable();
}

//------------------------------------------------------------------------------

void CResidueListWorkPanel::StructureUnregistered(void)
{
    // disable edit tools
    WidgetUI.buildTab->setEnabled(false);
    WidgetUI.orderTab->setEnabled(false);
    WidgetUI.chainsTab->setEnabled(false);
    WidgetUI.showTab->setEnabled(false);

    // clean visualization
    WidgetUI.atomsTV->setModel(NULL);
    delete AtomsModel;
    AtomsModel = NULL;

    WidgetUI.residuesTV->setModel(NULL);
    delete ResiduesModel;
    ResiduesModel = NULL;

    WidgetUI.editTW->clearContents();
    WidgetUI.editTW->setRowCount(0);
    WidgetUI.numOfResiduesLE->setText("");

    WidgetUI.deleteResiduesPB->setEnabled(false);
    WidgetUI.editResiduesPB->setEnabled(false);
    WidgetUI.mergeResiduesPB->setEnabled(false);
}

//------------------------------------------------------------------------------

void CResidueListWorkPanel::ProjectLockChanged(EHistoryChangeMessage message)
{
    if( message != EHCM_LOCK_LEVEL ) return;
    setEnabled(! GetProject()->IsHistoryLocked(EHCL_TOPOLOGY));
}

//------------------------------------------------------------------------------

void CResidueListWorkPanel::ListModelTVDblClicked(const QModelIndex& index)
{
    QAbstractItemModel* p_amodel = (QAbstractItemModel*)(index.model());
    CContainerModel* p_model = dynamic_cast<CContainerModel*>(p_amodel);
    if( p_model == NULL ) return;
    CProObject* p_obj = dynamic_cast<CProObject*>(p_model->GetItem(index));
    if( p_obj == NULL ) return;
    p_obj->OpenObjectDesigner();
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

void CResidueListWorkPanel::UpdateEditTable(void)
{
    // SetSeqIndexWH, etc in EditTWChanged emit signals that UpdateEditTable
    if( UpdatingItems ) return;
    UpdatingItems = true;

    WidgetUI.editTW->setRowCount(Structure->GetResidues()->GetNumberOfResidues());

    Qt::ItemFlags ro_flags(Qt::ItemIsSelectable|Qt::ItemIsEnabled);
    Qt::ItemFlags no_flags(Qt::ItemIsSelectable|Qt::ItemIsEditable|Qt::ItemIsEnabled);

    int nres = 0;
    int id = 0;
    foreach(QObject* p_qobj,Structure->GetResidues()->children()){
        CResidue* p_res = static_cast<CResidue*>(p_qobj);
        QTableWidgetItem* p_item;
        int res_idx = p_res->GetIndex();
        nres++;

        p_item = new QTableWidgetItem(QString().setNum(nres),res_idx);
        p_item->setFlags(ro_flags);
        WidgetUI.editTW->setItem(id, 0, p_item);

        p_item = new QTableWidgetItem(QString().setNum(p_res->GetSeqIndex()),res_idx);
        p_item->setFlags(no_flags);
        WidgetUI.editTW->setItem(id, 1, p_item);

        p_item = new QTableWidgetItem(p_res->GetName(),res_idx);
        p_item->setFlags(no_flags);
        WidgetUI.editTW->setItem(id, 2, p_item);

        p_item = new QTableWidgetItem(p_res->GetChain(),res_idx);
        p_item->setFlags(no_flags);
        WidgetUI.editTW->setItem(id, 3, p_item);

        p_item = new QTableWidgetItem(p_res->GetType(),res_idx);
        p_item->setFlags(no_flags);
        WidgetUI.editTW->setItem(id, 4, p_item);

        id++;
    }

    WidgetUI.numOfResiduesLE->setText(QString().setNum(nres));

    UpdatingItems = false;
}

//------------------------------------------------------------------------------

void CResidueListWorkPanel::EditTWChanged(QTableWidgetItem* p_item)
{
    if( UpdatingItems ) return;
    if( Structure == NULL ) return;
    if( p_item == NULL ) return;

    // get residue
    CResidue* p_res = Structure->GetResidues()->FindResidueByIndex(p_item->type());
    if( p_res == NULL ) return;

    UpdatingItems = true;

    if( p_item->column() == 1 ) {
        QString text = p_item->text();
        bool ok = false;
        int value = text.toInt(&ok);
        if( ok ){
            p_res->SetSeqIndexWH(value);
        }
        // change might be prohibited
        p_item->setText(QString().setNum(p_res->GetSeqIndex()));
    }

    if( p_item->column() == 2 ) {
        QString text = p_item->text();
        p_res->SetNameWH(text);
        // change might be prohibited
        p_item->setText(p_res->GetName());
    }

    if( p_item->column() == 3 ) {
        QString text = p_item->text();
        p_res->SetChainWH(text);
        // change might be prohibited
        p_item->setText(p_res->GetChain());
    }

    if( p_item->column() == 4 ) {
        QString text = p_item->text();
        p_res->SetTypeWH(text);
        // change might be prohibited
        p_item->setText(p_res->GetType());
    }

    UpdatingItems = false;
}

//------------------------------------------------------------------------------

void CResidueListWorkPanel::ItemSelectionChanged(void)
{
    if( Structure == NULL ) return;

    QSet<CResidue*>    sel_residues;

    // extract unique residues
    foreach(QTableWidgetItem* p_item, WidgetUI.editTW->selectedItems()) {
        CResidue* p_res = Structure->GetResidues()->FindResidueByIndex(p_item->type());
        if( p_res == NULL ) continue;
        sel_residues.insert(p_res);
    }

    WidgetUI.deleteResiduesPB->setEnabled(sel_residues.count() > 0);
    WidgetUI.editResiduesPB->setEnabled(sel_residues.count() > 0);
    WidgetUI.mergeResiduesPB->setEnabled(sel_residues.count() >= 2);

    // ------------------------
    UpdateHighlightedResidues();
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

void CResidueListWorkPanel::SeqIndxByOrder(void)
{
    CHistoryNode* p_history = Structure->BeginChangeWH(EHCL_TOPOLOGY,tr("update residue sequence indexes"));
    if( p_history == NULL ) return;
        Structure->GetResidues()->SequenceIndexByOrderWH();
        if( WidgetUI.automaticallyUpdateSerIndxCB->isChecked() ){
            Structure->GetAtoms()->SerialIndexBySeqAndLocIndexesWH();
        }
    Structure->EndChangeWH();

    if( WidgetUI.automaticallyReorderCB->isChecked() ){
        if( WidgetUI.automaticallyUpdateSerIndxCB->isChecked() == false ){
            Structure->GetResidues()->SortResidues();
        }
    }
}

//------------------------------------------------------------------------------

void CResidueListWorkPanel::SeqIndxByReverseOrder(void)
{
    CHistoryNode* p_history = Structure->BeginChangeWH(EHCL_TOPOLOGY,tr("update residue sequence indexes"));
    if( p_history == NULL ) return;
        Structure->GetResidues()->SequenceIndexByReverseOrderWH();
        if( WidgetUI.automaticallyUpdateSerIndxCB->isChecked() ){
            Structure->GetAtoms()->SerialIndexBySeqAndLocIndexesWH();
        }
    Structure->EndChangeWH();

    if( WidgetUI.automaticallyReorderCB->isChecked() ){
        if( WidgetUI.automaticallyUpdateSerIndxCB->isChecked() == false ){
            Structure->GetResidues()->SortResidues();
        }
    }
}

//------------------------------------------------------------------------------

void CResidueListWorkPanel::SeqIndxByChains(void)
{
    CHistoryNode* p_history = Structure->BeginChangeWH(EHCL_TOPOLOGY,tr("update residue sequence indexes"));
    if( p_history == NULL ) return;
        Structure->GetResidues()->SequenceIndexByChainsWH();
        if( WidgetUI.automaticallyUpdateSerIndxCB->isChecked() ){
            Structure->GetAtoms()->SerialIndexBySeqAndLocIndexesWH();
        }
    Structure->EndChangeWH();

    if( WidgetUI.automaticallyReorderCB->isChecked() ){
        if( WidgetUI.automaticallyUpdateSerIndxCB->isChecked() == false ){
            Structure->GetResidues()->SortResidues();
        }
    }
}

//------------------------------------------------------------------------------

void CResidueListWorkPanel::SortResidues(void)
{
    Structure->GetResidues()->SortResidues();
}

//------------------------------------------------------------------------------

void CResidueListWorkPanel::EditResidues(void)
{
    CResidueList* p_list = Structure->GetResidues();

    QSet<CResidue*>    sel_residues;

    // extract unique residues
    foreach(QTableWidgetItem* p_item, WidgetUI.editTW->selectedItems()) {
        CResidue* p_res = p_list->FindResidueByIndex(p_item->type());
        if( p_res == NULL ) continue;
        sel_residues.insert(p_res);
    }

    foreach(CResidue* p_res, sel_residues) {
        CExtUUID object_uuid;
        object_uuid.LoadFromString("{RESIDUE_WORK_PANEL:983992af-e31d-417a-bc21-0b35a32d855b}");

        CWorkPanel* p_wp = static_cast<CWorkPanel*>(PluginDatabase.CreateObject(object_uuid,p_res));
        if( p_wp == NULL ) {
            CSmallString error;
            error << "unable to create tool object " << object_uuid.GetStringForm();
            ES_ERROR(error);
        }
        p_wp->Show();
    }

    WidgetUI.editTW->clearSelection();
}

//------------------------------------------------------------------------------

void CResidueListWorkPanel::DeleteResidues(void)
{
    if( Structure == NULL ) return;

    CResidueList* p_list = Structure->GetResidues();

    QSet<CResidue*>    sel_residues;

    // extract unique residues
    foreach(QTableWidgetItem* p_item, WidgetUI.editTW->selectedItems()) {
        CResidue* p_res = p_list->FindResidueByIndex(p_item->type());
        if( p_res == NULL ) continue;
        sel_residues.insert(p_res);
    }

    if( sel_residues.count() == 0 ) return;

    CHistoryNode* p_history;
    p_history = p_list->BeginChangeWH(EHCL_TOPOLOGY,"remove residues");
    if( p_history == NULL ) return;

        p_list->BeginUpdate();
        foreach(CResidue* p_res, sel_residues) {
            p_res->Delete(true,p_history);
        }
        p_list->EndUpdate();

    p_list->EndChangeWH();

    WidgetUI.editTW->clearSelection();
}

//------------------------------------------------------------------------------

void CResidueListWorkPanel::DeleteEmptyStructures(void)
{
    if( Structure == NULL ) return;
    Structure->GetResidues()->DeleteEmptyResiduesWH();
}

//------------------------------------------------------------------------------

bool LessThanBySeqIndex(CResidue* p_left,CResidue* p_right)
{
    return( p_left->GetSeqIndex() < p_right->GetSeqIndex() );
}

//------------------------------------------------------------------------------

void CResidueListWorkPanel::MergeResidues(void)
{
    if( Structure == NULL ) return;

    CResidueList* p_list = Structure->GetResidues();

    QList<CResidue*>    sel_residues;

    // extract unique residues
    foreach(QTableWidgetItem* p_item, WidgetUI.editTW->selectedItems()) {
        CResidue* p_res = p_list->FindResidueByIndex(p_item->type());
        if( p_res == NULL ) continue;
        if( sel_residues.contains(p_res) ) continue;
        sel_residues.push_back(p_res);
    }

    if( sel_residues.count() < 2 ) return;

    std::sort(sel_residues.begin(),sel_residues.end(),LessThanBySeqIndex);

    CHistoryNode* p_history;
    p_history = p_list->BeginChangeWH(EHCL_TOPOLOGY,"merge residues");
    if( p_history == NULL ) return;

        p_list->BeginUpdate();
        CResidue* p_first = sel_residues.takeFirst();

        foreach(CResidue* p_res, sel_residues) {
            p_first->MergeWith(p_res,p_history);
        }
        p_list->EndUpdate();

    p_list->EndChangeWH();

    WidgetUI.editTW->clearSelection();
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

void CResidueListWorkPanel::UpdateHighlightedResidues(void)
{
    QSet<CResidue*>    sel_residues;

    // extract unique residues
    foreach(QTableWidgetItem* p_item, WidgetUI.editTW->selectedItems()) {
        CResidue* p_res = Structure->GetResidues()->FindResidueByIndex(p_item->type());
        if( p_res == NULL ) continue;
        sel_residues.insert(p_res);
    }

    // highlight residues
    if( WidgetUI.highlightResiduesCB->isChecked() ){
        foreach(QObject* p_qobj, Structure->GetResidues()->children()) {
            CResidue* p_res = static_cast<CResidue*>(p_qobj);
            p_res->SetFlag(EPOF_SELECTED,sel_residues.contains(p_res));
        }
    } else {
        foreach(QObject* p_qobj, Structure->GetResidues()->children()) {
            CResidue* p_res = static_cast<CResidue*>(p_qobj);
            p_res->SetFlag(EPOF_SELECTED,false);
        }
    }

    // update scene
    Structure->GetProject()->RepaintProject();
}

//------------------------------------------------------------------------------

void CResidueListWorkPanel::SetChainNames(void)
{
    CResidueList* p_list = Structure->GetResidues();
    p_list->SetChainNamesWH();
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

void CResidueListWorkPanel::SerialIndexBySeqAndLocIndexes(void)
{
    Structure->GetAtoms()->SerialIndexBySeqAndLocIndexesWH();
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================




