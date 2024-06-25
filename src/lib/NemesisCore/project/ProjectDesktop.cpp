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

#include <ProjectDesktop.hpp>
#include <XMLIterator.hpp>
#include <XMLParser.hpp>
#include <XMLPrinter.hpp>
#include <XMLElement.hpp>
#include <ErrorSystem.hpp>
#include <GlobalSetup.hpp>
#include <PluginDatabase.hpp>
#include <QApplication>
#include <QMessageBox>
#include <WorkPanel.hpp>
#include <WorkPanelList.hpp>
#include <Project.hpp>
#include <ProjectList.hpp>
#include <NemesisCoreModule.hpp>
#include <CategoryUUID.hpp>
#include <FileSystem.hpp>

//------------------------------------------------------------------------------

CExtUUID        ProjectDesktopID(
                    "{PROJECT_DESKTOP:a6bb1e46-eaa6-4ed7-aebf-5af80123b526}",
                    "Project Desktop");

CPluginObject   ProjectDesktopObject(&NemesisCorePlugin,
                    ProjectDesktopID,GENERIC_CAT,
                    NULL);

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

CProjectDesktop::CProjectDesktop(CProject* p_parent)
    : CDesktopSystem(&ProjectDesktopObject,p_parent)
{
    ConFlags |= EECOF_HIDDEN;
    Project = p_parent;
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

void CProjectDesktop::LoadDefaultDesktop(void)
{
    // load project default setup
    LoadDesktop("default.dsk");

    // create main element for main window first (only apply if no setup was loaded)
    GetMainWindowSetup();

    // load project main window system setup
    LoadMainWindowSetup();
}

//------------------------------------------------------------------------------

void CProjectDesktop::SaveDefaultDesktop(void)
{
    SaveDesktop("default.dsk");
}

//------------------------------------------------------------------------------

void CProjectDesktop::LoadDesktop(const QString& name)
{
    CFileName lname;
    CFileName sname(name);

    lname = GlobalSetup->GetUserDesktopPath(Project,sname);
    if( ! CFileSystem::IsFile(lname) ){
        lname = GlobalSetup->GetSystemDesktopPath(Project,sname);
    }

    CDesktopSystem::LoadDesktop(lname);
}

//------------------------------------------------------------------------------

void CProjectDesktop::SaveDesktop(const QString& name)
{
    CFileName lname;
    lname = GlobalSetup->GetUserDesktopPath(Project,CFileName(name));
    CDesktopSystem::SaveDesktop(lname);
}

//------------------------------------------------------------------------------

void CProjectDesktop::LoadMainWindowSetup(void)
{
    MainWindowExtraDefault.RemoveAllChildNodes();
    MainWindowExtra.RemoveAllChildNodes();

    // system setup
    CFileName lname;
    lname = GlobalSetup->GetSystemDesktopPath(Project,"mainwindow.xml");

    CXMLParser xml_parser;

    if( CFileSystem::IsFile(lname) ){
        xml_parser.SetOutputXMLNode(&MainWindowExtraDefault);

        if( xml_parser.Parse(lname) == false ) {
            ES_ERROR("unable to parse system desktop mainwindow setup");

            MainWindowExtraDefault.CreateChildDeclaration();
            MainWindowExtraDefault.CreateChildComment("NEMESIS mainwindow setup");
        }
    } else {
        MainWindowExtraDefault.CreateChildDeclaration();
        MainWindowExtraDefault.CreateChildComment("NEMESIS mainwindow setup");
    }

    // user setup
    lname = GlobalSetup->GetUserDesktopPath(Project,"mainwindow.xml");
    if( ! CFileSystem::IsFile(lname) ){
        lname = GlobalSetup->GetSystemDesktopPath(Project,"mainwindow.xml");
    }

    if( CFileSystem::IsFile(lname) ){

        xml_parser.SetOutputXMLNode(&MainWindowExtra);

        if( xml_parser.Parse(lname) == false ) {
            ES_ERROR("unable to parse user desktop mainwindow setup");

            MainWindowExtra.CreateChildDeclaration();
            MainWindowExtra.CreateChildComment("NEMESIS mainwindow setup");
        }
    } else {
        MainWindowExtra.CreateChildDeclaration();
        MainWindowExtra.CreateChildComment("NEMESIS mainwindow setup");
    }
}

//------------------------------------------------------------------------------

void CProjectDesktop::SaveMainWindowSetup(void)
{
    CFileName lname;
    lname = GlobalSetup->GetUserDesktopPath(Project,"mainwindow.xml");

    CXMLPrinter xml_printer;
    xml_printer.SetPrintedXMLNode(&MainWindowExtra);

    if( xml_printer.Print(lname) == false ) {
        ES_ERROR("unable to save user desktop mainwindow setup");
        return;
    }
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

CXMLElement* CProjectDesktop::GetMainWindowSetup(void)
{
    CXMLElement* p_root = DesktopData.GetChildElementByPath("desktop/mainwindow",true);
    return(p_root);
}

//------------------------------------------------------------------------------

CXMLElement* CProjectDesktop::GetMainWindowShortcuts(bool system)
{
    CXMLElement* p_root;
    if( system ){
        p_root = MainWindowExtraDefault.GetChildElementByPath("desktop/mainwindow/shortcuts",true);
    } else {
        p_root = MainWindowExtra.GetChildElementByPath("desktop/mainwindow/shortcuts",true);
    }
    return(p_root);
}

//------------------------------------------------------------------------------

CXMLElement* CProjectDesktop::GetMainWindowToolbars(bool system)
{
    CXMLElement* p_root;
    if( system ){
        p_root = MainWindowExtraDefault.GetChildElementByPath("desktop/mainwindow/toolbars",true);
    } else {
        p_root = MainWindowExtra.GetChildElementByPath("desktop/mainwindow/toolbars",true);
    }
    return(p_root);
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

