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

#include <ObjManipMouseDriver.hpp>
#include <ErrorSystem.hpp>
#include <Manipulator.hpp>
#include <SelectionList.hpp>
#include <SelectionRequest.hpp>
#include <GraphicsObjectListSelection.hpp>
#include <MouseHandler.hpp>
#include <GraphicsObject.hpp>
#include <HistoryList.hpp>
#include <GraphicsView.hpp>
#include <GraphicsViewList.hpp>
#include <MouseDriverSetup.hpp>

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

CObjManipMouseDriver::CObjManipMouseDriver(CMouseHandler* p_handler)
    : CMouseDriver(p_handler)
{
    ResetManipulation();

    Completed = false;

    // prepare new selection request
    SelRequest = new CSelectionRequest(NULL,"object manip");

    connect(SelRequest,SIGNAL(OnSelectionInitialized(void)),
            this,SLOT(RequestInitialized(void)));
    connect(SelRequest,SIGNAL(OnSelectionChanged(void)),
            this,SLOT(SelectionChanged(void)));
    connect(SelRequest,SIGNAL(OnDetached(void)),
            this,SLOT(RequestDetached(void)));

    SelRequest->SetRequest(GetSelection(),&SH_GraphicsObjectList,"to manipulate graphical object");
}

//------------------------------------------------------------------------------

