// =============================================================================
// NEMESIS - Molecular Modelling Package
// -----------------------------------------------------------------------------
//    Copyright (C) 2024 Petr Kulhanek, kulhanek@chemi.muni.cz
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
#include <ErrorSystem.hpp>
#include <CategoryUUID.hpp>
#include <DesktopSystem.hpp>
#include <ProjectList.hpp>
#include <Project.hpp>
#include <SelectionList.hpp>
#include <AtomSelection.hpp>
#include <BondSelection.hpp>
#include <ConnectAtomSelection.hpp>
#include <AtomSeqSelection.hpp>
#include <Atom.hpp>
#include <Bond.hpp>
#include <Structure.hpp>
#include <Fragment.hpp>
#include <GlobalSetup.hpp>
#include <Graphics.hpp>
#include <GraphicsProfile.hpp>
#include <PeriodicTableWidget.hpp>
#include <PeriodicTable.hpp>
#include <SelectionRequest.hpp>
#include <InserterList.hpp>
#include <OptimizerSetupList.hpp>
#include <Optimizer.hpp>
#include <WorkPanelList.hpp>

#include "BuildWorkPanel.hpp"
#include "StandardWPModule.hpp"

#include <QGridLayout>

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

QObject* BuildWorkPanelCB(void* p_data);

CExtUUID        BuildWorkPanelID(
                    "{BUILD_WORK_PANEL:e5c311d6-73f7-4091-a6e3-1d73024fe27d}",
                    "Build Panel");

CPluginObject   BuildWorkPanelObject(&StandardWPPlugin,
                     BuildWorkPanelID,WORK_PANEL_CAT,
                     ":images/StandardWP/StructureBuildStructure.svg",
                     BuildWorkPanelCB);

// -----------------------------------------------------------------------------

