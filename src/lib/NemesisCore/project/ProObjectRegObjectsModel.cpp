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

#include <ProObjectRegObjectsModel.hpp>
#include <NemesisCoreModule.hpp>
#include <CategoryUUID.hpp>
#include <ProObject.hpp>
#include <Project.hpp>

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

QObject* ProObjectRegObjectsModelCB(void* p_data);

CExtUUID        ProObjectRegObjectsModelID(
                    "{PROOBJECT_REGOBJECTS_MODEL:376c507c-9095-4489-ad60-8495ba4f48b9}",
                    "Registered object list model");

CPluginObject   ProObjectRegObjectsModelObject(&NemesisCorePlugin,
                    ProObjectRegObjectsModelID,MODEL_CAT,
                    NULL);

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

CProObjectRegObjectsModel::CProObjectRegObjectsModel(QObject* p_parent)
    : CContainerModel(&ProObjectRegObjectsModelObject,p_parent)
{
    RootObject = NULL;
}

//------------------------------------------------------------------------------

void CProObjectRegObjectsModel::SetRootObject(CProObject* p_data)
{
    if( RootObject != NULL ) {
        RootObject->disconnect(this);
    }

    RootObject = p_data;

    if( RootObject != NULL ) {
        connect(RootObject,SIGNAL(destroyed(QObject*)),
                this,SLOT(RootObjectDeleted(void)));

        connect(RootObject,SIGNAL(OnProObjectRegistered(CProObject*)),
                this,SLOT(ProObjectRegistered(CProObject*)));

        connect(RootObject,SIGNAL(OnProObjectUnregistered(CProObject*)),
                this,SLOT(ProObjectUnregistered(CProObject*)));

        foreach( CProObject* p_obj, RootObject->GetROList() ){
            connect(p_obj,SIGNAL(OnStatusChanged(EStatusChanged)),
                    this,SLOT(ProObjectChanged()));
        }
    }

    // FIXME
    beginResetModel();
    endResetModel();
}

//------------------------------------------------------------------------------

QObject* CProObjectRegObjectsModel::GetItem(const QModelIndex& index) const
{
    if( ! index.isValid() ) return(NULL);
    return( static_cast<QObject*>(index.internalPointer()));
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

QVariant CProObjectRegObjectsModel::headerData(int section, Qt::Orientation orientation,
                                       int role) const
{
    if( role != Qt::DisplayRole ) return( QVariant() );
    if( orientation != Qt::Horizontal ) return( QVariant() );

    switch(section) {
    case 0:
        return(tr("Name"));
    case 1:
        return(tr("Type"));
    case 2:
        return(tr("ID"));
    case 3:
        return(tr("Description"));
    default:
        return("");
    }
}

//------------------------------------------------------------------------------

int CProObjectRegObjectsModel::columnCount(const QModelIndex &parent) const
{
    return(4);
}

//------------------------------------------------------------------------------

int CProObjectRegObjectsModel::rowCount(const QModelIndex &parent) const
{
    if( RootObject == NULL ) return(0);
    return( RootObject->GetROList().size() );
}


//------------------------------------------------------------------------------

QModelIndex CProObjectRegObjectsModel::index(int row, int column,
                                     const QModelIndex &parent) const
{
    if( RootObject == NULL ) return( QModelIndex() );

    if( ! hasIndex(row, column, parent) ) return( QModelIndex() );
    if( parent.isValid() )  return( QModelIndex() );

    QModelIndex index = createIndex(row, column, RootObject->GetROList()[row]);

    return(index);
}

//------------------------------------------------------------------------------

QModelIndex CProObjectRegObjectsModel::parent(const QModelIndex &index) const
{
    // parent is always invalid
    return( QModelIndex() );
}

//------------------------------------------------------------------------------

QVariant CProObjectRegObjectsModel::data(const QModelIndex &index, int role) const
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
            return(p_obj->GetType().GetName());
        case 2:
            return(p_obj->GetIndex());
        case 3:
            return(p_obj->GetDescription());
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

    case Qt::TextAlignmentRole:
        switch( index.column() ) {
            case 0:
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

Qt::ItemFlags CProObjectRegObjectsModel::flags(const QModelIndex &index) const
{
    if( !index.isValid() ) return(Qt::ItemFlags());
    return(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

void CProObjectRegObjectsModel::RootObjectDeleted(void)
{
    RootObject = NULL;
    // FIXME
    beginResetModel();
    endResetModel();
}

//------------------------------------------------------------------------------

void CProObjectRegObjectsModel::ProObjectRegistered(CProObject* p_obj)
{
    if( p_obj ){
        p_obj->disconnect(this);
        connect(p_obj,SIGNAL(OnStatusChanged(EStatusChanged)),
                this,SLOT(ProObjectChanged()));
    }
    // FIXME
    beginResetModel();
    endResetModel();
}

//------------------------------------------------------------------------------

void CProObjectRegObjectsModel::ProObjectUnregistered(CProObject* p_obj)
{
    if( p_obj ){
        p_obj->disconnect(this);
    }
    // FIXME
    beginResetModel();
    endResetModel();
}

//------------------------------------------------------------------------------

void CProObjectRegObjectsModel::ProObjectChanged(void)
{
    // FIXME
    beginResetModel();
    endResetModel();
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================



