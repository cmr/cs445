#ifndef _SYMBOL_H
#define _SYMBOL_H

#include "util.h"
#include "parser.tab.h"
#include "lexer.h"
#include "ast.h"
#include "translate.h"

#define RESOLVE_FAILURE (-1)

struct stab {
    struct ptrvec *vars, *funcs, *types;
    // maps from "loc id" to "scope". generated by incrementing 1 for each
    // ast node visited, and kept globally. so when codegen goes to walk the
    // ast later, it can find the scopes necessary. will be sparse.
    struct hash_table *scopes;
    // a stack of scopes, for use during resolution but freed immediately
    // afterwards (though each individual scope is held onto)
    struct list *chain;
};

// all map from name -> table index
struct stab_scope {
    // variables in scope.
    struct hash_table *vars;
    // functions in scope. uses stab_var, but
    struct hash_table *funcs;
    // types in scope.
    struct hash_table *types;
};

struct stab_var {
    size_t type;
    char *name;
    YYLTYPE *defn; // todo: annotate all AST nodes with a span...
    bool address_taken; // whether this needs to be a cell or can be a register
    bool captured; // whether this variable needs to be lifted to a closure environment
};

struct stab_resolved_type {
    union {
        struct {
            int lower, upper;
            size_t elt_type;
        } array;
        struct {
            enum subprogs type; // func or proc?
            struct list *args; // arg types
            size_t retty; // ret ty
        } func;
        size_t pointer;
        struct {
            struct rec_layout *layout;
            struct list *fields;
        } record;
    };
    enum types tag;
};

struct stab_record_field {
    char *name;
    size_t type;
};

struct stab_type {
    struct stab_resolved_type ty;
    char *name;
    YYLTYPE *defn; // todo
    uint64_t size, align;
};

void stab_enter(struct stab *, intptr_t);
void stab_leave(struct stab *);

struct stab *stab_new();
void stab_free(struct stab *);

void stab_add_decls(struct stab *, struct ast_decls *);
void stab_add_func(struct stab *, char *, struct ast_type *);
void stab_add_type(struct stab *, char *, struct ast_type *);

bool stab_has_local_var(struct stab *, char *);
bool stab_has_local_func(struct stab *, char *);
bool stab_has_local_type(struct stab *, char *);

size_t stab_resolve_var(struct stab *, char *);
size_t stab_resolve_func(struct stab *, char *);
size_t stab_resolve_type(struct stab *, char *, struct ast_type *);
size_t stab_resolve_type_name(struct stab *, char *);

#endif
