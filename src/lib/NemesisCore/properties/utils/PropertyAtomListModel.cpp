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
#include <PropertyAtomListModel.hpp>
#include <CategoryUUID.hpp>
#include <PropertyAtomList.hpp>
#include <GraphicsProfile.hpp>
#include <PeriodicTable.hpp>
#include <PhysicalQuantities.hpp>
#include <PhysicalQuantity.hpp>

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

QObject* PropertyAtomListModelCB(void* p_data);

//------------------------------------------------------------------------------

CExtUUID        PropertyAtomListModelID(
                    "{PROPERTY_ATOM_LIST_MODEL:7daf2cf4-6c1e-4b09-a82b-2215f9bc9328}",
                    "Property atoms");

CPluginObject   PropertyAtomListModelObject(&NemesisCorePlugin,
                    PropertyAtomListModelID,MODEL_CAT,
                    PropertyAtomListModelCB);

// -----------------------------------------------------------------------------

QObject* PropertyAtomListModelCB(void* p_data)
{
    CPropertyAtomList* p_con = static_cast<CPropertyAtomList*>(p_data);
    if( p_con == NULL ) return(NULL);

    CPropertyAtomListModel* p_obj = new CPropertyAtomListModel(NULL);
    p_obj->SetRootObject(p_con);

    return(p_obj);
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

CPropertyAtomListModel::CPropertyAtomListModel(QObject* p_parent)
    : CContainerModel(&PropertyAtomListModelObject,p_parent)
{
    RootObject = NULL;
}

//------------------------------------------------------------------------------

void CPropertyAtomListModel::SetRootObject(CPropertyAtomList* p_data)
{
    if( RootObject != NULL ) {
        RootObject->disconnect(this);
    }

    RootObject = p_data;
    if( RootObject != NULL ) {
        connect(RootObject,SIGNAL(destroyed(QObject*)),
                this,SLOT(RootObjectDeleted(void)));

        connect(RootObject,SIGNAL(OnPropertyAtomListChanged(void)),
                this,SLOT(PropertyAtomListChanged(void)));
    }
    // FIXME
    beginResetModel();
    endResetModel();
}

//------------------------------------------------------------------------------

QObject*  CPropertyAtomListModel::GetItem(const QModelIndex& index) const
{
    if( ! index.isValid() ) return(NULL);
    CGraphicsProfile* p_obj = static_cast<CGraphicsProfile*>(index.internalPointer());
    return(p_obj);
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

QVariant CPropertyAtomListModel::headerData(int section, Qt::Orientation orientation,
                                    int role) const
{
    if( orientation != Qt::Horizontal ) return( QVariant() );
    if( role == Qt::TextAlignmentRole ) return(Qt::AlignHCenter);
    if( role != Qt::DisplayRole ) return( QVariant() );

    switch(section) {
    case 0:
        return(tr("Name"));
    case 1:
        return(tr("Description"));
    case 2:
        return(tr("ID"));
    default:
        return("");
    }
}

//------------------------------------------------------------------------------

int CPropertyAtomListModel::columnCount(const QModelIndex &parent) const
{
    return(3);
}

//------------------------------------------------------------------------------

int CPropertyAtomListModel::rowCount(const QModelIndex &parent) const
{
    if( RootObject == 0 ) return(0);
    return( RootObject->GetNumberOfAtoms() );
}

//------------------------------------------------------------------------------

QModelIndex CPropertyAtomListModel::index(int row, int column,
                                  const QModelIndex &parent) const
{
    if( RootObject == NULL ) return( QModelIndex() );

    if( ! hasIndex(row, column, parent) ) return( QModelIndex() );
    if( parent.isValid() )  return( QModelIndex() );

    QModelIndex index = createIndex(row, column, RootObject->GetAtom(row));

    return(index);
}

//------------------------------------------------------------------------------

QModelIndex CPropertyAtomListModel::parent(const QModelIndex &index) const
{
    // parent is always invalid
    return( QModelIndex() );
}

//------------------------------------------------------------------------------

QVariant CPropertyAtomListModel::data(const QModelIndex &index, int role) const
{
    if( RootObject == NULL ) return( QVariant() );
    if( ! index.isValid() ) return( QVariant() );

    CGraphicsProfile* p_obj = static_cast<CGraphicsProfile*>(index.internalPointer());

    switch(role) {
    case Qt::DisplayRole: {
        switch( index.column() ) {
        case 0:
            return(p_obj->GetName());
        case 1:
            return(p_obj->GetDescription());
        case 2:
            return(p_obj->GetIndex());
        default:
            return( QVariant() );
        }
        break;
    }

    case Qt::TextAlignmentRole:
        switch( index.column() ) {
        case 0:
        case 1:
            return(Qt::AlignLeft);
        case 2:
            return(Qt::AlignRight);
        default:
            return( QVariant() );
        }
        break;
    }

    return( QVariant() );
}

//------------------------------------------------------------------------------

Qt::ItemFlags CPropertyAtomListModel::flags(const QModelIndex &index) const
{
    if( !index.isValid() ) return(Qt::ItemFlags());
    return(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

void CPropertyAtomListModel::RootObjectDeleted(void)
{
    RootObject = NULL;
    // FIXME
    beginResetModel();
    endResetModel();
}

//------------------------------------------------------------------------------

void CPropertyAtomListModel::PropertyAtomListChanged(void)
{
    // FIXME
    beginResetModel();
    endResetModel();
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================


