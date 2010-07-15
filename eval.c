#include "scheme.h"

struct evaluator    *scm_create_evaluator(void) {
    struct evaluator *scm = malloc(sizeof(*scm)) ;
    ASSERT(scm) ;
    scm->s = NIL ;
    scm->e = env_create(NIL) ;
    scm->c = NIL ;
    scm->d = NIL ;
    define_toplevels(scm->e) ;
    return scm ;
}

void                scm_push(struct evaluator *scm,
        scm_val e, scm_val c, scm_val s) {
    scm->d = cons(cons(scm->s, cons(scm->e, scm->c)), scm->d) ;
    scm->c = c ;
    scm->e = e ;
    scm->s = s ;
}

#define PREV_S(scm) CAAR(scm->d)
#define PREV_E(scm) CADAR(scm->d)
#define PREV_C(scm) CDDAR(scm->d)
#define PREV_D(scm) CDR(scm->d)

#define S_APPLY     MKTAG(13, SPECIAL)
#define S_EVAL      MKTAG(14, SPECIAL)

#define PUSH(x)     scm->s = cons(x, scm->s)

void                scm_pop(struct evaluator *scm) {
    scm->s = cons(CAR(scm->s), PREV_S(scm)) ;
    scm->e = PREV_E(scm) ;
    scm->c = PREV_C(scm) ;
    scm->d = PREV_D(scm) ;
}

void        scm_invoke(struct evaluator *scm, scm_val c) {
    scm_val code = CAR(c), args = CDR(c) ;
    if (type_of(code) == SYMBOL) {
        code = env_get(scm->e, code) ;
        if (SYNTAX_P(code)) {
            scm_val apply = cons(S_APPLY, NIL) ;
            if (code.c->flags & FL_EVAL) apply = cons(S_EVAL, apply) ;
            scm_push(scm, scm->e, apply, cons(code, args)) ;
            return ;
        }
    }
    scm_push(scm, scm->e, reverse_append(c, cons(S_APPLY, NIL)), NIL) ;
}

scm_val         scm_apply(struct evaluator *scm) {
    scm_val proc = CAR(scm->s), args = CDR(scm->s) ;

    ASSERT(type_of(proc) == PROCEDURE) ;
    if (proc.c->flags & FL_BUILTIN) {
        scm_val (*f)() = CAR(proc).p ;
        return f(args, scm->e, CDR(proc)) ;
    } else {
        scm_push(scm,
                env_bind_formals(scm->e, CAAR(proc), args), CDAR(proc), NIL) ;
        return S_EVAL ;
    }
}

scm_val             scm_eval(struct evaluator *scm, scm_val code) {
    scm->s = NIL ;
    scm->c = cons(code, scm->c) ;

    while (PAIR_P(scm->c) || PAIR_P(scm->d)) {
        scm_val c ;

        if (NULL_P(scm->c)) {
            scm_pop(scm) ;
            continue ;
        }

        c = CAR(scm->c) ;
        scm->c = CDR(scm->c) ;

        switch (type_of(c)) {
            case FIXNUM: case CHAR: case BOOL: case FLOAT: case STRING:
                break ;
            case SYMBOL: c = env_get(scm->e, c) ; break ;
            case CONS:
                scm_invoke(scm, c) ;
                continue ;

            case SPECIAL:
                if (EQ_P(c, S_APPLY)) {
                    c = scm_apply(scm) ;
                    if (EQ_P(c, S_EVAL)) continue ;
                } else if (EQ_P(c, S_EVAL)) {
                    die("evaled syntax NIY\n") ;
                } else
                    die("unknown special %d\n", UNTAG(c)) ;
                break ;
            default:
                c = cons(intern("error"), c) ;
                break ;
        }
        PUSH(c) ;
    }

    return CAR(scm->s) ;
}