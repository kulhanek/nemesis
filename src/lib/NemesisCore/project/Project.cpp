// =============================================================================
// NEMESIS - Molecular Modelling Package
// -----------------------------------------------------------------------------
//    Copyright (C) 2010 Petr Kulhanek, kulhanek@chemi.muni.cz
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

#include <Project.hpp>

#include <ProjectList.hpp>
#include <ErrorSystem.hpp>
#include <XMLDocument.hpp>
#include <XMLPrinter.hpp>
#include <XMLDeclaration.hpp>
#include <XMLComment.hpp>
#include <XMLElement.hpp>
#include <SelectionList.hpp>
#include <Graphics.hpp>
#include <HistoryList.hpp>
#include <JobList.hpp>
#include <DesktopSystem.hpp>
#include <GraphicsObject.hpp>
#include <Structure.hpp>
#include <ProjectDesktop.hpp>
#include <StructureList.hpp>
#include <PropertyList.hpp>
#include <NemesisOptions.hpp>
#include <MainWindow.hpp>
#include <QCoreApplication>
#include <GraphicsProfileList.hpp>
#include <GraphicsObjectList.hpp>
#include <GraphicsViewList.hpp>
#include <WorkPanelList.hpp>
#include <StaticIndexes.hpp>
#include <TrajectoryList.hpp>

//------------------------------------------------------------------------------

