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

#include <QtGui>

#include <PluginObject.hpp>
#include <ProjectList.hpp>
#include <ExtUUID.hpp>
#include <CategoryUUID.hpp>

#include <ErrorSystem.hpp>
#include <GlobalSetup.hpp>
#include <Project.hpp>
#include <Structure.hpp>
#include <AtomList.hpp>
#include <BondList.hpp>
#include <ResidueList.hpp>
#include <Atom.hpp>
#include <Bond.hpp>
#include <Residue.hpp>
#include <PeriodicTable.hpp>
#include <MainWindow.hpp>
#include <JobList.hpp>
#include <QMessageBox>

#include <boost/algorithm/string.hpp>
#include <boost/format.hpp>

#include "AmberModule.hpp"

#include "OFFExportJob.hpp"

#include <fstream>

using namespace std;
using namespace boost;


// OFF documentation
// https://ambermd.org/doc/OFF_file_format.txt

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

QObject* OFFExportJobCB(void* p_data);

CExtUUID        OFFExportJobID(
                    "{OFF_ASCII_EXPORT_JOB:32397735-9ca5-44a2-81c0-07d9c47e002a}",
                    "Amber Object File Format (OFF) Export");

CPluginObject   OFFExportJobObject(&AmberPlugin,
                    OFFExportJobID,JOB_CAT,
                    NULL);

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

