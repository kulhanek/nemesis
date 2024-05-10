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
#include <QButtonGroup>

#include <CategoryUUID.hpp>
#include <DesktopSystem.hpp>
#include <ProjectList.hpp>
#include <Project.hpp>
#include <ContainerModel.hpp>

#include <SelectionList.hpp>
#include <SelectionRequest.hpp>
#include <PointSelection.hpp>
#include <DistanceSelection.hpp>
#include <AngleSelection.hpp>
#include <TorsionSelection.hpp>

#include <PhysicalQuantities.hpp>
#include <PhysicalQuantity.hpp>
#include <GeoMeasurement.hpp>
#include <AtomList.hpp>
#include <Atom.hpp>
#include <Bond.hpp>
#include <RestraintList.hpp>
#include <Structure.hpp>
#include <GeometryLabelObject.hpp>
#include <Graphics.hpp>
#include <GraphicsProfileList.hpp>
#include <GraphicsProfile.hpp>
#include <WorkPanelList.hpp>
#include <StructureList.hpp>
#include <PropertyList.hpp>
#include <PropertyAtomList.hpp>

#include <DistanceProperty.hpp>
#include <AngleProperty.hpp>
#include <TorsionProperty.hpp>
#include <DistanceToPositionProperty.hpp>

#include "GeometryWorkPanel.hpp"
#include "StandardWPModule.hpp"

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

QObject* GeometryWorkPanelCB(void* p_data);

CExtUUID        GeometryWorkPanelID(
                    "{GEOMETRY_WORK_PANEL:63a0f336-31f4-4509-84b7-4218e5a84e9d}",
                    "Geometry");

CPluginObject   GeometryWorkPanelObject(&StandardWPPlugin,
                    GeometryWorkPanelID,WORK_PANEL_CAT,
                    ":/images/StandardWP/Geometry.svg",
                    GeometryWorkPanelCB);

// -----------------------------------------------------------------------------

