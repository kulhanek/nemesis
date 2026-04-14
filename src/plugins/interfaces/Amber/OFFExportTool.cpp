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
#include <QMessageBox>
#include <MainWindow.hpp>

#include "AmberModule.hpp"

#include "OFFExportTool.hpp"
#include "OFFExportJob.hpp"

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

QObject* OFFExportToolCB(void* p_data);

CExtUUID        OFFExportToolID(
                    "{OFF_EXPORT_TOOL:5a1a2cf4-5ce5-43c3-930a-1a1cfc5a0e70}",
                    "Amber Object File Format (*.off)");

CPluginObject   OFFExportToolObject(&AmberPlugin,
                    OFFExportToolID,EXPORT_STRUCTURE_CAT,
                    OFFExportToolCB);

// -----------------------------------------------------------------------------

QObject* OFFExportToolCB(void* p_data)
{
    CProject* p_project = static_cast<CProject*>(p_data);
    if( p_project == NULL ){
        ES_ERROR("COFFExportTool requires active project");
        return(NULL);
    }

    COFFExportTool* p_object = new COFFExportTool(p_project);
    p_object->ExecuteDialog();
    delete p_object;

    return(NULL);
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

COFFExportTool::COFFExportTool(CProject* p_project)
    : CProObject(&OFFExportToolObject,NULL,p_project,true)
{
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

void COFFExportTool::ExecuteDialog(void)
{
    // --------------------------------
    QFileDialog* p_dialog = new QFileDialog(GetProject()->GetMainWindow());

    p_dialog->setDirectory(QString(GlobalSetup->GetLastOpenFilePath(OFFExportToolID)));

    QStringList filters;
    filters << OFFExportToolID.GetName();
    p_dialog->setNameFilters(filters);

    p_dialog->setFileMode(QFileDialog::AnyFile);
    p_dialog->setAcceptMode(QFileDialog::AcceptSave);
    p_dialog->setDefaultSuffix("rst7");

    if( p_dialog->exec() == QDialog::Accepted ){
        QString file = p_dialog->selectedFiles().at(0);
        QFileInfo finfo(file);
        if( finfo.suffix().isEmpty() ){
            file += ".rst7";
        }
        LaunchJob(file);
    }

    delete p_dialog;
}

//------------------------------------------------------------------------------

void COFFExportTool::LaunchJob(const QString& file)
{
    GlobalSetup->SetLastOpenFilePathFromFile(file,OFFExportToolID);

    // get active structure to export
    CStructure* p_str = GetProject()->GetActiveStructure();
    if( p_str == NULL ) {
        QMessageBox::critical(GetProject()->GetMainWindow(),tr("Error"),
                              tr("There is no molecule in the active project!"),
                              QMessageBox::Ok, QMessageBox::Ok);
        ES_ERROR("No molecule in project");
        return;
    }

    // create job
    COFFExportJob* p_job = new COFFExportJob(p_str,file);
    if( p_job->SubmitJob() == false ){
        delete p_job;
    }
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================
