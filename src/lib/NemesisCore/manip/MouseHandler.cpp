// =============================================================================
// NEMESIS - Molecular Modelling Package
// -----------------------------------------------------------------------------
//    Copyright (C) 2011 Petr Kulhanek, kulhanek@chemi.muni.cz
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

#include <MouseHandler.hpp>
#include <ErrorSystem.hpp>
#include <MouseDriver.hpp>
#include <typeinfo>
#include <PluginDatabase.hpp>
#include <NemesisCoreModule.hpp>
#include <CategoryUUID.hpp>
#include <Project.hpp>
#include <Graphics.hpp>

// primary drivers
#include <SceneMouseDriver.hpp>

// secondary drivers
#include <AtomManipMouseDriver.hpp>
#include <DirManipMouseDriver.hpp>
#include <MolManipMouseDriver.hpp>
#include <ObjManipMouseDriver.hpp>
#include <AtomManipRelaxMouseDriver.hpp>

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

CExtUUID        MouseHandlerID(
                    "{MOUSE_HANDLER:79ff0534-3a9b-4546-b88d-c0269044683e}",
                    "Mouse Handler");

CPluginObject   MouseHandlerObject(&NemesisCorePlugin,
                    MouseHandlerID,GENERIC_CAT,NULL);

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

CMouseHandler::CMouseHandler(CProject* p_project,bool no_index)
    : CProObject(&MouseHandlerObject,p_project,p_project,no_index)
{
    ConFlags |= EECOF_HIDDEN;

    if( p_project ){
        History = p_project->GetHistory();
        Selection = p_project->GetSelection();
        Views = p_project->GetGraphics()->GetViews();
    } else {
        History = NULL;
        Selection = NULL;
        Views = NULL;
    }

    PrimaryDriver = NULL;
    SecondaryDriver = NULL;
    ActiveDriver = NULL;

    // primary driver will be by default SCENE driver
    SetPrimaryDriver(EMD_SCENE);
}

//------------------------------------------------------------------------------

