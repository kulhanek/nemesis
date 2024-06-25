#ifndef BondH
#define BondH
// =============================================================================
// NEMESIS - Molecular Modelling Package
// -----------------------------------------------------------------------------
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
#include <Point.hpp>
#include <ProObject.hpp>

// -----------------------------------------------------------------------------

class CProject;
class CStructure;
class CAtom;
class CBond;
class CBondList;
class CXMLElement;

// -----------------------------------------------------------------------------

///  EBondOrder - bond orders

enum EBondOrder {
    BO_NONE     =  0,
    BO_WEAK     =  1,
    BO_SINGLE   =  2,
    BO_SINGLE_H =  3,
    BO_DOUBLE   =  4,
    BO_DOUBLE_H =  5,
    BO_TRIPLE   =  6
};

// ----------------------------------------------------------------------------

extern CExtUUID NEMESIS_CORE_PACKAGE BondID;

// ----------------------------------------------------------------------------

///  bond implementation

class NEMESIS_CORE_PACKAGE CBond : public CProObject {
public:
// constructors and destructors -----------------------------------------------
    CBond(CBondList* p_bl);
    CBond(CBondList* p_bl,bool no_index);

    virtual ~CBond(void);
    void    RemoveFromBaseList(CHistoryNode* p_history=NULL);

// methods with changes registered into history list --------------------------
    /// delete bond with recorded histroy
    bool DeleteWH(void);

    /// break bond
    bool BreakWH(void);

    /// change order
    bool SetOrderWH(EBondOrder order);

    /// change type
    bool SetBondTypeWH(const QString& type);

    /// set first atom
    bool SetFirstAtomWH(CAtom* p_atom);

    /// set second atom
    bool SetSecondAtomWH(CAtom* p_atom);

// informational methods ------------------------------------------------------
    /// get structure owner
    CStructure*     GetStructure(void) const;

    /// get bond list owner
    CBondList*      GetBonds(void) const;

    /// get either real bond name or derived one
    virtual const QString GetName(void) const;

    /// get bond order
    EBondOrder      GetBondOrder(void) const;

    /// get bond order
    QString         GetBondOrderInStringForm(void) const;

    /// get bond order
    const QString&  GetBondType(void) const;

    /// get bond length
    double          GetLength(void) const;

    /// get first bond atom
    CAtom*          GetFirstAtom(void) const;

    /// get second bond atom
    CAtom*          GetSecondAtom(void) const;

    /// ge opposite atom
    CAtom*          GetOppositeAtom(CAtom* p_a) const;

    /// get connect atom
    CAtom*          GetConnectAtom(void) const;

    /// get main bond vector - ve
    const CPoint    GetMainVector(void) const;

    /// is bond selected by residues?
    bool            IsSelectedByResidues(void);

    /// get geometry descriptor
    virtual CGeoDescriptor  GetGeoDescriptor(void);

    /// get PBC index
    int GetPBCIndex(void);

    /// is invalid bond? e.g. at least one atom is undefined
    inline bool IsInvalidBond(void);

// executive methods without change registered to history list ----------------
    /// set name
    virtual void SetName(const QString& name,CHistoryNode* p_history=NULL);

    /// set description
    virtual void SetDescription(const QString& descrip,CHistoryNode* p_history=NULL);

    /// break bond
    void Break(CHistoryNode* p_history = NULL);

    /// set bond order
    void SetBondOrder(EBondOrder order,CHistoryNode* p_history = NULL);

    /// set bond order
    void SetBondType(const QString& type,CHistoryNode* p_history = NULL);

    /// set bond first atom
    void SetFirstAtom(CAtom* p_a,CHistoryNode* p_history = NULL);

    /// set bond second atom
    void SetSecondAtom(CAtom* p_a,CHistoryNode* p_history = NULL);

    /// set PBC bond index
    void SetPBCIndex(int index);

    /// remove object from registered objects - server side
    virtual void RemoveFromRegistered(CProObject* p_object,CHistoryNode* p_history = NULL);

    /// change parent, see CBondList::MoveAllBondsFrom
    void ChangeParent(CBondList* p_newparent);

// input/output methods -------------------------------------------------------
    /// load bond data
    virtual void LoadData(CXMLElement* p_ele);

    /// save bond data
    virtual void SaveData(CXMLElement* p_ele);

// section of private data ----------------------------------------------------
private:
    CAtom*          A1;
    CAtom*          A2;
    EBondOrder      Order;
    QString         Type;
    // PBC bond index, 0 - no PBC bond
    // it is used to connect PBC bonds among cells during supercell building
    int             PBCIndex;
};

// -----------------------------------------------------------------------------

inline bool CBond::IsInvalidBond(void)
{
    return( (A1 == NULL) || (A2 == NULL) || (A1 == A2) );
}

//------------------------------------------------------------------------------

#endif

