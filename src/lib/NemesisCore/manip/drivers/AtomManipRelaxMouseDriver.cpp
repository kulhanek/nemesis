// =============================================================================
// NEMESIS - Molecular Modelling Package
// -----------------------------------------------------------------------------
//    Copyright (C) 2024 Petr Kulhanek, kulhanek@chemi.muni.cz
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

#include <AtomManipRelaxMouseDriver.hpp>
#include <ErrorSystem.hpp>
#include <Manipulator.hpp>
#include <SelectionList.hpp>
#include <GraphicsView.hpp>
#include <GraphicsViewList.hpp>
#include <MouseDriverSetup.hpp>
#include <MouseHandler.hpp>
#include <QPixmap>
#include <AtomSelection.hpp>
#include <Atom.hpp>
#include <Project.hpp>
#include <StructureList.hpp>

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

CAtomManipRelaxMouseDriver::CAtomManipRelaxMouseDriver(CMouseHandler* p_handler)
    : CMouseDriver(p_handler)
{
    // install cursors
    DefaultCursor           = QCursor(QPixmap(":/images/NemesisCore/cursors/DefaultCursor.svg"),5,6);
    SelectCursor            = QCursor(QPixmap(":/images/NemesisCore/cursors/SelectCursor.svg"),5,6);
    RotateYZCursor          = QCursor(QPixmap(":/images/NemesisCore/cursors/RotateYZCursor.svg"),15,15);
    RotateXCursor           = QCursor(QPixmap(":/images/NemesisCore/cursors/RotateXCursor.svg"),15,15);
    TranslateYZCursor       = QCursor(QPixmap(":/images/NemesisCore/cursors/TranslateYZCursor.svg"),15,15);
    TranslateXCursor        = QCursor(QPixmap(":/images/NemesisCore/cursors/TranslateXCursor.svg"),15,15);
    TranslateAtomYZCursor   = QCursor(QPixmap(":/images/NemesisCore/cursors/TranslateAtomYZCursor.svg"),15,15);
    TranslateAtomXCursor    = QCursor(QPixmap(":/images/NemesisCore/cursors/TranslateAtomXCursor.svg"),15,15);

    CursorAction = EMA_NOTHING;
    CursorManipLevel = EML_YZ;
    ResetManipulation();

    // prepare new selection request
    SelAtom = NULL;
    SelRequest = new CSelectionRequest(NULL,"atom manip");

    connect(SelRequest,SIGNAL(OnSelectionInitialized(void)),
            this,SLOT(RequestInitialized(void)));
    connect(SelRequest,SIGNAL(OnSelectionCompleted(void)),
            this,SLOT(SelectionCompleted(void)));
    connect(SelRequest,SIGNAL(OnDetached(void)),
            this,SLOT(RequestDetached(void)));

    SelRequest->SetRequest(GetSelection(),&SH_Atom,"to manipulate an atom position");
}

//------------------------------------------------------------------------------

CAtomManipRelaxMouseDriver::~CAtomManipRelaxMouseDriver(void)
{
    SelAtom = NULL;
    if( SelRequest ) delete SelRequest;
    SelRequest = NULL;
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

void CAtomManipRelaxMouseDriver::MousePressEvent(QMouseEvent* p_event)
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
        case EMA_SCALE:
            StartX = p_event->x();
            StartY = p_event->y();
            break;
        default:
            ES_ERROR("not implemented");
            break;
    }

    if( (oldaction == EMA_NOTHING) && ( (Action==EMA_MOVE) || (Action==EMA_ROTATE) || (Action==EMA_SCALE)) ) {
        GetManipulator()->StartManipulation();
    }

    UpdateCursor();
}

//------------------------------------------------------------------------------

void CAtomManipRelaxMouseDriver::MouseMoveEvent(QMouseEvent* p_event)
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
    double scale;

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

        case EMA_SCALE:
            switch(ManipLevel) {
                case EML_YZ:
                    scale = CMouseDriverSetup::SSensitivity * (p_event->y() - StartY);
                    break;
                case EML_X:
                    // nothing to do
                    break;
            }
            GetManipulator()->ChangeScale(scale,GetActiveView());
            break;
    }

    StartX = p_event->x();
    StartY = p_event->y();

    // repaint scene
    switch(Action) {        
        case EMA_MOVE:
        case EMA_ROTATE:
        case EMA_SCALE:
            GetActiveView()->Repaint(EDL_MANIP_DRAW);
            break;
        default:
            break;
    }

    UpdateCursor();
}

//------------------------------------------------------------------------------

void CAtomManipRelaxMouseDriver::MouseReleaseEvent(QMouseEvent* p_event)
{
    if( p_event == NULL ) {
        INVALID_ARGUMENT("p_event is NULL");
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
        case EMA_SCALE:
            if( GetManipulator() != NULL ){
                GetManipulator()->EndManipulation();
            }
            GetActiveView()->Repaint(EDL_USER_DRAW);
            break;
    }

    Action = EMA_NOTHING;
    UpdateCursor();
}

//------------------------------------------------------------------------------

