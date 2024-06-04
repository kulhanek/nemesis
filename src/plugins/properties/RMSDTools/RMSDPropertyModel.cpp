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

#include "RMSDToolsModule.hpp"
#include <CategoryUUID.hpp>
#include <Atom.hpp>

#include "RMSDProperty.hpp"
#include "MapCouple.hpp"
#include "RMSDPropertyModel.hpp"

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

QObject* RMSDPropertyModelCB(void* p_data);

//------------------------------------------------------------------------------

CExtUUID        RMSDPropertyModelID(
                    "{MAP_LIST_MODEL:4bbe3638-ef6f-4db2-aa5d-2f52fb46820b}",
                    "Map list model");

CPluginObject   RMSDPropertyModelObject(&RMSDToolsPlugin,
                    RMSDPropertyModelID,MODEL_CAT,
                    RMSDPropertyModelCB);

// -----------------------------------------------------------------------------

QObject* RMSDPropertyModelCB(void* p_data)
{
    CRMSDProperty* p_con = static_cast<CRMSDProperty*>(p_data);
    if( p_con == NULL ) return(NULL);

    CRMSDPropertyModel* p_obj = new CRMSDPropertyModel(NULL);
    p_obj->SetRootObject(p_con);

    return(p_obj);
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

CRMSDPropertyModel::CRMSDPropertyModel(QObject* p_parent)
    : CContainerModel(&RMSDPropertyModelObject,p_parent)
{
    RootObject = NULL;
}

//------------------------------------------------------------------------------

void CRMSDPropertyModel::SetRootObject(CRMSDProperty* p_data)
{
    if( RootObject != NULL ) {
        RootObject->disconnect(this);
    }

    RootObject = p_data;
    if( RootObject != NULL ) {
        connect(RootObject,SIGNAL(destroyed(QObject*)),
                this,SLOT(RootObjectDeleted(void)));

        connect(RootObject,SIGNAL(OnRMSDPropertyMapChanged(void)),
                this,SLOT(RMSDPropertyMapChanged(void)));
    }
    // FIXME
    beginResetModel();
    endResetModel();
}

//------------------------------------------------------------------------------

QObject*  CRMSDPropertyModel::GetItem(const QModelIndex& index) const
{
    if( ! index.isValid() ) return(NULL);
    CRMSDProperty* p_obj = static_cast<CRMSDProperty*>(index.internalPointer());
    return(p_obj);
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

QVariant CRMSDPropertyModel::headerData(int section, Qt::Orientation orientation,
                                    int role) const
{
    if( orientation != Qt::Horizontal ) return( QVariant() );
    if( role == Qt::TextAlignmentRole ) return(Qt::AlignHCenter);
    if( role != Qt::DisplayRole ) return( QVariant() );

    switch(section) {
    case 0:
        return(tr("Template"));
    case 1:
        return(tr("Target"));
    default:
        return("");
    }
}

//------------------------------------------------------------------------------

int CRMSDPropertyModel::columnCount(const QModelIndex &parent) const
{
    return(2);
}

//------------------------------------------------------------------------------

int CRMSDPropertyModel::rowCount(const QModelIndex &parent) const
{
    if( RootObject == 0 ) return(0);
    return( RootObject->children().count() );
}

//------------------------------------------------------------------------------

QModelIndex CRMSDPropertyModel::index(int row, int column,
                                  const QModelIndex &parent) const
{
    if( RootObject == NULL ) return( QModelIndex() );

    if( ! hasIndex(row, column, parent) ) return( QModelIndex() );
    if( parent.isValid() )  return( QModelIndex() );

    QModelIndex index = createIndex(row, column, RootObject->children()[row]);

    return(index);
}

//------------------------------------------------------------------------------

QModelIndex CRMSDPropertyModel::parent(const QModelIndex &index) const
{
    // parent is always invalid
    return( QModelIndex() );
}

//------------------------------------------------------------------------------

QVariant CRMSDPropertyModel::data(const QModelIndex &index, int role) const
{
    if( RootObject == NULL ) return( QVariant() );
    if( ! index.isValid() ) return( QVariant() );

    CMapCouple* p_obj = static_cast<CMapCouple*>(index.internalPointer());

    switch(role) {
        case Qt::DisplayRole: {
            switch( index.column() ) {
                case 0:{
                    if( p_obj->GetTemplateAtom() ){
                        return(p_obj->GetTemplateAtom()->GetName());
                    }
                }
                break;
                case 1:{
                    if( p_obj->GetTargetAtom() ){
                        return(p_obj->GetTargetAtom()->GetName());
                    }
                }
                break;
            }
            break;
        }
    }

    return( QVariant() );
}

//------------------------------------------------------------------------------

Qt::ItemFlags CRMSDPropertyModel::flags(const QModelIndex &index) const
{
    if( !index.isValid() ) return(Qt::ItemFlags());
    return(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

void CRMSDPropertyModel::RootObjectDeleted(void)
{
    RootObject = NULL;
    // FIXME
    beginResetModel();
    endResetModel();
}

//------------------------------------------------------------------------------

void CRMSDPropertyModel::RMSDPropertyMapChanged(void)
{
    // FIXME
    beginResetModel();
    endResetModel();
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================


