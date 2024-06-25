#ifndef MoleculeH
#define MoleculeH
// =============================================================================
// NEMESIS - Molecular Modelling Package
// -----------------------------------------------------------------------------
//    Copyright (C) 2024 Petr Kulhanek, kulhanek@chemi.muni.cz
//    Copyright (C) 2010 Petr Kulhanek, kulhanek@chemi.muni.cz
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

#include <NemesisCoreMainHeader.hpp>
#include <Manipulator.hpp>
#include <ProObject.hpp>
#include <PBCInfo.hpp>
#include <QMap>

//------------------------------------------------------------------------------

class CStructureList;
class CProject;
class CXMLElement;
class CAtomList;
class CBondList;
class CResidueList;
class CRestraintList;
class CTrajectory;
class CAtom;

/// \defgroup structure Describe the structure

//------------------------------------------------------------------------------

extern CExtUUID NEMESIS_CORE_PACKAGE StructureID;

//------------------------------------------------------------------------------

/// structure
/*! \ingroup structure
*/

class NEMESIS_CORE_PACKAGE CStructure : public CProObject {
Q_OBJECT
public:
// constructors and destructors -----------------------------------------------
    CStructure(CStructureList* p_list);
    CStructure(CProject* p_project);
    ~CStructure(void);

    //! remove structure from base list
    void RemoveFromBaseList(CHistoryNode* p_history=NULL);

// methods with changes registered into history list ---------------------------
    /// delete structure from the structure list
    bool DeleteWH(void);

    /// insert structure from SMILES
    bool InsertSMILESWH(const QString& smiles);

    /// insert structure from InChI
    bool InsertInChIWH(const QString& inchi);

    /// delete entire molecule contents
    bool DeleteAllContentsWH(void);

    /// delete selected molecule contents
    bool DeleteSelectedContentsWH(void); 

    /// paste molecule part from XML stream
    bool PasteWH(CXMLElement* p_ele,bool avoid_overlap);

    /// set box parameters
    bool SetBoxWH(bool pa,bool pb,bool pc,const CPoint& sizes,const CPoint& angles);

    /// build super cell - ka,kb,kc - dimmensions of supercell
    bool BuildSuperCellWH(int ka,int kb,int kc);

    /// build super cell - va,vb,vc - transformation vector, n - number of repeats
    bool BuildSuperCellWH(int va,int vb,int vc,int n);

    /// set sequence index
    bool SetSeqIndexWH(int seqidx);

// informational methods -------------------------------------------------------
    /// get base list
    CStructureList* GetStructures(void) const;

    /// get atom list
    CAtomList*      GetAtoms(void);

    /// get bond list
    CBondList*      GetBonds(void);

    /// get residue list
    CResidueList*   GetResidues(void);

    /// get restraint list
    CRestraintList* GetRestraints(void);

    /// get molecule metrics, e.g. min and max geometrical dimensions
    void GetObjectMetrics(CObjMetrics& metrics);

    /// is molecule empty?
    bool IsEmpty(void) const;

    /// get structure SMILES
    const QString GetSMILES(void);

    /// get sequence index
    int GetSeqIndex(void) const;

// executive methods  ----------------------------------------------------------
    /// delete entire molecule contents
    void DeleteAllContents(CHistoryNode* p_history=NULL);

    /// delete selected atoms and bonds
    void DeleteSelectedContents(CHistoryNode* p_history=NULL);

    /// copy molecule to XML stream
    void Copy(CXMLElement* p_ele,bool selected_only=false);

    /// paste molecule part from XML stream
    /**
        donotoverride - control if the p_ele stream stays intact or it is updated
        avoid_overlap - put new structure out of existing one
    */
    void Paste(CXMLElement* p_ele,bool donotoverride,bool avoid_overlap,CHistoryNode* p_history=NULL);

    /// paste molecule part from XML stream
    /**
        donotoverride - control if the p_ele stream stays intact or it is updated
        avoid_overlap - put new structure out of existing one
    */
    void Paste(CXMLElement* p_ele,bool donotoverride,const CPoint& offset,CHistoryNode* p_history=NULL);

    /// insert structure from SMILES
    void InsertSMILES(const QString& smiles);

    /// insert structure from InChI
    void InsertInChI(const QString& inchi);

    /// set name
    virtual void SetName(const QString& name,CHistoryNode* p_history=NULL);

    /// set description
    virtual void SetDescription(const QString& descrip,CHistoryNode* p_history=NULL);

    /// set sequential index
    void SetSeqIndex(int seqidx,CHistoryNode* p_history=NULL);

//------------------------------------------------------------------------------
    /// set box parameters
    void SetBox(bool pa,bool pb,bool pc,const CPoint& sizes,const CPoint& angles,
                CHistoryNode* p_history=NULL);

// input/output methods --------------------------------------------------------
    /// load structure
    virtual void LoadData(CXMLElement* p_ele);

    /// save structure
    virtual void SaveData(CXMLElement* p_ele);

    /// load structural data (only Residues, Atoms, Bonds)
    void LoadStructureData(CXMLElement* p_ele,CHistoryNode* p_history=NULL);

    /// save structural data (only Residues, Atoms, Bonds)
    void SaveStructureData(CXMLElement* p_ele,bool selected=false);

    /// save selected residues, for details see CStructureList::SaveSelectedResidues
    void SaveSelectedResidues(CXMLElement* p_ele);

// external notification -------------------------------------------------------
    /// block signals for massive update
    void BeginUpdate(CHistoryNode* p_history=NULL);

    /// end of massive update
    void EndUpdate(bool do_not_sort = false,CHistoryNode* p_history=NULL);

    /// begin masive change in geometry
    void BeginGeometryUpdate(void);

    /// end geometry update
    void EndGeometryUpdate(void);

    /// emit OnGeometryChangeTick in StructureList
    void NotifyGeometryChangeTick(void);

// trajectory support ----------------------------------------------------------
    /// get associated trajectory
    CTrajectory*    GetTrajectory(void);

    /// set trajectory
    bool SetTrajectoryWH(CTrajectory* p_trajectory);

    /// set trajectory
    void SetTrajectory(CTrajectory* p_trajectory,CHistoryNode* p_history=NULL);

    /// set trajectory indexes
    void SetTrajIndexes(void);

    /// get atom by traj index
    CAtom* GetAtomByTrajIndex(int idx);

// section of public data ------------------------------------------------------
public:
    CPBCInfo        PBCInfo;

// section of private data -----------------------------------------------------
private:
    CAtomList*          Atoms;
    CBondList*          Bonds;
    CResidueList*       Residues;
    CRestraintList*     Restraints;
    CTrajectory*        Trajectory;
    int                 SeqIndex;
    int                 GeometryUpdateLevel;
    QMap<int,CAtom*>    TrajIndexMap;

    friend class CAtom;
};

//------------------------------------------------------------------------------

#endif
