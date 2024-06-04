// =============================================================================
// NEMESIS - Molecular Modelling Package
// -----------------------------------------------------------------------------
//    Copyright (C) 2014        Petr Kulhanek, kulhanek@chemi.muni.cz
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

#include <BatchJobListModel.hpp>
#include <NemesisCoreModule.hpp>
#include <CategoryUUID.hpp>
#include <BatchJob.hpp>
#include <BatchJobList.hpp>

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

QObject* BatchJobListModelCB(void* p_data);

CExtUUID        BatchJobListModelID(
                    "{BATCH_JOB_LIST_MODEL:4a19da13-8a37-4b69-924e-ac645e5a61e2}",
                    "Batch Job list model");

CPluginObject   BatchJobListModelObject(&NemesisCorePlugin,
                    BatchJobListModelID,MODEL_CAT,
                    ":/images/NemesisCore/batchjob/BatchJobList.svg",
                    BatchJobListModelCB);

//------------------------------------------------------------------------------

QObject* BatchJobListModelCB(void* p_data)
{
    CBatchJobList* p_con = static_cast<CBatchJobList*>(p_data);
    if( p_con == NULL ) return(NULL);

    CBatchJobListModel* p_obj = new CBatchJobListModel(NULL);
    p_obj->SetRootObject(p_con);

    return(p_obj);
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

CBatchJobListModel::CBatchJobListModel(QObject* p_parent)
    : CContainerModel(&BatchJobListModelObject,p_parent)
{
    RootObject = NULL;
}

//------------------------------------------------------------------------------

void CBatchJobListModel::SetRootObject(CBatchJobList* p_data)
{
    if( RootObject != NULL ) {
        RootObject->disconnect(this);
    }

    RootObject = p_data;

    if( RootObject != NULL ) {
        connect(RootObject,SIGNAL(destroyed(QObject*)),
                this,SLOT(RootObjectDeleted(void)));
        connect(RootObject,SIGNAL(OnBatchJobAdded(CBatchJob*)),
                this,SLOT(BatchJobListUpdated(void)));
        connect(RootObject,SIGNAL(OnBatchJobStatusChanged(CBatchJob*)),
                this,SLOT(BatchJobListUpdated(void)));
        connect(RootObject,SIGNAL(OnBatchJobRemoved(CBatchJob*)),
                this,SLOT(BatchJobListUpdated(void)));
    }

    // FIXME
    beginResetModel();
    endResetModel();
}

//------------------------------------------------------------------------------

QObject* CBatchJobListModel::GetItem(const QModelIndex& index) const
{
    if( ! index.isValid() ) return(NULL);
    return( static_cast<QObject*>(index.internalPointer()));
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

QVariant CBatchJobListModel::headerData(int section, Qt::Orientation orientation,
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

int CBatchJobListModel::columnCount(const QModelIndex &parent) const
{
    return(3);
}

//------------------------------------------------------------------------------

int CBatchJobListModel::rowCount(const QModelIndex &parent) const
{
    if( RootObject == 0 ) return(0);
    return( RootObject->children().count() );
}

//------------------------------------------------------------------------------

QModelIndex CBatchJobListModel::index(int row, int column,
                                 const QModelIndex &parent) const
{
    if( RootObject == NULL ) return( QModelIndex() );

    if( ! hasIndex(row, column, parent) ) return( QModelIndex() );
    if( parent.isValid() )  return( QModelIndex() );

    const QObjectList& list = RootObject->children();
    QModelIndex index = createIndex(row, column, list[row]);

    return(index);
}

//------------------------------------------------------------------------------

QModelIndex CBatchJobListModel::parent(const QModelIndex &index) const
{
    // parent is always invalid
    return( QModelIndex() );
}

//------------------------------------------------------------------------------

QVariant CBatchJobListModel::data(const QModelIndex &index, int role) const
{
    if( RootObject == 0 ) return( QVariant() );
    if( ! index.isValid() ) return( QVariant() );

    switch(role) {
    case Qt::DisplayRole: {
        CBatchJob* p_item = static_cast<CBatchJob*>(index.internalPointer());
        switch(index.column()) {
        case 0:
            return(p_item->GetBatchJobStatusString());
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

Qt::ItemFlags CBatchJobListModel::flags(const QModelIndex &index) const
{
    if( !index.isValid() ) return(Qt::ItemFlags());
    return(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

void CBatchJobListModel::RootObjectDeleted(void)
{
    RootObject = NULL;
    // FIXME
    beginResetModel();
    endResetModel();
}

//------------------------------------------------------------------------------

void CBatchJobListModel::BatchJobListUpdated(void)
{
    // FIXME
    beginResetModel();
    endResetModel();
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================



