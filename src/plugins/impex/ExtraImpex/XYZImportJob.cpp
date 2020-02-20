// =============================================================================
// NEMESIS - Molecular Modelling Package
// -----------------------------------------------------------------------------
//    Copyright (C) 2013 Petr Kulhanek, kulhanek@chemi.muni.cz
//
//     This program sin free software; you can redistribute it and/or modify
//     it under the terms of the GNU General Public License as published by
//     the Free Software Foundation; either version 2 of the License, or
//     (at your option) any later version.
//
//     This program sin distributed in the hope that it will be useful,
//     but WITHOUT ANY WARRANTY; without even the implied warranty of
//     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//     GNU General Public License for more details.
//
//     You should have received a copy of the GNU General Public License along
//     with this program; if not, write to the Free Software Foundation, Inc.,
//     51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
// =============================================================================

#include <QMessageBox>
#include <QCoreApplication>

#include <PluginObject.hpp>
#include <ProjectList.hpp>
#include <ExtUUID.hpp>
#include <CategoryUUID.hpp>

#include <ErrorSystem.hpp>
#include <GlobalSetup.hpp>
#include <Project.hpp>
#include <Structure.hpp>
#include <JobList.hpp>
#include <OpenBabelUtils.hpp>
#include <MainWindow.hpp>
#include <StructureList.hpp>
#include <QMessageBox>
#include <AtomListHistory.hpp>
#include <Graphics.hpp>
#include <GraphicsProfileList.hpp>
#include <GraphicsObjectList.hpp>
#include <XYZStructure.hpp>
#include <Atom.hpp>
#include <PeriodicTable.hpp>

#include "ExtraImpexModule.hpp"

#include "XYZImportJob.hpp"

#include "openbabel/mol.h"
#include "openbabel/obconversion.h"

//------------------------------------------------------------------------------
using namespace std;
using namespace OpenBabel;
//------------------------------------------------------------------------------

#if defined _WIN32 || defined __CYGWIN__
#undef AddAtom
#endif

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

QObject* XYZImportJobCB(void* p_data);

CExtUUID        XYZImportJobID(
                    "{XYZ_IMPORT_JOB:7f8a1822-0d5c-4e51-8162-2a43e430254c}",
                    "XYZ File (*.xyz)",
                    "Import/inject XYZ file");

CPluginObject   XYZImportJobObject(&ExtraImpexPlugin,
                    XYZImportJobID,JOB_CAT,
                    QStringList() << "EPF_IMPORT_STRUCTURE" << "FORMAT=xyz",
                    XYZImportJobCB);

// -----------------------------------------------------------------------------

