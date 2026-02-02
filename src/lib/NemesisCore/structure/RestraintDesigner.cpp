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
#include <ProjectList.hpp>
#include <Project.hpp>
#include <ExtUUID.hpp>
#include <NemesisCoreModule.hpp>
#include <CategoryUUID.hpp>
#include <ErrorSystem.hpp>

#include <PeriodicTableWidget.hpp>
#include <PeriodicTable.hpp>
#include <PhysicalQuantities.hpp>
#include <Structure.hpp>
#include <StructureList.hpp>
#include <RestraintList.hpp>
#include <Residue.hpp>
#include <ContainerModel.hpp>
#include <PropertyRestraintSelection.hpp>
#include <Property.hpp>
#include <FCPhysicalQuantity.hpp>

#include <PODesignerGeneral.hpp>
#include <PODesignerFlags.hpp>
#include <PODesignerRefBy.hpp>

#include <RestraintDesigner.hpp>

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

QObject* RestraintDesignerCB(void* p_data);

CExtUUID        RestraintDesignerID(
                    "{RESTRAINT_DESIGNER:f88a491d-23ca-4c54-b305-73ebf53b4ff7}",
                    "Restraint");

CPluginObject   RestraintDesignerObject(&NemesisCorePlugin,
                    RestraintDesignerID,DESIGNER_CAT,
                    ":/images/NemesisCore/structure/Restraint.svg",
                    RestraintDesignerCB);

// -----------------------------------------------------------------------------

