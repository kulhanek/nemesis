// =============================================================================
// NEMESIS - Molecular Modelling Package
// -----------------------------------------------------------------------------
//    Copyright (C) 2010 Petr Kulhanek, kulhanek@chemi.muni.cz
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
#include <GraphicsObjectModel.hpp>
#include <CategoryUUID.hpp>
#include <GraphicsObject.hpp>

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

QObject* GraphicsObjectModelCB(void* p_data);

//------------------------------------------------------------------------------

CExtUUID        GraphicsObjectModelID(
                    "{GRAPHICS_OBJECT_MODEL:d886fff0-9cdb-4161-99bd-eef538d7df34}",
                    "Graphics object model");

CPluginObject   GraphicsObjectModelObject(&NemesisCorePlugin,
                    GraphicsObjectModelID,MODEL_CAT,
                    GraphicsObjectModelCB);

// -----------------------------------------------------------------------------

QObject* GraphicsObjectModelCB(void* p_data)
{
    CGraphicsObject* p_con = static_cast<CGraphicsObject*>(p_data);
    if( p_con == NULL ) return(NULL);

    CGraphicsObjectModel* p_obj = new CGraphicsObjectModel(NULL);
    p_obj->SetRootObject(p_con);

    return(p_obj);
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

CGraphicsObjectModel::CGraphicsObjectModel(QObject* p_parent)
    : CContainerModel(&GraphicsObjectModelObject,p_parent)
{
    RootObject = NULL;
}

//------------------------------------------------------------------------------

void CGraphicsObjectModel::SetRootObject(CGraphicsObject* p_data)
{
    if( RootObject != NULL ) {
        RootObject->disconnect(this);
    }

    RootObject = p_data;
    if( RootObject != NULL ) {
        connect(RootObject,SIGNAL(destroyed(QObject*)),
                this,SLOT(RootObjectDeleted(void)));

        connect(RootObject,SIGNAL(OnGraphicsObjectContentsChanged(void)),
                this,SLOT(GraphicsObjectChanged(void)));
    }
    // FIXME
    beginResetModel();
    endResetModel();
}

//------------------------------------------------------------------------------

QObject*  CGraphicsObjectModel::GetItem(const QModelIndex& index) const
{
    if( ! index.isValid() ) return(NULL);
    CGraphicsObject* p_obj = static_cast<CGraphicsObject*>(index.internalPointer());
    return(p_obj);
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

QVariant CGraphicsObjectModel::headerData(int section, Qt::Orientation orientation,
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

int CGraphicsObjectModel::columnCount(const QModelIndex &parent) const
{
    return(4);
}

//------------------------------------------------------------------------------

int CGraphicsObjectModel::rowCount(const QModelIndex &parent) const
{
    if( RootObject == 0 ) return(0);
    return( RootObject->Objects.count() );
}

//------------------------------------------------------------------------------

QModelIndex CGraphicsObjectModel::index(int row, int column,
                                  const QModelIndex &parent) const
{
    if( RootObject == NULL ) return( QModelIndex() );

    if( ! hasIndex(row, column, parent) ) return( QModelIndex() );
    if( parent.isValid() )  return( QModelIndex() );

    QModelIndex index = createIndex(row, column, RootObject->Objects[row] );

    return(index);
}

//------------------------------------------------------------------------------

QModelIndex CGraphicsObjectModel::parent(const QModelIndex &index) const
{
    // parent is always invalid
    return( QModelIndex() );
}

//------------------------------------------------------------------------------

QVariant CGraphicsObjectModel::data(const QModelIndex &index, int role) const
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
            return(p_obj->GetPluginObject()->GetName());
        case 2:
            return(p_obj->GetDescription());
        case 3:
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
        case 2:
            return(Qt::AlignLeft);
        case 3:
            return(Qt::AlignRight);
        default:
            return( QVariant() );
        }
        break;
    }

    return( QVariant() );
}

//------------------------------------------------------------------------------

Qt::ItemFlags CGraphicsObjectModel::flags(const QModelIndex &index) const
{
    if( !index.isValid() ) return(Qt::ItemFlags());
    return(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

void CGraphicsObjectModel::RootObjectDeleted(void)
{
    RootObject = NULL;
    // FIXME
    beginResetModel();
    endResetModel();
}

//------------------------------------------------------------------------------

void CGraphicsObjectModel::GraphicsObjectChanged(void)
{
    // FIXME
    beginResetModel();
    endResetModel();
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================


