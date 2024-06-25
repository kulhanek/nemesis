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

#include <WorkPanel.hpp>

#include <GlobalDesktop.hpp>
#include <XMLElement.hpp>
#include <PluginObject.hpp>
#include <WorkPanelList.hpp>
#include <TemplIterator.hpp>
#include <ErrorSystem.hpp>
#include <WorkPanel.hpp>
#include <Project.hpp>
#include <ProjectDesktop.hpp>
#include <MainWindow.hpp>
#include <WorkPanelDialogWidget.hpp>
#include <WorkPanelDockWidget.hpp>

#include <QSplitter>
#include <QHeaderView>
#include <QTreeView>
#include <QApplication>
#include <QVBoxLayout>
#include <QScreen>

using namespace  std;

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

CWorkPanel::CWorkPanel(CPluginObject* p_objectinfo,CProject* p_project,EWorkPanelRole role)
    : CComObject(p_objectinfo)
{
    Role = role;
    Project = p_project;
    GlobalWPSetup = false;

    // set open flag
    CXMLElement* p_ele = GetWorkPanelSetup();
    switch(Role) {
            // this cannot be automatically opened!!
        case EWPR_DIALOG:
            p_ele->SetAttribute("open",false);
            break;
        case EWPR_DESIGNER:
            p_ele->SetAttribute("open",false);
            break;
            // this might be automatically opened
        case EWPR_TOOL:
            p_ele->SetAttribute("open",true);
            break;
        case EWPR_WINDOW:
            p_ele->SetAttribute("open",true);
            break;
    }

    // -------------------------------------------
    setWindowTitle(QString(p_objectinfo->GetName()));

    // we want to destroy a window on close
    // if it is not desired, user can redefine this flag
    setAttribute(Qt::WA_DeleteOnClose,true);

    // show tooltips
    setAttribute(Qt::WA_AlwaysShowToolTips,true);

    // -------------------------------------------
    // set up popup menu
    PopMenu = new QMenu("PopupMenu", this);
    PopMenu->addAction("Save as default", this, SLOT(SaveAsDefault()));
    PopMenu->addAction("Close window", this, SLOT(close()));

    WorkPanels->WorkPanelAdded(this);
}

//------------------------------------------------------------------------------

CWorkPanel::~CWorkPanel(void)
{
    // set open flag
    CXMLElement* p_ele = GetWorkPanelSetup();
    p_ele->SetAttribute("open",false);

    switch(Container.Type) {
    case EWPT_SINGLE_WINDOW:
        break;
    case EWPT_DOCK_WINDOW:
        setParent(NULL);
        Container.Widget.DockWidget->hide();
        Container.Widget.DockWidget->deleteLater();
        Container.Widget.DockWidget = NULL;
        break;
    case EWPT_DIALOG:
        setParent(NULL);
        delete Container.Widget.Dialog;
        Container.Widget.Dialog = NULL;
        break;
    }

    WorkPanels->WorkPanelRemoved(this);
}

//------------------------------------------------------------------------------

