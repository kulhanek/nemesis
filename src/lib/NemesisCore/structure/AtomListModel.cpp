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
#include <AtomListModel.hpp>
#include <CategoryUUID.hpp>
#include <AtomList.hpp>
#include <Atom.hpp>
#include <PeriodicTable.hpp>
#include <PhysicalQuantities.hpp>
#include <PhysicalQuantity.hpp>
#include <Residue.hpp>
#include <Project.hpp>
#include <StructureList.hpp>
#include <QMimeData>

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

QObject* AtomListModelCB(void* p_data);

//------------------------------------------------------------------------------

CExtUUID        AtomListModelID(
                    "{ATOM_LIST_MODEL:dd5c4ca8-19a2-4b3d-b9d5-f6906e760e35}",
                    "Atoms");

CPluginObject   AtomListModelObject(&NemesisCorePlugin,
                    AtomListModelID,MODEL_CAT,
                    ":/images/NemesisCore/structure/AtomList.svg",
                    AtomListModelCB);

// -----------------------------------------------------------------------------

QObject* AtomListModelCB(void* p_data)
{
    CAtomList* p_con = static_cast<CAtomList*>(p_data);
    if( p_con == NULL ) return(NULL);

    CAtomListModel* p_obj = new CAtomListModel(NULL);
    p_obj->SetRootObject(p_con);

    return(p_obj);
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

CAtomListModel::CAtomListModel(QObject* p_parent)
    : CContainerModel(&AtomListModelObject,p_parent)
{
    RootObject = NULL;

    connect(PQ_DISTANCE,SIGNAL(OnUnitChanged(void)),
            this,SLOT(ListChanged(void)));

    connect(PQ_CHARGE,SIGNAL(OnUnitChanged(void)),
            this,SLOT(ListChanged(void)));
}

//------------------------------------------------------------------------------

void CAtomListModel::SetRootObject(CAtomList* p_data)
{
    if( RootObject != NULL ) {
        if( RootObject->GetProject()->GetStructures() ){
            RootObject->GetProject()->GetStructures()->disconnect(this);
        }
        RootObject->disconnect(this);
    }

    RootObject = p_data;
    if( RootObject != NULL ) {
        connect(RootObject,SIGNAL(destroyed(QObject*)),
                this,SLOT(RootObjectDeleted(void)));

        connect(RootObject,SIGNAL(OnAtomListChanged()),
                this,SLOT(ListChanged()));

        if( RootObject->GetProject()->GetStructures() ){
            connect(RootObject->GetProject()->GetStructures(),SIGNAL(OnGeometryChangeTick(void)),
                    this,SLOT(ListChanged(void)));
        }
    }
    // FIXME
    beginResetModel();
    endResetModel();
}

//------------------------------------------------------------------------------

QObject*  CAtomListModel::GetItem(const QModelIndex& index) const
{
    if( ! index.isValid() ) return(NULL);
    CAtom* p_obj = static_cast<CAtom*>(index.internalPointer());
    return(p_obj);
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

QVariant CAtomListModel::headerData(int section, Qt::Orientation orientation,
                                    int role) const
{
    if( orientation != Qt::Horizontal ) return( QVariant() );
    if( role == Qt::TextAlignmentRole ) return(Qt::AlignHCenter);
    if( role != Qt::DisplayRole ) return( QVariant() );

    switch(section) {
        case 0:
            return(tr("SerIndx"));
        case 1:
            return(tr("LocIndx"));
        case 2:
            return(tr("Name"));
        case 3:
            return(tr("SeqIndx"));
        case 4:
            return(tr("ResName"));
        case 5:
            return(tr("Chain"));
        case 6:
            return(tr("Symbol"));
        case 7:
            return(tr("Type"));
        case 8: {
            QString col = tr("X [%1]");
            col = col.arg(PQ_DISTANCE->GetUnitName());
            return(col);
        }
        case 9: {
            QString col = tr("Y [%1]");
            col = col.arg(PQ_DISTANCE->GetUnitName());
            return(col);
        }
        case 10: {
            QString col = tr("Z [%1]");
            col = col.arg(PQ_DISTANCE->GetUnitName());
            return(col);
        }
        case 11: {
            QString col = tr("Charge [%1]");
            col = col.arg(PQ_CHARGE->GetUnitName());
            return(col);
        }
        case 12:
            return(tr("ID"));
        case 13:
            return(tr("Description"));
        default:
            return("");
    }
}

//------------------------------------------------------------------------------

int CAtomListModel::columnCount(const QModelIndex &parent) const
{
    return(14);
}

//------------------------------------------------------------------------------

int CAtomListModel::rowCount(const QModelIndex &parent) const
{
    if( RootObject == 0 ) return(0);
    return( RootObject->children().count() );
}

//------------------------------------------------------------------------------

QModelIndex CAtomListModel::index(int row, int column,
                                  const QModelIndex &parent) const
{
    if( RootObject == NULL ) return( QModelIndex() );

    if( ! hasIndex(row, column, parent) ) return( QModelIndex() );
    if( parent.isValid() )  return( QModelIndex() );

    QModelIndex index = createIndex(row, column, RootObject->children()[row]);

    return(index);
}

//------------------------------------------------------------------------------

QModelIndex CAtomListModel::parent(const QModelIndex &index) const
{
    // parent is always invalid
    return( QModelIndex() );
}

//------------------------------------------------------------------------------

QVariant CAtomListModel::data(const QModelIndex &index, int role) const
{
    if( RootObject == NULL ) return( QVariant() );
    if( ! index.isValid() ) return( QVariant() );

    CAtom* p_obj = static_cast<CAtom*>(index.internalPointer());

    switch(role) {
    case Qt::DisplayRole: {
        switch( index.column() ) {
        case 0:
            return(p_obj->GetSerIndex());
        case 1:
            return(p_obj->GetLocIndex());
        case 2:
            return(p_obj->GetName());
        case 3:{
            CResidue* p_res = p_obj->GetResidue();
            if( p_res != NULL ){
                return(p_res->GetSeqIndex());
            }
            }
            return( QVariant() );
        case 4:{
            CResidue* p_res = p_obj->GetResidue();
            if( p_res != NULL ){
                return(p_res->GetName());
            }
            }
            return( QVariant() );
        case 5:{
            CResidue* p_res = p_obj->GetResidue();
            if( p_res != NULL ){
                return(p_res->GetChain());
            }
            }
            return( QVariant() );
        case 6:
            return(PeriodicTable.GetSymbol(p_obj->GetZ()));
        case 7:
            return(p_obj->GetType());
        case 8:
            return(PQ_DISTANCE->GetRealValueText(p_obj->GetPos().x));
        case 9:
            return(PQ_DISTANCE->GetRealValueText(p_obj->GetPos().y));
        case 10:
            return(PQ_DISTANCE->GetRealValueText(p_obj->GetPos().z));
        case 11:
            return(PQ_CHARGE->GetRealValueText(p_obj->GetCharge()));
        case 12:
            return(p_obj->GetIndex());
        case 13:
            return(p_obj->GetDescription());
        default:
            return( QVariant() );
        }
        break;
    }

    case Qt::TextAlignmentRole:
        switch( index.column() ) {
            case 0:
            case 1:
                return(Qt::AlignRight);
            case 2:
                return(Qt::AlignLeft);
            case 3:
                return(Qt::AlignRight);
            case 4:
            case 5:
            case 6:
            case 7:
                return(Qt::AlignLeft);
            case 8:
            case 9:
            case 10:
            case 11:
            case 12:
                return(Qt::AlignRight);
            case 13:
                return(Qt::AlignLeft);
            default:
                return( QVariant() );
        }
        break;
    }

    return( QVariant() );
}

//------------------------------------------------------------------------------

Qt::ItemFlags CAtomListModel::flags(const QModelIndex &index) const
{
    if( !index.isValid() ) return(Qt::ItemFlags());
    return(Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsDragEnabled);
}

//------------------------------------------------------------------------------

QStringList CAtomListModel::mimeTypes(void) const
{
    QStringList types;
    types << RootObject->GetProjectMIMEType("atom.indexes");
    return types;
}

//------------------------------------------------------------------------------

QMimeData* CAtomListModel::mimeData(const QModelIndexList &indexes) const
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

    p_mimeData->setData(RootObject->GetProjectMIMEType("atom.indexes"), encodedData);
    return(p_mimeData);
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

void CAtomListModel::RootObjectDeleted(void)
{
    RootObject = NULL;
    // FIXME
    beginResetModel();
    endResetModel();
}

//------------------------------------------------------------------------------

void CAtomListModel::ListChanged(void)
{
    // FIXME
    beginResetModel();
    endResetModel();
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================


