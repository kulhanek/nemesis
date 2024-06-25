// =============================================================================
// NEMESIS - Molecular Modelling Package
// -----------------------------------------------------------------------------
//    Copyright (C) 2009 Petr Kulhanek, kulhanek@chemi.muni.cz
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

#include <JobList.hpp>
#include <NemesisCoreModule.hpp>
#include <CategoryUUID.hpp>
#include <Project.hpp>
#include <ErrorSystem.hpp>

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

CExtUUID        JobListID(
                    "{JOB_LIST:dc2ff03c-6907-4faa-b21c-01e5c7ecfe19}",
                    "Jobs");

CPluginObject   JobListObject(&NemesisCorePlugin,
                    JobListID,JOB_CAT,
                    ":/images/NemesisCore/job/JobList.svg",
                    NULL);

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

CJobList::CJobList(CProject* p_owner,bool no_index)
    : CProObject(&JobListObject,p_owner,p_owner,no_index)
{
}

//------------------------------------------------------------------------------

CJobList::~CJobList(void)
{

}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

bool CJobList::RegisterJob(CJob* p_job)
{
    if( p_job == NULL ) {
        ES_ERROR("p_job is NULL");
        return(false);
    }

    // insert into the list and emit signal
    p_job->setParent(this);
    emit OnJobAdded(p_job);

    return(true);
}

//------------------------------------------------------------------------------

void CJobList::TerminateAllJobs(void)
{
    foreach(QObject* p_obj, children()){
        CJob* p_job = dynamic_cast<CJob*>(p_obj);
        if( p_job != NULL ){
            p_job->TerminateJob();
            p_job->WaitForEndWithEventLoop();
        }
    }
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

void CJobList::EmitJobStatusChanged(CJob* p_job)
{
    emit OnJobStatusChanged(p_job);
}

//------------------------------------------------------------------------------

void CJobList::EmitJobRemoved(CJob* p_job)
{
    emit OnJobRemoved(p_job);
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

