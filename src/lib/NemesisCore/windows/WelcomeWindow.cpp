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

#include "WelcomeWindow.hpp"

#include <QThread>
#include <QTimer>
#include <QScreen>

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

CWelcomeWindow::CWelcomeWindow(void)
    : QWidget(NULL)
{
    // init ui part
    WidgetUI.setupUi(this);

    // set window title
    setWindowTitle("NEMESIS");

    setAttribute(Qt::WA_DeleteOnClose);

    MaxProgessSteps = 0;
    CurrProgessStep = 0;

    setWindowFlags(Qt::FramelessWindowHint);
    WidgetUI.progressLabel->show();
    WidgetUI.progressBar->show();
    WidgetUI.frame->setFrameShape(QFrame::Box);

    adjustSize();

    // place welcome window to the midle of the primary screen
    // get the available size of primary screen
    QRect available_desktop = QGuiApplication::primaryScreen()->availableGeometry();

    // set position to middle
    // void setGeometry ( int x, int y, int w, int h )
    int x, y;
    x = (available_desktop.width() - width()) / 2;
    y = (available_desktop.height() - height()) / 2;
    setGeometry( x, y, width(), height());

    // show welcome window
    show();

    if(qApp != NULL) {
        QCoreApplication::processEvents(QEventLoop::AllEvents);
    }
}

//------------------------------------------------------------------------------

CWelcomeWindow::~CWelcomeWindow(void)
{

}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

void CWelcomeWindow::SetMaxProgressSteps(int max_steps)
{
    MaxProgessSteps = max_steps;
    WidgetUI.progressBar->setMaximum(max_steps);
}

//------------------------------------------------------------------------------

void CWelcomeWindow::IncrementProgress(const QString& text)
{
    CurrProgessStep++;
    WidgetUI.progressBar->setValue(CurrProgessStep);
    WidgetUI.progressLabel->setText(text);

    QCoreApplication::processEvents(QEventLoop::AllEvents);
    // FIXME - for testing
    QThread::msleep(10);

    if( CurrProgessStep >= MaxProgessSteps ){
        // FIXME
        // solve unity problem with app icons?
        QTimer *timer = new QTimer(this);
        connect(timer, SIGNAL(timeout()), this, SLOT(close()));
        timer->start(1000);
    }
}

//------------------------------------------------------------------------------

void CWelcomeWindow::ChangeNotification(const QString& text)
{
    WidgetUI.progressLabel->setText(text);

    QCoreApplication::processEvents(QEventLoop::AllEvents);
    // FIXME - for testing
    QThread::msleep(5);
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================