COFFExportJob::COFFExportJob(CStructure* p_mol,const QString& name)
    : CJob(&OFFExportJobObject,p_mol->GetProject())
{
    Structure = p_mol;
    FileName = name;

    Structure->GetProject()->GetJobs()->RegisterJob(this);

    MaxTicks = 0;
    Quantum = 0;
    Tick = 0;
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

bool COFFExportJob::JobAboutToBeSubmitted(void)
{
    sout.open(FileName.toLatin1());
    if( !sout ) {
        QMessageBox::critical(GetProject()->GetMainWindow(),tr("Error"),tr("Unable to open file for writing!"),QMessageBox::Ok,QMessageBox::Ok);
        ES_ERROR("Cannot open file to write");
        return(false);
    }

    // lock history
    CProject* p_project = Structure->GetProject();

    BackupLockLevels = p_project->GetHistory()->GetLockModeLevels();
    CLockLevels super_lock = ~CLockLevels();
    p_project->GetHistory()->SetLockModeLevels(super_lock);

    return(true);
}

//------------------------------------------------------------------------------

bool COFFExportJob::InitializeJob(void)
{
    MaxTicks = 2*Structure->GetAtoms()->GetNumberOfAtoms();

    CProject* p_project = Structure->GetProject();
    p_project->StartProgressNotification(MaxTicks);

    connect(this,SIGNAL(OnProgressNotification(int,const QString&)),
            p_project,SLOT(ProgressNotification(int,const QString&)));

    return(true);
}

//------------------------------------------------------------------------------

bool COFFExportJob::ExecuteJob(void)
{
    Tick = 1;
    Quantum = MaxTicks / 100;
    if( Quantum == 0 ){
        Quantum = 1;
    }

    emit OnProgressNotification(Tick,"Total progress %p% - Initialization ...");

    // write to stream
    WriteOFF();

    emit OnProgressNotification(Tick,"Total progress %p% - Finalization ...");

    return(true);
}

//------------------------------------------------------------------------------

bool COFFExportJob::FinalizeJob(void)
{
    // close file
    sout.close();

    // unlock history list
    CProject* p_project = Structure->GetProject();
    p_project->EndProgressNotification();
    p_project->GetHistory()->SetLockModeLevels(BackupLockLevels);

    return(true);
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

void COFFExportJob::WriteOFF(void)
{
    if( Structure->GetAtoms()->GetNumberOfAtoms() == 0 ) {
        return;
    }

    QString unit_name = Structure->GetName();
    unit_name.replace(" ","_");

    sout << "!!index array str" << endl;
    sout << " \"" << unit_name.toStdString() << "\"" << endl;

    sout << "!entry." << unit_name.toStdString() << ".unit.atoms table  str name  str type  int typex  int resx  int flags  int seq  int elmnt  dbl chg" << endl;
    //  "AN" "AT" typex resx flags sequence element charge

    int indx = 1;
    foreach(QObject* p_qobj, Structure->GetAtoms()->children()){
        CAtom* p_atom = static_cast<CAtom*>(p_qobj);

        IndexMap[p_atom->GetIndex()] = indx;
        indx++;

        sout << " \"" << p_atom->GetName().toStdString() << "\" ";
        sout << "\"" << p_atom->GetType().toStdString() << "\" ";
        sout << 0 << " ";
        if( p_atom->GetResidue() != NULL ){
            sout << p_atom->GetResidue()->GetSeqIndex() << " ";
        } else {
            sout << 1 << " ";
        }
        sout << "131072 ";
        sout << p_atom->GetLocIndex() << " ";
        sout << p_atom->GetZ() << " ";
        sout << format( "%12.7f" ) % p_atom->GetCharge() << endl;

        if( Tick % Quantum == 0 ){
            emit OnProgressNotification(Tick,"Total progress %p% - Saving atom positions (1/2) ...");
        }
        Tick++;
    }

    sout << "!entry." << unit_name.toStdString() << ".unit.atomspertinfo table  str pname  str ptype  int ptypex  int pelmnt  dbl pchg" << endl;
    // "AN" "AT" ptypex pelmnt pchg

    foreach(QObject* p_qobj, Structure->GetAtoms()->children()){
        CAtom* p_atom = static_cast<CAtom*>(p_qobj);

        sout << " \"" << p_atom->GetName().toStdString() << "\" ";
        sout << "\"" << p_atom->GetType().toStdString() << "\" ";
        sout << 0 << "0 -1 0.0" << endl;

        if( Tick % Quantum == 0 ){
            emit OnProgressNotification(Tick,"Total progress %p% - Saving atom positions (1/2) ...");
        }
        Tick++;
    }

    sout << "!entry." << unit_name.toStdString() << ".unit.boundbox array dbl" << endl;

    if( Structure->PBCInfo.IsValid() ){
        sout << " 1.0" <<  endl;

        // FIXME: check if Beta and Gamma ara the same as Alpha
        //     sout << format("%12.7f") % (Structure->PBCInfo.GetBeta()*180.0/M_PI);
        //     sout << format("%12.7f") % (Structure->PBCInfo.GetGamma()*180.0/M_PI);

        sout << format("%12.7f") % (Structure->PBCInfo.GetAlpha()*180.0/M_PI) <<  endl;
        sout << format( "%12.7f" ) % Structure->PBCInfo.GetAVectorSize() <<  endl;
        sout << format( "%12.7f" ) % Structure->PBCInfo.GetBVectorSize() <<  endl;
        sout << format( "%12.7f" ) % Structure->PBCInfo.GetCVectorSize() <<  endl;

    } else {
        sout << " -1.0" <<  endl;
        sout << "  0.0" <<  endl;
        sout << "  0.0" <<  endl;
        sout << "  0.0" <<  endl;
        sout << "  0.0" <<  endl;
    }

    sout << "!entry." << unit_name.toStdString() << ".unit.childsequence single int" << endl;
    sout << " " << Structure->GetResidues()->GetNumberOfResidues() + 1 << endl;

    sout << "!entry." << unit_name.toStdString() << ".unit.connect array int" << endl;
    sout << " 0" <<  endl;
    sout << " 0" <<  endl;

    sout << "!entry." << unit_name.toStdString() << ".unit.connectivity table  int atom1x  int atom2x  int flags" << endl;
    //atom1x atom2x flag

    foreach(QObject* p_qobj, Structure->GetBonds()->children()){
        CBond* p_bond = static_cast<CBond*>(p_qobj);

        int first = IndexMap[p_bond->GetFirstAtom()->GetIndex()];
        int second = IndexMap[p_bond->GetSecondAtom()->GetIndex()];
        sout << format( " %8d " ) % std::min( first, second );
        sout << format( "%8d " ) % std::max( first, second );
        sout << 1 << endl;

        if( Tick % Quantum == 0 ){
            emit OnProgressNotification(Tick,"Total progress %p% - Saving atom positions (1/2) ...");
        }
        Tick++;
    }

    sout << "!entry." << unit_name.toStdString() << ".unit.hierarchy table  str abovetype  int abovex  str belowtype  int belowx" << endl;
    // abovetype abovex belowtype belowx

    foreach(QObject* p_qobj, Structure->GetResidues()->children()){
        CResidue* p_res = static_cast<CResidue*>(p_qobj);

        sout << " \"U\" " << "0 " << "\"R\" " << p_res->GetSeqIndex() << endl;

        foreach(QObject* p_qobj, p_res->GetAtoms()){
            CAtom* p_atom = static_cast<CAtom*>(p_qobj);
            int aindx = IndexMap[p_atom->GetIndex()];
            sout << " \"R\" " << p_res->GetSeqIndex() << " \"A\" " << aindx << endl;
        }
    }

    sout << "!entry." << unit_name.toStdString() << ".unit.name single str" << endl;
    sout << " \"" << unit_name.toStdString() << "\"" << endl;

    sout << "!entry." << unit_name.toStdString() << ".unit.positions table  dbl x  dbl y  dbl z" << endl;
    //     x   y   z

    foreach(QObject* p_qobj, Structure->GetAtoms()->children()){
        CAtom* p_atom = static_cast<CAtom*>(p_qobj);

        CPoint pos = p_atom->GetPos();
        sout << format( " %12.7f " ) % pos.x;
        sout << format( "%12.7f " ) % pos.y;
        sout << format( "%12.7f" )  % pos.z << endl;

        if( Tick % Quantum == 0 ){
            emit OnProgressNotification(Tick,"Total progress %p% - Saving atom positions (1/2) ...");
        }
        Tick++;
    }

    sout << "!entry." << unit_name.toStdString() << ".unit.residueconnect table  int c1x  int c2x  int c3x  int c4x  int c5x  int c6x" << endl;
    //        c1x   c2x   c3x   c4x   c5x   c6x
    foreach(QObject* p_qobj, Structure->GetResidues()->children()){
        CResidue* p_res = static_cast<CResidue*>(p_qobj);
        sout << " 0 0 0 0 0 0" << endl;
    }

    sout << "!entry." << unit_name.toStdString() << ".unit.residues table  str name  int seq  int childseq  int startatomx  str restype  int imagingx" << endl;
    // "RES"   seq   childseq   startatomx   restype   imagingx

    int startat = 1;
    foreach(QObject* p_qobj, Structure->GetResidues()->children()){
        CResidue* p_res = static_cast<CResidue*>(p_qobj);

        sout << " \"" << p_res->GetName().toStdString() << "\" ";
        sout << p_res->GetSeqIndex() << " ";
        sout << p_res->GetNumberOfAtoms() + 1 << " ";
        sout << startat << " ";
        sout << "\"?\" ";
        startat += p_res->GetNumberOfAtoms();
        sout << 0 << endl;

        if( Tick % Quantum == 0 ){
            emit OnProgressNotification(Tick,"Total progress %p% - Saving atom positions (1/2) ...");
        }
        Tick++;
    }

    sout << "!entry." << unit_name.toStdString() << ".unit.residuesPdbSequenceNumber array int" << endl;
    foreach(QObject* p_qobj, Structure->GetResidues()->children()){
        CResidue* p_res = static_cast<CResidue*>(p_qobj);
        sout << " " << p_res->GetSeqIndex() << endl;

        if( Tick % Quantum == 0 ){
            emit OnProgressNotification(Tick,"Total progress %p% - Saving atom positions (1/2) ...");
        }
        Tick++;
    }

    sout << "!entry." << unit_name.toStdString() << ".unit.solventcap array dbl" << endl;
    sout << " -1.0" <<  endl;
    sout << "  0.0" <<  endl;
    sout << "  0.0" <<  endl;
    sout << "  0.0" <<  endl;
    sout << "  0.0" <<  endl;

    sout << "!entry." << unit_name.toStdString() << ".unit.velocities table  dbl x  dbl y  dbl z" << endl;
    // v_x   v_y   v_z

    foreach(QObject* p_qobj, Structure->GetAtoms()->children()){
        CAtom* p_atom = static_cast<CAtom*>(p_qobj);

        CPoint vel = p_atom->GetVel();
        sout << format( " %12.7f " ) % vel.x;
        sout << format( "%12.7f " ) % vel.y;
        sout << format( "%12.7f" )  % vel.z << endl;

        if( Tick % Quantum == 0 ){
            emit OnProgressNotification(Tick,"Total progress %p% - Saving atom velocities (1/2) ...");
        }
        Tick++;
    }

}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================
