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

#include <DirManipMouseDriver.hpp>
#include <ErrorSystem.hpp>
#include <Manipulator.hpp>
#include <SelectionList.hpp>
#include <SelectionRequest.hpp>
#include <DirectionSelection.hpp>
#include <MouseHandler.hpp>
#include <Atom.hpp>
#include <Structure.hpp>
#include <AtomListHistory.hpp>
#include <Project.hpp>
#include <StructureList.hpp>
#include <GraphicsView.hpp>
#include <GraphicsViewList.hpp>
#include <MouseDriverSetup.hpp>

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

CDirManipMouseDriver::CDirManipMouseDriver(CMouseHandler* p_handler)
    : CMouseDriver(p_handler)
{
    ResetManipulation();

    Completed = false;
    Allowed = false;

    // prepare new selection request
    SelRequest = new CSelectionRequest(NULL,"dir bond manip");

    connect(SelRequest,SIGNAL(OnSelectionInitialized(void)),
            this,SLOT(RequestInitialized(void)));
    connect(SelRequest,SIGNAL(OnSelectionChanged(void)),
            this,SLOT(SelectionChanged(void)));
    connect(SelRequest,SIGNAL(OnDetached(void)),
            this,SLOT(RequestDetached(void)));

    SelRequest->SetRequest(GetSelection(),&SH_Direction,"to manipulate direction");
}

//------------------------------------------------------------------------------

CDirManipMouseDriver::~CDirManipMouseDriver(void)
{
    if( SelRequest ) delete SelRequest;
    SelRequest = NULL;
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

void CDirManipMouseDriver::MousePressEvent(QMouseEvent* p_event)
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
        Allowed = GetManipulator()->StartManipulation();
    }
}

//------------------------------------------------------------------------------

void CDirManipMouseDriver::MouseMoveEvent(QMouseEvent* p_event)
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
            manip.x =  0;
            manip.y =  CMouseDriverSetup::MSensitivity.y * (p_event->x() - StartX);
            manip.z = -CMouseDriverSetup::MSensitivity.z * (p_event->y() - StartY);
            GetManipulator()->Move(manip,GetActiveView());
            break;

        case EMA_ROTATE:
            manip.x = 0;
            manip.z = CMouseDriverSetup::RSensitivity.z * (p_event->x() - StartX);
            manip.y = -CMouseDriverSetup::RSensitivity.y * (p_event->y() - StartY);
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

void CDirManipMouseDriver::MouseReleaseEvent(QMouseEvent* p_event)
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

void CDirManipMouseDriver::KeyPressEvent(QKeyEvent* p_event)
{
    if( p_event == NULL ) {
        ES_ERROR("p_event is NULL");
        return;
    }

    if( GetViews() == NULL ) return;

    // shift modifier
    if( p_event->key() == Qt::Key_I ) {
        // invert direction
        GetSelection()->RegisterObject(CSelObject(NULL,SEL_INVERT));
        GetViews()->RepaintAllViews();
    }

    // ctrl modifier
    if( p_event->key() == Qt::Key_C ) {
        // invert direction
        GetSelection()->RegisterObject(CSelObject(NULL,SEL_COMPLETE_MOLECULES));
        GetViews()->RepaintAllViews();
    }

    if( p_event->key() == Qt::Key_Escape ) {
        // reset selection request
        GetSelection()->RegisterObject(CSelObject(NULL,SEL_ESC_KEY));
        GetViews()->RepaintAllViews();
    }

    p_event->ignore();
}

//------------------------------------------------------------------------------

void CDirManipMouseDriver::LeaveEvent(QEvent* p_event)
{
    if( p_event == NULL ) {
        INVALID_ARGUMENT("p_event is NULL");
    }
    if( GetActiveView() == NULL ) return;

    ResetManipulation();
    GetActiveView()->Repaint(EDL_USER_DRAW);
}

//------------------------------------------------------------------------------

