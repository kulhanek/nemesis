#ifndef AtomH
#define AtomH
// =============================================================================
// NEMESIS - Molecular Modelling Package
// -----------------------------------------------------------------------------
//    Copyright (C) 2024 Petr Kulhanek, kulhanek@chemi.muni.cz
//    Copyright (C) 2010 Petr Kulhanek, kulhanek@chemi.muni.cz
//    Copyright (C) 2009 Petr Kulhanek, kulhanek@chemi.muni.cz,
//                       Jakub Stepan, xstepan3@chemi.muni.cz
//    Copyright (C) 2008 Petr Kulhanek, kulhanek@enzim.hu,
//                       Jakub Stepan, xstepan3@chemi.muni.cz
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

#include <NemesisCoreMainHeader.hpp>
#include <Point.hpp>
#include <ProObject.hpp>
#include <Bond.hpp>
#include <GeoDescriptor.hpp>

// -----------------------------------------------------------------------------

class CProject;
class CAtomList;
class CStructure;
class CBond;
class CXMLElement;
class CResidue;
class CSnapshot;

// -----------------------------------------------------------------------------

extern CExtUUID NEMESIS_CORE_PACKAGE AtomID;

// -----------------------------------------------------------------------------

///  Atom definition class
/*!
* The class defines work with atom data type, each atom is member of an AtomList.
* Use history to apply changes.
*/

class NEMESIS_CORE_PACKAGE CAtom : public CProObject {
Q_OBJECT
public:
// constructors and destructors -----------------------------------------------

    /// Constructor.
    /*!
     * \param 
     * CAtom object has to be coonected to an CAtomList.
    */
    CAtom(CAtomList *p_bl);

    CAtom(CAtomList *p_bl,bool no_index);
    
    /// Destructor - <b>use</b> RemoveFromBaseList instead.
    /*!
     * Do not use this function.
    */
    ~CAtom(void);

    /// Deletes atom with history note.
    /*!
    * This function should be used instead of destructor.
    */
    void RemoveFromBaseList(CHistoryNode* p_history=NULL);

// methods with changes registered into history list ---------------------------
    /// set user defined atom index number
    /*!
     \param ser_idx new serial index.
     \return true if completed succesfully
     \sa GetSerIndex() and set function without history note SetSerIndex().
    */
    bool   SetSerIndexWH(int ser_idx);

    /// set user defined atom index number
    /*!
     \param loc_idx new local index.
     \return true if completed succesfully
     \sa GetLocIndex() and set function without history note SetLocIndex().
    */
    bool   SetLocIndexWH(int loc_idx);

    /// set atom type
    /*!
     \param atom_type new atom type.
     \return true if completed succesfully
     \sa GetType() and set function without history note SetType().
    */
    bool   SetTypeWH(const QString& atom_type);

    /// set atom proton number
     /*!
     \param z new proton number.
     \return true if completed succesfully
     \sa GetZ().
    */
    bool   SetZWH(int newz);

    /// set atom charge
    bool   SetChargeWH(double charge);

    /// set atom position
     /*!
     \param position of atom.
     \return true if completed succesfully
     \sa GetPos().
     */
    bool   SetPosWH(const CPoint& pos); 

    /// set atom velocity
    bool   SetVelWH(const CPoint& vel/**< [in] new velocity. */); ///< Set an atom velocity.

    /// delete atom
    bool   DeleteWH(void);

    /// make bond with another atom
    bool   MakeBondWithWH(CAtom *p_atom,EBondOrder order);

    /// set new residue for atom
    bool   SetResidueWH(CResidue* p_res);

    /// add a new valence
    bool   AddValenceWH(void);

// informational methods ------------------------------------------------------
    /// get molecule
    CStructure*         GetStructure(void) const;

    /// get base list
    CAtomList*          GetAtoms(void) const;

    /// get residue
    CResidue*           GetResidue(void) const;

    /// get serial index
    int                 GetSerIndex(void) const;

    /// get local index
    int                 GetLocIndex(void) const;

    /// get trajectory atom index
    int                 GetTrajIndex(void) const;

    /// get atom type
    const QString&      GetType(void) const;

