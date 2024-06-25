// =============================================================================
// NEMESIS - Molecular Modelling Package
// -----------------------------------------------------------------------------
//    Copyright (C) 2008 Petr Kulhanek, kulhanek@enzim.hu,
//                       Jakub Stepan, xstepan3@chemi.muni.cz
//    Copyright (C) 1998-2004 Petr Kulhanek, kulhanek@chemi.muni.cz
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

#include <NemesisCoreModule.hpp>
#include <ProjectList.hpp>
#include <Project.hpp>
#include <PluginDatabase.hpp>
#include <ErrorSystem.hpp>
#include <GlobalSetup.hpp>
#include <XMLDocument.hpp>
#include <XMLParser.hpp>
#include <XMLElement.hpp>
#include <CategoryUUID.hpp>
#include <DesktopSystem.hpp>
#include <NemesisOptions.hpp>
#include <ProjectDesktop.hpp>
#include <MainWindow.hpp>
#include <RecentFileList.hpp>

#include <QMessageBox>
#include <QFileDialog>
#include <QApplication>
#include <QStyle>
#include <QStyleOptionTitleBar>

//------------------------------------------------------------------------------

CProjectList* Projects = NULL;

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

CExtUUID        ProjectListID(
                    "{PROJECT_LIST:dad82634-8c81-4bf5-876f-e918c3537911}",
                    "Projects");

CPluginObject   ProjectListObject(&NemesisCorePlugin,
                    ProjectListID,GENERIC_CAT,
                    ":/images/NemesisCore/project/ProjectList.svg",
                    NULL);

//------------------------------------------------------------------------------

/// this is used to store last file path for all nemesis projects
CExtUUID        GenericProjectID(
                    "{GENERIC_PROJECT:632459eb-0c98-4390-8f85-67766b0736be}",
                    "Nemesis generic project");

CPluginObject   GenericProject(&NemesisCorePlugin,
                    GenericProjectID,GENERIC_CAT,
                    ":/images/NemesisCore/project/Project.svg",
                    NULL);

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

