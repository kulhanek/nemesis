// =============================================================================
// NEMESIS - Molecular Modelling Package
// -----------------------------------------------------------------------------
//    Copyright (C) 2011 Jiri Klusak, anim4tor@mail.muni.cz
//    Copyright (C) 2010 Petr Kulhanek, kulhanek@chemi.muni.cz
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

#include <AtomList.hpp>
#include <Structure.hpp>
#include <XMLElement.hpp>
#include <ErrorSystem.hpp>
#include <NemesisCoreModule.hpp>
#include <CategoryUUID.hpp>
#include <PeriodicTable.hpp>

#include <Project.hpp>
#include <ProjectList.hpp>

#include <Graphics.hpp>
#include <GraphicsObjectList.hpp>
#include <BoxObject.hpp>
#include <PlaneObject.hpp>
#include <VectorObject.hpp>

#include <PrincipalAxes.hpp>
#include <GeoDescriptor.hpp>
#include <AtomListHistory.hpp>
#include <AtomHistory.hpp>
#include <Atom.hpp>
#include <OpenBabelUtils.hpp>
#include <BondList.hpp>
#include <ResidueList.hpp>
#include <Residue.hpp>
#include <StructureList.hpp>

#include <queue>

#include <openbabel/obfunctions.h>

using namespace std;

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

CExtUUID        AtomListID(
                    "{ATOM_LIST:405bc543-3852-4b5d-aa2a-60f79ce1f0c4}",
                    "Atoms");

CPluginObject   AtomListObject(&NemesisCorePlugin,
                    AtomListID,STRUCTURE_CAT,
                    ":/images/NemesisCore/structure/AtomList.svg",
                    NULL);

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

