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

#include <StructureList.hpp>
#include <Structure.hpp>
#include <XMLElement.hpp>
#include <ErrorSystem.hpp>
#include <NemesisCoreModule.hpp>
#include <CategoryUUID.hpp>
#include <Project.hpp>
#include <Graphics.hpp>
#include <StructureListHistory.hpp>
#include <StructureHistory.hpp>
#include <GraphicsProfileList.hpp>
#include <AtomList.hpp>
#include <BondList.hpp>
#include <ResidueList.hpp>
#include <RestraintList.hpp>

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

CExtUUID        StructureListID(
                    "{STRUCTURE_LIST:caf85f99-147c-4eb4-a7ba-15edc7e47b53}",
                    "Structures");

CPluginObject   StructureListObject(&NemesisCorePlugin,
                    StructureListID,STRUCTURE_CAT,
                    ":/images/NemesisCore/structure/StructureList.svg",
                    NULL);

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

CStructureList::CStructureList(CProject* p_project)
    : CProObject(&StructureListObject,p_project,p_project)
{
    ConFlags |= EECOF_SUB_CONTAINERS;
    ConFlags |= EECOF_DYNAMICAL_CONTENTS;
    ActiveStructure = NULL;
    GeometryUpdateLevel = 0;
    Changed = false;
    UpdateLevel = 0;
    ForceSorting = false;
}

//------------------------------------------------------------------------------

CStructureList::CStructureList(CProject* p_project,bool no_index)
    : CProObject(&StructureListObject,p_project,p_project,no_index)
{
    ConFlags |= EECOF_SUB_CONTAINERS;
    ConFlags |= EECOF_DYNAMICAL_CONTENTS;
    ActiveStructure = NULL;
    GeometryUpdateLevel = 0;
    Changed = false;
    UpdateLevel = 0;
    ForceSorting = false;
}

//------------------------------------------------------------------------------