void ProjectRegisterMetaObject(void)
{
    qRegisterMetaType<ETextNotificationType>("ETextNotificationType");
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

CProject::CProject(CPluginObject* p_objectinfo,CExtComObject* p_parent)
    : CProObject(p_objectinfo,p_parent,this,true)
{
    ConFlags |= EECOF_SUB_CONTAINERS;

    // generate unique ID - used for drag&drop
    ProjectID = QUuid::createUuid();

    // set default project flags
    SetFlag<EProjecFlag>(EPF_PERSISTENT_DESIGNERS,true);
    SetFlag<EProjecFlag>(EPF_PERSISTENT_DESKTOP,true);

    // set index management
    BaseIndex = 0;
    IndexCounter.SetIndex(PROJECT_INDEX);
    LinkingEnabled = 0;

    // set project index - it should be 1 for project
    SetIndex(GetFreeObjectIndex());

    Structures = new CStructureList(this,true);
    Structures->SetIndex(STRUCTURES_INDEX);

    Properties = new CPropertyList(this,true);
    Properties->SetIndex(PROPERTIES_INDEX);

    Graphics = new CGraphics(this,true);
    Graphics->SetStaticIndexes();

    Selection = new CSelectionList(this,true);
    Selection->SetIndex(SELECTION_LIST_INDEX);

    History = new CHistoryList(this,true);
    History->SetIndex(HISTORY_LIST_INDEX);

    connect(History,SIGNAL(OnHistoryChanged(EHistoryChangeMessage)),
            this,SLOT(ProjectHistoryChanged(void)));

    MouseHandler = new CMouseHandler(this,true);
    MouseHandler->SetIndex(MOUSE_HANDLER_INDEX);

    Jobs = new CJobList(this,true);
    Jobs->SetIndex(JOB_LIST_INDEX);

    Desktop = new CProjectDesktop(this);

    SetFlag(EPOF_TMP_NAME,true);

    // please do not forget to set LAST_USER_STATIC_INDEX in the top child constructor!!!
    // SetTopObjectIndex(LAST_USER_STATIC_INDEX);
}

//---------------------------------------------------------------------------

CProject::~CProject(void)
{   
    // stop all jobs if any is running
    Jobs->TerminateAllJobs();

    // destroy all children objects here, so their CProObject destructors can
    // reach ObjectMap
    // for details CProObject::~CProObject
    foreach(QObject* p_object,children()){
        delete p_object;
    }

    // process all pending events, so QEvent::DeferredDelete objects will be deleted
    QCoreApplication::processEvents();

    // be sure that project is not in the project list anymore
    // is it safe?
    setParent(NULL);

    Projects->EmitChildContainerRemoved(this);
    Projects->EmitProjectRemoved(this);
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

bool CProject::SaveProject(void)
{
    if( IsFlagSet(EPOF_READ_ONLY) == true ) {
        ES_ERROR("project is read-only");
        return(false);
    }

    // update project layout data
    if( GetMainWindow() ){
        GetMainWindow()->SaveDesktop();
    }

    CXMLDocument xml_document;

    // create document header
    xml_document.CreateChildDeclaration();

    // create document comment
    CXMLComment* p_info = xml_document.CreateChildComment();
    p_info->SetComment("NEMESIS v12.0 Project File");

    CXMLElement* p_root = xml_document.CreateChildElement("project");

    // header data
    CXMLElement* p_header = p_root->CreateChildElement("header");
    p_header->SetAttribute("uuid",GetType().GetFullStringForm());
    p_header->SetAttribute("version","12.0");

    // data
    CXMLElement* p_data = p_root->CreateChildElement("data");

    try {
        SaveData(p_data);
    } catch(std::exception& e) {
        ES_ERROR_FROM_EXCEPTION("unable to save",e);
        return(false);
    }

    // save data to disk
    CXMLPrinter xml_printer;

    xml_printer.SetPrintedXMLNode(&xml_document);

    if( xml_printer.Print(GetFullName()) == false ) {
        ES_ERROR("unable to save XML file");
        return(false);
    }

    SetFlag(EPOF_TMP_NAME,false);
    SetFlag(EPOF_PROJECT_CHANGED,false);

    return(true);
}

//---------------------------------------------------------------------------

bool CProject::SaveProjectAs(const CFileName& fullname)
{
    ProjectPath = fullname.GetFileDirectory();
    SetName(QString(fullname.GetFileNameWithoutExt()));
    bool result  = SaveProject();
    Projects->EmitProjectNameChanged(this);
    return(result);
}

//---------------------------------------------------------------------------

void CProject::CloseProject(void)
{
    setParent(NULL); // remove it from the list

    // close all related designers
    CloseOpenedObjectDesigners();

    // close all related workpanels
    WorkPanels->RemoveAllProjectWorkPanels(this);

    // close all graphics view
    GetGraphics()->GetViews()->CloseAllViews();

    // inform parent object that this will be removed - valid for tree view
    Projects->EmitChildContainerRemoved(this);

    deleteLater();
    if( GetMainWindow() ){
        GetMainWindow()->hide();
    }
}

//---------------------------------------------------------------------------

void CProject::CreateMainWindow(void)
{
    // nothing to do here
}

//---------------------------------------------------------------------------

void CProject::ShowProject(void)
{
    if( GetMainWindow() != NULL ) return;
    CreateMainWindow();

    BringToFront();
    OpenScheduledObjectDesigners();
}

//---------------------------------------------------------------------------

void CProject::BringToFront(void)
{
    QWidget* p_widget = GetMainWindow();
    if( p_widget == NULL ) return;
    p_widget->activateWindow();
    p_widget->raise();
}

//---------------------------------------------------------------------------

void CProject::ClearProject(void)
{
    Jobs->TerminateAllJobs();
    History->ClearHistory();
    Selection->ResetSelection();
    Properties->ClearProperties();
    Graphics->ClearGraphics();
    Structures->ClearStructures();

    BaseIndex = 0;
    IndexCounter.SetIndex(1);
    LinkingEnabled = false;
    Links.clear();

    // set top index
    SetTopObjectIndex(LAST_USER_STATIC_INDEX);
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

const CFileName  CProject::GetFullName(void) const
{
    return(ProjectPath / GetName() + ".npr");
}

//---------------------------------------------------------------------------

const CFileName& CProject::GetPath(void) const
{
    return(ProjectPath);
}

//---------------------------------------------------------------------------

CStructureList* CProject::GetStructures(void)
{
    return(Structures);
}

//---------------------------------------------------------------------------

CStructure* CProject::GetActiveStructure(void)
{
    return(Structures->GetActiveStructure());
}

//---------------------------------------------------------------------------

CTrajectoryList* CProject::GetTrajectories(void)
{
    return(NULL);
}

//---------------------------------------------------------------------------

CTrajectory* CProject::GetActiveTrajectory(void)
{
    if( GetTrajectories() ){
        return(GetTrajectories()->GetActiveTrajectory());
    } else {
        return(NULL);
    }
}

//---------------------------------------------------------------------------

CHistoryList* CProject::GetHistory(void)
{
    return(History);
}

//---------------------------------------------------------------------------

CSelectionList* CProject::GetSelection(void)
{
    return(Selection);
}

//---------------------------------------------------------------------------

CGraphics* CProject::GetGraphics(void)
{
    return(Graphics);
}

//---------------------------------------------------------------------------

CJobList* CProject::GetJobs(void)
{
    return(Jobs);
}

//---------------------------------------------------------------------------

CPropertyList* CProject::GetProperties(void)
{
    return(Properties);
}

//---------------------------------------------------------------------------

CMouseHandler* CProject::GetMouseHandler(void)
{
    return(MouseHandler);
}

//------------------------------------------------------------------------------

CMainWindow* CProject::GetMainWindow(void)
{
    return(NULL);
}

//------------------------------------------------------------------------------

CProjectDesktop* CProject::GetDesktop(void)
{
    return(Desktop);
}

//------------------------------------------------------------------------------

const QUuid& CProject::GetProjectID(void) const
{
    return(ProjectID);
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

void CProject::TextNotification(ETextNotificationType type,const QString& text,int time)
{
    emit OnTextNotification(type,text,time);
}

//------------------------------------------------------------------------------

void CProject::StartProgressNotification(int length)
{
    emit OnProgressNotification(EPNT_START,length,QString());
}

//------------------------------------------------------------------------------

void CProject::ProgressNotification(int progress,const QString& text)
{
    emit OnProgressNotification(EPNT_PROGRESS,progress,text);
}

//------------------------------------------------------------------------------

void CProject::EndProgressNotification(void)
{
    emit OnProgressNotification(EPNT_END,0,QString());
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

void CProject::NewData(void)
{
    // create root graphics objects -----------------
    CExtUUID objectuuid;

    CGraphicsProfile* p_profile = Graphics->GetProfiles()->CreateProfile();
    Graphics->GetProfiles()->SetActiveProfile(p_profile);

    CGraphicsObject* p_gobj;

    objectuuid.LoadFromString("{LIGHT_OBJECT:03f65a4c-2f8a-4629-8cdc-3409df428db8}");
    p_gobj = AddGraphicsObject(objectuuid,p_profile);

    objectuuid.LoadFromString("{BACKGROUND_OBJECT:f646b31e-e0f8-406d-a9ea-f7f9df0349d3}");
    p_gobj = AddGraphicsObject(objectuuid,p_profile);

    if( GetActiveStructure() ){
        objectuuid.LoadFromString("{STANDARD_MODEL_OBJECT:b8ec4377-b886-44c3-a206-4d40eaf7d5f2}");
        p_gobj = AddGraphicsObject(objectuuid,p_profile);
        if( p_gobj ){
            p_gobj->AddObject(GetStructures());
        }
    }

    objectuuid.LoadFromString("{FREEZED_ATOMS_OBJECT:d638ed90-b60a-4344-a810-eb55f09125a6}");
    p_gobj = AddGraphicsObject(objectuuid,p_profile);
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

void CProject::LoadData(CXMLElement* p_ele)
{
    if( p_ele == NULL ) {
        INVALID_ARGUMENT("p_ele is NULL");
    }

    // load core project data
    CProObject::LoadData(p_ele);

    CXMLElement* p_sele;

    // structures
    p_sele = p_ele->GetFirstChildElement("structures");
    if( p_sele ){
        GetStructures()->LoadData(p_sele);
    }

    // properties
    p_sele = p_ele->GetFirstChildElement("properties");
    if( p_sele ){
        GetProperties()->LoadData(p_sele);
    }

    // graphics
    p_sele = p_ele->GetFirstChildElement("graphics");
    if( p_sele ){
        GetGraphics()->LoadData(p_sele);
    }

    // desktop
    if( IsFlagSet<EProjecFlag>(EPF_PERSISTENT_DESKTOP) ){
        p_sele = p_ele->GetFirstChildElement("desktop");
        if( p_sele ){
            GetDesktop()->LoadData(p_sele);
        }
    }

}

//------------------------------------------------------------------------------

void CProject::SaveData(CXMLElement* p_ele)
{
    if( p_ele == NULL ) {
        INVALID_ARGUMENT("p_ele is NULL");
    }

    CXMLElement* p_sele;

    // save core project data
    CProObject::SaveData(p_ele);

    // save subproject data ----------------------

    // structures
    p_sele = p_ele->CreateChildElement("structures");
    GetStructures()->SaveData(p_sele);

    // properties
    p_sele = p_ele->CreateChildElement("properties");
    GetProperties()->SaveData(p_sele);

    // graphics
    p_sele = p_ele->CreateChildElement("graphics");
    GetGraphics()->SaveData(p_sele);

    // desktop
    if( IsFlagSet<EProjecFlag>(EPF_PERSISTENT_DESKTOP) ){
        p_sele = p_ele->CreateChildElement("desktop");
        GetDesktop()->SaveData(p_sele);
    }

    // currently we do not save other data
}

//------------------------------------------------------------------------------

bool CProject::ProcessArguments(int& narg)
{
    return(false);
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

CProObject* CProject::FindObject(int index)
{
    return(ObjectMap[index]);
}

//------------------------------------------------------------------------------

int CProject::GetBaseObjectIndex(void)
{
    return(BaseIndex);
}

//------------------------------------------------------------------------------

int CProject::GetFreeObjectIndex(void)
{
    return(IndexCounter.GetIndex());
}

//------------------------------------------------------------------------------

void CProject::SetTopObjectIndex(int object_id)
{
    IndexCounter.SetTopIndex(object_id);
}

//------------------------------------------------------------------------------

void CProject::BeginLinkProcedure(int base_index)
{
    if( LinkingEnabled > 0 ) return;
    LinkingEnabled++;
    if( LinkingEnabled != 1 ) return;

    if( base_index < 0 ){
        BaseIndex = IndexCounter.GetTopIndex();
    } else {
        BaseIndex = base_index;
    }
    Links.clear();
}

//------------------------------------------------------------------------------

void CProject::RequestToLinkObject(CProObject* p_requestor,CXMLElement* p_ele,const QString& role)
{
    if( p_requestor == NULL ){
        INVALID_ARGUMENT("p_requestor == NULL");
    }
    if( p_ele == NULL ){
        INVALID_ARGUMENT("p_ele == NULL");
    }

    CProObjectLink link;
    link.Requestor = p_requestor;
    p_ele->GetAttribute(role,link.ObjectID);
    link.ObjectID += GetBaseObjectIndex();
    link.Role = role;
    Links.append(link);
}

//------------------------------------------------------------------------------

void CProject::RequestToLinkObject(CProObject* p_requestor,CXMLElement* p_ele,const QString& attrname,const QString& role)
{
    if( p_requestor == NULL ){
        INVALID_ARGUMENT("p_requestor == NULL");
    }
    if( p_ele == NULL ){
        INVALID_ARGUMENT("p_ele == NULL");
    }

    CProObjectLink link;
    link.Requestor = p_requestor;
    p_ele->GetAttribute(attrname,link.ObjectID);
    link.ObjectID += GetBaseObjectIndex();
    link.Role = role;
    Links.append(link);
}

//------------------------------------------------------------------------------

void CProject::RequestToLinkObject(CProObject* p_requestor,int object_id,const QString& role)
{
    if( p_requestor == NULL ){
        INVALID_ARGUMENT("p_requestor == NULL");
    }
    CProObjectLink link;
    link.Requestor = p_requestor;
    link.ObjectID = object_id;
    link.ObjectID += GetBaseObjectIndex();
    link.Role = role;
    Links.append(link);
}

//------------------------------------------------------------------------------

void CProject::EndLinkProcedure(void)
{
    if( LinkingEnabled <= 0 ) return;
    LinkingEnabled--;
    if( LinkingEnabled > 0 ) return;
    foreach(CProObjectLink link, Links){
        CProObject* p_obj = FindObject(link.ObjectID);
        link.Requestor->LinkObject(link.Role,p_obj);
    }
    Links.clear();
    BaseIndex = 0;
    LinkingEnabled = 0;
}

//------------------------------------------------------------------------------

void CProject::ScheduleDesignerOpening(CProObject* p_obj)
{
    ScheduledDesigners.append(p_obj);
}

//------------------------------------------------------------------------------

void CProject::OpenScheduledObjectDesigners(void)
{
    foreach(CProObject* p_obj, ScheduledDesigners){
        p_obj->OpenObjectDesigner();
    }
    ScheduledDesigners.clear();
}

//------------------------------------------------------------------------------

void CProject::CloseOpenedObjectDesigners(void)
{
    foreach(CProObject* p_obj, OpenedDesigners){
        p_obj->CloseObjectDesigner();
    }
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

void CProject::RepaintProject(void)
{
    Graphics->GetViews()->RepaintAllViews();
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

void CProject::ProjectHistoryChanged(void)
{
    SetFlag(EPOF_PROJECT_CHANGED,true);
    OpenScheduledObjectDesigners();
    RepaintProject();
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

CGraphicsObject* CProject::AddGraphicsObject(const CUUID& uuid, CGraphicsProfile* p_profile)
{
    CGraphicsObject* p_obj = Graphics->GetObjects()->CreateObject(uuid);
    if( p_obj == NULL ) {
        CSmallString error;
        error << "unable to create object " << uuid.GetStringForm();
        RUNTIME_ERROR(error);
    }
    if( p_profile->AddObject(p_obj) == NULL) {
        delete p_obj;
        CSmallString error;
        error << "unable to add object " << uuid.GetStringForm() << " into a profile";
        RUNTIME_ERROR(error);
    }
    return(p_obj);
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

CProObjectLink::CProObjectLink(void)
{
    Requestor = NULL;
    ObjectID = 0;
    Role = "unk";
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

