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

#include <SelectionModel.hpp>
#include <NemesisCoreModule.hpp>
#include <CategoryUUID.hpp>
#include <SelectionList.hpp>

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

QObject* SelectionModelCB(void* p_data);

CExtUUID        SelectionModelID(
                    "{SELECTION_MODEL:3245fa86-9fec-40ed-9923-11626c9fad86}",
                    "Selection");

CPluginObject   SelectionModelObject(&NemesisCorePlugin,
                    SelectionModelID,MODEL_CAT,
                    ":/images/NemesisCore/selection/SelectionList.svg",
                    SelectionModelCB);

//------------------------------------------------------------------------------

QObject* SelectionModelCB(void* p_data)
{
    CSelectionList* p_con = static_cast<CSelectionList*>(p_data);
    if( p_con == NULL ) return(NULL);

    CSelectionModel* p_obj = new CSelectionModel(NULL);
    p_obj->SetRootObject(p_con);

    return(p_obj);
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

CSelectionModel::CSelectionModel(QObject* p_parent)
    : CContainerModel(&SelectionModelObject,p_parent)
{
    RootObject = NULL;
}

//------------------------------------------------------------------------------

void CSelectionModel::SetRootObject(CSelectionList* p_data)
{
    RootObject = p_data;
    if( RootObject != NULL ) {
        connect(RootObject,SIGNAL(OnObjectAdded(const CSelObject&)),
                this,SLOT(ObjectAdded(const CSelObject&)));
        connect(RootObject,SIGNAL(OnObjectRemoved(const CSelObject&)),
                this,SLOT(ObjectRemoved(const CSelObject&)));
    }
    // FIXME
    beginResetModel();
    endResetModel();
}

//------------------------------------------------------------------------------

QObject*  CSelectionModel::GetItem(const QModelIndex& index) const
{
    if( ! index.isValid() ) return(NULL);
    CSelObject* p_obj = static_cast<CSelObject*>(index.internalPointer());
    return(p_obj->GetObject());
}

//------------------------------------------------------------------------------

CSelObject*  CSelectionModel::GetFullItem(const QModelIndex& index)
{
    if( ! index.isValid() ) return(NULL);
    CSelObject* p_obj = static_cast<CSelObject*>(index.internalPointer());
    return(p_obj);
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

QVariant CSelectionModel::headerData(int section, Qt::Orientation orientation,
                                     int role) const
{
    if( role != Qt::DisplayRole ) return( QVariant() );
    if( orientation != Qt::Horizontal ) return( QVariant() );

    switch(section) {
    case 0:
        return(tr("SubID"));
    case 1:
        return(tr("Name"));
    case 2:
        return(tr("Type"));
    case 3:
        return(tr("ID"));
    case 4:
        return(tr("Description"));
    default:
        return("");
    }
}

//------------------------------------------------------------------------------

int CSelectionModel::columnCount(const QModelIndex &parent) const
{
    return(5);
}

//------------------------------------------------------------------------------

int CSelectionModel::rowCount(const QModelIndex &parent) const
{
    if( RootObject == 0 ) return(0);
    return( RootObject->NumOfSelectedObjects() );
}


//------------------------------------------------------------------------------

QModelIndex CSelectionModel::index(int row, int column,
                                   const QModelIndex &parent) const
{
    if( RootObject == NULL ) return( QModelIndex() );

    if( ! hasIndex(row, column, parent) ) return( QModelIndex() );
    if( parent.isValid() )  return( QModelIndex() );

    QModelIndex index = createIndex(row, column, RootObject->GetSelectedSelObject(row));

    return(index);
}

//------------------------------------------------------------------------------

QModelIndex CSelectionModel::parent(const QModelIndex &index) const
{
    // parent is always invalid
    return( QModelIndex() );
}

//------------------------------------------------------------------------------

QVariant CSelectionModel::data(const QModelIndex &index, int role) const
{
    if( RootObject == NULL ) return( QVariant() );
    if( ! index.isValid() ) return( QVariant() );

    CSelObject* p_obj = static_cast<CSelObject*>(index.internalPointer());

    switch(role) {
    case Qt::DisplayRole: {
        if( p_obj->GetObject() != NULL ) {
            switch( index.column() ) {
            case 0:
                return(p_obj->GetSubID());
            case 1:
                return(p_obj->GetObject()->GetName());
            case 2:
                return(p_obj->GetObject()->GetType().GetName());
            case 3:
                return(p_obj->GetObject()->GetIndex());
            case 4:
                return(p_obj->GetObject()->GetDescription());
            default:
                return( QVariant() );
            }
        } else {
            switch( index.column() ) {
            case 0:
                return(p_obj->GetSubID());
            default:
                return( QVariant() );
            }
        }
        break;
    }

    case Qt::DecorationRole: {
        if( p_obj->GetObject() != NULL ) {
            switch( index.column() ) {
            case 0:
                return(p_obj->GetObject()->GetPluginObject()->GetIcon());
            default:
                return( QVariant() );
            }
        }
        break;
    }
    }

    return( QVariant() );
}

//------------------------------------------------------------------------------

Qt::ItemFlags CSelectionModel::flags(const QModelIndex &index) const
{
    if( !index.isValid() ) return(Qt::ItemFlags());
    return(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

void CSelectionModel::ObjectAdded(const CSelObject& obj)
{
    // FIXME
    beginResetModel();
    endResetModel();
}

//------------------------------------------------------------------------------

void CSelectionModel::ObjectRemoved(const CSelObject& obj)
{
    // FIXME
    beginResetModel();
    endResetModel();
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================