CObjManipMouseDriver::~CObjManipMouseDriver(void)
{
    if( SelRequest ) delete SelRequest;
    SelRequest = NULL;
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

void CObjManipMouseDriver::MousePressEvent(QMouseEvent* p_event)
{
    if( p_event == NULL ) {
        ES_ERROR("p_event is NULL");
        return;
    }

    if( GetManipulator() == NULL ) return;

    // backup previous action
    EMouseAction oldaction = Action;

    // encode mouse buttons
    EncodeMouseButtonsPress(p_event);

    // execute selected action
    MouseX = p_event->x();
    MouseY = p_event->y();

    switch(Action) {
        case EMA_NOTHING:
            break;

        case EMA_SELECT:
            SelectObject(MouseX,MouseY);
            break;

        case EMA_MOVE:
        case EMA_ROTATE:
            StartX = p_event->x();
            StartY = p_event->y();
            break;
        default:
            ES_ERROR("not implemented");
            break;
    }

    if( (oldaction == EMA_NOTHING) && ( (Action==EMA_MOVE) || (Action==EMA_ROTATE) ) ) {
        GetManipulator()->StartManipulation();
    }
}

//------------------------------------------------------------------------------

void CObjManipMouseDriver::MouseMoveEvent(QMouseEvent* p_event)
{
    if( p_event == NULL ) {
        ES_ERROR("p_event is NULL");
        return;
    }

    if( GetManipulator() == NULL ) return;
    if( GetActiveView() == NULL ) return;

    if( Action == EMA_NOTHING ){
        MousePressEvent(p_event);
    }

    CPoint manip;

    switch(Action) {
        case EMA_NOTHING:
        case EMA_SELECT:
            return;

        case EMA_MOVE:
            switch(ManipLevel) {
                case EML_YZ:
                    manip.x =  0;
                    manip.y =  CMouseDriverSetup::MSensitivity.y * (p_event->x() - StartX);
                    manip.z = -CMouseDriverSetup::MSensitivity.z * (p_event->y() - StartY);
                    break;
                case EML_X:
                    manip.x = CMouseDriverSetup::MSensitivity.x * (p_event->y() - StartY);
                    manip.y = 0;
                    manip.z = 0;
                    break;
            }
            GetManipulator()->Move(manip,GetActiveView());
            break;

        case EMA_ROTATE:
            switch(ManipLevel) {
                case EML_YZ:
                    manip.x = 0;
                    manip.z = CMouseDriverSetup::RSensitivity.z * (p_event->x() - StartX);
                    manip.y = -CMouseDriverSetup::RSensitivity.y * (p_event->y() - StartY);

                    break;
                case EML_X:
                    manip.x = CMouseDriverSetup::RSensitivity.x * (p_event->x() - StartX);
                    manip.y = 0;
                    manip.z = 0;
                    break;
            }
            GetManipulator()->Rotate(manip,GetActiveView());
            break;
    }

    StartX = p_event->x();
    StartY = p_event->y();

    // repaint scene
    switch(Action) {
        case EMA_ROTATE:
        case EMA_MOVE:
            GetActiveView()->Repaint(EDL_MANIP_DRAW);
            break;
        default:
            break;
    }

}

//------------------------------------------------------------------------------

void CObjManipMouseDriver::MouseReleaseEvent(QMouseEvent* p_event)
{
    if( p_event == NULL ) {
        ES_ERROR("p_event is NULL");
        return;
    }

    if( GetViews() == NULL ) return;
    if( GetActiveView() == NULL ) return;

    switch(Action){
        case EMA_NOTHING:
            break;
        case EMA_SELECT:
            GetViews()->RepaintAllViews();
            break;
        case EMA_MOVE:
        case EMA_ROTATE:
            if( GetManipulator() != NULL ){
                GetManipulator()->EndManipulation();
            }
            GetActiveView()->Repaint(EDL_USER_DRAW);
            break;
    }

    Action = EMA_NOTHING;
}

//------------------------------------------------------------------------------

void CObjManipMouseDriver::KeyPressEvent(QKeyEvent* p_event)
{
    if( p_event == NULL ) {
        ES_ERROR("p_event is NULL");
        return;
    }

    if( GetViews() == NULL ) return;

    EManipLevel newml;
    if( p_event->modifiers().testFlag(Qt::ShiftModifier) ) {
        newml = EML_X;
    } else {
        newml = EML_YZ;
    }
    if( newml != ManipLevel ){
        ManipLevel = newml;
        GetHandler()->EmitOnDriverStatusChanged();
    }

    if( p_event->key() == Qt::Key_Escape ) {
        // reset selection request
        GetSelection()->RegisterObject(CSelObject(NULL,SEL_ESC_KEY));
        GetViews()->RepaintAllViews();
    }

    p_event->ignore();
}

//------------------------------------------------------------------------------

void CObjManipMouseDriver::KeyReleaseEvent(QKeyEvent* p_event)
{
    if( p_event == NULL ) {
        INVALID_ARGUMENT("p_event is NULL");
    }

    EManipLevel newml;
    if( p_event->modifiers().testFlag(Qt::ShiftModifier) ) {
        newml = EML_X;
    } else {
        newml = EML_YZ;
    }
    if( newml != ManipLevel ){
        ManipLevel = newml;
        GetHandler()->EmitOnDriverStatusChanged();
    }

    p_event->ignore();
}

//------------------------------------------------------------------------------

void CObjManipMouseDriver::LeaveEvent(QEvent* p_event)
{
    if( p_event == NULL ) {
        INVALID_ARGUMENT("p_event is NULL");
    }
    if( GetActiveView() == NULL ) return;

    ResetManipulation();
    GetActiveView()->Repaint(EDL_USER_DRAW);
}

//------------------------------------------------------------------------------

void CObjManipMouseDriver::ResetManipulation(void)
{
    Action = EMA_NOTHING;
    ManipLevel = EML_YZ;
    GetHandler()->EmitOnDriverStatusChanged();

    StartX = 0;
    StartY = 0;
    MouseX = 0;
    MouseY = 0;
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

CManipulator* CObjManipMouseDriver::GetManipulator(void)
{
    if( Completed ){
        return(this);
    } else {
        return(GetActiveView());
    }
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

void CObjManipMouseDriver::EncodeMouseButtonsPress(QMouseEvent* p_event)
{
    Action = EMA_NOTHING;

    if( p_event == NULL ) return;

    bool            LeftButton;
    bool            MiddleButton;
    bool            RightButton;

    LeftButton = p_event->buttons().testFlag(Qt::LeftButton);
    MiddleButton = p_event->buttons().testFlag(Qt::MiddleButton);
    RightButton = p_event->buttons().testFlag(Qt::RightButton);

    if( (CMouseDriverSetup::ThreeButtonSimul == true) && (MiddleButton == false) ) {
        MiddleButton = LeftButton && RightButton;
        if( MiddleButton ) {
            LeftButton = false;
            RightButton = false;
        }
    }

    switch( CMouseDriverSetup::MouseMode ){
        case EMM_SRT:
            // selection
            if( LeftButton && ( ! MiddleButton && ! RightButton ) ) {
                Action = EMA_SELECT;
                return;
            }

            // rotate
            if( MiddleButton && ( ! LeftButton && ! RightButton ) ) {
                Action = EMA_ROTATE;
                return;
            }

            // move
            if( RightButton && ( ! MiddleButton && ! LeftButton ) ) {
                Action = EMA_MOVE;
                return;
            }
            break;
        case EMM_SELECT:
            // selection
            if( LeftButton && ( ! MiddleButton && ! RightButton ) ) {
                Action = EMA_SELECT;
                return;
            }
            break;
        case EMM_ROTATE:
            // selection
            if( LeftButton && ( ! MiddleButton && ! RightButton ) ) {
                Action = EMA_ROTATE;
                return;
            }
            break;
        case EMM_TRANSLATE:
            // selection
            if( LeftButton && ( ! MiddleButton && ! RightButton ) ) {
                Action = EMA_MOVE;
                return;
            }
            break;
        case EMM_SCALE:
            // nothing to do
            break;
    }
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

bool CObjManipMouseDriver::StartManipulation(void)
{
    if( Completed == false ) return(false);
    if( GetSelection() == NULL ) return(false);
    if( GetActiveView() == NULL ) return(false);
    if( GetHistory() == NULL ) return(false);

    CHistoryNode* p_history = GetHistory()->BeginChangeWH(EHCL_GRAPHICS,tr("manipulation with graphics objects"));
    if( p_history == NULL ) return (false);

    for(int i = 0; i < GetSelection()->NumOfSelectedObjects(); i++){
        CGraphicsObject* p_go = dynamic_cast<CGraphicsObject*>(GetSelection()->GetSelectedObject(i));
        if( p_go ){
            p_go->StartManipulation(p_history);
        }
    }

    GetHistory()->EndChangeWH();

    return(true);
}

//------------------------------------------------------------------------------

void CObjManipMouseDriver::Move(const CPoint& dmov,CGraphicsView* p_view)
{
    if( Completed == false ) return;
    if( GetSelection() == NULL ) return;

    for(int i = 0; i < GetSelection()->NumOfSelectedObjects(); i++){
        CGraphicsObject* p_go = dynamic_cast<CGraphicsObject*>(GetSelection()->GetSelectedObject(i));
        if( p_go ){
            p_go->Move(dmov,p_view);
        }
    }
}

//------------------------------------------------------------------------------

void CObjManipMouseDriver::Rotate(const CPoint& drot,CGraphicsView* p_view)
{
    if( Completed == false ) return;
    if( GetSelection() == NULL ) return;

    for(int i = 0; i < GetSelection()->NumOfSelectedObjects(); i++){
        CGraphicsObject* p_go = dynamic_cast<CGraphicsObject*>(GetSelection()->GetSelectedObject(i));
        if( p_go ){
            p_go->Rotate(drot,p_view);
        }
    }
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

void CObjManipMouseDriver::RequestInitialized(void)
{
    Completed = false;
}

//------------------------------------------------------------------------------

void CObjManipMouseDriver::SelectionChanged(void)
{
    Completed = GetSelection()->NumOfSelectedObjects() > 0;
}

//------------------------------------------------------------------------------

void CObjManipMouseDriver::RequestDetached(void)
{
    Completed = false;
    GetHandler()->ReleaseSecondaryDriver(this);
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================


