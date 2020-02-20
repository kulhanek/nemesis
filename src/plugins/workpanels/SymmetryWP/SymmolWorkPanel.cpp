// =============================================================================
// NEMESIS - Molecular Modelling Package
// -----------------------------------------------------------------------------
//    Copyright (C) 2018 Petr Kulhanek, kulhanek@chemi.muni.cz
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

#include <StructureList.hpp>
#include <Structure.hpp>
#include <Project.hpp>
#include <AtomList.hpp>
#include <Atom.hpp>
#include <HistoryList.hpp>
#include <QString>
#include <QMessageBox>
#include <MainWindow.hpp>
#include <StructureSelection.hpp>
#include <fstream>
#include <QProcess>
#include <PeriodicTable.hpp>
#include <string>
#include <Structure.hpp>
#include <AtomListHistory.hpp>

#include "SymmetryModule.hpp"
#include "SymmolWorkPanel.hpp"

#include <boost/format.hpp>

//------------------------------------------------------------------------------

using namespace std;
using namespace boost;

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

QObject* SymmolWorkPanelCB(void* p_data);

CExtUUID        SymmolWorkPanelID(
                    "{SYMMOL_WORK_PANEL:d95a2b28-0fa9-4e7e-a84e-13f838edf9d7}",
                    "Symmol");

CPluginObject   SymmolWorkPanelObject(&SymmetryPlugin,
                    SymmolWorkPanelID,WORK_PANEL_CAT,
                    ":/images/SymmetryWP/SymmolWorkPanel.svg",
                    SymmolWorkPanelCB);

// -----------------------------------------------------------------------------

