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

#include <ProObject.hpp>
#include <PluginObject.hpp>
#include <XMLElement.hpp>
#include <ErrorSystem.hpp>
#include <PluginDatabase.hpp>
#include <ExtComObjectDesigner.hpp>
#include <Project.hpp>
#include <ProObjectHistory.hpp>
#include <HistoryList.hpp>
#include <ProObjectRegObjectsModel.hpp>
#include <StaticIndexes.hpp>

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

void ProObjectRegisterMetaObject(void)
{
    qRegisterMetaType<EStatusChanged>("EStatusChanged");
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

CProObject::CProObject(CPluginObject* objectinfo,
                       CExtComObject* p_owner,
                       CProject* p_project)
    : CExtComObject(objectinfo,p_owner)
{
    Project = p_project;
    ObjectDesigner = NULL;
    Flags |= EPOF_VISIBLE;
    Index = 0;

    // set default object index if linking is not enabled
    if( (Project != NULL) && (Project != this) ){
        if( Project->LinkingEnabled == 0 ) {
            SetIndex(Project->GetFreeObjectIndex());
        }
    }
}

//------------------------------------------------------------------------------

CProObject::CProObject(CPluginObject* objectinfo,
                       CExtComObject* p_owner,
                       CProject* p_project,
                       const QString& name)
    : CExtComObject(objectinfo,p_owner)
{
    Project = p_project;
    ObjectDesigner = NULL;
    Flags |= EPOF_VISIBLE;
    Index = 0;
    SetName(name);

    // set default object index if linking is not enabled
    if( (Project != NULL) && (Project != this) ){
        if( Project->LinkingEnabled == 0 ) {
            SetIndex(Project->GetFreeObjectIndex());
        }
    }
}

//------------------------------------------------------------------------------

CProObject::CProObject(CPluginObject* objectinfo,
                       CExtComObject* p_owner,
                       CProject* p_project,
                       bool no_index)
    : CExtComObject(objectinfo,p_owner)
{
    Project = p_project;
    ObjectDesigner = NULL;
    Flags |= EPOF_VISIBLE;
    Index = 0;

    // set default object index if linking is not enabled
    if( (Project != NULL) && (Project != this) && (no_index == false) ){
        if( Project->LinkingEnabled == 0 ) {
            SetIndex(Project->GetFreeObjectIndex());
        }
    }
}

//------------------------------------------------------------------------------

CProObject::~CProObject(void)
{
    // we need to remove record in ObjectMap
    if( (Project != NULL) && (Project != this) ){
        // we cannot work with ObjectMap for Project object since
        // CProject part is already destroyed when CProObject destructor is called
        Project->ObjectMap.remove(GetIndex());
    }
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

CHistoryNode* CProObject::BeginChangeWH(EHistoryChangeLevel level,
                                        const QString& short_descr,
                                        const QString& long_descr)
{
    if( Project == NULL ){
        ES_ERROR("unable to change history without project");
        return(NULL);
    }

    // begin history change - are we on correct level?
    if( Project->GetHistory()->BeginChange(level) == false ) {
        CSmallString warning;
        warning << "the change is not allowed because the subsystem is locked";
        ES_WARNING(warning);
        return(NULL);
    }

    // register history node
    CHistoryNode* p_hi = new CHistoryNode(Project,short_descr,long_descr);
    Project->GetHistory()->RegisterChange(p_hi);

    return(p_hi);
}

//------------------------------------------------------------------------------

void CProObject::EndChangeWH(void)
{
    if( Project == NULL ) return;
    Project->GetHistory()->EndChange();
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

bool CProObject::BeginChange(EHistoryChangeLevel lockmodelevel)
{
    if( Project == NULL ) return(false);
    return(Project->GetHistory()->BeginChange(lockmodelevel));
}

//------------------------------------------------------------------------------

void CProObject::RegisterChange(CHistoryNode *p_node)
{
    if( Project == NULL ) return;
    Project->GetHistory()->RegisterChange(p_node);
}

//------------------------------------------------------------------------------

void CProObject::EndChange(void)
{
    if( Project == NULL ) return;
    Project->GetHistory()->EndChange();
}

//------------------------------------------------------------------------------

bool CProObject::IsHistoryLocked(EHistoryChangeLevel level)
{
    if( Project == NULL ) return(false);
    return(Project->GetHistory()->IsLocked(level));
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

bool CProObject::SetNameWH(const QString& name)
{
    if( GetName() == name ) return(true);

    // begin history change - are we on correct level?
    CHistoryNode* p_history = BeginChangeWH(EHCL_DESCRIPTION,"name change");
    if( p_history == NULL ) {
        return(false);
    }

    // perform change
    SetName(name,p_history);

    // end of history recorded change
    EndChangeWH();
    return(true);
}

//------------------------------------------------------------------------------

bool CProObject::SetDescriptionWH(const QString& descrip)
{
    if( GetDescription() == descrip ) return(true);

    // begin history change - are we on correct level?
    CHistoryNode* p_history = BeginChangeWH(EHCL_DESCRIPTION,"description change");
    if( p_history == NULL ) {
        return(false);
    }

    SetDescription(descrip,p_history);

    EndChangeWH();
    return(true);
}

//------------------------------------------------------------------------------

bool CProObject::SetFlagsWH(const CProObjectFlags& flags)
{
    if( GetFlags() == flags ) return(true);

    // generate history record
    CHistoryNode* p_history = BeginChangeWH(EHCL_DESCRIPTION,"flags change");
    if( p_history == NULL ) {
        return(false);
    }

    SetFlags(flags,p_history);

    EndChangeWH();
    return(true);
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

void CProObject::SetName(const QString& name,CHistoryNode* p_history)
{
    if( GetName() == name ) return;

    if( p_history ){
        CHistoryItem* p_hi = new CProObjectNameHI(this,name);
        p_history->Register(p_hi);
    }

    CExtComObject::SetName(name);
    emit OnStatusChanged(ESC_NAME);
}

//------------------------------------------------------------------------------

void CProObject::SetDescription(const QString& descrip,CHistoryNode* p_history)
{
    if( GetDescription() == descrip ) return;

    if( p_history ){
        CHistoryItem* p_hi = new CProObjectDescrHI(this,descrip);
        p_history->Register(p_hi);
    }

    Description = descrip;
    emit OnStatusChanged(ESC_DESCRIPTION);
}

//------------------------------------------------------------------------------

const QString& CProObject::GetDescription(void)
{
    return(Description);
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

void CProObject::SetIndex(int index)
{
    // check index
    StaticIndexes.CheckIndex(index,GetType());

    if( Project == this ){
        // set index and quit
        // CProject is not constructed yet so we cannot update ObjectMap
        Index = index;
        return;
    }

    if( (Index > 0) && (Project != NULL) ){
        // cross-validate ID consistency
        CProObject* p_obj = Project->ObjectMap[index];
        if( (p_obj != NULL) && (p_obj != this) ){
            INVALID_ARGUMENT("different object with the same index is already registered");
        }
        // destroy previous key
        Project->ObjectMap.remove(Index);
    }
    Index = index;
    if( Project ){
        // register object in ObjectMap
        Project->ObjectMap[index] = this;
        // update top index
        Project->SetTopObjectIndex(index);
    }
}

//------------------------------------------------------------------------------

int CProObject::GetIndex(void) const
{
    return(Index);
}

//------------------------------------------------------------------------------

const QString CProObject::GetProjectMIMEType(const QString& baseid)
{
    // format: nemesis/projectID/baseid
    QString mime;
    mime = "nemesis/";
    if( GetProject() != NULL ){
        mime += GetProject()->GetProjectID().toString();
        mime += "/";
    }
    mime += baseid;
    return(mime);
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

void CProObject::SetFlags(const CProObjectFlags& flags,CHistoryNode* p_history)
{
    if( GetFlags() == flags ) return;

    if( p_history != NULL ){
        CHistoryItem* p_hi = new CProObjectFlagsHI(this,flags);
        p_history->Register(p_hi);
    }

    Flags = flags;
    emit OnStatusChanged(ESC_FLAG);

    CExtComObject*  p_parent = dynamic_cast<CExtComObject*>(parent());
    if( p_parent != NULL ) {
        if( p_parent->ConFlags.testFlag(EECOF_DYNAMICAL_CONTENTS) ) {
            p_parent->EmitChildContainerChanged(this);
        }
    }
}

//------------------------------------------------------------------------------

void CProObject::SetFlag(EProObjectFlag flag,bool set)
{
    if( set ) {
        Flags |= flag;
    } else {
        Flags &= ~flag;
    }
    emit OnStatusChanged(ESC_FLAG);

    CExtComObject*  p_parent = dynamic_cast<CExtComObject*>(parent());
    if( p_parent != NULL ) {
        if( p_parent->ConFlags.testFlag(EECOF_DYNAMICAL_CONTENTS) ) {
            p_parent->EmitChildContainerChanged(this);
        }
    }
}

//------------------------------------------------------------------------------

bool CProObject::IsFlagSet(EProObjectFlag flag)
{
    return(Flags.testFlag(flag));
}

//------------------------------------------------------------------------------

const CProObjectFlags& CProObject::GetFlags(void) const
{
    return(Flags);
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

void CProObject::RegisterObject(CProObject* p_ro)
{
    if( RegisteredObjects.contains(p_ro) ) return;
    RegisteredObjects.append(p_ro);
    emit OnProObjectRegistered(p_ro);
}

//------------------------------------------------------------------------------

void CProObject::UnregisterObject(CProObject* p_ro)
{
    if( ! RegisteredObjects.contains(p_ro) ) return;
    RegisteredObjects.removeOne(p_ro);
    emit OnProObjectUnregistered(p_ro);
}

//------------------------------------------------------------------------------

CProObject* CProObject::RemoveFirstFromROList(void)
{
    if( RegisteredObjects.isEmpty() ) return(NULL);
    CProObject* p_obj = RegisteredObjects.takeFirst();
    emit OnProObjectUnregistered(p_obj);
    return(p_obj);
}

//------------------------------------------------------------------------------

void  CProObject::UnregisterAllRegisteredObjects(CHistoryNode* p_history)
{
    while( ! RegisteredObjects.isEmpty() ){
        CProObject* p_ro = RegisteredObjects.takeFirst();
        p_ro->RemoveFromRegistered(this,p_history);
    }
}

//------------------------------------------------------------------------------

void CProObject::RemoveFromRegistered(CProObject* p_object,
                                      CHistoryNode* p_history)
{
    // nothing to be here
}

//------------------------------------------------------------------------------

CContainerModel* CProObject::GetRegObjectsContainerModel(QObject* p_parent)
{
    // data sent to CreateObject points to enumerated object
    // reason is that only object itself known how to use this information
    // model ownership is then solved via setParent() method
    CProObjectRegObjectsModel* p_obj = new CProObjectRegObjectsModel(NULL);
    p_obj->SetRootObject(this);
    p_obj->setParent(p_parent);
    return(p_obj);
}

//------------------------------------------------------------------------------

const QList<CProObject*>& CProObject::GetROList(void) const
{
    return(RegisteredObjects);
}

//------------------------------------------------------------------------------

void CProObject::UpdateDesignerRegistration(void)
{
    if( GetProject() == NULL ) return;
    if( IsObjectDesignerOpened() ){
        GetProject()->OpenedDesigners.append(this);
    } else {
        GetProject()->OpenedDesigners.removeAll(this);
    }
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

CProject* CProObject::GetProject(void) const
{
    return(Project);
}

//------------------------------------------------------------------------------

CGeoDescriptor  CProObject::GetGeoDescriptor()
{
    return(CGeoDescriptor());
}

//------------------------------------------------------------------------------

void CProObject::GetObjectMetrics(CObjMetrics& metrics)
{
    // nothing to do here - method prototype
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

void CProObject::LoadData(CXMLElement* p_ele)
{
    if( p_ele == NULL ) {
        INVALID_ARGUMENT("p_ele is NULL");
    }

    int index = 0;
    if( p_ele->GetAttribute("id",index) == true ) {
        // get base index
        if( Project != NULL ){
            index += Project->GetBaseObjectIndex();
            // set object id - only when object is member of project
            // if this will change in the future then you need to change
            // CProObject::SetIndex and CStaticIndexList::CheckIndex
            // so they will not argue about illegal static indexes
            SetIndex(index);
        }
    }

    QString buf;

    // load the rest
    if( ! IsFlagSet(EPOF_RO_NAME) ){
        if( p_ele->GetAttribute("name",buf) == true ) {
            SetName(buf);
        }
    }
    if( IsFlagSet(EPOF_RO_DESCRIPTION) ){
        if( p_ele->GetAttribute("descr",buf) == true ) {
            SetDescription(buf);
        }
    }

    int nflags = 0;
    CProObjectFlags flags;
    if( p_ele->GetAttribute("flags",nflags) == true ) {
        flags = CProObjectFlags(QFlag(nflags));
    } else {
        flags = CProObjectFlags(QFlag(0));
    }
    CProObjectFlags mask =  CProObjectFlags(QFlag(EPOF_SAVE_MASK));
    CProObjectFlags tech = Flags & (~mask);
    CProObjectFlags user = flags & mask;
    Flags = tech | user;

    // designer data
    CXMLElement* p_ext = p_ele->GetFirstChildElement("designer");
    if( p_ext != NULL ) {
        CXMLElement* p_dest = GetDesignerData(true);
        p_dest->RemoveAllChildNodes();
        p_dest->RemoveAllAttributes();
        p_dest->CopyContentsFrom(p_ext);
    }

    p_ele = GetDesignerData();
    if( p_ele != NULL ) {
        bool open = false;
        p_ele->GetAttribute("open",open);
        if( open == true ){
            if( GetProject() ) GetProject()->ScheduleDesignerOpening(this);
        }
    }
}

//------------------------------------------------------------------------------

void CProObject::LinkObject(const QString& role,CProObject* p_object)
{
    // nothing to be here
}

//------------------------------------------------------------------------------

void CProObject::SaveData(CXMLElement* p_ele)
{
    if( p_ele == NULL ) {
        INVALID_ARGUMENT("p_ele is NULL");
    }

    if( GetIndex() > 0 ) {
        // save only valid indexes
        p_ele->SetAttribute("id",GetIndex());
    }

    // for manipulation with object name, see
    // CExtComObject::GetName
    if( CExtComObject::GetName() != NULL ) p_ele->SetAttribute("name",CExtComObject::GetName());
    if( GetDescription() != NULL ) p_ele->SetAttribute("descr",GetDescription());

    CProObjectFlags mask =  CProObjectFlags(QFlag(EPOF_SAVE_MASK));
    int nflags = Flags & mask;
    if( nflags != 0 ){
        p_ele->SetAttribute("flags",nflags);
    }

    if( GetProject() && GetProject()->IsFlagSet<EProjecFlag>(EPF_PERSISTENT_DESIGNERS) ){
        // save designer data
        SaveObjectDesignerSetup();

        // extensions
        CXMLElement* p_ext = GetDesignerData();
        if( p_ext != NULL ) {
            p_ext->DuplicateNode(p_ele);
        }
    }
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================




