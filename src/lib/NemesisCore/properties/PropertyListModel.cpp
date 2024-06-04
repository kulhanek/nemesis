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
#include <PropertyListModel.hpp>
#include <CategoryUUID.hpp>
#include <PropertyList.hpp>
#include <Property.hpp>
#include <GraphicsObject.hpp>
#include <QBrush>
#include <QMimeData>

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

QObject* PropertyListModelCB(void* p_data);

//------------------------------------------------------------------------------

CExtUUID        PropertyListModelID(
                    "{PROPERTY_LIST_MODEL:112e5ad6-758b-49da-bfdb-f4386793d90c}",
                    "Property list model");

CPluginObject   PropertyListModelObject(&NemesisCorePlugin,
                    PropertyListModelID,MODEL_CAT,
                    PropertyListModelCB);

// -----------------------------------------------------------------------------

QObject* PropertyListModelCB(void* p_data)
{
    CPropertyList* p_con = static_cast<CPropertyList*>(p_data);
    if( p_con == NULL ) return(NULL);

    CPropertyListModel* p_obj = new CPropertyListModel(NULL);
    p_obj->SetRootObject(p_con);

    return(p_obj);
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

CPropertyListModel::CPropertyListModel(QObject* p_parent)
    : CContainerModel(&PropertyListModelObject,p_parent)
{
    RootObject = NULL;
}

//------------------------------------------------------------------------------

void CPropertyListModel::SetRootObject(CPropertyList* p_data)
{
    if( RootObject != NULL ) {
        RootObject->disconnect(this);
    }

    RootObject = p_data;
    if( RootObject != NULL ) {
        connect(RootObject,SIGNAL(destroyed(QObject*)),
                this,SLOT(RootObjectDeleted(void)));

        connect(RootObject,SIGNAL(OnPropertyListChanged(void)),
                this,SLOT(PropertyListChanged(void)));
    }
    // FIXME
    beginResetModel();
    endResetModel();
}

//------------------------------------------------------------------------------

QObject*  CPropertyListModel::GetItem(const QModelIndex& index) const
{
    if( ! index.isValid() ) return(NULL);
    CGraphicsObject* p_obj = static_cast<CGraphicsObject*>(index.internalPointer());
    return(p_obj);
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

QVariant CPropertyListModel::headerData(int section, Qt::Orientation orientation,
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

int CPropertyListModel::columnCount(const QModelIndex &parent) const
{
    return(4);
}

//------------------------------------------------------------------------------

int CPropertyListModel::rowCount(const QModelIndex &parent) const
{
    if( RootObject == 0 ) return(0);
    return( RootObject->GetNumberOfProperties() );
}

//------------------------------------------------------------------------------

QModelIndex CPropertyListModel::index(int row, int column,
                                  const QModelIndex &parent) const
{
    if( RootObject == NULL ) return( QModelIndex() );

    if( ! hasIndex(row, column, parent) ) return( QModelIndex() );
    if( parent.isValid() )  return( QModelIndex() );

    QModelIndex index = createIndex(row, column, RootObject->GetProperty(row) );

    return(index);
}

//------------------------------------------------------------------------------

QModelIndex CPropertyListModel::parent(const QModelIndex &index) const
{
    // parent is always invalid
    return( QModelIndex() );
}

//------------------------------------------------------------------------------

QVariant CPropertyListModel::data(const QModelIndex &index, int role) const
{
    if( RootObject == NULL ) return( QVariant() );
    if( ! index.isValid() ) return( QVariant() );

    CProperty* p_property = static_cast<CProperty*>(index.internalPointer());

    switch(role) {
        case Qt::DisplayRole: {
            switch( index.column() ) {
                case 0:
                    return(p_property->GetName());
                case 1:
                    return(p_property->GetPluginObject()->GetName());
                case 2:
                    return(p_property->GetDescription());
                case 3:
                    return(p_property->GetIndex());
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

        case Qt::DecorationRole: {
            switch( index.column() ) {
                case 0:{
                    return(p_property->GetPluginObject()->GetIcon());
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

Qt::ItemFlags CPropertyListModel::flags(const QModelIndex &index) const
{
    if( !index.isValid() ) return(Qt::ItemFlags());
    return(Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsDragEnabled);
}

//------------------------------------------------------------------------------

QStringList CPropertyListModel::mimeTypes(void) const
{
    QStringList types;
    types << RootObject->GetProjectMIMEType("property.indexes");
    return types;
}

//------------------------------------------------------------------------------

QMimeData* CPropertyListModel::mimeData(const QModelIndexList &indexes) const
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

    p_mimeData->setData(RootObject->GetProjectMIMEType("property.indexes"), encodedData);
    return(p_mimeData);
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

void CPropertyListModel::RootObjectDeleted(void)
{
    RootObject = NULL;
    // FIXME
    beginResetModel();
    endResetModel();
}

//------------------------------------------------------------------------------

void CPropertyListModel::PropertyListChanged(void)
{
    // FIXME
    beginResetModel();
    endResetModel();
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================


