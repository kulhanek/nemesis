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

#include <QtGui>

#include <PluginObject.hpp>
#include <ExtUUID.hpp>
#include <CategoryUUID.hpp>

#include <ErrorSystem.hpp>
#include <GlobalSetup.hpp>
#include <ProjectList.hpp>

#include <Project.hpp>
#include <Structure.hpp>
#include <AtomList.hpp>
#include <BondList.hpp>
#include <ResidueList.hpp>
#include <Atom.hpp>
#include <Bond.hpp>
#include <Residue.hpp>
#include <HistoryList.hpp>
#include <JobList.hpp>

#include <Graphics.hpp>
#include <GraphicsProfileList.hpp>
#include <GraphicsObjectList.hpp>
#include <GraphicsObject.hpp>
#include <GraphicsView.hpp>
#include <StructureList.hpp>

#include <AmberTopology.hpp>
#include <AmberRestart.hpp>

#include "AmberModule.hpp"
#include "AmberImportJob.hpp"

//------------------------------------------------------------------------------

#if defined _WIN32 || defined __CYGWIN__
#undef AddAtom
#endif

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

QObject* AmberImportJobCB(void* p_data);

CExtUUID        AmberImportJobID(
                    "{AMBER_IMPORT_JOB:453ff333-6ce0-4da0-aa9a-5a9a67b199be}",
                    "Amber Import");

CPluginObject   AmberImportJobObject(&AmberPlugin,
                    AmberImportJobID,JOB_CAT,
                    NULL);

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

