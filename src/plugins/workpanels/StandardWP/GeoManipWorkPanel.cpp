// =============================================================================
// NEMESIS - Molecular Modelling Package
// -----------------------------------------------------------------------------
//    Copyright (C) 2011 Jiri Klusak, anim4tor@mail.muni.cz
//                       Petr Kulhanek, kulhanek@chemi.muni.cz
//    Copyright (C) 2010 Petr Kulhanek, kulhanek@chemi.muni.cz
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
#include <Project.hpp>
#include <ProjectList.hpp>
#include <WorkPanelList.hpp>
#include <Structure.hpp>
#include <AtomList.hpp>
#include <SelectionList.hpp>
#include <SelectionRequest.hpp>

#include <PhysicalQuantities.hpp>
#include <PhysicalQuantity.hpp>
#include <GeoDescriptor.hpp>

#include "GeoManipWorkPanel.hpp"
#include "StandardWPModule.hpp"

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

QObject* GeoManipWorkPanelCB(void* p_data);

CExtUUID        GeoManipWorkPanelID(
                    "{GEO_MANIP_WORK_PANEL:b2865e28-8b2c-4e40-bb45-e9a705f8f7d6}",
                    "GeoManip");

CPluginObject   GeoManipWorkPanelObject(&StandardWPPlugin,
                    GeoManipWorkPanelID,WORK_PANEL_CAT,
                    ":/images/StandardWP/GeoManip.svg",
                    GeoManipWorkPanelCB);

// -----------------------------------------------------------------------------

