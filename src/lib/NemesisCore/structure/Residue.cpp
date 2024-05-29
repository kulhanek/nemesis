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

#include <Residue.hpp>
#include <ResidueList.hpp>
#include <NemesisCoreModule.hpp>
#include <CategoryUUID.hpp>
#include <Atom.hpp>
#include <ErrorSystem.hpp>
#include <XMLElement.hpp>
#include <Structure.hpp>
#include <ResidueHistory.hpp>
#include <OpenBabelUtils.hpp>
#include <AtomList.hpp>
#include <BondList.hpp>
#include <QSet>
#include <QMap>
#include <PeriodicTable.hpp>
#include <HistoryNode.hpp>
#include <queue>

// remove AddAtom macro defined on Windows
#if defined AddAtom
#undef AddAtom
#endif

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

CExtUUID        ResidueID(
                    "{RESIDUE:cf1a2b7f-69cc-4994-a630-b90c1c0c112e}",
                    "Residue");

CPluginObject   ResidueObject(&NemesisCorePlugin,
                    ResidueID,STRUCTURE_CAT,
                    ":/images/NemesisCore/structure/Residue.svg",
                    NULL);

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

CResidue::CResidue(CResidueList* p_bl)
    : CProObject(&ResidueObject,p_bl,p_bl->GetProject())
{
    SeqIndex = 0;
    UpdateLevel = 0;
}

//------------------------------------------------------------------------------

CResidue::CResidue(CResidueList* p_bl,bool no_index)
    : CProObject(&ResidueObject,p_bl,p_bl->GetProject(),no_index)
{
    SeqIndex = 0;
    UpdateLevel = 0;
}

//------------------------------------------------------------------------------

CResidue::~CResidue(void)
{
    CResidueList* p_list = GetResidues();
    if( p_list ){
        // this significantly speedup destruction time if the whole structure is destructed
        // see CStructure::~CStructure(void)
        if( p_list->GetStructure() ) setParent(NULL);    // remove object from the list
        p_list->ListSizeChanged();
    }
}

//------------------------------------------------------------------------------

