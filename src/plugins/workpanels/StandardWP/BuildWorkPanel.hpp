#ifndef BuildWorkPanelH
#define BuildWorkPanelH
// =============================================================================
// NEMESIS - Molecular Modelling Package
// -----------------------------------------------------------------------------
//    Copyright (C) 2024 Petr Kulhanek, kulhanek@chemi.muni.cz
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
#include "ui_BuildWorkPanel.h"
#include <FragmentPalette.hpp>
#include <QDialog>
#include <HistoryList.hpp>
#include <SelectionList.hpp>
#include <Job.hpp>
#include <Optimizer.hpp>

// -----------------------------------------------------------------------------

enum EBuildAction { // supported actions
    EBA_NONE,
    EBA_ADD_STRUCTURE,
    EBA_CHANGE_ORDER,
    EBA_CHANGE_Z,
    EBA_DELETE_ATOM,
    EBA_DELETE_BOND,
    EBA_MAKE_BOND,
    EBA_BREAK_BOND,
    EBA_ADD_VALENCE,
};

// -----------------------------------------------------------------------------

class CPeriodicTableWidget;

// -----------------------------------------------------------------------------

class CBuildWorkPanel : public CWorkPanel {
    Q_OBJECT
public:
// constructor and destructors ------------------------------------------------
    CBuildWorkPanel(CProject* p_project);
    ~CBuildWorkPanel();

// section of private data ----------------------------------------------------
private:
    Ui::BuildWorkPanel      WidgetUI;
    QDialog*                PeriodicTableDialog;
    QButtonGroup*           ActionGroup;
    QAbstractButton*        ActiveButton;
    CSelectionRequest*      SelRequest;
    EBuildAction            Action;
    int                     Z;
    EBondOrder              Order;
    CFragmentPalette        BasicFragments;
    CFragmentPalette        GeneralFragments;
    COptimizer*             Optimizer;

    // set action
    void SetAction(EBuildAction new_action);

    // load fragment palette
    bool LoadFragments(void);

    // return active fragment
    CFragment* GetActiveFragment(void);

    virtual void  LoadWorkPanelSpecificData(CXMLElement* p_ele);
    virtual void  SaveWorkPanelSpecificData(CXMLElement* p_ele);

private slots:
    void ActionChange(QAbstractButton* p_button);
    void SelectionCompleted(void);
    void SelectionDetached(void);
    void Optimize(bool checked);
    void ShowPeriodicTable(void);
    void ElementChanged(int z);
    void ProjectLockChanged(EHistoryChangeMessage message);
    void UpdateInserterSetup(void);
    void JobStatusChanged(CJob* p_job);
};

// -----------------------------------------------------------------------------

#endif