QObject* GeometryWorkPanelCB(void* p_data)
{
    CProject* p_project = static_cast<CProject*>(p_data);
    if( p_project == NULL ){
        ES_ERROR("Active project required.");
        return(NULL);
    }

    // only one instance is allowed
    QObject* p_wp = WorkPanels->Find(GeometryWorkPanelID,p_project);
    if( p_wp == NULL ) {
        p_wp = new CGeometryWorkPanel(p_project);
    }

    return(p_wp);
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

CGeometryWorkPanel::CGeometryWorkPanel(CProject* p_project)
    : CWorkPanel(&GeometryWorkPanelObject,p_project,EWPR_TOOL)
{
    Model = NULL;
    Atoms = NULL;

    // set up ui file
    WidgetUI.setupUi(this);

    connect(GetProject()->GetHistory(),SIGNAL(OnHistoryChanged(EHistoryChangeMessage)),
            this,SLOT(ProjectLockChanged(EHistoryChangeMessage)));

    if( layout() != NULL ) {
        QSpacerItem* p_sp = new QSpacerItem(20,5,QSizePolicy::Minimum, QSizePolicy::Expanding);
        layout()->addItem(p_sp);
    }

    // create button group
    ButtonGroup = new QButtonGroup(this);
    ButtonGroup->setExclusive(true);
    ButtonGroup->addButton(WidgetUI.PBNone,EGWPA_NONE);
    ButtonGroup->addButton(WidgetUI.PBPosition,EGWPA_POSITION);
    ButtonGroup->addButton(WidgetUI.PBDistance,EGWPA_DISTANCE);
    ButtonGroup->addButton(WidgetUI.PBAngle,EGWPA_ANGLE);
    ButtonGroup->addButton(WidgetUI.PBTorsion,EGWPA_TORSION);
    WidgetUI.PBNone->hide();

    connect(ButtonGroup,SIGNAL(buttonPressed(int)),
            this,SLOT(SetAction(int)));

    // units
    connect(PQ_DISTANCE,SIGNAL(OnUnitChanged(void)),
            this,SLOT(PhysicalQuantityUnitChange(void)));
    connect(PQ_ANGLE,SIGNAL(OnUnitChanged(void)),
            this,SLOT(PhysicalQuantityUnitChange(void)));

    // create selection request
    SelRequest = new CSelectionRequest(this,tr("geometry panel"));
    connect(SelRequest,SIGNAL(OnSelectionInitialized(void)),
            this,SLOT(SelectionInitialized(void)));
    connect(SelRequest,SIGNAL(OnSelectionCompleted(void)),
            this,SLOT(SelectionCompleted(void)));
    connect(SelRequest,SIGNAL(OnDetached(void)),
            this,SLOT(SelectionDetached(void)));
    connect(SelRequest,SIGNAL(OnSelectionChanged(void)),
            this,SLOT(SelectionChanged(void)));

    // label or restrain
    connect(WidgetUI.PBLabelDAT,SIGNAL(clicked(bool)),
            this,SLOT(LabelDAT(void)));
    connect(WidgetUI.PBRestrainDAT,SIGNAL(clicked(bool)),
            this,SLOT(RestrainDAT(void)));
    connect(WidgetUI.PBPropertyDAT,SIGNAL(clicked(bool)),
            this,SLOT(PropertyDAT(void)));

    CStructureList* p_sl = GetProject()->GetStructures();
    if( p_sl != NULL ) {
        connect(p_sl,SIGNAL(OnGeometryChangeTick(void)),
                this,SLOT(SelectionCompleted(void)));
    }

    //------------------
    connect(WidgetUI.listView,SIGNAL(doubleClicked(const QModelIndex&)),
            this,SLOT(ModelDblClick(const QModelIndex&)));

    SetAction(EGWPA_NONE);

    // load work panel setup
    LoadWorkPanelSetup();

    // update locks
    ProjectLockChanged(EHCM_LOCK_LEVEL);
}

// -----------------------------------------------------------------------------

CGeometryWorkPanel::~CGeometryWorkPanel(void)
{
    SaveWorkPanelSetup();
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

void CGeometryWorkPanel::SetAction(int id)
{
    // update enable/disable for tools
    ProjectLockChanged(EHCM_LOCK_LEVEL);

    EGeometryWorkPanelAction act = static_cast<EGeometryWorkPanelAction>(id);

    switch(act) {
        case EGWPA_NONE:
            WidgetUI.PBNone->setChecked(true);
            WidgetUI.stackedWidget->setCurrentIndex(0);
            break;
        case EGWPA_POSITION:
            WidgetUI.stackedWidget->setCurrentIndex(1);
            ButtonGroup->button(act)->setChecked(true);
            break;
        case EGWPA_DISTANCE:
        case EGWPA_ANGLE:
        case EGWPA_TORSION:
            WidgetUI.stackedWidget->setCurrentIndex(2);
            ButtonGroup->button(act)->setChecked(true);
            break;
    }

    SelectionInitialized();

    switch(act) {
        case EGWPA_NONE:
            if( Model != NULL ) delete Model;
            Model = NULL;
            WidgetUI.listView->setModel(NULL);
            SelRequest->SetRequest(NULL);
            GetProject()->RepaintProject();
            return;
            break;
        case EGWPA_POSITION:
            SelRequest->SetRequest(GetProject()->GetSelection(),&SH_Point);
            break;
        case EGWPA_DISTANCE:
            SelRequest->SetRequest(GetProject()->GetSelection(),&SH_Distance);
            break;
        case EGWPA_ANGLE:
            SelRequest->SetRequest(GetProject()->GetSelection(),&SH_Angle);
            break;
        case EGWPA_TORSION:
            SelRequest->SetRequest(GetProject()->GetSelection(),&SH_Torsion);
            break;
    }

    CSelectionList* p_sel = GetProject()->GetSelection();
    Model = p_sel->GetContainerModel(this);
    WidgetUI.listView->setModel(Model);
    for(int i=0; i < Model->columnCount(); i++) {
        WidgetUI.listView->resizeColumnToContents(i);
    }

    GetProject()->RepaintProject();
}

//------------------------------------------------------------------------------

void CGeometryWorkPanel::SelectionDetached(void)
{
    SetAction(EGWPA_NONE);
}

//------------------------------------------------------------------------------

void CGeometryWorkPanel::SelectionChanged(void)
{
    // update locks
    ProjectLockChanged(EHCM_LOCK_LEVEL);
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

void CGeometryWorkPanel::SelectionInitialized(void)
{
    // update enable/disable for tools
    ProjectLockChanged(EHCM_LOCK_LEVEL);

    EGeometryWorkPanelAction act = static_cast<EGeometryWorkPanelAction>(ButtonGroup->checkedId());

    switch(act) {
        case EGWPA_POSITION: {
            WidgetUI.positionXLE->setPhysicalQuantity(PQ_DISTANCE);
            WidgetUI.positionYLE->setPhysicalQuantity(PQ_DISTANCE);
            WidgetUI.positionZLE->setPhysicalQuantity(PQ_DISTANCE);

            WidgetUI.positionXLE->setInternalValue(0.0);
            WidgetUI.positionYLE->setInternalValue(0.0);
            WidgetUI.positionZLE->setInternalValue(0.0);
        }
        break;
        //------------------------------
        case EGWPA_DISTANCE: {
            WidgetUI.valueLE->setPhysicalQuantity(PQ_DISTANCE);
            WidgetUI.valueLE->setInternalValue(0.0);
        }
        break;
        //------------------------------
        case EGWPA_ANGLE: {
            WidgetUI.valueLE->setPhysicalQuantity(PQ_ANGLE);
            WidgetUI.valueLE->setInternalValue(0.0);
        }
        break;
        //------------------------------
        case EGWPA_TORSION: {
            WidgetUI.valueLE->setPhysicalQuantity(PQ_ANGLE);
            WidgetUI.valueLE->setInternalValue(0.0);
        }
        break;
        //------------------------------
        default:
            break;
    }
}

//------------------------------------------------------------------------------

void CGeometryWorkPanel::SelectionCompleted(void)
{
    CSelectionList* p_list = SelRequest->GetSelectionList();

    // these two conditions are necessary as SelectionCompleted is also called in response to
    // geometry change

    // update enable/disable for tools
    ProjectLockChanged(EHCM_LOCK_LEVEL);

    if( p_list == NULL ) return;
    if( p_list->GetStatus() != ESS_SELECTED_OBJECTS_END ) return;

    EGeometryWorkPanelAction act = static_cast<EGeometryWorkPanelAction>(ButtonGroup->checkedId());

    switch(act) {
        case EGWPA_POSITION: {
            CPoint pos = CGeoMeasurement::GetPosition(p_list);
            WidgetUI.positionXLE->setInternalValue(pos.x);
            WidgetUI.positionYLE->setInternalValue(pos.y);
            WidgetUI.positionZLE->setInternalValue(pos.z);
        }
        break;
        //------------------------------
        case EGWPA_DISTANCE: {
            double length = CGeoMeasurement::GetDistance(p_list);
            WidgetUI.valueLE->setInternalValue(length);
        }
        break;
        //------------------------------
        case EGWPA_ANGLE: {
            double angle = CGeoMeasurement::GetAngle(p_list);
            WidgetUI.valueLE->setInternalValue(angle);
        }
        break;
        //------------------------------
        case EGWPA_TORSION: {
            double angle = CGeoMeasurement::GetTorsion(p_list);
            WidgetUI.valueLE->setInternalValue(angle);
        }
        break;
        //------------------------------
        default:
            break;
    }
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

void CGeometryWorkPanel::LabelDAT(void)
{
    CSelectionList* p_list = SelRequest->GetSelectionList();

    // these two conditions are necessary as SelectionCompleted is also called in response to
    // geometry change

    if( p_list == NULL ) return;
    if( p_list->GetStatus() != ESS_SELECTED_OBJECTS_END ) return;

    CGraphicsProfile* p_profile = p_list->GetProject()->GetGraphics()->GetProfiles()->GetActiveProfile();
    if( p_profile == NULL ) return;

    CHistoryNode* p_history = p_profile->BeginChangeWH(EHCL_GRAPHICS,tr("label geometry parameter"));
    if( p_history == NULL ) return;

    CExtUUID objuuid;
    objuuid.LoadFromString("{GEOMETRY_LABEL_OBJECT:e8f67b10-14ec-4f72-9099-9a74827485b6}");
    CGraphicsObject* p_obj =  p_profile->CreateObjectWH(objuuid);
    if( p_obj != NULL ){
        CGeometryLabelObject* p_mobj = dynamic_cast<CGeometryLabelObject*>(p_obj);
        if( p_mobj != NULL ){
            p_mobj->AddSelectedObjectsWH();

            EGeometryWorkPanelAction act = static_cast<EGeometryWorkPanelAction>(ButtonGroup->checkedId());
            switch(act) {
                case EGWPA_NONE:
                    p_mobj->SetTypeWH(ESGLT_NONE);
                    break;

                case EGWPA_POSITION:
                    p_mobj->SetTypeWH(ESGLT_POINT);
                    break;

                case EGWPA_DISTANCE:
                    p_mobj->SetTypeWH(ESGLT_DISTANCE);
                    break;

                case EGWPA_ANGLE:
                    p_mobj->SetTypeWH(ESGLT_ANGLE);
                    break;

                case EGWPA_TORSION:
                    p_mobj->SetTypeWH(ESGLT_DIHEDRAL);
                    break;
            }

        } else {
            ES_ERROR("p_mobj is not CGeometryLabelObject");
        }
    }

    p_profile->EndChangeWH();
}

//------------------------------------------------------------------------------

void CGeometryWorkPanel::PropertyDAT(void)
{
    CSelectionList* p_list = SelRequest->GetSelectionList();

    // these two conditions are necessary as SelectionCompleted is also called in response to
    // geometry change

    if( p_list == NULL ) return;
    if( p_list->GetStatus() != ESS_SELECTED_OBJECTS_END ){
        GetProject()->TextNotification(ETNT_WARNING,tr("unable to set property - selection was not finished"),3000);
        return;
    }

    if( GetProject()->GetActiveStructure() == NULL ){
        GetProject()->TextNotification(ETNT_WARNING,tr("unable to set property - no structure is active"),3000);
        return;
    }

    EGeometryWorkPanelAction act = static_cast<EGeometryWorkPanelAction>(ButtonGroup->checkedId());
    if( act == EGWPA_NONE ) return;

    // check if all objects are atoms
    int acount = 0;
    for(int i=0; i < p_list->NumOfSelectedObjects(); i++ ){
        CProObject* p_selobj = p_list->GetSelectedObject(i);
        CAtom* p_atom = dynamic_cast<CAtom*>(p_selobj);
        if( p_atom == NULL ){
            GetProject()->TextNotification(ETNT_WARNING,tr("unable to set property - all selected objects must be atoms"),3000);
            return;
        }
        if( p_atom->GetStructure() == GetProject()->GetActiveStructure() ){
            acount++;
        }
    }

    if( acount == 0 ){
        GetProject()->TextNotification(ETNT_WARNING,tr("unable to set property - none of selected atoms is from the active structure"),3000);
        return;
    }

    CHistoryNode* p_history = GetProject()->BeginChangeWH(EHCL_PROPERTY,"create geometry property");
    if( p_history == NULL ){
        GetProject()->TextNotification(ETNT_WARNING,tr("unable to set property - property changes are prohibited"),3000);
        return;
    }

    // create property
    CGraphicsProperty* p_property = NULL;
    switch(act) {
        case EGWPA_NONE:
            // should not happen
            break;

        case EGWPA_POSITION:{
                p_property = dynamic_cast<CGraphicsProperty*>(GetProject()->GetProperties()->CreateProperty(DistanceToPositionPropertyID,"","",p_history));
                CDistanceToPositionProperty* p_dprop = dynamic_cast<CDistanceToPositionProperty*>(p_property);
                if( p_dprop ){
                    CAtom* p_atom;
                    p_atom = dynamic_cast<CAtom*>(p_list->GetSelectedObject(0));
                    if( p_atom ){
                        p_dprop->GetPointA()->AddAtom(p_atom,p_history);
                        p_dprop->SetPointB(p_atom->GetPos(),p_history);
                    }
                }
            }
            break;

        case EGWPA_DISTANCE:{
                p_property = dynamic_cast<CGraphicsProperty*>(GetProject()->GetProperties()->CreateProperty(DistancePropertyID,"","",p_history));
                CDistanceProperty* p_dprop = dynamic_cast<CDistanceProperty*>(p_property);
                if( p_dprop ){
                    CAtom* p_atom;
                    p_atom = dynamic_cast<CAtom*>(p_list->GetSelectedObject(0));
                    if( p_atom ) p_dprop->GetPointA()->AddAtom(p_atom,p_history);
                    p_atom = dynamic_cast<CAtom*>(p_list->GetSelectedObject(1));
                    if( p_atom ) p_dprop->GetPointB()->AddAtom(p_atom,p_history);
                }
            }
            break;

        case EGWPA_ANGLE:{
                p_property = dynamic_cast<CGraphicsProperty*>(GetProject()->GetProperties()->CreateProperty(AnglePropertyID,"","",p_history));
                CAngleProperty* p_dprop = dynamic_cast<CAngleProperty*>(p_property);
                if( p_dprop ){
                    CAtom* p_atom;
                    p_atom = dynamic_cast<CAtom*>(p_list->GetSelectedObject(0));
                    if( p_atom ) p_dprop->GetPointA()->AddAtom(p_atom,p_history);
                    p_atom = dynamic_cast<CAtom*>(p_list->GetSelectedObject(1));
                    if( p_atom ) p_dprop->GetPointB()->AddAtom(p_atom,p_history);
                    p_atom = dynamic_cast<CAtom*>(p_list->GetSelectedObject(2));
                    if( p_atom ) p_dprop->GetPointC()->AddAtom(p_atom,p_history);
                }
            }
            break;

        case EGWPA_TORSION:{
                p_property = dynamic_cast<CGraphicsProperty*>(GetProject()->GetProperties()->CreateProperty(TorsionPropertyID,"","",p_history));
                CTorsionProperty* p_dprop = dynamic_cast<CTorsionProperty*>(p_property);
                if( p_dprop ){
                    CAtom* p_atom;
                    p_atom = dynamic_cast<CAtom*>(p_list->GetSelectedObject(0));
                    if( p_atom ) p_dprop->GetPointA()->AddAtom(p_atom,p_history);
                    p_atom = dynamic_cast<CAtom*>(p_list->GetSelectedObject(1));
                    if( p_atom ) p_dprop->GetPointB()->AddAtom(p_atom,p_history);
                    p_atom = dynamic_cast<CAtom*>(p_list->GetSelectedObject(2));
                    if( p_atom ) p_dprop->GetPointC()->AddAtom(p_atom,p_history);
                    p_atom = dynamic_cast<CAtom*>(p_list->GetSelectedObject(3));
                    if( p_atom ) p_dprop->GetPointD()->AddAtom(p_atom,p_history);
                }
            }
            break;
    }

    if( p_property ){
        // create graphical representation
        p_property->CreateGraphicsObject(p_history);

    } else {
        GetProject()->TextNotification(ETNT_WARNING,tr("unable to set property - property was not created correctly"),3000);
        return;
    }

    GetProject()->EndChangeWH();

    if( p_property != NULL ){
        p_property->OpenObjectDesigner();
    }
}

//------------------------------------------------------------------------------

void CGeometryWorkPanel::RestrainDAT(void)
{
    CSelectionList* p_list = SelRequest->GetSelectionList();

    // these two conditions are necessary as SelectionCompleted is also called in response to
    // geometry change

    if( p_list == NULL ) return;
    if( p_list->GetStatus() != ESS_SELECTED_OBJECTS_END ){
        GetProject()->TextNotification(ETNT_WARNING,tr("unable to restrain - selection was not finished"),3000);
        return;
    }

    if( GetProject()->GetActiveStructure() == NULL ){
        GetProject()->TextNotification(ETNT_WARNING,tr("unable to restrain - no structure is active"),3000);
        return;
    }

    EGeometryWorkPanelAction act = static_cast<EGeometryWorkPanelAction>(ButtonGroup->checkedId());
    if( act == EGWPA_NONE ) return;

    // check if all objects are atoms
    int acount = 0;
    for(int i=0; i < p_list->NumOfSelectedObjects(); i++ ){
        CProObject* p_selobj = p_list->GetSelectedObject(i);
        CAtom* p_atom = dynamic_cast<CAtom*>(p_selobj);
        if( p_atom == NULL ){
            GetProject()->TextNotification(ETNT_WARNING,tr("unable to restrain - all selected objects must be atoms"),3000);
            return;
        }
        if( p_atom->GetStructure() == GetProject()->GetActiveStructure() ){
            acount++;
        }
    }

    if( acount == 0 ){
        GetProject()->TextNotification(ETNT_WARNING,tr("unable to restrain - none of selected atoms is from the active structure"),3000);
        return;
    }

    CHistoryNode* p_history = GetProject()->BeginChangeWH(EHCL_TOPOLOGY,"restraint");
    if( p_history == NULL ){
        GetProject()->TextNotification(ETNT_WARNING,tr("unable to restrain - geometry changes are prohibited"),3000);
        return;
    }

    // create property
    CGraphicsProperty* p_property = NULL;
    switch(act) {
        case EGWPA_NONE:
            // should not happen
            break;

        case EGWPA_POSITION:{
                p_property = dynamic_cast<CGraphicsProperty*>(GetProject()->GetProperties()->CreateProperty(DistanceToPositionPropertyID,"","",p_history));
                CDistanceToPositionProperty* p_dprop = dynamic_cast<CDistanceToPositionProperty*>(p_property);
                if( p_dprop ){
                    CAtom* p_atom;
                    p_atom = dynamic_cast<CAtom*>(p_list->GetSelectedObject(0));
                    if( p_atom ){
                        p_dprop->GetPointA()->AddAtom(p_atom,p_history);
                        p_dprop->SetPointB(p_atom->GetPos(),p_history);
                    }
                }
            }
            break;

        case EGWPA_DISTANCE:{
                p_property = dynamic_cast<CGraphicsProperty*>(GetProject()->GetProperties()->CreateProperty(DistancePropertyID,"","",p_history));
                CDistanceProperty* p_dprop = dynamic_cast<CDistanceProperty*>(p_property);
                if( p_dprop ){
                    CAtom* p_atom;
                    p_atom = dynamic_cast<CAtom*>(p_list->GetSelectedObject(0));
                    if( p_atom ) p_dprop->GetPointA()->AddAtom(p_atom,p_history);
                    p_atom = dynamic_cast<CAtom*>(p_list->GetSelectedObject(1));
                    if( p_atom ) p_dprop->GetPointB()->AddAtom(p_atom,p_history);
                }
            }
            break;

        case EGWPA_ANGLE:{
                p_property = dynamic_cast<CGraphicsProperty*>(GetProject()->GetProperties()->CreateProperty(AnglePropertyID,"","",p_history));
                CAngleProperty* p_dprop = dynamic_cast<CAngleProperty*>(p_property);
                if( p_dprop ){
                    CAtom* p_atom;
                    p_atom = dynamic_cast<CAtom*>(p_list->GetSelectedObject(0));
                    if( p_atom ) p_dprop->GetPointA()->AddAtom(p_atom,p_history);
                    p_atom = dynamic_cast<CAtom*>(p_list->GetSelectedObject(1));
                    if( p_atom ) p_dprop->GetPointB()->AddAtom(p_atom,p_history);
                    p_atom = dynamic_cast<CAtom*>(p_list->GetSelectedObject(2));
                    if( p_atom ) p_dprop->GetPointC()->AddAtom(p_atom,p_history);
                }
            }
            break;

        case EGWPA_TORSION:{
                p_property = dynamic_cast<CGraphicsProperty*>(GetProject()->GetProperties()->CreateProperty(TorsionPropertyID,"","",p_history));
                CTorsionProperty* p_dprop = dynamic_cast<CTorsionProperty*>(p_property);
                if( p_dprop ){
                    CAtom* p_atom;
                    p_atom = dynamic_cast<CAtom*>(p_list->GetSelectedObject(0));
                    if( p_atom ) p_dprop->GetPointA()->AddAtom(p_atom,p_history);
                    p_atom = dynamic_cast<CAtom*>(p_list->GetSelectedObject(1));
                    if( p_atom ) p_dprop->GetPointB()->AddAtom(p_atom,p_history);
                    p_atom = dynamic_cast<CAtom*>(p_list->GetSelectedObject(2));
                    if( p_atom ) p_dprop->GetPointC()->AddAtom(p_atom,p_history);
                    p_atom = dynamic_cast<CAtom*>(p_list->GetSelectedObject(3));
                    if( p_atom ) p_dprop->GetPointD()->AddAtom(p_atom,p_history);
                }
            }
            break;
    }

    CRestraint* p_rst = NULL;
    if( p_property ){
        // create graphical representation
        p_property->CreateGraphicsObject(p_history);

        // create restraint
        p_rst = GetProject()->GetActiveStructure()->GetRestraints()->CreateRestraint(p_property->GetName(),"",p_history);
        p_rst->SetProperty(p_property,p_history);

        switch(act) {
            case EGWPA_NONE:
                // should not happen
                break;

            case EGWPA_POSITION:
                p_rst->SetForceConstant(10.0,p_history);
                break;

            case EGWPA_DISTANCE:
                p_rst->SetForceConstant(10.0,p_history);
                break;

            case EGWPA_ANGLE:
                p_rst->SetForceConstant(100.0,p_history);
                break;

            case EGWPA_TORSION:
                p_rst->SetForceConstant(100.0,p_history);
                break;
        }

    } else {
        GetProject()->TextNotification(ETNT_WARNING,tr("unable to restrain - property was not created correctly"),3000);
        return;
    }

    GetProject()->EndChangeWH();

    if( p_rst != NULL ){
        p_rst->OpenObjectDesigner();
    }
}

//------------------------------------------------------------------------------

void CGeometryWorkPanel::ModelDblClick(const QModelIndex& index)
{
    CProObject* p_obj = dynamic_cast<CProObject*>(Model->GetItem(index));
    if( p_obj == NULL ) return;
    p_obj->OpenObjectDesigner();
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

void CGeometryWorkPanel::PhysicalQuantityUnitChange(void)
{
    if( SelRequest->IsCompleted() ) {
        SelectionCompleted();
    } else {
        SelectionInitialized();
    }
}

//------------------------------------------------------------------------------

void CGeometryWorkPanel::ProjectLockChanged(EHistoryChangeMessage message)
{
    if( message != EHCM_LOCK_LEVEL ) return;

    CSelectionList* p_list = SelRequest->GetSelectionList();

    bool set = false;
    if( p_list != NULL ) {
        if( p_list->GetStatus() == ESS_SELECTED_OBJECTS_END ) set = true;
    }

    WidgetUI.PBRestrainDAT->setEnabled(! GetProject()->GetHistory()->IsLocked(EHCL_TOPOLOGY) && set);
    WidgetUI.PBPropertyDAT->setEnabled(! GetProject()->GetHistory()->IsLocked(EHCL_PROPERTY) && set);
    WidgetUI.PBLabelDAT->setEnabled(! GetProject()->GetHistory()->IsLocked(EHCL_GRAPHICS) && set);
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================




