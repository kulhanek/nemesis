// =============================================================================
// NEMESIS - Molecular Modelling Package
// -----------------------------------------------------------------------------
//    Copyright (C) 2010 Petr Kulhanek, kulhanek@chemi.muni.cz
//    Copyright (C) 2008-2009 Petr Kulhanek, kulhanek@enzim.hu,
//                            Jakub Stepan, xstepan3@chemi.muni.cz
//    Copyright (C) 1998-2004 Petr Kulhanek, kulhanek@chemi.muni.cz
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
#include <HistoryListModel.hpp>
#include <CategoryUUID.hpp>

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

QObject* HistoryListModelCB(void* p_data);

//------------------------------------------------------------------------------

CExtUUID        HistoryListModelID(
                    "{HISTORY_LIST_MODEL:ab61accf-349f-4968-b34b-7c00c539214e}",
                    "History");

CPluginObject   HistoryListModelObject(&NemesisCorePlugin,
                    HistoryListModelID,MODEL_CAT,
                    ":/images/NemesisCore/project/HistoryItem.svg",
                    HistoryListModelCB);

// -----------------------------------------------------------------------------

QObject* HistoryListModelCB(void* p_data)
{
    CHistoryList* p_con = static_cast<CHistoryList*>(p_data);
    if( p_con == NULL ) return(NULL);

    CHistoryListModel* p_obj = new CHistoryListModel(NULL);
    p_obj->SetRootObject(p_con);

    return(p_obj);
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

CHistoryListModel::CHistoryListModel(QObject* p_parent)
    : CContainerModel(&HistoryListModelObject,p_parent)
{
    RootObject = NULL;
}

//------------------------------------------------------------------------------

void CHistoryListModel::SetRootObject(CHistoryList* p_data)
{
    if( RootObject != NULL ) {
        RootObject->disconnect(this);
    }

    RootObject = p_data;
    if( RootObject != NULL ) {
        connect(RootObject,SIGNAL(destroyed(QObject*)),
                this,SLOT(RootObjectDeleted(void)));
        connect(RootObject,SIGNAL(OnHistoryChanged(EHistoryChangeMessage)),
                this,SLOT(HistoryChanged(EHistoryChangeMessage)));
    }
    // FIXME
    beginResetModel();
    endResetModel();
}

//------------------------------------------------------------------------------

QObject*  CHistoryListModel::GetItem(const QModelIndex& index) const
{
    if( ! index.isValid() ) return(NULL);
    CHistoryNode* p_obj = static_cast<CHistoryNode*>(index.internalPointer());
    return(p_obj);
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

QVariant CHistoryListModel::headerData(int section, Qt::Orientation orientation,
                                       int role) const
{
    if( role != Qt::DisplayRole ) return( QVariant() );
    if( orientation != Qt::Horizontal ) return( QVariant() );

    switch(section) {
    case 0:
        return(tr("Type"));
    case 1:
        return(tr("Description"));
    case 2:
        return(tr("Details"));
    default:
        return("");
    }
}

//------------------------------------------------------------------------------

int CHistoryListModel::columnCount(const QModelIndex &parent) const
{
    return(3);
}

//------------------------------------------------------------------------------

int CHistoryListModel::rowCount(const QModelIndex &parent) const
{
    if( RootObject == 0 ) return(0);
    return( RootObject->children().count() );
}


//------------------------------------------------------------------------------

QModelIndex CHistoryListModel::index(int row, int column,
                                     const QModelIndex &parent) const
{
    if( RootObject == NULL ) return( QModelIndex() );

    if( ! hasIndex(row, column, parent) ) return( QModelIndex() );
    if( parent.isValid() )  return( QModelIndex() );

    int item_row = RootObject->children().count() - row - 1;
    QModelIndex index = createIndex(row, column, RootObject->children()[item_row]);

    return(index);
}

//------------------------------------------------------------------------------

QModelIndex CHistoryListModel::parent(const QModelIndex &index) const
{
    // parent is always invalid
    return( QModelIndex() );
}

//------------------------------------------------------------------------------

QVariant CHistoryListModel::data(const QModelIndex &index, int role) const
{
    if( RootObject == NULL ) return( QVariant() );
    if( ! index.isValid() ) return( QVariant() );

    CHistoryNode* p_obj = static_cast<CHistoryNode*>(index.internalPointer());

    int redo_count = RootObject->GetMaxNumberOfRedoAvailable();

    switch(role) {
    case Qt::DisplayRole: {
        switch( index.column() ) {
        case 0:
            if( index.row() < redo_count ) {
                return(tr("redo"));
            } else {
                return(tr("undo"));
            }
        case 1:
            return(p_obj->GetShortDescription());
        case 2:
            return(p_obj->GetLongDescription());
        default:
            return( QVariant() );
        }
        break;
    }

    case Qt::DecorationRole:
        switch( index.column() ) {
        case 0:
            if( index.row() < redo_count ) {
                return(QIcon(":/images/NemesisCore/window/MainWindow/edit/redo.png"));
            } else {
                return(QIcon(":/images/NemesisCore/window/MainWindow/edit/undo.png"));
            }
        default:
            return( QVariant() );
        }
        break;
    }

    return( QVariant() );
}

//------------------------------------------------------------------------------

Qt::ItemFlags CHistoryListModel::flags(const QModelIndex &index) const
{
    if( !index.isValid() ) return(Qt::ItemFlags());
    return(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

void CHistoryListModel::RootObjectDeleted(void)
{
    RootObject = NULL;
    // FIXME
    beginResetModel();
    endResetModel();
}

//------------------------------------------------------------------------------

void CHistoryListModel::HistoryChanged(EHistoryChangeMessage message)
{
    // FIXME
    beginResetModel();
    endResetModel();
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================


