#ifndef StructureListWorkPanelH
#define StructureListWorkPanelH
// =============================================================================
// NEMESIS - Molecular Modelling Package
// -----------------------------------------------------------------------------
//    Copyright (C) 2024 Petr Kulhanek, kulhanek@chemi.muni.cz
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

#include <WorkPanel.hpp>
#include <HistoryList.hpp>
#include "ui_StructureListWorkPanel.h"

//------------------------------------------------------------------------------

class CStructureList;

//------------------------------------------------------------------------------

class CStructureListWorkPanel : public CWorkPanel {
Q_OBJECT
public:
// constructor and destructor -------------------------------------------------
    CStructureListWorkPanel(CProject* p_owner);
    ~CStructureListWorkPanel(void);

// section of private data ----------------------------------------------------
private:
    Ui::StructureListWorkPanel  WidgetUI;
    CStructureList*             List;
    CContainerModel*            ObjectsModel;
    CContainerModel*            MergeObjectsModel;

    /// raised when window is docked/undocked from the main window
    virtual void WindowDockStatusChanged(bool docked);

    virtual void  LoadWorkPanelSpecificData(CXMLElement* p_ele);
    virtual void  SaveWorkPanelSpecificData(CXMLElement* p_ele);

private slots:
    void ProjectNameChanged(CProject* p_project);
    void OpenProjectDesigner(void);
    void ProjectLockChanged(EHistoryChangeMessage message);

    void ObjectsTVSelectionChanged(void);
    void ObjectsTVDblClicked(const QModelIndex& index);

    void NewStructure(void);
    void ActivateStructure(void);
    void RemoveStructure(void);
    void DuplicateStructure(void);
    void StructureInfo(void);

    void DeleteEmptyStructures(void);
    void DeleteSelectedStructures(void);
    void DeleteAllStructures(void);

    void NewFromSelectedResidues(void);
    void DeactivateActiveStructure(void);
    void SeqIndexByOrder(void);
    void SortStructures(void);

    void MergeTVSelectionChanged(void);
    void MergeTVDblClicked(const QModelIndex& index);
    void MergeAllStructures(void);
    void MergeSelectedStructures(void);
};

//------------------------------------------------------------------------------

#endif
