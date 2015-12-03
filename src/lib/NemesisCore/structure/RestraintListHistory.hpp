#ifndef RestraintListHistoryH
#define RestraintListHistoryH
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

#include <HistoryNode.hpp>
#include <SimpleVector.hpp>

//------------------------------------------------------------------------------

class CRestraintList;
class CStructure;

//------------------------------------------------------------------------------

class CRestraintListEnabledHI : public CHistoryItem {
public:
// constructors and destructors ------------------------------------------------
    CRestraintListEnabledHI(CProject *p_object);
    CRestraintListEnabledHI(CRestraintList* p_rl,bool set);

// section of private data -----------------------------------------------------
private:
    int     MoleculeIndex;
    bool    OldEnabled;
    bool    NewEnabled;

// executive methods -----------------------------------------------------------
    /// perform the change in the forward direction
    virtual void Forward(void);

    /// perform the change in the backward direction
    virtual void Backward(void);

// input/output methods --------------------------------------------------------
    /// load data
    virtual void LoadData(CXMLElement* p_ele);

    /// save data
    virtual void SaveData(CXMLElement* p_ele);
};

//------------------------------------------------------------------------------

class CRestraintListChangeParentHI : public CHistoryItem {
public:
// constructors and destructors ------------------------------------------------
    CRestraintListChangeParentHI(CProject* p_object);
    CRestraintListChangeParentHI(CStructure* p_master,CStructure* p_source,
                            const QVector<int>& indexes);

// section of private data -----------------------------------------------------
private:
    int                 MasterIndex;
    int                 SourceIndex;
    CSimpleVector<int>  Restraints;

// executive methods -----------------------------------------------------------
    /// perform the change in the forward direction
    virtual void Forward(void);

    /// perform the change in the backward direction
    virtual void Backward(void);

// input/output methods --------------------------------------------------------
    /// load data
    virtual void LoadData(CXMLElement* p_ele);

    /// save data
    virtual void SaveData(CXMLElement* p_ele);
};

//------------------------------------------------------------------------------

#endif