CAtomList::CAtomList(CStructure* p_str)
    : CProObject(&AtomListObject,p_str,p_str->GetProject())
{
    TopIndex.SetIndex(1);

    Changed = false;
    UpdateLevel = 0;
    ForceSorting = false;
    Snapshot = NULL;
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

CAtom* CAtomList::CreateAtomWH(int z,const CPoint& pos)
{
    CAtom* p_atom = NULL;

    CHistoryNode* p_history = BeginChangeWH(EHCL_TOPOLOGY,tr("add atom"));
    if( p_history == NULL ) return (p_atom);

    BeginUpdate(); // essential - disable sorting

    p_atom = CreateAtom(z,pos,p_history);

    EndUpdate();

    EndChangeWH();
    return(p_atom);
}

//------------------------------------------------------------------------------

bool CAtomList::AddHydrogensWH(bool polar_only)
{
    CHistoryNode* p_history = BeginChangeWH(EHCL_TOPOLOGY,tr("add missing hydrogens"));
    if( p_history == NULL ) return (false);

    // protonate structure by openbabel
    OpenBabel::OBMol obmol;
    COpenBabelUtils::Nemesis2OpenBabel(GetStructure(),obmol,true);

    // assign typical number of implicit hydrogens
    obmol.SetHydrogensAdded(false);
    for(unsigned int i= 1; i <= obmol.NumAtoms(); i++) {
        OpenBabel::OBAtom* p_obatom = obmol.GetAtom(i);
        OpenBabel::OBAtomAssignTypicalImplicitHydrogens(p_obatom);
    }

    unsigned int top_atom = obmol.NumAtoms();

    // newly added atoms are added to the end of molecule
    obmol.AddHydrogens(polar_only,false);

    BeginUpdate(); // essential - disable sorting

    // add new hydrogens
    for(unsigned int i= top_atom + 1; i <= obmol.NumAtoms(); i++) {
        OpenBabel::OBAtom* p_obatom1 = obmol.GetAtom(i);

        // atom
        CPoint  pos(p_obatom1->GetX(),p_obatom1->GetY(),p_obatom1->GetZ());
        CAtom* p_atom1 = CreateAtom(1,pos,p_history);

        // second atom
        OpenBabel::OBBondIterator obi;
        OpenBabel::OBAtom* p_obatom2 = p_obatom1->BeginNbrAtom(obi);
        CAtom* p_atom2 = static_cast<CAtom*>(children().at(p_obatom2->GetIdx()-1));

        // add to coresponding residue
        if( p_atom2->GetResidue() ){
            p_atom2->GetResidue()->AddAtom(p_atom1,p_history);
        }

        // bond
        GetStructure()->GetBonds()->CreateBond(p_atom1,p_atom2,BO_SINGLE,p_history);
    }

    EndUpdate();

    EndChangeWH();
    return(true);
}

//------------------------------------------------------------------------------

bool CAtomList::RemoveHydrogensWH(void)
{
    CHistoryNode* p_history = BeginChangeWH(EHCL_TOPOLOGY,tr("delete all hydrogens"));
    if( p_history == NULL ) return (false);

    BeginUpdate();

    foreach(QObject* p_qobj,children()) {
        CAtom* p_atom = static_cast<CAtom*>(p_qobj);
        if( p_atom->IsVirtual() == true ) {
            p_atom->RemoveFromBaseList(p_history);
        }
    }

    EndUpdate();

    EndChangeWH();
    return(true);
}

//------------------------------------------------------------------------------

bool CAtomList::RemoveSelectedAtomsWH(void)
{
    CHistoryNode* p_history = BeginChangeWH(EHCL_TOPOLOGY,tr("update serial indexes"));
    if( p_history == NULL ) return (false);

    BeginUpdate();

    foreach(QObject* p_qobj,children()) {
        CAtom* p_atom = static_cast<CAtom*>(p_qobj);
        if( p_atom->IsFlagSet(EPOF_SELECTED) ||
            (p_atom->GetResidue() && p_atom->GetResidue()->IsFlagSet(EPOF_SELECTED)) ){
            p_atom->RemoveFromBaseList(p_history);
        }
    }

    EndUpdate();
    EndChangeWH();
    return(true);
}

//------------------------------------------------------------------------------

bool CAtomList::SerialIndexBySeqAndLocIndexesWH(void)
{
    CHistoryNode* p_history = BeginChangeWH(EHCL_TOPOLOGY,tr("update serial indexes"));
    if( p_history == NULL ) return (false);

    BeginUpdate();

    // sort residues
    GetStructure()->GetResidues()->SortResidues();

    int indx = 1;
    foreach(QObject* p_qobj, GetStructure()->GetResidues()->children()){
        CResidue* p_res = static_cast<CResidue*>(p_qobj);
        foreach(CAtom* p_atom, p_res->GetAtoms()){
            p_atom->SetSerIndex(indx,p_history);
            indx++;
        }
    }

    GetStructure()->GetAtoms()->SortAtoms();

    EndUpdate();
    EndChangeWH();
    return(true);
}

//------------------------------------------------------------------------------

bool CAtomList::FreezeAllAtomsWH(void)
{
    CHistoryNode* p_history = BeginChangeWH(EHCL_TOPOLOGY,tr("freeze all atoms"));
    if( p_history == NULL ) return (false);

    FreezeAllAtoms(p_history);

    EndChangeWH();
    return(true);
}

//------------------------------------------------------------------------------

bool CAtomList::FreezeHeavyAtomsWH(void)
{
    CHistoryNode* p_history = BeginChangeWH(EHCL_TOPOLOGY,tr("freeze heavy atoms"));
    if( p_history == NULL ) return (false);

    FreezeHeavyAtoms(p_history);

    EndChangeWH();
    return(true);
}

//------------------------------------------------------------------------------

bool CAtomList::FreezeSelectedAtomsWH(void)
{
    CHistoryNode* p_history = BeginChangeWH(EHCL_TOPOLOGY,tr("freeze selected atoms"));
    if( p_history == NULL ) return (false);

    FreezeSelectedAtoms(p_history);

    EndChangeWH();
    return(true);
}

//------------------------------------------------------------------------------

bool CAtomList::UnfreezeAllAtomsWH(void)
{
    CHistoryNode* p_history = BeginChangeWH(EHCL_TOPOLOGY,tr("unfreeze all atoms"));
    if( p_history == NULL ) return (false);

    UnfreezeAllAtoms(p_history);

    EndChangeWH();
    return(true);
}

//------------------------------------------------------------------------------

bool CAtomList::UnfreezeSelectedAtomsWH(void)
{
    CHistoryNode* p_history = BeginChangeWH(EHCL_TOPOLOGY,tr("unfreeze selected atoms"));
    if( p_history == NULL ) return (false);

    UnfreezeSelectedAtoms(p_history);

    EndChangeWH();
    return(true);
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

bool CAtomList::MoveCOMToWH(EGeometryOrigin origin,EGeometryScope scope,bool com_from_all_atoms)
{
    CHistoryNode* p_history = BeginChangeWH(EHCL_GEOMETRY,tr("center to COM"));
    if( p_history == NULL ) return (false);

    // calculate COM -----------------------------
    bool    any_atom_selected = false;
    CPoint  old_com;
    double  tmass = 0.0;

    if( ! com_from_all_atoms ){
       any_atom_selected = IsAnyAtomSelected();
    }

    foreach(QObject* p_qobj,children()) {
        CAtom* p_atom = static_cast<CAtom*>(p_qobj);
        if( (! any_atom_selected) || (p_atom->IsFlagSet(EPOF_SELECTED)) ||
                ( (p_atom->GetResidue() != NULL) && (p_atom->GetResidue()->IsFlagSet(EPOF_SELECTED)) ) ){
            double mass = p_atom->GetMass();
            old_com += p_atom->GetPos()*mass;
            tmass += mass;
        }
    }

    if( tmass != 0 ){
        old_com /= tmass;
    }

    TransformAtomsBegin(scope);

    // transform ---------------------------------
    CTransformation trans;

    if( origin == EGO_ORIGIN ){
        trans.Translate(-old_com);
    }
    if( origin == EGO_BOX_CENTER ){
        trans.Translate(-old_com+GetStructure()->PBCInfo.GetBoxCenter());
    }

    CAtomListTransHI* p_node = new CAtomListTransHI(GetStructure(),trans);
    p_history->Register(p_node);

    TransformAtomsEnd(trans);

    EndChangeWH();
    return(true);
}

//------------------------------------------------------------------------------

bool CAtomList::MoveCOGToWH(EGeometryOrigin origin,EGeometryScope scope,bool cog_from_all_atoms)
{  
    CHistoryNode* p_history = BeginChangeWH(EHCL_GEOMETRY,tr("center to COG"));
    if( p_history == NULL ) return (false);

    // calculate COM -----------------------------
    bool    any_atom_selected = false;
    CPoint  old_cog;
    int     natoms = 0;

    if( ! cog_from_all_atoms ){
       any_atom_selected = IsAnyAtomSelected();
    }

    foreach(QObject* p_qobj,children()) {
        CAtom* p_atom = static_cast<CAtom*>(p_qobj);
        if( (! any_atom_selected) || (p_atom->IsFlagSet(EPOF_SELECTED)) ||
                ( (p_atom->GetResidue() != NULL) && (p_atom->GetResidue()->IsFlagSet(EPOF_SELECTED)) ) ){
            old_cog += p_atom->GetPos();
            natoms++;
        }
    }

    if( natoms != 0 ){
        old_cog /= natoms;
    }

    TransformAtomsBegin(scope);

    // transform ---------------------------------
    CTransformation trans;

    if( origin == EGO_ORIGIN ){
        trans.Translate(-old_cog);
    }
    if( origin == EGO_BOX_CENTER ){
        trans.Translate(-old_cog+GetStructure()->PBCInfo.GetBoxCenter());
    }

    CAtomListTransHI* p_node = new CAtomListTransHI(GetStructure(),trans);
    p_history->Register(p_node);

    TransformAtomsEnd(trans);

    EndChangeWH();
    return(true);
}

//------------------------------------------------------------------------------

bool CAtomList::MirrorInPlaneWH(EPlaneType mirror,EGeometryScope scope)
{
    // transform ---------------------------------
    CTransformation trans;
    QString         descr;

    switch(mirror) {
    case EPT_PLANE_XY:
        trans.Field[2][2] = -1;
        descr = tr("mirror in XY plane");
        break;
    case EPT_PLANE_XZ:
        trans.Field[1][1] = -1;
        descr = tr("mirror in XZ plane");
        break;
    case EPT_PLANE_YZ:
        trans.Field[0][0] = -1;
        descr = tr("mirror in YZ plane");
        break;
    }

    CHistoryNode* p_history = BeginChangeWH(EHCL_GEOMETRY,descr);
    if( p_history == NULL ) return (false);

    TransformAtomsBegin(scope);

    CAtomListTransHI* p_node = new CAtomListTransHI(GetStructure(),trans);
    p_history->Register(p_node);

    TransformAtomsEnd(trans);

    EndChangeWH();
    return(true);
}

//------------------------------------------------------------------------------

bool CAtomList::MirrorInPlaneWH(const CPoint& origin,const CPoint& normal, EGeometryScope scope)
{
    // transform ---------------------------------
    CTransformation trans;
    QString         descr;

    // compute transformation
    CPoint n = normal;
    double k = VectDot(origin,normal);

    trans.Field[0][0] = 1-(2*(n.x)*(n.x));
    trans.Field[1][0] = -2*(n.x)*(n.y);
    trans.Field[2][0] = -2*(n.x)*(n.z);
    trans.Field[3][0] = 2*(n.x)*k;

    trans.Field[0][1] = -2*(n.y)*(n.x);
    trans.Field[1][1] = 1-(2*(n.y)*(n.y));
    trans.Field[2][1] = -2*(n.y)*(n.z);
    trans.Field[3][1] = 2*(n.y)*k;

    trans.Field[0][2] = -2*(n.z)*(n.x);
    trans.Field[1][2] = -2*(n.z)*(n.y);
    trans.Field[2][2] = 1-(2*(n.z)*(n.z));
    trans.Field[3][2] = 2*(n.z)*k;

    trans.Field[0][3] = 0;
    trans.Field[1][3] = 0;
    trans.Field[2][3] = 0;
    trans.Field[3][3] = 1;

    CHistoryNode*  p_history = BeginChangeWH(EHCL_GEOMETRY,descr);
    if( p_history == NULL ) return (false);

    // init transformation
    TransformAtomsBegin(scope);

    CAtomListTransHI* p_node = new CAtomListTransHI(GetStructure(),trans);
    p_history->Register(p_node);

    TransformAtomsEnd(trans);

    EndChangeWH();
    return(true);
}

//------------------------------------------------------------------------------

bool CAtomList::AlignPrincipalAxesWH(EAxisType axis,EGeometryScope scope)
{
    CPrincipalAxes  princip_axes;
    double          atom_mass;
    CPoint          pos;
    CPoint          com;
    double          tmass = 0.0;
    bool            any_atom_selected = IsAnyAtomSelected();

    // get COM -----------------------------------
    foreach(QObject* p_qobj,children()) {
        CAtom* p_atom = static_cast<CAtom*>(p_qobj);
        if( (! any_atom_selected) || (p_atom->IsFlagSet(EPOF_SELECTED)) ||
                ( (p_atom->GetResidue() != NULL) && (p_atom->GetResidue()->IsFlagSet(EPOF_SELECTED)) ) ){
            atom_mass = p_atom->GetMass();
            com += p_atom->GetPos()*atom_mass;
            tmass += atom_mass;
        }
    }

    if( tmass != 0 ){
        com /= tmass;
    }

    // init principal axes -----------------------
    foreach(QObject* p_qobj,children()) {
        CAtom* p_atom = static_cast<CAtom*>(p_qobj);
        if( (! any_atom_selected) || (p_atom->IsFlagSet(EPOF_SELECTED)) ||
                ( (p_atom->GetResidue() != NULL) && (p_atom->GetResidue()->IsFlagSet(EPOF_SELECTED)) ) ){
            atom_mass = p_atom->GetMass();
            pos = p_atom->GetPos();
            princip_axes.AddPoint(pos-com, atom_mass);
        }
    }

    // solve eigenproblem
    princip_axes.CalcPrincipalAxes();

    // align with axis
    CPoint  aaxis, baxis, caxis;
    switch(axis) {
    case EAT_AXIS_X:
        // the largest moment is aligned with x-axis
        aaxis = princip_axes.GetAAxis();
        baxis = princip_axes.GetBAxis();
        caxis = princip_axes.GetCAxis();
        break;
    case EAT_AXIS_Y:
        // the largest moment is aligned with y-axis
        aaxis = princip_axes.GetCAxis();
        baxis = princip_axes.GetAAxis();
        caxis = princip_axes.GetBAxis();
        break;
    case EAT_AXIS_Z:
        // the largest moment is aligned with z-axis
        aaxis = princip_axes.GetBAxis();
        baxis = princip_axes.GetCAxis();
        caxis = princip_axes.GetAAxis();
        break;
    }

    // http://answers.google.com/answers/threadview/id/556169.html
    // 3-point alignment, coordinate conversion, ...
    // [Barycentric Coordinates], http://www.cap-lore.com/MathPhys/bcc.html

    CTransformation  align_trans;

    align_trans.Field[0][0] = aaxis.x;
    align_trans.Field[1][0] = aaxis.y;
    align_trans.Field[2][0] = aaxis.z;
    align_trans.Field[0][1] = baxis.x;
    align_trans.Field[1][1] = baxis.y;
    align_trans.Field[2][1] = baxis.z;
    align_trans.Field[0][2] = caxis.x;
    align_trans.Field[1][2] = caxis.y;
    align_trans.Field[2][2] = caxis.z;

    CTransformation trans;

    trans.Translate(-com);
    trans *= align_trans;
    trans.Translate(com);

    CHistoryNode* p_history = BeginChangeWH(EHCL_GEOMETRY,"align principal axes");
    if( p_history == NULL ) return (false);

    TransformAtomsBegin(scope);

    CAtomListTransHI* p_node = new CAtomListTransHI(GetStructure(),trans);
    p_history->Register(p_node);

    TransformAtomsEnd(trans);

    EndChangeWH();
    return(true);
}

//------------------------------------------------------------------------------

bool CAtomList::RotateByWH(EAxisType axis,double angle,EGeometryScope scope)
{
    CTransformation trans;
    QString         descr;

    switch(axis) {
    case EAT_AXIS_X:
        trans.Rotate(angle,CPoint(1,0,0));
        descr = tr("rotate around X-axis by %1 %2");
        break;
    case EAT_AXIS_Y:
        trans.Rotate(angle,CPoint(0,1,0));
        descr = tr("rotate around Y-axis by %1 %2");
        break;
    case EAT_AXIS_Z:
        trans.Rotate(angle,CPoint(0,0,1));
        descr = tr("rotate around Z-axis by %1 %2");
        break;
    }

    CHistoryNode* p_history = BeginChangeWH(EHCL_GEOMETRY,descr);
    if( p_history == NULL ) return (false);

    TransformAtomsBegin(scope);

    CAtomListTransHI* p_node = new CAtomListTransHI(GetStructure(),trans);
    p_history->Register(p_node);

    TransformAtomsEnd(trans);

    EndChangeWH();
    return(true);
}

//------------------------------------------------------------------------------

bool CAtomList::RotateByWH(const CPoint& orig,const CPoint& dir,double angle,EGeometryScope scope)
{
    CTransformation trans;
    QString         descr;

    trans.Translate(-orig);
    trans.Rotate(angle,dir);
    trans.Translate(orig);
    descr = tr("rotate around custom vector by %3");

    CHistoryNode* p_history = BeginChangeWH(EHCL_GEOMETRY,descr);
    if( p_history == NULL ) return (false);

    TransformAtomsBegin(scope);

    CAtomListTransHI* p_node = new CAtomListTransHI(GetStructure(),trans);
    p_history->Register(p_node);

    TransformAtomsEnd(trans);

    EndChangeWH();
    return(true);
}

//------------------------------------------------------------------------------

bool CAtomList::MoveByWH(const CPoint& vect,EGeometryScope scope)
{  
    CHistoryNode* p_history = BeginChangeWH(EHCL_GEOMETRY,"move by");
    if( p_history == NULL ) return (false);

    CTransformation trans;
    trans.Translate(vect);

    TransformAtomsBegin(scope);

    CAtomListTransHI* p_node = new CAtomListTransHI(GetStructure(),trans);
    p_history->Register(p_node);

    TransformAtomsEnd(trans);

    EndChangeWH();
    return(true);
}

//------------------------------------------------------------------------------

bool CAtomList::ImageCoordinatesWH(EImageMode mode, bool origin,bool familiar, EGeometryScope scope)
{
    if( GetStructure()->PBCInfo.IsPBCEnabled() == false ) return(false);

    CHistoryNode* p_history = BeginChangeWH(EHCL_GEOMETRY,"image coordinates");
    if( p_history == NULL ) return (false);

    // init transformation
    TransformAtomsBegin(scope);

    CHistoryItem* p_node = new CAtomListCoordinatesHI(GetStructure());
    p_history->Register(p_node);

    GetStructure()->BeginGeometryUpdate();

    switch(mode){
        case EIM_BY_ATOMS:
            ImageByAtoms(origin,familiar);
            break;
        case EIM_BY_RESIDUES:
            ImageByResidues(origin,familiar);
            break;
        case EIM_BY_MOLECULES:
            ImageByMolecules(origin,familiar);
            break;
    }

    GetStructure()->EndGeometryUpdate();

    EndChangeWH();
    return(true);
}

//------------------------------------------------------------------------------

void CAtomList::ImageByAtoms(bool origin,bool familiar)
{
    foreach(QObject* p_qobj,children()) {
        CAtom* p_atom = static_cast<CAtom*>(p_qobj);
        if( p_atom->IsFlagSet(EPOF_MANIP_FLAG) ){
            CPoint pos = p_atom->GetPos();
            pos = GetStructure()->PBCInfo.ImagePoint(pos,0,0,0,origin,familiar);
            p_atom->SetPos(pos);
        }
    }
}

//------------------------------------------------------------------------------

void CAtomList::ImageByResidues(bool origin,bool familiar)
{
    foreach(QObject* p_qobj,GetStructure()->GetResidues()->children()) {
        CResidue* p_res = static_cast<CResidue*>(p_qobj);

        CPoint com;
        double totmass=0.0;
        bool   procon = false;

        foreach(QObject* p_qobj,p_res->GetAtoms()) {
            CAtom* p_atom = static_cast<CAtom*>(p_qobj);

            if( p_atom->IsFlagSet(EPOF_MANIP_FLAG) ){
                procon = true;
            }

            com += p_atom->GetPos()*p_atom->GetMass();
            totmass += p_atom->GetMass();
        }

        if( procon == false ) continue;

        if( totmass > 0.0 ){
            com /= totmass;
        }

        // image residue
        CPoint icom = GetStructure()->PBCInfo.ImagePoint(com,0,0,0,origin,familiar);
        CPoint dmov = icom-com;

        foreach(QObject* p_qobj,p_res->GetAtoms()) {
            CAtom* p_atom = static_cast<CAtom*>(p_qobj);
            p_atom->SetPos(p_atom->GetPos()+dmov);
        }
    }
}


//------------------------------------------------------------------------------

void CAtomList::ImageByMolecules(bool origin,bool familiar)
{
    // clear processing flag
    foreach(QObject* p_qobj,children()) {
        CAtom* p_atom = static_cast<CAtom*>(p_qobj);
        p_atom->SetFlag(EPOF_PROC_FLAG,false);
    }

    // go through all atoms and detect molecules
    foreach(QObject* p_qobj,children()) {
        CAtom* p_atom1 = static_cast<CAtom*>(p_qobj);
        if( p_atom1->IsFlagSet(EPOF_PROC_FLAG) == true ) continue;
        if( p_atom1->IsFlagSet(EPOF_MANIP_FLAG) == false ) continue;

        std::queue<CAtom*>    stack;
        std::vector<CAtom*>   molecule;
        stack.push(p_atom1);
        p_atom1->SetFlag(EPOF_PROC_FLAG,true);

        CPoint com;
        double totmass=0.0;

        // go through the stack over interconnected atoms
        while( stack.size() > 0 ) {
            CAtom*  p_atom = stack.front();

            com += p_atom->GetPos()*p_atom->GetMass();
            totmass += p_atom->GetMass();
            molecule.push_back(p_atom);

            stack.pop();

            foreach(CBond* p_bond, p_atom->GetBonds()) {
                CAtom* p_atom2 = p_bond->GetOppositeAtom(p_atom);
                if( p_atom2->IsFlagSet(EPOF_PROC_FLAG) == false ) {
                    p_atom2->SetFlag(EPOF_PROC_FLAG,true);
                    stack.push(p_atom2);
                }
            }
        }

        if( totmass > 0.0 ){
            com /= totmass;
        }

        // image molecule
        CPoint icom = GetStructure()->PBCInfo.ImagePoint(com,0,0,0,origin,familiar);
        CPoint dmov = icom-com;

        std::vector<CAtom*>::iterator it = molecule.begin();
        std::vector<CAtom*>::iterator ie = molecule.end();

        while( it != ie ){
            (*it)->SetPos((*it)->GetPos()+dmov);
            it++;
        }

    }
}

//------------------------------------------------------------------------------

void CAtomList::MoveBy(const CPoint& vect,EGeometryScope scope)
{
    CTransformation trans;
    trans.Translate(vect);

    TransformAtomsBegin(scope);
    TransformAtomsEnd(trans);
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

CAtom* CAtomList::CreateAtom(int Z,CHistoryNode* p_history)
{
    return( CreateAtom(Z,CPoint(),p_history) );
}

//------------------------------------------------------------------------------

CAtom* CAtomList::CreateAtom(int Z,const CPoint& pos,CHistoryNode* p_history)
{
    CAtom* p_at = new CAtom(this);
    p_at->SetZ(Z);
    p_at->SetPos(pos);

    if( p_at->GetName().isEmpty() ) {
        QString name = "%1%2";
        name = name.arg(PeriodicTable.GetSymbol(p_at->GetZ()));
        name = name.arg(TopIndex.GetIndex());
        p_at->SetName(name);
    }

    if( p_history != NULL ) {
        CAtomHI* p_atomdata = new CAtomHI(p_at,EHID_FORWARD);
        p_history->Register(p_atomdata);
    }

    ListSizeChanged();
    return(p_at);
}

//------------------------------------------------------------------------------

CAtom* CAtomList::CreateAtom(CXMLElement* p_ele,CHistoryNode* p_history)
{
    if( p_ele == NULL ){
        INVALID_ARGUMENT("p_ele is NULL");
    }

    CAtom* p_at = new CAtom(this,true);
    p_at->LoadData(p_ele);

    if( p_history != NULL ) {
        CAtomHI* p_atomdata = new CAtomHI(p_at,EHID_FORWARD);
        p_history->Register(p_atomdata);
    }

    ListSizeChanged();
    return(p_at);
}

//------------------------------------------------------------------------------

CAtom* CAtomList::CreateAtom(const CAtomData& atom_data,CHistoryNode* p_history)
{
    CAtom* p_at = new CAtom(this);

    if( atom_data.Index > 0 ){
        // we need explicit object index
        int index = atom_data.Index;
        // get base index
        if( GetProject() != NULL ){
            index += GetProject()->GetBaseObjectIndex();
        }
        p_at->SetIndex(index);
    }

    p_at->SetName(atom_data.Name);
    p_at->SetDescription(atom_data.Description);
    if( atom_data.SerIndex == -1 ){
        p_at->SetSerIndex(GetTopSerIndex()+1);
    } else {
        p_at->SetSerIndex(atom_data.SerIndex);
    }
    p_at->SetLocIndex(atom_data.LocIndex);
    p_at->SetType(atom_data.Type);
    p_at->SetZ(atom_data.Z);
    p_at->SetCharge(atom_data.Charge);
    p_at->SetPos(atom_data.Pos);
    p_at->SetVel(atom_data.Vel);

    if( p_at->GetName().isEmpty() ) {
        QString name = "%1%2";
        name = name.arg(PeriodicTable.GetSymbol(p_at->GetZ()));
        name = name.arg(TopIndex.GetIndex());
        p_at->SetName(name);
    }

    // register elementary change to history list
    if( p_history != NULL ) {
        CAtomHI* p_atomdata = new CAtomHI(p_at,EHID_FORWARD);
        p_history->Register(p_atomdata);
    }

    ListSizeChanged();

    return(p_at);
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

void CAtomList::FreezeAllAtoms(CHistoryNode* p_history)
{
    BeginUpdate();

    foreach(QObject* p_qobj,children()) {
        CAtom* p_atom = static_cast<CAtom*>(p_qobj);
        CProObjectFlags flags = p_atom->GetFlags();
        SET_FLAG(flags,EPOF_FREEZED,true);
        p_atom->SetFlags(flags,p_history);
    }

    EndUpdate();
}

//------------------------------------------------------------------------------

void CAtomList::FreezeHeavyAtoms(CHistoryNode* p_history)
{
    BeginUpdate();

    foreach(QObject* p_qobj,children()) {
        CAtom* p_atom = static_cast<CAtom*>(p_qobj);
        if( ! p_atom->IsVirtual() ){
            CProObjectFlags flags = p_atom->GetFlags();
            SET_FLAG(flags,EPOF_FREEZED,true);
            p_atom->SetFlags(flags,p_history);
        }
    }

    EndUpdate();
}

//------------------------------------------------------------------------------

void CAtomList::FreezeSelectedAtoms(CHistoryNode* p_history)
{
    BeginUpdate();

    foreach(QObject* p_qobj,children()) {
        CAtom* p_atom = static_cast<CAtom*>(p_qobj);
        if( p_atom->IsFlagSet(EPOF_SELECTED) ){
            CProObjectFlags flags = p_atom->GetFlags();
            SET_FLAG(flags,EPOF_FREEZED,true);
            p_atom->SetFlags(flags,p_history);
        }
    }

    EndUpdate();
}

//------------------------------------------------------------------------------

void CAtomList::UnfreezeAllAtoms(CHistoryNode* p_history)
{
    BeginUpdate();

    foreach(QObject* p_qobj,children()) {
        CAtom* p_atom = static_cast<CAtom*>(p_qobj);
        CProObjectFlags flags = p_atom->GetFlags();
        SET_FLAG(flags,EPOF_FREEZED,false);
        p_atom->SetFlags(flags,p_history);
    }

    EndUpdate();
}

//------------------------------------------------------------------------------

void CAtomList::UnfreezeSelectedAtoms(CHistoryNode* p_history)
{
    BeginUpdate();

    foreach(QObject* p_qobj,children()) {
        CAtom* p_atom = static_cast<CAtom*>(p_qobj);
        if( p_atom->IsFlagSet(EPOF_SELECTED) ){
            CProObjectFlags flags = p_atom->GetFlags();
            SET_FLAG(flags,EPOF_FREEZED,false);
            p_atom->SetFlags(flags,p_history);
        }
    }

    EndUpdate();
}

//------------------------------------------------------------------------------

void CAtomList::MoveAllAtomsFrom(CAtomList* p_source,CHistoryNode* p_history)
{
    if( this == p_source ) return;  // source and target must be different

    BeginUpdate();
    p_source->BeginUpdate();

    QVector<int>    indexes;
    indexes.reserve(p_source->GetNumberOfAtoms());

    int topindex = GetTopSerIndex();
    int lowindex = p_source->GetLowSerIndex();

    foreach(QObject* p_qobj, p_source->children()){
        CAtom* p_atom = static_cast<CAtom*>(p_qobj);
        int index = p_atom->GetIndex();
        indexes.push_back(index);
        p_atom->ChangeParent(this);
        if( topindex >= lowindex ){
            int si = topindex - lowindex + 1 + p_atom->GetSerIndex();
            p_atom->SetSerIndex(si);
        }
    }

    // record the change to history list
    if( p_history ){
        CHistoryItem* p_hi = new CAtomListChangeParentHI(GetStructure(),
                                                         p_source->GetStructure(),indexes,
                                                         topindex,lowindex);
        p_history->Register(p_hi);
    }

    p_source->EndUpdate();
    EndUpdate();
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

void CAtomList::SortAtoms(void)
{
    QList<CAtom*> loc_copy;

    // create local copy of list and remove all from children()
    foreach(QObject* p_qobj,children()) {
        CAtom* p_atom = static_cast<CAtom*>(p_qobj);
        loc_copy.append(p_atom);
        p_atom->setParent(NULL);
    }

    // sort atoms
    std::sort(loc_copy.begin(),loc_copy.end(),LessThanBySerIndex);

    // add atoms to the list in sorted order
    foreach(CAtom* p_atom,loc_copy) {
        p_atom->setParent(this);
    }

    EmitOnAtomListChanged();
}

//------------------------------------------------------------------------------

bool CAtomList::LessThanBySerIndex(CAtom* p_left,CAtom* p_right)
{
    return( p_left->GetSerIndex() < p_right->GetSerIndex() );
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

CStructure* CAtomList::GetStructure(void) const
{
    return(static_cast<CStructure*>(parent()));
}

//------------------------------------------------------------------------------

int CAtomList::GetNumberOfAtoms(void) const
{
    return(children().count());
}

//------------------------------------------------------------------------------

CAtom* CAtomList::SearchBySerIndex(int seridx)
{
    foreach(QObject* p_qobj,children()) {
        CAtom* p_atom = static_cast<CAtom*>(p_qobj);
        if( p_atom->GetSerIndex() == seridx) return(p_atom);
    }
    return(NULL);
}

//------------------------------------------------------------------------------

int CAtomList::GetLowSerIndex(void)
{
    int low_index = 0;
    if( ! children().isEmpty() ){
        CAtom* p_atom = static_cast<CAtom*>(children().first());
        low_index = p_atom->GetSerIndex();
    }
    foreach(QObject* p_qobj,children()) {
        CAtom* p_atom = static_cast<CAtom*>(p_qobj);
        if( p_atom->GetSerIndex() < low_index){
            low_index = p_atom->GetSerIndex();
        }
    }
    return(low_index);
}

//------------------------------------------------------------------------------

int CAtomList::GetTopSerIndex(void)
{
    int top_index = 0;
    if( ! children().isEmpty() ){
        CAtom* p_atom = static_cast<CAtom*>(children().first());
        top_index = p_atom->GetSerIndex();
    }
    foreach(QObject* p_qobj,children()) {
        CAtom* p_atom = static_cast<CAtom*>(p_qobj);
        if( p_atom->GetSerIndex() > top_index){
            top_index = p_atom->GetSerIndex();
        }
    }
    return(top_index);
}

//------------------------------------------------------------------------------

const CPoint CAtomList::GetCenterOfGeometry(void)
{
    CPoint pos;
    int    count = 0;

    foreach(QObject* p_qobj,children()) {
        CAtom* p_atom = static_cast<CAtom*>(p_qobj);
        pos += p_atom->GetPos();
        count++;
    }

    if( count == 0 ) return(pos);
    pos /= count;
    return(pos);
}

//------------------------------------------------------------------------------

const CPoint CAtomList::GetCenterOfMass(void)
{
    CPoint     pos;
    double     tmass = 0.0;

    foreach(QObject* p_qobj,children()) {
        CAtom* p_atom = static_cast<CAtom*>(p_qobj);
        double mass = p_atom->GetMass();
        pos += p_atom->GetPos()*mass;
        tmass += mass;
    }
    if( tmass == 0 ) return(pos);
    pos /= tmass;
    return(pos);
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

void CAtomList::SetSnapshot(CSnapshot* p_snap)
{
    Snapshot = p_snap;
}

//------------------------------------------------------------------------------

CSnapshot* CAtomList::GetSnapshot(void)
{
    return(Snapshot);
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

void CAtomList::ClearMoleculeFlag(void)
{
    foreach(QObject* p_qobj,children()) {
        CAtom* p_atom = static_cast<CAtom*>(p_qobj);
        p_atom->SetFlag(EPOF_MANIP_FLAG,false);
    }
}

//------------------------------------------------------------------------------

void CAtomList::InitMoleculeFlagForAll(void)
{
    foreach(QObject* p_qobj,children()) {
        CAtom* p_atom = static_cast<CAtom*>(p_qobj);
        p_atom->SetFlag(EPOF_MANIP_FLAG,true);
    }
}

//------------------------------------------------------------------------------

void CAtomList::InitMoleculeFlagOnlyFromSelected(void)
{
    foreach(QObject* p_qobj,children()) {
        CAtom* p_atom = static_cast<CAtom*>(p_qobj);
        if( p_atom->IsFlagSet(EPOF_SELECTED) ||
            ( (p_atom->GetResidue() != NULL) && (p_atom->GetResidue()->IsFlagSet(EPOF_SELECTED)) ) ) {
            p_atom->SetFlag(EPOF_MANIP_FLAG,true);
        }
    }
}

//------------------------------------------------------------------------------

void CAtomList::InitMoleculeFlagFromSelected(void)
{
    std::queue<CAtom*>    stack;

    // prepare atoms and put selected on stack
    foreach(QObject* p_qobj,children()) {
        CAtom* p_atom = static_cast<CAtom*>(p_qobj);
        if( p_atom->IsFlagSet(EPOF_SELECTED) ||
            ( (p_atom->GetResidue() != NULL) && (p_atom->GetResidue()->IsFlagSet(EPOF_SELECTED)) ) ) {
            stack.push(p_atom);
            p_atom->SetFlag(EPOF_MANIP_FLAG,true);
        }
    }

    // go through the stack over interconnected atoms
    while( stack.size() > 0 ) {
        CAtom*                  p_atom = stack.front();

        stack.pop();

        foreach(CBond* p_bond, p_atom->GetBonds()) {
            CAtom* p_atom2 = p_bond->GetOppositeAtom(p_atom);
            if( p_atom2->IsFlagSet(EPOF_MANIP_FLAG) == false ) {
                stack.push(p_atom2);
                p_atom2->SetFlag(EPOF_MANIP_FLAG,true);
            }
        }
    }
}

//------------------------------------------------------------------------------

bool CAtomList::IsAnyAtomSelected(void)
{
    bool any_selected = false;

    foreach(QObject* p_qobj,children()) {
        CAtom* p_atom = static_cast<CAtom*>(p_qobj);
        any_selected |= p_atom->IsFlagSet(EPOF_SELECTED) ||
                ( (p_atom->GetResidue() != NULL) && (p_atom->GetResidue()->IsFlagSet(EPOF_SELECTED)) );
    }

    return( any_selected );
}

//------------------------------------------------------------------------------

int CAtomList::GetNumberOfSelectedAtoms(void)
{
    int count = 0;

    foreach(QObject* p_qobj,children()) {
        CAtom* p_atom = static_cast<CAtom*>(p_qobj);
        if( p_atom->IsFlagSet(EPOF_SELECTED) ){
            count++;
        }
    }

    return( count );
}

//------------------------------------------------------------------------------

bool CAtomList::HasInvalidAtoms(void)
{
    bool invalid = false;
    foreach(QObject* p_qobj, children()){
        CAtom* p_atom = static_cast<CAtom*>(p_qobj);
        invalid |= p_atom->GetResidue() == NULL;
    }
    return(invalid);
}

//------------------------------------------------------------------------------

void CAtomList::UnregisterAllRegisteredAtoms(CHistoryNode* p_history)
{
    foreach(QObject* p_qobj,children()) {
        CAtom* p_atom = static_cast<CAtom*>(p_qobj);
        p_atom->UnregisterAllRegisteredObjects(p_history);
    }
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

void CAtomList::BeginUpdate(void)
{
    UpdateLevel++;
    blockSignals(true);
}

//------------------------------------------------------------------------------

void CAtomList::EndUpdate(bool do_not_sort)
{
    if( UpdateLevel == 0 ) return;
    UpdateLevel--;
    if( UpdateLevel == 0 ){
        if( ForceSorting ) {
            if( ! do_not_sort ) SortAtoms();
            ForceSorting = false;
        }
        blockSignals(false);
        if( Changed ){
            emit OnAtomListChanged();
            Changed = false;
        }
    }
}

//------------------------------------------------------------------------------

void CAtomList::EmitOnAtomListChanged(void)
{
    if( UpdateLevel > 0 ) Changed = true;
    emit OnAtomListChanged();
    if( GetStructure()->GetStructures() ){
        GetStructure()->GetStructures()->EmitOnStructureListChanged();
    }
}

//------------------------------------------------------------------------------

void CAtomList::ListSizeChanged(bool do_not_sort)
{
    Changed = true;
    ForceSorting = ! do_not_sort;
    if( UpdateLevel > 0 ){
        Changed = true;
        return;
    }
    if( ! do_not_sort ) SortAtoms();
    EmitOnAtomListChanged();
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

void CAtomList::LoadData(CXMLElement* p_ele,CHistoryNode* p_history)
{
    // check input data -----------------------------
    if( p_ele == NULL ) {
        INVALID_ARGUMENT("p_ele is NULL");
    }

    if( p_ele->GetName() != "atoms" ) {
        LOGIC_ERROR("p_ele is not atoms");
    }

    // load object info -----------------------------
    CProObject::LoadData(p_ele);

    // load data ------------------------------------
    CXMLElement* p_ael;
    p_ael = p_ele->GetFirstChildElement("atom");

    while( p_ael != NULL ) {
        CreateAtom(p_ael,p_history);
        p_ael = p_ael->GetNextSiblingElement("atom");
    }
}

//------------------------------------------------------------------------------

void CAtomList::LoadData(CXMLElement* p_ele)
{
    LoadData(p_ele,NULL);
}

//------------------------------------------------------------------------------

void CAtomList::SaveData(CXMLElement* p_ele,bool selected)
{
    // check input data -----------------------------
    if( p_ele == NULL ) {
        INVALID_ARGUMENT("p_ele is NULL");
    }

    if( p_ele->GetName() != "atoms" ) {
        LOGIC_ERROR("p_ele is not 'atoms'");
    }

    // save object info -----------------------------
    CProObject::SaveData(p_ele);

    // save data ------------------------------------
    foreach(QObject* p_qobj,children()) {
        CAtom* p_atom = static_cast<CAtom*>(p_qobj);
        if( selected ) {
            // for EPOF_MANIP_FLAG see to bool CStructure::Copy(CXMLElement* p_ele,bool selected_only);
            if( p_atom->IsFlagSet(EPOF_MANIP_FLAG) ) {
                CXMLElement* p_ael = p_ele->CreateChildElement("atom");
                p_atom->SaveData(p_ael);
            }
        } else {
            CXMLElement* p_ael = p_ele->CreateChildElement("atom");
            p_atom->SaveData(p_ael);
        }
    }
}

//------------------------------------------------------------------------------

void CAtomList::SaveData(CXMLElement* p_ele)
{
    SaveData(p_ele,false);
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

void CAtomList::TransformAtomsBegin(EGeometryScope scope)
{
    // this is necessary as anyWH method can finish without TransformAtomsEnd
    ClearMoleculeFlag();

    switch(scope){
        case EGS_ALL_ATOMS:
            InitMoleculeFlagForAll();
            break;
        case EGS_SELECTED_ATOMS:
            InitMoleculeFlagOnlyFromSelected();
            break;
        case EGS_SELECTED_FRAGMENTS:
            InitMoleculeFlagFromSelected();
            break;
        default:
            ES_ERROR("not implemented");
            return;
    }
}

//------------------------------------------------------------------------------

void CAtomList::TransformAtomsEnd(const CTransformation& trans)
{
    GetStructure()->BeginGeometryUpdate();

    foreach(QObject* p_qobj,children()) {
        CAtom* p_atom = static_cast<CAtom*>(p_qobj);
        if( p_atom->IsFlagSet(EPOF_MANIP_FLAG) ){
            CPoint pos = p_atom->GetPos();
            p_atom->SetPos(trans.GetTransform(pos));
        }
    }

    GetStructure()->EndGeometryUpdate();

    ClearMoleculeFlag();
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

