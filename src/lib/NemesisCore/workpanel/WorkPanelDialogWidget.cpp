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

#include <WorkPanelDialogWidget.hpp>
#include <WorkPanel.hpp>
#include <iostream>
#include <QApplication>
#include <QScreen>

using namespace  std;

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

CWorkPanelDialogWidget::CWorkPanelDialogWidget(CWorkPanel* p_child,QWidget* p_parent)
    : QDialog(p_parent)
{
    Child = p_child;
}

//------------------------------------------------------------------------------

QByteArray CWorkPanelDialogWidget::saveGeometry(void) const
{
    QByteArray array;
    QDataStream stream(&array, QIODevice::WriteOnly);
    stream.setVersion(QDataStream::Qt_4_0);
    const quint32 magicNumber = 0x1D9D1CA;
    quint16 majorVersion = 1;
    quint16 minorVersion = 0;
    stream << magicNumber
           << majorVersion
           << minorVersion
           << geometry()
           << screen()->serialNumber();
    return array;
}

//------------------------------------------------------------------------------

bool CWorkPanelDialogWidget::restoreGeometry(const QByteArray &geometry)
{
    if (geometry.size() < 4) return false;
    QDataStream stream(geometry);
    stream.setVersion(QDataStream::Qt_4_0);

    const quint32 magicNumber = 0x1D9D1CA;
    quint32 storedMagicNumber;
    stream >> storedMagicNumber;
    if (storedMagicNumber != magicNumber)
        return false;

    const quint16 currentMajorVersion = 1;
    quint16 majorVersion = 0;
    quint16 minorVersion = 0;

    stream >> majorVersion >> minorVersion;

    if (majorVersion != currentMajorVersion)
        return false;
    // (Allow all minor versions.)

    QRect   restoredGeometry;
    QString restoredScreenNumber;

    stream >> restoredGeometry
           >> restoredScreenNumber;

   QRect availableGeometry = QGuiApplication::primaryScreen()->availableGeometry();

    foreach(QScreen* p_scr, QGuiApplication::screens()){
        if( p_scr->serialNumber() == restoredScreenNumber ){
            availableGeometry = p_scr->availableGeometry();
        }
    }

    // Modify the restored geometry if we are about to restore to coordinates
    // that would make the window "lost".

    if( ! restoredGeometry.intersects(availableGeometry) ) {
        restoredGeometry.moveBottom(qMin(restoredGeometry.bottom(), availableGeometry.bottom()));
        restoredGeometry.moveLeft(qMax(restoredGeometry.left(), availableGeometry.left()));
        restoredGeometry.moveRight(qMin(restoredGeometry.right(), availableGeometry.right()));
    }
    restoredGeometry.moveTop(qMax(restoredGeometry.top(), availableGeometry.top()));

    setGeometry(restoredGeometry);
    return true;
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

