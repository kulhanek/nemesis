#ifndef PRDesignerGeoGraphicsH
#define PRDesignerGeoGraphicsH
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

#include <QWidget>
#include "ui_PRDesignerGeoGraphics.h"
#include <HistoryList.hpp>

//------------------------------------------------------------------------------

class CGeoProperty;
class CProObjectDesigner;
class CPRDesignerGraphics;

//------------------------------------------------------------------------------

class NEMESIS_CORE_PACKAGE CPRDesignerGeoGraphics : public QWidget {
Q_OBJECT
public:
// constructor and destructor --------------------------------------------------
    CPRDesignerGeoGraphics(QWidget* p_owner,CGeoProperty* p_prop,CProObjectDesigner* p_designer);

// executive methods -----------------------------------------------------------
public slots:
    /// initialize visualization of general properties
    void InitValues(void);

    /// update object properties according to visual setup
    void ApplyValues(void);

public:
    /// load designer setup
    void LoadDesignerSetup(CXMLElement* p_ele);

    /// save designer setup
    void SaveDesignerSetup(CXMLElement* p_ele);

// section of private data ----------------------------------------------------
private:
    Ui::PRDesignerGeoGraphics   WidgetUI;
    CGeoProperty*               Property;
    CProObjectDesigner*         Designer;
    CPRDesignerGraphics*        Graphics;

private slots:
    void ProjectLockChanged(EHistoryChangeMessage message);
};

//------------------------------------------------------------------------------

#endif
