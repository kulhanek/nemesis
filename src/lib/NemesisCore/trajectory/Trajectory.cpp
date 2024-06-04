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

#include <Trajectory.hpp>
#include <TrajectoryHistory.hpp>
#include <PluginObject.hpp>
#include <NemesisCoreModule.hpp>
#include <Project.hpp>
#include <TrajectoryList.hpp>
#include <Structure.hpp>
#include <ErrorSystem.hpp>
#include <XMLElement.hpp>
#include <TrajectorySegment.hpp>
#include <TrajectorySegmentHistory.hpp>
#include <SnapshotFilter.hpp>
#include <SnapshotFilterHistory.hpp>
#include <QTimer>
#include <PluginDatabase.hpp>
#include <AtomList.hpp>
#include <BondList.hpp>
#include <StructureList.hpp>

//------------------------------------------------------------------------------

CExtUUID        TrajectoryID(
                    "{TRAJECTORY:2f2b512b-382b-4a33-8575-c8203dd94858}",
                    "Trajectory");

CPluginObject   TrajectoryObject(&NemesisCorePlugin,
                    TrajectoryID,TRAJECTORY_CAT,
                    ":/images/NemesisCore/trajectory/Trajectory.svg",
                    NULL);

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

CTrajectory::CTrajectory(CTrajectoryList* p_list)
    : CProObject(&TrajectoryObject,p_list,p_list->GetProject())
{
    Structure = NULL;
    CurrentSegmentIndex = 0;
    CurrentSnapshotIndex = 0;

    SegmentsChanged = false;
    FiltersChanged = false;
    UpdateLevel = 0;

    SegmentCounter.SetTopIndex(0);

    PlayMode = ETPM_ONCE;
    PlayStatus = ETPS_STOP;
    PlayTickTime = 10;
    PlayForward = false;

    PlayTimer = new QTimer(this);
    connect(PlayTimer, SIGNAL(timeout()), this, SLOT(PlayTick(void)));
}

//------------------------------------------------------------------------------

CTrajectory::CTrajectory(CTrajectoryList* p_list,bool no_index)
    : CProObject(&TrajectoryObject,p_list,p_list->GetProject(),no_index)
{
    Structure = NULL;
    CurrentSegmentIndex = 0;
    CurrentSnapshotIndex = 0;

    SegmentsChanged = false;
    FiltersChanged = false;
    UpdateLevel = 0;

    SegmentCounter.SetTopIndex(0);

    PlayMode = ETPM_ONCE;
    PlayStatus = ETPS_STOP;
    PlayTickTime = 10;
    PlayForward = false;

    PlayTimer = new QTimer(this);
    connect(PlayTimer, SIGNAL(timeout()), this, SLOT(PlayTick(void)));
}

//------------------------------------------------------------------------------

CTrajectory::~CTrajectory(void)
{
    PlayTimer->stop();

    CTrajectoryList* p_list = GetTrajectories();
    if( p_list ) p_list->BeginUpdate();

    if( p_list ){
        if( p_list->GetActiveTrajectory() == this ){
            p_list->SetActiveTrajectory(NULL);
        }
    }

    // release associated structure
    if( Structure ) {
        SetStructure(NULL);
    }

    setParent(NULL);    // remove object from the list

    if( p_list ){
        p_list->EmitChildContainerRemoved(this);
        p_list->ListSizeChanged();
    }

    // delete segments
    foreach(CTrajectorySegment* p_seg, Segments){
        delete p_seg;
    }

    // delete filters
    foreach(CSnapshotFilter* p_filter, Filters){
        delete p_filter;
    }

    if( p_list ) p_list->EndUpdate();
}

// ----------------------------------------------------------------------------

