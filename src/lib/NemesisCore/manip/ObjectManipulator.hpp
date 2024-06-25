#ifndef ObjectManipulatorH
#define ObjectManipulatorH
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

#include <NemesisCoreMainHeader.hpp>
#include <Manipulator.hpp>
#include <QElapsedTimer>

//------------------------------------------------------------------------------

// three updates per second
#define DEFAULT_MSTICK 333

//------------------------------------------------------------------------------

/// extension to object manipulation

class NEMESIS_CORE_PACKAGE CObjectManipulator : public CManipulator {
public:
// constructors and destructors ------------------------------------------------
    CObjectManipulator(void);

// executive methods -----------------------------------------------------------
    /// start event
    void StartEvent(void);

    /// end event
    void EndEvent(void);

    /// raise event - mstick in ms
    void RaiseEvent(int mstick=DEFAULT_MSTICK);

    /// respond to event
    virtual void RespondToEvent(void);

// section of private data -----------------------------------------------------
private:
    QElapsedTimer   Time;
    bool            EventRaised;
};

//------------------------------------------------------------------------------

#endif

