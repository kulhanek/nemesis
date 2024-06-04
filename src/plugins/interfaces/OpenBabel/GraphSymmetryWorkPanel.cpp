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

#include "OpenBabelModule.hpp"

#include "GraphSymmetryWorkPanel.hpp"

#include "openbabel/mol.h"
#include "openbabel/graphsym.h"

#include <boost/algorithm/string/join.hpp>

using namespace std;
using namespace OpenBabel;

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

QObject* GraphSymmetryWorkPanelCB(void* p_data);

CExtUUID        GraphSymmetryWorkPanelID(
                    "{GRAPH_SYMMETRY_WORK_PANEL:183ec892-7639-414a-b17c-d313caaf11b7}",
                    "Graph Symmetry Classes");

CPluginObject   GraphSymmetryWorkPanelObject(&OpenBabelPlugin,
                    GraphSymmetryWorkPanelID,WORK_PANEL_CAT,
                    ":/images/OpenBabel/GraphSymmetryWorkPanel.svg",
                    GraphSymmetryWorkPanelCB);

// -----------------------------------------------------------------------------

QObject* GraphSymmetryWorkPanelCB(void* p_data)
{
    CProject* p_project = static_cast<CProject*>(p_data);
    if( p_project == NULL ){
        ES_ERROR("CGraphSymmetryWorkPanel requires active project");
        return(NULL);
    }

    QObject* p_build_wp = new CGraphSymmetryWorkPanel(p_project);
    return(p_build_wp);
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

CGraphSymmetryWorkPanel::CGraphSymmetryWorkPanel(CProject* p_project)
    : CWorkPanel(&GraphSymmetryWorkPanelObject,p_project,EWPR_TOOL)
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
        this,SLOT(GenerateGraphSymmetry(void)));
    // -------------
    connect(WidgetUI.imprintPB,SIGNAL(clicked(bool)),
        this,SLOT(ImprintGroups(void)));
    // -------------
    connect(WidgetUI.copyPB,SIGNAL(clicked(bool)),
        this,SLOT(CopyToClipboard(void)));
    // -------------
    connect(WidgetUI.highlightSelectedCB,SIGNAL(toggled(bool)),
        this,SLOT(HighlightingToggled(bool)));
    // -------------
    connect(WidgetUI.structureW,SIGNAL(OnObjectChanged(void)),
        this,SLOT(ClearAllVertices(void)));
    // -------------
    connect(WidgetUI.verticesTV->selectionModel(), SIGNAL(selectionChanged(const QItemSelection&, const QItemSelection&)),
            this, SLOT(VerticesSelectionChanged(void)));
    // -------------
    connect(p_project->GetHistory(),SIGNAL(OnHistoryChanged(EHistoryChangeMessage)),
        this,SLOT(ClearAllVertices(void)));

    // load work panel setup
    LoadWorkPanelSetup();

    // initialize data
    ClearAllVertices();

    DoNotReset = false;
}

//------------------------------------------------------------------------------

