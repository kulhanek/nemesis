// =============================================================================
// NEMESIS - Molecular Modelling Package
// -----------------------------------------------------------------------------
//    Copyright (C) 2010        Petr Kulhanek, kulhanek@chemi.muni.cz
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

#include <WorkPanelModel.hpp>
#include <NemesisCoreModule.hpp>
#include <CategoryUUID.hpp>
#include <WorkPanelList.hpp>

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

QObject* WorkPanelModelCB(void* p_data);

CExtUUID        WorkPanelModelID(
                    "{WORK_PANEL_LIST_MODEL:e9ee1124-0177-4058-b35f-b2de29e3bba6}",
                    "Work panel list model");

CPluginObject   WorkPanelModelObject(&NemesisCorePlugin,
                     WorkPanelModelID,MODEL_CAT,
                     WorkPanelModelCB);

//------------------------------------------------------------------------------

QObject* WorkPanelModelCB(void* p_data)
{
    CWorkPanelList* p_con = static_cast<CWorkPanelList*>(p_data);
    if( p_con == NULL ) return(NULL);

    CWorkPanelModel* p_obj = new CWorkPanelModel(NULL);
    p_obj->SetRootObject(p_con);

    return(p_obj);
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

CWorkPanelModel::CWorkPanelModel(QObject* p_parent)
    : CContainerModel(&WorkPanelModelObject,p_parent)
{
    RootObject = NULL;
}

//------------------------------------------------------------------------------

void CWorkPanelModel::SetRootObject(CWorkPanelList* p_data)
{
    RootObject = p_data;
    if( RootObject != NULL ) {
        connect(RootObject,SIGNAL(OnWorkPanelAdded(CWorkPanel*)),
                this,SLOT(WorkPanelAdded(CWorkPanel*)));
        connect(RootObject,SIGNAL(OnWorkPanelRemoved(CWorkPanel*)),
                this,SLOT(WorkPanelRemoved(CWorkPanel*)));
    }
    // FIXME
    beginResetModel();
    endResetModel();
}

//------------------------------------------------------------------------------

QObject* CWorkPanelModel::GetItem(const QModelIndex& index) const
{
    if( ! index.isValid() ) return(NULL);
    return( static_cast<QObject*>(index.internalPointer()));
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

QVariant CWorkPanelModel::headerData(int section, Qt::Orientation orientation,
                                     int role) const
{
    if( role != Qt::DisplayRole ) return( QVariant() );
    if( orientation != Qt::Horizontal ) return( QVariant() );

    switch(section) {
    case 0:
        return(tr("Name"));
    case 1:
        return(tr("Description"));
    default:
        return("");
    }
}

//------------------------------------------------------------------------------

int CWorkPanelModel::columnCount(const QModelIndex &parent) const
{
    return(2);
}

//------------------------------------------------------------------------------

int CWorkPanelModel::rowCount(const QModelIndex &parent) const
{
    if( RootObject == 0 ) return(0);
    return( RootObject->WorkPanels.count() );
}

//------------------------------------------------------------------------------

QModelIndex CWorkPanelModel::index(int row, int column,
                                   const QModelIndex &parent) const
{
    if( RootObject == NULL ) return( QModelIndex() );

    if( ! hasIndex(row, column, parent) ) return( QModelIndex() );
    if( parent.isValid() )  return( QModelIndex() );

    QModelIndex index = createIndex(row, column, RootObject->WorkPanels[row]);

    return(index);
}

//------------------------------------------------------------------------------

QModelIndex CWorkPanelModel::parent(const QModelIndex &index) const
{
    // parent is always invalid
    return( QModelIndex() );
}

//------------------------------------------------------------------------------

QVariant CWorkPanelModel::data(const QModelIndex &index, int role) const
{
    if( RootObject == NULL ) return( QVariant() );
    if( ! index.isValid() ) return( QVariant() );

    CWorkPanel* p_obj = static_cast<CWorkPanel*>(index.internalPointer());

    switch(role) {
    case Qt::DisplayRole: {
        switch( index.column() ) {
        case 0:
            return(p_obj->GetPluginObject()->GetName());
        case 1:
            return(p_obj->GetPluginObject()->GetDescription());
        default:
            return( QVariant() );
        }
        break;
    }

    case Qt::DecorationRole: {
        switch( index.column() ) {
        case 0:
            return(p_obj->GetPluginObject()->GetIcon());
        default:
            return( QVariant() );
        }
        break;
    }
    }

    return( QVariant() );
}

//------------------------------------------------------------------------------

Qt::ItemFlags CWorkPanelModel::flags(const QModelIndex &index) const
{
    if( !index.isValid() ) return(Qt::ItemFlags());
    return(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

void CWorkPanelModel::WorkPanelAdded(CWorkPanel* p_wp)
{
    // FIXME
    beginResetModel();
    endResetModel();
}

//------------------------------------------------------------------------------

void CWorkPanelModel::WorkPanelRemoved(CWorkPanel* p_wp)
{
    // FIXME
    beginResetModel();
    endResetModel();
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================



