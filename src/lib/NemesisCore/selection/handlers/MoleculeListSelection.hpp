#ifndef MoleculeListSelectionH
#define MoleculeListSelectionH
// =============================================================================
// NEMESIS - Molecular Modelling Package
// -----------------------------------------------------------------------------
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

#include <SelectionHandler.hpp>

// -----------------------------------------------------------------------------

class CAtom;

// -----------------------------------------------------------------------------

// Object ID
// NULL   SEL_ESC_KEY     - reset selection

// never throw ESR_SELECTED_OBJECTS_END

// -----------------------------------------------------------------------------

class NEMESIS_CORE_PACKAGE CMoleculeListSelection : public CSelectionHandler {
public:
// handler main method -----------------------------------------------------
    virtual ESelResult RegisterObject(CSelectionList* p_sel,const CSelObject& obj);

// handler description
    virtual const QString GetHandlerDescription(void) const;

private:
    void SelectMolecule(CSelectionList* p_sel,CAtom* p_at1);
};

// -----------------------------------------------------------------------------

extern CMoleculeListSelection NEMESIS_CORE_PACKAGE SH_MoleculeList;

// -----------------------------------------------------------------------------

#endif