QObject* RestraintDesignerCB(void* p_data)
{
    CRestraint* p_rst = static_cast<CRestraint*>(p_data);
    QObject* p_object = new CRestraintDesigner(p_rst);
    return(p_object);
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

CRestraintDesigner::CRestraintDesigner(CRestraint* p_rst)
    : CProObjectDesigner(&RestraintDesignerObject,p_rst)
{
    Object = p_rst;
    WidgetUI.setupUi(this);

    // attached common setup part ----------------
    General = new CPODesignerGeneral(WidgetUI.generalGB,Object,this);
    RefBy = new CPODesignerRefBy(WidgetUI.refByTab,Object,this);

    // external events ---------------------------
    connect(Object,SIGNAL(OnStatusChanged(EStatusChanged)),
            this,SLOT(InitValues()));
    connect(Object->GetProject()->GetStructures(),SIGNAL(OnGeometryChangeTick(void)),
            this,SLOT(GeometryChanged(void)));
    connect(Object->GetProject()->GetHistory(), SIGNAL(OnHistoryChanged(EHistoryChangeMessage)),
            this, SLOT(ProjectLockChanged(EHistoryChangeMessage)));

    // units ------------------------------------
    PQ_Property = NULL;
    PQ_Force = NULL;
    WidgetUI.energyLE->setPhysicalQuantity(PQ_ENERGY);

    // LO object ---------------------------------
    Property=NULL;
    WidgetUI.propertyLO->setProject(Object->GetProject());
    WidgetUI.propertyLO->setObjectBaseMIMEType("property.indexes");
    WidgetUI.propertyLO->setSelectionHandler(&SH_PropertyRestraint);
    PropertyChanged();

    // local actions -----------------------------  
    connect(WidgetUI.buttonBox, SIGNAL(clicked (QAbstractButton *)),
            SLOT(ButtonBoxClicked(QAbstractButton *)));
    // change events
    connect(WidgetUI.targetSB, SIGNAL(valueChanged(double)),
            this, SLOT(SetChangedFlagTrue()));
    connect(WidgetUI.targetSB, SIGNAL(valueChanged(double)),
            this, SLOT(GeometryChanged()));
    connect(WidgetUI.forceSB, SIGNAL(valueChanged(double)),
            this, SLOT(SetChangedFlagTrue()));
    connect(WidgetUI.forceSB, SIGNAL(valueChanged(double)),
            this, SLOT(GeometryChanged()));
    connect(WidgetUI.enabledCB, SIGNAL(clicked(bool)),
            this, SLOT(SetChangedFlagTrue()));
    connect(WidgetUI.propertyLO, SIGNAL(OnObjectChanged()),
            this, SLOT(SetChangedFlagTrue()));
    connect(WidgetUI.propertyLO, SIGNAL(OnObjectChanged()),
            this, SLOT(PropertyChanged()));
    connect(WidgetUI.resetTargetPB, SIGNAL(clicked(bool)),
            this, SLOT(ResetTargetValue()));

    // initial values
    ProjectLockChanged(EHCM_LOCK_LEVEL);
    InitAllValues();
}

//------------------------------------------------------------------------------

CRestraintDesigner::~CRestraintDesigner(void)
{
    if( PQ_Force != NULL ) delete PQ_Force;
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

void CRestraintDesigner::ButtonBoxClicked(QAbstractButton *button)
{
    if( WidgetUI.buttonBox->standardButton(button) == QDialogButtonBox::Reset ) {
        InitAllValues();
        return;
    }

    if( WidgetUI.buttonBox->standardButton(button) == QDialogButtonBox::Apply ) {
        ApplyAllValues();
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

void CRestraintDesigner::InitAllValues(void)
{
    if( IsItChangingContent() == true ) return;

    General->InitValues();
    RefBy->InitValues();
    InitValues();

    SetChangedFlag(false);
}

//------------------------------------------------------------------------------

void CRestraintDesigner::ApplyAllValues(void)
{
    if( IsChangedFlagSet() == false ) return;

    if( Object->BeginChangeWH(EHCL_COMPOSITE,tr("restraint setup")) == NULL ) return;

    Changing = true;
        General->ApplyValues();
        RefBy->ApplyValues();
        ApplyValues();
    Changing = false;

    Object->EndChangeWH(); // this also repaint the project

    // some changes can be prohibited - reinit visualization
    InitAllValues();

    // do not repaint project here, it is done in EndChangeWH
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

void CRestraintDesigner::InitValues(void)
{
    if( IsItChangingContent() ) return;

    WidgetUI.structureW->setObject(Object->GetStructure());
    WidgetUI.enabledCB->setChecked(Object->IsEnabled());

    WidgetUI.propertyLO->setObject(Object->GetProperty());
    WidgetUI.targetSB->setInternalValue(Object->GetTargetValue());
    WidgetUI.forceSB->setInternalValue(Object->GetForceConstant());

    GeometryChanged();

    // set unchanged
    SetChangedFlag(false);
}

//------------------------------------------------------------------------------

void CRestraintDesigner::GeometryChanged(void)
{
    CProperty* p_prop = WidgetUI.propertyLO->getObject<CProperty*>();

    if( p_prop != NULL ){

        if( p_prop->IsReady() ){
            double deviation = p_prop->GetScalarDeviation(WidgetUI.targetSB->getInternalValue());
            double energy = 0.5*WidgetUI.forceSB->getInternalValue()*deviation*deviation;
            WidgetUI.currentValueLE->setInternalValue(p_prop->GetScalarValue());
            WidgetUI.deviationLE->setInternalValue(deviation);
            WidgetUI.energyLE->setInternalValue(energy);
        } else {
            WidgetUI.currentValueLE->setInternalValue(0.0);
            WidgetUI.deviationLE->setInternalValue(0.0);
            WidgetUI.energyLE->setInternalValue(0.0);
        }
    } else {
        WidgetUI.energyLE->setInternalValue(0.0);
    }

    if( WidgetUI.enabledCB->isChecked() == false ){
        WidgetUI.restraintStatusLA->setText(tr("disabled - restraint not enabled"));
    } else if( Object->GetRestraints()->IsEnabled() == false ){
        WidgetUI.restraintStatusLA->setText(tr("disabled - restraints not enabled"));
    } else if( p_prop == NULL ) {
        WidgetUI.restraintStatusLA->setText(tr("disabled - no property"));
    } else if( p_prop->IsReady() == false ){
        WidgetUI.restraintStatusLA->setText(tr("disabled - property not ready"));
    } else if( p_prop->HasGradient(Object->GetProject()->GetActiveStructure()) == false ) {
        WidgetUI.restraintStatusLA->setText(tr("disabled - no gradients for the active structure"));
    } else {
        WidgetUI.restraintStatusLA->setText(tr("enabled"));
    }
}

//------------------------------------------------------------------------------

void CRestraintDesigner::ApplyValues(void)
{
    Object->SetPropertyWH(WidgetUI.propertyLO->getObject<CProperty*>());
    Object->SetTargetValueWH(WidgetUI.targetSB->getInternalValue());
    Object->SetForceConstantWH(WidgetUI.forceSB->getInternalValue());
    Object->SetEnabledWH(WidgetUI.enabledCB->isChecked());
}

//------------------------------------------------------------------------------

void CRestraintDesigner::PropertyChanged(void)
{
    CProperty* p_prop = WidgetUI.propertyLO->getObject<CProperty*>();

    WidgetUI.resetTargetPB->setEnabled(p_prop != NULL);

    if(  p_prop == NULL ){
        if( PQ_Force != NULL ) delete PQ_Force;
        PQ_Property = NULL;
        PQ_Force = NULL;
        if( Property ){
            Property->disconnect(this);
        }
        Property = NULL;
    } else {
        PQ_Property = p_prop->GetPropertyUnit();
        PQ_Force = new CFCPhysicalQuantity(NULL,"force",PQ_Property);
        Property = p_prop;
        connect(Property,SIGNAL(OnStatusChanged(EStatusChanged)),
                this,SLOT(GeometryChanged()));
    }

    WidgetUI.currentValueLE->setPhysicalQuantity(PQ_Property);
    WidgetUI.targetSB->setPhysicalQuantity(PQ_Property);
    WidgetUI.forceSB->setPhysicalQuantity(PQ_Force);
    WidgetUI.deviationLE->setPhysicalQuantity(PQ_Property);

    if( p_prop ){
        WidgetUI.targetSB->setInternalValue(p_prop->GetScalarValue());
        if( WidgetUI.forceSB->getInternalValue() == 0 ){
            WidgetUI.forceSB->setInternalValue(5.0);
        }
    }

    GeometryChanged();
}

//------------------------------------------------------------------------------

void CRestraintDesigner::ResetTargetValue(void)
{
    CProperty* p_prop = WidgetUI.propertyLO->getObject<CProperty*>();
    if( p_prop != NULL ){
        WidgetUI.targetSB->setInternalValue(p_prop->GetScalarValue());
    }
    GeometryChanged();
    SetChangedFlagTrue();
}

//------------------------------------------------------------------------------

void CRestraintDesigner::ProjectLockChanged(EHistoryChangeMessage message)
{
    if( message != EHCM_LOCK_LEVEL ) return;

    WidgetUI.specificGB->setEnabled(! (Object->IsHistoryLocked(EHCL_TOPOLOGY) || Object->IsHistoryLocked(EHCL_GEOMETRY)));
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================