void CDirManipMouseDriver::ResetManipulation(void)
{
    Action = EMA_NOTHING;
    GetHandler()->EmitOnDriverStatusChanged();

    StartX = 0;
    StartY = 0;
    MouseX = 0;
    MouseY = 0;
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

CManipulator* CDirManipMouseDriver::GetManipulator(void)
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

void CDirManipMouseDriver::EncodeMouseButtonsPress(QMouseEvent* p_event)
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

bool CDirManipMouseDriver::StartManipulation(void)
{
    if( Completed == false ) return(true);

    CAtom* p_atom = dynamic_cast<CAtom*>(GetSelection()->GetSelectedObject(0));
    if( p_atom == NULL ){
        Completed = false;
        return(false);
    }
    CStructure* p_mol = p_atom->GetStructure();

    CHistoryNode* p_history = p_mol->BeginChangeWH(EHCL_GEOMETRY,"direction manipulation");
    if( p_history == NULL ){
        // change is not allowed
        return(false);
    }

    // capture current coordinates
    CAtomListCoordinatesHI* p_hitem = new CAtomListCoordinatesHI(p_mol,true);
    p_history->Register(p_hitem);

    p_mol->EndChangeWH();

    // use delayed event notification
    StartEvent();

    return(true);
}

//------------------------------------------------------------------------------

void CDirManipMouseDriver::Move(const CPoint &dmov,CGraphicsView* p_view)
{
    if( Completed == false ) return;
    if( Allowed == false ) return;
    if( GetSelection() == NULL ) return;

    CSelObject* p_obj1 = GetSelection()->GetSelectedSelObject(0);
    CSelObject* p_obj2 = GetSelection()->GetSelectedSelObject(1);

    CAtom* p_at1 = NULL;
    CAtom* p_at2 = NULL;

    int start = 1;
    if( p_obj1->GetSubID() == 2 ) start = 0;

    p_at1 = dynamic_cast<CAtom*>(p_obj1->GetObject());
    p_at2 = dynamic_cast<CAtom*>(p_obj2->GetObject());

    if( (p_at1 == NULL) || (p_at2 == NULL) ) return;

    CPoint          direction;
    CTransformation trans;

    direction = p_at2->GetPos()-p_at1->GetPos();

    if( start == 1 ){
        if( (Size(direction) < 1.0) && (dmov.z < 0) ) return;
    }

    direction.Normalize();

    trans.Translate(direction*dmov.z);

    GetSelection()->GetProject()->GetStructures()->BeginGeometryUpdate();

    for(int i = start; i < GetSelection()->NumOfSelectedObjects(); i++){
        CAtom* p_atom = dynamic_cast<CAtom*>(GetSelection()->GetSelectedObject(i));
        if( p_atom ){
            p_atom->SetPos(trans.GetTransform(p_atom->GetPos()));
        }
    }

    GetSelection()->GetProject()->GetStructures()->EndGeometryUpdate(false);

    // use delayed event notification
    RaiseEvent();
}

//------------------------------------------------------------------------------

void CDirManipMouseDriver::Rotate(const CPoint &drot,CGraphicsView* p_view)
{
    if( Completed == false ) return;
    if( Allowed == false ) return;
    if( GetSelection() == NULL ) return;

    CSelObject* p_obj1 = GetSelection()->GetSelectedSelObject(0);
    CSelObject* p_obj2 = GetSelection()->GetSelectedSelObject(1);

    CAtom* p_at1 = NULL;
    CAtom* p_at2 = NULL;

    int start = 1;
    if( p_obj1->GetSubID() == 2 ) start = 0;

    p_at1 = dynamic_cast<CAtom*>(p_obj1->GetObject());
    p_at2 = dynamic_cast<CAtom*>(p_obj2->GetObject());

    if( (p_at1 == NULL) || (p_at2 == NULL) ) return;

    CPoint          direction;
    CTransformation trans;

    direction = p_at2->GetPos()-p_at1->GetPos();

    direction.Normalize();

    trans.Translate(p_at2->GetPos()*(-1.0));
    trans.Rotate(drot.y,direction);
    trans.Translate(p_at2->GetPos());

    GetSelection()->GetProject()->GetStructures()->BeginGeometryUpdate();

    for(int i = start; i < GetSelection()->NumOfSelectedObjects(); i++){
        CAtom* p_atom = dynamic_cast<CAtom*>(GetSelection()->GetSelectedObject(i));
        if( p_atom ){
            p_atom->SetPos(trans.GetTransform(p_atom->GetPos()));
        }
    }

    GetSelection()->GetProject()->GetStructures()->EndGeometryUpdate(false);

    // use delayed event notification
    RaiseEvent();
}

//------------------------------------------------------------------------------

void CDirManipMouseDriver::EndManipulation(void)
{
    // end delayed event notification
    EndEvent();
}

//------------------------------------------------------------------------------

void CDirManipMouseDriver::RespondToEvent(void)
{
    GetSelection()->GetProject()->GetStructures()->NotifyGeometryChangeTick();
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

void CDirManipMouseDriver::RequestInitialized(void)
{
    Completed = false;
}

//------------------------------------------------------------------------------

void CDirManipMouseDriver::SelectionChanged(void)
{
    Completed = GetSelection()->NumOfSelectedObjects() > 2;
    CStructure* p_mol = NULL;
    for(int i = 0; i < GetSelection()->NumOfSelectedObjects(); i++){
        CAtom* p_atom = dynamic_cast<CAtom*>(GetSelection()->GetSelectedObject(i));
        if( p_mol == NULL ){
            p_mol = p_atom->GetStructure();
        }
        if( p_mol != p_atom->GetStructure() ){
            Completed = false;
            CProject* p_project = p_atom->GetProject();
            if( p_project ){
                p_project->TextNotification(ETNT_ERROR,tr("unable to manipulate with atoms from different structures"),3000);
            }
        }
    }
}

//------------------------------------------------------------------------------

void CDirManipMouseDriver::RequestDetached(void)
{
    Completed = false;
    GetHandler()->ReleaseSecondaryDriver(this);
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================


