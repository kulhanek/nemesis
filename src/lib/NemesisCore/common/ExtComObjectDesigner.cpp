// =============================================================================
// NEMESIS - Molecular Modelling Package
// -----------------------------------------------------------------------------
//    Copyright (C) 2010 Petr Kulhanek, kulhanek@chemi.muni.cz
//    Copyright (C) 2008-2009 Petr Kulhanek, kulhanek@enzim.hu,
//                            Jakub Stepan, xstepan3@chemi.muni.cz
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

#include <ExtComObjectDesigner.hpp>
#include <ProjectList.hpp>
#include <GlobalDesktop.hpp>
#include <ExtComObject.hpp>
#include <XMLElement.hpp>
#include <ErrorSystem.hpp>
#include <QTabWidget>
#include <QSplitter>
#include <QTreeView>
#include <QApplication>
#include <QHeaderView>
#include <QContextMenuEvent>
#include <QMessageBox>
#include <QScreen>

#if defined _WIN32 || defined __CYGWIN__
#undef GetObject
#endif

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

CExtComObjectDesigner::CExtComObjectDesigner(CPluginObject* p_pluginobject,
        CExtComObject* p_owner)
    : CComObject(p_pluginobject)
{
    Object = p_owner;

    ContentsChanged = false;
    Changing = false;

    setWindowIcon(QIcon(QString::fromUtf8(":/images/NemesisCore/designers/changed_blue_n.png")));

    // we want to destroy a window on close
    // if it is not desired, user can redefine this flag
    setAttribute(Qt::WA_DeleteOnClose,true);

    // set up popup menu
    PopMenu = new QMenu("PopupMenu", this);

    PopMenu->addAction("Save position as default", this, SLOT(savePositionAsDefault()));
    PopMenu->addAction("Close window", this, SLOT(close()));
}

//---------------------------------------------------------------------------

