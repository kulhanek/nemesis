#ifndef PropertyH
#define PropertyH
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

#include <NemesisCoreMainHeader.hpp>
#include <ProObject.hpp>
#include <QVector>
#include <GeoMeasurement.hpp>

// -----------------------------------------------------------------------------

class CPropertyList;
class CPhysicalQuantity;
class CStructure;

// -----------------------------------------------------------------------------

// property read-only flags

enum EPropertyFlag {
        EPF_SCALAR_VALUE        = 0x00000001,   // scalar property
        EPF_VECTOR_VALUE        = 0x00000002,   // vector property
        EPF_FIELD_VALUE         = 0x00000004,   // field property
        EPF_CARTESIAN_GRADIENT  = 0x00000008    // scalar property has cartesian gradient
};

typedef QFlags<EPropertyFlag> CPropertyFlags;

// -----------------------------------------------------------------------------

///  Property definition class

class NEMESIS_CORE_PACKAGE CProperty : public CProObject {
Q_OBJECT
public:
// constructors and destructors ------------------------------------------------
    /// constructor.
    CProperty(CPluginObject* p_po,CPropertyList *p_bl);
    
    /// destructor - <b>use</b> RemoveFromBaseList instead.
    ~CProperty(void);

    /// deletes property with history note.
    void RemoveFromBaseList(CHistoryNode* p_history=NULL);

// methods with changes recorded into history list -----------------------------
    /// delete property
    bool DeletePropertyWH(void);

    /// restrain property
    bool RestrainPropertyWH(void);

// executive methods -----------------------------------------------------------
    /// set name
    virtual void SetName(const QString& name,CHistoryNode* p_history=NULL);

    /// set description
    virtual void SetDescription(const QString& descrip,CHistoryNode* p_history=NULL);

    /// restrain property
    void RestrainProperty(CHistoryNode* p_history=NULL);

// informational methods -------------------------------------------------------
    /// get base list
    CPropertyList*      GetPropertyList(void) const;

    /// get property type
    const QString&      GetPropertyType(void) const;

    /// get property unit
    CPhysicalQuantity*  GetPropertyUnit(void) const;

    /// is property completed?
    virtual bool IsReady(void);

    /// is property from given structure?
    virtual bool IsFromStructure(CStructure* p_str);

    /// composed from single atom groups?
    virtual bool ComposedBySingleAtomGroups(void);

    /// has cartesian gradient for given structure
    virtual bool HasGradient(CStructure* p_structure);

    // with generic support we need to know only a value of scalar property
    // other property types can use such single value to report a magnitude of vector, etc.

    /// get property value - scalar value
    virtual double GetScalarValue(void);

    /// get property diviation from target_value - scalar value
    virtual double GetScalarDeviation(double target_value);

    /// get property cartesian gradient and its value
    virtual double GetGradient(QVector<CAtomGrad>& grads);

    /// get property cartesian gradient and its deviation
    virtual double GetDeviationAndGradient(QVector<CAtomGrad>& grads,double target_value);

    /// set property value - scalar value (for trajectory update)
    virtual void SetScalarValue(double value);

// input/output methods --------------------------------------------------------
    /// load atom data
    virtual void LoadData(CXMLElement* p_ele);

    /// save atom data
    virtual void SaveData(CXMLElement* p_ele);

// section of private data ----------------------------------------------------
protected:
    CPhysicalQuantity*  PropUnit;
    CPropertyFlags      PropFlags;
};

// -----------------------------------------------------------------------------

#endif

