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

#include "CPMDModule.hpp"
#include <CategoryUUID.hpp>
#include <ErrorSystem.hpp>
#include <QMessageBox>
#include <QFileDialog>
#include <QFile>
#include <QFileInfo>
#include <QTextStream>
#include <XMLElement.hpp>
#include <Structure.hpp>
#include <AtomList.hpp>
#include <Atom.hpp>
#include <PeriodicTable.hpp>
#include <Project.hpp>
#include <GlobalSetup.hpp>
#include <Residue.hpp>
#include <list>

#include "CPMDInputExportTool.hpp"

using namespace std;

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

QObject* CPMDInputExportToolCB(void* p_data);

CExtUUID        CPMDInputExportToolID(
                    "{CPMD_INPUT_EXPORT_TOOL:032882c3-3b99-4a59-abb8-c116047a108f}",
                    "CPMD Input (*.inp)",
                    "Create CPMD input file");

CPluginObject   CPMDInputExportToolObject(&CPMDPlugin,
                    CPMDInputExportToolID,EXPORT_STRUCTURE_CAT,
                    CPMDInputExportToolCB);

// -----------------------------------------------------------------------------

QObject* CPMDInputExportToolCB(void* p_data)
{
    CProject* p_project = static_cast<CProject*>(p_data);
    if( p_project == NULL ){
        ES_ERROR("CCPMDInputExportTool requires active project");
        return(NULL);
    }

    CCPMDInputExportTool* p_object = new CCPMDInputExportTool(p_project);
    p_object->ShowAsDialog();
    delete p_object;
    return(NULL);
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

CCPMDInputExportTool::CCPMDInputExportTool(CProject* p_project)
    : CWorkPanel(&CPMDInputExportToolObject,p_project,EWPR_DIALOG)
{
    WidgetUI.setupUi(this);    

    Structure = p_project->GetActiveStructure();

    // Connect the GUI elements to the correct slots
    connect(WidgetUI.titleLE, SIGNAL(textChanged(const QString &)),
        this, SLOT(UpdatePreviewText()));
    connect(WidgetUI.calculationCB, SIGNAL(currentIndexChanged(int)),
        this, SLOT(UpdatePreviewText()));
    connect(WidgetUI.chargeSB, SIGNAL(valueChanged(int)),
        this, SLOT(UpdatePreviewText()));
    connect(WidgetUI.cutoffSB, SIGNAL(valueChanged(int)),
        this, SLOT(UpdatePreviewText()));
    connect(WidgetUI.functionalCB, SIGNAL(currentIndexChanged(int)),
        this, SLOT(UpdatePreviewText()));
    connect(WidgetUI.pseudopotentialCB, SIGNAL(currentIndexChanged(int)),
        this, SLOT(UpdatePreviewText()));
    connect(WidgetUI.isolatedCB, SIGNAL(clicked()),
        this, SLOT(UpdatePreviewText()));

    connect(WidgetUI.previewText, SIGNAL(textChanged()),
        this, SLOT(PreviewEdited()));
    connect(WidgetUI.generateButton, SIGNAL(clicked()),
        this, SLOT(SaveInputFile()));
    connect(WidgetUI.resetButton, SIGNAL(clicked()),
        this, SLOT(ResetSetup()));
    connect(WidgetUI.saveMapButton, SIGNAL(clicked()),
            this, SLOT(SaveMapFile()));

    // reset setup
    ResetSetup();

    // load work panel setup
    LoadWorkPanelSetup();

    // update view - again with possibly new setup
    UpdatePreviewText();
}

//------------------------------------------------------------------------------

CCPMDInputExportTool::~CCPMDInputExportTool(void)
{
    SaveWorkPanelSetup();
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

void CCPMDInputExportTool::LoadWorkPanelSpecificData(CXMLElement* p_ele)
{
    if( p_ele == NULL ){
        INVALID_ARGUMENT("p_ele == NULL");
    }

    bool bdummy;
    if( p_ele->GetAttribute("saveSetup",bdummy) ){
        WidgetUI.saveSetupCB->setChecked(bdummy);
    }
    if( p_ele->GetAttribute("closeOnSave",bdummy) ){
        WidgetUI.closeOnSaveCB->setChecked(bdummy);
    }

    if( WidgetUI.saveSetupCB->isChecked() == false ) return;

    int dummy;

    if( p_ele->GetAttribute("calcType",dummy) ){
        WidgetUI.calculationCB->setCurrentIndex(dummy);
    }
    if( p_ele->GetAttribute("functional",dummy) ){
        WidgetUI.functionalCB->setCurrentIndex(dummy);
    }
    if( p_ele->GetAttribute("pseudopotential",dummy) ){
        WidgetUI.pseudopotentialCB->setCurrentIndex(dummy);
    }
    if( p_ele->GetAttribute("charge",dummy) ){
        WidgetUI.chargeSB->setValue(dummy);
    }
    if( p_ele->GetAttribute("cutoff",dummy) ){
        WidgetUI.cutoffSB->setValue(dummy);
    }
    if( p_ele->GetAttribute("isolated",bdummy) ){
        WidgetUI.isolatedCB->setChecked(bdummy);
    }

    CWorkPanel::LoadWorkPanelSpecificData(p_ele);
}

//------------------------------------------------------------------------------

void CCPMDInputExportTool::SaveWorkPanelSpecificData(CXMLElement* p_ele)
{
    if( p_ele == NULL ){
        INVALID_ARGUMENT("p_ele == NULL");
    }

    p_ele->SetAttribute("saveSetup",WidgetUI.saveSetupCB->isChecked());
    p_ele->SetAttribute("closeOnSave",WidgetUI.closeOnSaveCB->isChecked());

    if( WidgetUI.saveSetupCB->isChecked() ){
        p_ele->SetAttribute("calcType", WidgetUI.calculationCB->currentIndex());
        p_ele->SetAttribute("functional", WidgetUI.functionalCB->currentIndex());
        p_ele->SetAttribute("pseudopotential", WidgetUI.pseudopotentialCB->currentIndex());
        p_ele->SetAttribute("isolated", WidgetUI.isolatedCB->isChecked());
        p_ele->SetAttribute("charge", WidgetUI.chargeSB->value());
        p_ele->SetAttribute("cutoff", WidgetUI.cutoffSB->value());
    }

    CWorkPanel::SaveWorkPanelSpecificData(p_ele);
}

//------------------------------------------------------------------------------

void CCPMDInputExportTool::ResetSetup(void)
{
    // first unlock user changes
    if( ! WidgetUI.controlsW->isEnabled() ){
        WidgetUI.controlsW->setEnabled(true);
        UpdatePreviewText();
        return;
    }

    // Reset the form to defaults
    WidgetUI.titleLE->setText("Title");
    WidgetUI.calculationCB->setCurrentIndex(WF_OPT);
    WidgetUI.chargeSB->setValue(0);
    WidgetUI.cutoffSB->setValue(70);

    WidgetUI.functionalCB->setCurrentIndex(0);
    WidgetUI.pseudopotentialCB->setCurrentIndex(0);

    WidgetUI.controlsW->setEnabled(true);

    UpdatePreviewText();
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

void CCPMDInputExportTool::UpdatePreviewText(void)
{
    if( Structure == NULL ){
        WidgetUI.previewText->setText(tr("No structure is active in the project!"));
        WidgetUI.controlsW->setEnabled(false);
        return;
    }
    if( Structure->PBCInfo.Is3DPBCEnabled() == false ){
        WidgetUI.previewText->setText(tr("PBC has to be set for the active structure!"));
        WidgetUI.controlsW->setEnabled(false);
        return;
    }

    if( WidgetUI.controlsW->isEnabled() ){
        WidgetUI.previewText->setText(GenerateInputDeck());
        WidgetUI.controlsW->setEnabled(true);
    }
}

//------------------------------------------------------------------------------

void CCPMDInputExportTool::SaveInputFile(void)
{
    // parse formats list ------------------------
    QStringList filters;
    filters << "CPMD input file (*.inp)";
    filters << "All files (*)";

    QFileDialog* p_dialog = new QFileDialog();
    p_dialog->setDirectory (QString(GlobalSetup->GetLastOpenFilePath(CPMDInputExportToolID)));
    p_dialog->setNameFilters(filters);
    p_dialog->setFileMode(QFileDialog::AnyFile);
    p_dialog->setAcceptMode(QFileDialog::AcceptSave);
    p_dialog->setDefaultSuffix("inp");

    if( p_dialog->exec() == QDialog::Rejected ){
        return;
    }

    QString fileName = p_dialog->selectedFiles().at(0);
    GlobalSetup->SetLastOpenFilePathFromFile(fileName,CPMDInputExportToolID);

    delete p_dialog;

    QFileInfo finfo(fileName);
    if( finfo.suffix().isEmpty() ){
        fileName += ".inp";
    }

    QFile file(fileName);

    // open file
    if( ! file.open(QIODevice::WriteOnly | QIODevice::Text) ){
        QMessageBox::critical(NULL,"Error","Unable to open file!",QMessageBox::Ok,QMessageBox::Ok);
        return;
    }

    QString previewText = WidgetUI.previewText->toPlainText();
    QTextStream out(&file);
    out << previewText;

    // should we close this window?
    if( WidgetUI.closeOnSaveCB->isChecked() ){
        close();
    }
}

//------------------------------------------------------------------------------

void CCPMDInputExportTool::SaveMapFile(void)
{
    // parse formats list ------------------------
    QStringList filters;
    filters << "CPMD map file (*.map)";
    filters << "All files (*)";

    QFileDialog* p_dialog = new QFileDialog();
    p_dialog->setDirectory (QString(GlobalSetup->GetLastOpenFilePath(CPMDInputExportToolID)));
    p_dialog->setNameFilters(filters);
    p_dialog->setFileMode(QFileDialog::AnyFile);
    p_dialog->setAcceptMode(QFileDialog::AcceptSave);
    p_dialog->setDefaultSuffix("map");

    if( p_dialog->exec() == QDialog::Rejected ){
        return;
    }

    QString fileName = p_dialog->selectedFiles().at(0);
    GlobalSetup->SetLastOpenFilePathFromFile(fileName,CPMDInputExportToolID);

    delete p_dialog;

    QFileInfo finfo(fileName);
    if( finfo.suffix().isEmpty() ){
        fileName += ".map";
    }

    QFile file(fileName);

    // open file
    if( ! file.open(QIODevice::WriteOnly | QIODevice::Text) ){
        QMessageBox::critical(NULL,"Error","Unable to open file!",QMessageBox::Ok,QMessageBox::Ok);
        return;
    }

    QTextStream out(&file);

    out << "# CPMDID   AtomID  AtomName  ResID  ResName" << Qt::endl;
    out << "# ------- -------- -------- ------- -------" << Qt::endl;

    std::list<int>  elements;

    // generate list of unique atoms
    foreach(QObject* p_qatom, Structure->GetAtoms()->children()) {
        CAtom* p_atom = static_cast<CAtom*>(p_qatom);
        elements.push_back(p_atom->GetZ());
    }
    elements.sort();
    elements.unique();

    // for each element
    int i = 1;
    std::list<int>::iterator it = elements.begin();
    std::list<int>::iterator ie = elements.end();
    while( it != ie ){
        foreach(QObject* p_qatom, Structure->GetAtoms()->children()) {
            CAtom* p_atom = static_cast<CAtom*>(p_qatom);
            if( p_atom->GetZ() != *it ) continue;
            out << Qt::right << qSetFieldWidth(9) << i << qSetFieldWidth(1) << " ";
            out << qSetFieldWidth(8) << p_atom->GetSerIndex() << qSetFieldWidth(1) << " ";
            out << qSetFieldWidth(8) << Qt::left << p_atom->GetName() << Qt::right << qSetFieldWidth(1) << " ";
            out << qSetFieldWidth(7) << p_atom->GetResidue()->GetSeqIndex() << qSetFieldWidth(1) << " ";
            out << qSetFieldWidth(7) << Qt::left << p_atom->GetResidue()->GetName() << qSetFieldWidth(1) << Qt::endl;
            i++;
        }
        it++;
    }

    // should we close this window?
    if( WidgetUI.closeOnSaveCB->isChecked() ){
        close();
    }
}

//------------------------------------------------------------------------------

void CCPMDInputExportTool::PreviewEdited(void)
{
    // Determine if the preview text has changed from the form generated
    WidgetUI.controlsW->setEnabled(WidgetUI.previewText->toPlainText() == GenerateInputDeck() );
}

//------------------------------------------------------------------------------

QString CCPMDInputExportTool::GenerateInputDeck(void)
{
    if( Structure == NULL ) return("");

    // Generate an input deck based on the settings of the dialog
    QString buffer;
    QTextStream mol(&buffer);

    GenerateHeader(mol);
    GenerateCPMD(mol);
    GenerateSystem(mol);
    GenerateDFT(mol);
    GenerateAtoms(mol);

    return buffer;
}

//------------------------------------------------------------------------------

void CCPMDInputExportTool::GenerateHeader(QTextStream& str)
{
    str << "&INFO" << Qt::endl;
    str << "    " << WidgetUI.titleLE->text() << Qt::endl;
    str << "&END" << Qt::endl;
    str << Qt::endl;
}

//------------------------------------------------------------------------------

void CCPMDInputExportTool::GenerateCPMD(QTextStream& str)
{
    str << "&CPMD" << Qt::endl;
    switch( WidgetUI.calculationCB->currentIndex() ){
        case WF_OPT:
    str << "    OPTIMIZE WAVEFUNCTION" << Qt::endl;
    str << "    INITIALIZE WAVEFUNCTION ATOMS" << Qt::endl;
    str << "    MAXSTEP" << Qt::endl;
    str << "        500" << Qt::endl;
    str << "    CONVERGENCE ORBITALS" << Qt::endl;
    str << "        1.0E-7" << Qt::endl;
        break;
        case GEO_OPT:
    str << "    OPTIMIZE GEOMETRY XYZ" << Qt::endl;
    str << "    INITIALIZE WAVEFUNCTION ATOMS" << Qt::endl;
    str << "    HESSIAN UNIT" << Qt::endl;
    str << "    LBFGS" << Qt::endl;
    str << "    MAXSTEP" << Qt::endl;
    str << "        500" << Qt::endl;
    str << "    CONVERGENCE ORBITALS" << Qt::endl;
    str << "        1.0E-6" << Qt::endl;
    str << "    CONVERGENCE GEOMETRY" << Qt::endl;
    str << "        5.0E-4" << Qt::endl;
        break;
        case CP_DYN:
    str << "    MOLECULAR DYNAMICS CP" << Qt::endl;
    str << "    RESTART WAVEFUNCTION COORDINATES LATEST" << Qt::endl;
    str << "    MAXSTEP" << Qt::endl;
    str << "        50000" << Qt::endl;
    str << "    TIMESTEP" << Qt::endl;
    str << "        5" << Qt::endl;
    str << "    EMASS" << Qt::endl;
    str << "        600" << Qt::endl;
    str << "    TRAJECTORY XYZ" << Qt::endl;
    str << "    TRAJECTORY SAMPLE" << Qt::endl;
    str << "        10" << Qt::endl;
    if( WidgetUI.isolatedCB->isChecked() ){
    str << "    ISOLATED MOLECULE" << Qt::endl;
    }
        break;
    }

    if( WidgetUI.isolatedCB->isChecked() ){
    str << "    CENTER MOLECULE" << Qt::endl;
    }
    str << "&END" << Qt::endl;
    str << Qt::endl;
}

//------------------------------------------------------------------------------

void CCPMDInputExportTool::GenerateSystem(QTextStream& str)
{
    str << "&SYSTEM" << Qt::endl;
    str << "    ANGSTROM" << Qt::endl;
    if( WidgetUI.isolatedCB->isChecked() ){
    str << "    SYMMETRY" << Qt::endl;
    str << "        0" << Qt::endl;
    str << "    CELL ABSOLUTE DEGREES" << Qt::endl;
    str << qSetFieldWidth(13) << qSetRealNumberPrecision(6) << Qt::forcepoint << Qt::fixed << Qt::right;
    str << Structure->PBCInfo.GetAVectorSize();
    str << Structure->PBCInfo.GetBVectorSize();
    str << Structure->PBCInfo.GetCVectorSize();
    str << qSetFieldWidth(0);
    str << " 90.0 90.0 90.0" << Qt::endl;
    str << "    POISSON SOLVER HOCKNEY" << Qt::endl;
    } else {
    str << "    CELL VECTORS" << Qt::endl;
    str << qSetFieldWidth(13) << qSetRealNumberPrecision(6) << Qt::forcepoint << Qt::fixed << Qt::right;
    str << Structure->PBCInfo.GetAVector().x << Structure->PBCInfo.GetAVector().y << Structure->PBCInfo.GetAVector().z << Qt::endl;
    str << Structure->PBCInfo.GetBVector().x << Structure->PBCInfo.GetBVector().y << Structure->PBCInfo.GetBVector().z << Qt::endl;
    str << Structure->PBCInfo.GetCVector().x << Structure->PBCInfo.GetCVector().y << Structure->PBCInfo.GetCVector().z << Qt::endl;
    str << qSetFieldWidth(0);
    }
    str << "    CUTOFF" << Qt::endl;
    str << "        " << WidgetUI.cutoffSB->value() << Qt::endl;
    str << "    CHARGE" << Qt::endl;
    str << "        " << WidgetUI.chargeSB->value() << Qt::endl;
    str << "&END" << Qt::endl;
    str << Qt::endl;
}

//------------------------------------------------------------------------------

void CCPMDInputExportTool::GenerateDFT(QTextStream& str)
{
    str << "&DFT" << Qt::endl;
    str << "    FUNCTIONAL " << WidgetUI.functionalCB->currentText() << Qt::endl;
    str << "&END" << Qt::endl;
    str << Qt::endl;
}

//------------------------------------------------------------------------------

void CCPMDInputExportTool::GenerateAtoms(QTextStream& str)
{
    std::list<int>  elements;

    // generate list of unique atoms
    foreach(QObject* p_qatom, Structure->GetAtoms()->children()) {
        CAtom* p_atom = static_cast<CAtom*>(p_qatom);
        elements.push_back(p_atom->GetZ());
    }
    elements.sort();
    elements.unique();

    str << "&ATOMS" << Qt::endl;

    // for each element
    std::list<int>::iterator it = elements.begin();
    std::list<int>::iterator ie = elements.end();
    while( it != ie ){
        str << "*" << PeriodicTable.GetSymbol(*it)<< "_" << WidgetUI.pseudopotentialCB->currentText() << "_" << WidgetUI.functionalCB->currentText() << ".psp";
        if( WidgetUI.pseudopotentialCB->currentText() == "MT" ){
        str << " KLEINMAN-BYLANDER" << Qt::endl;
        }
        str << "    LMAX=" << GetLMax(*it) << Qt::endl;
        int count = 0;
        foreach(QObject* p_qatom, Structure->GetAtoms()->children()) {
            CAtom* p_atom = static_cast<CAtom*>(p_qatom);
            if( p_atom->GetZ() != *it ) continue;
            count++;
        }
        str << "    " << count << Qt::endl;
        foreach(QObject* p_qatom, Structure->GetAtoms()->children()) {
            CAtom* p_atom = static_cast<CAtom*>(p_qatom);
            if( p_atom->GetZ() != *it ) continue;
            str << qSetFieldWidth(13) << qSetRealNumberPrecision(6) << Qt::forcepoint
                << Qt::fixed << Qt::right << p_atom->GetPos().x << p_atom->GetPos().y
                << p_atom->GetPos().z << qSetFieldWidth(0) << '\n';
        }
        it++;
    }

    str << "&END" << Qt::endl;
    str << Qt::endl;
}

//------------------------------------------------------------------------------

QString CCPMDInputExportTool::GetLMax(int z)
{
    if( z <= 2 ) return("S");
    if( z <= 10 ) return("P");
    return("D");
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================


