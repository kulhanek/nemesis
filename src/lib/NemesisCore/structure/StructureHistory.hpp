#ifndef MoleculeHistoryH
#define MoleculeHistoryH
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

#include <NemesisCoreMainHeader.hpp>
#include <HistoryItem.hpp>
#include <Point.hpp>
#include <XMLDocument.hpp>

//------------------------------------------------------------------------------

class CStructure;

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

class CStructureHI : public CHistoryItem {
public:
// constructors and destructors ------------------------------------------------
    CStructureHI(CProject* p_object);
    CStructureHI(CStructure* p_atom,EHistoryItemDirection change);

// section of private data -----------------------------------------------------
private:
    int             MoleculeIndex;
    CXMLDocument    StructureData;

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

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

class CStructureBoxHI : public CHistoryItem {
public:
// constructors and destructors ------------------------------------------------
    CStructureBoxHI(CProject* p_object);
    CStructureBoxHI(CStructure* p_mol,bool newpa,bool newpb,bool newpc,
                            const CPoint& newsizes,const CPoint& newangles);

// section of private data -----------------------------------------------------
private:
    int             MoleculeIndex;
    CPoint          OldSizes;
    CPoint          OldAngles;
    bool            OldPA,OldPB,OldPC;

    CPoint          NewSizes;
    CPoint          NewAngles;
    bool            NewPA,NewPB,NewPC;

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

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

class CStructureUpdateHI : public CHistoryItem {
public:
// constructors and destructors ------------------------------------------------
    CStructureUpdateHI(CProject* p_object);
    CStructureUpdateHI(CStructure* p_mol,EHistoryItemDirection change);

// section of private data -----------------------------------------------------
private:
    int             MoleculeIndex;

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

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

class CStructureSeqIndexHI : public CHistoryItem {
public:
// constructors and destructors ------------------------------------------------
    CStructureSeqIndexHI(CProject* p_object);
    CStructureSeqIndexHI(CStructure* p_res,int newseqidx);

// section of private data -----------------------------------------------------
private:
    int StructureIndex;
    int NewSeqIndex;
    int OldSeqIndex;

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

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

class CStructureTotalChargeHI : public CHistoryItem {
public:
// constructors and destructors ------------------------------------------------
    CStructureTotalChargeHI(CProject* p_object);
    CStructureTotalChargeHI(CStructure* p_res,int newTotalCharge);

// section of private data -----------------------------------------------------
private:
    int StructureIndex;
    int NewTotalCharge;
    int OldTotalCharge;

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

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

#endif
