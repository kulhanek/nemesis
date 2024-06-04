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

#include <ResidueList.hpp>
#include <Structure.hpp>
#include <XMLElement.hpp>
#include <ErrorSystem.hpp>
#include <NemesisCoreModule.hpp>
#include <CategoryUUID.hpp>
#include <Project.hpp>
#include <Residue.hpp>
#include <ResidueHistory.hpp>
#include <AtomList.hpp>
#include <Atom.hpp>
#include <SelectionList.hpp>
#include <HistoryNode.hpp>
#include <ResidueListHistory.hpp>
#include <QStack>

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

CExtUUID        ResidueListID(
                    "{RESIDUE_LIST:79634e2f-8a7c-46f1-b71c-491d25ab74ac}",
                    "Residues");

CPluginObject   ResidueListObject(&NemesisCorePlugin,
                    ResidueListID,STRUCTURE_CAT,
                    ":/images/NemesisCore/structure/ResidueList.svg",
                    NULL);

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

CResidueList::CResidueList(CStructure* p_str)
    : CProObject(&ResidueListObject,p_str,p_str->GetProject())
{
    Changed = false;
    UpdateLevel = 0;
    ForceSorting = false;
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

CResidue* CResidueList::CreateResidueWH(const QString& name,const QString& chain,
                                        int seqindex)
{
    CHistoryNode*    p_history;
    p_history = BeginChangeWH(EHCL_TOPOLOGY,tr("new residue"));
    if( p_history == NULL ) return (NULL);

    CResidue* p_res =CreateResidue(name,chain,seqindex,p_history);

    EndChangeWH();

    return(p_res);
}

//------------------------------------------------------------------------------

CResidue* CResidueList::CreateResidueFromSelectedAtomsWH(const QString& name,
                                                    const QString& chain,
                                                    int seqindex)
{
    if( GetStructure()->GetAtoms()->GetNumberOfSelectedAtoms() == 0 ){
        GetProject()->TextNotification(ETNT_WARNING,"no selected atom for residue creation",ETNT_WARNING_DELAY);
    }

    CHistoryNode*    p_history;
    p_history = BeginChangeWH(EHCL_TOPOLOGY,tr("new residue from selected atoms"));
    if( p_history == NULL ) return (NULL);

    CResidue* p_res = CreateResidue(name,chain,seqindex,p_history);

    // add all selected atoms
    foreach(QObject* p_qobj,GetStructure()->GetAtoms()->children()) {
        CAtom* p_atom = static_cast<CAtom*>(p_qobj);
        if( p_atom->IsFlagSet(EPOF_SELECTED) ){
            p_res->AddAtom(p_atom,p_history);
        }
    }

    EndChangeWH();

    // reset current selection
    GetProject()->GetSelection()->ResetSelection();

    return(p_res);
}

//------------------------------------------------------------------------------

bool CResidueList::DeleteSelectedResiduesWH(bool keep_connectors)
{
    CHistoryNode*    p_history;

    p_history = BeginChangeWH(EHCL_TOPOLOGY,tr("delete selected residues"));
    if( p_history == NULL ) return (false);

        DeleteSelectedResidues(keep_connectors,p_history);

    EndChangeWH();

    return(true);
}

//------------------------------------------------------------------------------

bool CResidueList::DeleteEmptyResiduesWH(void)
{
    CHistoryNode*    p_history;
    p_history = BeginChangeWH(EHCL_TOPOLOGY,tr("delete empty residues"));
    if( p_history == NULL ) return (false);

        DeleteEmptyResidues(p_history);

    EndChangeWH();

    return(true);
}

//------------------------------------------------------------------------------

bool CResidueList::SequenceIndexByOrderWH(void)
{
    CHistoryNode*    p_history;
    p_history = BeginChangeWH(EHCL_TOPOLOGY,tr("update residue sequence indexes"));
    if( p_history == NULL ) return (false);

    BeginUpdate();

    int indx = 1;
    foreach(QObject* p_qobj,GetStructure()->GetResidues()->children()) {
        CResidue* p_res = static_cast<CResidue*>(p_qobj);
        p_res->SetSeqIndex(indx,p_history);
        indx++;
    }

    EndUpdate();
    EndChangeWH();
    return(true);
}

//------------------------------------------------------------------------------

bool CResidueList::SequenceIndexByReverseOrderWH(void)
{
    CHistoryNode*    p_history;
    p_history = BeginChangeWH(EHCL_TOPOLOGY,tr("update residue sequence indexes in reverse order"));
    if( p_history == NULL ) return (false);

    BeginUpdate();

    int indx = GetNumberOfResidues();
    foreach(QObject* p_qobj,GetStructure()->GetResidues()->children()) {
        CResidue* p_res = static_cast<CResidue*>(p_qobj);
        p_res->SetSeqIndex(indx,p_history);
        indx--;
    }

    EndUpdate();
    EndChangeWH();
    return(true);
}

//------------------------------------------------------------------------------

bool CResidueList::SequenceIndexByChainsWH(void)
{
    CHistoryNode*    p_history;
    p_history = BeginChangeWH(EHCL_TOPOLOGY,tr("update residue sequence indexes by chain orders"));
    if( p_history == NULL ) return (false);

    BeginUpdate();

    QList<CResidue*> residues;
    QList<CResidue*> alone_residues;

    // set for all EPOF_PROC_FLAG to false
    foreach(CResidue* p_res,GetStructure()->GetResidues()->findChildren<CResidue*>()){
        p_res->SetFlag(EPOF_PROC_FLAG,false);

        if( p_res->GetConnectorBonds().count() == 0 ){
            // add to alone list
            alone_residues.append(p_res);
        } else {
            residues.append(p_res); // add to the list for further processing
        }
    }

    int seq_index = 1;
    while( residues.count() > 0 ){

        CResidue*           p_res;
        QStack<CResidue*>   chain_stack;

        p_res = *(residues.begin());    // take the first item
        residues.removeOne(p_res);         // remove from the set
        chain_stack.push(p_res);        // add to stack

        QList<CResidue*> chain;

        // scan the chain
        while( chain_stack.isEmpty() == false ){
            p_res = chain_stack.pop();
            chain.append(p_res);
            // add to stack its neighbourous
            foreach(CAtom* p_atom,p_res->GetConnectorAtoms(true)){
                CResidue* p_ores = p_atom->GetResidue();
                if( residues.contains(p_ores) ){
                    chain_stack.push(p_ores);
                    residues.removeOne(p_ores);
                }
            }
        }

        // index chain
        SetSeqIndexesForChain(chain,seq_index,p_history);
    }

    // set indexes for the remainig residues
    foreach(CResidue* p_res,alone_residues){
        p_res->SetSeqIndex(seq_index,p_history);
        seq_index++;
    }

    EndUpdate();
    EndChangeWH();
    return(true);
}

//------------------------------------------------------------------------------

void CResidueList::SetSeqIndexesForChain(QList<CResidue*>& chain,int& seq_index,
                                         CHistoryNode*    p_history)
{
    // find head
    // head has one connector that has local index higher than the last atom
    // or it has the first connector very long (PBC situation)

    foreach(CResidue* p_res,chain){
        QList<CBond*> bonds = p_res->GetConnectorBonds();

        // get first connector
        if( bonds.count() != 1 ) continue;

        CAtom* p_atom1 = p_res->GetConnectorAtoms(false)[0];
        CAtom* p_lasta = p_res->GetAtoms().last();
        if( p_atom1->GetLocIndex() > p_lasta->GetLocIndex() ){
            // head residue
            SetSeqIndexesForChain(p_res,seq_index,p_history);
            return;
        } else {
            // try to test for head with small number of atoms
            CAtom* p_atom2 = p_res->GetConnectorAtoms(true)[0];
            // check the last residue in the list
            CResidue* p_nres = p_atom2->GetResidue();

            CAtom* p_atom1 = p_nres->GetConnectorAtoms(false)[0];
            CAtom* p_lasta = p_nres->GetAtoms().last();

            if( p_atom1->GetLocIndex() < p_lasta->GetLocIndex() ){
                // head residue
                SetSeqIndexesForChain(p_res,seq_index,p_history);
                return;
            }
        }
    }

    // try handle PBC situations
    foreach(CResidue* p_res,chain){
        QList<CBond*> bonds = p_res->GetConnectorBonds();

        // get first connector
        if( bonds.count() <= 1 ) continue;

        if( GetStructure()->PBCInfo.IsPBCEnabled() ){
            if( bonds.first()->GetLength() > GetStructure()->PBCInfo.GetLargestSphereRadius() ){
                // head residue
                SetSeqIndexesForChain(p_res,seq_index,p_history);
                return;
            }
        }
    }

    // no head residue? - take first, this will happen for small terminal residues
    SetSeqIndexesForChain(chain.first(),seq_index,p_history);
    return;
}

//------------------------------------------------------------------------------

void CResidueList::SetSeqIndexesForChain(CResidue* p_res,int& seq_index,
                                         CHistoryNode*  p_history)
{
    QStack<CResidue*>   chain_stack;
    chain_stack.push(p_res);            // add to stack
    p_res->SetFlag(EPOF_PROC_FLAG,true);

    // index
    while( chain_stack.isEmpty() == false ){
        p_res = chain_stack.pop();

        p_res->SetSeqIndex(seq_index,p_history);
        seq_index++;

        // add to stack its neighbourous
        QList<CAtom*> connectors;
        QList<CBond*> bonds = p_res->GetConnectorBonds();
        CBond* p_fbond = bonds.first();
        foreach(CAtom* p_atom,p_res->GetConnectorAtoms(true)){
            CResidue* p_ores = p_atom->GetResidue();

            if( GetStructure()->PBCInfo.IsPBCEnabled() ){
                if( (p_fbond->GetFirstAtom() == p_atom) || (p_fbond->GetSecondAtom() == p_atom) ){
                    if( p_fbond->GetLength() > GetStructure()->PBCInfo.GetLargestSphereRadius() ){
                        // skip this connector
                        continue;
                    }
                }
            }

            if( p_ores->IsFlagSet(EPOF_PROC_FLAG) == false ){
                connectors.prepend(p_atom); // we need to reverse the order
            }
        }
        // in reverse order
        foreach(CAtom* p_atom,connectors){
            CResidue* p_ores = p_atom->GetResidue();
            p_ores->SetFlag(EPOF_PROC_FLAG,true);
            chain_stack.push(p_ores);
        }
    }
}

//------------------------------------------------------------------------------

bool CResidueList::SetChainNamesWH(void)
{
    CHistoryNode*    p_history;
    p_history = BeginChangeWH(EHCL_TOPOLOGY,tr("set chain names"));
    if( p_history == NULL ) return (false);

    BeginUpdate();

    // make list of resideus
    QList<CResidue*>    residues;

    foreach(QObject* p_qobj,GetStructure()->GetResidues()->children()){
        CResidue* p_res = dynamic_cast<CResidue*>(p_qobj);
        if( p_res == NULL ) continue;
        if( p_res->GetConnectorBonds().count() == 0 ){
            // if no partners - set no chain name
            p_res->SetChain("",p_history);
        } else {
            residues.append(p_res); // add to the list for further processing
        }
    }

    // detect chains for the rest
    int chain_index = 0;
    while( residues.count() > 0 ){

        CResidue*           p_res;
        QStack<CResidue*>   chain_stack;

        p_res = *(residues.begin());    // take the first item
        residues.removeOne(p_res);         // remove from the set
        chain_stack.push(p_res);        // add to stack

        // generate chain name
        QString chain_name;
        int l_chain_index = chain_index;
        do{
            chain_name += QChar('A' + l_chain_index%67 );
            l_chain_index = l_chain_index / 67;
        } while( l_chain_index > 0  );

        // scan the chain
        while( chain_stack.isEmpty() == false ){
            p_res = chain_stack.pop();
            p_res->SetChain(chain_name,p_history);
            // add to stack its neighbourous
            foreach(CAtom* p_atom,p_res->GetConnectorAtoms(true)){
                CResidue* p_ores = p_atom->GetResidue();
                if( residues.contains(p_ores) ){
                    chain_stack.push(p_ores);
                    residues.removeOne(p_ores);
                }
            }
        }

        chain_index++;
    }

    EndUpdate();
    EndChangeWH();

    return(true);
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

CResidue* CResidueList::CreateResidue(const QString& name,CHistoryNode* p_history)
{
    return( CreateResidue(name,"",-1,p_history) );
}

//------------------------------------------------------------------------------

CResidue* CResidueList::CreateResidue(const QString& name,const QString& chain,
                                      int seqindex,CHistoryNode* p_history)
{
    // create residue
    CResidue* p_res = new CResidue(this);

    // set data
    p_res->SetName(name);
    p_res->SetChain(chain);

    if( seqindex == -1 ){
        seqindex = GetTopSeqIndex() + 1;
    }

    p_res->SetSeqIndex(seqindex);

    // register elementary change to history list
    if( p_history != NULL ) {
        CResidueHI* p_resdata = new CResidueHI(p_res,EHID_FORWARD);
        p_history->Register(p_resdata);
    }

    ListSizeChanged();

    return(p_res);
}

//------------------------------------------------------------------------------

CResidue* CResidueList::CreateResidue(CXMLElement* p_ele,CHistoryNode* p_history)
{
    CResidue* p_res = new CResidue(this,true);
    p_res->LoadData(p_ele);

    if( p_history != NULL ) {
        CResidueHI* p_resdata = new CResidueHI(p_res,EHID_FORWARD);
        p_history->Register(p_resdata);
    }

    ListSizeChanged();
    return(p_res);
}

//------------------------------------------------------------------------------

void CResidueList::SortResidues(void)
{
    QList<CResidue*> loc_copy;

    // create local copy of list and remove all from children()
    foreach(QObject* p_qobj,children()) {
        CResidue* p_res = static_cast<CResidue*>(p_qobj);
        loc_copy.append(p_res);
        p_res->setParent(NULL);
    }

    // sort residues
    std::sort(loc_copy.begin(),loc_copy.end(),LessThanBySeqIndex);

    // add residues to the list in sorted order
    foreach(CResidue* p_res,loc_copy) {
        p_res->setParent(this);
    }

    EmitOnResidueListChanged();
}

//------------------------------------------------------------------------------

bool CResidueList::LessThanBySeqIndex(CResidue* p_left,CResidue* p_right)
{
    return( p_left->GetSeqIndex() < p_right->GetSeqIndex() );
}

//------------------------------------------------------------------------------

void CResidueList::DeleteSelectedResidues(bool keep_connectors,CHistoryNode* p_history)
{
    BeginUpdate();
    foreach(QObject* p_qobj,children()) {
        CResidue* p_res = static_cast<CResidue*>(p_qobj);
        if( p_res->IsFlagSet(EPOF_SELECTED) ){
            p_res->Delete(keep_connectors,p_history);
        }
    }
    EndUpdate();
}

//------------------------------------------------------------------------------

void CResidueList::DeleteEmptyResidues(CHistoryNode* p_history)
{
    BeginUpdate();
    foreach(QObject* p_qobj,children()) {
        CResidue* p_res = static_cast<CResidue*>(p_qobj);
        if( p_res->GetNumberOfAtoms() == 0 ){
            p_res->RemoveFromBaseList(p_history);
        }
    }
    EndUpdate();
}

//------------------------------------------------------------------------------

void CResidueList::UnregisterAllRegisteredResidues(CHistoryNode* p_history)
{
    foreach(QObject* p_qobj,children()) {
        CResidue* p_res = static_cast<CResidue*>(p_qobj);
        p_res->UnregisterAllRegisteredObjects(p_history);
    }
}

//------------------------------------------------------------------------------

void CResidueList::MoveAllResiduesFrom(CResidueList* p_source,CHistoryNode* p_history)
{
    if( this == p_source ) return;  // source and target must be different

    BeginUpdate();
    p_source->BeginUpdate();

    QVector<int>    indexes;
    indexes.reserve(p_source->GetNumberOfResidues());

    int topindex = GetTopSeqIndex();
    int lowindex = p_source->GetLowSeqIndex();

    foreach(QObject* p_qobj, p_source->children()){
        CResidue* p_res = static_cast<CResidue*>(p_qobj);
        int index = p_res->GetIndex();
        indexes.push_back(index);
        p_res->ChangeParent(this);
        if( topindex >= lowindex ){
            int si = topindex - lowindex + 1 + p_res->GetSeqIndex();
            p_res->SetSeqIndex(si);
        }
    }

    // record the change to history list
    if( p_history ){
        CHistoryItem* p_hi = new CResidueListChangeParentHI(GetStructure(),
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

CStructure* CResidueList::GetStructure(void) const
{
    return(dynamic_cast<CStructure*>(parent()));
}

//------------------------------------------------------------------------------

CResidue* CResidueList::SearchBySeqIndex(int seqidx)
{
    foreach(QObject* p_qobj,children()) {
        CResidue* p_res = static_cast<CResidue*>(p_qobj);
        if( p_res->GetSeqIndex() == seqidx) return(p_res);
    }
    return(NULL);
}

//------------------------------------------------------------------------------

int CResidueList::GetLowSeqIndex(void)
{
    int low_index = 0;
    if( ! children().isEmpty() ){
        CResidue* p_res = static_cast<CResidue*>(children().first());
        low_index = p_res->GetSeqIndex();
    }
    foreach(QObject* p_qobj,children()) {
        CResidue* p_res = static_cast<CResidue*>(p_qobj);
        if( p_res->GetSeqIndex() < low_index){
            low_index = p_res->GetSeqIndex();
        }
    }
    return(low_index);
}

//------------------------------------------------------------------------------

int CResidueList::GetTopSeqIndex(void)
{
    int top_index = 0;
    if( ! children().isEmpty() ){
        CResidue* p_res = static_cast<CResidue*>(children().first());
        top_index = p_res->GetSeqIndex();
    }
    foreach(QObject* p_qobj,children()) {
        CResidue* p_res = static_cast<CResidue*>(p_qobj);
        if( p_res->GetSeqIndex() > top_index){
            top_index = p_res->GetSeqIndex();
        }
    }
    return(top_index);
}

//------------------------------------------------------------------------------

int CResidueList::GetNumberOfResidues(void)
{
    return(children().count());
}

//------------------------------------------------------------------------------

CResidue* CResidueList::FindResidueByIndex(int index)
{
    foreach(QObject* p_qobj,children()) {
        CResidue* p_res = static_cast<CResidue*>(p_qobj);
        if( p_res->GetIndex() == index){
            return(p_res);
        }
    }
    return(NULL);
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

void CResidueList::LoadData(CXMLElement* p_ele,CHistoryNode* p_history)
{
    // check input data -----------------------------
    if( p_ele == NULL ) {
        INVALID_ARGUMENT("p_ele is NULL");
    }

    if( p_ele->GetName() != "residues" ) {
        LOGIC_ERROR("p_ele is not residues");
    }

    // load object info -----------------------------
    CProObject::LoadData(p_ele);

    // load data ------------------------------------
    CXMLElement* p_ael;
    p_ael = p_ele->GetFirstChildElement("residue");

    while( p_ael != NULL ) {
        CreateResidue(p_ael,p_history);
        p_ael = p_ael->GetNextSiblingElement("residue");
    }
}

//------------------------------------------------------------------------------

void CResidueList::LoadData(CXMLElement* p_ele)
{
    LoadData(p_ele,NULL);
}

//------------------------------------------------------------------------------

void CResidueList::SaveData(CXMLElement* p_ele,bool selected)
{
    // check input data -----------------------------
    if( p_ele == NULL ) {
        INVALID_ARGUMENT("p_ele is NULL");
    }

    if( p_ele->GetName() != "residues" ) {
        LOGIC_ERROR("p_ele is not residues");
    }

    // save object info -----------------------------
    CProObject::SaveData(p_ele);

    // save data ------------------------------------
    foreach(QObject* p_qobj,children()) {
        CResidue* p_res = static_cast<CResidue*>(p_qobj);
        if( selected ){
            if( p_res->IsFlagSet(EPOF_MANIP_FLAG) ){
                CXMLElement* p_ael = p_ele->CreateChildElement("residue");
                p_res->SaveData(p_ael);
            }
        } else {
            CXMLElement* p_ael = p_ele->CreateChildElement("residue");
            p_res->SaveData(p_ael);
        }
    }
}

//------------------------------------------------------------------------------

void CResidueList::SaveData(CXMLElement* p_ele)
{
    SaveData(p_ele,false);
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

void CResidueList::BeginUpdate(void)
{
    if( UpdateLevel == 0 ){
        blockSignals(true);
        foreach(QObject* p_qobj,children()) {
            CResidue* p_res = static_cast<CResidue*>(p_qobj);
            p_res->BeginUpdate();
        }
    }
    UpdateLevel++;
}

//------------------------------------------------------------------------------

void CResidueList::EndUpdate(bool do_not_sort)
{
    if( UpdateLevel == 0 ) return;
    UpdateLevel--;
    if( UpdateLevel == 0 ){
        foreach(QObject* p_qobj,children()) {
            CResidue* p_res = static_cast<CResidue*>(p_qobj);
            p_res->EndUpdate(do_not_sort);
        }
        if( ForceSorting ) {
            if( ! do_not_sort ) SortResidues();
            ForceSorting = false;
        }
        blockSignals(false);
        if( Changed ){
            emit OnResidueListChanged();
            Changed = false;
        }
    }
}

//------------------------------------------------------------------------------

bool CResidueList::IsUpdating(void)
{
    return(UpdateLevel > 0);
}

//------------------------------------------------------------------------------

void CResidueList::EmitOnResidueListChanged(void)
{
    if( UpdateLevel > 0 ) Changed = true;
    emit OnResidueListChanged();
}

//------------------------------------------------------------------------------

void CResidueList::ListSizeChanged(bool do_not_sort)
{
    Changed = true;
    ForceSorting = ! do_not_sort;
    if( UpdateLevel > 0 ){
        Changed = true;
        return;
    }
    if( ! do_not_sort ) SortResidues();
    EmitOnResidueListChanged();
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================
