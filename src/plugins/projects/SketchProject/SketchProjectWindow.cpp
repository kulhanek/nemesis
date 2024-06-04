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

#include "SketchProjectWindow.hpp"
#include <ProjectList.hpp>
#include <ErrorSystem.hpp>
#include <GlobalSetup.hpp>
#include <QCloseEvent>
#include <Project.hpp>
#include <WorkPanelList.hpp>
#include <DesktopSystem.hpp>
#include <ProjectStatusBar.hpp>
#include <JobScheduler.hpp>
#include <QWebEnginePage>
#include <QWebEngineSettings>
#include <QWebChannel>

#include "SketchProjectJSObject.hpp"
#include "SketchProject.hpp"

#include <iostream>

using namespace std;

//------------------------------------------------------------------------------

#define KETCHER_VERSION "1.0.b4"

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

CSketchProjectWindow::CSketchProjectWindow(CSketchProject* p_project)
    : CMainWindow(p_project)
{
    // init ui part
    WidgetUI.setupUi(this);

    // shortcuts
    Project = p_project;
    HistoryList = Project->GetHistory();

    // js engine
    JSObject = new CSketchProjectJSObject(this);
    JSObject->getInitialStructure(); // init loaded structure if any

    // setup Ketcher area and webchannel to JSObject
    WebView = new QWebEngineView(this);
    WebChannel = new QWebChannel(this);
    WebChannel->registerObject("nemesis",JSObject);
    WebView->page()->setWebChannel(WebChannel);

    // load page
    CFileName ketcher_index = GlobalSetup->GetSystemSharePath() / "ketcher" / KETCHER_VERSION / "ketcher.html";
    QUrl url = QUrl::fromLocalFile(QString(ketcher_index));

    if( ! url.isValid() ){
        ES_ERROR("url is not valid");
    }

    cout << url.toString().toStdString() << endl;

    setCentralWidget(WebView);
    WebView->load(url);

    // no scrollbars
    // FIXME
    // WebView->settings()->setAttribute(QWebEngineSettings::ShowScrollBars,false);
    WebView->setContextMenuPolicy(Qt::CustomContextMenu);

    // status bar
    ProjectStatusBar = new CProjectStatusBar(this,Project);
    setStatusBar(ProjectStatusBar);
}

//------------------------------------------------------------------------------

CSketchProjectWindow::~CSketchProjectWindow(void)
{
    WebChannel->deregisterObject(JSObject);
}

//------------------------------------------------------------------------------

void CSketchProjectWindow::ConnectAllMenu(void)
{
    // restore desktop was already called
    ConnectFileMenu();
    ConnectEditMenu();
    ConnectStructureMenu();
    ConnectSetupMenu();
    ConnectHelpMenu();
}

//------------------------------------------------------------------------------

void CSketchProjectWindow::LoadData(const QString& smiles)
{
    // layout data
    QString cleaned_mol = JSObject->cleanProject(smiles);

    // update view
    QStringList script;
    script << "var data=[";

    QStringList molsep = cleaned_mol.split("\n");
    for(int i =0; i < molsep.size(); i++){
        if( i+1 != molsep.size() ){
            script << "'" + molsep[i] + "',";
        } else {
            script << "'" + molsep[i] + "'";
        }
    }
    script << "];";
    script << "ui.loadMolecule(data.join('\\n'),false);";

    QWebEnginePage* p_mf = WebView->page();
    p_mf->runJavaScript(script.join("\n"));
}

//------------------------------------------------------------------------------

const QString CSketchProjectWindow::SaveData(void)
{
    // update data - blocking
    JSObject->updateSMILESData();

    // get data
    return(JSObject->getSMILESData());
}

//------------------------------------------------------------------------------

void CSketchProjectWindow::keyPressEvent(QKeyEvent* p_event)
{
    if( p_event->key() == Qt::Key_Escape ){
        JobScheduler->TerminateProjectRunningJobs(Project);
    }
    CMainWindow::keyPressEvent(p_event);
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================