CGraphSymmetryWorkPanel::~CGraphSymmetryWorkPanel()
{
    SaveWorkPanelSetup();
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

void CGraphSymmetryWorkPanel::GenerateGraphSymmetry(void)
{
    // clear first everything
    ClearAllVertices();

    // do we have structure
    if( WidgetUI.structureW->getObject() == NULL ) return;

    CStructure* p_str = WidgetUI.structureW->getObject<CStructure*>();

    // create openbabel molecule
    OBMol       mol;
    COpenBabelUtils::Nemesis2OpenBabel(p_str,mol);

    // generate symmetry classes
    OBGraphSym                  graph_sym(&mol);
    graph_sym.GetSymmetry(SymmetryClasses);

    // make list of unique vertices
    std::set<unsigned int>  unique_labels;
    for(size_t i = 0; i < SymmetryClasses.size(); i++ ){
        unique_labels.insert(SymmetryClasses[i]);
    }
    WidgetUI.numOfVerticesSB->setValue(unique_labels.size());

    std::set<unsigned int>::iterator    uit = unique_labels.begin();
    std::set<unsigned int>::iterator    uie = unique_labels.end();

    size_t i=0;
    while( uit != uie ){
        unsigned int cl = *uit;

        // label
        QString label;
        label.asprintf("gr%d",(int)i+1);

        // atoms, residues, types, charges
        std::set<std::string> names;
        std::set<std::string> resid;
        std::set<std::string> types;
        double chrsum   = 0;
        double chrsum2  = 0;
        int    count = 0;
        for( size_t j=0; j < SymmetryClasses.size(); j++){
            if( SymmetryClasses[j] == cl ){
                CAtom* p_atom = static_cast<CAtom*>(p_str->GetAtoms()->children().at(j));
                names.insert(p_atom->GetName().toStdString());
                QString rid;
                rid.asprintf("%d",p_atom->GetResidue()->GetSeqIndex());
                resid.insert(rid.toStdString());
                types.insert(p_atom->GetType().toStdString());
                chrsum  += p_atom->GetCharge();
                chrsum2 += p_atom->GetCharge() * p_atom->GetCharge();
                count++;
            }
        }
        double avchr = 0;
        double chrms = 0;
        if( count > 0 ){
            avchr = chrsum / count;
            double ms = count*chrsum2 - chrsum*chrsum;
            if( ms > 0 ){
                chrms = sqrt(ms) / count;
            }
        }

        QList<QStandardItem*> items;
        QStandardItem* p_item;

        p_item = new QStandardItem(label);
        p_item->setData(cl);
        items << p_item;

        p_item = new QStandardItem(QString::fromStdString(boost::algorithm::join(names,",")));
        items << p_item;

        p_item = new QStandardItem(QString::fromStdString(boost::algorithm::join(resid,",")));
        items << p_item;

        p_item = new QStandardItem(QString::fromStdString(boost::algorithm::join(types,",")));
        items << p_item;

        p_item = new QStandardItem(PQ_CHARGE->GetRealValueText(avchr));
        p_item->setData(Qt::AlignRight,Qt::TextAlignmentRole);
        items << p_item;

        p_item = new QStandardItem(PQ_CHARGE->GetRealValueText(chrms));
        p_item->setData(Qt::AlignRight,Qt::TextAlignmentRole);
        items << p_item;

        VerticesModel->appendRow(items);

        uit++;
        i++;
    }
}

//------------------------------------------------------------------------------

void CGraphSymmetryWorkPanel::ClearAllVertices(void)
{
    if( DoNotReset ){
        DoNotReset = false;
        return;
    }

    // clear counters
    WidgetUI.numOfVerticesSB->setValue(0);

    // clear models
    VerticesModel->clear();

    // restore headers
    QStringList header;

    header << tr("Group") << tr("Atoms") << tr("Residues") << tr("Types") << tr("<Charge>") << tr("s(Charge)");
    VerticesModel->setHorizontalHeaderLabels(header);

    SymmetryClasses.clear();

    // cancel highlighting
    HighlightNone(true);
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

void CGraphSymmetryWorkPanel::HighlightingToggled(bool set)
{
    if( set == false ){
        HighlightNone(true);
    } else {
        VerticesSelectionChanged();
    }
}

//------------------------------------------------------------------------------

void CGraphSymmetryWorkPanel::HighlightNone(bool repaint)
{
    // do we have structure
    if( WidgetUI.structureW->getObject() == NULL ) return;

    // set EPOF_SELECTED to false for every atom
    CStructure* p_str = WidgetUI.structureW->getObject<CStructure*>();
    foreach(QObject* p_qobj, p_str->GetAtoms()->children() ){
        CAtom* p_atom = static_cast<CAtom*>(p_qobj);
        p_atom->SetFlag(EPOF_SELECTED,false);
    }

    // repaint project
    if( repaint ) GetProject()->RepaintProject();
}

//------------------------------------------------------------------------------

void CGraphSymmetryWorkPanel::VerticesSelectionChanged(void)
{
    // reset highlighting
    HighlightNone();

    CStructure* p_str = WidgetUI.structureW->getObject<CStructure*>();

    // do we have structure
    if( p_str == NULL ) return;

    // shell we select bond?
    if( WidgetUI.highlightSelectedCB->isChecked() == false ) return;

    // select atoms
    QModelIndexList selected_rows = WidgetUI.verticesTV->selectionModel()->selectedRows(0);
    for(int i=0; i < selected_rows.count(); i++){
        QStandardItem* p_it = VerticesModel->itemFromIndex(selected_rows.at(i));
        unsigned int cl = p_it->data().toInt();
        for( size_t j=0; j < SymmetryClasses.size(); j++){
            if( SymmetryClasses[j] == cl ){
                CAtom* p_atom = static_cast<CAtom*>(p_str->GetAtoms()->children().at(j));
                p_atom->SetFlag(EPOF_SELECTED,true);
            }
        }
    }
    // repaint project
    GetProject()->RepaintProject();
}

//------------------------------------------------------------------------------

void CGraphSymmetryWorkPanel::ImprintGroups(void)
{
    CStructure* p_str = WidgetUI.structureW->getObject<CStructure*>();

    // do we have structure and correct number of atoms?
    if( (p_str == NULL) && (p_str->GetAtoms()->GetNumberOfAtoms() != (int)SymmetryClasses.size()) ) return;

    DoNotReset = true;

    CHistoryNode* p_history = p_str->BeginChangeWH(EHCL_DESCRIPTION,"imprint symmetry labels");
    if( p_history == NULL ) return;

    int j = 0;
    foreach(QObject* p_obj,p_str->GetAtoms()->children()){
        CAtom* p_atom = static_cast<CAtom*>(p_obj);
        unsigned cl = SymmetryClasses[j];
        QString descr;
        descr.asprintf("gr%d",cl);
        p_atom->SetDescription(descr,p_history);
        j++;
    }

    p_str->EndChangeWH();
}

//------------------------------------------------------------------------------

void CGraphSymmetryWorkPanel::CopyToClipboard(void)
{
    QString selected_text;

    selected_text += tr("Group");
    selected_text += "\t";
    selected_text += tr("Atoms");
    selected_text += "\t";
    selected_text += tr("Residues");
    selected_text += "\t";
    selected_text += tr("Types");
    selected_text += "\t";
    selected_text += tr("<Charge>");
    selected_text += "\t";
    selected_text += tr("s(Charge)");
    selected_text += "\n";

    for(int i=0; i < VerticesModel->rowCount(); i++){
        for(int j=0; j < VerticesModel->columnCount(); j++){
            selected_text += VerticesModel->item(i,j)->text();
            selected_text += "\t";
        }
            selected_text += "\n";
    }

    QApplication::clipboard()->setText(selected_text);
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================



