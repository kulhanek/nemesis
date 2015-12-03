/* =============================================================================
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
// =========================================================================== */

#include "ASLParser.hpp"
#include <stdlib.h>
#include <stdio.h>
#include <vector>
#include <ErrorSystem.hpp>
#include <string.h>

using namespace std;

//------------------------------------------------------------------------------

int                 LexPosition     = 0;
struct SExpression* TopExpression   = NULL;

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

// track info about allocation
// this class (instead of individual global objects) is used to prevent
// double free corruption problem caused by
// __cxa_finalize and C exit

class CASLMaskStack{
public:
    ~CASLMaskStack(void);
    void Clear(void);

public:
    vector<SListItem*>      ListItemAllocations;
    vector<SList*>          ListAllocations;
    vector<SSelection*>     SelectionAllocations;
    vector<SExpression*>    ExpressionAllocations;
};

CASLMaskStack ASLMaskStack;

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

CASLMaskStack::~CASLMaskStack(void)
{
    Clear();
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

void CASLMaskStack::Clear(void)
{
    for(unsigned int i=0; i < ListItemAllocations.size(); i++) {
        delete ListItemAllocations[i];
    }
    ListItemAllocations.clear();

    for(unsigned int i=0; i < ListAllocations.size(); i++) {
        delete ListAllocations[i];
    }
    ListAllocations.clear();

    for(unsigned int i=0; i < SelectionAllocations.size(); i++) {
        delete SelectionAllocations[i];
    }
    SelectionAllocations.clear();

    for(unsigned int i=0; i < ExpressionAllocations.size(); i++) {
        delete ExpressionAllocations[i];
    }
    ExpressionAllocations.clear();
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

bool print_asl_expression(FILE* p_fout,struct SExpression* p_expr);
bool print_asl_selection(FILE* p_fout,struct SSelection* p_sel);
bool print_asl_list(FILE* p_fout,struct SList* p_list);

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

// int main(void)
// {
//  init_mask();
//  parse_mask(":1-2@4");
//  print_asl_expression_tree(get_expression_tree());
//  free_mask_tree();
// }

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

int init_asl_mask(void)
{
    free_asl_mask_tree();
    LexPosition = 1;
    TopExpression = NULL;
    return(0);
}

//------------------------------------------------------------------------------

struct SExpression* get_asl_expression_tree(void) {
    return(TopExpression);
}

//------------------------------------------------------------------------------

int free_asl_mask_tree(void)
{
    ASLMaskStack.Clear();

    LexPosition = 0;
    TopExpression = NULL;

    return(0);
}

//------------------------------------------------------------------------------

int print_asl_expression_tree(struct SExpression* p_expr)
{
    if( print_asl_expression(stdout,p_expr) != true ) return(-1);
    return(0);
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

bool print_asl_expression(FILE* p_fout,struct SExpression* p_expr)
{
    if( p_expr == NULL ) {
        fprintf(p_fout,"<- NULL expr");
        return(false);
    }

    fprintf(p_fout,"EXP(");
    if( p_expr->Selection != NULL ) {
        if( print_asl_selection(p_fout,p_expr->Selection) == false ) return(false);
    } else {
        switch(p_expr->Operator) {
        case O_AND:
            if( print_asl_expression(p_fout,p_expr->LeftExpression) == false ) return(false);
            fprintf(p_fout," AND ");
            if( print_asl_expression(p_fout,p_expr->RightExpression) == false ) return(false);
            break;
        case O_OR:
            if( print_asl_expression(p_fout,p_expr->LeftExpression) == false ) return(false);
            fprintf(p_fout," OR ");
            if( print_asl_expression(p_fout,p_expr->RightExpression) == false ) return(false);
            break;
        case O_NOT:
            fprintf(p_fout,"NOT ");
            if( print_asl_expression(p_fout,p_expr->RightExpression) == false ) return(false);
            break;
        default:
            fprintf(p_fout,"<- unknown operator");
            return(false);
        };
    }
    fprintf(p_fout,")");
    return(true);
}

//------------------------------------------------------------------------------

bool print_asl_selection(FILE* p_fout,struct SSelection* p_sel)
{
    if( p_sel == NULL ) {
        fprintf(p_fout,"<- NULL selection");
        return(false);
    }

    fprintf(p_fout,"SEL");

    char selector;
    switch(p_sel->Type) {
    case T_RSELECTOR:
        selector = 'R';
        break;
    case T_ASELECTOR:
        selector = 'A';
        break;
    case T_TSELECTOR:
        selector = 'T';
        break;
    default:
        fprintf(p_fout,"<- unknown selector");
        return(false);
    };

    fprintf(p_fout,"[%c](",selector);
    if( print_asl_list(p_fout,p_sel->Items) == false ) return(false);
    fprintf(p_fout,")");

    return(true);
}

//------------------------------------------------------------------------------

bool print_asl_list(FILE* p_fout,struct SList* p_list)
{
    if( p_list == NULL ) {
        fprintf(p_fout,"<- NULL list");
        return(false);
    }

    struct SListItem* p_item = p_list->FirstItem;

    while( p_item != NULL ) {
        if( p_item->Index < 0 ) {
            fprintf(p_fout,"*");
        }
        if( p_item->Index > 0 ) {
            if( p_item->Length > 1 ) {
                fprintf(p_fout,"%d-%d",p_item->Index,p_item->Index+p_item->Length);
            } else {
                if( p_item->Length == 1 ) {
                    fprintf(p_fout,"%d",p_item->Index);
                } else {
                    fprintf(p_fout,"<- illegal range");
                    return(false);
                }
            }
        }

        if( p_item->Index == 0 ) {
            fprintf(p_fout,"%-4s",p_item->Name);
        }

        p_item = p_item->NextItem;
        if( p_item != NULL ) fprintf(p_fout,",");
    }

    return(true);
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

int yyerror(const char* p_error)
{
    ES_ERROR(p_error);
    return(0);
}

//------------------------------------------------------------------------------

int pperror(const char* p_error,int position)
{
    CSmallString error;
    error << position << ": " << p_error;
    ES_ERROR(error);
    return(0);
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

struct SListItem* AllocateListItem(void) {
    struct SListItem* p_item = new struct SListItem;

    p_item->Index = 0;
    p_item->Length = 0;
    memset(p_item->Name,0,4);
    p_item->NextItem = NULL;

    ASLMaskStack.ListItemAllocations.push_back(p_item);
    return(p_item);
}

//------------------------------------------------------------------------------

struct SList* AllocateList(void) {
    struct SList* p_list = new struct SList;

    p_list->FirstItem = NULL;
    p_list->LastItem = NULL;

    ASLMaskStack.ListAllocations.push_back(p_list);
    return(p_list);
}

//------------------------------------------------------------------------------

struct SSelection* AllocateSelection(enum SType type,struct SList* p_list) {
    struct SSelection* p_sel = new struct SSelection;

    p_sel->Type = type;
    p_sel->Items = p_list;

    ASLMaskStack.SelectionAllocations.push_back(p_sel);
    return(p_sel);
}

//------------------------------------------------------------------------------

struct SExpression* AllocateExpression(void) {
    struct SExpression* p_expr = new struct SExpression;

    p_expr->Operator = O_NONE;
    p_expr->Distance = 0.0;
    p_expr->Selection = NULL;
    p_expr->LeftExpression = NULL;
    p_expr->RightExpression = NULL;

    ASLMaskStack.ExpressionAllocations.push_back(p_expr);
    return(p_expr);
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================
