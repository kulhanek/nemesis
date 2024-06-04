// =============================================================================
// NEMESIS - Molecular Modelling Package
// -----------------------------------------------------------------------------
//    Copyright (C) 2011 Petr Kulhanek, kulhanek@chemi.muni.cz
//                  This code is based on ASL mask implementation in
//                  Amber Support Library
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

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <ASLMask.hpp>
#include <ErrorSystem.hpp>
#include <ASLSelection.hpp>
#include "maskparser/ASLParser.hpp"
#include <Structure.hpp>
#include <AtomList.hpp>
#include <Atom.hpp>
#include <ResidueList.hpp>
#include <Bond.hpp>
#include <Residue.hpp>
#include <QSet>

#include "maskparser/ASLParser.hpp"

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

CASLMask::CASLMask(CStructure* p_mol)
{
    Structure = p_mol;
    Selection = NULL;
}

//------------------------------------------------------------------------------

CASLMask::~CASLMask(void)
{
    if( Selection != NULL ) delete Selection;
    Selection = NULL;
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

bool CASLMask::SetMask(const QString& mask)
{
    if( Structure == NULL ) {
        LOGIC_ERROR("no molecule is assigned with mask");
    }

    if( mask == NULL ) {
        ES_ERROR("mask is empty");
        return(false);
    }

    // remove previous selection -----------------------------
    Mask = mask;
    if( Selection != NULL ) delete Selection;
    Selection = NULL;

    // init mask parser
    init_asl_mask();

    // parse mask
    if( parse_asl_mask(Mask.toLatin1()) != 0 ) {
        free_asl_mask_tree();
        ES_ERROR("unable to parse mask");
        return(false);
    }

    // get top mask expression
    struct SExpression* p_top_expr = get_asl_expression_tree();
    if( p_top_expr == NULL ) {
        ES_ERROR("top expression is NULL");
        return(false);
    }

    // init selection tree
    Selection = new CASLSelection(this,this);

    if( Selection->ExpandAndReduceTree(p_top_expr) == false ) {
        ES_ERROR("unable to expand and reduce expression tree");
        free_asl_mask_tree();
        return(false);
    }

    // free parser data
    free_asl_mask_tree();

    return(true);
}

//---------------------------------------------------------------------------

const QString& CASLMask::GetMask(void) const
{
    return(Mask);
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

const QList<CAtom*> CASLMask::GetSelectedAtoms(void)
{
    QList<CAtom*> list;
    if( Selection == NULL ) return(list);

    foreach(QObject* p_qobj, GetAtoms()){
        CAtom* p_atom = static_cast<CAtom*>(p_qobj);
        if( Selection->Selection[p_atom] == true ){
            list.append(p_atom);
        }
    }

    return(list);
}

//------------------------------------------------------------------------------

const QList<CBond*> CASLMask::GetSelectedBonds(void)
{
    QList<CBond*> list;
    if( Selection == NULL ) return(list);

    QSet<CBond*>  bm;

    foreach(QObject* p_qobj, GetAtoms()){
        CAtom* p_atom = static_cast<CAtom*>(p_qobj);
        if( Selection->Selection[p_atom] == false ) continue;

        foreach(CBond* p_bond,p_atom->GetBonds()) {
            bm.insert(p_bond);
        }
    }

    return(bm.values());
}

//------------------------------------------------------------------------------

const QList<CResidue*> CASLMask::GetSelectedResidues(void)
{
    QList<CResidue*> list;
    if( Selection == NULL ) return(list);

    QSet<CResidue*>  rset;

    foreach(QObject* p_qobj, GetAtoms()){
        CAtom* p_atom = static_cast<CAtom*>(p_qobj);
        if( Selection->Selection[p_atom] == true ){
            if( p_atom->GetResidue() ) rset.insert(p_atom->GetResidue());
        }
    }

    return(rset.values());
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

const QList<QObject*> CASLMask::GetAtoms(void)
{
    return(Structure->GetAtoms()->children());
}

//------------------------------------------------------------------------------

const QList<QObject*> CASLMask::GetResidues(void)
{
    return(Structure->GetResidues()->children());
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================


