#ifndef StructureListH
#define StructureListH
// =============================================================================
// NEMESIS - Molecular Modelling Package
// -----------------------------------------------------------------------------
//    Copyright (C) 2024 Petr Kulhanek, kulhanek@chemi.muni.cz
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

#include <NemesisCoreMainHeader.hpp>
#include <Structure.hpp>

// -----------------------------------------------------------------------------

class CStructure;
class CProject;
class CXMLElement;

// -----------------------------------------------------------------------------

extern CExtUUID NEMESIS_CORE_PACKAGE StructureListID;

// -----------------------------------------------------------------------------

///  declare structure list
class NEMESIS_CORE_PACKAGE CStructureList: public CProObject {
Q_OBJECT
public:
// constructors and destructors ------------------------------------------------
    CStructureList(CProject* p_project);
    CStructureList(CProject* p_project,bool no_index);
    ~CStructureList(void);

//------------------------------------------------------------------------------
    /// create new structure
    CStructure* CreateStructureWH(bool activate,const QString& name = QString(),
                                  const QString& descr = QString());

    /// create structure from selected residues
    CStructure* CreateStructureFromSelectedResiduesWH(const QString& name = QString(),
                                  const QString& descr = QString());

    /// duplicate structure, new structure is activated
    CStructure* DuplicateStructureWH(CStructure* p_ostr,
                                     bool move_to_all=false,
                                     bool move_x=false,bool move_y=false,bool move_z=false,double offset=0.0);

    /// activate structure - structure must be owned by the list
    bool SetActiveStructureWH(CStructure* p_str);

    /// delete selected structures
    bool DeleteSelectedStructuresWH(void);

    /// delete empty structures (e.g. structures without any atoms)
    bool DeleteEmptyStructuresWH(void);

    /// delete all structures
    bool DeleteAllStructuresWH(void);

    /// update sequence index by its order (e.g. it will remove gaps)
    bool SequenceIndexByOrderWH(void);

    /// merge structures
    bool MergeStructuresWH(const QList<CStructure*>& structures);

    /// merge all structures
    bool MergeAllStructuresWH(void);

// executive methods -----------------------------------------------------------
    /// create empty structure
    CStructure* CreateStructure(const QString& name = QString(),const QString& descr = QString(),
                                CHistoryNode* p_history=NULL);

    /// create structure from XML stream
    CStructure* CreateStructure(CXMLElement* p_ele,CHistoryNode* p_history=NULL);

    /// duplicate structure
    CStructure* DuplicateStructure(CStructure* p_ostr,bool activate,
                                   bool move_to_all=false,
                                   bool move_x=false,bool move_y=false,bool move_z=false,double offset=0.0,
                                   CHistoryNode* p_history=NULL);

    /// activate structure - structure must be owned by the list
    void SetActiveStructure(CStructure* p_str,CHistoryNode* p_history=NULL);

    /// delete selected structures
    void DeleteSelectedStructures(CHistoryNode* p_history=NULL);

    /// delete empty structures, e.g. structures that does not contain any atom
    void DeleteEmptyStructures(CHistoryNode* p_history=NULL);

    /// delete all structures
    void DeleteAllStructures(CHistoryNode* p_history=NULL);

    /// merge structures
    void MergeStructures(const QList<CStructure*>& structures,CHistoryNode* p_history=NULL);

    /// clear structures
    void ClearStructures(void);

    /// sort structures by sequence index
    void SortStructures(void);

// informational methods -------------------------------------------------------
    /// get active structire
    CStructure*  GetActiveStructure(void) const;

    /// return number of structures
    int         GetNumberOfStructures(void) const;

    /// return number of selected structures
    int         GetNumberOfSelectedStructures(void) const;

    /// return number of empty structures
    int         GetNumberOfEmptyStructures(void) const;

    /// return largest used sequnce index
    int         GetLargestSequenceIndex(void) const;

    /// get all molecule metrics, e.g. min and max geometrical dimensions
    void GetObjectMetrics(CObjMetrics& metrics);

// input/output methods --------------------------------------------------------
    /// load all structures with history recording
    void LoadData(CXMLElement* p_ele,CHistoryNode* p_history);

    /// load all structures
    virtual void LoadData(CXMLElement* p_ele);

    /// save all structures
    virtual void SaveData(CXMLElement* p_ele);

    /// save selected residues
    void SaveSelectedResidues(CXMLElement* p_ele);

// external notification -------------------------------------------------------
    /// begin masive change in geometry
    void BeginGeometryUpdate(void);

    /// end geometry update
    void EndGeometryUpdate(bool no_event=false);

    /// emit OnGeometryChangeTick signal
    void NotifyGeometryChangeTick(void);

// external notification -------------------------------------------------------
    /// block signals for massive update, only structures
    void BeginUpdate(void);

    /// end of massive update
    void EndUpdate(bool do_not_sort=false);

    /// emit OnStructureListChanged signal
    void EmitOnStructureListChanged(void);

    /// list size changed
    void ListSizeChanged(bool do_not_sort=false);

// -----------------------------------------------------------------------------
signals:
    /// emmited when structure list is changed
    void OnStructureListChanged(void);

    /// extra signal when geometry was changed
    void OnGeometryChangeTick(void);

// section of private data -----------------------------------------------------
private:
    CStructure*     ActiveStructure;
    int             GeometryUpdateLevel;
    bool            Changed;
    int             UpdateLevel;
    bool            ForceSorting;

    static bool LessThanBySeqIndex(CStructure* p_left,CStructure* p_right);

    friend class CStructure;
};

// -----------------------------------------------------------------------------

#endif