void CTrajectory::RemoveFromBaseList(CHistoryNode* p_history)
{
    CProObject* p_obj;

    CTrajectoryList* p_list = GetTrajectories();
    if( p_list ) p_list->BeginUpdate();

    // unregister other objects
    while( (p_obj = RemoveFirstFromROList()) !=NULL ) {
        p_obj->RemoveFromRegistered(this,p_history);
    }

    // deactivate trajectory if it was active
    if( p_list ){
        if( p_list->GetActiveTrajectory() == this ){
            p_list->SetActiveTrajectory(NULL,p_history);
        }
    }

    // register elementary change to history list
    if( p_history != NULL ) {
        CTrajectoryHI* p_strdata = new CTrajectoryHI(this,EHID_BACKWARD);
        p_history->Register(p_strdata);
    }

    // --------------------------------
    delete this; // destroy object

    if( p_list ) p_list->EndUpdate();
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

bool CTrajectory::DeleteWH(void)
{
    // object will be deleted - use parent for all operation
    CTrajectoryList* p_sl = GetTrajectories();

    CHistoryNode* p_history = p_sl->BeginChangeWH(EHCL_TRAJECTORIES,tr("delete trajectory"));
    if( p_history == NULL ) return (false);

    RemoveFromBaseList(p_history);

    p_sl->EndChangeWH();
    return(true);
}

//------------------------------------------------------------------------------

bool CTrajectory::DeleteAllSegmentsWH(void)
{
    CHistoryNode* p_history = BeginChangeWH(EHCL_TRAJECTORIES,tr("delete all trajectory segments"));
    if( p_history == NULL ) return (false);

    DeleteAllSegments(p_history);

    EndChangeWH();
    return(true);
}

//------------------------------------------------------------------------------

bool CTrajectory::DeleteAllFiltersWH(void)
{
    CHistoryNode* p_history = BeginChangeWH(EHCL_TRAJECTORIES,tr("delete all snapshot filters"));
    if( p_history == NULL ) return (false);

    DeleteAllFilters(p_history);

    EndChangeWH();
    return(true);
}

//------------------------------------------------------------------------------

CSnapshotFilter* CTrajectory::CreateFilterWH(const CUUID& objectuuid,const QString& name,
                                const QString& descr)
{
    // FIXME
    return(NULL);
}

//------------------------------------------------------------------------------

bool CTrajectory::SetStructureWH(CStructure* p_str)
{
    if( Structure == p_str ) return(true);

    CHistoryNode* p_history = BeginChangeWH(EHCL_TRAJECTORIES,tr("associate trajectory with structure"));
    if( p_history == NULL ) return (false);

    SetStructure(p_str,p_history);

    EndChangeWH();
    return(true);
}

//------------------------------------------------------------------------------

bool CTrajectory::SetPlayModeWH(ETrajectoryPlayMode mode)
{
    if( PlayMode == mode ) return(true);

    CHistoryNode* p_history = BeginChangeWH(EHCL_TRAJECTORIES,tr("set play mode"));
    if( p_history == NULL ) return (false);

    SetPlayMode(mode,p_history);

    EndChangeWH();
    return(true);
}

//------------------------------------------------------------------------------

bool CTrajectory::SetPlayTickTimeWH(int time)
{
    if( PlayTickTime == time ) return(true);

    CHistoryNode* p_history = BeginChangeWH(EHCL_TRAJECTORIES,tr("set play tick time"));
    if( p_history == NULL ) return (false);

    SetPlayTickTime(time,p_history);

    EndChangeWH();
    return(true);
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

bool CTrajectory::FirstSnapshot(void)
{
    if( GetNumberOfSegments() == 0 ){
        CurrentSegmentIndex = 0;
        EmitOnSnapshotChanged();
        return(false);
    }

    CurrentSegmentIndex = 1;

    // try to find first non-empty snapshot
    while( CurrentSegmentIndex <= GetNumberOfSegments() ){
        CTrajectorySegment* p_seg = GetSegment(CurrentSegmentIndex);
        if( p_seg->FirstSnapshot() ){
            CurrentSnapshotIndex = 1;
            EmitOnSnapshotChanged();
            return(true);
        }
        CurrentSegmentIndex++;
    }

    CurrentSegmentIndex = 0;
    CurrentSnapshotIndex = 0;
    EmitOnSnapshotChanged();
    return(false);
}

//------------------------------------------------------------------------------

bool CTrajectory::NextSnapshot(void)
{
    CTrajectorySegment* p_seg = GetSegment(CurrentSegmentIndex);
    if( p_seg == NULL ){
        // CurrentSegmentIndex is out of legal range
        CurrentSegmentIndex = 0;
        CurrentSnapshotIndex = 0;
        EmitOnSnapshotChanged();
        return(false);
    }
    // get next snapshot in the current segment
    if( p_seg->NextSnapshot() ){
        CurrentSnapshotIndex++;
        EmitOnSnapshotChanged();
        return(true);
    }

    // then try to find first non-empty snapshot from following segments
    while( CurrentSegmentIndex < GetNumberOfSegments() ){
        CurrentSegmentIndex++;
        CTrajectorySegment* p_seg = GetSegment(CurrentSegmentIndex);
        if( p_seg->FirstSnapshot() ){
            CurrentSnapshotIndex++;
            EmitOnSnapshotChanged();
            return(true);
        }
    }

    // move back to the last snapshot
    CurrentSnapshotIndex = 0; // invalidate snapshot index
    return( LastSnapshot() );
}

//------------------------------------------------------------------------------

bool CTrajectory::PrevSnapshot(void)
{
    CTrajectorySegment* p_seg = GetSegment(CurrentSegmentIndex);
    if( p_seg == NULL ){
        // CurrentSegmentIndex is out of legal range
        CurrentSegmentIndex = 0;
        CurrentSnapshotIndex = 0;
        EmitOnSnapshotChanged();
        return(false);
    }
    // get prev snapshot in the current segment
    if( p_seg->PrevSnapshot() ){
        CurrentSnapshotIndex--;
        EmitOnSnapshotChanged();
        return(true);
    }

    // then try to find last non-empty snapshot from prev segments
    while( CurrentSegmentIndex > 0 ){
        CTrajectorySegment* p_seg = GetSegment(CurrentSegmentIndex);
        if( p_seg->LastSnapshot() ){
            CurrentSnapshotIndex--;
            EmitOnSnapshotChanged();
            return(true);
        }
        CurrentSegmentIndex--;
    }

    // move back to the first snapshot
    CurrentSnapshotIndex = 0; // invalidate snapshot index
    return( FirstSnapshot() );
}

//------------------------------------------------------------------------------

bool CTrajectory::LastSnapshot(void)
{
    if( GetNumberOfSegments() == 0 ){
        CurrentSegmentIndex = 0;
        EmitOnSnapshotChanged();
        return(false);
    }

    CurrentSegmentIndex = GetNumberOfSegments();

    // try to find last non-empty snapshot
    while( CurrentSegmentIndex > 0 ){
        CTrajectorySegment* p_seg = GetSegment(CurrentSegmentIndex);
        if( p_seg->LastSnapshot() ){
            CurrentSnapshotIndex = GetNumberOfSnapshots();
            EmitOnSnapshotChanged();
            return(true);
        }
        CurrentSegmentIndex--;
    }

    CurrentSegmentIndex = 0;
    CurrentSnapshotIndex = 0;
    EmitOnSnapshotChanged();
    return(false);
}

//------------------------------------------------------------------------------

bool CTrajectory::NextSegment(void)
{
    // we need random access to correctly count number of snapshots
    CurrentSegmentIndex++;

    // then try to find first non-empty snapshot from following segments
    while( CurrentSegmentIndex <= GetNumberOfSegments() ){
        CTrajectorySegment* p_seg = GetSegment(CurrentSegmentIndex);
        if( p_seg == NULL ){
            break;
        }
        if( p_seg->FirstSnapshot() ){
            CurrentSnapshotIndex = 0;
            for(long int i=1; i < CurrentSegmentIndex; i++){
                CurrentSnapshotIndex += GetSegment(i)->GetNumberOfSnapshots();
            }
            CurrentSnapshotIndex++;
            EmitOnSnapshotChanged();
            return(true);
        }
        CurrentSegmentIndex++;
    }

    // move back to the first snapshot
    CurrentSnapshotIndex = 0; // invalidate snapshot index
    return( LastSegment() );
}

//------------------------------------------------------------------------------

bool CTrajectory::PrevSegment(void)
{
    // we need random access to correctly count number of snapshots
    CurrentSegmentIndex--;

    // then try to find last non-empty snapshot from prev segments
    while( CurrentSegmentIndex > 0 ){
        CTrajectorySegment* p_seg = GetSegment(CurrentSegmentIndex);
        if( p_seg == NULL ){
            break;
        }
        if( p_seg->FirstSnapshot() ){
            CurrentSnapshotIndex = 0;
            for(long int i=1; i < CurrentSegmentIndex; i++){
                CurrentSnapshotIndex += GetSegment(i)->GetNumberOfSnapshots();
            }
            CurrentSnapshotIndex++;
            EmitOnSnapshotChanged();
            return(true);
        }
        CurrentSegmentIndex--;
    }

    // move back to the first snapshot
    CurrentSnapshotIndex = 0; // invalidate snapshot index
    return( FirstSnapshot() );
}

//------------------------------------------------------------------------------

bool CTrajectory::LastSegment(void)
{
    // we need random access to correctly count number of snapshots

    CurrentSegmentIndex = GetNumberOfSegments();

    // then try to find last non-empty snapshot from prev segments
    while( CurrentSegmentIndex > 0 ){
        CTrajectorySegment* p_seg = GetSegment(CurrentSegmentIndex);
        if( p_seg == NULL ){
            break;
        }
        if( p_seg->FirstSnapshot() ){
            CurrentSnapshotIndex = 0;
            for(long int i=1; i < CurrentSegmentIndex; i++){
                CurrentSnapshotIndex += GetSegment(i)->GetNumberOfSnapshots();
            }
            CurrentSnapshotIndex++;
            EmitOnSnapshotChanged();
            return(true);
        }
        CurrentSegmentIndex--;
    }

    CurrentSegmentIndex = 0;
    CurrentSnapshotIndex = 0;
    EmitOnSnapshotChanged();
    return(false);
}
//------------------------------------------------------------------------------

bool CTrajectory::MoveToSegment(long int index)
{
    // method move segment in to segment with intern seg index of index
    CurrentSegmentIndex = 1;
    CurrentSnapshotIndex = 0;

    // try to find segment of intern segment index
    while( CurrentSegmentIndex <= GetNumberOfSegments() ){
        CTrajectorySegment* p_seg = GetSegment(CurrentSegmentIndex);
        if( p_seg == NULL ){
            break;
        }
        if( p_seg->GetSeqIndex() == index ){
            CurrentSnapshotIndex += p_seg->GetCurrentSnapshotIndex();
            EmitOnSnapshotChanged();
            return(true);
        }
        long int slen = p_seg->GetNumberOfSnapshots();
        CurrentSegmentIndex++;
        CurrentSnapshotIndex += slen;

    }

    CurrentSegmentIndex = 0;
    CurrentSnapshotIndex = 0;
    EmitOnSnapshotChanged();
    return(false);
}

//------------------------------------------------------------------------------

bool CTrajectory::MoveToSnapshot(long int index)
{
    if( (index < 1) || (index > GetNumberOfSnapshots()) ) return(false);

    // find segment
    CurrentSegmentIndex = 1;
    CurrentSnapshotIndex = 0;
    while( CurrentSegmentIndex > 0 ){
        CTrajectorySegment* p_seg = GetSegment(CurrentSegmentIndex);
        if( p_seg == NULL ){
            break;
        }
        long int slen = p_seg->GetNumberOfSnapshots();
        if( index <= slen ){
            p_seg->MoveToSnapshot(index);
            CurrentSnapshotIndex += index;
            EmitOnSnapshotChanged();
            return(true);
        }
        index -= slen;
        CurrentSegmentIndex++;
        CurrentSnapshotIndex += slen;
    }

    CurrentSegmentIndex = 0;
    CurrentSnapshotIndex = 0;
    EmitOnSnapshotChanged();
    return(false);
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

void CTrajectory::Play(void)
{
    if( PlayStatus == ETPS_PLAY ) return;
    PlayStatus = ETPS_PLAY;
    PlayForward = true;
    EmitOnSnapshotChanged();
    PlayTimer->start(10);
}

//------------------------------------------------------------------------------

void CTrajectory::Pause(void)
{
    if( PlayStatus == ETPS_STOP ) return;
    if( PlayStatus == ETPS_PAUSE ){
        Play();
    } else {
        PlayTimer->stop();
        PlayStatus = ETPS_PAUSE;
        EmitOnSnapshotChanged();
    }
}

//------------------------------------------------------------------------------

void CTrajectory::Stop(void)
{
    PlayTimer->stop();
    PlayStatus = ETPS_STOP;
    EmitOnSnapshotChanged();
}

//------------------------------------------------------------------------------

bool CTrajectory::IsPlaying(void)
{
    return(PlayStatus == ETPS_PLAY || PlayStatus == ETPS_PAUSE );
}

//------------------------------------------------------------------------------

bool CTrajectory::IsPaused(void)
{
    return(PlayStatus == ETPS_PAUSE);
}

//------------------------------------------------------------------------------

ETrajectoryPlayMode CTrajectory::GetPlayMode(void)
{
    return(PlayMode);
}

//------------------------------------------------------------------------------

int CTrajectory::GetPlayTickTime(void)
{
    return(PlayTickTime);
}

//------------------------------------------------------------------------------

void CTrajectory::PlayTick(void)
{
    if( PlayStatus != ETPS_PLAY ) return;
    if( PlayForward ){
        if( IsLastSnapshot() == true ){
            if( PlayMode == ETPM_ONCE ){
                PlayStatus = ETPS_STOP;
                EmitOnSnapshotChanged(); // emit signal egain
                return;
            }
            if( PlayMode == ETPM_LOOP ){
                if( FirstSnapshot() == false ) {
                    PlayStatus = ETPS_STOP;
                    EmitOnSnapshotChanged(); // emit signal egain
                    return;
                }
            }
            if( PlayMode == ETPM_ROLL ){
                PlayForward = false;
                if( PrevSnapshot() == false ) {
                    PlayStatus = ETPS_STOP;
                    EmitOnSnapshotChanged(); // emit signal egain
                    return;
                }
            }
        } else {
            if( NextSnapshot() == false ) {
                PlayStatus = ETPS_STOP;
                EmitOnSnapshotChanged(); // emit signal egain
                return;
            }
        }
    } else {
        // this is in ETPM_ROLL mode
        if( IsFirstSnapshot() == true ){
            PlayForward = true;
            if( NextSnapshot() == false ) {
                PlayStatus = ETPS_STOP;
                EmitOnSnapshotChanged(); // emit signal egain
                return;
            }
        } else {
            if( PrevSnapshot() == false ) {
                PlayStatus = ETPS_STOP;
                EmitOnSnapshotChanged(); // emit signal egain
                return;
            }
        }
    }
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

bool CTrajectory::IsFirstSnapshot(void)
{
    return( CurrentSnapshotIndex == 1 );
}

//------------------------------------------------------------------------------

bool CTrajectory::IsLastSnapshot(void)
{
    return( CurrentSnapshotIndex == GetNumberOfSnapshots() );
}

//------------------------------------------------------------------------------

bool CTrajectory::IsPrevSnapshot(void)
{
    return( CurrentSnapshotIndex > 1 );
}

//------------------------------------------------------------------------------

bool CTrajectory::IsNextSnapshot(void)
{
    return( CurrentSnapshotIndex < GetNumberOfSnapshots() );
}

//------------------------------------------------------------------------------

bool CTrajectory::IsPrevSegment(void)
{
    return( CurrentSegmentIndex > 1 );
}

//------------------------------------------------------------------------------

bool CTrajectory::IsNextSegment(void)
{
    return( CurrentSegmentIndex < GetNumberOfSegments() );
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

CTrajectoryList* CTrajectory::GetTrajectories(void) const
{
    return(static_cast<CTrajectoryList*>(parent()));
}

//------------------------------------------------------------------------------

long int CTrajectory::GetNumberOfSnapshots(void)
{
    long int count = 0;
    foreach(CTrajectorySegment* p_segment,findChildren<CTrajectorySegment*>()){
        count += p_segment->GetNumberOfSnapshots();
    }
    return(count);
}

//------------------------------------------------------------------------------

long int CTrajectory::GetNumberOfSnapshotsBefore(long int index)
{
    // method get count of segment before segment of index
    long int count = 0;
    int n = 1;
    while( n <= GetNumberOfSegments() ){
        CTrajectorySegment* p_seg = GetSegment(n);
        if( p_seg == NULL ){
            break;
        }
        if( p_seg->GetSeqIndex() == index ){
            return(count);
        }
        count += p_seg->GetNumberOfSnapshots();
        n++;
    }
    return (-1);

}

//------------------------------------------------------------------------------

long int CTrajectory::GetNumberOfSegments(void)
{
    return( Segments.count() );
}

//------------------------------------------------------------------------------

long int CTrajectory::GetCurrentSnapshotIndex(void)
{
    return(CurrentSnapshotIndex);
}

//------------------------------------------------------------------------------

long int CTrajectory::GetCurrentSegmentIndex(void)
{
    return(CurrentSegmentIndex);
}

//------------------------------------------------------------------------------

long int CTrajectory::GetNumberOfFilters(void)
{
    return( Filters.count() );
}

//------------------------------------------------------------------------------

CSnapshot* CTrajectory::GetCurrentSnapshot(void)
{
    if( (CurrentSegmentIndex < 1) || (CurrentSegmentIndex > GetNumberOfSegments() ) ){
        return(NULL);
    }
    CTrajectorySegment* p_seg = Segments.at(CurrentSegmentIndex-1);
    return(p_seg->GetCurrentSnapshot());
}

//------------------------------------------------------------------------------

bool CTrajectory::IsSegmentActive(CTrajectorySegment* p_segment)
{
    return( GetSegment(CurrentSegmentIndex) == p_segment );
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

void CTrajectory::SetStructure(CStructure* p_str,CHistoryNode* p_history)
{
    if( Structure == p_str ) return;

    CTrajectoryList* p_list = GetTrajectories();
    if( p_list ) p_list->BeginUpdate();

    // release possibly other trajectory attached to the structure
    if( p_str ){
        p_str->SetTrajectory(NULL,p_history); // with history
    }

    // delete this trajectory contents
    DeleteAllSegments(p_history);

    // generate record to history list
    if( p_history ){
        CHistoryItem* p_hitem = new CTrajectorySetStructureHI(this,p_str);
        p_history->Register(p_hitem);
    }

    if( Structure ){
        // release registration on the other side
        CStructure* p_str = Structure;
        Structure = NULL;   // it will be called from the other side (which we do not want)
        p_str->SetTrajectory(NULL); // no history
    }
    Structure = p_str;
    if( Structure ){
        // set registration on other side
        Structure->SetTrajectory(this); // no history
    }

    emit OnStatusChanged(ESC_OTHER);
    GetTrajectories()->EmitOnTrajectoryListChanged();

    if( p_list ) p_list->EndUpdate();
}

//------------------------------------------------------------------------------

CStructure* CTrajectory::GetStructure(void)
{
    return(Structure);
}

//------------------------------------------------------------------------------

void CTrajectory::DeleteAllSegments(CHistoryNode* p_history)
{
    BeginUpdate();
    foreach(CTrajectorySegment* p_seg, Segments){
        p_seg->RemoveFromBaseList(p_history);
    }
    EndUpdate();
}

//------------------------------------------------------------------------------

void CTrajectory::DeleteAllFilters(CHistoryNode* p_history)
{
    BeginUpdate();
    foreach(CSnapshotFilter* p_filter, Filters){
        p_filter->RemoveFromBaseList(p_history);
    }
    EndUpdate();
}

//------------------------------------------------------------------------------

CTrajectorySegment* CTrajectory::CreateSegment(CXMLElement* p_ele,CHistoryNode* p_history)
{   
    if( p_ele == NULL ){
        RUNTIME_ERROR("p_ele is NULL");
    }
    if( p_ele->GetName() != "segment" ){
        RUNTIME_ERROR("p_ele is not segement");
    }

    QString suuid;
    if( p_ele->GetAttribute("uuid",suuid) == false ) {
        ES_ERROR("unable to get uuid attribute");
        return(NULL);
    }
    CExtUUID  ext_uuid;
    if( ext_uuid.LoadFromString(suuid) == false ) {
        CSmallString error;
        error << "unable to decode uuid attribute " << suuid;
        ES_ERROR(error);
        return(NULL);
    }

    // create object
    CTrajectorySegment* p_seg = static_cast<CTrajectorySegment*>(PluginDatabase.CreateObject(ext_uuid,this));

    if( p_seg == NULL ) {
        CSmallString error;
        error << "unable to create trajectory segment " << ext_uuid.GetStringForm();
        ES_ERROR(error);
        return(NULL);
    }

    BeginUpdate();

    Segments.append(p_seg);

    // load data
    GetProject()->BeginLinkProcedure(0);
    p_seg->LoadData(p_ele);
    GetProject()->EndLinkProcedure();

    // register change to history
    if( p_history != NULL ){
        CTrajectorySegmentHI* p_item = new CTrajectorySegmentHI(p_seg,EHID_FORWARD);
        p_history->Register(p_item);
    }

    EmitOnTrajectorySegmentsChanged();
    EndUpdate();
    emit OnStatusChanged(ESC_OTHER);

    // notify list models
    GetTrajectories()->EmitOnTrajectoryListChanged();

    return(p_seg);
}

//------------------------------------------------------------------------------

void CTrajectory::RegisterSegment(CTrajectorySegment* p_seg,CHistoryNode* p_history)
{
    if( p_seg == NULL ) return;
    if( Segments.contains(p_seg) ) return;

    BeginUpdate();

    p_seg->SetIndex(GetProject()->GetFreeObjectIndex());
    Segments.append(p_seg);
    p_seg->SeqIndex = SegmentCounter.GetIndex();

    // register change to history
    if( p_history != NULL ){
        CTrajectorySegmentHI* p_item = new CTrajectorySegmentHI(p_seg,EHID_FORWARD);
        p_history->Register(p_item);
    }

    EmitOnTrajectorySegmentsChanged();
    EndUpdate();
    emit OnStatusChanged(ESC_OTHER);

    // notify list models
    GetTrajectories()->EmitOnTrajectoryListChanged();
}

//------------------------------------------------------------------------------

CSnapshotFilter* CTrajectory::CreateFilter(const CUUID& objectuuid,const QString& name,
                                           const QString& descr,CHistoryNode* p_history)
{
    // create object
    CSnapshotFilter* p_flt = static_cast<CSnapshotFilter*>(PluginDatabase.CreateObject(objectuuid,this));

    if( p_flt == NULL ) {
        CSmallString error;
        error << "unable to create snapshot filter " << objectuuid.GetStringForm();
        ES_ERROR(error);
        return(NULL);
    }

    BeginUpdate();

    Filters.append(p_flt);

    // init data
    p_flt->SetName(name);
    p_flt->SetDescription(descr);
    p_flt->SeqIndex = SegmentCounter.GetIndex();

    // register change to history
    if( p_history != NULL ){
        CSnapshotFilterHI* p_item = new CSnapshotFilterHI(p_flt,EHID_FORWARD);
        p_history->Register(p_item);
    }

    EndUpdate();
    emit OnStatusChanged(ESC_OTHER);

    // notify list models
    GetTrajectories()->EmitOnTrajectoryListChanged();

    return(p_flt);
}

//------------------------------------------------------------------------------

CSnapshotFilter* CTrajectory::CreateFilter(CXMLElement* p_ele,CHistoryNode* p_history)
{
    if( p_ele == NULL ){
        RUNTIME_ERROR("p_ele is NULL");
    }
    if( p_ele->GetName() != "filter" ){
        RUNTIME_ERROR("p_ele is not filter");
    }

    QString suuid;
    if( p_ele->GetAttribute("uuid",suuid) == false ) {
        ES_ERROR("unable to get uuid attribute");
        return(NULL);
    }
    CExtUUID  ext_uuid;
    if( ext_uuid.LoadFromString(suuid) == false ) {
        CSmallString error;
        error << "unable to decode uuid attribute " << suuid;
        ES_ERROR(error);
        return(NULL);
    }

    // create object
    CSnapshotFilter* p_flt = static_cast<CSnapshotFilter*>(PluginDatabase.CreateObject(ext_uuid,this));

    if( p_flt == NULL ) {
        CSmallString error;
        error << "unable to create snapshot filter " << ext_uuid.GetStringForm();
        ES_ERROR(error);
        return(NULL);
    }

    BeginUpdate();

    Filters.append(p_flt);

    // load data
    GetProject()->BeginLinkProcedure(0);
    p_flt->LoadData(p_ele);
    GetProject()->EndLinkProcedure();

    // register change to history
    if( p_history != NULL ){
        CHistoryItem* p_item = new CSnapshotFilterHI(p_flt,EHID_FORWARD);
        p_history->Register(p_item);
    }

    EndUpdate();
    emit OnStatusChanged(ESC_OTHER);

    // notify list models
    GetTrajectories()->EmitOnTrajectoryListChanged();

    return(p_flt);
}

//------------------------------------------------------------------------------

void CTrajectory::SetPlayMode(ETrajectoryPlayMode mode,CHistoryNode* p_history)
{
    if( PlayMode == mode ) return;

    // register change to history
    if( p_history != NULL ){
        CHistoryItem* p_item = new CTrajectorySetPlayModeHI(this,mode);
        p_history->Register(p_item);
    }

    PlayMode = mode;
    emit OnStatusChanged(ESC_OTHER);
}

//------------------------------------------------------------------------------

void CTrajectory::SetPlayTickTime(int time,CHistoryNode* p_history)
{
    if( PlayTickTime == time ) return;

    // register change to history
    if( p_history != NULL ){
        CHistoryItem* p_item = new CTrajectorySetPlayTickTimeHI(this,PlayTickTime);
        p_history->Register(p_item);
    }

    PlayTickTime = time;
    emit OnStatusChanged(ESC_OTHER);
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

void CTrajectory::SetName(const QString& name,CHistoryNode* p_history)
{
    CProObject::SetName(name,p_history);
    if( GetTrajectories() ){
        GetTrajectories()->EmitOnTrajectoryListChanged();
    }
}

//------------------------------------------------------------------------------

void CTrajectory::SetDescription(const QString& descrip,CHistoryNode* p_history)
{
    CProObject::SetDescription(descrip,p_history);
    if( GetTrajectories() ){
        GetTrajectories()->EmitOnTrajectoryListChanged();
    }
}

//------------------------------------------------------------------------------

void CTrajectory::BeginUpdate(void)
{
    if( UpdateLevel == 0 ){
        blockSignals(true);
    }
    UpdateLevel++;
}

//------------------------------------------------------------------------------

void CTrajectory::EndUpdate(void)
{
    if( UpdateLevel == 0 ) return;
    UpdateLevel--;
    if( UpdateLevel == 0 ){
        blockSignals(false);
        if( SegmentsChanged ){
            SortSegments();
            emit OnTrajectorySegmentsChanged();
        }
        if( FiltersChanged ){
            SortFilters();
            emit OnSnapshotFiltersChanged();
        }
        if( SegmentsChanged || FiltersChanged ){
            EmitOnSnapshotChanged(); // this will notify master owner too
        }
        SegmentsChanged = false;
        FiltersChanged = false;
    }
}

//------------------------------------------------------------------------------

void CTrajectory::EmitOnSnapshotChanged(void)
{
    if( GetStructure() != NULL ){
        CSnapshot* p_oldsnap = GetStructure()->GetAtoms()->GetSnapshot();
        CSnapshot* p_newsnap = GetCurrentSnapshot();
        GetStructure()->GetAtoms()->SetSnapshot(p_newsnap);
        if( p_oldsnap != p_newsnap ){
            // rebuild bonds if requested
            if( IsFlagSet(static_cast<EProObjectFlag>(EPOF_TRAJ_REBUILD_BONDS)) ){
                GetStructure()->GetBonds()->RecreateBonds();
            }
        }
    }
    emit OnSnapshotChanged();
    if( GetTrajectories() ){
        GetTrajectories()->EmitOnSnapshotChanged();
    }
    GetProject()->GetStructures()->NotifyGeometryChangeTick();
    GetProject()->RepaintProject();
}

//------------------------------------------------------------------------------

void CTrajectory::EmitOnTrajectorySegmentsChanged(void)
{
    if( UpdateLevel > 0 ){
        SegmentsChanged = true;
        return;
    }
    SortSegments();
    emit OnTrajectorySegmentsChanged();
    EmitOnSnapshotChanged();
}

//------------------------------------------------------------------------------

void CTrajectory::EmitOnSnapshotFiltersChanged(void)
{
    if( UpdateLevel > 0 ){
        FiltersChanged = true;
        return;
    }
    SortFilters();
    emit OnSnapshotFiltersChanged();
    EmitOnSnapshotChanged();
}

//------------------------------------------------------------------------------

void CTrajectory::SortSegments(void)
{
    std::sort(Segments.begin(),Segments.end(),LessThanBySeqIndexSegment);

    // update indexes
    if( CurrentSegmentIndex > Segments.count() ){
        CurrentSegmentIndex = Segments.count();
    }
    CurrentSnapshotIndex = 0;
    for(int i=1; i < CurrentSegmentIndex; i++){
        CTrajectorySegment* p_seg = GetSegment(i);
        CurrentSnapshotIndex += p_seg->GetNumberOfSnapshots();
    }
    if( CurrentSegmentIndex > 0 ){
        CTrajectorySegment* p_seg = GetSegment(CurrentSegmentIndex);
        CurrentSnapshotIndex += p_seg->GetCurrentSnapshotIndex();
    }
}

//------------------------------------------------------------------------------

bool CTrajectory::LessThanBySeqIndexSegment(CTrajectorySegment* p_left,CTrajectorySegment* p_right)
{
    return( p_left->GetSeqIndex() < p_right->GetSeqIndex() );
}

//------------------------------------------------------------------------------

void CTrajectory::SortSegmentsByID(void)
{
    std::sort(Segments.begin(),Segments.end(),LessThanByIdIndexSegment);

    // update indexes
    if( CurrentSegmentIndex > Segments.count() ){
        CurrentSegmentIndex = Segments.count();
    }
    CurrentSnapshotIndex = 0;
    for(int i=1; i < CurrentSegmentIndex; i++){
        CTrajectorySegment* p_seg = GetSegment(i);
        CurrentSnapshotIndex += p_seg->GetNumberOfSnapshots();
    }
    if( CurrentSegmentIndex > 0 ){
        CTrajectorySegment* p_seg = GetSegment(CurrentSegmentIndex);
        CurrentSnapshotIndex += p_seg->GetCurrentSnapshotIndex();
    }
}

//------------------------------------------------------------------------------

bool CTrajectory::LessThanByIdIndexSegment(CTrajectorySegment* p_left,CTrajectorySegment* p_right)
{
    return( p_left->GetIndex() < p_right->GetIndex() );
}

//------------------------------------------------------------------------------

void CTrajectory::SortFilters(void)
{
    std::sort(Filters.begin(),Filters.end(),LessThanBySeqIndexFilter);
}

//------------------------------------------------------------------------------

bool CTrajectory::LessThanBySeqIndexFilter(CSnapshotFilter* p_left,CSnapshotFilter* p_right)
{
    return( p_left->GetSeqIndex() < p_right->GetSeqIndex() );
}

//------------------------------------------------------------------------------

CTrajectorySegment* CTrajectory::GetSegment(long int segid)
{
    if( (segid < 1) || (segid > GetNumberOfSegments()) ) return(NULL);
    segid--;
    return( Segments.at(segid) );
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

void CTrajectory::LoadData(CXMLElement* p_ele)
{
    // check input data -----------------------------
    if( p_ele == NULL ) {
        INVALID_ARGUMENT("p_ele is NULL");
    }

    if( p_ele->GetName() != "trajectory" ) {
        LOGIC_ERROR("p_ele is not trajectory");
    }

    // load object info -----------------------------
    CProObject::LoadData(p_ele);

    // we need structure before loading segments and filters
    int sid = -1;
    if( p_ele->GetAttribute("sid",sid) == true ){
        CStructure* p_str = GetProject()->FindObject<CStructure*>(sid);
        if( p_str ){
            SetStructure(p_str);
        }
    }

    // load segments --------------------------------
    CXMLElement* p_sele = p_ele->GetFirstChildElement("segments");
    if( p_sele != NULL ){
        CXMLElement* p_seg_ele = p_sele->GetFirstChildElement("segment");
        while( p_seg_ele != NULL ){
            CreateSegment(p_seg_ele);
            p_seg_ele = p_seg_ele->GetNextSiblingElement("segment");
        }
    }

    // load filters --------------------------------
    CXMLElement* p_fele = p_ele->GetFirstChildElement("filters");
    if( p_fele != NULL ){
        CXMLElement* p_flt_ele = p_fele->GetFirstChildElement("filter");
        while( p_flt_ele != NULL ){
            CreateFilter(p_flt_ele);
            p_flt_ele = p_flt_ele->GetNextSiblingElement("filter");
        }
    }

    // at the end set first snapshot
    FirstSnapshot();
}

//------------------------------------------------------------------------------

void CTrajectory::SaveData(CXMLElement* p_ele)
{
    // check input data -----------------------------
    if( p_ele == NULL ) {
        INVALID_ARGUMENT("p_ele is NULL");
    }

    if( p_ele->GetName() != "trajectory" ) {
        LOGIC_ERROR("p_ele is not trajectory");
    }

    // save object info -----------------------------
    CProObject::SaveData(p_ele);

    // save structure index
    if( Structure ){
        p_ele->SetAttribute("sid",Structure->GetIndex());
    }

    // save segments
    CXMLElement* p_sele = p_ele->CreateChildElement("segments");

    foreach(CTrajectorySegment* p_seg, Segments){
        CXMLElement* p_seg_ele = p_sele->CreateChildElement("segment");
        p_seg->SaveData(p_seg_ele);
    }

    // save filters
    CXMLElement* p_fele = p_ele->CreateChildElement("filters");

    foreach(CSnapshotFilter* p_flt, Filters){
        CXMLElement* p_flt_ele = p_fele->CreateChildElement("filter");
        p_flt->SaveData(p_flt_ele);
    }

}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

