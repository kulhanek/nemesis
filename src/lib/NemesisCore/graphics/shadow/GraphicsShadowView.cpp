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

#include <ErrorSystem.hpp>
#include <SmallString.hpp>
#include <XMLElement.hpp>
#include <PluginObject.hpp>
#include <ExtUUID.hpp>
#include <CategoryUUID.hpp>
#include <WorkPanelList.hpp>
#include <GraphicsView.hpp>
#include <Project.hpp>
#include <GraphicsShadowViewDockWidget.hpp>
#include <MainWindow.hpp>
#include <GlobalDesktop.hpp>
#include <GraphicsShadowControls.hpp>
#include <GraphicsShadowFSNotice.hpp>
#include <GraphicsShadowCtrlNotice.hpp>
#include <NemesisCoreModule.hpp>
#include <QGraphicsItem>
#include <GraphicsSetupProfile.hpp>
#include <GraphicsCommonGLScene.hpp>
#include <QScreen>

#include <GraphicsShadowView.hpp>

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

QObject* GraphicsShadowViewCB(void* p_data);

CExtUUID        GraphicsShadowViewID(
                    "{SHADOW_VIEW:eeffcbc3-3e6c-45fd-8fcf-9b370e2ad77d}",
                    "Shadow GraphicsView");

CPluginObject   GraphicsShadowViewObject(&NemesisCorePlugin,
                    GraphicsShadowViewID,WORK_PANEL_CAT,
                    GraphicsShadowViewCB);

// -----------------------------------------------------------------------------

