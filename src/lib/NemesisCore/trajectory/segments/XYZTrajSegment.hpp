#ifndef XYZTrajSegmentH
#define XYZTrajSegmentH
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
#include <TrajectorySegment.hpp>
#include <Snapshot.hpp>
#include <istream>

// -----------------------------------------------------------------------------

class CTrajectory;

// -----------------------------------------------------------------------------

///  XYZ cached trajectory segment

class NEMESIS_CORE_PACKAGE CXYZTrajSegment : public CTrajectorySegment {
Q_OBJECT
public:
// constructor -----------------------------------------------------------------
    CXYZTrajSegment(CTrajectory* p_traj);
    ~CXYZTrajSegment(void);

// information methods ---------------------------------------------------------
    /// get number of snaphots in the file
    virtual long int GetNumberOfSnapshots(void);

    /// get current segment
    virtual CSnapshot* GetCurrentSnapshot(void);

// input/output methods --------------------------------------------------------
    /// load trajectory data
    virtual void LoadTrajectoryData(void);

// section of private data -----------------------------------------------------
private:
    QList<CSnapshot*>   Snapshots;

    // read snapshot
    bool ReadSnapshot(std::istream& sin,CSnapshot* p_snap,int snap);
};

// -----------------------------------------------------------------------------

#endif

