#ifndef NormalVibModeH
#define NormalVibModeH
// =============================================================================
// NEMESIS - Molecular Modelling Package
// -----------------------------------------------------------------------------
//    Copyright (C) 2013 Petr Kulhanek, kulhanek@chemi.muni.cz
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
#include <TrajectorySegment.hpp>
#include <SimpleVector.hpp>
#include <Point.hpp>
#include <string>

// -----------------------------------------------------------------------------

///  normal vibration mode

class NEMESIS_CORE_PACKAGE CNormalVibMode {
public:
// constructor -----------------------------------------------------------------
    CNormalVibMode(int numofatoms);
    ~CNormalVibMode(void);

// section of private data -----------------------------------------------------
public:
    bool                    IsActive;
    double                  Frequency;
    double                  Intensity;
    bool                    IsRaman;
    bool                    IsIR;
    std::string             Symmetry;
    CSimpleVector<CPoint>   Mode;
};

// -----------------------------------------------------------------------------

#endif

