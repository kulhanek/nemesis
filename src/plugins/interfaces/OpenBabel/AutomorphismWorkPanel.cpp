// =============================================================================
// NEMESIS - Molecular Modelling Package
// -----------------------------------------------------------------------------
//    Copyright (C) 2014 Petr Kulhanek, kulhanek@chemi.muni.cz
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
#include <Residue.hpp>
#include <StructureSelection.hpp>
#include <HistoryList.hpp>
#include <OpenBabelUtils.hpp>
#include <QString>
#include <PhysicalQuantities.hpp>
#include <PhysicalQuantity.hpp>
#include <QFileDialog>
#include <QMessageBox>
#include <MainWindow.hpp>

#include "OpenBabelModule.hpp"

#include "AutomorphismWorkPanel.hpp"

#include "openbabel/mol.h"
#include "openbabel/graphsym.h"

#include <boost/algorithm/string/join.hpp>

using namespace std;
using namespace OpenBabel;

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

QObject* AutomorphismWorkPanelCB(void* p_data);

CExtUUID        AutomorphismWorkPanelID(
                    "{AUTOMORPHISM_WORK_PANEL:74f9957e-2623-4170-a2de-33010604ac55}",
                    "Automorphism");

CPluginObject   AutomorphismWorkPanelObject(&OpenBabelPlugin,
                    AutomorphismWorkPanelID,WORK_PANEL_CAT,
                    ":/images/OpenBabel/AutomorphismWorkPanel.svg",
                    AutomorphismWorkPanelCB);

// -----------------------------------------------------------------------------

