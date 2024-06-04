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

#include <ProjectListModel.hpp>
#include <NemesisCoreModule.hpp>
#include <CategoryUUID.hpp>
#include <ProjectList.hpp>
#include <Project.hpp>

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

QObject* ProjectListModelCB(void* p_data);

CExtUUID        ProjectListModelID(
                    "{PROJECT_LIST_MODEL:f879822f-5e08-4686-947b-abb6af144789}",
                    "Projects");

CPluginObject   ProjectListModelObject(&NemesisCorePlugin,
                    ProjectListModelID,MODEL_CAT,
                    ":/images/NemesisCore/project/ProjectList.svg",
                    ProjectListModelCB);

//------------------------------------------------------------------------------

QObject* ProjectListModelCB(void* p_data)
{
    CProjectList* p_con = static_cast<CProjectList*>(p_data);
    if( p_con == NULL ) return(NULL);

    CProjectListModel* p_obj = new CProjectListModel(NULL);
    p_obj->SetRootObject(p_con);

    return(p_obj);
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

CProjectListModel::CProjectListModel(QObject* p_parent)
    : CContainerModel(&ProjectListModelObject,p_parent)
{
    RootObject = NULL;
}

//------------------------------------------------------------------------------

void CProjectListModel::SetRootObject(CProjectList* p_data)
{
    if( RootObject != NULL ) {
        RootObject->disconnect(this);
    }

    RootObject = p_data;

    if( RootObject != NULL ) {
        connect(RootObject,SIGNAL(destroyed(QObject*)),
                this,SLOT(RootObjectDeleted(void)));
        connect(RootObject,SIGNAL(OnProjectAdded(CProject*)),
                this,SLOT(UpdateModel(void)));
        connect(RootObject,SIGNAL(OnProjectRemoved(CProject*)),
                this,SLOT(UpdateModel(void)));
    }

    // FIXME
    beginResetModel();
    endResetModel();
}

//------------------------------------------------------------------------------

QObject* CProjectListModel::GetItem(const QModelIndex& index) const
{
    if( ! index.isValid() ) return(NULL);
    return( static_cast<QObject*>(index.internalPointer()));
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

QVariant CProjectListModel::headerData(int section, Qt::Orientation orientation,
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

int CProjectListModel::columnCount(const QModelIndex &parent) const
{
    return(2);
}

//------------------------------------------------------------------------------

int CProjectListModel::rowCount(const QModelIndex &parent) const
{
    if( RootObject == 0 ) return(0);
    return( RootObject->children().count() );
}


//------------------------------------------------------------------------------

QModelIndex CProjectListModel::index(int row, int column,
                                     const QModelIndex &parent) const
{
    if( RootObject == NULL ) return( QModelIndex() );

    if( ! hasIndex(row, column, parent) ) return( QModelIndex() );
    if( parent.isValid() )  return( QModelIndex() );

    const QObjectList& list = RootObject->children();
    QModelIndex index = createIndex(row, column, list[row]);

    return(index);
}

//------------------------------------------------------------------------------

QModelIndex CProjectListModel::parent(const QModelIndex &index) const
{
    // parent is always invalid
    return( QModelIndex() );
}

//------------------------------------------------------------------------------

QVariant CProjectListModel::data(const QModelIndex &index, int role) const
{
    if( RootObject == NULL ) return( QVariant() );
    if( ! index.isValid() ) return( QVariant() );

    CProject* p_obj = static_cast<CProject*>(index.internalPointer());

    switch(role) {
    case Qt::DisplayRole: {
        switch( index.column() ) {
        case 0:
            return(p_obj->GetName());
        case 1:
            return(p_obj->GetDescription());
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

Qt::ItemFlags CProjectListModel::flags(const QModelIndex &index) const
{
    if( !index.isValid() ) return(Qt::ItemFlags());
    return(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

void CProjectListModel::RootObjectDeleted(void)
{
    RootObject = NULL;
    // FIXME
    beginResetModel();
    endResetModel();
}

//------------------------------------------------------------------------------

void CProjectListModel::UpdateModel(void)
{
    // FIXME
    beginResetModel();
    endResetModel();
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================



