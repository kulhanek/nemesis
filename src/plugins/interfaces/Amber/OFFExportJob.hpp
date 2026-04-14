#ifndef OFFExportJobH
#define OFFExportJobH
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

#include <Job.hpp>
#include <HistoryList.hpp>
#include <fstream>

//------------------------------------------------------------------------------

class CStructure;
class CAtomList;
class CBondList;

//------------------------------------------------------------------------------

class COFFExportJob : public CJob {
    Q_OBJECT
public:
// constructor and destructor -------------------------------------------------
    COFFExportJob(CStructure* p_mol,const QString& name);

// section of private data ----------------------------------------------------
private:
    CStructure*          Structure;
    QString             FileName;
    std::ofstream       sout;
    CLockLevels         BackupLockLevels;
    std::map<int,int>   IndexMap;
    int                 MaxTicks;
    int                 Quantum;
    int                 Tick;

    /// set molecule
    void SetMolecule(CStructure* p_mol);

    /// can we submit the job? - executed from main thread during JobSubmit
    virtual bool JobAboutToBeSubmitted(void);

    /// initialize job - executed from main thread
    virtual bool InitializeJob(void);

    /// job main execution point - executed from job thread
    virtual bool ExecuteJob(void);

    /// finalize job - executed from main thread
    virtual bool FinalizeJob(void);

    /// helper methods
    void WriteOFF(void);

signals:
    void OnProgressNotification(int progress,const QString& text);
};

//------------------------------------------------------------------------------

#endif