CXMLElement* CWorkPanel::GetWorkPanelSetup(void)
{
    if( Project && (GlobalWPSetup == false) ){
        return( Project->GetDesktop()->GetWorkPanelSetup(GetType()) );
    } else {
        return( GlobalDesktop->GetWorkPanelSetup(GetType()) );
    }
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

void CWorkPanel::LoadWorkPanelSetup(void)
{
    switch(Role) {
        case EWPR_TOOL:
            SetDockWidget(true);
            break;
        case EWPR_DIALOG:
            if( Container.Widget.Dialog == NULL ) return;
            break;
        case EWPR_WINDOW:
        case EWPR_DESIGNER:
            // nothing to do
            break;
    }

    // ************ automagic ************
    // set title to work panel type description
    setWindowTitle(GetType().GetName());
    // set all tabwidgets to the first tab
    QList<QTabWidget*> tabs = findChildren<QTabWidget*>();
    foreach(QTabWidget* p_tabw, tabs){
        if( p_tabw->count() > 1 ){
            p_tabw->setCurrentIndex(0);
        }
    }
    // ************ automagic ************

    LoadWorkPanelSetup(GetWorkPanelSetup());
}

//------------------------------------------------------------------------------

void CWorkPanel::SaveAsDefault(void)
{
    // save locally
    SaveWorkPanelSetup();

    // save project or global desktop
    if( Project && (GlobalWPSetup == false) ){
        Project->GetDesktop()->SaveDefaultDesktop();
        return;
    } else {
        GlobalDesktop->SaveDefaultDesktop();
        return;
    }
}

//------------------------------------------------------------------------------

void CWorkPanel::SaveWorkPanelSetup(void)
{
    switch(Role) {
        case EWPR_TOOL:
            // nothing to do
            break;
        case EWPR_DIALOG:
            if( Container.Widget.Dialog == NULL ) return;
            break;
        case EWPR_WINDOW:
        case EWPR_DESIGNER:
            // nothing to do
            break;
    }

    SaveWorkPanelSetup(GetWorkPanelSetup());
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

void CWorkPanel::ShowFullScreen(void)
{
    switch(Role) {
        case EWPR_DIALOG:
        case EWPR_DESIGNER:
            // no full screen support
            return;
        case EWPR_TOOL:
            // undock widget
            SetDockWidget(false);
            showFullScreen();
            return;
        case EWPR_WINDOW:
            // switch to full screen directly
            showFullScreen();
            return;
    }
}

//------------------------------------------------------------------------------

void CWorkPanel::ShowNormal(void)
{
    switch(Role) {
        case EWPR_DIALOG:
        case EWPR_DESIGNER:
            // no full screen support
            return;
        case EWPR_TOOL:
            // undock widget
            showNormal();
            SetDockWidget(true);
            Show();
            return;
        case EWPR_WINDOW:
            // switch to full screen directly
            showFullScreen();
            return;
    }
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

void CWorkPanel::LoadWorkPanelSetup(CXMLElement* p_ele)
{
    if( p_ele == NULL ){
        INVALID_ARGUMENT("p_ele == NULL");
    }

    QByteArray geomarray;
    if( p_ele->GetAttribute("geom",geomarray) == true ){
        // restore geometry of window
        switch(Container.Type) {
            case EWPT_SINGLE_WINDOW:
                restoreGeometry(geomarray);
                break;
            case EWPT_DIALOG:
                if( Container.Widget.Dialog != NULL ) {
                    Container.Widget.Dialog->restoreGeometry(geomarray);
                }
                break;
            case EWPT_DOCK_WINDOW:
                if( Container.Widget.DockWidget != NULL ) {
                    Container.Widget.DockWidget->restoreGeometry(geomarray);
                }
                break;
        }
    } else {
        // default geometry
        CMainWindow* p_mainwin = NULL;
        if( Project ){
            p_mainwin = Project->GetMainWindow();
        }
        QRect geom;

        int w = width();
        int h = height();

        // no position or dimmension -> center to desktop center
        // size 1/4 of desktop size

        QRect sc;
        if( p_mainwin ){
            sc = p_mainwin->screen()->availableGeometry();
        } else {
            sc = QGuiApplication::primaryScreen()->availableGeometry();
        }

        // sanity check
        if( w <= 0 ) w = 200;
        if( h <= 0 ) h = 200;

        int l,t;
        l = (sc.width() - w)/2.0 + sc.left();
        t = (sc.height() - h)/2.0 + sc.top();

        // sanity check
        if( l <= sc.left() ) l = sc.left();
        if( t <= sc.top() )  t = sc.top();

        geom.setLeft(l);
        geom.setTop(t);
        geom.setWidth(w);
        geom.setHeight(h);

        // set geometry
        switch(Container.Type) {
            case EWPT_SINGLE_WINDOW:
                setGeometry(geom);
                break;
            case EWPT_DIALOG:
                if( Container.Widget.Dialog != NULL ) {
                    Container.Widget.Dialog->setGeometry(geom);
                }
                break;
            case EWPT_DOCK_WINDOW:
                if( Container.Widget.DockWidget != NULL ) {
                    Container.Widget.DockWidget->setGeometry(geom);
                }
                break;
        }
    }

    // extra data
    CXMLElement* p_sele = p_ele->GetFirstChildElement("data");
    if( p_sele != NULL ) {
        LoadWorkPanelSpecificData(p_sele);
    }
}

//------------------------------------------------------------------------------

void CWorkPanel::SaveWorkPanelSetup(CXMLElement* p_ele)
{
    if( p_ele == NULL ){
        INVALID_ARGUMENT("p_ele == NULL");
    }

    QByteArray geomarray;

    switch(Container.Type) {
        case EWPT_SINGLE_WINDOW:
            geomarray = saveGeometry();
            break;
        case EWPT_DIALOG:
            if( Container.Widget.Dialog != NULL ) {
                geomarray = Container.Widget.Dialog->saveGeometry();
            }
            break;
        case EWPT_DOCK_WINDOW:
            if( Container.Widget.DockWidget != NULL ) {
                geomarray = Container.Widget.DockWidget->saveGeometry();
            }
            break;
    }

    p_ele->SetAttribute("geom",geomarray);

    // extra data -----------------------------------
    // create/open new element
    CXMLElement* p_sele = p_ele->GetChildElementByPath("data",true);
    // destroy previous data
    p_sele->RemoveAllChildNodes();
    p_sele->RemoveAllAttributes();
    // save new data
    SaveWorkPanelSpecificData(p_sele);
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

void CWorkPanel::LoadWorkPanelSpecificData(CXMLElement* p_ele)
{
    if( p_ele == NULL ){
        INVALID_ARGUMENT("p_ele == NULL");
    }

    // load positions of all splitters
    CXMLElement* p_saele = p_ele->GetFirstChildElement("splitters");
    if( p_saele ){
        CXMLElement* p_sele = p_saele->GetFirstChildElement("splitter");
        while( p_sele != NULL ){
            QString name;
            p_sele->GetAttribute("name",name);
            QSplitter* p_splt = findChild<QSplitter*>(name);
            if( p_splt ){
                QByteArray barray;
                if( p_sele->GetAttribute("data",barray) == true ){
                    p_splt->restoreState(barray);
                }
            }
            p_sele = p_sele->GetNextSiblingElement("splitter");
        }
    }

    // save tab positions
    CXMLElement* p_taele = p_ele->GetFirstChildElement("tabws");
    if( p_taele ){
        CXMLElement* p_tele = p_taele->GetFirstChildElement("tabw");
        while( p_tele != NULL ){
            QString name;
            p_tele->GetAttribute("name",name);
            QTabWidget* p_tabw = findChild<QTabWidget*>(name);
            if( p_tabw ){
                int index;
                if( p_tele->GetAttribute("data",index) == true ){
                    p_tabw->setCurrentIndex(index);
                }
            }
            p_tele = p_tele->GetNextSiblingElement("tabw");
        }
    }

    // and finaly view headers
    CXMLElement* p_vaele = p_ele->GetFirstChildElement("tviews");
    if( p_vaele ){
        CXMLElement* p_vele = p_vaele->GetFirstChildElement("tview");
        while( p_vele != NULL ){
            QString name;
            p_vele->GetAttribute("name",name);
            QTreeView* p_tview = findChild<QTreeView*>(name);
            if( p_tview ){
                QHeaderView* p_header = p_tview->header();
                if( p_header ){
                    QByteArray barray;
                    if( p_vele->GetAttribute("data",barray) == true ){
                        p_header->restoreState(barray);
                    }
                }
            }
            p_vele = p_vele->GetNextSiblingElement("tview");
        }
    }
}

//------------------------------------------------------------------------------

void CWorkPanel::SaveWorkPanelSpecificData(CXMLElement* p_ele)
{
    if( p_ele == NULL ){
        INVALID_ARGUMENT("p_ele == NULL");
    }

    // save positions of all splitters
    QList<QSplitter*> spliters = findChildren<QSplitter*>();
    if( spliters.size() > 0 ){
        CXMLElement* p_saele = p_ele->CreateChildElement("splitters");
        foreach(QSplitter* p_splt, spliters){
            CXMLElement* p_sele = p_saele->CreateChildElement("splitter");
            p_sele->SetAttribute("name",p_splt->objectName());
            p_sele->SetAttribute("data",p_splt->saveState());
        }
    }

    // save tab positions
    QList<QTabWidget*> tabs = findChildren<QTabWidget*>();
    if( tabs.size() > 0 ){
        CXMLElement* p_taele = p_ele->CreateChildElement("tabws");
        foreach(QTabWidget* p_tabw, tabs){
            CXMLElement* p_tele = p_taele->CreateChildElement("tabw");
            p_tele->SetAttribute("name",p_tabw->objectName());
            p_tele->SetAttribute("data",p_tabw->currentIndex());
        }
    }

    // and finaly view headers
    QList<QTreeView*> views = findChildren<QTreeView*>();
    if( views.size() > 0 ){
        CXMLElement* p_vaele = p_ele->CreateChildElement("tviews");
        foreach(QTreeView* p_tview, views){
            QHeaderView* p_header = p_tview->header();
            if( p_header ){
                CXMLElement* p_vele = p_vaele->CreateChildElement("tview");
                p_vele->SetAttribute("name",p_tview->objectName());
                p_vele->SetAttribute("data",p_header->saveState());
            }
        }
    }
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

void CWorkPanel::Hide(void)
{
    switch(Container.Type) {
        case EWPT_SINGLE_WINDOW:
            hide();
            break;
        case EWPT_DIALOG:
            if( Container.Widget.Dialog != NULL ) {
                Container.Widget.Dialog->hide();
            }
            break;
        case EWPT_DOCK_WINDOW:
            if( Container.Widget.DockWidget != NULL ) {
                Container.Widget.DockWidget->hide();
            }
            break;
    }
}

//------------------------------------------------------------------------------

void CWorkPanel::Show(void)
{
    switch(Container.Type) {
        case EWPT_SINGLE_WINDOW:
            show();
            raise();
            break;
        case EWPT_DIALOG:
            if( Container.Widget.Dialog != NULL ) {
                Container.Widget.Dialog->show();
                Container.Widget.Dialog->raise();
            }
            break;
        case EWPT_DOCK_WINDOW:
            if( Container.Widget.DockWidget != NULL ) {
                show();
                Container.Widget.DockWidget->show();
                Container.Widget.DockWidget->raise();
            }
            break;
    }
    QApplication::processEvents();
}

//------------------------------------------------------------------------------

CProject* CWorkPanel::GetProject(void)
{
    return(Project);
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

void CWorkPanel::AddPopupMenuAction(QAction* p_act)
{
    // add action to begin of menu
    QList<QAction*> actions = PopMenu->actions();
    if (actions.isEmpty() == true) {
        PopMenu->addAction(p_act);
    } else {
        PopMenu->insertAction(actions.first(), p_act);
    }
}

//------------------------------------------------------------------------------

void CWorkPanel::AddPopupMenuSeparator(void)
{
    QList<QAction*> actions = PopMenu->actions();
    PopMenu->insertSeparator(actions.first());
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

void CWorkPanel::contextMenuEvent(QContextMenuEvent *event)
{
    if( event->reason() == QContextMenuEvent::Mouse ) {
        // handle right mouse button here
        PopMenu->popup( QCursor::pos() );
    } else {
        // pass on other buttons to base class
        QWidget::contextMenuEvent(event);
    }
}

//------------------------------------------------------------------------------

void CWorkPanel::closeEvent(QCloseEvent *event)
{
    // set open flag to closed
    CXMLElement* p_ele = GlobalDesktop->GetWorkPanelSetup(GetType());
    if( p_ele != NULL ) {
        p_ele->SetAttribute("open",false);
    }

    switch(Container.Type) {
        case EWPT_SINGLE_WINDOW:
        case EWPT_DOCK_WINDOW:
            // nothing to do
            break;
        case EWPT_DIALOG:
            if( Container.Widget.Dialog != NULL ) {
                Container.Widget.Dialog->done(Container.Widget.Dialog->result());
            }
            break;
    }
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

bool CWorkPanel::SetDockWidget(bool set)
{
    switch(Container.Type) {
        case EWPT_SINGLE_WINDOW:
            if( set == false ) return(true);
            break;
        case EWPT_DOCK_WINDOW:
            if( set == true ) return(true);
            break;
        case EWPT_DIALOG:
            return(false);
    }

    // remove dock window if requested
    if( set == false ) {
        Container.Type = EWPT_SINGLE_WINDOW;
        if( Container.Widget.DockWidget == NULL ) return(true);
        setParent(NULL);
        delete Container.Widget.DockWidget;
        Container.Widget.DockWidget = NULL;
        return(true);
    }

    CMainWindow* p_mainwin = NULL;
    if( Project ){
        p_mainwin = Project->GetMainWindow();
    }

    // create dock window
    Container.Widget.DockWidget = new CWorkPanelDockWidget(this,p_mainwin);
    Container.Type = EWPT_DOCK_WINDOW;
    Container.Widget.DockWidget->setWidget(this);
    if( p_mainwin ) p_mainwin->addDockWidget(Qt::RightDockWidgetArea, Container.Widget.DockWidget);
    Container.Widget.DockWidget->setFloating(true);
    Container.Widget.DockWidget->setObjectName(GetType().GetFullStringForm());
    if( p_mainwin ) p_mainwin->restoreDockWidget(Container.Widget.DockWidget);

    // bug in QT - set icon to empty icon, otherwise a lot of messages related to painting into
    // invalid QPainter will appear, the icon is probably set by setWidget(this)

    Container.Widget.DockWidget->setWindowIcon(QIcon());

    return(true);
}

//------------------------------------------------------------------------------

void CWorkPanel::WindowDockStatusChanged(bool docked)
{
    // nothing here
}

//------------------------------------------------------------------------------

int  CWorkPanel::ShowAsDialog(void)
{
    switch(Container.Type) {
        case EWPT_SINGLE_WINDOW:
            // nothing to do
            break;
        case EWPT_DOCK_WINDOW:
        case EWPT_DIALOG:
            // error
            return(0);
    }

    QVBoxLayout*   p_layout = NULL;

    setAttribute(Qt::WA_DeleteOnClose,false);

    CMainWindow* p_mainwin = NULL;
    if( Project ){
        p_mainwin = Project->GetMainWindow();
    }

    Container.Widget.Dialog = new CWorkPanelDialogWidget(this,p_mainwin);
    Container.Widget.Dialog->setWindowTitle(windowTitle());
    Container.Type = EWPT_DIALOG;

    p_layout = new QVBoxLayout(Container.Widget.Dialog);
    p_layout->setContentsMargins(0,0,0,0);
    p_layout->addWidget(this);
    Container.Widget.Dialog->setLayout(p_layout);

    LoadWorkPanelSetup();
   // Container.Widget.Dialog->setAttribute(Qt::WA_Moved);
    int result = Container.Widget.Dialog->exec();

    SaveWorkPanelSetup();

    p_layout->removeWidget(this);
    setParent(NULL);

    delete Container.Widget.Dialog;

    Container.Widget.Dialog = NULL;
    Container.Type = EWPT_SINGLE_WINDOW;

    setAttribute(Qt::WA_DeleteOnClose,true);

    return(result);
}

//------------------------------------------------------------------------------

void CWorkPanel::SetDialogResult(int code)
{
    if( Container.Widget.Dialog != NULL ){
        Container.Widget.Dialog->setResult(code);
    }
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

void CWorkPanel::SetChangedFlag(bool set)
{
    if( ContentsChanged == set ) return;
    ContentsChanged = set;
    QString title = windowTitle();
    if( ContentsChanged ) {
        setWindowIcon(QIcon(QString::fromUtf8(":/images/AdminWorkPanels/changed_red_n.png")));
        title += "*";
    } else {
        setWindowIcon(QIcon(QString::fromUtf8(":/images/AdminWorkPanels/changed_blue_n.png")));
        title.resize(title.size()-1);
    }
    setWindowTitle(title);
}

//------------------------------------------------------------------------------

void CWorkPanel::SetChangedFlagTrue(void)
{
    SetChangedFlag(true);
}

//------------------------------------------------------------------------------

bool CWorkPanel::IsChangedFlagSet(void)
{
    return(ContentsChanged);
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