QObject* SymmolWorkPanelCB(void* p_data)
{
    CProject* p_project = static_cast<CProject*>(p_data);
    if( p_project == NULL ){
        ES_ERROR("CSymmolWorkPanel requires active project");
        return(NULL);
    }

    QObject* p_build_wp = new CSymmolWorkPanel(p_project);
    return(p_build_wp);
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

CSymmolWorkPanel::CSymmolWorkPanel(CProject* p_project)
    : CWorkPanel(&SymmolWorkPanelObject,p_project,EWPR_TOOL)
{
    WidgetUI.setupUi(this);

    // set initial structure
    WidgetUI.structureW->setProject(p_project);
    WidgetUI.structureW->setObjectBaseMIMEType("structure.indexes");
    WidgetUI.structureW->setSelectionHandler(&SH_Structure);
    WidgetUI.structureW->setObject(p_project->GetStructures()->GetActiveStructure());

    // signals
    connect(WidgetUI.runPB,SIGNAL(clicked(bool)),
        this,SLOT(RunSymmol(void)));
    // -------------
    connect(WidgetUI.symmetrizePB,SIGNAL(clicked(bool)),
        this,SLOT(SymmetrizeStructure(void)));
    // -------------
    connect(WidgetUI.structureW,SIGNAL(OnObjectChanged(void)),
        this,SLOT(ClearAll(void)));
    // -------------
    connect(p_project->GetHistory(),SIGNAL(OnHistoryChanged(EHistoryChangeMessage)),
        this,SLOT(ClearAll(void)));

    // load work panel setup
    LoadWorkPanelSetup();

    // init data
    ClearAll();
}

//------------------------------------------------------------------------------

CSymmolWorkPanel::~CSymmolWorkPanel()
{
    SaveWorkPanelSetup();
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

void CSymmolWorkPanel::LoadWorkPanelSpecificData(CXMLElement* p_ele)
{
    if( p_ele == NULL ){
        INVALID_ARGUMENT("p_ele == NULL");
    }

    double ddummy;
    if( p_ele->GetAttribute("dcm",ddummy) ){
        WidgetUI.dcmSB->setValue(ddummy);
    }
}

//------------------------------------------------------------------------------

void CSymmolWorkPanel::SaveWorkPanelSpecificData(CXMLElement* p_ele)
{
    if( p_ele == NULL ){
        INVALID_ARGUMENT("p_ele == NULL");
    }
    p_ele->SetAttribute("dcm",WidgetUI.dcmSB->value());

    CWorkPanel::SaveWorkPanelSpecificData(p_ele);
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

void CSymmolWorkPanel::ClearAll(void)
{
    bool can_run = false;
    if( WidgetUI.structureW->getObject() != NULL ){
        CStructure* p_str = WidgetUI.structureW->getObject<CStructure*>();
        if( p_str ) can_run = p_str->GetAtoms()->GetNumberOfAtoms() > 0;
    }
    WidgetUI.runPB->setEnabled(can_run);
    WidgetUI.symbolLE->setText("C1");
    WidgetUI.csmLE->setText("0.0000");
    WidgetUI.rmsLE->setText("0.0000");
    WidgetUI.symmetrizePB->setEnabled(false);
}

//------------------------------------------------------------------------------

void CSymmolWorkPanel::RunSymmol(void)
{
    SymmolOutput = QString();
    ClearAll();

    CStructure* p_str = WidgetUI.structureW->getObject<CStructure*>();
    if( p_str == NULL ) return;

    if( ! TmpDir.isValid() ){
        GetProject()->TextNotification(ETNT_ERROR,"unable to setup temporary working directory for symmol",5000);
        return;
    }
    QDir workdir(TmpDir.path());

    // create imput file
    QString stdin_name = workdir.absoluteFilePath("stdin");
    QString stdout_name = workdir.absoluteFilePath("stdout");

    ofstream ofs(stdin_name.toStdString().c_str());
    if( ! ofs ){
        GetProject()->TextNotification(ETNT_ERROR,"unable to open file for symmol input",5000);
        return;
    }

    ofs << "# structure generated by Nemesis" <<  endl;
    ofs << "1.00000  1.00000  1.00000 90.00000 90.00000 90.00000" << endl;
    ofs << " 1 1 " << format("%10.4f %10.4f") % WidgetUI.dcmSB->value() %  WidgetUI.dcmSB->value() << endl;

    foreach(QObject* p_obj,p_str->GetAtoms()->children()){
        CAtom* p_atom = dynamic_cast<CAtom*>(p_obj);
        if( p_atom == NULL ) continue;
        int grp = 1;
        CPoint pos = p_atom->GetPos();
        ofs << format("%-6s%2d%12.5f%12.5f%12.5f") % PeriodicTable.GetSymbol(p_atom->GetZ())
                          % grp % pos.x %pos.y % pos.z << endl;
    }
    ofs.close();

    // execute symmol
    QProcess proc;
    proc.setWorkingDirectory(TmpDir.path());
    proc.setReadChannelMode(QProcess::SeparateChannels);
    proc.setStandardInputFile(stdin_name);
    proc.setStandardOutputFile(stdout_name);
    proc.start("symmol");
    bool finished = false;
    for(int i=0;i < 100; i++){
        QCoreApplication::processEvents();
        finished = proc.waitForFinished(100);
        if( proc.state() != QProcess::NotRunning ){
            finished = true;
            break;
        }
        if( finished ) break;
    }
    if( ! finished ){
        proc.kill();
    }
    if( (proc.exitStatus() != QProcess::NormalExit) && (proc.exitCode() != 0) ){
        GetProject()->TextNotification(ETNT_ERROR,"unable to execute symmol",5000);
        return;
    }

    // parse output
    ifstream ifs(stdout_name.toStdString().c_str());
    if( ! ifs ){
        GetProject()->TextNotification(ETNT_ERROR,"unable to open stdout from symmol",5000);
        return;
    }

    std::string  line;
    bool    found = false;
    while( getline(ifs,line) ){
        if( line.find("Schoenflies symbol =") != string::npos ){
            stringstream str(line);
            string buff;
            //  Schoenflies symbol = D6h      CSM =  0.0000     Molecular RMS =  0.0000
            str >> buff >> buff >> buff >> buff;
            WidgetUI.symbolLE->setText(QString::fromStdString(buff));
            str >> buff >> buff >> buff;
            WidgetUI.csmLE->setText(QString::fromStdString(buff));
            str >> buff >> buff >> buff >> buff;
            WidgetUI.rmsLE->setText(QString::fromStdString(buff));
            found = true;
            break;
        }
        if( line.find("SYMMETRIZATION OF GROUP NR. 1") != string::npos ){
            found = true;
        }
    }

    if( found == false ){
        GetProject()->TextNotification(ETNT_ERROR,"unable to parse stdout from symmol",5000);
        return;
    }

    if( WidgetUI.symbolLE->text() == "C1" ){
        GetProject()->TextNotification(ETNT_ERROR,"no symmetry detected",5000);
    }

    SymmolOutput = workdir.absoluteFilePath("symmol.out");
    if( WidgetUI.symbolLE->text() != "C1" ){
        WidgetUI.symmetrizePB->setEnabled(true);
    }
}

//------------------------------------------------------------------------------

void CSymmolWorkPanel::SymmetrizeStructure(void)
{
    CStructure* p_str = WidgetUI.structureW->getObject<CStructure*>();
    if( p_str == NULL ) return;

    // parse output
    ifstream ifs(SymmolOutput.toStdString().c_str());
    if( ! ifs ){
        GetProject()->TextNotification(ETNT_ERROR,"unable to open symmol.out",5000);
        return;
    }

    std::string  line;
    bool    found = false;
    while( getline(ifs,line) ){
        if( line.find("SYMMETRIZED ORTHOGONAL COORDINATES") != string::npos ){
            found = true;
            break;
        }
    }

    if( ! found ){
        GetProject()->TextNotification(ETNT_ERROR,"unable to open read symmetized coordinates",5000);
        return;
    }

    CHistoryNode* p_root = p_str->BeginChangeWH(EHCL_GEOMETRY,"symmetrize structure");
    CAtomListCoordinatesHI* p_history = new CAtomListCoordinatesHI(p_str);
    p_root->Register(p_history);
    p_str->BeginUpdate(p_root);

    foreach(QObject* p_qobj, p_str->GetAtoms()->children()){
        CAtom* p_atom = static_cast<CAtom*>(p_qobj);
        if( getline(ifs,line) ){
            CPoint pos;
            stringstream str(line);
            string buff;
            str >> buff >> buff >> pos.x >> pos.y >> pos.z >> buff >> buff >> buff >> buff;
            p_atom->SetPos(pos);
            if( str ){
                p_atom->SetDescription(QString::fromStdString(buff),p_root);
            }
        }
    }

    p_str->EndUpdate(true,p_root);
    p_str->EndChangeWH();
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================