QObject* AutomorphismWorkPanelCB(void* p_data)
{
    CProject* p_project = static_cast<CProject*>(p_data);
    if( p_project == NULL ){
        ES_ERROR("CAutomorphismWorkPanel requires active project");
        return(NULL);
    }

    QObject* p_build_wp = new CAutomorphismWorkPanel(p_project);
    return(p_build_wp);
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

CAutomorphismWorkPanel::CAutomorphismWorkPanel(CProject* p_project)
    : CWorkPanel(&AutomorphismWorkPanelObject,p_project,EWPR_TOOL)
{
    WidgetUI.setupUi(this);

    // create and assign models
    VerticesModel = new QStandardItemModel(this);
    WidgetUI.verticesTV->setModel(VerticesModel);

    // set initial structure
    WidgetUI.structureW->setProject(p_project);
    WidgetUI.structureW->setObjectBaseMIMEType("structure.indexes");
    WidgetUI.structureW->setSelectionHandler(&SH_Structure);
    WidgetUI.structureW->setObject(p_project->GetStructures()->GetActiveStructure());

    // signals
    connect(WidgetUI.generatePB,SIGNAL(clicked(bool)),
        this,SLOT(GenerateAutomorphism(void)));
    // -------------
    connect(WidgetUI.savePatternsPB,SIGNAL(clicked(bool)),
        this,SLOT(SaveMapPatterns(void)));
    // -------------
    connect(WidgetUI.structureW,SIGNAL(OnObjectChanged(void)),
        this,SLOT(ClearAllVertices(void)));
    // -------------
    connect(p_project->GetHistory(),SIGNAL(OnHistoryChanged(EHistoryChangeMessage)),
        this,SLOT(ClearAllVertices(void)));

    // load work panel setup
    LoadWorkPanelSetup();

    // initialize data
    ClearAllVertices();
}

//------------------------------------------------------------------------------

CAutomorphismWorkPanel::~CAutomorphismWorkPanel()
{
    SaveWorkPanelSetup();
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

void CAutomorphismWorkPanel::GenerateAutomorphism(void)
{
    // clear first everything
    ClearAllVertices();

    // do we have structure
    if( WidgetUI.structureW->getObject() == NULL ) return;

    CStructure* p_str = WidgetUI.structureW->getObject<CStructure*>();

    // create openbabel molecule
    OBMol       mol;
    COpenBabelUtils::Nemesis2OpenBabel(p_str,mol);

    // shall we use only selected atoms?
    OBBitVec    mask;
    if( WidgetUI.onlySelectedCB->isChecked() ){
        int at_lid = 1;

        foreach(QObject* p_qobj,p_str->GetAtoms()->children()) {
            CAtom*  p_atom = static_cast<CAtom*>(p_qobj);
            if( p_atom->IsFlagSet(EPOF_SELECTED) ){
                if( WidgetUI.excludeHydrogensCB->isChecked() ){
                    if( p_atom->IsVirtual() ){
                        mask.SetBitOff(at_lid);
                    } else {
                        mask.SetBitOn(at_lid);
                    }
                } else {
                    mask.SetBitOn(at_lid);
                }
            } else {
                mask.SetBitOff(at_lid);
            }
            at_lid++;
        }
    } else if( WidgetUI.excludeHydrogensCB->isChecked() ){
        int at_lid = 1;

        foreach(QObject* p_qobj,p_str->GetAtoms()->children()) {
            CAtom*  p_atom = static_cast<CAtom*>(p_qobj);
            if( p_atom->IsVirtual() ){
                mask.SetBitOff(at_lid);
            } else {
                mask.SetBitOn(at_lid);
            }
            at_lid++;
        }
    }

    // generate symmetry classes
    std::vector<unsigned int>   sym_classes;
    OBGraphSym                  graph_sym(&mol);
    graph_sym.GetSymmetry(sym_classes);

    // find automorphisms
    std::vector<OBIsomorphismMapper::Mapping> aut;
    FindAutomorphisms(&mol,aut,sym_classes,mask);

    // make list of unique vertices
    WidgetUI.numOfMapsSB->setValue(aut.size());

    std::vector<OBIsomorphismMapper::Mapping>::iterator     uit = aut.begin();
    std::vector<OBIsomorphismMapper::Mapping>::iterator     uie = aut.end();

    int i=1;
    while( uit != uie ){
        QList<QStandardItem*> items;
        QStandardItem* p_item;

        // label
        QString label;
        label.asprintf("%d",i);
        p_item = new QStandardItem(label);
        items << p_item;

        // map
        OBIsomorphismMapper::Mapping::iterator  mit = (*uit).begin();
        OBIsomorphismMapper::Mapping::iterator  eit = (*uit).end();
        QString map;
        int j = 0;
        while( mit != eit ){
            if( j > 0 ) map += ",";
            map += QString().asprintf("%d:%d",mit->first+1,mit->second+1);
            mit++;
            j++;
        }
        p_item = new QStandardItem(map);
        items << p_item;

        VerticesModel->appendRow(items);

        uit++;
        i++;
    }
}

//------------------------------------------------------------------------------

void CAutomorphismWorkPanel::ClearAllVertices(void)
{
    // clear counters
    WidgetUI.numOfMapsSB->setValue(0);

    // clear models
    VerticesModel->clear();

    // restore headers
    QStringList header;

    header << tr("#") << tr("Map");
    VerticesModel->setHorizontalHeaderLabels(header);
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

void CAutomorphismWorkPanel::SaveMapPatterns(void)
{
    QStringList filters;
    // Send format to dialog
    filters << "CSV File (*.txt *.csv)";

    // --------------------------------
    QFileDialog* p_dialog = new QFileDialog(GetProject()->GetMainWindow());
    p_dialog->setNameFilters(filters);
    p_dialog->setFileMode(QFileDialog::AnyFile);
    p_dialog->setAcceptMode(QFileDialog::AcceptSave);

    if( p_dialog->exec() != QDialog::Accepted ){
        delete p_dialog;
        return;
    }
    QString filename = p_dialog->selectedFiles().at(0);
    delete p_dialog;

    QFile file(filename);
    if( ! file.open(QFile::WriteOnly) ){
        QMessageBox::critical(GetProject()->GetMainWindow(),tr("Error"),
                              tr("Unable to open the file for writing!"),QMessageBox::Ok,QMessageBox::Ok);
        return;
    }

    QTextStream stream(&file);

    // save model
    for(int i=0; i < VerticesModel->rowCount(); i++){
        QStandardItem* p_item = VerticesModel->item(i,1);
        if( p_item != NULL ){
            stream << p_item->text() << Qt::endl;
        }

    }

}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================



