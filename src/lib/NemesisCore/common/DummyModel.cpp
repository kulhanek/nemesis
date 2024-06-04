// =============================================================================
// NEMESIS - Molecular Modelling Package
// -----------------------------------------------------------------------------
//    Copyright (C) 2012        Petr Kulhanek, kulhanek@chemi.muni.cz
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

#include <DummyModel.hpp>
#include <NemesisCoreModule.hpp>
#include <CategoryUUID.hpp>

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

QObject* DummyModelCB(void* p_data);

CExtUUID        DummyModelID(
                    "{DUMMY_MODEL:8a8e91a6-28b7-4f68-9f08-24413d0eb170}",
                    "Dummy model");

CPluginObject   DummyModelObject(&NemesisCorePlugin,
                    DummyModelID,MODEL_CAT,
                    DummyModelCB);

//------------------------------------------------------------------------------

QObject* DummyModelCB(void* p_data)
{
    CDummyModel* p_obj = new CDummyModel(NULL);
    return(p_obj);
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

CDummyModel::CDummyModel(QObject* p_parent)
    : CContainerModel(&DummyModelObject,p_parent)
{

}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

QVariant CDummyModel::headerData(int section, Qt::Orientation orientation,
                                       int role) const
{
    if( role != Qt::DisplayRole ) return( QVariant() );
    if( orientation != Qt::Horizontal ) return( QVariant() );

    switch(section) {
    case 0:
        return(tr("No data"));
    default:
        return("");
    }
}

//------------------------------------------------------------------------------

int CDummyModel::columnCount(const QModelIndex &parent) const
{
    return(1);
}

//------------------------------------------------------------------------------

int CDummyModel::rowCount(const QModelIndex &parent) const
{
    return( 0 );
}


//------------------------------------------------------------------------------

QModelIndex CDummyModel::index(int row, int column,
                                     const QModelIndex &parent) const
{
    if( ! hasIndex(row, column, parent) ) return( QModelIndex() );
    if( parent.isValid() )  return( QModelIndex() );

    QModelIndex index = createIndex(row, column, (void*)NULL);

    return(index);
}

//------------------------------------------------------------------------------

QModelIndex CDummyModel::parent(const QModelIndex &index) const
{
    // parent is always invalid
    return( QModelIndex() );
}

//------------------------------------------------------------------------------

QVariant CDummyModel::data(const QModelIndex &index, int role) const
{
    if( ! index.isValid() ) return( QVariant() );
    return( QVariant() );
}

//------------------------------------------------------------------------------

Qt::ItemFlags CDummyModel::flags(const QModelIndex &index) const
{
    if( !index.isValid() ) return(Qt::ItemFlags());
    return(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================
