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

#include <NemesisCoreModule.hpp>
#include <TrajectoryListModel.hpp>
#include <CategoryUUID.hpp>
#include <TrajectoryList.hpp>
#include <Trajectory.hpp>
#include <Structure.hpp>
#include <AtomList.hpp>
#include <QMimeData>

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

QObject* TrajectoryListModelCB(void* p_data);

//------------------------------------------------------------------------------

CExtUUID        TrajectoryListModelID(
                    "{TRAJECTORY_LIST_MODEL:9ea0a9e4-38b1-48f4-ae04-addabcdd149e}",
                    "Trajectories");

CPluginObject   TrajectoryListModelObject(&NemesisCorePlugin,
                    TrajectoryListModelID,MODEL_CAT,
                    ":/images/NemesisCore/trajectory/TrajectoryList.svg",
                    TrajectoryListModelCB);

// -----------------------------------------------------------------------------

QObject* TrajectoryListModelCB(void* p_data)
{
    CTrajectoryList* p_con = static_cast<CTrajectoryList*>(p_data);
    if( p_con == NULL ) return(NULL);

    CTrajectoryListModel* p_obj = new CTrajectoryListModel(NULL);
    p_obj->SetRootObject(p_con);

    return(p_obj);
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

CTrajectoryListModel::CTrajectoryListModel(QObject* p_parent)
    : CContainerModel(&TrajectoryListModelObject,p_parent)
{
    RootObject = NULL;
}

//------------------------------------------------------------------------------

void CTrajectoryListModel::SetRootObject(CTrajectoryList* p_data)
{
    if( RootObject != NULL ) {
        RootObject->disconnect(this);
    }

    RootObject = p_data;
    if( RootObject != NULL ) {
        connect(RootObject,SIGNAL(destroyed(QObject*)),
                this,SLOT(RootObjectDeleted(void)));

        connect(RootObject,SIGNAL(OnTrajectoryListChanged(void)),
                this,SLOT(ListChanged()));
    }
    // FIXME
    beginResetModel();
    endResetModel();
}

//------------------------------------------------------------------------------

QObject*  CTrajectoryListModel::GetItem(const QModelIndex& index) const
{
    if( ! index.isValid() ) return(NULL);
    CTrajectory* p_obj = static_cast<CTrajectory*>(index.internalPointer());
    return(p_obj);
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

QVariant CTrajectoryListModel::headerData(int section, Qt::Orientation orientation,
                                    int role) const
{
    if( orientation != Qt::Horizontal ) return( QVariant() );
    if( role == Qt::TextAlignmentRole ) return(Qt::AlignHCenter);
    if( role != Qt::DisplayRole ) return( QVariant() );

    switch(section) {
    case 0:
        return(tr("Name"));
    case 1:
        return(tr("Structure"));
    case 2:
        return(tr("Atoms"));
    case 3:
        return(tr("Description"));
    case 4:
        return(tr("ID"));
    default:
        return("");
    }
}

//------------------------------------------------------------------------------

int CTrajectoryListModel::columnCount(const QModelIndex &parent) const
{
    return(5);
}

//------------------------------------------------------------------------------

int CTrajectoryListModel::rowCount(const QModelIndex &parent) const
{
    if( RootObject == 0 ) return(0);
    return( RootObject->children().count() );
}

//------------------------------------------------------------------------------

QModelIndex CTrajectoryListModel::index(int row, int column,
                                  const QModelIndex &parent) const
{
    if( RootObject == NULL ) return( QModelIndex() );

    if( ! hasIndex(row, column, parent) ) return( QModelIndex() );
    if( parent.isValid() )  return( QModelIndex() );

    QModelIndex index = createIndex(row, column, RootObject->children()[row]);

    return(index);
}

//------------------------------------------------------------------------------

QModelIndex CTrajectoryListModel::parent(const QModelIndex &index) const
{
    // parent is always invalid
    return( QModelIndex() );
}

//------------------------------------------------------------------------------

QVariant CTrajectoryListModel::data(const QModelIndex &index, int role) const
{
    if( RootObject == NULL ) return( QVariant() );
    if( ! index.isValid() ) return( QVariant() );

    CTrajectory* p_obj = static_cast<CTrajectory*>(index.internalPointer());

    switch(role) {
        case Qt::DisplayRole: {
            switch( index.column() ) {
                case 0:
                    return(p_obj->GetName());
                case 1:
                    if( p_obj->GetStructure() != NULL ){
                        return(p_obj->GetStructure()->GetName());
                    }
                    return( QVariant() );
                case 2:
                    if( p_obj->GetStructure() != NULL ){
                        return(p_obj->GetStructure()->GetAtoms()->GetNumberOfAtoms());
                    }
                    return( QVariant() );
                case 3:
                    return(p_obj->GetDescription());
                case 4:
                    return(p_obj->GetIndex());
                default:
                    return( QVariant() );
            }
        }
        break;

        case Qt::TextAlignmentRole:
            switch( index.column() ) {
                case 0:
                    return(Qt::AlignLeft);
                case 1:
                    return(Qt::AlignRight);
                case 2:
                    return(Qt::AlignRight);
                case 3:
                    return(Qt::AlignLeft);
                case 4:
                    return(Qt::AlignRight);
                default:
                    return( QVariant() );
            }
        break;

        case Qt::DecorationRole: {
            switch( index.column() ) {
                case 0:{
                    if( RootObject->GetActiveTrajectory() == p_obj ){
                        return(QIcon(":/images/NemesisCore/models/active.svg"));
                    } else {
                        return(QIcon(":/images/NemesisCore/models/notactive.svg"));
                    }
                }
                break;
                default:
                    return( QVariant() );
            }
        }
        break;
    }

    return( QVariant() );
}

//------------------------------------------------------------------------------

Qt::ItemFlags CTrajectoryListModel::flags(const QModelIndex &index) const
{
    if( !index.isValid() ) return(Qt::ItemFlags());
    return(Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsDragEnabled);
}

//------------------------------------------------------------------------------

QStringList CTrajectoryListModel::mimeTypes(void) const
{
    QStringList types;
    types << RootObject->GetProjectMIMEType("trajectory.indexes");
    return types;
}

//------------------------------------------------------------------------------

QMimeData* CTrajectoryListModel::mimeData(const QModelIndexList &indexes) const
{
    QMimeData* p_mimeData = new QMimeData();
    QByteArray encodedData;

    QDataStream stream(&encodedData, QIODevice::WriteOnly);

    foreach(const QModelIndex &index, indexes) {
        if( index.column() != 0 ) continue;
        CProObject* p_object = dynamic_cast<CProObject*>(GetItem(index));
        if( p_object ){
             stream << p_object->GetIndex();
         }
    }

    p_mimeData->setData(RootObject->GetProjectMIMEType("trajectory.indexes"), encodedData);
    return(p_mimeData);
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

void CTrajectoryListModel::RootObjectDeleted(void)
{
    RootObject = NULL;
    // FIXME
    beginResetModel();
    endResetModel();
}

//------------------------------------------------------------------------------

void CTrajectoryListModel::ListChanged(void)
{
    // FIXME
    beginResetModel();
    endResetModel();
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================


