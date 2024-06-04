// =============================================================================
// NEMESIS - Molecular Modelling Package
// -----------------------------------------------------------------------------
//    Copyright (C) 2011 Petr Kulhanek, kulhanek@chemi.muni.cz
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
#include <RestraintListModel.hpp>
#include <CategoryUUID.hpp>
#include <RestraintList.hpp>
#include <Restraint.hpp>
#include <PhysicalQuantities.hpp>
#include <PhysicalQuantity.hpp>
#include <QBrush>
#include <Property.hpp>
#include <Project.hpp>
#include <StructureList.hpp>
#include <QMimeData>

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

QObject* RestraintListModelCB(void* p_data);

//------------------------------------------------------------------------------

CExtUUID        RestraintListModelID(
                    "{RESTRAINT_LIST_MODEL:25dd0612-e381-4fa1-a182-dca07fab159d}",
                    "Restraints");

CPluginObject   RestraintListModelObject(&NemesisCorePlugin,
                    RestraintListModelID,MODEL_CAT,
                    ":/images/NemesisCore/structure/RestraintList.svg",
                    RestraintListModelCB);

// -----------------------------------------------------------------------------

QObject* RestraintListModelCB(void* p_data)
{
    CRestraintList* p_con = static_cast<CRestraintList*>(p_data);
    if( p_con == NULL ) return(NULL);

    CRestraintListModel* p_obj = new CRestraintListModel(NULL);
    p_obj->SetRootObject(p_con);

    return(p_obj);
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

CRestraintListModel::CRestraintListModel(QObject* p_parent)
    : CContainerModel(&RestraintListModelObject,p_parent)
{
    RootObject = NULL;

    connect(PhysicalQuantities,SIGNAL(OnUnitChanged(void)),
            this,SLOT(ListChanged(void)));
}

//------------------------------------------------------------------------------

void CRestraintListModel::SetRootObject(CRestraintList* p_data)
{
    if( RootObject != NULL ) {
        RootObject->disconnect(this);
        if( RootObject->GetProject()->GetStructures() ){
            RootObject->GetProject()->GetStructures()->disconnect(this);
        }
    }

    RootObject = p_data;
    if( RootObject != NULL ) {
        connect(RootObject,SIGNAL(destroyed(QObject*)),
                this,SLOT(RootObjectDeleted(void)));

        connect(RootObject,SIGNAL(OnRestraintListChanged(void)),
                this,SLOT(ListChanged(void)));

        if( RootObject->GetProject()->GetStructures() ){
            connect(RootObject->GetProject()->GetStructures(),SIGNAL(OnGeometryChangeTick(void)),
                    this,SLOT(ListChanged(void)));
        }
    }
    // FIXME
    beginResetModel();
    endResetModel();
}

//------------------------------------------------------------------------------

QObject*  CRestraintListModel::GetItem(const QModelIndex& index) const
{
    if( ! index.isValid() ) return(NULL);
    CRestraint* p_obj = static_cast<CRestraint*>(index.internalPointer());
    return(p_obj);
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

QVariant CRestraintListModel::headerData(int section, Qt::Orientation orientation,
                                    int role) const
{
    if( orientation != Qt::Horizontal ) return( QVariant() );
    if( role == Qt::TextAlignmentRole ) return(Qt::AlignHCenter);
    if( role != Qt::DisplayRole ) return( QVariant() );

    switch(section) {
        case 0:
            return(tr("Name"));
        case 1:
            return(tr("Target"));
        case 2:
            return(tr("Current"));
        case 3:
            return(tr("Deviation"));
        case 4:
            return(tr("Energy"));
        case 5:
            return(tr("Force"));
        case 6:
            return(tr("Property"));
        case 7:
            return(tr("ID"));
        case 8:
            return(tr("Description"));
        default:
            return("");
    }
}

//------------------------------------------------------------------------------

int CRestraintListModel::columnCount(const QModelIndex &parent) const
{
    return(9);
}

//------------------------------------------------------------------------------

int CRestraintListModel::rowCount(const QModelIndex &parent) const
{
    if( RootObject == 0 ) return(0);
    return( RootObject->children().count() );
}

//------------------------------------------------------------------------------

QModelIndex CRestraintListModel::index(int row, int column,
                                  const QModelIndex &parent) const
{
    if( RootObject == NULL ) return( QModelIndex() );

    if( ! hasIndex(row, column, parent) ) return( QModelIndex() );
    if( parent.isValid() )  return( QModelIndex() );

    QModelIndex index = createIndex(row, column, RootObject->children()[row]);

    return(index);
}

//------------------------------------------------------------------------------

QModelIndex CRestraintListModel::parent(const QModelIndex &index) const
{
    // parent is always invalid
    return( QModelIndex() );
}

//------------------------------------------------------------------------------

QVariant CRestraintListModel::data(const QModelIndex &index, int role) const
{
    if( RootObject == NULL ) return( QVariant() );
    if( ! index.isValid() ) return( QVariant() );

    CRestraint* p_obj = static_cast<CRestraint*>(index.internalPointer());

    switch(role) {
        case Qt::DisplayRole: {
            switch( index.column() ) {
            case 0:
                return(p_obj->GetName());
            case 1:{
                    CPhysicalQuantity* p_pq = p_obj->GetValueUnit();
                    if( p_pq ){
                        return(p_pq->GetRealValueText(p_obj->GetTargetValue()));
                    } else {
                        return("");
                    }
                }
            case 2:{
                    CPhysicalQuantity* p_pq = p_obj->GetValueUnit();
                    if( p_pq ){
                        return(p_pq->GetRealValueText(p_obj->GetCurrentValue()));
                    } else {
                        return("");
                    }
                }
            case 3:{
                    CPhysicalQuantity* p_pq = p_obj->GetValueUnit();
                    if( p_pq ){
                        return(p_pq->GetRealValueText(p_obj->GetDeviationValue()));
                    } else {
                        return("");
                    }
                }
            case 4:{
                    if( p_obj->GetProperty() ){
                        return(PQ_ENERGY->GetRealValueText(p_obj->GetEnergy()));
                    } else {
                        return("");
                    }
                }
            case 5:{
                    CPhysicalQuantity* p_pq = p_obj->GetForceConstantUnit();
                    if( p_pq ){
                        return(p_pq->GetRealValueText(p_obj->GetForceConstant()));
                    } else {
                        return("");
                    }
                }
            case 6:{
                    if( p_obj->GetProperty() ){
                        return(p_obj->GetProperty()->GetName());
                    } else {
                        return("");
                    }
                }
            case 7:
                return(p_obj->GetIndex());
            case 8:
                return(p_obj->GetDescription());
            default:
                return( QVariant() );
            }
            break;
        }

        case Qt::ForegroundRole: {
                QBrush brush;
                bool set = true;
                set &= p_obj->IsEnabled();
                set &= p_obj->GetRestraints()->IsEnabled();
                set &= p_obj->GetProperty() != NULL;
                if( p_obj->GetProperty() != NULL ){
                    set &= p_obj->GetProperty()->IsReady();
                    set &= p_obj->GetProperty()->HasGradient(p_obj->GetStructure());
                } else {
                    set = false;
                }
                if( set ){
                    brush.setColor(Qt::black);
                } else {
                    brush.setColor(Qt::lightGray);
                }
                return(brush);
            }
            break;

        case Qt::TextAlignmentRole:
            switch( index.column() ) {
            case 0:
                return(Qt::AlignLeft);
            case 1:
            case 2:
            case 3:
            case 4:
            case 5:
                return(Qt::AlignRight);
            case 6:
                return(Qt::AlignLeft);
            case 7:
                return(Qt::AlignRight);
            case 8:
                return(Qt::AlignLeft);
            default:
                return( QVariant() );
            }
            break;
    }

    return( QVariant() );
}

//------------------------------------------------------------------------------

Qt::ItemFlags CRestraintListModel::flags(const QModelIndex &index) const
{
    if( !index.isValid() ) return(Qt::ItemFlags());
    return(Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsDragEnabled);
}

//------------------------------------------------------------------------------

QStringList CRestraintListModel::mimeTypes(void) const
{
    QStringList types;
    types << RootObject->GetProjectMIMEType("restraint.indexes");
    return types;
}

//------------------------------------------------------------------------------

QMimeData* CRestraintListModel::mimeData(const QModelIndexList &indexes) const
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

    p_mimeData->setData(RootObject->GetProjectMIMEType("restraint.indexes"), encodedData);
    return(p_mimeData);
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

void CRestraintListModel::RootObjectDeleted(void)
{
    RootObject = NULL;
    // FIXME
    beginResetModel();
    endResetModel();
}

//------------------------------------------------------------------------------

void CRestraintListModel::ListChanged(void)
{
    // FIXME
    beginResetModel();
    endResetModel();
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================


