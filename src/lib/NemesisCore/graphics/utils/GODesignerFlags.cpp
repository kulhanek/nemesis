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

#include <ErrorSystem.hpp>
#include <NemesisCoreModule.hpp>
#include <GraphicsObject.hpp>
#include <ProObjectDesigner.hpp>
#include <Project.hpp>

#include <GODesignerFlags.hpp>

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

CGODesignerFlags::CGODesignerFlags(QWidget* p_owner,CGraphicsObject* p_object,
                                   CProObjectDesigner* p_designer)
    : QWidget(p_owner)
{
    Object = p_object;
    if( Object == NULL ){
        INVALID_ARGUMENT("p_object is NULL");
    }
    Designer = p_designer;

    // set layout --------------------------------
    if( p_owner->layout() ) delete p_owner->layout();
    QVBoxLayout* p_layout = new QVBoxLayout();
    p_layout->setContentsMargins(0,0,0,0);
    p_layout->addWidget(this);
    p_owner->setLayout(p_layout);

    // load graphics -----------------------------
    WidgetUI.setupUi(this);

    // connect signals ---------------------------
    // object related signals
    connect(WidgetUI.visibleCB, SIGNAL(clicked(bool)),
            Designer,SLOT(SetChangedFlagTrue()));
    //------------------
    connect(WidgetUI.highlightedCB, SIGNAL(clicked(bool)),
            Designer,SLOT(SetChangedFlagTrue()));
    //------------------
    connect(Object->GetProject()->GetHistory(),SIGNAL(OnHistoryChanged(EHistoryChangeMessage)),
            this,SLOT(ProjectLockChanged(EHistoryChangeMessage)));
    //------------------
    connect(Object,SIGNAL(OnStatusChanged(EStatusChanged)),
            this,SLOT(InitValues()));

    // set initial values ------------------------
    ProjectLockChanged(EHCM_LOCK_LEVEL);
    // do not init values here, the owner of this componet is
    // responsible to call InitValues explicitly
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

void CGODesignerFlags::InitValues(void)
{
    if( Designer->IsItChangingContent() ) return;

    WidgetUI.visibleCB->setChecked(Object->IsFlagSet(EPOF_VISIBLE));
    WidgetUI.highlightedCB->setChecked(Object->IsFlagSet(EPOF_SELECTED));
}

//------------------------------------------------------------------------------

void CGODesignerFlags::ApplyValues(void)
{
    CProObjectFlags flags = Object->GetFlags();
    SET_FLAG(flags,EPOF_VISIBLE,WidgetUI.visibleCB->isChecked());
    SET_FLAG(flags,EPOF_SELECTED,WidgetUI.highlightedCB->isChecked());

    Object->SetFlagsWH(flags);
}

//------------------------------------------------------------------------------

void CGODesignerFlags::LoadDesignerSetup(CXMLElement* p_ele)
{
    // nothing to be here
}

//------------------------------------------------------------------------------

void CGODesignerFlags::SaveDesignerSetup(CXMLElement* p_ele)
{
    // nothing to be here
}

//------------------------------------------------------------------------------

void CGODesignerFlags::ProjectLockChanged(EHistoryChangeMessage message)
{
    if( message != EHCM_LOCK_LEVEL ) return;
    setEnabled(! Object->IsHistoryLocked(EHCL_GRAPHICS));
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================


