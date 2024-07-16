#ifndef TotalChargeWorkPanelH
#define TotalChargeWorkPanelH
// =============================================================================
// NEMESIS - Molecular Modelling Package
// -----------------------------------------------------------------------------
//    Copyright (C) 2024 Petr Kulhanek, kulhanek@chemi.muni.cz
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
#include "ui_TotalChargeWorkPanel.h"
#include <QFileDialog>
#include <QStandardItemModel>

//------------------------------------------------------------------------------

/// print internal geometry of selected structure

class CTotalChargeWorkPanel : public CWorkPanel {
Q_OBJECT
public:
// constructor and destructor -------------------------------------------------
    CTotalChargeWorkPanel(CProject* p_project);
    ~CTotalChargeWorkPanel(void);

// section of private data ----------------------------------------------------
private:
    Ui::TotalChargeWorkPanel   WidgetUI;

private slots:
    void CalculateTotalCharge(void);
    void ClearTotalCharge(void);
    void RemoveResidules(void);
};

//------------------------------------------------------------------------------

#endif
