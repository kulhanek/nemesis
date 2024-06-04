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
#include <TrajectoryModelFilters.hpp>
#include <CategoryUUID.hpp>
#include <Trajectory.hpp>
#include <TrajectorySegment.hpp>
#include <Project.hpp>

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

QObject* TrajectoryModelFiltersCB(void* p_data);

//------------------------------------------------------------------------------

CExtUUID        TrajectoryModelFiltersID(
                    "{TRAJECTORY_MODEL_FILTERS:e5244fe5-9cb0-4005-9360-d308904fd467}",
                    "Filters");

CPluginObject   TrajectoryModelFiltersObject(&NemesisCorePlugin,
                    TrajectoryModelFiltersID,MODEL_CAT,
                    ":/images/NemesisCore/trajectory/Trajectory.svg",
                    TrajectoryModelFiltersCB);

// -----------------------------------------------------------------------------

QObject* TrajectoryModelFiltersCB(void* p_data)
{
    CTrajectory* p_con = static_cast<CTrajectory*>(p_data);
    if( p_con == NULL ) return(NULL);

    CTrajectoryModelFilters* p_obj = new CTrajectoryModelFilters(NULL);
    p_obj->SetRootObject(p_con);

    return(p_obj);
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

CTrajectoryModelFilters::CTrajectoryModelFilters(QObject* p_parent)
    : CContainerModel(&TrajectoryModelFiltersObject,p_parent)
{
    RootObject = NULL;
}

//------------------------------------------------------------------------------

void CTrajectoryModelFilters::SetRootObject(CTrajectory* p_data)
{
    if( RootObject != NULL ) {
        RootObject->disconnect(this);
    }

    RootObject = p_data;
    if( RootObject != NULL ) {
        connect(RootObject,SIGNAL(destroyed(QObject*)),
                this,SLOT(RootObjectDeleted(void)));

        connect(RootObject,SIGNAL(OnSnapshotFiltersChanged()),
                this,SLOT(ListChanged()));
    }
    // FIXME
    beginResetModel();
    endResetModel();
}

//------------------------------------------------------------------------------

QObject*  CTrajectoryModelFilters::GetItem(const QModelIndex& index) const
{
    if( ! index.isValid() ) return(NULL);
    CTrajectorySegment* p_obj = static_cast<CTrajectorySegment*>(index.internalPointer());
    return(p_obj);
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

QVariant CTrajectoryModelFilters::headerData(int section, Qt::Orientation orientation,
                                    int role) const
{
    if( orientation != Qt::Horizontal ) return( QVariant() );
    if( role == Qt::TextAlignmentRole ) return(Qt::AlignHCenter);
    if( role != Qt::DisplayRole ) return( QVariant() );

    switch(section) {
        case 0:
            return(tr("Name"));
        case 1:
            return(tr("Type"));
        case 2:
            return(tr("Description"));
        case 3:
            return(tr("ID"));
        default:
            return("");
    }
}

//------------------------------------------------------------------------------

int CTrajectoryModelFilters::columnCount(const QModelIndex &parent) const
{
    return(4);
}

//------------------------------------------------------------------------------

int CTrajectoryModelFilters::rowCount(const QModelIndex &parent) const
{
    if( RootObject == 0 ) return(0);
    return( RootObject->Filters.count() );
}

//------------------------------------------------------------------------------

QModelIndex CTrajectoryModelFilters::index(int row, int column,
                                  const QModelIndex &parent) const
{
    if( RootObject == NULL ) return( QModelIndex() );

    if( ! hasIndex(row, column, parent) ) return( QModelIndex() );
    if( parent.isValid() )  return( QModelIndex() );

    QModelIndex index = createIndex(row, column, RootObject->Filters[row]);

    return(index);
}

//------------------------------------------------------------------------------

QModelIndex CTrajectoryModelFilters::parent(const QModelIndex &index) const
{
    // parent is always invalid
    return( QModelIndex() );
}

//------------------------------------------------------------------------------

QVariant CTrajectoryModelFilters::data(const QModelIndex &index, int role) const
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
                    return(p_obj->GetType().GetName());
                case 2:
                    return(p_obj->GetIndex());
                case 3:
                    return(p_obj->GetDescription());
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
                    return(Qt::AlignLeft);
                case 2:
                    return(Qt::AlignRight);
                case 3:
                    return(Qt::AlignLeft);
                default:
                    return( QVariant() );
            }
        break;
    }

    return( QVariant() );
}

//------------------------------------------------------------------------------

Qt::ItemFlags CTrajectoryModelFilters::flags(const QModelIndex &index) const
{
    if( !index.isValid() ) return(Qt::ItemFlags());
    return(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

void CTrajectoryModelFilters::RootObjectDeleted(void)
{
    RootObject = NULL;
    // FIXME
    beginResetModel();
    endResetModel();
}

//------------------------------------------------------------------------------

void CTrajectoryModelFilters::ListChanged(void)
{
    // FIXME
    beginResetModel();
    endResetModel();
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================


