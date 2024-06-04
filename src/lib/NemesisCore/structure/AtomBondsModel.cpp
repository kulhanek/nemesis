// =============================================================================
// NEMESIS - Molecular Modelling Package
// -----------------------------------------------------------------------------
//    Copyright (C) 2011        Petr Kulhanek, kulhanek@chemi.muni.cz
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

#include <AtomBondsModel.hpp>
#include <NemesisCoreModule.hpp>
#include <CategoryUUID.hpp>
#include <ProObject.hpp>
#include <Project.hpp>
#include <PhysicalQuantities.hpp>
#include <PhysicalQuantity.hpp>
#include <Atom.hpp>
#include <Bond.hpp>
#include <QMimeData>

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

QObject* AtomBondsModelCB(void* p_data);

CExtUUID        AtomBondsModelID(
                    "{ATOM_BONDS_MODEL:91905bee-ec8d-4d64-bcc7-4c2da423787c}",
                    "Registered bonds model");

CPluginObject   AtomBondsModelObject(&NemesisCorePlugin,
                    AtomBondsModelID,MODEL_CAT,
                    NULL);

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

CAtomBondsModel::CAtomBondsModel(QObject* p_parent)
    : CContainerModel(&AtomBondsModelObject,p_parent)
{
    RootObject = NULL;
}

//------------------------------------------------------------------------------

void CAtomBondsModel::SetRootObject(CAtom* p_data)
{
    if( RootObject != NULL ) {
        RootObject->disconnect(this);
    }

    RootObject = p_data;

    if( RootObject != NULL ) {
        connect(RootObject,SIGNAL(destroyed(QObject*)),
                this,SLOT(RootObjectDeleted(void)));

        connect(RootObject,SIGNAL(OnBondRegistered(CBond*)),
                this,SLOT(BondRegistered(CBond*)));

        connect(RootObject,SIGNAL(OnBondUnregistered(CBond*)),
                this,SLOT(BondUnregistered(CBond*)));

        foreach(CBond* p_proobj, RootObject->GetBonds()){
            connect(p_proobj,SIGNAL(OnStatusChanged(EStatusChanged)),
                    this,SLOT(BondChanged()));
        }
    }

    // FIXME
    beginResetModel();
    endResetModel();
}

//------------------------------------------------------------------------------

QObject* CAtomBondsModel::GetItem(const QModelIndex& index) const
{
    if( ! index.isValid() ) return(NULL);
    return( static_cast<QObject*>(index.internalPointer()));
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

QVariant CAtomBondsModel::headerData(int section, Qt::Orientation orientation,
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

int CAtomBondsModel::columnCount(const QModelIndex &parent) const
{
    return(6);
}

//------------------------------------------------------------------------------

int CAtomBondsModel::rowCount(const QModelIndex &parent) const
{
    if( RootObject == NULL ) return(0);
    return( RootObject->GetNumberOfBonds() );
}


//------------------------------------------------------------------------------

QModelIndex CAtomBondsModel::index(int row, int column,
                                     const QModelIndex &parent) const
{
    if( RootObject == NULL ) return( QModelIndex() );

    if( ! hasIndex(row, column, parent) ) return( QModelIndex() );
    if( parent.isValid() )  return( QModelIndex() );

    QModelIndex index = createIndex(row, column, RootObject->GetBonds()[row]);

    return(index);
}

//------------------------------------------------------------------------------

QModelIndex CAtomBondsModel::parent(const QModelIndex &index) const
{
    // parent is always invalid
    return( QModelIndex() );
}

//------------------------------------------------------------------------------

QVariant CAtomBondsModel::data(const QModelIndex &index, int role) const
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

Qt::ItemFlags CAtomBondsModel::flags(const QModelIndex &index) const
{
    if( !index.isValid() ) return(Qt::ItemFlags());
    return(Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsDragEnabled);
}

//------------------------------------------------------------------------------

QStringList CAtomBondsModel::mimeTypes(void) const
{
    QStringList types;
    types << RootObject->GetProjectMIMEType("bond.indexes");
    return types;
}

//------------------------------------------------------------------------------

QMimeData* CAtomBondsModel::mimeData(const QModelIndexList &indexes) const
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

void CAtomBondsModel::RootObjectDeleted(void)
{
    RootObject = NULL;
    // FIXME
    beginResetModel();
    endResetModel();
}

//------------------------------------------------------------------------------

void CAtomBondsModel::BondRegistered(CBond* p_obj)
{
    if( p_obj ){
        p_obj->disconnect(this);
        connect(p_obj,SIGNAL(OnStatusChanged(EStatusChanged)),
                this,SLOT(BondChanged()));
    }
    // FIXME
    beginResetModel();
    endResetModel();
}

//------------------------------------------------------------------------------

void CAtomBondsModel::BondUnregistered(CBond* p_obj)
{
    if( p_obj ){
        p_obj->disconnect(this);
    }
    // FIXME
    beginResetModel();
    endResetModel();
}

//------------------------------------------------------------------------------

void CAtomBondsModel::BondChanged(void)
{
    // FIXME
    beginResetModel();
    endResetModel();
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================