CProjectList::CProjectList(QObject* p_parent)
    : CExtComObject(&ProjectListObject,p_parent)
{
    ConFlags |= EECOF_SUB_CONTAINERS;
    ConFlags |= EECOF_DYNAMICAL_CONTENTS;
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

CProject* CProjectList::NewProject(const CUUID& projectuuid)
{
    QString pro_name = GenerateName(tr("Project %1"));
    return(NewProject(projectuuid,pro_name));
}

//---------------------------------------------------------------------------

CProject* CProjectList::NewProject(const CUUID& projectuuid,const QString& pro_name)
{
    CProject* p_project = static_cast<CProject*>(PluginDatabase.CreateObject(projectuuid,this));
    if( p_project == NULL ) {
        ES_ERROR("unable to create object");
        return(NULL);
    }

    p_project->SetName(pro_name);
    p_project->NewData();

    emit OnChildContainerAdded(this,p_project);
    emit OnProjectAdded(p_project);

    // load default project desktop setup
    p_project->GetDesktop()->LoadDefaultDesktop();

    return(p_project);
}

//---------------------------------------------------------------------------

CProject* CProjectList::OpenProject(const CFileName& fullname)
{
    if( fullname == NULL ) {
        ES_ERROR("fullname is NULL");
        return(NULL);
    }

    // load project XML file
    CXMLDocument xml_document;
    CXMLParser   xml_parser;

    xml_parser.SetOutputXMLNode(&xml_document);

    // parse XML document
    if( xml_parser.Parse(fullname) == false ) {
        ES_ERROR("unable to parse XML document");
        return(NULL);
    }

    // open document comment
    CXMLElement* p_root = xml_document.GetFirstChildElement("project");
    if( p_root == NULL ) {
        ES_ERROR("unable to open root element");
        return(NULL);
    }

    // open header data
    CXMLElement* p_header = p_root->GetFirstChildElement("header");
    if( p_header == NULL ) {
        ES_ERROR("unable to open header element");
        return(NULL);
    }

    QString suuid;

    bool result = true;
    result &= p_header->GetAttribute("uuid",suuid);

    if( result == false ) {
        ES_ERROR("unable to get header element data");
        return(NULL);
    }

    // data
    CXMLElement* p_data = p_root->GetFirstChildElement("data");
    if( p_data == NULL ) {
        ES_ERROR("unable to open data element");
        return(NULL);
    }

    // create new project
    CExtUUID pro_uuid;

    if( pro_uuid.LoadFromString(suuid) == false ) {
        ES_ERROR("unable to decode uuid of project");
        return(NULL);
    }

    // create project
    CProject* p_project = static_cast<CProject*>(PluginDatabase.CreateObject(pro_uuid,this));
    if( p_project == NULL ) {
        ES_ERROR("unable to create object");
        return(NULL);
    }

    p_project->ProjectPath = fullname.GetFileDirectory();
    p_project->SetName(QString(fullname.GetFileNameWithoutExt()));

    // load default project desktop setup
    // it will be partially overwritten by p_project->LoadData(p_data);
    p_project->GetDesktop()->LoadDefaultDesktop();

    // load data
    try {
        p_project->BeginLinkProcedure(0);
        p_project->LoadData(p_data);
        p_project->EndLinkProcedure();
    } catch(std::exception& e) {
        ES_ERROR_FROM_EXCEPTION("unable to load data",e);
        delete p_project;
        return(NULL);
    }

    p_project->SetFlag(EPOF_TMP_NAME,false);
    p_project->SetFlag(EPOF_PROJECT_CHANGED,false);

    RecentFiles->PushProjectFile(fullname);

    emit OnChildContainerAdded(this,p_project);
    emit OnProjectAdded(p_project);

    return(p_project);
}

//---------------------------------------------------------------------------

bool CProjectList::CanCloseProject(CProject* p_proj)
{
    if( p_proj == NULL ) return(false);

    // is project changed?
    if( p_proj->IsFlagSet(EPOF_PROJECT_CHANGED) == false ) return(true);

    QString message(tr("Project %1 has been modified.\nDo you want to save your changes?"));
    message = message.arg(p_proj->GetName());

    int ret = QMessageBox::warning(NULL, tr("Close Project"),
                                   message,
                                   QMessageBox::Save | QMessageBox::Discard
                                   | QMessageBox::Cancel,
                                   QMessageBox::Save);

    switch(ret) {
    case QMessageBox::Cancel:
        return(false);

    case QMessageBox::Save:
        return( ProjectSave(p_proj) );

    case QMessageBox::Discard:
    default:
        return(true);
    }
}

//------------------------------------------------------------------------------

bool CProjectList::ProjectSave(CProject* p_proj)
{
    if( p_proj == NULL ) return(false);

    bool result = true;

    if(  p_proj->IsFlagSet(EPOF_TMP_NAME) == true ) {
        if( ProjectSaveAs(p_proj) == false ) return(false);
    } else {
        result = p_proj->SaveProject();
    }

    if( result == true ) return(true);

    // inform about error
    QMessageBox::critical(NULL, tr("Save Project"),
                          tr("An error occurred during project saving!"),
                          QMessageBox::Ok,
                          QMessageBox::Ok);

    return(false);
}

//------------------------------------------------------------------------------

bool CProjectList::ProjectSaveAs(CProject* p_proj)
{
    if( p_proj == NULL ) return(false);

    QString title("Save Project - ");
    //TODO: title += p_proj->GetName().GetBuffer();

    QString filename = QFileDialog::getSaveFileName(NULL,
                       title,
                       (const char*)GlobalSetup->GetLastOpenFilePath(GenericProjectID),
                       "Nemesis projects (*.npr)");

    if( filename == NULL ) return(false); // no file was selected

    CFileName project_name(filename.toLatin1().constData());

    // add extension if there is not ".npr"
    if(project_name.GetFileNameExt() != ".npr") {
        project_name += ".npr";
    }

    // update last open path
    GlobalSetup->SetLastOpenFilePath(project_name.GetFileDirectory(),GenericProjectID);

    // load project
    bool result = p_proj->SaveProjectAs(project_name);
    if( result == true ) return(true);

    ES_ERROR("unable to save project");
    QMessageBox::critical(NULL, tr("Save Project As"),
                          tr("An error occurred during project saving!"),
                          QMessageBox::Ok,
                          QMessageBox::Ok);

    return(false);
}

//------------------------------------------------------------------------------

int CProjectList::GetNumberOfProjects(void)
{
    return(children().count());
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

void CProjectList::RepaintAllProjects(void)
{
    foreach(QObject* p_qobj,children()) {
        CProject* p_pro = static_cast<CProject*>(p_qobj);
        p_pro->RepaintProject();
    }
}

//---------------------------------------------------------------------------

void CProjectList::RemoveAllProjects(void)
{
    foreach(QObject* p_qobj,children()) {
        CProject* p_pro = static_cast<CProject*>(p_qobj);
        delete p_pro;
    }
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

void CProjectList::EmitProjectRemoved(CProject* p_project)
{
    emit OnProjectRemoved(p_project);
}

//------------------------------------------------------------------------------

void CProjectList::EmitProjectNameChanged(CProject* p_project)
{
    emit OnProjectNameChanged(p_project);
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================



