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
#include <TrajectoryModelSegments.hpp>
#include <CategoryUUID.hpp>
#include <Trajectory.hpp>
#include <TrajectorySegment.hpp>
#include <Project.hpp>

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

QObject* TrajectoryModelSegmentsCB(void* p_data);

//------------------------------------------------------------------------------

CExtUUID        TrajectoryModelSegmentsID(
                    "{TRAJECTORY_MODEL_SEGMENTS:2db1471f-56e9-4587-b395-bddcac7ff225}",
                    "Segments");

CPluginObject   TrajectoryModelSegmentsObject(&NemesisCorePlugin,
                    TrajectoryModelSegmentsID,MODEL_CAT,
                    ":/images/NemesisCore/trajectory/Trajectory.svg",
                    TrajectoryModelSegmentsCB);

// -----------------------------------------------------------------------------

QObject* TrajectoryModelSegmentsCB(void* p_data)
{
    CTrajectory* p_con = static_cast<CTrajectory*>(p_data);
    if( p_con == NULL ) return(NULL);

    CTrajectoryModelSegments* p_obj = new CTrajectoryModelSegments(NULL);
    p_obj->SetRootObject(p_con);

    return(p_obj);
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

CTrajectoryModelSegments::CTrajectoryModelSegments(QObject* p_parent)
    : CContainerModel(&TrajectoryModelSegmentsObject,p_parent)
{
    RootObject = NULL;
}

//------------------------------------------------------------------------------

void CTrajectoryModelSegments::SetRootObject(CTrajectory* p_data)
{
    if( RootObject != NULL ) {
        RootObject->disconnect(this);
    }

    RootObject = p_data;
    if( RootObject != NULL ) {
        connect(RootObject,SIGNAL(destroyed(QObject*)),
                this,SLOT(RootObjectDeleted(void)));

        connect(RootObject,SIGNAL(OnTrajectorySegmentsChanged()),
                this,SLOT(ListChanged()));
    }
    // FIXME
    beginResetModel();
    endResetModel();
}

//------------------------------------------------------------------------------

QObject*  CTrajectoryModelSegments::GetItem(const QModelIndex& index) const
{
    if( ! index.isValid() ) return(NULL);
    CTrajectorySegment* p_obj = static_cast<CTrajectorySegment*>(index.internalPointer());
    return(p_obj);
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

QVariant CTrajectoryModelSegments::headerData(int section, Qt::Orientation orientation,
                                    int role) const
{
    if( orientation != Qt::Horizontal ) return( QVariant() );
    if( role == Qt::TextAlignmentRole ) return(Qt::AlignHCenter);
    if( role != Qt::DisplayRole ) return( QVariant() );

    switch(section) {
        case 0:
            return(tr("SI"));
        case 1:
            return(tr("Name"));
        case 2:
            return(tr("Snapshots"));
        case 3:
            return(tr("Type"));
        case 4:
            return(tr("Path"));
        case 5:
            return(tr("ID"));
        case 6:
            return(tr("Description"));
        default:
            return("");
    }
}

//------------------------------------------------------------------------------

int CTrajectoryModelSegments::columnCount(const QModelIndex &parent) const
{
    return(7);
}

//------------------------------------------------------------------------------

int CTrajectoryModelSegments::rowCount(const QModelIndex &parent) const
{
    if( RootObject == 0 ) return(0);
    return( RootObject->Segments.count() );
}

//------------------------------------------------------------------------------

QModelIndex CTrajectoryModelSegments::index(int row, int column,
                                  const QModelIndex &parent) const
{
    if( RootObject == NULL ) return( QModelIndex() );

    if( ! hasIndex(row, column, parent) ) return( QModelIndex() );
    if( parent.isValid() )  return( QModelIndex() );

    QModelIndex index = createIndex(row, column, RootObject->Segments[row]);

    return(index);
}

//------------------------------------------------------------------------------

QModelIndex CTrajectoryModelSegments::parent(const QModelIndex &index) const
{
    // parent is always invalid
    return( QModelIndex() );
}

//------------------------------------------------------------------------------

QVariant CTrajectoryModelSegments::data(const QModelIndex &index, int role) const
{
    if( RootObject == NULL ) return( QVariant() );
    if( ! index.isValid() ) return( QVariant() );

    CTrajectorySegment* p_obj = static_cast<CTrajectorySegment*>(index.internalPointer());

    switch(role) {
        case Qt::DisplayRole: {
            switch( index.column() ) {
                case 0:
                    return(p_obj->GetSeqIndex());
                case 1:
                    return(p_obj->GetName());
                case 2:
                    return((qlonglong)p_obj->GetNumberOfSnapshots());
                case 3:
                    return(p_obj->GetType().GetName());
                case 4:
                    return(p_obj->GetFileName());
                case 5:
                    return(p_obj->GetIndex());
                case 6:
                    return(p_obj->GetDescription());

                default:
                    return( QVariant() );
            }
        }
        break;

        case Qt::TextAlignmentRole:
            switch( index.column() ) {
                case 0:
                    return(Qt::AlignRight);
                case 1:
                    return(Qt::AlignLeft);
                case 2:
                    return(Qt::AlignRight);
                case 3:
                    return(Qt::AlignLeft);
                case 4:
                    return(Qt::AlignLeft);
                case 5:
                    return(Qt::AlignRight);
                case 6:
                    return(Qt::AlignLeft);
                default:
                    return( QVariant() );
            }
        break;
    }

    return( QVariant() );
}

//------------------------------------------------------------------------------

Qt::ItemFlags CTrajectoryModelSegments::flags(const QModelIndex &index) const
{
    if( !index.isValid() ) return(Qt::ItemFlags());
    return(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

void CTrajectoryModelSegments::RootObjectDeleted(void)
{
    RootObject = NULL;
    // FIXME
    beginResetModel();
    endResetModel();
}

//------------------------------------------------------------------------------

void CTrajectoryModelSegments::ListChanged(void)
{
    // FIXME
    beginResetModel();
    endResetModel();
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================


