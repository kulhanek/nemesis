// =============================================================================
// NEMESIS - Molecular Modelling Package
// -----------------------------------------------------------------------------
//    Copyright (C) 2010 Petr Kulhanek, kulhanek@chemi.muni.cz
//    Copyright (C) 2008-2009 Petr Kulhanek, kulhanek@enzim.hu,
//                            Jakub Stepan, xstepan3@chemi.muni.cz
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

#include <NemesisCoreModule.hpp>
#include <BondListModel.hpp>
#include <CategoryUUID.hpp>
#include <BondList.hpp>
#include <Bond.hpp>
#include <PeriodicTable.hpp>
#include <PhysicalQuantities.hpp>
#include <PhysicalQuantity.hpp>
#include <Project.hpp>
#include <StructureList.hpp>
#include <QMimeData>

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

QObject* BondListModelCB(void* p_data);

//------------------------------------------------------------------------------

CExtUUID        BondListModelID(
                    "{BOND_LIST_MODEL:3562c103-e92d-40f8-833b-08b0e8744276}",
                    "Bonds");

CPluginObject   BondListModelObject(&NemesisCorePlugin,
                    BondListModelID,MODEL_CAT,
                    ":/images/NemesisCore/structure/BondList.svg",
                    BondListModelCB);

// -----------------------------------------------------------------------------

QObject* BondListModelCB(void* p_data)
{
    CBondList* p_con = static_cast<CBondList*>(p_data);
    if( p_con == NULL ) return(NULL);

    CBondListModel* p_obj = new CBondListModel(NULL);
    p_obj->SetRootObject(p_con);

    return(p_obj);
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

CBondListModel::CBondListModel(QObject* p_parent)
    : CContainerModel(&BondListModelObject,p_parent)
{
    RootObject = NULL;

    connect(PQ_DISTANCE,SIGNAL(OnUnitChanged(void)),
            this,SLOT(ListChanged(void)));
}

//------------------------------------------------------------------------------

void CBondListModel::SetRootObject(CBondList* p_data)
{
    if( RootObject != NULL ) {
        if( RootObject->GetProject()->GetStructures() ){
            RootObject->GetProject()->GetStructures()->disconnect(this);
        }
        RootObject->disconnect(this);
    }

    RootObject = p_data;
    if( RootObject != NULL ) {
        connect(RootObject,SIGNAL(destroyed(QObject*)),
                this,SLOT(RootObjectDeleted(void)));

        connect(RootObject,SIGNAL(OnBondListChanged()),
                this,SLOT(ListChanged()));

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

QObject*  CBondListModel::GetItem(const QModelIndex& index) const
{
    if( ! index.isValid() ) return(NULL);
    CBond* p_obj = static_cast<CBond*>(index.internalPointer());
    return(p_obj);
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

QVariant CBondListModel::headerData(int section, Qt::Orientation orientation,
                                    int role) const
{
    if( orientation != Qt::Horizontal ) return( QVariant() );
    if( role == Qt::TextAlignmentRole ) return(Qt::AlignHCenter);
    if( role != Qt::DisplayRole ) return( QVariant() );

    switch(section) {
    case 0:
        return(tr("Name"));
    case 1:
        return(tr("Order"));
    case 2:
        return(tr("Type"));
    case 3: {
        QString col = tr("d [%1]");
        col = col.arg(PQ_DISTANCE->GetUnitName());
        return(col);
    }
    case 4:
        return(tr("ID"));
    case 5:
        return(tr("Description"));
    default:
        return("");
    }
}

//------------------------------------------------------------------------------

int CBondListModel::columnCount(const QModelIndex &parent) const
{
    return(6);
}

//------------------------------------------------------------------------------

int CBondListModel::rowCount(const QModelIndex &parent) const
{
    if( RootObject == 0 ) return(0);
    return( RootObject->children().count() );
}

//------------------------------------------------------------------------------

QModelIndex CBondListModel::index(int row, int column,
                                  const QModelIndex &parent) const
{
    if( RootObject == NULL ) return( QModelIndex() );

    if( ! hasIndex(row, column, parent) ) return( QModelIndex() );
    if( parent.isValid() )  return( QModelIndex() );

    QModelIndex index = createIndex(row, column, RootObject->children()[row]);

    return(index);
}

//------------------------------------------------------------------------------

QModelIndex CBondListModel::parent(const QModelIndex &index) const
{
    // parent is always invalid
    return( QModelIndex() );
}

//------------------------------------------------------------------------------

QVariant CBondListModel::data(const QModelIndex &index, int role) const
{
    if( RootObject == NULL ) return( QVariant() );
    if( ! index.isValid() ) return( QVariant() );

    CBond* p_obj = static_cast<CBond*>(index.internalPointer());

    switch(role) {
    case Qt::DisplayRole: {
        switch( index.column() ) {
        case 0:
            return(p_obj->GetName());
        case 1:
            return(p_obj->GetBondOrderInStringForm());
        case 2:
            return(p_obj->GetBondType());
        case 3:
            return(PQ_DISTANCE->GetRealValueText(p_obj->GetLength()));
        case 4:
            return(p_obj->GetIndex());
        case 5:
            return(p_obj->GetDescription());
        default:
            return( QVariant() );
        }
        break;
    }

    case Qt::TextAlignmentRole:
        switch( index.column() ) {
        case 0:
        case 1:
        case 2:
        case 3:
            return(Qt::AlignLeft);
        case 4:
            return(Qt::AlignRight);
        case 5:
            return(Qt::AlignLeft);
        default:
            return( QVariant() );
        }
        break;
    }

    return( QVariant() );
}

//------------------------------------------------------------------------------

Qt::ItemFlags CBondListModel::flags(const QModelIndex &index) const
{
    if( !index.isValid() ) return(Qt::ItemFlags());
    return(Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsDragEnabled);
}

//------------------------------------------------------------------------------

QStringList CBondListModel::mimeTypes(void) const
{
    QStringList types;
    types << RootObject->GetProjectMIMEType("bond.indexes");
    return types;
}

//------------------------------------------------------------------------------

QMimeData* CBondListModel::mimeData(const QModelIndexList &indexes) const
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

    p_mimeData->setData(RootObject->GetProjectMIMEType("bond.indexes"), encodedData);
    return(p_mimeData);
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

void CBondListModel::RootObjectDeleted(void)
{
    RootObject = NULL;
    // FIXME
    beginResetModel();
    endResetModel();
}

//------------------------------------------------------------------------------

void CBondListModel::ListChanged(void)
{
    // FIXME
    beginResetModel();
    endResetModel();
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================


