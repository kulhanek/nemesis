#ifndef AmberMaskCoreH
#define AmberMaskCoreH
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

/* object types ------------------------------------------------------------- */

typedef struct {
    int         Position;
} SGrammar;

typedef struct {
    int         Position;
} SDistRight;

typedef struct {
    int         Position;
    int         Number;
} SInteger;

typedef struct {
    int         Position;
    double      Number;
} SReal;

typedef struct {
    int         Position;
    char        String[4];
} SString;

/* parser types ------------------------------------------------------------- */

enum SType {
    T_RSELECTOR,
    T_ASELECTOR,
    T_TSELECTOR
};

enum SOperator {
    O_NONE,
    O_NOT,
    O_AND,
    O_OR,
    O_RLT,
    O_RGT,
    O_ALT,
    O_AGT
};

enum DModificator {
    D_ORIGIN,
    D_CBOX,
    D_LIST,
    D_COM,
    D_PLANE
};

struct SListItem {
    int                 Index;
    int                 Length;
    char                Name[4];
    struct SListItem*   NextItem;
};

struct SList {
    struct SListItem*   FirstItem;
    struct SListItem*   LastItem;
};

struct SSelection {
    enum SType          Type;
    struct SList*       Items;
};

struct SExpression {
    enum SOperator          Operator;
    double                  Distance;
    enum DModificator       Modificator;
    struct SSelection*      Selection;
    struct SExpression*     LeftExpression;
    struct SExpression*     RightExpression;
};

/* global variables --------------------------------------------------------- */
extern int                 LexPosition;        /* position in a mask during lexical analysis */
extern struct SExpression* TopExpression;      /* top expression to be evaluated */

/* global functions --------------------------------------------------------- */
#ifdef __cplusplus
extern "C" {
#endif

    /* init mask parser for new job */
    extern int init_asl_mask(void);

    /* parse mask */
    extern int parse_asl_mask(const char* p_mask);

    /* free data associated with parsed mask */
    extern int free_asl_mask_tree(void);

    /* get top node of mask expression */
    extern struct SExpression* get_asl_expression_tree(void);

    /* print mask expression */
    extern int print_asl_expression_tree(struct SExpression* p_expr);

    /* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

    extern int yyerror(const char* p_error);
    extern int pperror(const char* p_error,int position);


    /* helper functions */
    extern struct SListItem*   AllocateListItem(void);
    extern struct SList*       AllocateList(void);
    extern struct SSelection*  AllocateSelection(enum SType type,struct SList* p_list);
    extern struct SExpression* AllocateExpression(void);

    /* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

#ifdef __cplusplus
}
#endif

#endif
