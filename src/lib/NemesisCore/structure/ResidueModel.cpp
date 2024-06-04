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
#include <ResidueModel.hpp>
#include <CategoryUUID.hpp>
#include <Residue.hpp>
#include <PhysicalQuantities.hpp>
#include <PhysicalQuantity.hpp>
#include <PeriodicTable.hpp>
#include <Atom.hpp>
#include <QMimeData>

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

QObject* ResidueModelCB(void* p_data);

//------------------------------------------------------------------------------

CExtUUID        ResidueModelID(
                    "{RESIDUE_MODEL:0e7e5995-596e-46dc-a65b-7f3eb38427fa}",
                    "Residue");

CPluginObject   ResidueModelObject(&NemesisCorePlugin,
                    ResidueModelID,MODEL_CAT,
                    ":/images/NemesisCore/structure/Residue.svg",
                    ResidueModelCB);

// -----------------------------------------------------------------------------

QObject* ResidueModelCB(void* p_data)
{
    CResidue* p_con = static_cast<CResidue*>(p_data);
    if( p_con == NULL ) return(NULL);

    CResidueModel* p_obj = new CResidueModel(NULL);
    p_obj->SetRootObject(p_con);

    return(p_obj);
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

CResidueModel::CResidueModel(QObject* p_parent)
    : CContainerModel(&ResidueModelObject,p_parent)
{
    RootObject = NULL;

    connect(PQ_DISTANCE,SIGNAL(OnUnitChanged(void)),
            this,SLOT(DistanceUnitChanged(void)));

    connect(PQ_CHARGE,SIGNAL(OnUnitChanged(void)),
            this,SLOT(ChargeUnitChanged(void)));
}

//------------------------------------------------------------------------------

void CResidueModel::SetRootObject(CResidue* p_data)
{
    if( RootObject != NULL ) {
        RootObject->disconnect(this);
    }

    RootObject = p_data;
    if( RootObject != NULL ) {
        connect(RootObject,SIGNAL(destroyed(QObject*)),
                this,SLOT(RootObjectDeleted(void)));

        connect(RootObject,SIGNAL(OnAtomListChanged(void)),
                this,SLOT(ResidueChanged(void)));
    }
    // FIXME
    beginResetModel();
    endResetModel();
}

//------------------------------------------------------------------------------

QObject*  CResidueModel::GetItem(const QModelIndex& index) const
{
    if( ! index.isValid() ) return(NULL);
    CResidue* p_obj = static_cast<CResidue*>(index.internalPointer());
    return(p_obj);
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

QVariant CResidueModel::headerData(int section, Qt::Orientation orientation,
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
            return(tr("Symbol"));
        case 4:
            return(tr("Type"));
        case 5: {
            QString col = tr("X [%1]");
            col = col.arg(PQ_DISTANCE->GetUnitName());
            return(col);
        }
        case 6: {
            QString col = tr("Y [%1]");
            col = col.arg(PQ_DISTANCE->GetUnitName());
            return(col);
        }
        case 7: {
            QString col = tr("Z [%1]");
            col = col.arg(PQ_DISTANCE->GetUnitName());
            return(col);
        }
        case 8: {
            QString col = tr("Charge [%1]");
            col = col.arg(PQ_CHARGE->GetUnitName());
            return(col);
        }
        case 9:
            return(tr("ID"));
        case 10:
            return(tr("Description"));
        default:
            return("");
    }
}

//------------------------------------------------------------------------------

int CResidueModel::columnCount(const QModelIndex &parent) const
{
    return(11);
}

//------------------------------------------------------------------------------

int CResidueModel::rowCount(const QModelIndex &parent) const
{
    if( RootObject == 0 ) return(0);
    return( RootObject->Atoms.count() );
}

//------------------------------------------------------------------------------

QModelIndex CResidueModel::index(int row, int column,
                                  const QModelIndex &parent) const
{
    if( RootObject == NULL ) return( QModelIndex() );

    if( ! hasIndex(row, column, parent) ) return( QModelIndex() );
    if( parent.isValid() )  return( QModelIndex() );

    QModelIndex index = createIndex(row, column, RootObject->Atoms[row] );

    return(index);
}

//------------------------------------------------------------------------------

QModelIndex CResidueModel::parent(const QModelIndex &index) const
{
    // parent is always invalid
    return( QModelIndex() );
}

//------------------------------------------------------------------------------

QVariant CResidueModel::data(const QModelIndex &index, int role) const
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
        case 3:
            return(PeriodicTable.GetSymbol(p_obj->GetZ()));
        case 4:
            return(p_obj->GetType());
        case 5:
            return(PQ_DISTANCE->GetRealValueText(p_obj->GetPos().x));
        case 6:
            return(PQ_DISTANCE->GetRealValueText(p_obj->GetPos().y));
        case 7:
            return(PQ_DISTANCE->GetRealValueText(p_obj->GetPos().z));
        case 8:
            return(PQ_CHARGE->GetRealValueText(p_obj->GetCharge()));
        case 9:
            return(p_obj->GetIndex());
        case 10:
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
            case 4:
                return(Qt::AlignLeft);
            case 5:
            case 6:
            case 7:
            case 8:
            case 9:
                return(Qt::AlignRight);
            case 10:
                return(Qt::AlignLeft);
            default:
                return( QVariant() );
        }
        break;
    }

    return( QVariant() );
}

//------------------------------------------------------------------------------

Qt::ItemFlags CResidueModel::flags(const QModelIndex &index) const
{
    if( !index.isValid() ) return(Qt::ItemFlags());
    return(Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsDragEnabled);
}


//------------------------------------------------------------------------------

QStringList CResidueModel::mimeTypes(void) const
{
    QStringList types;
    types << RootObject->GetProjectMIMEType("atom.indexes");
    return types;
}

//------------------------------------------------------------------------------

QMimeData* CResidueModel::mimeData(const QModelIndexList &indexes) const
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

void CResidueModel::RootObjectDeleted(void)
{
    RootObject = NULL;
    // FIXME
    beginResetModel();
    endResetModel();
}

//------------------------------------------------------------------------------

void CResidueModel::ResidueChanged(void)
{
    // FIXME
    beginResetModel();
    endResetModel();
}

//------------------------------------------------------------------------------

void CResidueModel::DistanceUnitChanged(void)
{
    // FIXME
    beginResetModel();
    endResetModel();
}

//------------------------------------------------------------------------------

void CResidueModel::ChargeUnitChanged(void)
{
    // FIXME
    beginResetModel();
    endResetModel();
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================