CExtComObjectDesigner::~CExtComObjectDesigner(void)
{
    CExtComObject* p_obj = GetExtComObject();
    if(  p_obj != NULL ) {
        p_obj->ObjectDesigner = NULL;
        p_obj->UpdateDesignerRegistration();
    }
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

CExtComObject* CExtComObjectDesigner::GetExtComObject(void)
{
    return(Object);
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

void CExtComObjectDesigner::SetChangedFlag(bool set)
{
    if( ContentsChanged == set ) return;
    ContentsChanged = set;
    QString title = windowTitle();
    if( ContentsChanged ) {
        setWindowIcon(QIcon(QString::fromUtf8(":/images/NemesisCore/designers/changed_red_n.png")));
        title += "*";
    } else {
        setWindowIcon(QIcon(QString::fromUtf8(":/images/NemesisCore/designers/changed_blue_n.png")));
        title.resize(title.size()-1);
    }
    setWindowTitle(title);
}

//------------------------------------------------------------------------------

bool CExtComObjectDesigner::IsChangedFlagSet(void)
{
    return(ContentsChanged);
}

//---------------------------------------------------------------------------

bool CExtComObjectDesigner::IsItChangingContent(void)
{
    return(Changing);
}

//------------------------------------------------------------------------------

void CExtComObjectDesigner::SetChangedFlagTrue(void)
{
    SetChangedFlag(true);
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

void CExtComObjectDesigner::LoadObjectDesignerSetup(void)
{
    if( GetExtComObject() == NULL ) return;

    // ************ automagic ************
    // set title to object type
    setWindowTitle(GetExtComObject()->GetType().GetName());
    // set all tabwidgets to the first tab
    QList<QTabWidget*> tabs = findChildren<QTabWidget*>();
    foreach(QTabWidget* p_tabw, tabs){
        if( p_tabw->count() > 1 ){
            p_tabw->setCurrentIndex(0);
        }
    }
    // ************ automagic ************

    // try to use object specific data
    CXMLElement* p_ele = GetExtComObject()->GetDesignerData();
    if( p_ele == NULL ) {
        // if not present - try to use desktop default
        p_ele = GlobalDesktop->GetObjectDesignerSetup(GetExtComObject()->GetType());
    }

    if( p_ele != NULL ) {
        LoadObjectDesignerSetup(p_ele);
    }

    // set open flag
    CXMLElement* p_dele = GetExtComObject()->GetDesignerData(true);
    p_dele->SetAttribute("open",true);
}

//------------------------------------------------------------------------------

void CExtComObjectDesigner::SaveObjectDesignerSetup(void)
{
    if( GetExtComObject() == NULL ) return;
    CXMLElement* p_ele = GetExtComObject()->GetDesignerData(true);
    SaveObjectDesignerSetup(p_ele);
}

//------------------------------------------------------------------------------

void CExtComObjectDesigner::savePositionAsDefault()
{
    // save it as desktop default
    CXMLElement* p_ele = GlobalDesktop->GetObjectDesignerSetup(GetExtComObject()->GetType());
    if( p_ele == NULL ) {
        ES_ERROR("element is NULL");
        return;
    }
    SaveObjectDesignerSetup(p_ele);
}

//------------------------------------------------------------------------------

QWidget* CExtComObjectDesigner::GetMainWindow(void)
{
    return(NULL);
}

//------------------------------------------------------------------------------

void CExtComObjectDesigner::LoadObjectDesignerSetup(CXMLElement* p_ele)
{
    if( p_ele == NULL ){
        INVALID_ARGUMENT("p_ele == NULL");
    }

    QByteArray geomarray;
    if( p_ele->GetAttribute("geom",geomarray) == true ){
        // restore geometry of window
        restoreGeometry(geomarray);
    } else {
        QRect geom;

        int w = width();
        int h = height();

        // no position or dimmension -> center to desktop center
        // size 1/4 of desktop size
        QRect sc;
        if( GetMainWindow() ){
            sc = GetMainWindow()->screen()->availableGeometry();
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
        setGeometry(geom);
    }

    // extra data
    CXMLElement* p_sele = p_ele->GetFirstChildElement("data");
    if( p_sele != NULL ) {
        LoadObjectDesignerSpecificData(p_sele);
    }
}

//------------------------------------------------------------------------------

void CExtComObjectDesigner::SaveObjectDesignerSetup(CXMLElement* p_ele)
{
    if( p_ele == NULL ){
        INVALID_ARGUMENT("p_ele == NULL");
    }

    QByteArray geomarray;
    geomarray = saveGeometry();
    p_ele->SetAttribute("geom",geomarray);

    // extra data -----------------------------------
    // create/open new element
    CXMLElement* p_sele = p_ele->GetChildElementByPath("data",true);
    // destroy previous data
    p_sele->RemoveAllChildNodes();
    p_sele->RemoveAllAttributes();
    // save new data
    SaveObjectDesignerSpecificData(p_sele);
    if( (p_sele->NumOfAttributes() == 0) && (p_sele->GetNumberOfChildNodes() == 0) ){
        // if no data - delete element
        delete p_sele;
    }
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

void CExtComObjectDesigner::LoadObjectDesignerSpecificData(CXMLElement* p_ele)
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

void CExtComObjectDesigner::SaveObjectDesignerSpecificData(CXMLElement* p_ele)
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

void CExtComObjectDesigner::AddPopupMenuAction(QAction* p_act)
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

void CExtComObjectDesigner::AddPopupMenuSeparator(void)
{
    QList<QAction*> actions = PopMenu->actions();
    PopMenu->insertSeparator(actions.first());
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

void CExtComObjectDesigner::contextMenuEvent(QContextMenuEvent *event)
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

void CExtComObjectDesigner::closeEvent(QCloseEvent *event)
{
    if( ContentsChanged ){
        int result = QMessageBox::warning(this,tr("Warning"),
                tr("The setup was changed! Do you want to close the window anyway?"),
                QMessageBox::Yes|QMessageBox::No,
                QMessageBox::No);
        if( result == QMessageBox::No ){
            event->ignore();
            return;
        }
    }

    if( GetExtComObject() != NULL ){
        CXMLElement* p_ele = GetExtComObject()->GetDesignerData(true);
        p_ele->SetAttribute("open",false);
    }
    SaveObjectDesignerSetup();

    event->accept();
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================
