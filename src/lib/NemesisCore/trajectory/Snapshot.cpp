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

#include <Snapshot.hpp>
#include <TrajectorySegment.hpp>
#include <Trajectory.hpp>
#include <Structure.hpp>
#include <AtomList.hpp>

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

CSnapshot::CSnapshot(CTrajectorySegment* p_seg)
{
    Trajectory = p_seg->GetTrajectory();
}

//------------------------------------------------------------------------------

void CSnapshot::InitSnapshot(bool include_vel)
{
    if( (Trajectory == NULL) || (Trajectory->GetStructure() == NULL ) ){
        Coordinates.FreeVector();
        Velocities.FreeVector();
        ScalarProperties.clear();
        return;
    }
    CStructure* p_str = Trajectory->GetStructure();
    int num_of_atoms = p_str->GetAtoms()->GetNumberOfAtoms();
    Coordinates.CreateVector(num_of_atoms);
    if( include_vel )  Velocities.CreateVector(num_of_atoms);
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

const CPoint& CSnapshot::GetPos(int seqindex)
{
    static CPoint zero;
    if( Coordinates.GetLength() == 0 ){
        return(zero);
    }
    if( (seqindex < 0) || (seqindex >= (int)Coordinates.GetLength()) ){
        return(zero);
    }
    return(Coordinates[seqindex]);
}

//------------------------------------------------------------------------------

const CPoint& CSnapshot::GetVel(int seqindex)
{
    static CPoint zero;
    if( Velocities.GetLength() == 0 ){
        return(zero);
    }
    if( (seqindex < 0) || (seqindex >= (int)Velocities.GetLength()) ){
        return(zero);
    }
    return(Velocities[seqindex]);
}

//------------------------------------------------------------------------------

double CSnapshot::GetProperty(int key)
{
    return(ScalarProperties[key]);
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

void CSnapshot::SetPos(int seqindex,const CPoint& pos)
{
    if( Coordinates.GetLength() == 0 ){
        return;
    }
    if( (seqindex < 0) || (seqindex >= (int)Coordinates.GetLength()) ){
        return;
    }
    Coordinates[seqindex] = pos;
}

//------------------------------------------------------------------------------

void CSnapshot::SetVel(int seqindex,const CPoint& vel)
{
    if( Velocities.GetLength() == 0 ){
        return;
    }
    if( (seqindex < 0) || (seqindex >= (int)Velocities.GetLength()) ){
        return;
    }
    Velocities[seqindex] = vel;
}

//------------------------------------------------------------------------------

void CSnapshot::SetProperty(int key,double value)
{
    ScalarProperties[key] = value;
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