QObject* GeoManipWorkPanelCB(void* p_data)
{
    CProject* p_project = static_cast<CProject*>(p_data);
    if( p_project == NULL ){
        ES_ERROR("Active project required!");
        return(NULL);
    }

    // only one instance is allowed
    QObject* p_wp = WorkPanels->Find(GeoManipWorkPanelID,p_project);
    if( p_wp == NULL ) {
        p_wp = new CGeoManipWorkPanel(p_project);
    }

    return(p_wp);
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

CGeoManipWorkPanel::CGeoManipWorkPanel(CProject* p_project)
    : CWorkPanel(&GeoManipWorkPanelObject,p_project,EWPR_TOOL)
{
    // set up ui file
    WidgetUI.setupUi(this);

    // extra setup
    DestinationBG = new QButtonGroup(this);
    DestinationBG->addButton(WidgetUI.originPB);
    DestinationBG->addButton(WidgetUI.boxCenterPB);
    WidgetUI.originPB->setChecked(true);

    AxisBG = new QButtonGroup(this);
    AxisBG->addButton(WidgetUI.xAxisPB);
    AxisBG->addButton(WidgetUI.yAxisPB);
    AxisBG->addButton(WidgetUI.zAxisPB);
    WidgetUI.xAxisPB->setChecked(true);

    PlanesBG = new QButtonGroup(this);
    PlanesBG->addButton(WidgetUI.xyPlanePB);
    PlanesBG->addButton(WidgetUI.xzPlanePB);
    PlanesBG->addButton(WidgetUI.yzPlanePB);
    WidgetUI.xyPlanePB->setChecked(true);

    AlignAxisBG = new QButtonGroup(this);
    AlignAxisBG->addButton(WidgetUI.xAxisPB_2);
    AlignAxisBG->addButton(WidgetUI.yAxisPB_2);
    AlignAxisBG->addButton(WidgetUI.zAxisPB_2);
    WidgetUI.xAxisPB_2->setChecked(true);

    ImageBG = new QButtonGroup(this);
    ImageBG->addButton(WidgetUI.byMoleculesPB);
    ImageBG->addButton(WidgetUI.byResiduesPB);
    ImageBG->addButton(WidgetUI.byAtomsPB);
    WidgetUI.byMoleculesPB->setChecked(true);

    // units
    WidgetUI.translateXSB->setPhysicalQuantity(PQ_DISTANCE);
    WidgetUI.translateYSB->setPhysicalQuantity(PQ_DISTANCE);
    WidgetUI.translateZSB->setPhysicalQuantity(PQ_DISTANCE);

    WidgetUI.vectorOriginXSB->setPhysicalQuantity(PQ_DISTANCE);
    WidgetUI.vectorOriginYSB->setPhysicalQuantity(PQ_DISTANCE);
    WidgetUI.vectorOriginZSB->setPhysicalQuantity(PQ_DISTANCE);
    WidgetUI.vectorDirectionXSB->setPhysicalQuantity(PQ_DISTANCE);
    WidgetUI.vectorDirectionYSB->setPhysicalQuantity(PQ_DISTANCE);
    WidgetUI.vectorDirectionZSB->setPhysicalQuantity(PQ_DISTANCE);
    WidgetUI.rotateAngleSB->setPhysicalQuantity(PQ_ANGLE);

    WidgetUI.planeOriginXSB->setPhysicalQuantity(PQ_DISTANCE);
    WidgetUI.planeOriginYSB->setPhysicalQuantity(PQ_DISTANCE);
    WidgetUI.planeOriginZSB->setPhysicalQuantity(PQ_DISTANCE);
    WidgetUI.planeNormalXSB->setPhysicalQuantity(PQ_DISTANCE);
    WidgetUI.planeNormalYSB->setPhysicalQuantity(PQ_DISTANCE);
    WidgetUI.planeNormalZSB->setPhysicalQuantity(PQ_DISTANCE);

    // signals
    connect(WidgetUI.translatePB, SIGNAL(clicked(bool)),
            this, SLOT(TranslateAction(void)));
    // -----------------
    connect(WidgetUI.rotatePB, SIGNAL(clicked(bool)),
            this, SLOT(RotateAction(void)));
    // -----------------
    connect(WidgetUI.mirrorPB, SIGNAL(clicked(bool)),
            this, SLOT(MirrorAction(void)));
    // -----------------
    connect(WidgetUI.alignPB, SIGNAL(clicked(bool)),
            this, SLOT(AlignAction(void)));
    // -----------------
    connect(WidgetUI.imagePB, SIGNAL(clicked(bool)),
            this, SLOT(ImageAction(void)));

    // project events
    connect(GetProject()->GetHistory(),SIGNAL(OnHistoryChanged(EHistoryChangeMessage)),
            this,SLOT(ProjectLockChanged(EHistoryChangeMessage)));
    // -----------------
    connect(GetProject()->GetSelection(),SIGNAL(OnSelectionChanged(void)),
            this,SLOT(SelectionChanged(void)));

    // load work panel setup
    LoadWorkPanelSetup();

    // update correct locks
    ProjectLockChanged(EHCM_LOCK_LEVEL);
}

// -----------------------------------------------------------------------------

CGeoManipWorkPanel::~CGeoManipWorkPanel(void)
{
    SaveWorkPanelSetup();
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

EGeometryScope CGeoManipWorkPanel::GetGeometryScope(void)
{
    EGeometryScope  scope;
    scope = static_cast<EGeometryScope>(WidgetUI.CBGeoScope->currentIndex());
    return(scope);
}

//------------------------------------------------------------------------------

void CGeoManipWorkPanel::TranslateAction(void)
{
    CStructure* p_str = GetProject()->GetActiveStructure();
    if( p_str == NULL ){
        GetProject()->TextNotification(ETNT_ERROR,tr("no structure active"),ETNT_ERROR_DELAY);
        return;
    }

    switch(WidgetUI.translateTabWidget->currentIndex()) {
        // translate to
        case 0:
            if( WidgetUI.originPB->isChecked() ){
                if( WidgetUI.massWeightedCB->isChecked() ){
                    p_str->GetAtoms()->MoveCOMToWH(EGO_ORIGIN,GetGeometryScope());
                } else {
                    p_str->GetAtoms()->MoveCOGToWH(EGO_ORIGIN,GetGeometryScope());
                }
            }
            if( WidgetUI.boxCenterPB->isChecked() ){
                if( ! p_str->PBCInfo.IsPBCEnabled() ){
                    GetProject()->TextNotification(ETNT_ERROR,tr("PBC is not set for the active structure"),ETNT_ERROR_DELAY);
                    return;
                }
                if( WidgetUI.massWeightedCB->isChecked() ){
                    p_str->GetAtoms()->MoveCOMToWH(EGO_BOX_CENTER,GetGeometryScope());
                } else {
                    p_str->GetAtoms()->MoveCOGToWH(EGO_BOX_CENTER,GetGeometryScope());
                }
            }
        break;
        // translate by vector
        case 1:{
            // compute vector
            CPoint vector;
            vector.x = WidgetUI.translateXSB->getInternalValue();
            vector.y = WidgetUI.translateYSB->getInternalValue();
            vector.z = WidgetUI.translateZSB->getInternalValue();

            // translate by vector
            p_str->GetAtoms()->MoveByWH(vector,GetGeometryScope());
            }
        break;
        // translate by PBC vector
        case 2:{
            if( ! p_str->PBCInfo.IsPBCEnabled() ){
                GetProject()->TextNotification(ETNT_ERROR,tr("PBC is not set for the active structure"),ETNT_ERROR_DELAY);
                return;
            }
            // compute vector
            CPoint vector;
            vector += p_str->PBCInfo.GetAVector()*WidgetUI.translatePBCASB->value();
            vector += p_str->PBCInfo.GetBVector()*WidgetUI.translatePBCBSB->value();
            vector += p_str->PBCInfo.GetCVector()*WidgetUI.translatePBCCSB->value();

            // translate by vector
            p_str->GetAtoms()->MoveByWH(vector,GetGeometryScope());
            }
        break;
    }
}

//------------------------------------------------------------------------------

void CGeoManipWorkPanel::RotateAction(void)
{
    CStructure* p_str = GetProject()->GetActiveStructure();
    if( p_str == NULL ){
        GetProject()->TextNotification(ETNT_ERROR,tr("no structure active"),ETNT_ERROR_DELAY);
        return;
    }

    // rotate around angle
    double angle = WidgetUI.rotateAngleSB->getInternalValue();

    // rotate
    switch(WidgetUI.rotateTabWidget->currentIndex()) {
        // rotate default axis
        case 0:
            if( WidgetUI.xAxisPB->isChecked() ){
                p_str->GetAtoms()->RotateByWH(EAT_AXIS_X,angle,GetGeometryScope());
            }
            if( WidgetUI.yAxisPB->isChecked() ){
                p_str->GetAtoms()->RotateByWH(EAT_AXIS_Y,angle,GetGeometryScope());
            }
            if( WidgetUI.zAxisPB->isChecked() ){
                p_str->GetAtoms()->RotateByWH(EAT_AXIS_Z,angle,GetGeometryScope());
            }
        break;

        // custom vector
        case 1: {
            CPoint origin;
            origin.x = WidgetUI.vectorOriginXSB->getInternalValue();
            origin.y = WidgetUI.vectorOriginYSB->getInternalValue();
            origin.z = WidgetUI.vectorOriginZSB->getInternalValue();

            CPoint vector;
            vector.x = WidgetUI.vectorDirectionXSB->getInternalValue();
            vector.y = WidgetUI.vectorDirectionYSB->getInternalValue();
            vector.z = WidgetUI.vectorDirectionZSB->getInternalValue();
            if( Size(vector) != 0.0 ) {
                p_str->GetAtoms()->RotateByWH(origin,Norm(vector),angle,GetGeometryScope());
            }
        }
        break;
    }
}

//------------------------------------------------------------------------------

void CGeoManipWorkPanel::MirrorAction(void)
{
    CStructure* p_str = GetProject()->GetActiveStructure();
    if( p_str == NULL ){
        GetProject()->TextNotification(ETNT_ERROR,tr("no structure active"),ETNT_ERROR_DELAY);
        return;
    }

    switch(WidgetUI.translateTabWidget->currentIndex()) {
        // default plane
        case 0:
            if( WidgetUI.xyPlanePB->isChecked() ){
                // mirror in XY plane
                p_str->GetAtoms()->MirrorInPlaneWH(EPT_PLANE_XY,GetGeometryScope());
            }
            if( WidgetUI.xzPlanePB->isChecked() ){
                // mirror in XZ plane
                p_str->GetAtoms()->MirrorInPlaneWH(EPT_PLANE_XZ,GetGeometryScope());
            }
            if( WidgetUI.yzPlanePB->isChecked() ){
                // mirror in YZ plane
                p_str->GetAtoms()->MirrorInPlaneWH(EPT_PLANE_YZ,GetGeometryScope());
            }
            break;
        // custom plane
        case 1:{
            // compute origin
            CPoint origin;
            origin.x = WidgetUI.vectorOriginXSB->getInternalValue();
            origin.y = WidgetUI.vectorOriginYSB->getInternalValue();
            origin.z = WidgetUI.vectorOriginZSB->getInternalValue();

            // compute normal
            CPoint normal;
            normal.x = WidgetUI.vectorDirectionXSB->getInternalValue();
            normal.y = WidgetUI.vectorDirectionYSB->getInternalValue();
            normal.z = WidgetUI.vectorDirectionZSB->getInternalValue();

            // mirror in graphic plane
            p_str->GetAtoms()->MirrorInPlaneWH(origin,Norm(normal),GetGeometryScope());
            }
            break;
    }

}

//------------------------------------------------------------------------------

void CGeoManipWorkPanel::AlignAction(void)
{
    CStructure* p_str = GetProject()->GetActiveStructure();
    if( p_str == NULL ){
        GetProject()->TextNotification(ETNT_ERROR,tr("no structure active"),ETNT_ERROR_DELAY);
        return;
    }

    if( WidgetUI.xAxisPB_2->isChecked() ){
        p_str->GetAtoms()->AlignPrincipalAxesWH(EAT_AXIS_X,GetGeometryScope());
    }
    if( WidgetUI.yAxisPB_2->isChecked() ){
        p_str->GetAtoms()->AlignPrincipalAxesWH(EAT_AXIS_Y,GetGeometryScope());
    }
    if( WidgetUI.zAxisPB_2->isChecked() ){
        p_str->GetAtoms()->AlignPrincipalAxesWH(EAT_AXIS_Z,GetGeometryScope());
    }
}

//------------------------------------------------------------------------------

void CGeoManipWorkPanel::ImageAction(void)
{
    CStructure* p_str = GetProject()->GetActiveStructure();
    if( p_str == NULL ){
        GetProject()->TextNotification(ETNT_ERROR,tr("no structure active"),ETNT_ERROR_DELAY);
        return;
    }
    if( ! p_str->PBCInfo.Is3DPBCEnabled() ){
        GetProject()->TextNotification(ETNT_ERROR,tr("PBC is not set for the active structure"),ETNT_ERROR_DELAY);
        return;
    }

    if( WidgetUI.byMoleculesPB->isChecked() ){
        p_str->GetAtoms()->ImageCoordinatesWH(EIM_BY_MOLECULES,
                                  WidgetUI.aroundOriginCB->isChecked(),
                                  WidgetUI.familiarShapeCB->isChecked(),
                                  GetGeometryScope());
    }
    if( WidgetUI.byResiduesPB->isChecked() ){
        p_str->GetAtoms()->ImageCoordinatesWH(EIM_BY_RESIDUES,
                                  WidgetUI.aroundOriginCB->isChecked(),
                                  WidgetUI.familiarShapeCB->isChecked(),
                                  GetGeometryScope());
    }
    if( WidgetUI.byAtomsPB->isChecked() ){
        p_str->GetAtoms()->ImageCoordinatesWH(EIM_BY_ATOMS,
                                  WidgetUI.aroundOriginCB->isChecked(),
                                  WidgetUI.familiarShapeCB->isChecked(),
                                  GetGeometryScope());
    }
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

void CGeoManipWorkPanel::SelectionChanged(void)
{
    CSelectionList* p_sel = GetProject()->GetSelection();
    if( p_sel == NULL ) return;

    int n_sobj = p_sel->NumOfSelectedObjects();
    WidgetUI.NumSelObjLabel->setText(QString::number(n_sobj));
}

//------------------------------------------------------------------------------

void CGeoManipWorkPanel::ProjectLockChanged(EHistoryChangeMessage message)
{
    if( message != EHCM_LOCK_LEVEL ) return;

    setEnabled( ! GetProject()->GetHistory()->IsLocked(EHCL_GEOMETRY));
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================
