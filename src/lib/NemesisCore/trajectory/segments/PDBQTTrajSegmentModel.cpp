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

#include <iostream>

#include <NemesisCoreModule.hpp>
#include <CategoryUUID.hpp>
#include <Project.hpp>
#include <PhysicalQuantity.hpp>
#include <PhysicalQuantities.hpp>

#include <Trajectory.hpp>
#include <TrajectorySegment.hpp>

#include <PDBQTTrajSegment.hpp>
#include <PDBQTTrajSegmentModel.hpp>

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

QObject* PDBQTTrajSegmentModelCB(void* p_data);

//------------------------------------------------------------------------------

CExtUUID        PDBQTTrajSegmentModelID(
                    "{PDBQT_TRAJ_SEGMENT_MODEL:ad3f4620-9896-4bf3-8266-0f8bba545eb7}",
                    "PDBQT Traj Segments");

CPluginObject   PDBQTTrajSegmentModelObject(&NemesisCorePlugin,
                    PDBQTTrajSegmentModelID,MODEL_CAT,
                    ":/images/NemesisCore/trajectory/Trajectory.svg",
                    PDBQTTrajSegmentModelCB);

// -----------------------------------------------------------------------------

QObject* PDBQTTrajSegmentModelCB(void* p_data)
{
    CPDBQTTrajSegment* p_con = static_cast<CPDBQTTrajSegment*>(p_data);
    if( p_con == NULL ) return(NULL);

    CPDBQTTrajSegmentModel* p_obj = new CPDBQTTrajSegmentModel(NULL);
    p_obj->SetRootObject(p_con);

    return(p_obj);
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

CPDBQTTrajSegmentModel::CPDBQTTrajSegmentModel(QObject* p_parent)
    : CContainerModel(&PDBQTTrajSegmentModelObject,p_parent)
{
    RootObject = NULL;
}

//------------------------------------------------------------------------------

void CPDBQTTrajSegmentModel::SetRootObject(CPDBQTTrajSegment *p_data)
{
    if( RootObject != NULL ) {
        RootObject->disconnect(this);
    }

    RootObject = p_data;
    if( RootObject != NULL ) {
        connect(RootObject,SIGNAL(destroyed(QObject*)),
                this,SLOT(RootObjectDeleted(void)));

        connect(RootObject->GetTrajectory(),SIGNAL(OnSnapshotChanged(void)),
                this,SLOT(ListChanged()));
    }
        // FIXME
    beginResetModel();
    endResetModel();
}

//------------------------------------------------------------------------------

QObject* CPDBQTTrajSegmentModel::GetItem(const QModelIndex& index) const
{
    if( ! index.isValid() ) return(NULL);
    CPDBQTTrajSegment* p_obj = static_cast<CPDBQTTrajSegment*>(index.internalPointer());
    return (p_obj);
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

QVariant CPDBQTTrajSegmentModel::headerData(int section, Qt::Orientation orientation,
                                            int role) const
{
    if( orientation != Qt::Horizontal ) return( QVariant() );
    if( role == Qt::TextAlignmentRole ) return(Qt::AlignHCenter);
    if( role != Qt::DisplayRole ) return( QVariant() );

    switch(section) {
    case 0:
        return(tr("Pose"));
    case 1:{
        QString col = tr("E [%1]");
        col = col.arg(PQ_ENERGY->GetUnitName());
        return(col);
    }
    case 2:{
        QString col = tr("RMSD low [%1]");
        col = col.arg(PQ_DISTANCE->GetUnitName());
        return(col);
    }
    case 3:{
        QString col = tr("RMSD up [%1]");
        col = col.arg(PQ_DISTANCE->GetUnitName());
        return(col);
    }
    default:
        return("");
}
}

//------------------------------------------------------------------------------

int CPDBQTTrajSegmentModel::columnCount(const QModelIndex &parent) const
{
    return(4);
}

//------------------------------------------------------------------------------

int CPDBQTTrajSegmentModel::rowCount(const QModelIndex &parent) const
{
    if( RootObject == 0 ) return(0);
    return( RootObject->Snapshots.count() );
}

//------------------------------------------------------------------------------

QModelIndex CPDBQTTrajSegmentModel::index(int row, int column,
                                  const QModelIndex &parent) const
{
    if( RootObject == NULL ) return( QModelIndex() );

    if( ! hasIndex(row, column, parent) ) return( QModelIndex() );
    if( parent.isValid() )  return( QModelIndex() );
    // snapshots are numered from 0 id QList and row is from 0 too
    QModelIndex index = createIndex(row, column, RootObject->Snapshots[row]);
    return(index);
}

//------------------------------------------------------------------------------

QModelIndex CPDBQTTrajSegmentModel::parent(const QModelIndex &index) const
{
    // parent is always invalid
    return( QModelIndex() );
}

//------------------------------------------------------------------------------

QVariant CPDBQTTrajSegmentModel::data(const QModelIndex &index, int role) const
{
    if( RootObject == NULL ) return( QVariant() );
    if( ! index.isValid() ) return( QVariant() );

    CSnapshot* p_obj = static_cast<CSnapshot*>(index.internalPointer());

    switch(role) {
        case Qt::DisplayRole: {
            switch( index.column() ) {
                case 0:
                    return(p_obj->GetProperty(0) );
                case 1:
                    return(PQ_ENERGY->GetRealValueText(p_obj->GetProperty(1) ) );
                case 2:
                    return(PQ_DISTANCE->GetRealValueText(p_obj->GetProperty(2) ) );
                case 3:
                    return(PQ_DISTANCE->GetRealValueText(p_obj->GetProperty(3) ) );
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
                    return(Qt::AlignRight);
                case 2:
                    return(Qt::AlignRight);
                case 3:
                    return(Qt::AlignRight);
                default:
                    return( QVariant() );
            }
        break;

        case Qt::DecorationRole: {
            switch( index.column() ) {
                case 0:{
                    if( RootObject->IsSnapshotActive(p_obj) ){
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

        case Qt::FontRole: {
           switch( index.column() ) {
           // for every of columns will check if is snaphot active
           default:
               if( RootObject->IsSnapshotActive(p_obj) ){
                   QFont boldFont;
                   boldFont.setBold(true);
                   return boldFont;
               }
           }
        }
        break;
    }

    return( QVariant() );
}

//------------------------------------------------------------------------------

Qt::ItemFlags CPDBQTTrajSegmentModel::flags(const QModelIndex &index) const
{
    if( !index.isValid() ) return(Qt::ItemFlags());
    return(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

void CPDBQTTrajSegmentModel::RootObjectDeleted(void)
{
    RootObject = NULL;
        // FIXME
    beginResetModel();
    endResetModel();
}

//------------------------------------------------------------------------------

void CPDBQTTrajSegmentModel::ListChanged(void)
{
        // FIXME
    beginResetModel();
    endResetModel();
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================