void CAtomManipRelaxMouseDriver::WheelEvent(QWheelEvent* p_event)
{
    if( p_event == NULL ) {
        INVALID_ARGUMENT("p_event is NULL");
    }

    if( Action != EMA_NOTHING ) {
        return;
    }

    if( GetManipulator() == NULL ) return;
    if( GetActiveView() == NULL ) return;

    int dangle = - p_event->angleDelta().y();

    double dscalfac = CMouseDriverSetup::WheelSensitivity * CMouseDriverSetup::SSensitivity * dangle;
    GetManipulator()->ChangeScale(dscalfac,GetActiveView());

    GetActiveView()->Repaint(EDL_USER_DRAW);
}

//------------------------------------------------------------------------------

void CAtomManipRelaxMouseDriver::KeyPressEvent(QKeyEvent* p_event)
{
    if( p_event == NULL ) {
        INVALID_ARGUMENT("p_event is NULL");
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
        UpdateCursor();
        GetHandler()->EmitOnDriverStatusChanged();
    }

    // special keys
    if( p_event->key() == Qt::Key_Escape ) {
        if( GetSelection() != NULL ){
            GetSelection()->RegisterObject(CSelObject(NULL,SEL_ESC_KEY));
            GetViews()->RepaintAllViews();
        }
    }
    if( p_event->key() == Qt::Key_Enter ) {
        if( GetSelection() != NULL ){
            GetSelection()->RegisterObject(CSelObject(NULL,SEL_ENTER_KEY));
            GetViews()->RepaintAllViews();
        }
    }

    p_event->ignore();
}

//------------------------------------------------------------------------------

void CAtomManipRelaxMouseDriver::KeyReleaseEvent(QKeyEvent* p_event)
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
        UpdateCursor();
        GetHandler()->EmitOnDriverStatusChanged();
    }

    p_event->ignore();
}

//------------------------------------------------------------------------------

void CAtomManipRelaxMouseDriver::LeaveEvent(QEvent* p_event)
{
    if( p_event == NULL ) {
        INVALID_ARGUMENT("p_event is NULL");
    }
    if( GetActiveView() == NULL ) return;

    ResetManipulation();
    GetActiveView()->Repaint(EDL_USER_DRAW);
}

//------------------------------------------------------------------------------

void CAtomManipRelaxMouseDriver::ResetManipulation(void)
{
    Action = EMA_NOTHING;
    ManipLevel = EML_YZ;
    UpdateCursor();
    GetHandler()->EmitOnDriverStatusChanged();

    StartX = 0;
    StartY = 0;
    MouseX = 0;
    MouseY = 0;
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

void CAtomManipRelaxMouseDriver::UpdateCursor(void)
{
    if( (Action == CursorAction) && (ManipLevel == CursorManipLevel) ) return;
    switch(Action){
        case EMA_NOTHING:
            SetCursor(DefaultCursor);
            break;
        case EMA_SELECT:
            if( SelAtom == NULL ){
                SetCursor(SelectCursor);
            } else {
                switch(ManipLevel){
                    case EML_YZ:
                        SetCursor(TranslateAtomYZCursor);
                        break;
                    case EML_X:
                        SetCursor(TranslateAtomXCursor);
                        break;
                }
            }
            break;
        case EMA_MOVE:
            switch(ManipLevel){
                case EML_YZ:
                    SetCursor(TranslateYZCursor);
                    break;
                case EML_X:
                    SetCursor(TranslateXCursor);
                    break;
            }
            break;
        case EMA_ROTATE:
            switch(ManipLevel){
                case EML_YZ:
                    SetCursor(RotateYZCursor);
                    break;
                case EML_X:
                    SetCursor(RotateXCursor);
                    break;
            }
            break;
        case EMA_SCALE:
            break;
    }
    CursorAction = Action;
    CursorManipLevel = ManipLevel;
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

void CAtomManipRelaxMouseDriver::EncodeMouseButtonsPress(QMouseEvent* p_event)
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
            // selection
            if( LeftButton && ( ! MiddleButton && ! RightButton ) ) {
                Action = EMA_SCALE;
                return;
            }
            break;
    }
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

void CAtomManipRelaxMouseDriver::AtomMove(const CPoint &dmov,CGraphicsView* p_view)
{
    if( SelAtom == NULL ) return;

    CPoint _dmov = dmov;
    CTransformation coord = p_view->GetTrans();
    coord.Invert();
    coord.Apply(_dmov);

    GetSelection()->GetProject()->GetStructures()->BeginGeometryUpdate();
    SelAtom->SetPos(SelAtom->GetPos()+_dmov);
    GetSelection()->GetProject()->GetStructures()->EndGeometryUpdate(false);
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

void CAtomManipRelaxMouseDriver::RequestInitialized(void)
{
    SelAtom = NULL;
}

//------------------------------------------------------------------------------

void CAtomManipRelaxMouseDriver::SelectionCompleted(void)
{
    SelAtom = NULL;
    if( GetSelection()->NumOfSelectedObjects() == 1 ){
        SelAtom = dynamic_cast<CAtom*>(GetSelection()->GetSelectedObject(0));
    }
    // std::cout << Action << " " << CursorAction << std::endl;
    CursorAction = EMA_NOTHING;
    UpdateCursor();
}

//------------------------------------------------------------------------------

void CAtomManipRelaxMouseDriver::RequestDetached(void)
{
    SelAtom = NULL;
    GetHandler()->ReleaseSecondaryDriver(this);
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================


