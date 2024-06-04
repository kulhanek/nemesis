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
#include <TrajectoryModelProperties.hpp>
#include <CategoryUUID.hpp>
#include <Trajectory.hpp>
#include <TrajectorySegment.hpp>
#include <Project.hpp>

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

QObject* TrajectoryModelCB(void* p_data);

//------------------------------------------------------------------------------

CExtUUID        TrajectoryModelID(
                    "{TRAJECTORY_MODEL:4e05dfc0-4733-4e62-a62b-9bc85296c081}",
                    "Segments");

CPluginObject   TrajectoryModelObject(&NemesisCorePlugin,
                    TrajectoryModelID,MODEL_CAT,
                    ":/images/NemesisCore/trajectory/Trajectory.svg",
                    TrajectoryModelCB);

// -----------------------------------------------------------------------------

QObject* TrajectoryModelCB(void* p_data)
{
    CTrajectory* p_con = static_cast<CTrajectory*>(p_data);
    if( p_con == NULL ) return(NULL);

    CTrajectoryModel* p_obj = new CTrajectoryModel(NULL);
    p_obj->SetRootObject(p_con);

    return(p_obj);
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

CTrajectoryModel::CTrajectoryModel(QObject* p_parent)
    : CContainerModel(&TrajectoryModelObject,p_parent)
{
    RootObject = NULL;
}

//------------------------------------------------------------------------------

void CTrajectoryModel::SetRootObject(CTrajectory* p_data)
{
    if( RootObject != NULL ) {
        RootObject->disconnect(this);
    }

    RootObject = p_data;
    if( RootObject != NULL ) {
        connect(RootObject,SIGNAL(destroyed(QObject*)),
                this,SLOT(RootObjectDeleted(void)));

        connect(RootObject,SIGNAL(OnTrajectoryChanged()),
                this,SLOT(ListChanged()));
    }
    // FIXME
    beginResetModel();
    endResetModel();
}

//------------------------------------------------------------------------------

QObject*  CTrajectoryModel::GetItem(const QModelIndex& index) const
{
    if( ! index.isValid() ) return(NULL);
    CTrajectorySegment* p_obj = static_cast<CTrajectorySegment*>(index.internalPointer());
    return(p_obj);
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

QVariant CTrajectoryModel::headerData(int section, Qt::Orientation orientation,
                                    int role) const
{
    if( orientation != Qt::Horizontal ) return( QVariant() );
    if( role == Qt::TextAlignmentRole ) return(Qt::AlignHCenter);
    if( role != Qt::DisplayRole ) return( QVariant() );

    switch(section) {
        case 0:
            return(tr("Name"));
        case 1:
            return(tr("Snapshots"));
        case 2:
            return(tr("Type"));
        case 3:
            return(tr("Description"));
        case 4:
            return(tr("Path"));
        default:
            return("");
    }
}

//------------------------------------------------------------------------------

int CTrajectoryModel::columnCount(const QModelIndex &parent) const
{
    return(5);
}

//------------------------------------------------------------------------------

int CTrajectoryModel::rowCount(const QModelIndex &parent) const
{
    if( RootObject == 0 ) return(0);
    return( RootObject->children().count() );
}

//------------------------------------------------------------------------------

QModelIndex CTrajectoryModel::index(int row, int column,
                                  const QModelIndex &parent) const
{
    if( RootObject == NULL ) return( QModelIndex() );

    if( ! hasIndex(row, column, parent) ) return( QModelIndex() );
    if( parent.isValid() )  return( QModelIndex() );

    QModelIndex index = createIndex(row, column, RootObject->children()[row]);

    return(index);
}

//------------------------------------------------------------------------------

QModelIndex CTrajectoryModel::parent(const QModelIndex &index) const
{
    // parent is always invalid
    return( QModelIndex() );
}

//------------------------------------------------------------------------------

QVariant CTrajectoryModel::data(const QModelIndex &index, int role) const
{
    if( RootObject == NULL ) return( QVariant() );
    if( ! index.isValid() ) return( QVariant() );

    CTrajectorySegment* p_obj = static_cast<CTrajectorySegment*>(index.internalPointer());

    switch(role) {
        case Qt::DisplayRole: {
            switch( index.column() ) {
                case 0:
                    return(p_obj->GetName());
                case 1:
                    return( (qlonglong) p_obj->GetNumberOfSnapshots() );
                case 2:
                    return(p_obj->GetType().GetName());
                case 3:
                    return(p_obj->GetDescription());
                case 4:
                    return(p_obj->GetFileName());
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
                    return(Qt::AlignLeft);
                case 3:
                    return(Qt::AlignLeft);
                case 4:
                    return(Qt::AlignLeft);
                default:
                    return( QVariant() );
            }
        break;

        case Qt::DecorationRole: {
            switch( index.column() ) {
                case 0:{
                    if( RootObject->IsSegmentActive(p_obj) ){
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

Qt::ItemFlags CTrajectoryModel::flags(const QModelIndex &index) const
{
    if( !index.isValid() ) return(Qt::ItemFlags());
    return(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

void CTrajectoryModel::RootObjectDeleted(void)
{
    RootObject = NULL;
    // FIXME
    beginResetModel();
    endResetModel();
}

//------------------------------------------------------------------------------

void CTrajectoryModel::ListChanged(void)
{
    // FIXME
    beginResetModel();
    endResetModel();
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================