QObject* GraphicsShadowViewCB(void* p_data)
{
    CGraphicsView* p_view = static_cast<CGraphicsView*>(p_data);
    if( p_view == NULL ){
        ES_ERROR("CGraphicsShadowView requires p_view");
        return(NULL);
    }

    // create instance
    CGraphicsShadowView* p_wp;
    p_wp = new CGraphicsShadowView(p_view);

    return(p_wp);
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

CGraphicsShadowView::CGraphicsShadowView(CGraphicsView* p_view)
    : CGraphicsCommonView(NULL),CComObject(&GraphicsShadowViewObject)
{
    setAttribute(Qt::WA_DeleteOnClose,true);
    setWindowTitle(tr("Shadow view"));

    // this prevents huge amount of messages starting with:
    // QPainter::begin: Paint device returned engine == 0, type: 3
    setWindowIcon(QApplication::windowIcon());

    SetGraphicsView(p_view);
    FullScreen = false;

    // create controls widget
    ControlsW = new CGraphicsShadowControls(NULL);
    MoveControlsUp = false;
    ControlsOffset = ControlsW->height();
    scene()->addWidget(ControlsW);
    int cheight = ControlsW->height();
    ControlsW->move(0,height() - cheight);
    ControlsW->resize(width(),cheight);
    ControlsW->show();

    // create FS notice widget
    FSNoticeW = new CGraphicsShadowFSNotice(NULL);
    FSNoticeW->hide();
    scene()->addWidget(FSNoticeW);

    // create Control notice widget
    CtrlNoticeW = new CGraphicsShadowCtrlNotice(NULL);
    CtrlNoticeW->hide();
    scene()->addWidget(CtrlNoticeW);

    foreach (QGraphicsItem *item,scene()->items()) {
        item->setCacheMode(QGraphicsItem::DeviceCoordinateCache);
    }

    // signals -----------------------------------
    connect(GraphicsView,SIGNAL(OnStatusChanged(EStatusChanged)),
            this,SLOT(ViewChanged(void)));
    // -------------
    connect(GraphicsView,SIGNAL(destroyed(QObject*)),
            this,SLOT(ViewDestroyed(void)));

    // -------------
    connect(ControlsW->WidgetUI.autofitPB,SIGNAL(clicked(bool)),
            this,SLOT(AutoFit(void)));
    // -------------
    connect(ControlsW->WidgetUI.saveAsDefaultPB,SIGNAL(clicked(bool)),
            this,SLOT(SavePositionAsDefault(void)));
    // -------------
    connect(ControlsW->WidgetUI.viewInfoPB,SIGNAL(clicked(bool)),
            this,SLOT(OpenObjectDesigner(void)));
    // -------------
    connect(ControlsW->WidgetUI.syncToPrimaryPB,SIGNAL(clicked(bool)),
            this,SLOT(SyncToPrimary(void)));
    // -------------
    connect(ControlsW->WidgetUI.fullScreenPB,SIGNAL(clicked(bool)),
            this,SLOT(ToggleFullScreen(void)));

    // final initialization
    SetDockWidget(true);
    LoadShadowSetup();
    ViewChanged();
    Show();

    QTimer::singleShot(1000,this,SLOT(ShowCtrlNotice(void)));
}

//------------------------------------------------------------------------------

CGraphicsShadowView::~CGraphicsShadowView()
{
    SaveShadowSetup();
    SetDockWidget(false);
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

void CGraphicsShadowView::ShowControls(void)
{
    if( MoveControlsUp == true ) return;
    MoveControlsUp = true;
    QTimer::singleShot(5,this,SLOT(ControlsViewStateUpdate(void)));
}

//------------------------------------------------------------------------------

void CGraphicsShadowView::HideControls(void)
{
    if( MoveControlsUp == false ) return;
    MoveControlsUp = false;
    QTimer::singleShot(5,this,SLOT(ControlsViewStateUpdate(void)));
}

//------------------------------------------------------------------------------

void CGraphicsShadowView::ShallHideControls(void)
{
    QPoint cpos = QCursor::pos();
    QPoint wpos = mapFromGlobal(cpos);

    if( wpos.x() < 0 || wpos.x() > width() ){
        HideControls();
        return;
    }
    if( wpos.y() < height() - ControlsW->height() || wpos.y() > height() ){
        HideControls();
        return;
    }

    QTimer::singleShot(2000,this,SLOT(ShallHideControls(void)));
}

//------------------------------------------------------------------------------

void CGraphicsShadowView::ControlsViewStateUpdate(void)
{
    if( MoveControlsUp ){
        if( ControlsOffset <= 0 ){
            ControlsOffset = 0;
            QTimer::singleShot(2000,this,SLOT(ShallHideControls(void)));
            return;
        }
        ControlsOffset--;
    } else {
        if( ControlsOffset >= ControlsW->height() ){
            ControlsOffset = ControlsW->height();
            return;
        }
        ControlsOffset++;
    }
    float opacity = 1.0 - ((float)ControlsOffset)/((float)ControlsW->height());

    ControlsW->move(0,height()-ControlsW->height()+ControlsOffset);
    ControlsW->setWindowOpacity(opacity);

    QTimer::singleShot(5,this,SLOT(ControlsViewStateUpdate(void)));
}

//------------------------------------------------------------------------------

void CGraphicsShadowView::ShowFSNotice(void)
{
    if( FSNoticeW->isVisible() == true ) return;
    FSNoticeW->move(0,(height()-FSNoticeW->height())/2);
    FSNoticeW->resize(width(),FSNoticeW->height());
    FSNoticeW->setWindowOpacity(1.0);
    FSNoticeW->show();
    QTimer::singleShot(3000,this,SLOT(FSNoticeStateUpdate(void)));
}

//------------------------------------------------------------------------------

void CGraphicsShadowView::FSNoticeStateUpdate(void)
{
    float opacity = FSNoticeW->windowOpacity();
    opacity -= 0.01;
    if( opacity <= 0 ){
        FSNoticeW->hide();
        return;
    }

    FSNoticeW->setWindowOpacity(opacity);
    QTimer::singleShot(5,this,SLOT(FSNoticeStateUpdate(void)));
}

//------------------------------------------------------------------------------

void CGraphicsShadowView::ShowCtrlNotice(void)
{
    if( CtrlNoticeW->isVisible() == true ) return;
    CtrlNoticeW->move(0,(height()-CtrlNoticeW->height())/2);
    CtrlNoticeW->resize(width(),CtrlNoticeW->height());
    CtrlNoticeW->setWindowOpacity(1.0);
    CtrlNoticeW->show();
    QTimer::singleShot(3000,this,SLOT(CtrlNoticeStateUpdate(void)));
}

//------------------------------------------------------------------------------

void CGraphicsShadowView::CtrlNoticeStateUpdate(void)
{
    float opacity = CtrlNoticeW->windowOpacity();
    opacity -= 0.01;
    if( opacity <= 0 ){
        CtrlNoticeW->hide();
        return;
    }

    CtrlNoticeW->setWindowOpacity(opacity);
    QTimer::singleShot(5,this,SLOT(CtrlNoticeStateUpdate(void)));
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

void CGraphicsShadowView::AutoFit(void)
{
    if( GraphicsView == NULL ) return;
    GraphicsView->FitScene();
}

//------------------------------------------------------------------------------

void CGraphicsShadowView::ShowFullScreen(void)
{
    if( FullScreen == true ) return;
    SetDockWidget(false);
    showFullScreen();
    FullScreen = true;
    ControlsW->WidgetUI.viewInfoPB->setEnabled(false);
}

//------------------------------------------------------------------------------

void CGraphicsShadowView::ShowNormal(void)
{
    if( FullScreen == false ) return;
    FullScreen = false;
    showNormal();
    SetDockWidget(true);
    Show();

    ControlsW->WidgetUI.viewInfoPB->setEnabled(true);
}

//------------------------------------------------------------------------------

void CGraphicsShadowView::CloseShadowView(void)
{
    SetDockWidget(false);
    deleteLater();
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

bool CGraphicsShadowView::SetDockWidget(bool set)
{
    // remove dock window if requested
    if( set == false ) {
        setParent(NULL);
        if( DockWidget ) DockWidget->deleteLater();
        DockWidget = NULL;
        return(true);
    }

    CMainWindow* p_mainwin = NULL;
    if( Project ){
        p_mainwin = Project->GetMainWindow();
    }

    // create dock window
    try {
        DockWidget = new CGraphicsShadowViewDockWidget(this,p_mainwin);
    } catch(...) {
        ES_ERROR("Unable to create CGraphicsShadowViewDockWidget");
        return(false);
    }

    if( p_mainwin ) p_mainwin->addDockWidget(Qt::RightDockWidgetArea, DockWidget);
    DockWidget->setFloating(true);
    DockWidget->setObjectName(GetType().GetFullStringForm());
    if( p_mainwin ) p_mainwin->restoreDockWidget(DockWidget);
    DockWidget->setWidget(this);

    return(true);
}

//------------------------------------------------------------------------------

void CGraphicsShadowView::Show(void)
{
    show();
    if( DockWidget != NULL ) {
        DockWidget->show();
        DockWidget->raise();
    }
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

void CGraphicsShadowView::OpenObjectDesigner(void)
{
    if( GraphicsView == NULL ) return;
    GraphicsView->OpenObjectDesigner();
}

//------------------------------------------------------------------------------

void CGraphicsShadowView::SyncToPrimary(void)
{
    if( GraphicsView == NULL ) return;
    GraphicsView->SyncWithPrimaryView(ControlsW->WidgetUI.syncToPrimaryPB->isChecked());
}

//------------------------------------------------------------------------------

void CGraphicsShadowView::ViewChanged(void)
{
    if( GraphicsView != NULL ){
        setWindowTitle(GraphicsView->GetName());
        if( DockWidget ){
            DockWidget->setWindowTitle(GraphicsView->GetName());
        }
        ControlsW->WidgetUI.viewInfoPB->setEnabled(!FullScreen);
        ControlsW->WidgetUI.syncToPrimaryPB->setEnabled(true);
        ControlsW->WidgetUI.syncToPrimaryPB->setChecked(GraphicsView->IsSynchronizedWithPrimaryView());
    } else {
        setWindowTitle(tr("-none-"));
        if( DockWidget ){
            DockWidget->setWindowTitle(tr("-none-"));
        }
        ControlsW->WidgetUI.viewInfoPB->setEnabled(false);
        ControlsW->WidgetUI.syncToPrimaryPB->setEnabled(false);
        ControlsW->WidgetUI.syncToPrimaryPB->setChecked(false);
    }
    ControlsW->WidgetUI.fullScreenPB->setChecked(FullScreen);
}

//------------------------------------------------------------------------------

void CGraphicsShadowView::ViewDestroyed(void)
{
    GraphicsView = NULL;
    ViewChanged();
    deleteLater();
}

//------------------------------------------------------------------------------

void CGraphicsShadowView::ToggleFullScreen(void)
{
    if( ControlsW->WidgetUI.fullScreenPB->isChecked() ){
        ShowFullScreen();
        HideControls();
        QTimer::singleShot(600,this,SLOT(ShowFSNotice(void)));
    } else {
        ShowNormal();
    }
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

void CGraphicsShadowView::LoadShadowSetup(void)
{
    if( GraphicsView == NULL ) return;

    // try to use object specific data
    CXMLElement* p_ele = GraphicsView->GetShadowData();
    if( p_ele == NULL ) {
        // if not present - try to use desktop default
        p_ele = GlobalDesktop->GetObjectDesignerSetup(GetType());
    }

    if( p_ele != NULL ) {
        LoadShadowSetupMain(p_ele);
    }

    // set open flag
    CXMLElement* p_dele = GraphicsView->GetShadowData(true);
    p_dele->SetAttribute("attached",true);
}

//------------------------------------------------------------------------------

void CGraphicsShadowView::SaveShadowSetup(void)
{
    if( GraphicsView == NULL ) return;
    CXMLElement* p_ele = GraphicsView->GetShadowData(true);
    SaveShadowSetupMain(p_ele);
}

//------------------------------------------------------------------------------

void CGraphicsShadowView::SavePositionAsDefault(void)
{
    // save it as desktop default
    CXMLElement* p_ele = GlobalDesktop->GetObjectDesignerSetup(GetType());
    if( p_ele == NULL ) {
        ES_ERROR("element is NULL");
        return;
    }
    SaveShadowSetupMain(p_ele);
}

//------------------------------------------------------------------------------

void CGraphicsShadowView::LoadShadowSetupMain(CXMLElement* p_ele)
{
    if( p_ele == NULL ){
        INVALID_ARGUMENT("p_ele == NULL");
    }

    QRect   geom;
    int     num;
    geom = DockWidget->geometry();

    bool    fullscreen = false;

    CMainWindow* p_mainwin = NULL;
    if( Project ){
        p_mainwin = Project->GetMainWindow();
    }

    // standard data
    if( p_ele->GetAttribute("f",fullscreen) ){
        if( p_ele->GetAttribute("l",num) ) geom.setLeft(num);
        if( p_ele->GetAttribute("t",num) ) geom.setTop(num);
        if( p_ele->GetAttribute("w",num) ) geom.setWidth(num);
        if( p_ele->GetAttribute("h",num) ) geom.setHeight(num);
    } else {
        // no position or dimmension -> center to desktop center
        // size 1/4 of desktop size
        QRect sc = p_mainwin->screen()->availableGeometry();
        int l,t;
        l = (sc.width() - width())/2;
        t = (sc.height() - height())/2;
        if( l < 0 ) l = 50;
        if( t < 0 ) t = 50;
        geom.setLeft(l);
        geom.setTop(t);
        geom.setWidth(width());
        geom.setHeight(height());
    }

    DockWidget->setGeometry(geom);
    if( fullscreen ){
        ShowFullScreen();
    }
}

//------------------------------------------------------------------------------

void CGraphicsShadowView::SaveShadowSetupMain(CXMLElement* p_ele)
{
    if( p_ele == NULL ){
        INVALID_ARGUMENT("p_ele == NULL");
    }

    if( FullScreen ){
        p_ele->SetAttribute("f",FullScreen);
    } else {
        QRect geom;
        geom = DockWidget->geometry();

        // standard data --------------------------------
        p_ele->SetAttribute("l",geom.x());
        p_ele->SetAttribute("t",geom.y());
        p_ele->SetAttribute("w",geom.width());
        p_ele->SetAttribute("h",geom.height());
        p_ele->SetAttribute("f",FullScreen);
    }
}

//------------------------------------------------------------------------------

void CGraphicsShadowView::closeEvent(QCloseEvent *event)
{
    if( GraphicsView != NULL ){
        CXMLElement* p_ele = GraphicsView->GetShadowData(true);
        p_ele->SetAttribute("attached",false);
    }
    SaveShadowSetup();
}

//------------------------------------------------------------------------------

void CGraphicsShadowView::resizeEvent(QResizeEvent *event)
{
    QSize size = event->size();
    ControlsW->move(0,size.height() - ControlsW->height() + ControlsOffset);
    ControlsW->resize(size.width(),ControlsW->height());
    CGraphicsCommonView::resizeEvent(event);
}

//------------------------------------------------------------------------------

void CGraphicsShadowView::mouseMoveEvent(QMouseEvent* p_event)
{
    CGraphicsCommonView::mouseMoveEvent(p_event);
}

//------------------------------------------------------------------------------

void CGraphicsShadowView::keyPressEvent(QKeyEvent* p_event)
{
    if( p_event->key() == Qt::Key_F1 ){
        ShowControls();
        p_event->accept();
    }
    if( p_event->key() == Qt::Key_F10 ){
        if( FullScreen ){
            ShowNormal();
            ViewChanged();
            p_event->accept();
        }
    }
    CGraphicsCommonView::keyPressEvent(p_event);
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

