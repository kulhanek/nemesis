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

#include <ColorPushButton.hpp>
#include <ErrorSystem.hpp>
#include <QColorDialog>

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

QColorPushButton::QColorPushButton(QWidget* p_parent)
    : QPushButton(p_parent)
{
    connect(this,SIGNAL(clicked(bool)),
            this,SLOT(editColor(void)));
}

//------------------------------------------------------------------------------

const QColor& QColorPushButton::getQColor(void) const
{
    return(Color);
}

//------------------------------------------------------------------------------

void QColorPushButton::setColor(const QColor& value)
{
    Color = value;

    QString         sht;
    QString         sh;
    sht = "QPushButton { background-color: #%1;}";

    CColor ncolor  = CColor(Color.redF(),Color.greenF(),Color.blueF(),Color.alphaF());
    sh = sht.arg(QString(ncolor.GetRGBHex()));
    setStyleSheet(sh);
}

//------------------------------------------------------------------------------

const CColor  QColorPushButton::getCColor(void) const
{
    CColor color(Color.redF(), Color.greenF(),
                 Color.blueF(), Color.alphaF());
    return(color);
}

//------------------------------------------------------------------------------

void QColorPushButton::setColor(const CColor& value)
{
    QColor color(value.R()*255,value.G()*255,value.B()*255,value.Alpha()*255);
    setColor(color);
}

//------------------------------------------------------------------------------

void QColorPushButton::editColor(void)
{
    QColor color = Color;
    color = QColorDialog::getColor(color.rgb());
    if( color.isValid() == false ) return;
    setColor(color);
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================