void CResidue::RemoveFromBaseList(CHistoryNode* p_history)
{
    CProObject* p_obj;

    // unregister other objects
    while( (p_obj = RemoveFirstFromROList()) !=NULL ) {
        p_obj->RemoveFromRegistered(this,p_history);
    }

    // remove atom registrations
    foreach(CAtom* p_atom,Atoms) {
        RemoveAtom(p_atom,p_history);
    }

    // register elementary change to history list
    if( p_history != NULL ) {
        CResidueHI* p_atomdata = new CResidueHI(this,EHID_BACKWARD);
        p_history->Register(p_atomdata);
    }

    // --------------------------------
    delete this;  // destroy object
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

bool CResidue::DeleteWH(bool keep_connectors)
{
    CResidueList* p_list = GetResidues();

    CHistoryNode* p_history = p_list->BeginChangeWH(EHCL_TOPOLOGY,"delete residue");
    if( p_history == NULL ) return (false);

    Delete(keep_connectors,p_history);

    p_list->EndChangeWH();

    return(true);
}

//------------------------------------------------------------------------------

bool CResidue::SetSeqIndexWH(int seqidx)
{
    if( GetSeqIndex() == seqidx ) return(true);

    CHistoryNode* p_history = BeginChangeWH(EHCL_TOPOLOGY,"sequence index");
    if( p_history == NULL ) return (false);

    SetSeqIndex(seqidx,p_history);

    EndChangeWH();
    return(true);
}

//------------------------------------------------------------------------------

bool CResidue::SetChainWH(const QString& chain)
{
    if( GetChain() == chain ) return(true);

    CHistoryNode* p_history = BeginChangeWH(EHCL_TOPOLOGY,"residue chain");
    if( p_history == NULL ) return (false);

    SetChain(chain,p_history);

    EndChangeWH();
    return(true);
}

//------------------------------------------------------------------------------

bool CResidue::SetTypeWH(const QString& type)
{
    if( GetType() == type ) return(true);

    CHistoryNode* p_history = BeginChangeWH(EHCL_TOPOLOGY,"residue type");
    if( p_history == NULL ) return (false);

    SetType(type,p_history);

    EndChangeWH();
    return(true);
}

//------------------------------------------------------------------------------

bool CResidue::AddHydrogensWH(bool polar_only)
{
    CHistoryNode*    p_history;
    p_history = BeginChangeWH(EHCL_TOPOLOGY,tr("add missing hydrogens"));
    if( p_history == NULL ) return (false);

    BeginUpdate();

    // protonate structure by openbabel
    OpenBabel::OBMol obmol;
    COpenBabelUtils::Nemesis2OpenBabel(this,obmol,true,true);

    unsigned int top_atom = obmol.NumAtoms();

    // newly added atoms are added to the end of molecule
    obmol.AddHydrogens(polar_only);

    int loc_index = GetTopLocalIndex();

    // add added hydrogens new hydrogens
    for(unsigned int i= top_atom + 1; i <= obmol.NumAtoms(); i++) {
        OpenBabel::OBAtom* p_obatom1 = obmol.GetAtom(i);

        // atom
        CPoint  pos(p_obatom1->GetX(),p_obatom1->GetY(),p_obatom1->GetZ());

        CAtomData data;

        loc_index++;

        QString name = "%1%2";
        name = name.arg(PeriodicTable.GetSymbol(1));
        name = name.arg(loc_index);
        data.Name = name;

        data.Z = 1;
        data.Pos = pos;
        data.SerIndex = loc_index;

        CAtom* p_atom1 = GetStructure()->GetAtoms()->CreateAtom(data,p_history);

        // residue
        AddAtom(p_atom1,p_history);

        // second atom
        OpenBabel::OBBondIterator obi;
        OpenBabel::OBAtom* p_obatom2 = p_obatom1->BeginNbrAtom(obi);
        CAtom* p_atom2 = Atoms[p_obatom2->GetIdx()-1];

        // bond
        GetStructure()->GetBonds()->CreateBond(p_atom1,p_atom2,BO_SINGLE,p_history);
    }

    EndUpdate();

    EndChangeWH();
    return(true);
}

//------------------------------------------------------------------------------

bool CResidue::RemoveHydrogensWH(void)
{
    CHistoryNode*    p_history;
    p_history = BeginChangeWH(EHCL_TOPOLOGY,tr("delete hydrogens"));
    if( p_history == NULL ) return (false);

    BeginUpdate();

    foreach(CAtom* p_atom,Atoms) {
        if( p_atom->IsVirtual() == true ) {
            p_atom->RemoveFromBaseList(p_history);
        }
    }

    EndUpdate();
    EndChangeWH();
    return(true);
}

//------------------------------------------------------------------------------

bool CResidue::NamesByLocalIndexWH(void)
{
    CHistoryNode*    p_history;
    p_history = BeginChangeWH(EHCL_TOPOLOGY,tr("rename by local index"));
    if( p_history == NULL ) return (false);

    BeginUpdate();

    foreach(CAtom* p_atom,Atoms) {
        QString name = "%1%2";
        name = name.arg(PeriodicTable.GetSymbol(p_atom->GetZ()));
        name = name.arg(p_atom->GetLocIndex());

        p_atom->SetName(name,p_history);
    }

    EndUpdate();
    EndChangeWH();
    return(true);
}

//------------------------------------------------------------------------------

bool CResidue::LocalIndexByOrderWH(void)
{
    CHistoryNode*    p_history;
    p_history = BeginChangeWH(EHCL_TOPOLOGY,tr("update local indexes"));
    if( p_history == NULL ) return (false);

    BeginUpdate();

    // sort atoms by local index
    QList<CAtom*>   loc_copy = Atoms;
    qSort(loc_copy.begin(),loc_copy.end(),LessThanByLocalIndex);

    int index = 1;
    foreach(CAtom* p_atom, loc_copy) {
        p_atom->SetLocIndex(index,p_history);
        index++;
    }

    SortAtoms();

    EndUpdate();
    EndChangeWH();
    return(true);
}

//------------------------------------------------------------------------------

bool CResidue::LocalIndexByMoleculesWH(void)
{
    CHistoryNode*    p_history;
    p_history = BeginChangeWH(EHCL_TOPOLOGY,tr("update local indexes by molecules"));
    if( p_history == NULL ) return (false);

    BeginUpdate();

    // at the begining, each atom belongs to different molecule
    QMap<CAtom*,int>    mol_indexes;

    int index = 1;
    foreach(CAtom* p_atom, Atoms) {
        mol_indexes[p_atom] = index;
        p_atom->SetFlag(EPOF_PROC_FLAG,false);
        index++;
    }

    // traverse over all atoms
    foreach(CAtom* p_atom, Atoms) {

        // add atom to stack
        std::queue<CAtom*>    stack;
        stack.push(p_atom);

        int mol_id = mol_indexes[p_atom];
        p_atom->SetFlag(EPOF_PROC_FLAG,true);

        // go through the stack over interconnected atoms
        while( stack.size() > 0 ) {
            CAtom*      p_atom = stack.front();
            stack.pop();

            foreach(CBond* p_bond, p_atom->GetBonds()) {
                CAtom* p_atom2 = p_bond->GetOppositeAtom(p_atom);
                if( p_atom2->GetResidue() != this ) continue;
                if( p_atom2->IsFlagSet(EPOF_PROC_FLAG) ) continue;
                stack.push(p_atom2);
                p_atom2->SetFlag(EPOF_PROC_FLAG,true);
                mol_indexes[p_atom2] = mol_id;
            }
        }
    }

    // set indexes
    foreach(CAtom* p_atom, Atoms) {
        p_atom->SetLocIndex(mol_indexes[p_atom],p_history);
    }

    SortAtoms();

    EndUpdate();
    EndChangeWH();
    return(true);
}

//------------------------------------------------------------------------------

bool CResidue::ReverseLocalIndexWH(void)
{
    CHistoryNode*    p_history;
    p_history = BeginChangeWH(EHCL_TOPOLOGY,tr("update local indexes"));
    if( p_history == NULL ) return (false);

    BeginUpdate();

    // sort atoms by local index
    QList<CAtom*>   loc_copy = Atoms;
    qSort(loc_copy.begin(),loc_copy.end(),LessThanByLocalIndex);

    QListIterator<CAtom*> it(loc_copy);
    it.toBack();

    int index = 1;
    while( it.hasPrevious() ){
        CAtom* p_atom = it.previous();
        p_atom->SetLocIndex(index,p_history);
        index++;
    }

    SortAtoms();

    EndUpdate();
    EndChangeWH();
    return(true);
}

//------------------------------------------------------------------------------

bool CResidue::AggregateHydrogensWH(void)
{
    CHistoryNode*    p_history;
    p_history = BeginChangeWH(EHCL_TOPOLOGY,tr("aggregate hydrogens"));
    if( p_history == NULL ) return (false);

    BeginUpdate();

    // sort atoms by local index
    QList<CAtom*>   loc_copy = Atoms;
    qSort(loc_copy.begin(),loc_copy.end(),LessThanByLocalIndex);

    int index = 0;
    foreach(CAtom* p_atom, loc_copy) {
        if( p_atom->IsVirtual() ) continue;

        index++;
        p_atom->SetLocIndex(index);

        foreach(CBond* p_bond,p_atom->GetBonds()){
            CAtom* p_opposite = p_bond->GetOppositeAtom(p_atom);
            if( p_opposite == NULL ) continue;
            if( p_opposite->GetResidue() != this ) continue;
            if( p_opposite->IsVirtual() == false ) continue;

            index++;
            p_opposite->SetLocIndex(index);
        }

    }

    SortAtoms();

    EndUpdate();
    EndChangeWH();
    return(true);
}

//------------------------------------------------------------------------------

bool CResidue::AggregateTerminalsWH(void)
{
    CHistoryNode*    p_history;
    p_history = BeginChangeWH(EHCL_TOPOLOGY,tr("aggregate terminals"));
    if( p_history == NULL ) return (false);

    BeginUpdate();

    // sort atoms by local index
    QList<CAtom*>   loc_copy = Atoms;
    qSort(loc_copy.begin(),loc_copy.end(),LessThanByLocalIndex);

    int index = 0;
    foreach(CAtom* p_atom, loc_copy) {
        if( (p_atom->GetBonds().count() == 1) &&
            (! p_atom->IsVirtual()) ) continue;

        index++;
        p_atom->SetLocIndex(index);

        foreach(CBond* p_bond,p_atom->GetBonds()){
            CAtom* p_opposite = p_bond->GetOppositeAtom(p_atom);
            if( p_opposite == NULL ) continue;
            if( p_opposite->GetResidue() != this ) continue;
            if( p_opposite->IsVirtual() ) continue;
            if( p_opposite->GetBonds().count() != 1 ) continue;

            index++;
            p_opposite->SetLocIndex(index);
        }

    }

    SortAtoms();

    EndUpdate();
    EndChangeWH();
    return(true);
}

//------------------------------------------------------------------------------

bool CResidue::NamesByHeavyAtomsWH(void)
{
    CHistoryNode*    p_history;
    p_history = BeginChangeWH(EHCL_TOPOLOGY,tr("rename by ordered heavy atoms"));
    if( p_history == NULL ) return (false);

    BeginUpdate();

    // sort atoms by local index
    QList<CAtom*>   loc_copy = Atoms;
    qSort(loc_copy.begin(),loc_copy.end(),LessThanByLocalIndex);

    int index = 0;
    foreach(CAtom* p_atom, loc_copy) {
        if( p_atom->IsVirtual() ) continue;

        index++;

        QString name = "%1%2";
        name = name.arg(PeriodicTable.GetSymbol(p_atom->GetZ()));
        name = name.arg(index);
        p_atom->SetName(name,p_history);

        // count hydrogens
        int num_h = 0;

        foreach(CBond* p_bond,p_atom->GetBonds()){
            CAtom* p_opposite = p_bond->GetOppositeAtom(p_atom);
            if( p_opposite == NULL ) continue;
            if( p_opposite->GetResidue() != this ) continue;
            if( p_opposite->IsVirtual() == false ) continue;
            num_h++;
        }

        foreach(CBond* p_bond,p_atom->GetBonds()){
            CAtom* p_opposite = p_bond->GetOppositeAtom(p_atom);
            if( p_opposite == NULL ) continue;
            if( p_opposite->GetResidue() != this ) continue;
            if( p_opposite->IsVirtual() == false ) continue;

            int h_index = p_opposite->GetLocIndex() - p_atom->GetLocIndex();

            QString name;
            if( num_h > 1 ){
                name = "%1%2%3";
                name = name.arg(PeriodicTable.GetSymbol(p_opposite->GetZ()));
                name = name.arg(index);
                name = name.arg(h_index);
            } else {
                name = "%1%2";
                name = name.arg(PeriodicTable.GetSymbol(p_opposite->GetZ()));
                name = name.arg(index);
            }

            p_opposite->SetName(name,p_history);
        }

    }

    EndUpdate();
    EndChangeWH();
    return(true);
}

//------------------------------------------------------------------------------

bool CResidue::MergeWithWH(CResidue* p_res)
{
    if( p_res == NULL ){
        INVALID_ARGUMENT("p_res == NULL");
    }

    CHistoryNode* p_history = BeginChangeWH(EHCL_TOPOLOGY,"merge two residues");
    if( p_history == NULL ) return (false);

    MergeWith(p_res,p_history);

    EndChangeWH();
    return(true);
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

CStructure* CResidue::GetStructure(void) const
{
    if( GetResidues() == NULL ) return(NULL);
    return( GetResidues()->GetStructure() );
}

//------------------------------------------------------------------------------

CResidueList* CResidue::GetResidues(void) const
{
    return(static_cast<CResidueList*>(parent()));
}

//------------------------------------------------------------------------------

int CResidue::GetNumberOfAtoms(void) const
{
    return(Atoms.count());
}

//------------------------------------------------------------------------------

int CResidue::GetSeqIndex(void) const
{
    return(SeqIndex);
}

//------------------------------------------------------------------------------

int CResidue::GetTopLocalIndex(void) const
{
    int top_index = 0;
    foreach(CAtom* p_atom,Atoms) {
        if( p_atom->GetLocIndex() > top_index){
            top_index = p_atom->GetLocIndex();
        }
    }
    return(top_index);
}

//------------------------------------------------------------------------------

QString CResidue::GetLongName(void) const
{
    QString text;
    text = QString().setNum(GetSeqIndex()) + " / " + GetName()
           + " / " + GetChain();
    return(text);
}

//------------------------------------------------------------------------------

const QString& CResidue::GetChain(void) const
{
    return(Chain);
}

//------------------------------------------------------------------------------

const QString& CResidue::GetType(void) const
{
    return(Type);
}

//------------------------------------------------------------------------------

bool CResidue::IsOwned(CAtom* p_atom)
{
    return( Atoms.indexOf(p_atom) >= 0 );
}

//------------------------------------------------------------------------------

QList<CAtom*>&  CResidue::GetAtoms(void)
{
    return(Atoms);
}

//------------------------------------------------------------------------------

QList<CBond*> CResidue::GetBonds(bool include_connectors)
{
    QSet<CBond*>  bm;

    foreach(CAtom* p_atom,Atoms) {
        foreach(CBond* p_bond,p_atom->GetBonds()) {
            CAtom* p_at1 = p_bond->GetFirstAtom();
            CAtom* p_at2 = p_bond->GetSecondAtom();
            if( include_connectors ) {
                bm.insert(p_bond);
            } else {
                if( p_at1->GetResidue() == p_at2->GetResidue() ) {
                    bm.insert(p_bond);
                }
            }
        }
    }

    return(bm.toList());
}

//------------------------------------------------------------------------------

QList<CAtom*> CResidue::GetConnectorAtoms(bool opposite)
{
    QList<CAtom*> ca;

    foreach(CAtom* p_atom,Atoms) {
        foreach(CBond* p_bond,p_atom->GetBonds()) {
            CAtom* p_at1 = p_bond->GetFirstAtom();
            CAtom* p_at2 = p_bond->GetSecondAtom();
            if( p_at1->GetResidue() == p_at2->GetResidue() ) continue;
            if( opposite ){
                if( p_at1->GetResidue() == this ){
                    ca.append(p_at2);
                }
                if( p_at2->GetResidue() == this ){
                    ca.append(p_at1);
                }
            } else {
                if( p_at1->GetResidue() == this ){
                    ca.append(p_at1);
                }
                if( p_at2->GetResidue() == this ){
                    ca.append(p_at2);
                }
            }
        }
    }

    return(ca);
}

//------------------------------------------------------------------------------

QList<CBond*> CResidue::GetConnectorBonds(void)
{
    QList<CBond*> cb;

    foreach(CAtom* p_atom,Atoms) {
        foreach(CBond* p_bond,p_atom->GetBonds()) {
            CAtom* p_at1 = p_bond->GetFirstAtom();
            CAtom* p_at2 = p_bond->GetSecondAtom();
            if( p_at1->GetResidue() != p_at2->GetResidue() ) {
                cb.append(p_bond);
            }
        }
    }

    return(cb);
}

//------------------------------------------------------------------------------

CAtom* CResidue::FindAtomByIndex(int index)
{
    foreach(CAtom* p_atom,Atoms) {
        if( p_atom->GetIndex() == index ) return(p_atom);
    }
    return(NULL);
}

//------------------------------------------------------------------------------

bool CResidue::IsAnyAtomSelected(bool use_manip_flag)
{
    foreach(CAtom* p_atom,Atoms) {
        if( use_manip_flag ){
            if( p_atom->IsFlagSet(EPOF_MANIP_FLAG) ) return(true);
        } else {
            if( p_atom->IsFlagSet(EPOF_SELECTED) ) return(true);
        }
    }
    return(false);
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

void CResidue::SetName(const QString& name,CHistoryNode* p_history)
{
    CProObject::SetName(name,p_history);
    EmitOnStatusChanged(ESC_NAME);
    GetResidues()->EmitOnResidueListChanged();
    GetStructure()->GetAtoms()->EmitOnAtomListChanged();
}

//------------------------------------------------------------------------------

void CResidue::SetDescription(const QString& descrip,CHistoryNode* p_history)
{
    CProObject::SetDescription(descrip,p_history);
    EmitOnStatusChanged(ESC_DESCRIPTION);
    GetResidues()->EmitOnResidueListChanged();
}

//------------------------------------------------------------------------------

void CResidue::SetSeqIndex(int seqidx,CHistoryNode* p_history)
{
    if( SeqIndex == seqidx ) return;

    if( p_history ){
        CResidueSeqIndexHI* p_item = new CResidueSeqIndexHI(this,seqidx);
        p_history->Register(p_item);
    }

    SeqIndex = seqidx;
    EmitOnStatusChanged(ESC_OTHER);
    Changed = true;
    GetResidues()->EmitOnResidueListChanged();
    GetStructure()->GetAtoms()->EmitOnAtomListChanged();
}

//------------------------------------------------------------------------------

void CResidue::SetChain(const QString& chain,CHistoryNode* p_history)
{
    if( Chain == chain ) return;

    if( p_history ){
        CResidueChainHI* p_item = new CResidueChainHI(this,chain);
        p_history->Register(p_item);
    }

    Chain = chain;
    EmitOnStatusChanged(ESC_OTHER);
    Changed = true;
    GetResidues()->EmitOnResidueListChanged();
    GetStructure()->GetAtoms()->EmitOnAtomListChanged();
}

//------------------------------------------------------------------------------

void CResidue::SetType(const QString& type,CHistoryNode* p_history)
{
    if( Type == type ) return;

    if( p_history ){
        CResidueTypeHI* p_item = new CResidueTypeHI(this,type);
        p_history->Register(p_item);
    }

    Type = type;
    EmitOnStatusChanged(ESC_OTHER);
    Changed = true;
    GetResidues()->EmitOnResidueListChanged();
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

void CResidue::Delete(bool keep_connectors,CHistoryNode* p_history)
{
    if( ! keep_connectors ){
        foreach(CAtom* p_atom,Atoms) {
            p_atom->RemoveFromBaseList(p_history);
        }
    } else {
        // break connector bonds
        foreach(CAtom* p_atom,Atoms) {
            foreach(CBond* p_bond,p_atom->GetBonds()){
                CAtom* p_opposite = p_bond->GetOppositeAtom(p_atom);
                if( p_opposite->GetResidue() != this ){
                    p_bond->Break(p_history);
                }
            }
        }

        // delete all atoms
        foreach(CAtom* p_atom,Atoms) {
            p_atom->RemoveFromBaseList(p_history);
        }
    }

    // remove residue
    RemoveFromBaseList(p_history);
}

//------------------------------------------------------------------------------

void CResidue::AddAtom(CAtom* p_atom,CHistoryNode* p_history)
{
    if( p_atom == NULL ){
        INVALID_ARGUMENT("p_atom == NULL");
    }

    // is the atom owned?
    if( IsOwned(p_atom) ){
        return;
    }

    // unregister atom from previous residue
    CResidue* p_prevres = p_atom->GetResidue();
    if( p_prevres ){
        p_prevres->RemoveAtom(p_atom,p_history);
    }

    if( p_history != NULL ) {
        CHistoryItem* p_item = new CResidueAddAtomHI(this,p_atom,EHID_FORWARD);
        p_history->Register(p_item);
    }

    // add atom
    Atoms.append(p_atom);

    // register object for original object
    p_atom->Residue = this;
    p_atom->EmitOnStatusChanged();

    // fix local index
    if( p_atom->GetLocIndex() == -1 ){
        p_atom->SetLocIndex(GetTopLocalIndex()+1,p_history);
    }

    // sort atoms and notify change
    SortAtoms();
}

//------------------------------------------------------------------------------

void CResidue::RemoveAtom(CAtom* p_atom,CHistoryNode* p_history)
{
    if( p_atom == NULL ){
        INVALID_ARGUMENT("p_atom == NULL");
    }

    // is the atom owned?
    if( ! IsOwned(p_atom) ){
        LOGIC_ERROR("p_atom is not in the residue");
    }

    if( p_history != NULL ) {
        CHistoryItem* p_item = new CResidueAddAtomHI(this,p_atom,EHID_BACKWARD);
        p_history->Register(p_item);
    }

    // remove atom
    Atoms.removeOne(p_atom);

    // unregister object for original object
    p_atom->Residue = NULL;
    p_atom->EmitOnStatusChanged();

    // sort atoms and notify change
    SortAtoms();
}

//------------------------------------------------------------------------------

void CResidue::MergeWith(CResidue* p_res,CHistoryNode* p_history)
{
    if( p_res == NULL ){
        INVALID_ARGUMENT("p_res == NULL");
    }

    BeginUpdate();

    // move atoms to this residue
    foreach(CAtom* p_atom,p_res->GetAtoms()){
        AddAtom(p_atom,p_history);
    }

    // delete residue
    p_res->RemoveFromBaseList(p_history);

    EmitOnAtomListChanged();
    EndUpdate();
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

void CResidue::SortAtoms(void)
{
    // skip sorting when residue list in updating state
    if( GetResidues()->IsUpdating() ) return;

    // sort atoms by local index
    qSort(Atoms.begin(),Atoms.end(),LessThanByLocalIndex);

    // emit event
    EmitOnAtomListChanged();
}

//------------------------------------------------------------------------------

bool CResidue::LessThanByLocalIndex(CAtom* p_left,CAtom* p_right)
{
    return( p_left->GetLocIndex() < p_right->GetLocIndex() );
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

void CResidue::LoadData(CXMLElement* p_ele)
{
    // check input data -----------------------------
    if( p_ele == NULL ) {
        INVALID_ARGUMENT("p_ele is NULL");
    }

    if( p_ele->GetName() != "residue" ) {
        LOGIC_ERROR("element is not residue");
    }

    // load object info -----------------------------
    CProObject::LoadData(p_ele);

    // read residue specific data (all of them are optional)
    p_ele->GetAttribute("chain",Chain);
    p_ele->GetAttribute("seqid",SeqIndex);
    p_ele->GetAttribute("type",Type);

    // atoms are not loaded here
    // residues are referenced by atoms therefore atoms include themself to residues
    // for further details, see: CAtom:LoadData()
}

//------------------------------------------------------------------------------

void CResidue::SaveData(CXMLElement* p_ele)
{
    // check input data -----------------------------
    if( p_ele == NULL ) {
        INVALID_ARGUMENT("p_ele is NULL");
    }

    if( p_ele->GetName() != "residue" ) {
        LOGIC_ERROR("element is not residue");
    }

    // save object info -----------------------------
    CProObject::SaveData(p_ele);

    // save object specific data --------------------
    if( ! Chain.isEmpty() ) p_ele->SetAttribute("chain",Chain);
    p_ele->SetAttribute("seqid",SeqIndex);
    if( ! Type.isEmpty() )p_ele->SetAttribute("type",Type);

    // atoms are not saved here
    // residues are referenced by atoms
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

void CResidue::BeginUpdate(void)
{
    UpdateLevel++;
    blockSignals(true);
}

//------------------------------------------------------------------------------

void CResidue::EndUpdate(bool do_not_sort)
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
        if( ObjectChanged.testFlag(ESC_NAME) ){
            emit OnStatusChanged(ESC_NAME);
        }
        if( ObjectChanged.testFlag(ESC_DESCRIPTION) ){
            emit OnStatusChanged(ESC_DESCRIPTION);
        }
        if( ObjectChanged.testFlag(ESC_OTHER) ){
            emit OnStatusChanged(ESC_OTHER);
        }
        if( ObjectChanged.testFlag(ESC_PARENT) ){
            emit OnStatusChanged(ESC_PARENT);
        }
        ObjectChanged = QFlags<EStatusChanged>();
    }
}

//------------------------------------------------------------------------------

void CResidue::EmitOnAtomListChanged(void)
{
    if( UpdateLevel > 0 ){
        Changed = true;
    } else {
        emit OnAtomListChanged();
    }
}

//------------------------------------------------------------------------------

void CResidue::EmitOnStatusChanged(EStatusChanged status)
{
    if( UpdateLevel > 0 ){
        ObjectChanged |= status;
    } else {
        emit OnStatusChanged(status);
    }
}

//------------------------------------------------------------------------------

void CResidue::ListSizeChanged(bool do_not_sort)
{
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

void CResidue::ChangeParent(CResidueList* p_newparent)
{
    // inform old parent
    GetResidues()->EmitOnResidueListChanged();
    // set new parent
    setParent(p_newparent);
    // inform new parent
    GetResidues()->EmitOnResidueListChanged();
    // inform object designers
    EmitOnStatusChanged(ESC_PARENT);
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================
