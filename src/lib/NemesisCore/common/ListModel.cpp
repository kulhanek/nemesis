// =============================================================================
// NEMESIS - Molecular Modelling Package
// -----------------------------------------------------------------------------
//    Copyright (C) 2010        Petr Kulhanek, kulhanek@chemi.muni.cz
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

#include <ListModel.hpp>
#include <NemesisCoreModule.hpp>
#include <CategoryUUID.hpp>
#include <ExtComObject.hpp>

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

QObject* ListModelCB(void* p_data);

CExtUUID        ListModelID(
                    "{LIST_MODEL:3968bd2d-8f85-4473-b74c-5ac401e2d4bc}",
                    "List model");

CPluginObject   ListModelObject(&NemesisCorePlugin,
                    ListModelID,MODEL_CAT,
                    ListModelCB);

//------------------------------------------------------------------------------

QObject* ListModelCB(void* p_data)
{
    QObject* p_con = static_cast<QObject*>(p_data);
    if( p_con == NULL ) return(NULL);

    CListModel* p_obj = new CListModel(NULL);
    p_obj->SetRootObject(p_con);

    return(p_obj);
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

CListModel::CListModel(QObject* p_parent)
    : CContainerModel(&ListModelObject,p_parent)
{
    RootObject = NULL;
}

//------------------------------------------------------------------------------

void CListModel::SetRootObject(QObject* p_data)
{
    RootObject = p_data;
    // FIXME
    beginResetModel();
    endResetModel();
}

//------------------------------------------------------------------------------

QObject* CListModel::GetItem(const QModelIndex& index) const
{
    if( ! index.isValid() ) return(NULL);
    return( static_cast<QObject*>(index.internalPointer()));
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

QVariant CListModel::headerData(int section, Qt::Orientation orientation,
                                int role) const
{
    if( role != Qt::DisplayRole ) return( QVariant() );
    if( orientation != Qt::Horizontal ) return( QVariant() );

    return(RootObject->objectName());
}

//------------------------------------------------------------------------------

int CListModel::columnCount(const QModelIndex &parent) const
{
    return(1);
}

//------------------------------------------------------------------------------

int CListModel::rowCount(const QModelIndex &parent) const
{
    if( RootObject == 0 ) return(0);
    return( RootObject->children().count() );
}

//------------------------------------------------------------------------------

QModelIndex CListModel::index(int row, int column,
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

QModelIndex CListModel::parent(const QModelIndex &index) const
{
    // parent is always invalid
    return( QModelIndex() );
}

//------------------------------------------------------------------------------

QVariant CListModel::data(const QModelIndex &index, int role) const
{
    if( RootObject == 0 ) return( QVariant() );
    if( ! index.isValid() ) return( QVariant() );

    switch(role) {
    case Qt::DisplayRole: {
        QObject* p_item = static_cast<QObject*>(index.internalPointer());
        return(p_item->objectName());
    }

    case Qt::DecorationRole: {
        QObject* p_item = static_cast<QObject*>(index.internalPointer());
        CComObject* p_com = dynamic_cast<CComObject*>(p_item);
        if( p_com == NULL ) return( QVariant() );
        return(p_com->GetPluginObject()->GetIcon());
    }
    }

    return( QVariant() );
}

//------------------------------------------------------------------------------

Qt::ItemFlags CListModel::flags(const QModelIndex &index) const
{
    if( !index.isValid() ) return(Qt::ItemFlags());
    return(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================



