#ifndef MouseHandlerH
#define MouseHandlerH
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

#include <NemesisCoreMainHeader.hpp>
#include <QKeyEvent>
#include <QMouseEvent>
#include <ProObject.hpp>

// -----------------------------------------------------------------------------

class CMouseDriver;
class CSelectionList;
class CGraphicsProfileList;
class CHistoryList;
class CGraphicsView;
class CGraphicsViewList;

// -----------------------------------------------------------------------------

enum EMouseDriver{
    EMD_NONE,
    // primary drivers -----------------
    EMD_SCENE,             // scene driver

    // secondary drivers ---------------
    EMD_ATOM_MANIP,
    EMD_DIR_MANIP,
    EMD_MOL_MANIP,
    EMD_OBJ_MANIP,
    EMD_ATOM_MANIP_RELAX,
};

// -----------------------------------------------------------------------------

/// support of mouse movements via specialized mouse drivers

class NEMESIS_CORE_PACKAGE CMouseHandler : public CProObject {
Q_OBJECT
public:
// constructors and destructors -----------------------------------------------
    CMouseHandler(CProject* p_project,bool no_index);
    ~CMouseHandler(void);

// event receviers ------------------------------------------------------------
    void MousePressEvent(QMouseEvent* p_event,CGraphicsView* p_view);
    void MouseMoveEvent(QMouseEvent* p_event,CGraphicsView* p_view);
    void MouseReleaseEvent(QMouseEvent* p_event,CGraphicsView* p_view);

    void WheelEvent(QWheelEvent* p_event,CGraphicsView* p_view);

    void KeyPressEvent(QKeyEvent* p_event,CGraphicsView* p_view);
    void KeyReleaseEvent(QKeyEvent* p_event,CGraphicsView* p_view);

    void EnterEvent(QEvent* p_event,CGraphicsView* p_view);
    void LeaveEvent(QEvent* p_event,CGraphicsView* p_view);

// information methods ---------------------------------------------------------
    /// get associated selection list
    CSelectionList*     GetSelection(void);

    /// get history list
    CHistoryList*       GetHistory(void);

    /// get view list
    CGraphicsViewList*  GetViews(void);

    /// get type of primary mouse driver
    EMouseDriver        GetPrimaryDriver(void);

    /// get type of secondary mouse driver
    EMouseDriver        GetSecondaryDriver(void);

    /// return active mouse driver
    CMouseDriver*       GetActiveDriver(void);

    /// is secondary driver active?
    bool                IsSecondaryDriverActive(void);

    /// get driver type
    EMouseDriver        GetDriverType(CMouseDriver* p_driver);

// set drivers ----------------------------------------------------------------
    /// set primary driver - if NULL is provided SRT driver will be set
    void SetPrimaryDriver(EMouseDriver driver);

    /// set secondary driver - if NULL is provided, the secondary driver is released
    void SetSecondaryDriver(EMouseDriver driver);

    /// release secondary driver by external event (e.g. another selection req)
    void ReleaseSecondaryDriver(CMouseDriver* p_driver);

    /// emit OnDriverStatusChanged signal
    void EmitOnDriverStatusChanged(void);

// signals ---------------------------------------------------------------------
signals:
    /// this signal is emitted whenever the mouse pointer is moved
    void OnMouseMove(void);

    /// this signal is emitted whenever the active driver is changed or its status was changed
    void OnDriverStatusChanged(void);

// section of private data ----------------------------------------------------
private:
    CSelectionList*         Selection;
    CHistoryList*           History;
    CGraphicsViewList*      Views;
    CMouseDriver*           PrimaryDriver;
    CMouseDriver*           SecondaryDriver;
    CMouseDriver*           ActiveDriver;

    /// driver factory
    CMouseDriver* CreateDriver(EMouseDriver driver);
};

// -----------------------------------------------------------------------------

#endif