QObject* XYZImportJobCB(void* p_data)
{
    CProject* p_project = static_cast<CProject*>(p_data);
    if( p_project == NULL ){
        ES_ERROR("CXYZImportJob requires active project");
        return(NULL);
    }

    QObject* p_importjob = new CXYZImportJob(p_project);
    return(p_importjob);
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

CXYZImportJob::CXYZImportJob(CProject* p_project)
    : CImportJob(&XYZImportJobObject,p_project)
{
    Structure = NULL;
    FileName = "";

    p_project->GetJobs()->RegisterJob(this);
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

bool CXYZImportJob::JobAboutToBeSubmitted(void)
{
    sin.open(FileName.toLatin1());
    if( !sin ) {
        QMessageBox::critical(GetProject()->GetMainWindow(),tr("Error"),tr("Unable to open file for reading!"),QMessageBox::Ok,QMessageBox::Ok);
        ES_ERROR("Cannot open file to read");
        return(false);
    }

    // check if it is allowed to import to empty structure
    CProject* p_project = Structure->GetProject();
    if( p_project->IsFlagSet<EProjecFlag>(EPF_ALLOW_IMP_TO_EMPTY_STR) ){
        if( Structure->IsEmpty() ){
            History = NULL;
            BackupLockLevels = p_project->GetHistory()->GetLockModeLevels();
            CLockLevels super_lock = ~CLockLevels();
            p_project->GetHistory()->SetLockModeLevels(super_lock);
            return(true);
        }
    }

    // following change is composite
    // it is composed from EHCL_TOPOLOGY and EHCL_GRAPHICS
    CHistoryNode* p_history;

    if( GetProject()->property("impex.inject") == false ){
        p_history = Structure->BeginChangeWH(EHCL_COMPOSITE,"import XYZ structure");
    } else {
        p_history = Structure->BeginChangeWH(EHCL_COMPOSITE,"inject XYZ structure");
    }
    if( p_history == NULL ) return(false);

    // initialize topology change
    if( GetProject()->property("impex.inject") == false ){
        History = Structure->BeginChangeWH(EHCL_TOPOLOGY,"import");
    } else{
        History = Structure->BeginChangeWH(EHCL_GEOMETRY,"inject");
        CAtomListCoordinatesHI* p_history = new CAtomListCoordinatesHI(Structure);
        History->Register(p_history);
        Structure->BeginUpdate(History);
    }
    if( History == NULL ){
        // end composite change
        Structure->EndChangeWH();
        return(false); // is change permitted?
    }

    // lock history
    BackupLockLevels = p_project->GetHistory()->GetLockModeLevels();
    CLockLevels super_lock = ~CLockLevels();
    p_project->GetHistory()->SetLockModeLevels(super_lock);

    return(true);
}

//------------------------------------------------------------------------------

bool CXYZImportJob::InitializeJob(void)
{
    CProject* p_project = Structure->GetProject();

    connect(this,SIGNAL(OnStartProgressNotification(int)),
            p_project,SLOT(StartProgressNotification(int)));

    connect(this,SIGNAL(OnProgressNotification(int,const QString&)),
            p_project,SLOT(ProgressNotification(int,const QString&)));

    connect(this,SIGNAL(OnTextNotification(ETextNotificationType,const QString&,int)),
            p_project,SLOT(TextNotification(ETextNotificationType,const QString&,int)));

    // THREAD SAFETY !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
    // do some magic with parent and thread ownership
    OldMoleculeParent = Structure->parent();
    if( History ) OldHistoryParent = History->parent();

    // temporarily remove parent
    Structure->setParent(NULL);
    if( History ) History->setParent(NULL);

    // move objects to processing thread
    Structure->moveToThread(GetJobThread());
    if( History ) History->moveToThread(GetJobThread());
    // END OF THREAD SAFETY !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

    CGraphics* p_grp = p_project->GetGraphics();
    p_grp->GetProfiles()->SetDataManipulationMode(true);

    emit OnStartProgressNotification(4);

    return(true);
}

//------------------------------------------------------------------------------

bool CXYZImportJob::ExecuteJob(void)
{
    if( GetProject()->property("impex.inject") == false ){
        return(ImportStructure());
    } else {
        return(InjectCoordinates());
    }
}

//------------------------------------------------------------------------------

bool CXYZImportJob::ImportStructure(void)
{
    Structure->BeginUpdate(History);

    // read molecule from file to babel internal
    OBConversion    conv(&sin, &cout);
    OBFormat*       obFormat = conv.FormatFromExt(FileName.toStdString());
    OBMol           mol;

    emit OnProgressNotification(0,"Total progress %p% - Loading structure ...");

    bool result = true;

    if( obFormat == NULL ) {
        GetProject()->TextNotification(ETNT_ERROR,"unsupported format",ETNT_ERROR_DELAY);
        result = false;
    }

    if( result && (! conv.SetInFormat(obFormat)) ) {
        QString message(tr("Failed to set format for %1"));
        message = message.arg(QString(FileName));
        GetProject()->TextNotification(ETNT_ERROR,message,ETNT_ERROR_DELAY);
        result = false;
    }

    if( result && (! conv.Read(&mol)) ) {
        QString message(tr("Failed to read molecule from file %1"));
        message = message.arg(QString(FileName));
        GetProject()->TextNotification(ETNT_ERROR,message,ETNT_ERROR_DELAY);
        result = false;
    }

    if( result ){
        emit OnProgressNotification(1,"Total progress %p% - Generating bonds ...");
        mol.ConnectTheDots();

        emit OnProgressNotification(2,"Total progress %p% - Generating bond types ...");
        mol.PerceiveBondOrders();

        // convert data
        emit OnProgressNotification(3,"Total progress %p% - Converting to internal representation ...");
        COpenBabelUtils::OpenBabel2Nemesis(mol,Structure,History);
    }

    // we do not need to sort the lists
    Structure->EndUpdate(true,History);

    // THREAD SAFETY !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
    // do some magic with parent and thread ownership
    // return objects back to main thread
    Structure->moveToThread(QCoreApplication::instance()->thread());
    if( History ) History->moveToThread(QCoreApplication::instance()->thread());
    // END OF THREAD SAFETY !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

    return(result);
}

//------------------------------------------------------------------------------

bool CXYZImportJob::InjectCoordinates(void)
{
    emit OnStartProgressNotification(4);

    // read XYZ structure
    emit OnProgressNotification(1,"Total progress %p% - Reading XYZ structure ...");
    CXYZStructure xyz_structure;

    if( xyz_structure.Load(FileName) == false ){
        emit OnTextNotification(ETNT_ERROR,tr("Unable to read XYZ file!"),ETNT_ERROR_DELAY);
        return(true);
    }

    // check XYZ structure
    emit OnProgressNotification(2,"Total progress %p% - Checking XYZ structure ...");

    if( xyz_structure.GetNumberOfAtoms() != Structure->GetAtoms()->GetNumberOfAtoms() ){
        QString error = tr("Inconsistent number of atoms! Active structure: %1, injected structure: %2.");
        emit OnTextNotification(ETNT_ERROR,error.arg(Structure->GetAtoms()->GetNumberOfAtoms()).arg(xyz_structure.GetNumberOfAtoms()),ETNT_ERROR_DELAY);
        return(true);
    }

    int id = 0;
    foreach(QObject* p_qobj, Structure->GetAtoms()->children()){
        CAtom* p_atom = static_cast<CAtom*>(p_qobj);
        if( p_atom->GetZ() != PeriodicTable.SearchZBySymbol(xyz_structure.GetSymbol(id)) ){
            QString error = tr("Inconsistent atom no. %1! Active structure: %2, injected structure: %3.");
            emit OnTextNotification(ETNT_ERROR,error.arg(id+1).arg(QString(PeriodicTable.GetSymbol(p_atom->GetZ()))).arg(xyz_structure.GetSymbol(id)),ETNT_ERROR_DELAY);
            return(true);
        }
        id++;
    }

    // update coordinates
    emit OnProgressNotification(3,"Total progress %p% - Updating coordinates ...");

    id = 0;
    foreach(QObject* p_qobj, Structure->GetAtoms()->children()){
        CAtom* p_atom = static_cast<CAtom*>(p_qobj);
        p_atom->SetPos(xyz_structure.GetPosition(id));
        id++;
    }

    // THREAD SAFETY !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
    // do some magic with parent and thread ownership
    // return objects back to main thread
    Structure->moveToThread(QCoreApplication::instance()->thread());
    if( History ) History->moveToThread(QCoreApplication::instance()->thread());
    // END OF THREAD SAFETY !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

    return(true);
}

//------------------------------------------------------------------------------

bool CXYZImportJob::FinalizeJob(void)
{
    if( GetProject()->property("impex.inject") == true ){
        // we do not need to sort the lists
        Structure->EndUpdate(true,History);
    }

    emit OnProgressNotification(4,"Total progress %p% - Finalization ...");

    // close file
    sin.close();

    // THREAD SAFETY !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
    // do some magic with parent and thread ownership

    // restore parents
    Structure->setParent(OldMoleculeParent);
    if( History ) History->setParent(OldHistoryParent);

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

    if( History != NULL ){
        // end topology change
        Structure->EndChangeWH();
    }

    // adjust graphics
    if( GetProject()->property("impex.inject") == false ){
        AdjustGraphics();
    }

    if( History != NULL ){
        // end composite change
        Structure->EndChangeWH();
    }

    PushToRecentFiles();
    return(true);
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================