    /// get atom proton number
    int                 GetZ(void) const;

    /// get atom partial charge
    double              GetCharge(void) const;

    /// get atom mass
    double              GetMass(void) const;

    /// get atom position
    const CPoint&       GetPos(void) const;

    /// get base atom position
    const CPoint&       GetBasePos(void) const;

    /// get atom velocities
    const CPoint&       GetVel(void) const;

    /// get base atom velocities
    const CPoint&       GetBaseVel(void) const;

    /// is atom virtual
    /*! An atom is virtual if Z == 0 or Z == 1. */
    bool                IsVirtual(void);

    /// is atom connected with other atom
    CBond*              IsBondedWith(const CAtom* p_a2);

    /// return valence bond, if it exists
    CBond*              GetConnectBond(void);

    /// get object descriptor
    virtual CGeoDescriptor  GetGeoDescriptor(void);

// executive methods without change registration to history list --------------
    /// set name
    virtual void SetName(const QString& name,CHistoryNode* p_history=NULL);

    /// set description
    virtual void SetDescription(const QString& descrip,CHistoryNode* p_history=NULL);

    ///  set serial index
    void SetSerIndex(int atom_number,CHistoryNode* p_history=NULL);

    /// set local index
    void SetLocIndex(int loc_idx,CHistoryNode* p_history=NULL);

    /// set atom type
    void SetType(const QString& atomtype,CHistoryNode* p_history=NULL);

    /// set atom proton number
    void SetZ(int z,CHistoryNode* p_history=NULL);

    /// set atom partial charge
    void SetCharge(double charge,CHistoryNode* p_history=NULL);

    /// set atom position
    void SetPos(const CPoint& pos,CHistoryNode* p_history=NULL);

    /// set manipulation dmove - used by Optimizers and CAtomManipRelaxMouseDriver
    void SetManipDMove(const CPoint& dmov);

    /// was the atom moved?
    bool IsManipDMoved(void);

    /// get manip dmove
    const CPoint GetManipDMove(void);

    /// set atom velocities
    void SetVel(const CPoint& vel,CHistoryNode* p_history=NULL);

    /// set atom trajectory index
    void SetTrajIndex(int traj_idx);

    /// change parent, see CAtomList::MoveAllAtomsFrom
    void ChangeParent(CAtomList* p_newparent);

// bond related methods -------------------------------------------------------
    /// register bond connected to atom
    bool RegisterBond(CBond *p_bond);

    /// unregister bond formely connected to atom
    bool UnregisterBond(CBond *p_bond);

    /// return number of registered bonds
    int GetNumberOfBonds(void) const;

    /// return list of bonds
    const QList<CBond*>&   GetBonds(void) const;

    /// return model for registered bonds
    CContainerModel* GetRegBondsContainerModel(QObject* p_parent);

    /// get associated snapshot
    CSnapshot*      GetSnapshot(void) const;

// input/output methods -------------------------------------------------------
    /// load atom data
    virtual void LoadData(CXMLElement* p_ele);

    /// save atom data
    virtual void SaveData(CXMLElement* p_ele);

// -----------------------------------------------------------------------------
public:
    /// emit OnStatusChanged signal
    void EmitOnStatusChanged(void);

signals:
    /// emmited when bond is registered
    void OnBondRegistered(CBond* p_bond);

    /// emmited when bond is unregistered
    void OnBondUnregistered(CBond* p_bond);

// section of private data ----------------------------------------------------
private:
    CResidue*           Residue;
    int                 SerIndex;           ///< atom number
    int                 LocIndex;           ///< atom number
    int                 Z;                  ///< proton number
    QString             AtomType;           ///< atom type
    double              Charge;             ///< atom charge
    CPoint              Pos;                ///< position
    CPoint              Vel;                ///< velocity
    QList<CBond*>       Bonds;              /*!< bond list */
    int                 TrajIndex;          ///< trajectory index for accessing data in snapshot

    /// partial move
    CPoint              DMove;
    bool                DMoveSet;

    /// helper method
    CBond*  RemoveBondFromBegin(void);

    friend class CResidue;
    friend class CAtomList;
};

// -----------------------------------------------------------------------------

#endif

