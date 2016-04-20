#ifndef GESPAsRESPImportJobH
#define GESPAsRESPImportJobH
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

#include <ImportJob.hpp>
#include <HistoryList.hpp>
#include <fstream>
#include <Project.hpp>

//------------------------------------------------------------------------------

class CStructure;
class CAtom;
class CGESPAsRESPGridObject;

//------------------------------------------------------------------------------

/// import GESPAsRESP file job

class CGESPAsRESPImportJob : public CImportJob {
Q_OBJECT
public:
// constructor and destructor -------------------------------------------------
    CGESPAsRESPImportJob(CProject* p_project);

// section of private data ----------------------------------------------------
private:
    CHistoryNode*       History;
    std::ifstream       sin;
    CLockLevels         BackupLockLevels;
    QObject*            OldMoleculeParent;
    QObject*            OldHistoryParent;
    CGESPAsRESPGridObject*    GESPAsRESPGridObject;

    /// can we submit the job? - executed from main thread during JobSubmit
    virtual bool JobAboutToBeSubmitted(void);

    /// initialize job - executed from main thread
    virtual bool InitializeJob(void);

    /// job main execution point - executed from job thread
    virtual bool ExecuteJob(void);

    /// finalize job - executed from main thread
    virtual bool FinalizeJob(void);

    /// helper methods
    bool ImportStructure(void);

signals:
    void OnStartProgressNotification(int length);
    void OnProgressNotification(int progress,const QString& text);
    void OnTextNotification(ETextNotificationType type,const QString& text,int time);
};

//------------------------------------------------------------------------------

#endif