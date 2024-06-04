#ifndef SketchProjectWindowH
#define SketchProjectWindowH
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

#include <MainWindow.hpp>
#include <QToolBar>
#include <QWebEngineView>

#include "ui_SketchProjectWindow.h"

//------------------------------------------------------------------------------

class CProject;
class CSketchProject;
class CProjectStatusBar;
class CSketchProjectJSObject;

//------------------------------------------------------------------------------

/// sketch main window

class CSketchProjectWindow : public CMainWindow {
Q_OBJECT
public:
// constructor and destructors -------------------------------------------------
    CSketchProjectWindow(CSketchProject* p_project);
    virtual ~CSketchProjectWindow(void);

    /// connect all menu - do not call it in constructor
    // it requires restored desktop
    void ConnectAllMenu(void);

    // load data from smiles
    void LoadData(const QString& smiles);

    // save data to smiles
    const QString SaveData(void);

// EDIT ========================================================================
private:
    void ConnectEditMenu(void);

private slots:
    void UndoChange(void);
    void RedoChange(void);

    void SelectAll(void);
    void ClearSelection(void);

    void CutStructure(void);
    void CopyStructure(void);
    void PasteStructure(void);

    void DeleteAll(void);

    void PageLoaded(void);

// STRUCTURE ====================================================================
private:
    void ConnectStructureMenu(void);

private slots:
    void InsertSMILES(void);
    void ShowSMILES(void);

    void ZoomIn(void);
    void ZoomOut(void);
    void CleanStructure(void);
    void ConvertTo3D(void);
    void CopyAsSVG(void);

// section of private data -----------------------------------------------------
private:
    CSketchProject*         Project;
    QWebEngineView*         WebView;
    QWebChannel*            WebChannel;
    CProjectStatusBar*      ProjectStatusBar;
    Ui::SketchProjectWindow WidgetUI;           // ui of main window
    CSketchProjectJSObject* JSObject;

// events -----------------------------------
    // process ESC key to stop project running jobs
    virtual void keyPressEvent(QKeyEvent* p_event);

    friend class CSketchProjectJSObject;
    friend class CSketchProject;
};

//------------------------------------------------------------------------------

#endif
