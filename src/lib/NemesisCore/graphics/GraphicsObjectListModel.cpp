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
#include <GraphicsObjectListModel.hpp>
#include <CategoryUUID.hpp>
#include <GraphicsObjectList.hpp>
#include <GraphicsObject.hpp>
#include <PeriodicTable.hpp>
#include <PhysicalQuantities.hpp>
#include <PhysicalQuantity.hpp>
#include <GraphicsObjectLink.hpp>
#include <QBrush>
#include <QMimeData>

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

QObject* GraphicsObjectListModelCB(void* p_data);

//------------------------------------------------------------------------------

CExtUUID        GraphicsObjectListModelID(
                    "{GRAPHICS_OBJECT_LIST_MODEL:6ef10a49-f781-4af9-aaab-fabc045af09e}",
                    "Objects");

CPluginObject   GraphicsObjectListModelObject(&NemesisCorePlugin,
                    GraphicsObjectListModelID,MODEL_CAT,
                    ":/images/NemesisCore/graphics/ObjectList.svg",
                    GraphicsObjectListModelCB);

// -----------------------------------------------------------------------------

QObject* GraphicsObjectListModelCB(void* p_data)
{
    CGraphicsObjectList* p_con = static_cast<CGraphicsObjectList*>(p_data);
    if( p_con == NULL ) return(NULL);

    CGraphicsObjectListModel* p_obj = new CGraphicsObjectListModel(NULL);
    p_obj->SetRootObject(p_con);

    return(p_obj);
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

CGraphicsObjectListModel::CGraphicsObjectListModel(QObject* p_parent)
    : CContainerModel(&GraphicsObjectListModelObject,p_parent)
{
    RootObject = NULL;
}

//------------------------------------------------------------------------------

void CGraphicsObjectListModel::SetRootObject(CGraphicsObjectList* p_data)
{
    if( RootObject != NULL ) {
        RootObject->disconnect(this);
    }

    RootObject = p_data;
    if( RootObject != NULL ) {
        connect(RootObject,SIGNAL(destroyed(QObject*)),
                this,SLOT(RootObjectDeleted(void)));

        connect(RootObject,SIGNAL(OnGraphicsObjectListChanged(void)),
                this,SLOT(GraphicsObjectChanged(void)));
    }
    // FIXME
    beginResetModel();
    endResetModel();
}

//------------------------------------------------------------------------------

QObject*  CGraphicsObjectListModel::GetItem(const QModelIndex& index) const
{
    if( ! index.isValid() ) return(NULL);
    CGraphicsObject* p_obj = static_cast<CGraphicsObject*>(index.internalPointer());
    return(p_obj);
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

QVariant CGraphicsObjectListModel::headerData(int section, Qt::Orientation orientation,
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

int CGraphicsObjectListModel::columnCount(const QModelIndex &parent) const
{
    return(4);
}

//------------------------------------------------------------------------------

int CGraphicsObjectListModel::rowCount(const QModelIndex &parent) const
{
    if( RootObject == 0 ) return(0);
    return( RootObject->children().count() );
}

//------------------------------------------------------------------------------

QModelIndex CGraphicsObjectListModel::index(int row, int column,
                                  const QModelIndex &parent) const
{
    if( RootObject == NULL ) return( QModelIndex() );

    if( ! hasIndex(row, column, parent) ) return( QModelIndex() );
    if( parent.isValid() )  return( QModelIndex() );

    QModelIndex index = createIndex(row, column, RootObject->children()[row]);

    return(index);
}

//------------------------------------------------------------------------------

QModelIndex CGraphicsObjectListModel::parent(const QModelIndex &index) const
{
    // parent is always invalid
    return( QModelIndex() );
}

//------------------------------------------------------------------------------

QVariant CGraphicsObjectListModel::data(const QModelIndex &index, int role) const
{
    if( RootObject == NULL ) return( QVariant() );
    if( ! index.isValid() ) return( QVariant() );

    CGraphicsObject* p_obj = static_cast<CGraphicsObject*>(index.internalPointer());

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

        case Qt::ForegroundRole: {
                QBrush brush;
                if( p_obj->IsFlagSet(EPOF_VISIBLE) ){
                    brush.setColor(Qt::black);
                } else {
                    brush.setColor(Qt::lightGray);
                }
                return(brush);
            }
            break;

        case Qt::DecorationRole: {
            switch( index.column() ) {
                case 0:{
                    QIcon icon;
                    CGraphicsObjectLink* p_glink = dynamic_cast<CGraphicsObjectLink*>(p_obj);
                    if( p_glink != NULL ){
                        CProObject* p_pobj = p_glink->GetEndObject();
                        if( p_obj ){
                            icon = p_pobj->GetPluginObject()->GetIcon();
                        }
                    } else {
                        icon = p_obj->GetPluginObject()->GetIcon();
                    }
                    QPixmap pixmap;
                    if( p_obj->IsFlagSet(EPOF_VISIBLE) ){
                        pixmap = icon.pixmap(64,QIcon::Normal);
                    } else {
                        pixmap = icon.pixmap(64,QIcon::Disabled);
                    }
                    QIcon final(pixmap);
                    return(final);
                }
                default:
                    return( QVariant() );
            }
            break;
        }
    }

    return( QVariant() );
}

//------------------------------------------------------------------------------

Qt::ItemFlags CGraphicsObjectListModel::flags(const QModelIndex &index) const
{
    if( !index.isValid() ) return(Qt::ItemFlags());
    return(Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsDragEnabled);
}

//------------------------------------------------------------------------------

QStringList CGraphicsObjectListModel::mimeTypes(void) const
{
    QStringList types;
    types << RootObject->GetProjectMIMEType("graphics.indexes");
    return types;
}

//------------------------------------------------------------------------------

QMimeData* CGraphicsObjectListModel::mimeData(const QModelIndexList &indexes) const
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

    p_mimeData->setData(RootObject->GetProjectMIMEType("graphics.indexes"), encodedData);
    return(p_mimeData);
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

void CGraphicsObjectListModel::RootObjectDeleted(void)
{
    RootObject = NULL;
    // FIXME
    beginResetModel();
    endResetModel();
}

//------------------------------------------------------------------------------

void CGraphicsObjectListModel::GraphicsObjectChanged(void)
{
    // FIXME
    beginResetModel();
    endResetModel();
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================


