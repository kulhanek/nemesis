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
#include <StructureListModel.hpp>
#include <CategoryUUID.hpp>
#include <StructureList.hpp>
#include <Structure.hpp>
#include <AtomList.hpp>
#include <QMimeData>
#include <QListWidget>

#include <HistoryList.hpp>
#include <Project.hpp>
#include <ProjectList.hpp>

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

QObject* StructureListModelCB(void* p_data);

//------------------------------------------------------------------------------

CExtUUID        StructureListModelID(
                    "{STRUCTURE_LIST_MODEL:ee0a9cd7-d4a0-42a4-8364-752aad243215}",
                    "Structures");

CPluginObject   StructureListModelObject(&NemesisCorePlugin,
                    StructureListModelID,MODEL_CAT,
                    ":/images/NemesisCore/structure/StructureList.svg",
                    StructureListModelCB);

// -----------------------------------------------------------------------------

QObject* StructureListModelCB(void* p_data)
{
    CStructureList* p_con = static_cast<CStructureList*>(p_data);
    if( p_con == NULL ) return(NULL);

    CStructureListModel* p_obj = new CStructureListModel(NULL);
    p_obj->SetRootObject(p_con);

    return(p_obj);
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

CStructureListModel::CStructureListModel(QObject* p_parent)
    : CContainerModel(&StructureListModelObject,p_parent)
{
    RootObject = NULL;
}

//------------------------------------------------------------------------------

void CStructureListModel::SetRootObject(CStructureList* p_data)
{
    if( RootObject != NULL ) {
        RootObject->disconnect(this);
    }

    RootObject = p_data;
    if( RootObject != NULL ) {
        connect(RootObject,SIGNAL(destroyed(QObject*)),
                this,SLOT(RootObjectDeleted(void)));

        connect(RootObject,SIGNAL(OnStructureListChanged(void)),
                this,SLOT(ListChanged()));

        // repaint view if was clicked undo/redo - changes in history
        connect(RootObject->GetProject()->GetHistory(),SIGNAL( OnHistoryChanged(EHistoryChangeMessage)),
                this,SLOT(ListChanged()));

    }
    // FIXME
    beginResetModel();
    endResetModel();
}

//------------------------------------------------------------------------------

QObject*  CStructureListModel::GetItem(const QModelIndex& index) const
{
    if( ! index.isValid() ) return(NULL);
    CStructure* p_obj = static_cast<CStructure*>(index.internalPointer());
    return(p_obj);
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

QVariant CStructureListModel::headerData(int section, Qt::Orientation orientation,
                                    int role) const
{
    if( orientation != Qt::Horizontal ) return( QVariant() );
    if( role == Qt::TextAlignmentRole ) return(Qt::AlignHCenter);
    if( role != Qt::DisplayRole ) return( QVariant() );

    switch(section) {
    case 0:
        return(tr("Name"));
    case 1:
        return(tr("SID"));
    case 2:
        return(tr("Atoms"));
    case 3:
        return(tr("Description"));
    case 4:
        return(tr("ID"));
    default:
        return("");
    }
}

//------------------------------------------------------------------------------

int CStructureListModel::columnCount(const QModelIndex &parent) const
{
    return(5);
}

//------------------------------------------------------------------------------

int CStructureListModel::rowCount(const QModelIndex &parent) const
{
    if( RootObject == 0 ) return(0);
    return( RootObject->children().count() );
}

//------------------------------------------------------------------------------

QModelIndex CStructureListModel::index(int row, int column,
                                  const QModelIndex &parent) const
{
    if( RootObject == NULL ) return( QModelIndex() );

    if( ! hasIndex(row, column, parent) ) return( QModelIndex() );
    if( parent.isValid() )  return( QModelIndex() );

    QModelIndex index = createIndex(row, column, RootObject->children()[row]);

    return(index);
}

//------------------------------------------------------------------------------

QModelIndex CStructureListModel::parent(const QModelIndex &index) const
{
    // parent is always invalid
    return( QModelIndex() );
}

//------------------------------------------------------------------------------

QVariant CStructureListModel::data(const QModelIndex &index, int role) const
{
    if( RootObject == NULL ) return( QVariant() );
    if( ! index.isValid() ) return( QVariant() );

    CStructure* p_obj = static_cast<CStructure*>(index.internalPointer());

    switch(role) {
        case Qt::DisplayRole: {
            switch( index.column() ) {
            case 0:
                return(p_obj->GetName());
            case 1:
                return(p_obj->GetSeqIndex());
            case 2:
                return(p_obj->GetAtoms()->GetNumberOfAtoms());
            case 3:
                return(p_obj->GetDescription());
            case 4:
                return(p_obj->GetIndex());
            default:
                return( QVariant() );
            }
        }
        break;

        case Qt::TextAlignmentRole:
            switch( index.column() ) {
            case 0:
                return(Qt::AlignLeft);
            case 1:
                return(Qt::AlignRight);
            case 2:
                return(Qt::AlignRight);
            case 3:
                return(Qt::AlignLeft);
            case 4:
                return(Qt::AlignRight);
            default:
                return( QVariant() );
        }
        break;

        case Qt::DecorationRole: {
            switch( index.column() ) {
            case 0:{
                if( RootObject->GetActiveStructure() == p_obj ){
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

        case Qt::CheckStateRole: {
            switch( index.column() ) {
            // add checkbox into second column - where is SID
            case 1:{
                // if have structure set visible flag, then checkbox will we checked
                if (p_obj->IsFlagSet(EPOF_VISIBLE)) {
                    return Qt::Checked;
                } else {
                    return Qt::Unchecked;
                }
            }
                break;
            default:
                return( QVariant() );
            }
        }
        break;

        case Qt::ToolTipRole: {
            switch( index.column() ) {
            // add checkbox tool tip into second column - where is SID
            case 1:{
                return tr("Visible flag");
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

bool CStructureListModel::setData(const QModelIndex & index, const QVariant & value, int role)
{
    // metod for store data to the structure

    // only in role checkstate
    if (role == Qt::CheckStateRole) {

        beginResetModel();

        // take structure
        CStructure* p_obj = static_cast<CStructure*>(index.internalPointer());
        // take flags for current structure for adding
        CProObjectFlags flags = p_obj->GetFlags();

        if (value == Qt::Checked)
        {
            // add visible flag to flags
            SET_FLAG(flags,EPOF_VISIBLE,true);
        }
        if (value == Qt::Unchecked)
        {
            // add non-visible flag to flags
            SET_FLAG(flags,EPOF_VISIBLE,false);
        }
        // set new setting to object - this will emit history change and others
        p_obj->SetFlagsWH(flags);

        endResetModel();

        return (true);
    }
    return (false);
}

//------------------------------------------------------------------------------

Qt::ItemFlags CStructureListModel::flags(const QModelIndex &index) const
{
    if( !index.isValid() ) return(Qt::ItemFlags());

    if(index.column() == 1 )
    {
        // we need clickable flag for column where checkbox
        return( Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsDragEnabled | Qt::ItemIsUserCheckable );
    }
    return(Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsDragEnabled);
}

//------------------------------------------------------------------------------

QStringList CStructureListModel::mimeTypes(void) const
{
    QStringList types;
    types << RootObject->GetProjectMIMEType("structure.indexes");
    return types;
}

//------------------------------------------------------------------------------

QMimeData* CStructureListModel::mimeData(const QModelIndexList &indexes) const
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

    p_mimeData->setData(RootObject->GetProjectMIMEType("structure.indexes"), encodedData);
    return(p_mimeData);
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

void CStructureListModel::RootObjectDeleted(void)
{
    RootObject = NULL;
    // FIXME
    beginResetModel();
    endResetModel();
}

//------------------------------------------------------------------------------

void CStructureListModel::ListChanged(void)
{
    // FIXME
    beginResetModel();
    endResetModel();
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================
