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
/**********************************************************************
  CGaussianInputExportTool - Dialog for generating Gaussian input decks

  Copyright (C) 2008 Marcus D. Hanwell
  Copyright (C) 2008 Michael Banck

  This file is part of the Avogadro molecular editor project.
  For more information, see <http://avogadro.openmolecules.net/>

  Avogadro is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  Avogadro is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
  02110-1301, USA.
 **********************************************************************/

#include "GaussianModule.hpp"
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
#include <OpenBabelUtils.hpp>
#include <openbabel/obiter.h>
#include <list>

#include "GaussianInputExportTool.hpp"
#include "GaussianInputExportTool.moc"

using namespace OpenBabel;
using namespace std;

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

QObject* GaussianInputExportToolCB(void* p_data);

CExtUUID        GaussianInputExportToolID(
                    "{GAUSSIAN_INPUT_EXPORT_TOOL:9067cbb2-772b-4acb-b66f-3732e0775b2c}",
                    "Gaussian Input (*.com)");

CPluginObject   GaussianInputExportToolObject(&GaussianPlugin,
                    GaussianInputExportToolID,EXPORT_STRUCTURE_CAT,
                    GaussianInputExportToolCB);

// -----------------------------------------------------------------------------

QObject* GaussianInputExportToolCB(void* p_data)
{
    CProject* p_project = static_cast<CProject*>(p_data);
    if( p_project == NULL ){
        ES_ERROR("CGaussianInputExportTool requires active project");
        return(NULL);
    }

    CGaussianInputExportTool* p_object = new CGaussianInputExportTool(p_project);
    p_object->ShowAsDialog();
    delete p_object;
    return(NULL);
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

CGaussianInputExportTool::CGaussianInputExportTool(CProject* p_project)
    : CWorkPanel(&GaussianInputExportToolObject,p_project,EWPR_DIALOG)
{
    WidgetUI.setupUi(this);

    Structure = p_project->GetActiveStructure();

    // Connect the GUI elements to the correct slots
    connect(WidgetUI.titleLE, SIGNAL(textChanged(const QString &)),
        this, SLOT(UpdatePreviewText()));
    //----------------
    connect(WidgetUI.calculationCB, SIGNAL(currentIndexChanged(int)),
        this, SLOT(UpdatePreviewText()));
    //----------------
    connect(WidgetUI.theoryCB, SIGNAL(currentIndexChanged(int)),
        this, SLOT(SetTheory()));
    //----------------
    connect(WidgetUI.basisCB, SIGNAL(currentIndexChanged(int)),
        this, SLOT(UpdatePreviewText()));
    //----------------
    connect(WidgetUI.multiplicitySB, SIGNAL(valueChanged(int)),
        this, SLOT(UpdatePreviewText()));
    //----------------
    connect(WidgetUI.chargeSB, SIGNAL(valueChanged(int)),
        this, SLOT(UpdatePreviewText()));
    //----------------
    connect(WidgetUI.procSB, SIGNAL(valueChanged(int)),
        this, SLOT(UpdatePreviewText()));
    //----------------
    connect(WidgetUI.coordCB, SIGNAL(currentIndexChanged(int)),
        this, SLOT(UpdatePreviewText()));
    //----------------
    connect(WidgetUI.checkpointCB, SIGNAL(toggled(bool)),
        this, SLOT(UpdatePreviewText()));
    //----------------
    connect(WidgetUI.includePBCCB, SIGNAL(toggled(bool)),
        this, SLOT(UpdatePreviewText()));
    //----------------
    connect(WidgetUI.includeFragCB, SIGNAL(toggled(bool)),
        this, SLOT(UpdatePreviewText()));
    //----------------
    connect(WidgetUI.previewText, SIGNAL(textChanged()),
        this, SLOT(PreviewEdited()));
    //----------------
    connect(WidgetUI.saveButton, SIGNAL(clicked()),
        this, SLOT(SaveInputFile()));
    //----------------
    connect(WidgetUI.resetButton, SIGNAL(clicked()),
        this, SLOT(ResetSetup()));

    // reset setup
    ResetSetup();

    // load work panel setup
    LoadWorkPanelSetup();

    // update view - again with possibly new setup
    UpdatePreviewText();
}

//------------------------------------------------------------------------------

CGaussianInputExportTool::~CGaussianInputExportTool(void)
{
    SaveWorkPanelSetup();
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

void CGaussianInputExportTool::LoadWorkPanelSpecificData(CXMLElement* p_ele)
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

    if( p_ele->GetAttribute("gaussianProcs",dummy) ){
        WidgetUI.procSB->setValue(dummy);
    }
    if( p_ele->GetAttribute("gaussianCalcType",dummy) ){
        WidgetUI.calculationCB->setCurrentIndex(dummy);
    }
    if( p_ele->GetAttribute("gaussianTheory",dummy) ){
        WidgetUI.theoryCB->setCurrentIndex(dummy);
    }
    if( p_ele->GetAttribute("gaussianBasis",dummy) ){
        WidgetUI.basisCB->setCurrentIndex(dummy);
    }
    if( p_ele->GetAttribute("gaussianCoord",dummy) ){
        WidgetUI.coordCB->setCurrentIndex(dummy);
    }

    bool ddummy;
    if( p_ele->GetAttribute("gaussianChk",ddummy) ){
        WidgetUI.checkpointCB->setChecked(ddummy);
    }
}

//------------------------------------------------------------------------------

void CGaussianInputExportTool::SaveWorkPanelSpecificData(CXMLElement* p_ele)
{
    if( p_ele == NULL ){
        INVALID_ARGUMENT("p_ele == NULL");
    }
    p_ele->SetAttribute("saveSetup",WidgetUI.saveSetupCB->isChecked());
    p_ele->SetAttribute("closeOnSave",WidgetUI.closeOnSaveCB->isChecked());

    if( WidgetUI.saveSetupCB->isChecked() ){
        p_ele->SetAttribute("gaussianCalcType", WidgetUI.calculationCB->currentIndex());
        p_ele->SetAttribute("gaussianProcs", WidgetUI.procSB->value());
        p_ele->SetAttribute("gaussianTheory", WidgetUI.theoryCB->currentIndex());
        p_ele->SetAttribute("gaussianBasis", WidgetUI.basisCB->currentIndex());
        p_ele->SetAttribute("gaussianChk", WidgetUI.checkpointCB->isChecked());
        p_ele->SetAttribute("gaussianCoord", WidgetUI.coordCB->currentIndex());
    }

    CWorkPanel::SaveWorkPanelSpecificData(p_ele);
}

//------------------------------------------------------------------------------

void CGaussianInputExportTool::ResetSetup(void)
{
    // first unlock user changes
    if( ! WidgetUI.controlsW->isEnabled() ){
        WidgetUI.controlsW->setEnabled(true);
        UpdatePreviewText();
        return;
    }

    // Reset the form to defaults
    WidgetUI.titleLE->setText("Title");
    WidgetUI.calculationCB->setCurrentIndex(0);
    WidgetUI.theoryCB->setCurrentIndex(4);
    WidgetUI.basisCB->setCurrentIndex(0);
    WidgetUI.multiplicitySB->setValue(0);
    WidgetUI.chargeSB->setValue(0);
    WidgetUI.procSB->setValue(1);
    WidgetUI.basisCB->setCurrentIndex(0);
    WidgetUI.checkpointCB->setChecked(true);

    WidgetUI.controlsW->setEnabled(true);

    SetTheory();
    UpdatePreviewText();
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

void CGaussianInputExportTool::UpdatePreviewText(void)
{
    if( Structure == NULL ){
        WidgetUI.previewText->setText(tr("No structure is active in the project!"));
        WidgetUI.controlsW->setEnabled(false);
        return;
    }

    if( WidgetUI.controlsW->isEnabled() ){
        WidgetUI.previewText->setText(GenerateInputDeck());
        WidgetUI.controlsW->setEnabled(true);
    }
}

//------------------------------------------------------------------------------

void CGaussianInputExportTool::SaveInputFile(void)
{
    // parse formats list ------------------------
    QStringList filters;
    filters << "Gaussian input file (*.com)";
    filters << "All files (*)";

    QFileDialog* p_dialog = new QFileDialog();
    p_dialog->setDirectory (QString(GlobalSetup->GetLastOpenFilePath(GaussianInputExportToolID)));
    p_dialog->setNameFilters(filters);
    p_dialog->setFileMode(QFileDialog::AnyFile);
    p_dialog->setAcceptMode(QFileDialog::AcceptSave);
    p_dialog->setDefaultSuffix("com");

    if( p_dialog->exec() == QDialog::Rejected ){
        return;
    }

    QString fileName = p_dialog->selectedFiles().at(0);
    GlobalSetup->SetLastOpenFilePathFromFile(fileName,GaussianInputExportToolID);

    delete p_dialog;

    QFile file(fileName);

    // open file
    if( ! file.open(QIODevice::WriteOnly | QIODevice::Text) ){
        QMessageBox::critical(NULL,"Error","Unable to open file!",QMessageBox::Ok,QMessageBox::Ok);
        return;
    }

    QString previewText = WidgetUI.previewText->toPlainText();
    QString checkpointName = QFileInfo(fileName).baseName();
    checkpointName.prepend("%Chk=");
    checkpointName.append(".chk");
    previewText.replace(QLatin1String("%Chk=checkpoint.chk"), checkpointName, Qt::CaseInsensitive);

    QTextStream out(&file);
    out << previewText;

    // should we close this window?
    if( WidgetUI.closeOnSaveCB->isChecked() ){
        close();
    }
}

//------------------------------------------------------------------------------

void CGaussianInputExportTool::PreviewEdited(void)
{
    // Determine if the preview text has changed from the form generated
    WidgetUI.controlsW->setEnabled(WidgetUI.previewText->toPlainText() == GenerateInputDeck() );
}

//------------------------------------------------------------------------------

QString CGaussianInputExportTool::GetCalculationType(void)
{
    // Translate the enum to text for the output generation
    switch (WidgetUI.calculationCB->currentIndex()) {
        case 0:
            return "SCF=Tight";
        case 1:
            return "Opt";
        case 2:
            return "Opt=(CalcFC,TS,NoEigenTest,MaxCycle=25)";
        case 3:
            return "Freq";
        default:
            return "SP";
    }
}

//------------------------------------------------------------------------------

void CGaussianInputExportTool::SetTheory(void)
{
    if ( IsBasisRequired() ) {
        WidgetUI.basisCB->setEnabled(true);
        WidgetUI.basisCB->show();
    } else {
        WidgetUI.basisCB->setEnabled(false);
        WidgetUI.basisCB->hide();
    }

    UpdatePreviewText();
}

//------------------------------------------------------------------------------

QString CGaussianInputExportTool::GetTheoryType(bool spinprefix)
{
    // Translate the enum to text for the output generation
    QString prefix;

    if( spinprefix ){
        if( WidgetUI.multiplicitySB->value() > 1 ){
            prefix = "U";
        } else {
            prefix = "R";
        }
    }
    switch (WidgetUI.theoryCB->currentIndex()) {
        case 0:
            return(prefix + "AM1");
        case 1:
            return(prefix + "PM3");
        case 2:
            return(prefix + "PM6");
        case 3:
            return(prefix + "HF");
        case 4:
            return(prefix + "B3LYP");
        case 5:
            return(prefix + "PBEPBE");
        case 6:
            return(prefix + "MP2");
        default:
            return(prefix + "HF");
    }
}

//------------------------------------------------------------------------------

bool CGaussianInputExportTool::IsBasisRequired(void)
{
    if( GetTheoryType(false) == "AM1" ) return(false);
    if( GetTheoryType(false) == "PM3" ) return(false);
    if( GetTheoryType(false) == "PM6" ) return(false);
    return(true);
}

//------------------------------------------------------------------------------

QString CGaussianInputExportTool::GetBasisType(void)
{
    // Translate the enum to text for the output generation
    switch (WidgetUI.basisCB->currentIndex()) {
        case 0:
            return "6-31G*";
        case 1:
            return "6-31G**";
        case 2:
            return "6-31+G**";
        default:
            return "6-31G*";
    }
}

//------------------------------------------------------------------------------

void CGaussianInputExportTool::GenerateHeader(QTextStream& str)
{
    // These directives are required before the job specification
    if (WidgetUI.procSB->value() > 1) {
        str << "%NProcShared=" << WidgetUI.procSB->value() << '\n';
    }
    if (WidgetUI.checkpointCB->isChecked() ) {
        str << "%Chk=checkpoint.chk\n";
    }

    // Now specify the job type etc
    str << "# " << GetTheoryType(true);

    // Not all theories have a basis set
    if( IsBasisRequired() ) {
      str << '/' << GetBasisType();
    }

    // Now for the calculation type
    str << ' ' << GetCalculationType();

    if( WidgetUI.multiplicitySB->value() > 1 ){
        str << " Guess=Mix";
    }

    str << " NoSymm";

    // Title line
    if( ! WidgetUI.titleLE->text().isEmpty() ){
        str << "\n\n" << WidgetUI.titleLE->text() << "\n\n";
    } else {
        str << "\n\n" << "Title" << "\n\n";
    }
}

//------------------------------------------------------------------------------

void CGaussianInputExportTool::GenerateCoordinates(QTextStream& str)
{
    if( Structure == NULL ) return;

    // Now for the charge and multiplicity
    str << WidgetUI.chargeSB->value() << ' ' << WidgetUI.multiplicitySB->value() << '\n';

    if( WidgetUI.includeFragCB->isChecked() ){
        InitFragmentIndexes();
    }

    // Now to output the actual molecular coordinates
    // Cartesian coordinates
    switch(WidgetUI.coordCB->currentIndex() ) {
        case 0: {
            foreach(QObject* p_qatom, Structure->GetAtoms()->children()) {
                CAtom* p_atom = static_cast<CAtom*>(p_qatom);
                str << qSetFieldWidth(2) << right
                    << QString(PeriodicTable.GetSymbol(p_atom->GetZ()));
                if( WidgetUI.includeFragCB->isChecked() ){
                    str << "(Fragment=" << qSetFieldWidth(0) << FragIndexes[p_atom] << ")";
                }
                str << qSetFieldWidth(13) << qSetRealNumberPrecision(6) << forcepoint
                    << fixed << right << p_atom->GetPos().x << p_atom->GetPos().y
                    << p_atom->GetPos().z << qSetFieldWidth(0) << '\n';
            }
            GeneratePBCCoordinates(str);
            str << '\n';
        }
        break;
        // -------------
        case 1: {
            // Z-matrix
            OpenBabel::OBAtom *a, *b, *c;
            double r, w, t;

            /* Taken from OpenBabel's gzmat file format converter */
            std::vector<OpenBabel::OBInternalCoord*> vic;
            vic.push_back((OpenBabel::OBInternalCoord*)NULL);

            OpenBabel::OBMol obmol;
            COpenBabelUtils::Nemesis2OpenBabel(Structure,obmol);
            FOR_ATOMS_OF_MOL(atom, &obmol) {
                vic.push_back(new OpenBabel::OBInternalCoord);
            }

            CartesianToInternal(vic, obmol);

            int index = 0;
            foreach(QObject* p_qatom, Structure->GetAtoms()->children()) {
                CAtom* p_atom = static_cast<CAtom*>(p_qatom);
                a = vic[index+1]->_a;
                b = vic[index+1]->_b;
                c = vic[index+1]->_c;

                str << qSetFieldWidth(2) << right
                    << QString(PeriodicTable.GetSymbol(p_atom->GetZ()));
                if( WidgetUI.includeFragCB->isChecked() ){
                    str << "(Fragment=" << qSetFieldWidth(0) << FragIndexes[p_atom] << ")";
                }
                str << qSetFieldWidth(0);
                if(index > 0) {
                    str << ' ' << a->GetIdx() << " B" << index;
                }
                if(index > 1) {
                    str << ' ' << b->GetIdx() << " A" << index;
                }
                if(index > 2) {
                    str << ' ' << c->GetIdx() << " D" << index;
                }
                str << '\n';
                index++;
            }

            GeneratePBCCoordinates(str);

            str << "Variables:" << endl;
            for(int index=0; index < Structure->GetAtoms()->GetNumberOfAtoms(); index++) {
                r = vic[index+1]->_dst;
                w = vic[index+1]->_ang;
                if (w < 0.0) {
                    w += 360.0;
                }
                t = vic[index+1]->_tor;
                if (t < 0.0) {
                    t += 360.0;
                }
                if(index > 0) {
                    str << "B" << index << qSetFieldWidth(13)
                        << qSetRealNumberPrecision(6) << forcepoint << fixed << right
                        << r << qSetFieldWidth(0) << '\n';
                }
                if(index > 1) {
                    str << "A" << index << qSetFieldWidth(13)
                        << qSetRealNumberPrecision(6) << forcepoint << fixed << right
                        << w << qSetFieldWidth(0) << '\n';
                }
                if(index > 2) {
                    str << "D" << index << qSetFieldWidth(13)
                        << qSetRealNumberPrecision(6) << forcepoint << fixed << right
                        << t << qSetFieldWidth(0) << '\n';
                }
            }
            str << '\n';
            foreach(OpenBabel::OBInternalCoord *c, vic) {
                delete c;
            }
        }
        break;
        // -------------
        case 2:{
            OBAtom *a, *b, *c;
            double r, w, t;

            /* Taken from OpenBabel's gzmat file format converter */
            std::vector<OpenBabel::OBInternalCoord*> vic;
            vic.push_back((OpenBabel::OBInternalCoord*)NULL);

            OpenBabel::OBMol obmol;
            COpenBabelUtils::Nemesis2OpenBabel(Structure,obmol);
            FOR_ATOMS_OF_MOL(atom, &obmol) {
                vic.push_back(new OpenBabel::OBInternalCoord);
            }

            CartesianToInternal(vic, obmol);

            FOR_ATOMS_OF_MOL(atom, &obmol) {
                a = vic[atom->GetIdx()]->_a;
                b = vic[atom->GetIdx()]->_b;
                c = vic[atom->GetIdx()]->_c;
                r = vic[atom->GetIdx()]->_dst;
                w = vic[atom->GetIdx()]->_ang;
                if (w < 0.0) {
                  w += 360.0;
                }
                t = vic[atom->GetIdx()]->_tor;
                if (t < 0.0) {
                  t += 360.0;
                }

                str << qSetFieldWidth(3) << right << QString(PeriodicTable.GetSymbol(atom->GetAtomicNum()));
                str << qSetFieldWidth(3) << right;
                if (atom->GetIdx() > 1)
                  str << a->GetIdx() << qSetFieldWidth(13)
                  << qSetRealNumberPrecision(6) << forcepoint << fixed << right << r;
                if (atom->GetIdx() > 2)
                  str << qSetFieldWidth(3) << right << b->GetIdx() << qSetFieldWidth(13)
                  << qSetRealNumberPrecision(6) << forcepoint << fixed << right << w;
                if (atom->GetIdx() > 3)
                  str << qSetFieldWidth(3) << right << c->GetIdx() << qSetFieldWidth(13)
                  << qSetRealNumberPrecision(6) << forcepoint << fixed << right << t;
                str << qSetFieldWidth(0) << '\n';
            }
            GeneratePBCCoordinates(str);
            str << '\n';
            foreach(OpenBabel::OBInternalCoord *c, vic) {
                delete c;
            }
        }
        break;
    }
}

//------------------------------------------------------------------------------

void CGaussianInputExportTool::GeneratePBCCoordinates(QTextStream& str)
{
    if( WidgetUI.includePBCCB->isChecked() == false ) return;
    if( Structure->PBCInfo.IsValid() == false ) return;

    if( Structure->PBCInfo.IsPeriodicAlongA() ){
        str << qSetFieldWidth(3) << left
            << QString("TV")
            << qSetFieldWidth(13) << qSetRealNumberPrecision(6) << forcepoint
            << fixed << right << Structure->PBCInfo.GetAVector().x << Structure->PBCInfo.GetAVector().y
            << Structure->PBCInfo.GetAVector().z << qSetFieldWidth(0) << '\n';
    }
    if( Structure->PBCInfo.IsPeriodicAlongB() ){
        str << qSetFieldWidth(3) << left
            << QString("TV")
            << qSetFieldWidth(13) << qSetRealNumberPrecision(6) << forcepoint
            << fixed << right << Structure->PBCInfo.GetBVector().x << Structure->PBCInfo.GetBVector().y
            << Structure->PBCInfo.GetBVector().z << qSetFieldWidth(0) << '\n';
    }
    if( Structure->PBCInfo.IsPeriodicAlongC() ){
        str << qSetFieldWidth(3) << left
            << QString("TV")
            << qSetFieldWidth(13) << qSetRealNumberPrecision(6) << forcepoint
            << fixed << right << Structure->PBCInfo.GetCVector().x << Structure->PBCInfo.GetCVector().y
            << Structure->PBCInfo.GetCVector().z << qSetFieldWidth(0) << '\n';
    }
}

//------------------------------------------------------------------------------

void CGaussianInputExportTool::InitFragmentIndexes(void)
{
    FragIndexes.clear();
    if( Structure == NULL ) return;

    int index = 1;
    // reset indexes
    foreach(QObject* p_qobj, Structure->GetAtoms()->children()) {
        CAtom* p_atom = static_cast<CAtom*>(p_qobj);
        FragIndexes[p_atom] = index;
        p_atom->SetFlag(EPOF_PROC_FLAG,false);
        index++;
    }

    // traverse over all atoms
    foreach(QObject* p_qobj, Structure->GetAtoms()->children()) {
        CAtom* p_atom = static_cast<CAtom*>(p_qobj);
        // add atom to stack
        std::queue<CAtom*>    stack;
        stack.push(p_atom);

        int mol_id = FragIndexes[p_atom];
        p_atom->SetFlag(EPOF_PROC_FLAG,true);

        // go through the stack over interconnected atoms
        while( stack.size() > 0 ) {
            CAtom*      p_atom = stack.front();
            stack.pop();

            foreach(CBond* p_bond, p_atom->GetBonds()) {
                CAtom* p_atom2 = p_bond->GetOppositeAtom(p_atom);
                if( p_atom2->IsFlagSet(EPOF_PROC_FLAG) ) continue;
                stack.push(p_atom2);
                p_atom2->SetFlag(EPOF_PROC_FLAG,true);
                FragIndexes[p_atom2] = mol_id;
            }
        }
    }

    // compress indexes
    list<int>   indexes;

    foreach(QObject* p_qobj, Structure->GetAtoms()->children()) {
        CAtom* p_atom = static_cast<CAtom*>(p_qobj);
        indexes.push_back(FragIndexes[p_atom]);
    }

    indexes.sort();
    indexes.unique();

    // reindex
    foreach(QObject* p_qobj, Structure->GetAtoms()->children()) {
        CAtom* p_atom = static_cast<CAtom*>(p_qobj);
        int index = FragIndexes[p_atom];
        list<int>::iterator it = indexes.begin();
        list<int>::iterator ie = indexes.end();
        int newindx = 1;
        while( it != ie ){
            if( *it == index ) break;
            it++;
            newindx++;
        }
        FragIndexes[p_atom] = newindx;
    }
}

//------------------------------------------------------------------------------

QString CGaussianInputExportTool::GenerateInputDeck(void)
{
    // Generate an input deck based on the settings of the dialog
    QString buffer;
    QTextStream mol(&buffer);

    GenerateHeader(mol);
    GenerateCoordinates(mol);

    return buffer;
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================


