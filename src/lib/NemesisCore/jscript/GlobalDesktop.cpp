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

#include <GlobalDesktop.hpp>
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

CExtUUID        GlobalDesktopID(
                    "{GLOBAL_DESKTOP:4ebd4e83-c6d5-4986-a5ea-376a3f9e8c80}",
                    "Global desktop");

CPluginObject   GlobalDesktopObject(&NemesisCorePlugin,
                    GlobalDesktopID,GENERIC_CAT,
                    ":/images/NemesisCore/jscript/GlobalDesktop.svg",
                    NULL);

//------------------------------------------------------------------------------

CGlobalDesktop* GlobalDesktop = NULL;

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

CGlobalDesktop::CGlobalDesktop(QObject* p_parent)
    : CDesktopSystem(&GlobalDesktopObject,p_parent)
{
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

bool CGlobalDesktop::LoadDefaultDesktop(void)
{
    CFileName lname;
    lname = GlobalSetup->GetUserDesktopPath() / "default.dsk";
    if( ! CFileSystem::IsFile(lname) ){
        lname = GlobalSetup->GetSystemDesktopPath() / "default.dsk";
    }

    return( LoadDesktop(lname) );
}

//------------------------------------------------------------------------------

bool CGlobalDesktop::SaveDefaultDesktop(void)
{
    if( DesktopData.HasChildNodes() == false ){
        // no data to save
        return(true);
    }

    CFileName lname;
    lname = GlobalSetup->GetUserDesktopPath() / "default.dsk";
    return( SaveDesktop(lname) );
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

void CGlobalDesktop::SetInitialProjectType(const CUUID& uuid)
{
    CXMLElement* p_root = DesktopData.GetChildElementByPath("desktop/main",true);
    p_root->SetAttribute("InitialProjectUUID",uuid.GetStringForm());
}

//------------------------------------------------------------------------------

const CUUID CGlobalDesktop::GetInitialProjectType(void)
{
    CUUID uuid;
    CXMLElement* p_root = DesktopData.GetChildElementByPath("desktop/main");

    if( p_root ) {
        CSmallString suuid;
        if( p_root->GetAttribute("InitialProjectUUID",suuid) ){
            uuid.SetFromStringForm(suuid);
        }
    }

    return(uuid);
}

//------------------------------------------------------------------------------

void CGlobalDesktop::SetProjectRestoreMode(EDesktopProjectRestoreMode mode)
{
    CXMLElement* p_root = DesktopData.GetChildElementByPath("desktop/main",true);
    p_root->SetAttribute("ProjectRestoreMode",mode);
}

//------------------------------------------------------------------------------

EDesktopProjectRestoreMode CGlobalDesktop::GetProjectRestoreMode(void)
{
    CXMLElement* p_root = DesktopData.GetChildElementByPath("desktop/main");

    int imode;
    EDesktopProjectRestoreMode mode = EDPRM_NEW_ROJECT_WP;
    if( (p_root != NULL) && p_root->GetAttribute("ProjectRestoreMode",imode) ){
        mode = static_cast<EDesktopProjectRestoreMode>(imode);
    }

    return(mode);
}

//------------------------------------------------------------------------------

void CGlobalDesktop::RestoreInitialProject(void)
{
    // be sure that all events were processed
    QCoreApplication::processEvents();

    // restore initial project ----------------------
    switch(GetProjectRestoreMode()) {
        case EDPRM_NEW_ROJECT_WP: {
            // open new project WP
            CExtUUID mp_uuid(NULL);
            mp_uuid.LoadFromString("{NEW_PROJECT_WORK_PANEL:4cf836af-c5ca-4799-b0b7-bd53209d969e}");

            CWorkPanel* p_wp = static_cast<CWorkPanel*>(PluginDatabase.CreateObject(mp_uuid));
            if( p_wp == NULL ) {
                ES_ERROR("unable to create new project work panel ({NEW_PROJECT_WORK_PANEL:4cf836af-c5ca-4799-b0b7-bd53209d969e})");
                QMessageBox::critical(NULL,tr("Error"),
                                      tr("An error occurred during new project work panel opening!"),
                                      QMessageBox::Ok,
                                      QMessageBox::Ok);
                break;
            }
            p_wp->Show();
        }
        break;
        case EDPRM_USER: {
            CUUID mp_uuid;
            mp_uuid = GetInitialProjectType();

            CProject* p_project = Projects->NewProject(mp_uuid);
            if( p_project == NULL ) {
                ES_ERROR("unable to create initial project");
                QMessageBox::critical(NULL,tr("Error"),
                                      tr("An error occurred during new project opening!"),
                                      QMessageBox::Ok,
                                      QMessageBox::Ok);
                break;
            }
            p_project->ShowProject();
        }
        break;
    }

}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

void CGlobalDesktop::EnableWPRestoration(bool enable)
{
    CXMLElement* p_root = DesktopData.GetChildElementByPath("desktop/main",true);
    p_root->SetAttribute("WPRestorationEnabled",enable);
}

//------------------------------------------------------------------------------

bool CGlobalDesktop::IsWPRestorationEnabled(void)
{
    CXMLElement* p_root = DesktopData.GetChildElementByPath("desktop/main");

    // by default WP desktop restoration is enabled
    bool setup = true;

    if( p_root ) {
        p_root->GetAttribute("WPRestorationEnabled", setup);
    }

    return(setup);
}

//------------------------------------------------------------------------------

void CGlobalDesktop::EnableInitialProjectAfterLastIsClosed(bool enable)
{
    CXMLElement* p_root = DesktopData.GetChildElementByPath("desktop/main",true);
    p_root->SetAttribute("InitialProjectEnabledAfterLastIsClosed",enable);
}

//------------------------------------------------------------------------------

bool CGlobalDesktop::IsInitialProjectEnabledAfterLastIsClosed(void)
{
    CXMLElement* p_root = DesktopData.GetChildElementByPath("desktop/main");

    // by default InitialProjectAfterLastIsClosed is disabled
    bool setup = false;

    if( p_root ) {
        p_root->GetAttribute("InitialProjectEnabledAfterLastIsClosed", setup);
    }

    return(setup);
}

//------------------------------------------------------------------------------

void CGlobalDesktop::RestoreWorkPanels(void)
{
    // shall we restore workpanels?
    if( IsWPRestorationEnabled() == false ) return;

    // close all global work panels if any
    // FIXME
    // WorkPanels->RemoveAllGlobalWorkPanels();

    // create new set of workpanels
    CXMLElement*    p_root = DesktopData.GetChildElementByPath("desktop/workpanels");
    CXMLIterator    I(p_root);
    CXMLElement*    p_ele;

    while( (p_ele = I.GetNextChildElement("item")) != NULL ) {
        CExtUUID uuid;
        if( uuid.GetValue(p_ele,"uuid") ) {
            bool opened = false;
            p_ele->GetAttribute("open",opened);
            if( opened == true ) {
                PluginDatabase.CreateObject(uuid);
            }
        }
    }
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

void CGlobalDesktop::NotifyAllMainWindows(const QString sextuuid)
{
    emit OnMainWindowSetupChanged(sextuuid);
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

