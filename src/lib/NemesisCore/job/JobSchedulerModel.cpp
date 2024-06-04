// =============================================================================
// NEMESIS - Molecular Modelling Package
// -----------------------------------------------------------------------------
//    Copyright (C) 2010        Petr Kulhanek, kulhanek@chemi.muni.cz
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

#include <JobSchedulerModel.hpp>
#include <NemesisCoreModule.hpp>
#include <CategoryUUID.hpp>
#include <JobScheduler.hpp>
#include <Job.hpp>

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

QObject* JobSchedulerModelCB(void* p_data);

CExtUUID        JobSchedulerModelID(
                    "{JOB_SCHEDULER_MODEL:aa68e43e-548a-448d-a7fd-f544c91c93de}",
                    "Jobs");

CPluginObject   JobSchedulerModelObject(&NemesisCorePlugin,
                    JobSchedulerModelID,MODEL_CAT,
                    ":/images/NemesisCore/job/JobList.svg",
                    JobSchedulerModelCB);

//------------------------------------------------------------------------------

QObject* JobSchedulerModelCB(void* p_data)
{
    CJobScheduler* p_con = static_cast<CJobScheduler*>(p_data);
    if( p_con == NULL ) return(NULL);

    CJobSchedulerModel* p_obj = new CJobSchedulerModel(NULL);
    p_obj->SetRootObject(p_con);

    return(p_obj);
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

CJobSchedulerModel::CJobSchedulerModel(QObject* p_parent)
    : CContainerModel(&JobSchedulerModelObject,p_parent)
{
    RootObject = NULL;
}

//------------------------------------------------------------------------------

void CJobSchedulerModel::SetRootObject(CJobScheduler* p_data)
{
    if( RootObject != NULL ) {
        RootObject->disconnect(this);
    }

    RootObject = p_data;

    if( RootObject != NULL ) {
        connect(RootObject,SIGNAL(destroyed(QObject*)),
                this,SLOT(RootObjectDeleted(void)));

        connect(RootObject,SIGNAL(OnJobAdded(CJob*)),
                this,SLOT(JobSchedulerUpdated(void)));
        connect(RootObject,SIGNAL(OnJobStatusChanged(CJob*)),
                this,SLOT(JobSchedulerUpdated(void)));
        connect(RootObject,SIGNAL(OnJobRemoved(CJob*)),
                this,SLOT(JobSchedulerUpdated(void)));
    }

    // FIXME
    beginResetModel();
    endResetModel();
}

//------------------------------------------------------------------------------

QObject* CJobSchedulerModel::GetItem(const QModelIndex& index) const
{
    if( ! index.isValid() ) return(NULL);
    return( static_cast<QObject*>(index.internalPointer()));
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

QVariant CJobSchedulerModel::headerData(int section, Qt::Orientation orientation,
                                        int role) const
{
    if( role != Qt::DisplayRole ) return( QVariant() );
    if( orientation != Qt::Horizontal ) return( QVariant() );

    switch(section) {
    case 0:
        return(tr("ST"));
    case 1:
        return(tr("Name"));
    case 2:
        return(tr("Description"));
    }

    return( QVariant() );
}

//------------------------------------------------------------------------------

int CJobSchedulerModel::columnCount(const QModelIndex &parent) const
{
    return(3);
}

//------------------------------------------------------------------------------

int CJobSchedulerModel::rowCount(const QModelIndex &parent) const
{
    if( RootObject == NULL ) return(0);
    int count = RootObject->Jobs.NumOfMembers();
    return(count);
}

//------------------------------------------------------------------------------

QModelIndex CJobSchedulerModel::index(int row, int column,
                                      const QModelIndex &parent) const
{
    if( RootObject == NULL ) return( QModelIndex() );

    if( ! hasIndex(row, column, parent) ) return( QModelIndex() );
    if( parent.isValid() )  return( QModelIndex() );

    QObject* p_obj = RootObject->Jobs[row];
    QModelIndex index = createIndex(row, column, p_obj);

    return(index);
}

//------------------------------------------------------------------------------

QModelIndex CJobSchedulerModel::parent(const QModelIndex &index) const
{
    // parent is always invalid
    return( QModelIndex() );
}

//------------------------------------------------------------------------------

QVariant CJobSchedulerModel::data(const QModelIndex &index, int role) const
{
    if( RootObject == 0 ) return( QVariant() );
    if( ! index.isValid() ) return( QVariant() );

    switch(role) {
    case Qt::DisplayRole: {
        CJob* p_item = static_cast<CJob*>(index.internalPointer());
        switch(index.column()) {
        case 0:
            return(p_item->GetJobStatusString());
        case 1:
            return(p_item->GetName());
        case 2:
            return(p_item->GetDescription());
        }
    }

    case Qt::DecorationRole: {
        QObject* p_item = static_cast<QObject*>(index.internalPointer());
        CComObject* p_com = dynamic_cast<CComObject*>(p_item);
        if( p_com == NULL ) return( QVariant() );
        switch(index.column()) {
        case 0:
            return(p_com->GetPluginObject()->GetIcon());
        }
        return( QVariant() );
    }
    }

    return( QVariant() );
}

//------------------------------------------------------------------------------

Qt::ItemFlags CJobSchedulerModel::flags(const QModelIndex &index) const
{
    if( !index.isValid() ) return(Qt::ItemFlags());
    return(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

void CJobSchedulerModel::RootObjectDeleted(void)
{
    RootObject = NULL;
    // FIXME
    beginResetModel();
    endResetModel();
}

//------------------------------------------------------------------------------

void CJobSchedulerModel::JobSchedulerUpdated(void)
{
    // FIXME
    beginResetModel();
    endResetModel();
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================



