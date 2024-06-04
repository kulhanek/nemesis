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
#include <GraphicsViewListModel.hpp>
#include <CategoryUUID.hpp>
#include <GraphicsViewList.hpp>
#include <GraphicsView.hpp>
#include <PeriodicTable.hpp>
#include <PhysicalQuantities.hpp>
#include <PhysicalQuantity.hpp>
#include <QBrush>
#include <QMimeData>

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

QObject* GraphicsViewListModelCB(void* p_data);

//------------------------------------------------------------------------------

CExtUUID        GraphicsViewListModelID(
                    "{GRAPHICS_VIEW_LIST_MODEL:4609b8bd-bc46-4bbb-adf8-6c820ef5e8a1}",
                    "Views");

CPluginObject   GraphicsViewListModelObject(&NemesisCorePlugin,
                    GraphicsViewListModelID,MODEL_CAT,
                    ":/images/NemesisCore/graphics/ViewList.svg",
                    GraphicsViewListModelCB);

// -----------------------------------------------------------------------------

QObject* GraphicsViewListModelCB(void* p_data)
{
    CGraphicsViewList* p_con = static_cast<CGraphicsViewList*>(p_data);
    if( p_con == NULL ) return(NULL);

    CGraphicsViewListModel* p_obj = new CGraphicsViewListModel(NULL);
    p_obj->SetRootObject(p_con);

    return(p_obj);
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

CGraphicsViewListModel::CGraphicsViewListModel(QObject* p_parent)
    : CContainerModel(&GraphicsViewListModelObject,p_parent)
{
    RootObject = NULL;
}

//------------------------------------------------------------------------------

void CGraphicsViewListModel::SetRootObject(CGraphicsViewList* p_data)
{
    if( RootObject != NULL ) {
        RootObject->disconnect(this);
    }

    RootObject = p_data;
    if( RootObject != NULL ) {
        connect(RootObject,SIGNAL(destroyed(QObject*)),
                this,SLOT(RootObjectDeleted(void)));

        connect(RootObject,SIGNAL(OnGraphicsViewListChanged(void)),
                this,SLOT(GraphicsViewChanged(void)));
    }
    // FIXME
    beginResetModel();
    endResetModel();
}

//------------------------------------------------------------------------------

QObject*  CGraphicsViewListModel::GetItem(const QModelIndex& index) const
{
    if( ! index.isValid() ) return(NULL);
    CGraphicsView* p_obj = static_cast<CGraphicsView*>(index.internalPointer());
    return(p_obj);
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

QVariant CGraphicsViewListModel::headerData(int section, Qt::Orientation orientation,
                                    int role) const
{
    if( orientation != Qt::Horizontal ) return( QVariant() );
    if( role == Qt::TextAlignmentRole ) return(Qt::AlignHCenter);
    if( role != Qt::DisplayRole ) return( QVariant() );

    switch(section) {
    case 0:
        return(tr("Name"));
    case 1:
        return(tr("State"));
    case 2:
        return(tr("Description"));
    case 3:
        return(tr("ID"));
    default:
        return("");
    }
}

//------------------------------------------------------------------------------

int CGraphicsViewListModel::columnCount(const QModelIndex &parent) const
{
    return(4);
}

//------------------------------------------------------------------------------

int CGraphicsViewListModel::rowCount(const QModelIndex &parent) const
{
    if( RootObject == 0 ) return(0);
    return( RootObject->children().count() );
}

//------------------------------------------------------------------------------

QModelIndex CGraphicsViewListModel::index(int row, int column,
                                  const QModelIndex &parent) const
{
    if( RootObject == NULL ) return( QModelIndex() );

    if( ! hasIndex(row, column, parent) ) return( QModelIndex() );
    if( parent.isValid() )  return( QModelIndex() );

    QModelIndex index = createIndex(row, column, RootObject->children()[row]);

    return(index);
}

//------------------------------------------------------------------------------

QModelIndex CGraphicsViewListModel::parent(const QModelIndex &index) const
{
    // parent is always invalid
    return( QModelIndex() );
}

//------------------------------------------------------------------------------

QVariant CGraphicsViewListModel::data(const QModelIndex &index, int role) const
{
    if( RootObject == NULL ) return( QVariant() );
    if( ! index.isValid() ) return( QVariant() );

    CGraphicsView* p_obj = static_cast<CGraphicsView*>(index.internalPointer());

    switch(role) {
        case Qt::DisplayRole: {
            switch( index.column() ) {
                case 0:
                    return(p_obj->GetName());
                case 1:{
                    QString out;
                    if( p_obj->IsOpenGLCanvasAttached() ){
                        out = tr("attached");
                    }
                    return(out);
                    }
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
                    icon = p_obj->GetPluginObject()->GetIcon();
                    return(icon);
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

Qt::ItemFlags CGraphicsViewListModel::flags(const QModelIndex &index) const
{
    if( !index.isValid() ) return(Qt::ItemFlags());
    return(Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsDragEnabled);
}

//------------------------------------------------------------------------------

QStringList CGraphicsViewListModel::mimeTypes(void) const
{
    QStringList types;
    types << RootObject->GetProjectMIMEType("graphics.views");
    return types;
}

//------------------------------------------------------------------------------

QMimeData* CGraphicsViewListModel::mimeData(const QModelIndexList &indexes) const
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

    p_mimeData->setData(RootObject->GetProjectMIMEType("graphics.views"), encodedData);
    return(p_mimeData);
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

void CGraphicsViewListModel::RootObjectDeleted(void)
{
    RootObject = NULL;
    // FIXME
    beginResetModel();
    endResetModel();
}

//------------------------------------------------------------------------------

void CGraphicsViewListModel::GraphicsViewChanged(void)
{
    // FIXME
    beginResetModel();
    endResetModel();
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================


