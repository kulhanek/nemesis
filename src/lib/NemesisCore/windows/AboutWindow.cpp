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

#include "AboutWindow.hpp"
#include <NemesisCoreMainHeader.hpp>
#include <QScreen>

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

void CAboutWindow::Register(QScriptEngine* engine)
{
    QScriptValue ctor = engine->newFunction(CAboutWindow::New);
    QScriptValue metaObject = engine->newQMetaObject(&CAboutWindow::staticMetaObject, ctor);
    engine->globalObject().setProperty("About", metaObject);
}

//------------------------------------------------------------------------------

QScriptValue CAboutWindow::New(QScriptContext *context,
                               QScriptEngine *engine)
{
    if( context->isCalledAsConstructor() == false ) {
        context->throwError("About() can be called only as a constructor\nusage: new About");
        return(engine->undefinedValue());
    }
    if( context->argumentCount() != 0 ) {
        context->throwError("Illegal number of arguments\nusage: new About");
        return(engine->undefinedValue());
    }
    CAboutWindow* p_obj = new CAboutWindow();
    return(engine->newQObject(p_obj, QScriptEngine::ScriptOwnership));
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

CAboutWindow::CAboutWindow(void)
{
    // init ui part
    WidgetUI.setupUi(this);

    // set window title
    setAttribute(Qt::WA_DeleteOnClose);
    setModal(true);

    QRect available_desktop = QGuiApplication::primaryScreen()->availableGeometry();

    // set position to middle
    // void setGeometry ( int x, int y, int w, int h )
    int x, y;
    x = (available_desktop.width() - width()) / 2;
    y = (available_desktop.height() - height()) / 2;
    setGeometry( x, y, width(), height());

    WidgetUI.VersionLB->setText(LibBuildVersion_NEMESIS);

    // show window
    show();
}

//------------------------------------------------------------------------------

CAboutWindow::~CAboutWindow(void)
{

}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================



