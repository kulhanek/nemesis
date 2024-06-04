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
#include <GraphicsProfileListModel.hpp>
#include <CategoryUUID.hpp>
#include <GraphicsProfileList.hpp>
#include <GraphicsProfile.hpp>
#include <PeriodicTable.hpp>
#include <PhysicalQuantities.hpp>
#include <PhysicalQuantity.hpp>

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

QObject* GraphicsProfileListModelCB(void* p_data);

//------------------------------------------------------------------------------

CExtUUID        GraphicsProfileListModelID(
                    "{GRAPHICS_PROFILE_LIST_MODEL:2f682c81-34ed-4988-ae99-ab5eac2393e7}",
                    "Profiles");

CPluginObject   GraphicsProfileListModelObject(&NemesisCorePlugin,
                    GraphicsProfileListModelID,MODEL_CAT,
                    ":/images/NemesisCore/graphics/ProfileList.svg",
                    GraphicsProfileListModelCB);

// -----------------------------------------------------------------------------

QObject* GraphicsProfileListModelCB(void* p_data)
{
    CGraphicsProfileList* p_con = static_cast<CGraphicsProfileList*>(p_data);
    if( p_con == NULL ) return(NULL);

    CGraphicsProfileListModel* p_obj = new CGraphicsProfileListModel(NULL);
    p_obj->SetRootObject(p_con);

    return(p_obj);
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

CGraphicsProfileListModel::CGraphicsProfileListModel(QObject* p_parent)
    : CContainerModel(&GraphicsProfileListModelObject,p_parent)
{
    RootObject = NULL;
}

//------------------------------------------------------------------------------

void CGraphicsProfileListModel::SetRootObject(CGraphicsProfileList* p_data)
{
    if( RootObject != NULL ) {
        RootObject->disconnect(this);
    }

    RootObject = p_data;
    if( RootObject != NULL ) {
        connect(RootObject,SIGNAL(destroyed(QObject*)),
                this,SLOT(RootObjectDeleted(void)));

        connect(RootObject,SIGNAL(OnGraphicsProfileListChanged(void)),
                this,SLOT(GraphicsProfileListChanged(void)));
    }
    // FIXME
    beginResetModel();
    endResetModel();
}

//------------------------------------------------------------------------------

QObject*  CGraphicsProfileListModel::GetItem(const QModelIndex& index) const
{
    if( ! index.isValid() ) return(NULL);
    CGraphicsProfile* p_obj = static_cast<CGraphicsProfile*>(index.internalPointer());
    return(p_obj);
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

QVariant CGraphicsProfileListModel::headerData(int section, Qt::Orientation orientation,
                                    int role) const
{
    if( orientation != Qt::Horizontal ) return( QVariant() );
    if( role == Qt::TextAlignmentRole ) return(Qt::AlignHCenter);
    if( role != Qt::DisplayRole ) return( QVariant() );

    switch(section) {
    case 0:
        return(tr(""));
    case 1:
        return(tr("Name"));
    case 2:
        return(tr("Description"));
    case 3:
        return(tr("ID"));
    default:
        return("");
    }
}

//------------------------------------------------------------------------------

int CGraphicsProfileListModel::columnCount(const QModelIndex &parent) const
{
    return(3);
}

//------------------------------------------------------------------------------

int CGraphicsProfileListModel::rowCount(const QModelIndex &parent) const
{
    if( RootObject == 0 ) return(0);
    return( RootObject->children().count() );
}

//------------------------------------------------------------------------------

QModelIndex CGraphicsProfileListModel::index(int row, int column,
                                  const QModelIndex &parent) const
{
    if( RootObject == NULL ) return( QModelIndex() );

    if( ! hasIndex(row, column, parent) ) return( QModelIndex() );
    if( parent.isValid() )  return( QModelIndex() );

    QModelIndex index = createIndex(row, column, RootObject->children()[row]);

    return(index);
}

//------------------------------------------------------------------------------

QModelIndex CGraphicsProfileListModel::parent(const QModelIndex &index) const
{
    // parent is always invalid
    return( QModelIndex() );
}

//------------------------------------------------------------------------------

QVariant CGraphicsProfileListModel::data(const QModelIndex &index, int role) const
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
        break;

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

        case Qt::DecorationRole: {
            switch( index.column() ) {
                case 0:{
                    if( p_obj->IsProfileActive() ){
                        return(QIcon(":/images/NemesisCore/models/active.svg"));
                    } else {
                        return(QIcon(":/images/NemesisCore/models/notactive.svg"));
                    }
                }
                break;
                default:
                    return( QVariant() );
            }
        }
        break;
    }

    return( QVariant() );
}

//------------------------------------------------------------------------------

Qt::ItemFlags CGraphicsProfileListModel::flags(const QModelIndex &index) const
{
    if( !index.isValid() ) return(Qt::ItemFlags());
    return(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

void CGraphicsProfileListModel::RootObjectDeleted(void)
{
    RootObject = NULL;
    // FIXME
    beginResetModel();
    endResetModel();
}

//------------------------------------------------------------------------------

void CGraphicsProfileListModel::GraphicsProfileListChanged(void)
{
    // FIXME
    beginResetModel();
    endResetModel();
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================


