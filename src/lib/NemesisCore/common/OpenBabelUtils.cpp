// =============================================================================
// NEMESIS - Molecular Modelling Package
// -----------------------------------------------------------------------------
//    Copyright (C) 2010 Petr Kulhanek, kulhanek@chemi.muni.cz
//    Copyright (C) 2009 Petr Kulhanek, kulhanek@chemi.muni.cz,
//                       Jakub Stepan, xstepan3@chemi.muni.cz
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

#include <OpenBabelUtils.hpp>
#include <ErrorSystem.hpp>
#include <Structure.hpp>
#include <Atom.hpp>
#include <AtomList.hpp>
#include <BondList.hpp>
#include <iostream>
#include <fstream>
#include <sstream>
#include <ResidueList.hpp>
#include <Residue.hpp>
#include <PeriodicTable.hpp>

#include <openbabel/mol.h>
#include <openbabel/obconversion.h>
#include <openbabel/builder.h>
#include <openbabel/atom.h>
#include <openbabel/bond.h>


using namespace OpenBabel;
using namespace std;

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

void COpenBabelUtils::Nemesis2OpenBabel(CStructure* p_mol,OpenBabel::OBMol& obmol,bool add_as_conformer)
{
    if( p_mol == NULL ) {
        INVALID_ARGUMENT("p_mol is NULL");
    }

    // general
    obmol.SetTitle(p_mol->GetName().toLatin1().constData());

    std::map<int,int>   id_map;
    int                 conf_size = p_mol->GetAtoms()->GetNumberOfAtoms();

    double* p_conf = NULL;
    if( (conf_size > 0) && add_as_conformer ) {
        p_conf = new double[3*conf_size];
    }

    // add atoms
    int i=0;
    int at_lid = 1;

    foreach(QObject* p_qobj,p_mol->GetAtoms()->children()) {
        CAtom*  p_atom = static_cast<CAtom*>(p_qobj);
        OBAtom* p_ob_atom = obmol.NewAtom();
        p_ob_atom->SetAtomicNum(p_atom->GetZ());
        p_ob_atom->SetVector(p_atom->GetPos().x, p_atom->GetPos().y, p_atom->GetPos().z);
        id_map[p_atom->GetIndex()] = at_lid;

        if( add_as_conformer ) {
            p_conf[i++] = p_atom->GetPos().x;
            p_conf[i++] = p_atom->GetPos().y;
            p_conf[i++] = p_atom->GetPos().z;
        }
        at_lid++;
    }

    if( p_conf != NULL ) {
        obmol.AddConformer(p_conf);
    }

    // add bonds
    foreach(QObject* p_qobj,p_mol->GetBonds()->children()) {
        CBond*  p_bond = static_cast<CBond*>(p_qobj);
        if( p_bond->IsInvalidBond() ) continue;
        int order = COpenBabelUtils::NemesisToOBBondOrder(p_bond->GetBondOrder());

        // get begin and end atom indexes
        int a1_id = p_bond->GetFirstAtom()->GetIndex();
        int a2_id = p_bond->GetSecondAtom()->GetIndex();

        unsigned int ob_a1_id = id_map[a1_id];
        unsigned int ob_a2_id = id_map[a2_id];

        // create bond
        obmol.AddBond(ob_a1_id, ob_a2_id, order);
    }

    // set charge
    obmol.SetTotalCharge(p_mol->GetTotalCharge());
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

void COpenBabelUtils::Nemesis2OpenBabel(CResidue* p_res,OpenBabel::OBMol& obmol,
                                        bool add_as_conformer,bool add_connectors)
{
    if( p_res == NULL ) {
        INVALID_ARGUMENT("p_res is NULL");
    }

    std::map<int,int>   id_map;
    double*             p_conf = NULL;
    int                 conf_size = p_res->GetNumberOfAtoms();

    if( add_connectors ){
        conf_size += p_res->GetConnectorAtoms().count();
    }

    if( ( conf_size > 0) && add_as_conformer ) {
        p_conf = new double[3*conf_size];
    }

    // add atoms
    int i=0;
    int at_lid = 1;
    foreach(CAtom* p_atom,p_res->GetAtoms()) {
        OBAtom* p_ob_atom = obmol.NewAtom();
        p_ob_atom->SetAtomicNum(p_atom->GetZ());
        p_ob_atom->SetVector(p_atom->GetPos().x, p_atom->GetPos().y, p_atom->GetPos().z);
        id_map[p_atom->GetIndex()] = at_lid;
        if( add_as_conformer ) {
            p_conf[i++] = p_atom->GetPos().x;
            p_conf[i++] = p_atom->GetPos().y;
            p_conf[i++] = p_atom->GetPos().z;
        }
        at_lid++;
    }
    // add connectors
    if( add_connectors ){
        foreach(CAtom* p_atom,p_res->GetConnectorAtoms(true)) {
            OBAtom* p_ob_atom = obmol.NewAtom();
            p_ob_atom->SetAtomicNum(1);
            p_ob_atom->SetVector(p_atom->GetPos().x, p_atom->GetPos().y, p_atom->GetPos().z);
            id_map[p_atom->GetIndex()] = at_lid;
            if( add_as_conformer ) {
                p_conf[i++] = p_atom->GetPos().x;
                p_conf[i++] = p_atom->GetPos().y;
                p_conf[i++] = p_atom->GetPos().z;
            }
            at_lid++;
        }
    }

    if( p_conf != NULL ) {
        obmol.AddConformer(p_conf);
    }

    // add bonds
    foreach(CBond* p_bond,p_res->GetBonds(add_connectors)) {
        if( p_bond->IsInvalidBond() ) continue;
        int order = COpenBabelUtils::NemesisToOBBondOrder(p_bond->GetBondOrder());

        // get begin and end atom indexes
        int a1_id = p_bond->GetFirstAtom()->GetIndex();
        int a2_id = p_bond->GetSecondAtom()->GetIndex();

        unsigned int ob_a1_id = id_map[a1_id];
        unsigned int ob_a2_id = id_map[a2_id];

        // create bond
        obmol.AddBond(ob_a1_id, ob_a2_id, order);
    }
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

void COpenBabelUtils::OpenBabelPos2NemesisPos(OpenBabel::OBMol& obmol,CStructure* p_mol)
{
    if( p_mol == NULL ) {
        INVALID_ARGUMENT("p_mol is NULL");
    }

    int i = 1;
    foreach(QObject* p_qobj,p_mol->GetAtoms()->children()) {
        CAtom*  p_atom = static_cast<CAtom*>(p_qobj);
        OBAtom* p_ob_atom = obmol.GetAtom(i);
        p_atom->SetPos(CPoint(p_ob_atom->x(), p_ob_atom->y(), p_ob_atom->z()));
        i++;
    }
}

//------------------------------------------------------------------------------

void COpenBabelUtils::OpenBabel2Nemesis(OpenBabel::OBMol& obmol,CStructure* p_mol,
                                        CHistoryNode* p_history)
{
    if( p_mol == NULL ) {
        INVALID_ARGUMENT("p_mol is NULL");
    }

    CResidue* p_res = NULL;
    int top_res_index = p_mol->GetResidues()->GetTopSeqIndex();
    int top_index = p_mol->GetAtoms()->GetTopSerIndex();

    OBResidue* p_last_res = NULL;

    // go throught all atoms in babel mol
    int loc_index = 1;
    for( unsigned int i = 1; i <= obmol.NumAtoms(); i++) {
        OBAtom* p_obAtom = obmol.GetAtom(i);

        // assign residue
        if( (p_obAtom->GetResidue() != p_last_res)
            && (p_obAtom->GetResidue() != NULL) ){
            p_last_res = p_obAtom->GetResidue();
            p_res = p_mol->GetResidues()->CreateResidue(QString(p_last_res->GetName().c_str()),
                                                        QString(p_last_res->GetChain()),
                                                        top_res_index+p_last_res->GetNum(),
                                                        p_history);
            loc_index = 1;
        }

        CAtomData data;

        QString name = "%1%2";
        name = name.arg(PeriodicTable.GetSymbol(p_obAtom->GetAtomicNum()));
        name = name.arg(loc_index);
        data.Name = name;

        data.Z = p_obAtom->GetAtomicNum();
        data.Pos = CPoint(p_obAtom->x(), p_obAtom->y(), p_obAtom->z());
        data.Charge = p_obAtom->GetPartialCharge();
        data.SerIndex = top_index+i;
        loc_index++;

        CAtom*  p_nemAtom = p_mol->GetAtoms()->CreateAtom(data,p_history);

        if( p_res != NULL ){
            p_res->AddAtom(p_nemAtom,p_history);
        }
    }

    // bonds ...
    for( unsigned int i = 0; i < obmol.NumBonds(); i++) {
        OBBond* obBond = obmol.GetBond(i);
        // set new nemesis bond
        EBondOrder order = COpenBabelUtils::OBToNemesisBondOrder(obBond->GetBondOrder());
        p_mol->GetBonds()->CreateBond(
            p_mol->GetAtoms()->SearchBySerIndex(top_index + obBond->GetBeginAtomIdx()),
            p_mol->GetAtoms()->SearchBySerIndex(top_index + obBond->GetEndAtomIdx()),
            order,p_history);
    }
}

//------------------------------------------------------------------------------

EBondOrder COpenBabelUtils::OBToNemesisBondOrder(int order)
{
    // Bond order (1, 2, 3, 5=aromatic).
    switch(order) {
    case 1:
        return(BO_SINGLE);
        break;
    case 2:
        return(BO_DOUBLE);
        break;
    case 3:
        return(BO_TRIPLE);
        break;
    case 5:
        return(BO_SINGLE_H);
    default:
        return(BO_SINGLE);
    }
}

//------------------------------------------------------------------------------

int COpenBabelUtils::NemesisToOBBondOrder(EBondOrder order)
{
    switch(order) {
        // open babel : 1 = single, 2 = double, 5 = aromatic
        // nemesis : BO_NONE = -1,BO_WEAK =0,BO_H=1,BO_SINGLE = 2,BO_SINGLE_H = 3,
        // BO_DOUBLE = 4, BO_DOUBLE_H =5, BO_TRIPLE =6,
    case BO_SINGLE:
        return(1);
    case BO_DOUBLE:
        return(2);
    case BO_TRIPLE:
        return(3);
    case BO_SINGLE_H:
        return(5);
    default:
        return(1);
    }
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

bool COpenBabelUtils::Nemesis2File(CStructure* p_mol, const char* format, ofstream& of)
{
    if( p_mol == NULL ) {
        ES_ERROR("p_mol is NULL");
        return(false);
    }

    OBMol mol;

    // convert data into openbabel
    COpenBabelUtils::Nemesis2OpenBabel(p_mol,mol);

    OpenBabel::OBConversion conv(&cin, &of);//

    // output format setting
    if (!conv.SetOutFormat(format)) {
        ES_ERROR("Cannot set format");
        return(false);
    }

    // write data
    if( ! conv.Write(&mol) ) {
        ES_ERROR("Cannot export to file");
        return(false);
    }

    return(true);
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

bool COpenBabelUtils::GetInChI(CStructure* p_mol, QString& inchi, QString& inchikey)
{
    OBMol mol;

    // get openbabel molecule
    COpenBabelUtils::Nemesis2OpenBabel(p_mol,mol);

    // get inchi and inchikey
    OBConversion   conv;

    OpenBabel::OBFormat* outFormat = conv.FindFormat("inchi");
    if( outFormat == NULL ){
        ES_ERROR("unable to find inchi output format");
        return(false);
    }

    if( ! conv.SetOutFormat(outFormat) ) {
        ES_ERROR("unable to select inchi output format");
        return(false);
    }

    // convert data
    string sinchi,sinchi_key;
    sinchi = conv.WriteString(&mol);

    conv.SetOptions("K",OBConversion::OUTOPTIONS);
    sinchi_key = conv.WriteString(&mol);

    inchi = sinchi.c_str();
    inchikey = sinchi_key.c_str();
    return(true);
}

//------------------------------------------------------------------------------

bool COpenBabelUtils::GetSMILES(CStructure* p_mol, QString& smiles)
{
    OBMol mol;

    // get openbabel molecule
    COpenBabelUtils::Nemesis2OpenBabel(p_mol,mol);

    // get inchi and inchikey
    OBConversion   conv;

    OpenBabel::OBFormat* outFormat = conv.FindFormat("smi");
    if( outFormat == NULL ){
        ES_ERROR("unable to find smi output format");
        return(false);
    }

    if( ! conv.SetOutFormat(outFormat) ) {
        ES_ERROR("unable to select smi output format");
        return(false);
    }

    // convert data
    string s_smiles;
    s_smiles = conv.WriteString(&mol);
    smiles = s_smiles.c_str();

    return(true);
}

//------------------------------------------------------------------------------

bool COpenBabelUtils::FromSMILES(const QString& smiles,CStructure* p_mol,CHistoryNode* p_history)
{
    OBMol           mol;
    OBConversion    conv;

    // populate obmol with 2D representation -------------------------
    OpenBabel::OBFormat* inFormat = conv.FindFormat("smi");
    if( inFormat == NULL ){
        ES_ERROR("unable to find smi input format");
        return(false);
    }

    if( ! conv.SetInFormat(inFormat) ) {
        ES_ERROR("unable to select smi input format");
        return(false);
    }

    // convert data
    conv.ReadString(&mol,smiles.toStdString());

    mol.AddHydrogens(false);

    // build 3D model ------------------------------------------------
    OpenBabel::OBBuilder builder;
    builder.Build(mol);

    // convert to nemesis
    OpenBabel2Nemesis(mol,p_mol,p_history);

    return(true);
}

//------------------------------------------------------------------------------

bool COpenBabelUtils::FromInChI(const QString& inchi,CStructure* p_mol,CHistoryNode* p_history)
{
    OBMol           mol;
    OBConversion    conv;

    // populate obmol with 2D representation -------------------------
    OpenBabel::OBFormat* inFormat = conv.FindFormat("inchi");
    if( inFormat == NULL ){
        ES_ERROR("unable to find inchi input format");
        return(false);
    }

    if( ! conv.SetInFormat(inFormat) ) {
        ES_ERROR("unable to select inchi input format");
        return(false);
    }

    // convert data
    conv.ReadString(&mol,inchi.toStdString());

    mol.AddHydrogens(false);

    // build 3D model ------------------------------------------------
    OpenBabel::OBBuilder builder;
    builder.Build(mol);

    // convert to nemesis
    OpenBabel2Nemesis(mol,p_mol,p_history);


    return(true);
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================
