// =============================================================================
// NEMESIS - Molecular Modelling Package
// -----------------------------------------------------------------------------
//    Copyright (C) 2011 Petr Kulhanek, kulhanek@chemi.muni.cz
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
#include <ResidueListModel.hpp>
#include <CategoryUUID.hpp>
#include <ResidueList.hpp>
#include <Residue.hpp>
#include <PeriodicTable.hpp>
#include <PhysicalQuantities.hpp>
#include <PhysicalQuantity.hpp>
#include <QMimeData>

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

QObject* ResidueListModelCB(void* p_data);

//------------------------------------------------------------------------------

CExtUUID        ResidueListModelID(
                    "{RESIDUE_LIST_MODEL:44711a1e-c457-42b3-89b6-ae7a5c962a69}",
                    "Residues");

CPluginObject   ResidueListModelObject(&NemesisCorePlugin,
                    ResidueListModelID,MODEL_CAT,
                    ":/images/NemesisCore/structure/ResidueList.svg",
                    ResidueListModelCB);

// -----------------------------------------------------------------------------

QObject* ResidueListModelCB(void* p_data)
{
    CResidueList* p_con = static_cast<CResidueList*>(p_data);
    if( p_con == NULL ) return(NULL);

    CResidueListModel* p_obj = new CResidueListModel(NULL);
    p_obj->SetRootObject(p_con);

    return(p_obj);
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

CResidueListModel::CResidueListModel(QObject* p_parent)
    : CContainerModel(&ResidueListModelObject,p_parent)
{
    RootObject = NULL;
}

//------------------------------------------------------------------------------

void CResidueListModel::SetRootObject(CResidueList* p_data)
{
    if( RootObject != NULL ) {
        RootObject->disconnect(this);
    }

    RootObject = p_data;
    if( RootObject != NULL ) {
        connect(RootObject,SIGNAL(destroyed(QObject*)),
                this,SLOT(RootObjectDeleted(void)));

        connect(RootObject,SIGNAL(OnResidueListChanged(void)),
                this,SLOT(ResidueListChanged(void)));

    }
    // FIXME
    beginResetModel();
    endResetModel();
}

//------------------------------------------------------------------------------

QObject*  CResidueListModel::GetItem(const QModelIndex& index) const
{
    if( ! index.isValid() ) return(NULL);
    CResidue* p_obj = static_cast<CResidue*>(index.internalPointer());
    return(p_obj);
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

QVariant CResidueListModel::headerData(int section, Qt::Orientation orientation,
                                    int role) const
{
    if( orientation != Qt::Horizontal ) return( QVariant() );
    if( role == Qt::TextAlignmentRole ) return(Qt::AlignHCenter);
    if( role != Qt::DisplayRole ) return( QVariant() );

    switch(section) {
    case 0:
        return(tr("SeqIndx"));
    case 1:
        return(tr("Name"));
    case 2:
        return(tr("Chain"));
    case 3:
        return(tr("Type"));
    case 4:
        return(tr("ID"));
    case 5:
        return(tr("Description"));
    default:
        return("");
    }
}

//------------------------------------------------------------------------------

int CResidueListModel::columnCount(const QModelIndex &parent) const
{
    return(6);
}

//------------------------------------------------------------------------------

int CResidueListModel::rowCount(const QModelIndex &parent) const
{
    if( RootObject == 0 ) return(0);
    return( RootObject->children().count() );
}

//------------------------------------------------------------------------------

QModelIndex CResidueListModel::index(int row, int column,
                                  const QModelIndex &parent) const
{
    if( RootObject == NULL ) return( QModelIndex() );

    if( ! hasIndex(row, column, parent) ) return( QModelIndex() );
    if( parent.isValid() )  return( QModelIndex() );

    QModelIndex index = createIndex(row, column, RootObject->children()[row]);

    return(index);
}

//------------------------------------------------------------------------------

QModelIndex CResidueListModel::parent(const QModelIndex &index) const
{
    // parent is always invalid
    return( QModelIndex() );
}

//------------------------------------------------------------------------------

QVariant CResidueListModel::data(const QModelIndex &index, int role) const
{
    if( RootObject == NULL ) return( QVariant() );
    if( ! index.isValid() ) return( QVariant() );

    CResidue* p_obj = static_cast<CResidue*>(index.internalPointer());

    switch(role) {
    case Qt::DisplayRole: {
        switch( index.column() ) {
        case 0:
            return(p_obj->GetSeqIndex());
        case 1:
            return(p_obj->GetName());
        case 2:
            return(p_obj->GetChain());
        case 3:
            return(p_obj->GetType());
        case 4:
            return(p_obj->GetIndex());
        case 5:
            return(p_obj->GetDescription());
        default:
            return( QVariant() );
        }
        break;
    }

    case Qt::TextAlignmentRole:
        switch( index.column() ) {
        case 0:
            return(Qt::AlignRight);
        case 1:
        case 2:
        case 3:
            return(Qt::AlignLeft);
        case 4:
            return(Qt::AlignRight);
        case 5:
            return(Qt::AlignLeft);
        default:
            return( QVariant() );
        }
        break;
    }

    return( QVariant() );
}

//------------------------------------------------------------------------------

Qt::ItemFlags CResidueListModel::flags(const QModelIndex &index) const
{
    if( !index.isValid() ) return(Qt::ItemFlags());
    return(Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsDragEnabled);
}

//------------------------------------------------------------------------------

QStringList CResidueListModel::mimeTypes(void) const
{
    QStringList types;
    types << RootObject->GetProjectMIMEType("residue.indexes");
    return types;
}

//------------------------------------------------------------------------------

QMimeData* CResidueListModel::mimeData(const QModelIndexList &indexes) const
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

    p_mimeData->setData(RootObject->GetProjectMIMEType("residue.indexes"), encodedData);
    return(p_mimeData);
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

void CResidueListModel::RootObjectDeleted(void)
{
    RootObject = NULL;
    // FIXME
    beginResetModel();
    endResetModel();
}

//------------------------------------------------------------------------------

void CResidueListModel::ResidueListChanged(void)
{
    // FIXME
    beginResetModel();
    endResetModel();
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================