QObject* BuildWorkPanelCB(void* p_data)
{
    CProject* p_project = static_cast<CProject*>(p_data);
    if( p_project == NULL ){
        ES_ERROR("CBuildWorkPanel requires active project");
        return(NULL);
    }

    // only one instance is allowed
    QObject* p_wp = WorkPanels->Find(BuildWorkPanelID,p_project);
    if( p_wp == NULL ) {
        p_wp = new CBuildWorkPanel(p_project);
    }

    return(p_wp);
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

CBuildWorkPanel::CBuildWorkPanel(CProject* p_project)
    : CWorkPanel(&BuildWorkPanelObject,p_project,EWPR_TOOL)
{
    // set up ui file
    WidgetUI.setupUi(this);

    Optimizer = NULL;

    connect(GetProject()->GetHistory(),SIGNAL(OnHistoryChanged(EHistoryChangeMessage)),
            this,SLOT(ProjectLockChanged(EHistoryChangeMessage)));

    PeriodicTableDialog = new QDialog(this);
    PeriodicTableDialog->setWindowTitle(tr("Periodic Table"));
    CPeriodicTableWidget* p_ptw = new CPeriodicTableWidget(PeriodicTableDialog);
    p_ptw->show();
    PeriodicTableDialog->updateGeometry();

    connect(p_ptw,SIGNAL(OnElementChanged(int)),
            this,SLOT(ElementChanged(int)));
    connect(p_ptw,SIGNAL(OnElementChanged(int)),
            PeriodicTableDialog,SLOT(close(void)));

    if( layout() != NULL ) {
        QSpacerItem* p_sp = new QSpacerItem(20,5,QSizePolicy::Minimum, QSizePolicy::Expanding);
        layout()->addItem(p_sp);
    }

    Action = EBA_NONE;
    Z = -1;
    Order = BO_NONE;

    // action group
    ActionGroup = new QButtonGroup(this);
    ActionGroup->setExclusive(true);
    ActionGroup->addButton(WidgetUI.nonePB);
    ActionGroup->addButton(WidgetUI.deleteAtomPB);
    ActionGroup->addButton(WidgetUI.makeBondPB);
    ActionGroup->addButton(WidgetUI.breakBondPB);
    ActionGroup->addButton(WidgetUI.deleteBondPB);

    ActionGroup->addButton(WidgetUI.order_1_0_PB);
    ActionGroup->addButton(WidgetUI.order_1_5_PB);
    ActionGroup->addButton(WidgetUI.order_2_0_PB);
    ActionGroup->addButton(WidgetUI.order_3_0_PB);

    ActionGroup->addButton(WidgetUI.changeZPB);
    ActionGroup->addButton(WidgetUI.addValencePB);

    WidgetUI.nonePB->setChecked(true);
    WidgetUI.nonePB->hide();

    connect(ActionGroup,SIGNAL(buttonClicked(QAbstractButton*)),
            this,SLOT(ActionChange(QAbstractButton*)));

    connect(WidgetUI.optimizePB,SIGNAL(clicked(bool)),
            this,SLOT(Optimize(bool)));

    // create selection request
    SelRequest = new CSelectionRequest(this,tr("build panel"));
    connect(SelRequest,SIGNAL(OnSelectionCompleted(void)),
            this,SLOT(SelectionCompleted(void)));
    connect(SelRequest,SIGNAL(OnDetached(void)),
            this,SLOT(SelectionDetached(void)));

    connect(WidgetUI.newStructureCB,SIGNAL(toggled(bool)),
            Inserters,SLOT(SetNewStructureAllowed(bool)));
    connect(WidgetUI.autoCenterCB,SIGNAL(toggled(bool)),
            Inserters,SLOT(SetAutoCenter(bool)));
    UpdateInserterSetup();

    connect(Inserters,SIGNAL(SetupChanged()),
            this,SLOT(UpdateInserterSetup(void)));


    // load structure palete
    LoadFragments();

    // set default action
    SetAction(EBA_NONE);

    // load work panel setup
    LoadWorkPanelSetup();

    // update locks
    ProjectLockChanged(EHCM_LOCK_LEVEL);
}

// -----------------------------------------------------------------------------

CBuildWorkPanel::~CBuildWorkPanel(void)
{
    SaveWorkPanelSetup();

    delete PeriodicTableDialog;
    PeriodicTableDialog = NULL;
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

void CBuildWorkPanel::LoadWorkPanelSpecificData(CXMLElement* p_ele)
{
    if( p_ele == NULL ){
        INVALID_ARGUMENT("p_ele == NULL");
    }

    bool bdummy = false;
    if( p_ele->GetAttribute("opt_preopt",bdummy) ){
        WidgetUI.preoptimizeCB->setChecked(bdummy);
    }
    if( p_ele->GetAttribute("opt_manip_atoms",bdummy) ){
        WidgetUI.manipAtomsCB->setChecked(bdummy);
    }

    double ddummy = 0.0;
    if( p_ele->GetAttribute("opt_preopt_steps",ddummy) ){
        WidgetUI.preoptStepsSB->setValue(ddummy);
    }

    CWorkPanel::LoadWorkPanelSpecificData(p_ele);
}

//------------------------------------------------------------------------------

void CBuildWorkPanel::SaveWorkPanelSpecificData(CXMLElement* p_ele)
{
    if( p_ele == NULL ){
        INVALID_ARGUMENT("p_ele == NULL");
    }

    p_ele->SetAttribute("opt_preopt",WidgetUI.preoptimizeCB->isChecked());
    p_ele->SetAttribute("opt_manip_atoms",WidgetUI.manipAtomsCB->isChecked());
    p_ele->SetAttribute("opt_preopt_steps",WidgetUI.preoptStepsSB->value());

    CWorkPanel::SaveWorkPanelSpecificData(p_ele);
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

void CBuildWorkPanel::UpdateInserterSetup(void)
{
    WidgetUI.newStructureCB->setChecked(Inserters->IsNewStructureAllowed());
    WidgetUI.autoCenterCB->setChecked(Inserters->IsAutoCenterEnabled());
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

void CBuildWorkPanel::ActionChange(QAbstractButton *p_button)
{
    if( ActiveButton == p_button ) {
        CFragment* p_structure = NULL;
        if((p_structure = GetActiveFragment()) != NULL) {
            // change to next connector
            p_structure->SetNextConnector();
            // change icon
            QIcon ico = p_structure->GetConnectorIcon();
            QSize size = ico.actualSize(QSize(128,128));
            p_button->setIconSize(size+QSize(8,8));
            p_button->setIcon(ico);
        }
        return;
    }
    ActiveButton = p_button;

    // ---------------------------------
    if( p_button == WidgetUI.deleteAtomPB ) {
        SetAction(EBA_DELETE_ATOM);
        return;
    }
    if( p_button == WidgetUI.makeBondPB ) {
        SetAction(EBA_MAKE_BOND);
        return;
    }
    if( p_button == WidgetUI.breakBondPB ) {
        SetAction(EBA_BREAK_BOND);
        return;
    }
    if( p_button == WidgetUI.deleteBondPB ) {
        SetAction(EBA_DELETE_BOND);
        return;
    }

    // ---------------------------------
    if( p_button == WidgetUI.order_1_0_PB ) {
        Order = BO_SINGLE;
        SetAction(EBA_CHANGE_ORDER);
        return;
    }
    if( p_button == WidgetUI.order_1_5_PB ) {
        Order = BO_SINGLE_H;
        SetAction(EBA_CHANGE_ORDER);
        return;
    }
    if( p_button == WidgetUI.order_2_0_PB ) {
        Order = BO_DOUBLE;
        SetAction(EBA_CHANGE_ORDER);
        return;
    }
    if( p_button == WidgetUI.order_3_0_PB ) {
        Order = BO_TRIPLE;
        SetAction(EBA_CHANGE_ORDER);
        return;
    }

    // ---------------------------------
    if( p_button == WidgetUI.changeZPB ) {
        SetAction(EBA_CHANGE_Z);
        return;
    }

    // ---------------------------------
    if( p_button == WidgetUI.addValencePB ) {
        SetAction(EBA_ADD_VALENCE);
        return;
    }

    // ---------------------------------
    SetAction(EBA_ADD_STRUCTURE);
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

void CBuildWorkPanel::SetAction(EBuildAction new_action)
{
    Action = new_action;

    switch(Action) {
    case EBA_NONE:
        WidgetUI.nonePB->setChecked(true);
        ActiveButton = NULL;
        SelRequest->SetRequest(NULL);
        break;
        //------------------------------
    case EBA_ADD_STRUCTURE:
        SelRequest->SetRequest(GetProject()->GetSelection(),&SH_ConnectAtom,"to attach a molecular fragment");
        break;
        //------------------------------
    case EBA_MAKE_BOND:
        SelRequest->SetRequest(GetProject()->GetSelection(),&SH_TwoAtoms,"to make a new bond");
        break;
        //------------------------------
    case EBA_DELETE_ATOM:
        SelRequest->SetRequest(GetProject()->GetSelection(),&SH_Atom,"to delete it");
        break;
        //------------------------------
    case EBA_DELETE_BOND:
        SelRequest->SetRequest(GetProject()->GetSelection(),&SH_Bond,"to delete it");
        break;
        //------------------------------
    case EBA_BREAK_BOND:
        SelRequest->SetRequest(GetProject()->GetSelection(),&SH_Bond,"to break it");
        break;
        //------------------------------
    case EBA_CHANGE_Z:
        SelRequest->SetRequest(GetProject()->GetSelection(),&SH_Atom,"to change its proton number");
        break;
        //------------------------------
    case EBA_CHANGE_ORDER:
        SelRequest->SetRequest(GetProject()->GetSelection(),&SH_Bond,"to change its order");
        break;
        //------------------------------
    case EBA_ADD_VALENCE:
        SelRequest->SetRequest(GetProject()->GetSelection(),&SH_Atom,"to add a valence");
        break;
        //------------------------------
    }
    GetProject()->RepaintProject();
}

//------------------------------------------------------------------------------

void CBuildWorkPanel::SelectionCompleted(void)
{
    CSelectionList* p_sel = SelRequest->GetSelectionList();

    bool changed = false;

    switch(Action) {
    case EBA_ADD_STRUCTURE: {
        CFragment* p_str = GetActiveFragment();
        if( p_str == NULL) break;
        p_str->SetMasterAtomZ(Z);

        CProObject* p_object = p_sel->PopSelectedObject();

        // attach structure to the selected atom
        CAtom* p_atom = dynamic_cast<CAtom*>(p_object);
        if( p_atom != NULL ) {
            changed = Inserters->InsertStructureWH(p_str,p_atom);
        } else {
            // it has to be in else block since p_object can be destroyed by AttachStructureWH
            // insert structure to the molecule
            CStructure* p_mol = dynamic_cast<CStructure*>(p_object);
            if( p_mol != NULL ) {
                changed = Inserters->InsertStructureWH(p_str,p_mol);
            }
        }
    }
    break;
    //------------------------------
    case EBA_MAKE_BOND: {
        CAtom* p_atom1 = dynamic_cast<CAtom*>(p_sel->PopSelectedObject());
        CAtom* p_atom2 = dynamic_cast<CAtom*>(p_sel->PopSelectedObject());
        if( (p_atom1 != NULL) && (p_atom1 != NULL) ) {
            changed = p_atom1->MakeBondWithWH(p_atom2,BO_SINGLE);
        }
    }
    break;
    //------------------------------
    case EBA_DELETE_ATOM: {
        CAtom* p_atom = dynamic_cast<CAtom*>(p_sel->PopSelectedObject());
        if( p_atom != NULL ) changed = p_atom->DeleteWH();
    }
    break;
    //------------------------------
    case EBA_DELETE_BOND: {
        CBond* p_bond = dynamic_cast<CBond*>(p_sel->PopSelectedObject());
        if( p_bond != NULL ) changed = p_bond->DeleteWH();
    }
    break;
    //------------------------------
    case EBA_BREAK_BOND: {
        CBond* p_bond = dynamic_cast<CBond*>(p_sel->PopSelectedObject());
        if( p_bond != NULL ) changed = p_bond->BreakWH();
    }
    break;
    //------------------------------
    case EBA_CHANGE_Z: {
        CAtom* p_atom = dynamic_cast<CAtom*>(p_sel->PopSelectedObject());
        if( p_atom != NULL ) changed = p_atom->SetZWH(Z);
    }
    break;
    //------------------------------
    case EBA_CHANGE_ORDER: {
        CBond* p_bond = dynamic_cast<CBond*>(p_sel->PopSelectedObject());
        if( p_bond != NULL ) changed = p_bond->SetOrderWH(Order);
    }
    break;
    //------------------------------
    case EBA_ADD_VALENCE: {
        CAtom* p_atom = dynamic_cast<CAtom*>(p_sel->PopSelectedObject());
        if( p_atom != NULL ) changed = p_atom->AddValenceWH();
    }
    break;
    //------------------------------
    default:
        break;
    }

    if( changed && WidgetUI.preoptimizeCB->isChecked() && (WidgetUI.preoptStepsSB->value() > 0) &&
        (Optimizer == NULL) ){
        CStructure* p_mol = dynamic_cast<CStructure*>(GetProject()->GetActiveStructure());
        if( p_mol == NULL ) return;

        Optimizer = OptimizerSetups->CreateDefaultOptimizerJob(p_mol);
        if( Optimizer == NULL ) return;
        Optimizer->SetMaxOptSteps(WidgetUI.preoptStepsSB->value());

        connect(Optimizer,SIGNAL(OnJobStatusChanged(CJob*)),this,SLOT(JobStatusChanged(CJob*)));
        WidgetUI.optimizePB->setText("Stop");
        WidgetUI.optimizePB->setChecked(true);

        Optimizer->SubmitJob();
    } else {
        p_sel->GetProject()->RepaintProject();
    }
}

//------------------------------------------------------------------------------

void CBuildWorkPanel::SelectionDetached(void)
{
    SetAction(EBA_NONE);
}

//------------------------------------------------------------------------------

void CBuildWorkPanel::Optimize(bool checked)
{
    if( Optimizer && (checked == true) ) return;

    if( Optimizer && (checked == false) ){
        Optimizer->TerminateJob();
        return;
    }

    CStructure* p_mol = dynamic_cast<CStructure*>(GetProject()->GetActiveStructure());
    if( p_mol == NULL ) return;

    Optimizer = OptimizerSetups->CreateDefaultOptimizerJob(p_mol);
    if( Optimizer == NULL ) return;

    if( WidgetUI.manipAtomsCB->isChecked() ){
        Optimizer->SetMaxOptSteps(-1);
        GetProject()->GetMouseHandler()->SetSecondaryDriver(EMD_ATOM_MANIP_RELAX);
    } else {
        GetProject()->GetMouseHandler()->SetSecondaryDriver(EMD_NONE);
    }

    connect(Optimizer,SIGNAL(OnJobStatusChanged(CJob*)),this,SLOT(JobStatusChanged(CJob*)));
    WidgetUI.optimizePB->setText("Stop");
    WidgetUI.optimizePB->setChecked(true);

    Optimizer->SubmitJob();
}

//------------------------------------------------------------------------------

void CBuildWorkPanel::JobStatusChanged(CJob* p_job)
{
    if( (p_job->GetJobStatus() == EJS_FINISHED) ||
        (p_job->GetJobStatus() == EJS_ABORTED) ||
        (p_job->GetJobStatus() == EJS_ENDED) ) {
        WidgetUI.optimizePB->setText("Start");
        WidgetUI.optimizePB->setChecked(false);
        GetProject()->GetMouseHandler()->SetSecondaryDriver(EMD_NONE);
        Optimizer = NULL;
    }
}

//------------------------------------------------------------------------------

void CBuildWorkPanel::ShowPeriodicTable(void)
{
    PeriodicTableDialog->exec();
}

//------------------------------------------------------------------------------

void CBuildWorkPanel::ElementChanged(int z)
{
    Z = z;
    WidgetUI.periodicTablePB->setText("X: " + QString(PeriodicTable.GetName(z)));
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

bool CBuildWorkPanel::LoadFragments(void)
{
    CFileName path = GlobalSetup->GetWorkPanelSetupPath(GetType());

    // ---------------------------------
    bool result = true;
    result &= BasicFragments.Load(path / "basic.xml");
    result &= GeneralFragments.Load(path / "general.xml");

    if( result == false ) {
        ES_ERROR("unable to load structures");
    }

    // ---------------------------------
    if( WidgetUI.basicTab->layout() != NULL ) {
        delete WidgetUI.basicTab->layout();
    }
    QGridLayout* p_gl = new QGridLayout(WidgetUI.basicTab);
    p_gl->setSpacing(2);
    p_gl->setContentsMargins(0,2,0,2);
    WidgetUI.basicTab->setLayout(p_gl);

    // add items
    int maxrow = 0;
    for(int i=0; i < BasicFragments.GetNumberOfFragments(); i++) {
        QPushButton* p_pb = new QPushButton(WidgetUI.basicTab);
        p_pb->setProperty("basic",i);
        p_pb->setCheckable(true);
        p_pb->setFocusPolicy(Qt::NoFocus);
        QIcon ico = BasicFragments.GetFragment(i)->GetConnectorIcon();
        QSize size = ico.actualSize(QSize(128,128));
        p_pb->setIconSize(size+QSize(8,8));
        p_pb->setIcon(ico);
        ActionGroup->addButton(p_pb);

        int row = i / 4;
        if( row > maxrow ) maxrow  = row;
        int col = i % 4;
        p_gl->addWidget(p_pb,row,col,1,1);
    }
    QSpacerItem* p_sp = new QSpacerItem(20,5,QSizePolicy::Minimum, QSizePolicy::Expanding);
    p_gl->addItem(p_sp,maxrow+1,0,1,4);

    // ---------------------------------
    if( WidgetUI.generalTab->layout() != NULL ) {
        delete WidgetUI.generalTab->layout();
    }
    p_gl = new QGridLayout(WidgetUI.generalTab);
    p_gl->setSpacing(2);
    p_gl->setContentsMargins(0,2,0,2);
    WidgetUI.generalTab->setLayout(p_gl);

    p_gl->addWidget(WidgetUI.periodicTablePB, 0, 0, 1, 2);
    connect(WidgetUI.periodicTablePB, SIGNAL(clicked()),
            this, SLOT(ShowPeriodicTable()));
    ElementChanged(6); // set default element to carbon

    p_gl->addWidget(WidgetUI.changeZPB,0,2,1,2);

    maxrow = 0;
    for(int i=0; i < GeneralFragments.GetNumberOfFragments(); i++) {
        QPushButton* p_pb = new QPushButton(WidgetUI.generalTab);
        p_pb->setProperty("general",i);
        p_pb->setCheckable(true);
        p_pb->setFocusPolicy(Qt::NoFocus);
        QIcon ico = GeneralFragments.GetFragment(i)->GetConnectorIcon();
        QSize size = ico.actualSize(QSize(128,128));
        p_pb->setIconSize(size+QSize(8,8));
        p_pb->setIcon(ico);
        ActionGroup->addButton(p_pb);

        int row = i / 4 + 1;
        if( row > maxrow ) maxrow  = row;
        int col = i % 4;
        p_gl->addWidget(p_pb,row,col,1,1);
    }
    p_sp = new QSpacerItem(20,5,QSizePolicy::Minimum, QSizePolicy::Expanding);
    p_gl->addItem(p_sp,maxrow+1,0,1,4);

    return(result);
}

//------------------------------------------------------------------------------

CFragment* CBuildWorkPanel::GetActiveFragment(void)
{
    QAbstractButton* p_but = ActionGroup->checkedButton();
    if( p_but == NULL ) return(NULL);

    if( p_but->property("basic").isValid() ) {
        int id = p_but->property("basic").toInt();
        return( BasicFragments.GetFragment(id));
    }

    if( p_but->property("general").isValid() ) {
        int id = p_but->property("general").toInt();
        return( GeneralFragments.GetFragment(id));
    }

    return(NULL);
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

void CBuildWorkPanel::ProjectLockChanged(EHistoryChangeMessage message)
{
    if( message != EHCM_LOCK_LEVEL ) return;

    WidgetUI.tabWidget->setEnabled( ! GetProject()->GetHistory()->IsLocked(EHCL_TOPOLOGY));
    WidgetUI.structureEditGB->setEnabled( ! GetProject()->GetHistory()->IsLocked(EHCL_TOPOLOGY));
    WidgetUI.preoptStepsSB->setEnabled( ! GetProject()->GetHistory()->IsLocked(EHCL_TOPOLOGY));
    WidgetUI.preoptimizeCB->setEnabled( ! GetProject()->GetHistory()->IsLocked(EHCL_TOPOLOGY));
    WidgetUI.manipAtomsCB->setEnabled( ! GetProject()->GetHistory()->IsLocked(EHCL_TOPOLOGY));
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

