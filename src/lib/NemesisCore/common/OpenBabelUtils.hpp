#ifndef OpenBabelUtilsH
#define OpenBabelUtilsH
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

#include <NemesisCoreMainHeader.hpp>
#include <openbabel/mol.h>
#include <openbabel/atom.h>
#include <openbabel/bond.h>
#include <QByteArray>
#include <Bond.hpp>

//------------------------------------------------------------------------------

class CStructure;
class CResidue;
class CHistoryNode;

//------------------------------------------------------------------------------

class NEMESIS_CORE_PACKAGE COpenBabelUtils {
public:
// main executive method ------------------------------------------------------
    /// nemesis to openbabel
    static void Nemesis2OpenBabel(CStructure* p_mol,OpenBabel::OBMol& obmol,
                                  bool add_as_conformer=false);

    /// nemesis to openbabel
    static void Nemesis2OpenBabel(CResidue* p_res,OpenBabel::OBMol& obmol,
                                  bool add_as_conformer=false,bool add_connectors=false);

    /// openbabel position to nemesis positions
    static void OpenBabelPos2NemesisPos(OpenBabel::OBMol& obmol,CStructure* p_mol);

    /// openbabel to nemesis
    static void OpenBabel2Nemesis(OpenBabel::OBMol& obmol,CStructure* p_mol,
                                  CHistoryNode* p_history=NULL);

    /// nemesis to different openbabel formats and write to file
    static bool Nemesis2File(CStructure* p_mol, const char* format, std::ofstream& of);

    /// get InChI and InChIKey from CStructure
    static bool GetInChI(CStructure* p_mol, QString& inchi, QString& inchikey);

    /// get SMILES from CStructure
    static bool GetSMILES(CStructure* p_mol, QString& smiles);

    /// convert SMILES to CStructure
    static bool FromSMILES(const QString& smiles,CStructure* p_mol,CHistoryNode* p_history=NULL);

    /// convert InChI to CStructure
    static bool FromInChI(const QString& inchi,CStructure* p_mol,CHistoryNode* p_history=NULL);

    /// convert bond order
    static EBondOrder OBToNemesisBondOrder(int order);

    /// convert bond order
    static int NemesisToOBBondOrder(EBondOrder order);
};

//------------------------------------------------------------------------------

#endif
