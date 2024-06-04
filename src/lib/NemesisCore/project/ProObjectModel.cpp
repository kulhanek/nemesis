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

#include <ProObjectModel.hpp>
#include <NemesisCoreModule.hpp>
#include <CategoryUUID.hpp>
#include <ProObject.hpp>

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

QObject* ProObjectModelCB(void* p_data);

CExtUUID        ProObjectModelID(
                    "{PRO_OBJECT_MODEL:165adc10-a3ad-4524-92ee-9448bec52eed}",
                    "Project object model");

CPluginObject   ProObjectModelObject(&NemesisCorePlugin,
                    ProObjectModelID,MODEL_CAT,
                    ":images/AdminWorkPanels/error.png",
                    ProObjectModelCB);

//------------------------------------------------------------------------------

QObject* ProObjectModelCB(void* p_data)
{
    CProObject* p_con = static_cast<CProObject*>(p_data);
    if( p_con == NULL ) return(NULL);

    CProObjectModel* p_obj = new CProObjectModel(NULL);
    p_obj->SetRootObject(p_con);

    return(p_obj);
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

CProObjectModel::CProObjectModel(CProObject* p_parent)
    : CContainerModel(&ProObjectModelObject,p_parent)
{
    RootObject = NULL;
}

//------------------------------------------------------------------------------

void CProObjectModel::SetRootObject(CProObject* p_data)
{
    RootObject = p_data;

    if( RootObject != NULL ) {
        connect(RootObject,SIGNAL(destroyed(QObject*)),
                this,SLOT(RootObjectDeleted(void)));
    }

    // FIXME
    beginResetModel();
    endResetModel();
}

//------------------------------------------------------------------------------

QObject* CProObjectModel::GetItem(const QModelIndex& index) const
{
    if( ! index.isValid() ) return(NULL);
    return( static_cast<QObject*>(index.internalPointer()));
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

QVariant CProObjectModel::headerData(int section, Qt::Orientation orientation,
                                     int role) const
{
    if( role != Qt::DisplayRole ) return( QVariant() );
    if( orientation != Qt::Horizontal ) return( QVariant() );

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

int CProObjectModel::columnCount(const QModelIndex &parent) const
{
    return(3);
}

//------------------------------------------------------------------------------

int CProObjectModel::rowCount(const QModelIndex &parent) const
{
    if( RootObject == NULL ) return(0);

    int crow = 0;
    foreach(QObject* p_obj,RootObject->children()){
        CExtComObject* p_extobj = static_cast<CExtComObject*>(p_obj);
        if( ! p_extobj->ConFlags.testFlag(EECOF_HIDDEN) ) crow++;
    }

    return( crow );
}


//------------------------------------------------------------------------------

QModelIndex CProObjectModel::index(int row, int column,
                                   const QModelIndex &parent) const
{
    if( RootObject == NULL ) return( QModelIndex() );

    if( ! hasIndex(row, column, parent) ) return( QModelIndex() );
    if( parent.isValid() )  return( QModelIndex() );

    CExtComObject* p_child_item = NULL;

    int cindex = -1;
    foreach(QObject* p_obj,RootObject->children()){
        CExtComObject* p_extobj = static_cast<CExtComObject*>(p_obj);
        if( ! p_extobj->ConFlags.testFlag(EECOF_HIDDEN) ){
            cindex++;
        }
        if( cindex == row ){
            p_child_item = p_extobj;
            break;
        }
    }

    QModelIndex index = createIndex(row, column, p_child_item);
    return(index);
}

//------------------------------------------------------------------------------

QModelIndex CProObjectModel::parent(const QModelIndex &index) const
{
    // parent is always invalid
    return( QModelIndex() );
}

//------------------------------------------------------------------------------

QVariant CProObjectModel::data(const QModelIndex &index, int role) const
{
    if( RootObject == NULL ) return( QVariant() );
    if( ! index.isValid() ) return( QVariant() );

    CProObject* p_obj = static_cast<CProObject*>(index.internalPointer());

    switch(role) {
    case Qt::DisplayRole: {
        switch( index.column() ) {
        case 0:
            return(p_obj->GetName());
        case 1:
            return(p_obj->GetDescription());
        case 2:
            if( p_obj->GetIndex() > 0 ){
                return(p_obj->GetIndex());
            } else {
                return( QVariant() );
            }
        default:
            return( QVariant() );
        }
        break;
    }

    case Qt::DecorationRole: {
        switch( index.column() ) {
        case 0:
            return(p_obj->GetPluginObject()->GetIcon());
        default:
            return( QVariant() );
        }
        break;
    }
    }

    return( QVariant() );
}

//------------------------------------------------------------------------------

Qt::ItemFlags CProObjectModel::flags(const QModelIndex &index) const
{
    if( !index.isValid() ) return(Qt::ItemFlags());
    return(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
}

//------------------------------------------------------------------------------

void CProObjectModel::RootObjectDeleted(void)
{
    RootObject = NULL;
    // FIXME
    beginResetModel();
    endResetModel();
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================



