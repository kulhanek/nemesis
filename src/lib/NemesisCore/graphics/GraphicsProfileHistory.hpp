#ifndef GraphicsProfileHistoryH
#define GraphicsProfileHistoryH
// =============================================================================
// NEMESIS - Molecular Modelling Package
// -----------------------------------------------------------------------------
//    Copyright (C) 2010 Petr Kulhanek, kulhanek@chemi.muni.cz
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
#include <HistoryNode.hpp>
#include <SmallString.hpp>
#include <XMLDocument.hpp>

//------------------------------------------------------------------------------

class CProject;
class CGraphicsProfile;
class CGraphicsObject;

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

class CGraphicsProfileHI: public CHistoryItem {
public:
// constructors and destructors ------------------------------------------------
    CGraphicsProfileHI(CProject *p_object);
    CGraphicsProfileHI(CGraphicsProfile* p_gp,EHistoryItemDirection change);

// section of private data ----------------------------------------------------
private:
    CXMLDocument    GraphicsProfileData;

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

class CGraphicsProfileAddObjectHI: public CHistoryItem {
public:
// constructors and destructors ------------------------------------------------
    CGraphicsProfileAddObjectHI(CProject *p_object);
    CGraphicsProfileAddObjectHI(CGraphicsProfile* p_gp,CGraphicsObject* p_obj,
                                int position, EHistoryItemDirection change);

// section of private data -----------------------------------------------------
private:
    int GraphicsProfileIndex;
    int GraphicsObjectIndex;
    int Position;

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

class CGraphicsProfileMoveObjectHI: public CHistoryItem {
public:
// constructors and destructors ------------------------------------------------
    CGraphicsProfileMoveObjectHI(CProject *p_object);
    CGraphicsProfileMoveObjectHI(CGraphicsProfile* p_gp,CGraphicsObject* p_obj,
                                          EHistoryItemDirection change);

// section of private data ----------------------------------------------------
private:
    int GraphicsProfileIndex;
    int GraphicsObjectIndex;

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