CStructureList::~CStructureList(void)
{
    foreach(QObject* p_qobj,children()) {
        delete p_qobj;
    }
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

CStructure* CStructureList::CreateStructureWH(bool activate,const QString& name,
                                              const QString& descr)
{
    CHistoryNode*    p_history;

    p_history = BeginChangeWH(EHCL_STRUCTURES,tr("new structure"));
    if( p_history == NULL ) return (NULL);


    CStructure* p_str = CreateStructure(name,descr,p_history);

    if( activate ){
        SetActiveStructure(p_str,p_history);
    }

    EndChangeWH();

    return(p_str);
}

//------------------------------------------------------------------------------

CStructure* CStructureList::CreateStructureFromSelectedResiduesWH(const QString& name,const QString& descr)
{
    CHistoryNode*    p_history;

    p_history = BeginChangeWH(EHCL_STRUCTURES,tr("new structure from selected residues"));
    if( p_history == NULL ) return (NULL);

    CStructure* p_str = NULL;

    // create new structure
    CStructure* p_nstr = CreateStructure(name,descr,p_history);

    CXMLDocument xml_doc;
    CXMLElement* p_ele = xml_doc.CreateChildElement("structure");

    // save the selected residues from all structures
    SaveSelectedResidues(p_ele);

    // load data
    GetProject()->BeginLinkProcedure();
    p_nstr->LoadStructureData(p_ele);
    GetProject()->EndLinkProcedure();

    // activate new structure
    SetActiveStructure(p_nstr,p_history);

    EndChangeWH();

    return(p_str);
}

//------------------------------------------------------------------------------

CStructure* CStructureList::DuplicateStructureWH(CStructure* p_ostr)
{
    CHistoryNode* p_history = BeginChangeWH(EHCL_STRUCTURES,tr("duplicate structure"));
    if( p_history == NULL ){
        // change is not allowed
        return(NULL);
    }

    CStructure* p_nstr = DuplicateStructure(p_ostr,true,p_history);

    EndChangeWH();

    return(p_nstr);
}

//------------------------------------------------------------------------------

bool CStructureList::SetActiveStructureWH(CStructure* p_str)
{
    CHistoryNode*    p_history;

    p_history = BeginChangeWH(EHCL_STRUCTURES,tr("activate structure"));
    if( p_history == NULL ) return (false);

    SetActiveStructure(p_str,p_history);

    EndChangeWH();

    return(true);
}

//------------------------------------------------------------------------------

bool CStructureList::DeleteSelectedStructuresWH(void)
{
    if( GetNumberOfSelectedStructures() == 0 ){
        GetProject()->TextNotification(ETNT_WARNING,"no selected structure is available to be deleted",ETNT_WARNING_DELAY);
        return(false);
    }

    CHistoryNode*    p_history;

    p_history = BeginChangeWH(EHCL_STRUCTURES,tr("delete selected structures"));
    if( p_history == NULL ) return (false);

        DeleteSelectedStructures(p_history);

    EndChangeWH();

    return(true);
}

//------------------------------------------------------------------------------

bool CStructureList::DeleteEmptyStructuresWH(void)
{
    if( GetNumberOfEmptyStructures() == 0 ){
        GetProject()->TextNotification(ETNT_WARNING,"no empty structure is available to be deleted",ETNT_WARNING_DELAY);
        return(false);
    }

    CHistoryNode*    p_history;
    p_history = BeginChangeWH(EHCL_STRUCTURES,tr("delete empty structures"));
    if( p_history == NULL ) return (false);

        DeleteEmptyStructures(p_history);

    EndChangeWH();

    return(true);
}

//------------------------------------------------------------------------------

bool CStructureList::DeleteAllStructuresWH(void)
{
    if( children().count() == 0 ){
        GetProject()->TextNotification(ETNT_WARNING,"no structure is available to be deleted",ETNT_WARNING_DELAY);
        return(false);
    }

    CHistoryNode*    p_history;
    p_history = BeginChangeWH(EHCL_STRUCTURES,tr("delete all structures"));
    if( p_history == NULL ) return (false);

        DeleteAllStructures(p_history);

    EndChangeWH();

    return(true);
}

//------------------------------------------------------------------------------

bool CStructureList::SequenceIndexByOrderWH(void)
{
    CHistoryNode*    p_history;
    p_history = BeginChangeWH(EHCL_STRUCTURES,tr("update structure sequence indexes"));
    if( p_history == NULL ) return (false);

    BeginUpdate();

    int indx = 1;
    foreach(QObject* p_qobj,children()) {
        CStructure* p_str = static_cast<CStructure*>(p_qobj);
        p_str->SetSeqIndex(indx,p_history);
        indx++;
    }

    EndUpdate();
    EndChangeWH();
    return(true);
}

//------------------------------------------------------------------------------

bool CStructureList::MergeStructuresWH(const QList<CStructure*>& structures)
{
    if( structures.size() <= 1 ){
        GetProject()->TextNotification(ETNT_WARNING,"at least two structures are required for merging",ETNT_WARNING_DELAY);
        return(false);
    }

    CHistoryNode*    p_history;
    p_history = BeginChangeWH(EHCL_TOPOLOGY,tr("merge structures"));
    if( p_history == NULL ) return (false);

    BeginUpdate();

    MergeStructures(structures,p_history);

    EndUpdate();
    EndChangeWH();
    return(true);
}

//------------------------------------------------------------------------------

bool CStructureList::MergeAllStructuresWH(void)
{
    if( children().count() <= 1 ){
        GetProject()->TextNotification(ETNT_WARNING,"at least two structures are required for merging",ETNT_WARNING_DELAY);
        return(false);
    }

    // prepare the list of all structures
    QList<CStructure*> structures;
    foreach(QObject* p_qobj, children()){
        CStructure* p_str = static_cast<CStructure*>(p_qobj);
        structures.push_back(p_str);
    }

    // and merge them
    CHistoryNode*    p_history;
    p_history = BeginChangeWH(EHCL_TOPOLOGY,tr("merge all structures"));
    if( p_history == NULL ) return (false);

    BeginUpdate();

    MergeStructures(structures,p_history);

    EndUpdate();
    EndChangeWH();
    return(true);
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

CStructure* CStructureList::CreateStructure(const QString& name,const QString& descr,CHistoryNode* p_history)
{
    BeginUpdate();

    QString str_name = name;

    if( str_name.isEmpty() ){
        str_name = GenerateName(tr("Structure %1"));
    }

    // find next seq index
    int seqindx = 0;
    foreach(QObject* p_qobj,children()) {
        CStructure* p_str = static_cast<CStructure*>(p_qobj);
        if( p_str->GetSeqIndex() > seqindx ){
            seqindx = p_str->GetSeqIndex();
        }
    }

    CStructure* p_str = new CStructure(this);
    p_str->SetName(str_name);
    p_str->SetDescription(descr);
    seqindx++;
    p_str->SetSeqIndex(seqindx);

    if( p_history ){
        CStructureHI* p_item = new CStructureHI(p_str,EHID_FORWARD);
        p_history->Register(p_item);
    }

    // do we have active structure?
    if( ActiveStructure == NULL ){
        SetActiveStructure(p_str,p_history);
    }

    ForceSorting = true;
    EndUpdate();

    emit OnChildContainerAdded(this,p_str);

    return(p_str);
}

//------------------------------------------------------------------------------

CStructure* CStructureList::CreateStructure(CXMLElement* p_ele,CHistoryNode* p_history)
{
    BeginUpdate();

    CStructure* p_str = new CStructure(this);
    p_str->LoadData(p_ele);

    if( p_history ){
        CStructureHI* p_item = new CStructureHI(p_str,EHID_FORWARD);
        p_history->Register(p_item);
    }

    // do we have active structure?
    if( ActiveStructure == NULL ){
        SetActiveStructure(p_str,p_history);
    }

    ForceSorting = true;
    EndUpdate();

    emit OnChildContainerAdded(this,p_str);

    return(p_str);
}

//------------------------------------------------------------------------------

void CStructureList::SetActiveStructure(CStructure* p_str,CHistoryNode* p_history)
{
    // no change
    if( ActiveStructure == p_str ) return;

    // record change if necessary
    if( p_history ){
        CSetActiveStructureHI* p_item = new CSetActiveStructureHI(this,
                                                    ActiveStructure,p_str,EHID_FORWARD);
        p_history->Register(p_item);
    }

    // change the active structure
    ActiveStructure = p_str;

    // update master selection object
    GetProject()->GetGraphics()->GetProfiles()->SetSelectionMasterObject(ActiveStructure);

    EmitOnStructureListChanged();
}

//------------------------------------------------------------------------------

CStructure* CStructureList::DuplicateStructure(CStructure* p_ostr,bool activate, CHistoryNode* p_history)
{
    // create new structure
    CStructure* p_nstr = CreateStructure(QString(),QString(),p_history);

    CXMLDocument xml_doc;
    CXMLElement* p_ele = xml_doc.CreateChildElement("structure");

    // save the structure
    p_ostr->SaveStructureData(p_ele);

    // load data
    GetProject()->BeginLinkProcedure();
    p_nstr->LoadStructureData(p_ele);
    GetProject()->EndLinkProcedure();

    // activate new structure
    if( activate ) SetActiveStructure(p_nstr,p_history);

    return(p_nstr);
}

//------------------------------------------------------------------------------

void CStructureList::DeleteSelectedStructures(CHistoryNode* p_history)
{
    BeginUpdate();
    foreach(QObject* p_qobj,children()) {
        CStructure* p_str = static_cast<CStructure*>(p_qobj);
        if( p_str->IsFlagSet(EPOF_SELECTED) ){
            p_str->RemoveFromBaseList(p_history);
        }
    }
    EndUpdate();
}

//------------------------------------------------------------------------------

void CStructureList::DeleteEmptyStructures(CHistoryNode* p_history)
{
    BeginUpdate();
    foreach(QObject* p_qobj,children()) {
        CStructure* p_str = static_cast<CStructure*>(p_qobj);
        if( p_str->GetAtoms()->GetNumberOfAtoms() == 0 ){
            p_str->RemoveFromBaseList(p_history);
        }
    }
    EndUpdate();
}

//------------------------------------------------------------------------------

void CStructureList::DeleteAllStructures(CHistoryNode* p_history)
{
    BeginUpdate();
    foreach(QObject* p_qobj,children()) {
        CStructure* p_str = static_cast<CStructure*>(p_qobj);
        p_str->RemoveFromBaseList(p_history);
    }
    EndUpdate();
}

//------------------------------------------------------------------------------

void CStructureList::MergeStructures(const QList<CStructure*>& structures,CHistoryNode* p_history)
{
    if( structures.size() <= 1 ) return;    // not enough structures to merge

    // create local copy and sort the list by sequence index
    QList<CStructure*>  lstrs(structures);
    std::sort(lstrs.begin(),lstrs.end(),LessThanBySeqIndex);

    // remove the first structure, which will be the acceptor
    CStructure* p_mstr = lstrs.takeFirst();

    // activate the main structure
    SetActiveStructure(p_mstr,p_history);

    // for each other structure transfer the contents to the acceptor
    foreach(CStructure* p_tstr, lstrs){
        // transfer atoms
        p_mstr->GetAtoms()->MoveAllAtomsFrom(p_tstr->GetAtoms(),p_history);

        // transfer bonds
        p_mstr->GetBonds()->MoveAllBondsFrom(p_tstr->GetBonds(),p_history);

        // transfer residues
        p_mstr->GetResidues()->MoveAllResiduesFrom(p_tstr->GetResidues(),p_history);

        // transfer restraints
        p_mstr->GetRestraints()->MoveAllRestraintsFrom(p_tstr->GetRestraints(),p_history);

        // delete the structure
        p_tstr->RemoveFromBaseList(p_history);
    }
}

//------------------------------------------------------------------------------

void CStructureList::ClearStructures(void)
{
    BeginUpdate();

    foreach(QObject* p_object,children()){
        delete p_object;
    }
    // SetActiveStructure is called in CStructure destructor
    // SetActiveStructure(NULL);

    EndUpdate();
}

//------------------------------------------------------------------------------

bool CStructureList::LessThanBySeqIndex(CStructure* p_left,CStructure* p_right)
{
    return( p_left->GetSeqIndex() < p_right->GetSeqIndex() );
}

//------------------------------------------------------------------------------

void CStructureList::SortStructures(void)
{
    QList<CStructure*> loc_copy;

    // create local copy of list and remove all from children()
    foreach(QObject* p_qobj,children()) {
        CStructure* p_str = static_cast<CStructure*>(p_qobj);
        loc_copy.append(p_str);
        p_str->setParent(NULL);
    }

    // sort residues
    std::sort(loc_copy.begin(),loc_copy.end(),LessThanBySeqIndex);

    // add residues to the list in sorted order
    foreach(CStructure* p_str,loc_copy) {
        p_str->setParent(this);
    }

    EmitOnStructureListChanged();
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

CStructure* CStructureList::GetActiveStructure(void) const
{
    return(ActiveStructure);
}

//------------------------------------------------------------------------------

int CStructureList::GetNumberOfStructures(void) const
{
    return(children().size());
}

//------------------------------------------------------------------------------

int CStructureList::GetNumberOfSelectedStructures(void) const
{
     int count = 0;
    foreach(QObject* p_qobj,children()) {
        CStructure* p_str = static_cast<CStructure*>(p_qobj);
        if( p_str->IsFlagSet(EPOF_SELECTED) ){
            count++;
        }
    }
    return(count);
}

//------------------------------------------------------------------------------

int CStructureList::GetNumberOfEmptyStructures(void) const
{
    int count = 0;
    foreach(QObject* p_qobj,children()) {
        CStructure* p_str = static_cast<CStructure*>(p_qobj);
        if( p_str->GetAtoms()->GetNumberOfAtoms() == 0 ){
            count++;
        }
    }
    return(count);
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

void CStructureList::LoadData(CXMLElement* p_ele,CHistoryNode* p_history)
{
    // check input data -----------------------------
    if( p_ele == NULL ) {
        INVALID_ARGUMENT("p_ele is NULL");
    }

    if( p_ele->GetName() != "structures" ) {
        LOGIC_ERROR("p_ele is not structures");
    }

    // load object info -----------------------------
    CProObject::LoadData(p_ele);

    // load data ------------------------------------
    CXMLElement* p_ael;
    p_ael = p_ele->GetFirstChildElement("structure");
    while( p_ael != NULL ) {
        CreateStructure(p_ael,p_history);
        p_ael = p_ael->GetNextSiblingElement("structure");
    }
}

//------------------------------------------------------------------------------

void CStructureList::LoadData(CXMLElement* p_ele)
{
    LoadData(p_ele,NULL);
}

//------------------------------------------------------------------------------

void CStructureList::SaveData(CXMLElement* p_ele)
{
    // check input data -----------------------------
    if( p_ele == NULL ) {
        INVALID_ARGUMENT("p_ele is NULL");
    }

    if( p_ele->GetName() != "structures" ) {
        LOGIC_ERROR("p_ele is not structures");
    }

    // save object info -----------------------------
    CProObject::SaveData(p_ele);

    // save data ------------------------------------
    foreach(QObject* p_qobj,children()) {
        CStructure* p_str = static_cast<CStructure*>(p_qobj);
        CXMLElement* p_sel = p_ele->CreateChildElement("structure");
        p_str->SaveData(p_sel);
    }
}

//------------------------------------------------------------------------------

void CStructureList::SaveSelectedResidues(CXMLElement* p_ele)
{
    // check input data -----------------------------
    if( p_ele == NULL ) {
        INVALID_ARGUMENT("p_ele is NULL");
    }

    if( p_ele->GetName() != "structure" ) {
        LOGIC_ERROR("p_ele is not structure");
    }

    // create required sections
    // DO NOT CHANGE ORDER of sections
    p_ele->CreateChildElement("residues");
    p_ele->CreateChildElement("atoms");
    p_ele->CreateChildElement("bonds");

    // save data ------------------------------------
    foreach(QObject* p_qobj,children()) {
        CStructure* p_str = static_cast<CStructure*>(p_qobj);
        p_str->SaveSelectedResidues(p_ele);
    }
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

void CStructureList::BeginGeometryUpdate(void)
{
    GeometryUpdateLevel++;
    if( GeometryUpdateLevel == 1 ){
        foreach(QObject* p_qobj,children()) {
            CStructure* p_str = static_cast<CStructure*>(p_qobj);
            p_str->BeginGeometryUpdate();
        }
    }
}

//------------------------------------------------------------------------------

void CStructureList::EndGeometryUpdate(bool no_event)
{
    if( GeometryUpdateLevel == 0 ) return;
    if( GeometryUpdateLevel == 1 ){
        foreach(QObject* p_qobj,children()) {
            CStructure* p_str = static_cast<CStructure*>(p_qobj);
            p_str->EndGeometryUpdate();
        }
    }
    GeometryUpdateLevel--;
    if( GeometryUpdateLevel == 0 ){
        if( ! no_event ) NotifyGeometryChangeTick();
    }
}

//------------------------------------------------------------------------------

void CStructureList::NotifyGeometryChangeTick(void)
{
    if( GeometryUpdateLevel > 0 ){
        return;
    }
    emit OnGeometryChangeTick();
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

void CStructureList::BeginUpdate()
{
    if( UpdateLevel == 0 ){
        blockSignals(true);
    }
    UpdateLevel++;
}

//------------------------------------------------------------------------------

void CStructureList::EndUpdate(bool do_not_sort)
{
    if( UpdateLevel == 0 ) return;
    UpdateLevel--;
    if( UpdateLevel == 0 ){
        if( ForceSorting ) {
            if( ! do_not_sort ) SortStructures();
            ForceSorting = false;
        }
        blockSignals(false);
        if( Changed ){
            emit OnStructureListChanged();
            Changed = false;
        }
    }
}

//------------------------------------------------------------------------------

void CStructureList::EmitOnStructureListChanged(void)
{
    if( UpdateLevel > 0 ) Changed = true;
    emit OnStructureListChanged();
}

//------------------------------------------------------------------------------

void CStructureList::ListSizeChanged(bool do_not_sort)
{
    ForceSorting = ! do_not_sort;
    if( UpdateLevel > 0 ){
        Changed = true;
        return;
    }
    if( ! do_not_sort ) SortStructures();
    EmitOnStructureListChanged();
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================