CMouseHandler::~CMouseHandler(void)
{
    ActiveDriver = NULL;

    // release drivers
    if( PrimaryDriver != NULL ) delete PrimaryDriver;
    PrimaryDriver = NULL;

    if( SecondaryDriver != NULL ) delete SecondaryDriver;
    SecondaryDriver = NULL;
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

void CMouseHandler::MousePressEvent(QMouseEvent* p_event,CGraphicsView* p_view)
{
    if( p_event == NULL ) {
        ES_ERROR("p_event is NULL");
        return;
    }
    if( ActiveDriver == NULL ) return;
    ActiveDriver->SetActiveView(p_view);
    ActiveDriver->MousePressEvent(p_event);
    ActiveDriver->SetActiveView(NULL);
}

//------------------------------------------------------------------------------

void CMouseHandler::MouseMoveEvent(QMouseEvent* p_event,CGraphicsView* p_view)
{
    if( p_event == NULL ) {
        ES_ERROR("p_event is NULL");
        return;
    }
    if( ActiveDriver == NULL ) return;
    ActiveDriver->SetActiveView(p_view);
    ActiveDriver->MouseMoveEvent(p_event);
    ActiveDriver->SetActiveView(NULL);
    emit OnMouseMove();
}

//------------------------------------------------------------------------------

void CMouseHandler::MouseReleaseEvent(QMouseEvent* p_event,CGraphicsView* p_view)
{
    if( p_event == NULL ) {
        ES_ERROR("p_event is NULL");
        return;
    }
    if( ActiveDriver == NULL ) return;
    ActiveDriver->SetActiveView(p_view);
    ActiveDriver->MouseReleaseEvent(p_event);
    ActiveDriver->SetActiveView(NULL);
}

//------------------------------------------------------------------------------

void CMouseHandler::WheelEvent(QWheelEvent* p_event,CGraphicsView* p_view)
{
    if( p_event == NULL ) {
        ES_ERROR("p_event is NULL");
        return;
    }
    if( ActiveDriver == NULL ) return;
    ActiveDriver->SetActiveView(p_view);
    ActiveDriver->WheelEvent(p_event);
    ActiveDriver->SetActiveView(NULL);
}

//------------------------------------------------------------------------------

void CMouseHandler::KeyPressEvent(QKeyEvent* p_event,CGraphicsView* p_view)
{
    if( p_event == NULL ) {
        ES_ERROR("p_event is NULL");
        return;
    }

    // possibly change active driver
    if( p_event->modifiers().testFlag(Qt::ControlModifier) ) {
        if( SecondaryDriver == ActiveDriver ){
            if( ActiveDriver != NULL ) ActiveDriver->ResetManipulation();
            ActiveDriver = PrimaryDriver;
            emit OnDriverStatusChanged();
        }
    }

    // notify driver about key
    if( ActiveDriver == NULL ) return;
    ActiveDriver->SetActiveView(p_view);
    ActiveDriver->KeyPressEvent(p_event);
    ActiveDriver->SetActiveView(NULL);
}

//------------------------------------------------------------------------------

void CMouseHandler::KeyReleaseEvent(QKeyEvent* p_event,CGraphicsView* p_view)
{
    if( p_event == NULL ) {
        ES_ERROR("p_event is NULL");
        return;
    }

    // possibly change active driver
    if( ! p_event->modifiers().testFlag(Qt::ControlModifier) ) {
        if( (SecondaryDriver != NULL) && (ActiveDriver == PrimaryDriver) ){
            if( ActiveDriver != NULL ) ActiveDriver->ResetManipulation();
            ActiveDriver = SecondaryDriver;
            emit OnDriverStatusChanged();
        }
    }

    // first notify driver about key release
    if( ActiveDriver == NULL ) return;
    ActiveDriver->SetActiveView(p_view);
    ActiveDriver->KeyReleaseEvent(p_event);
    ActiveDriver->SetActiveView(NULL);
}

//------------------------------------------------------------------------------

void CMouseHandler::EnterEvent(QEvent* p_event,CGraphicsView* p_view)
{
    if( p_event == NULL ) {
        ES_ERROR("p_event is NULL");
        return;
    }

    // first notify driver about key release
    if( ActiveDriver == NULL ) return;
    ActiveDriver->SetActiveView(p_view);
    ActiveDriver->EnterEvent(p_event);
    ActiveDriver->SetActiveView(NULL);
}

//------------------------------------------------------------------------------

void CMouseHandler::LeaveEvent(QEvent* p_event,CGraphicsView* p_view)
{
    if( p_event == NULL ) {
        ES_ERROR("p_event is NULL");
        return;
    }

    // first notify driver about key release
    if( ActiveDriver == NULL ) return;
    ActiveDriver->SetActiveView(p_view);
    ActiveDriver->LeaveEvent(p_event);
    ActiveDriver->SetActiveView(NULL);
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

CSelectionList* CMouseHandler::GetSelection(void)
{
    return(Selection);
}

//------------------------------------------------------------------------------

CHistoryList* CMouseHandler::GetHistory(void)
{
    return(History);
}

//------------------------------------------------------------------------------

CGraphicsViewList* CMouseHandler::GetViews(void)
{
    return(Views);
}

//------------------------------------------------------------------------------

EMouseDriver CMouseHandler::GetPrimaryDriver(void)
{
    return( GetDriverType(PrimaryDriver) );
}

//------------------------------------------------------------------------------

EMouseDriver CMouseHandler::GetSecondaryDriver(void)
{
    return( GetDriverType(SecondaryDriver) );
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

void CMouseHandler::SetPrimaryDriver(EMouseDriver driver)
{
    CMouseDriver* p_driver = CreateDriver(driver);

    if( ActiveDriver == PrimaryDriver ){
        if( ActiveDriver != NULL ){
            ActiveDriver->ResetManipulation();
            ActiveDriver = NULL;
        }
    }
    if( PrimaryDriver != NULL ) delete PrimaryDriver;

    if( p_driver ){
        PrimaryDriver = p_driver;
    } else {
        PrimaryDriver = new CSceneMouseDriver(this);
    }

    if( ActiveDriver == NULL ){
        ActiveDriver = PrimaryDriver;
    }

    emit OnDriverStatusChanged();
}

//------------------------------------------------------------------------------

void CMouseHandler::SetSecondaryDriver(EMouseDriver driver)
{
    CMouseDriver* p_driver = CreateDriver(driver);

    if( ActiveDriver != NULL ){
        ActiveDriver->ResetManipulation();
        ActiveDriver = NULL;
    }

    // release previous driver
    if( SecondaryDriver != NULL ) delete SecondaryDriver;

    SecondaryDriver = p_driver;

    if( SecondaryDriver != NULL ){
        ActiveDriver = SecondaryDriver;
    } else {
        ActiveDriver = PrimaryDriver;
    }

    emit OnDriverStatusChanged();
}

//------------------------------------------------------------------------------

void CMouseHandler::ReleaseSecondaryDriver(CMouseDriver* p_driver)
{
    if( SecondaryDriver != p_driver ) return;

    if( ActiveDriver == SecondaryDriver ){
        if( ActiveDriver != NULL ) ActiveDriver->ResetManipulation();
    }
    SecondaryDriver = NULL;
    ActiveDriver = PrimaryDriver;

    emit OnDriverStatusChanged();

    // delete object
    if( p_driver ) p_driver->deleteLater();
}

//------------------------------------------------------------------------------

void CMouseHandler::EmitOnDriverStatusChanged(void)
{
    emit OnDriverStatusChanged();
}

//------------------------------------------------------------------------------

CMouseDriver* CMouseHandler::CreateDriver(EMouseDriver driver)
{
    switch(driver){
        case EMD_NONE:
            return(NULL);

    // primary
        case EMD_SCENE:
            return( new CSceneMouseDriver(this) );

    // secondary
        case EMD_ATOM_MANIP:
            return( new CAtomManipMouseDriver(this) );
        case EMD_DIR_MANIP:
            return( new CDirManipMouseDriver(this) );
        case EMD_MOL_MANIP:
            return( new CMolManipMouseDriver(this) );
        case EMD_OBJ_MANIP:
            return( new CObjManipMouseDriver(this) );

        case EMD_ATOM_MANIP_RELAX:
            return( new CAtomManipRelaxMouseDriver(this) );
        default:
            ES_ERROR("not implemented mouse driver");
    }
    return(NULL);
}

//------------------------------------------------------------------------------

EMouseDriver CMouseHandler::GetDriverType(CMouseDriver* p_driver)
{
    if( p_driver == NULL ) return(EMD_NONE);

    // primary
    if( typeid(*p_driver) == typeid(CSceneMouseDriver) ) return(EMD_SCENE);

    // secondary
    if( typeid(*p_driver) == typeid(CAtomManipMouseDriver) ) return(EMD_ATOM_MANIP);
    if( typeid(*p_driver) == typeid(CDirManipMouseDriver) ) return(EMD_DIR_MANIP);
    if( typeid(*p_driver) == typeid(CMolManipMouseDriver) ) return(EMD_MOL_MANIP);
    if( typeid(*p_driver) == typeid(CObjManipMouseDriver) ) return(EMD_OBJ_MANIP);
    if( typeid(*p_driver) == typeid(CAtomManipRelaxMouseDriver) ) return(EMD_ATOM_MANIP_RELAX);

    LOGIC_ERROR("not recognized mouse driver");
}

//------------------------------------------------------------------------------

CMouseDriver* CMouseHandler::GetActiveDriver(void)
{
    return(ActiveDriver);
}

//------------------------------------------------------------------------------

bool CMouseHandler::IsSecondaryDriverActive(void)
{
    return( (ActiveDriver != NULL) && (ActiveDriver == SecondaryDriver) );
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================


