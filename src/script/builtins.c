#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <string.h>

#include "builtins.h"

#define FLOAT_EQUALS_MARGIN 1e-6

static bool equal_atoms(struct Atom *atom1, struct Atom *atom2)
{
    assert(atom1);
    assert(atom2);

    if (atom1->type != atom2->type) {
        return false;
    }

    switch (atom1->type) {
    case ATOM_SYMBOL:
        return strcmp(atom1->sym, atom2->sym) == 0;

    case ATOM_NUMBER:
        return fabsf(atom1->num - atom2->num) <= FLOAT_EQUALS_MARGIN;

    case ATOM_STRING:
        return strcmp(atom1->str, atom2->str) == 0;
    }

    return false;
}

static bool equal_cons(struct Cons *cons1, struct Cons *cons2)
{
    assert(cons1);
    assert(cons2);
    return equal(cons1->car, cons2->car) && equal(cons1->cdr, cons2->cdr);
}

bool equal(struct Expr obj1, struct Expr obj2)
{
    if (obj1.type != obj2.type) {
        return false;
    }

    switch (obj1.type) {
    case EXPR_ATOM:
        return equal_atoms(obj1.atom, obj2.atom);

    case EXPR_CONS:
        return equal_cons(obj1.cons, obj2.cons);

    case EXPR_VOID:
        return true;
    }

    return true;
}

bool nil_p(struct Expr obj)
{
    return symbol_p(obj)
        && strcmp(obj.atom->sym, "nil") == 0;
}

bool symbol_p(struct Expr obj)
{
    return obj.type == EXPR_ATOM
        && obj.atom->type == ATOM_SYMBOL;
}

bool cons_p(struct Expr obj)
{
    return obj.type == EXPR_CONS;
}

struct Expr assoc(struct Expr key, struct Expr alist)
{
    while (cons_p(alist)) {
        if (cons_p(alist.cons->car) && equal(alist.cons->car.cons->car, key)) {
            return alist.cons->car;
        }

        alist = alist.cons->cdr;
    }

    return alist;
}