CAmberImportJob::CAmberImportJob(CProject* p_project)
    : CImportJob(&AmberImportJobObject,p_project)
{
    History = NULL;
    MaxTicks = 0;
    p_project->GetJobs()->RegisterJob(this);
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

bool CAmberImportJob::JobAboutToBeSubmitted(void)
{
    // following change is composite
    // it is composed from EHCL_TOPOLOGY and EHCL_GRAPHICS
    CHistoryNode* p_history = Structure->BeginChangeWH(EHCL_COMPOSITE,"import AMBER structure");
    if( p_history == NULL ) return(false);

    // initialize topology change
    History = Structure->BeginChangeWH(EHCL_TOPOLOGY,"import");
    if( History == NULL ){
        // end composite change
        Structure->EndChangeWH();
        return(false); // is change permitted?
    }

    // lock history
    CProject* p_project = Structure->GetProject();
    BackupLockLevels = p_project->GetHistory()->GetLockModeLevels();
    CLockLevels super_lock = ~CLockLevels();
    p_project->GetHistory()->SetLockModeLevels(super_lock);

    return(true);
}

//------------------------------------------------------------------------------

bool CAmberImportJob::InitializeJob(void)
{
    CProject* p_project = Structure->GetProject();

    MaxTicks = Topology.ResidueList.GetNumberOfResidues()
            +Topology.AtomList.GetNumberOfAtoms()
            +Topology.BondList.GetNumberOfBonds();

    p_project->StartProgressNotification(MaxTicks);

    connect(this,SIGNAL(OnProgressNotification(int,const QString&)),
            p_project,SLOT(ProgressNotification(int,const QString&)));

    // THREAD SAFETY !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
    // do some magic with parent and thread ownership
    OldMoleculeParent = Structure->parent();
    OldHistoryParent = History->parent();

    // temporarily remove parent
    Structure->setParent(NULL);
    History->setParent(NULL);

    // move objects to processing thread
    Structure->moveToThread(GetJobThread());
    History->moveToThread(GetJobThread());
    // END OF THREAD SAFETY !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

    // disable scene repainting
    CGraphics* p_grp = p_project->GetGraphics();
    p_grp->GetProfiles()->SetDataManipulationMode(true);

    return(true);
}

//------------------------------------------------------------------------------

bool CAmberImportJob::ExecuteJob(void)
{
    Structure->BeginUpdate(History);

    int top_seridx = Structure->GetAtoms()->GetTopSerIndex();
    int top_seqidx = Structure->GetResidues()->GetTopSeqIndex();

    QHash<int,CResidue*>    res_map;
    QHash<int,CAtom*>       atm_map;

    int tick = 1;
    int quantum = MaxTicks / 100;
    if( quantum == 0 ){
        quantum = 1;
    }

    // residues
    for (int i = 0; i < Topology.ResidueList.GetNumberOfResidues(); i++) {
        // is terminated?
        if( Terminated ) break;

        CAmberResidue* amberRes = Topology.ResidueList.GetResidue(i);

        CSmallString name = amberRes->GetName();
        // trim spaces
        int end = name.Verify(" ",-1,-1,true);
        name = name.GetSubStringFromTo(0,end);

        CResidue* p_res = Structure->GetResidues()->CreateResidue(QString(name),"",i+1+top_seqidx,History);
        res_map[amberRes->GetIndex()] = p_res;

        if( tick % quantum == 0 ){
            emit OnProgressNotification(tick,"Total progress %p% - Loading residues (1/3) ...");
        }
        tick++;
    }

    // go throught all atoms in Topology.AtomList
    for (int i = 0; i < Topology.AtomList.GetNumberOfAtoms(); i++) {
        // is terminated?
        if( Terminated ) break;

        CAmberAtom* amberAtom = Topology.AtomList.GetAtom(i);
        CAmberResidue* amberRes = amberAtom->GetResidue();

        CAtomData atomdata;
        atomdata.Index = -1;
        CSmallString name = amberAtom->GetName();
        // trim spaces
        int end = name.Verify(" ",-1,-1,true);
        name = name.GetSubStringFromTo(0,end);
        atomdata.Name = name;
        atomdata.SerIndex = i+1+top_seridx;
        atomdata.Z = amberAtom->GuessZ();
        atomdata.Type = amberAtom->GetType();
        atomdata.Pos = Restart.GetPosition(i);
        atomdata.Vel = Restart.GetVelocity(i);
        atomdata.Charge = amberAtom->GetStandardCharge();

        CAtom* p_atom = Structure->GetAtoms()->CreateAtom(atomdata,History);
        atm_map[i] = p_atom;

        CResidue* p_res = res_map[amberRes->GetIndex()];
        if( p_res != NULL ){
            p_res->AddAtom(p_atom,History);
        }

        if( tick % quantum == 0 ){
            emit OnProgressNotification(tick,"Total progress %p% - Loading atoms (2/3) ...");
        }
        tick++;
    }

    // bonds ...
    for (int i = 0; i < Topology.BondList.GetNumberOfBonds(); i++) {
        // is terminated?
        if( Terminated ) break;

        CAmberBond* amberBond = Topology.BondList.GetBond(i);
        EBondOrder order = BO_SINGLE;
        CAtom* p_at1 = atm_map[amberBond->GetIB()];
        CAtom* p_at2 = atm_map[amberBond->GetJB()];
        Structure->GetBonds()->CreateBond(p_at1,p_at2,order,History);

        if( tick % quantum == 0 ){
            emit OnProgressNotification(tick,"Total progress %p% - Loading bonds (3/3) ...");
        }
        tick++;
    }

    // box ...
    if( (Topology.BoxInfo.GetType() != AMBER_BOX_NONE) && (! Terminated) ){
        CPoint sizes = Restart.GetBox();
        CPoint angles = Restart.GetAngles();
        angles /= 180.0/M_PI;
        Structure->SetBox(true,true,true,sizes,angles,History);
    }

    // we do not need to sort the lists
    Structure->EndUpdate(true,History);

    // THREAD SAFETY !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
    // do some magic with parent and thread ownership

    // return objects back to main thread
    Structure->moveToThread(QCoreApplication::instance()->thread());
    History->moveToThread(QCoreApplication::instance()->thread());
    // END OF THREAD SAFETY !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

    return(true);
}

//------------------------------------------------------------------------------

bool CAmberImportJob::FinalizeJob(void)
{
    emit OnProgressNotification(MaxTicks,"Total progress %p% - Finalization ...");

    // THREAD SAFETY !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
    // do some magic with parent and thread ownership

    // restore parents
    Structure->setParent(OldMoleculeParent);
    History->setParent(OldHistoryParent);

    // notify master list - that the structure is back in the list
    if( Structure->GetStructures() ){
        Structure->GetStructures()->EmitOnStructureListChanged();
    }

    // END OF THREAD SAFETY !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

    // unlock history list
    CProject* p_project = Structure->GetProject();
    p_project->EndProgressNotification();
    p_project->GetHistory()->SetLockModeLevels(BackupLockLevels);

    CGraphics* p_grp = p_project->GetGraphics();
    p_grp->GetProfiles()->SetDataManipulationMode(false);

    // end topology change
    Structure->EndChangeWH();

    // adjust graphics
    AdjustGraphics();

    // end composite change
    Structure->EndChangeWH();
    return(true);
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================


