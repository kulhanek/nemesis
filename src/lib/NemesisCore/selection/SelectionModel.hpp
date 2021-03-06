#ifndef SelectionModelH
#define SelectionModelH
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

#include <NemesisCoreMainHeader.hpp>
#include <ContainerModel.hpp>
#include <GLSelection.hpp>

// -----------------------------------------------------------------------------

class CSelectionList;
class CProObject;

// -----------------------------------------------------------------------------

class NEMESIS_CORE_PACKAGE CSelectionModel : public CContainerModel {
    Q_OBJECT
public:
// constructors and destructors -----------------------------------------------
    CSelectionModel(QObject* p_parent);

    /// set root object
    void SetRootObject(CSelectionList* p_data);

    /// return object part of SelObject item from model index
    virtual QObject* GetItem(const QModelIndex& index) const;

    /// return whole sel object item from model index
    CSelObject* GetFullItem(const QModelIndex& index);

// section of private data ----------------------------------------------------
private:
    CSelectionList*    RootObject;

    QVariant headerData(int section, Qt::Orientation orientation,
                        int role = Qt::DisplayRole) const;

    int columnCount(const QModelIndex &parent) const;
    int rowCount(const QModelIndex &parent) const;

    QModelIndex index(int row, int column,
                      const QModelIndex &parent) const;
    QModelIndex parent(const QModelIndex &index) const;

    QVariant data(const QModelIndex &index, int role) const;
    Qt::ItemFlags flags(const QModelIndex &index) const;

private slots:
    void ObjectAdded(const CSelObject& obj);
    void ObjectRemoved(const CSelObject& obj);
};

// -----------------------------------------------------------------------------

#endif
