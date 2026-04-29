#ifndef DEFINES_AROS_VOIDCALL_H
#define DEFINES_AROS_VOIDCALL_H

#define AROS_VOID_LC0(n, bt, bn, o, s) \
	(((void (*)(__AROS_LP_BASE(bt, bn)))__AROS_GETVECADDR(bn, o))(__AROS_LC_BASE(bt, bn)))
#define AROS_VOID_LC1(n, a1, bt, bn, o, s) \
	(((void (*)(__AROS_LPA(a1), __AROS_LP_BASE(bt, bn)))__AROS_GETVECADDR(bn, o))(__AROS_LCA(a1), __AROS_LC_BASE(bt, bn)))
#define AROS_VOID_LC2(n, a1, a2, bt, bn, o, s) \
	(((void (*)(__AROS_LPA(a1), __AROS_LPA(a2), __AROS_LP_BASE(bt, bn)))__AROS_GETVECADDR(bn, o))(__AROS_LCA(a1), __AROS_LCA(a2), __AROS_LC_BASE(bt, bn)))
#define AROS_VOID_LC3(n, a1, a2, a3, bt, bn, o, s) \
	(((void (*)(__AROS_LPA(a1), __AROS_LPA(a2), __AROS_LPA(a3), __AROS_LP_BASE(bt, bn)))__AROS_GETVECADDR(bn, o))(__AROS_LCA(a1), __AROS_LCA(a2), __AROS_LCA(a3), __AROS_LC_BASE(bt, bn)))
#define AROS_VOID_LC4(n, a1, a2, a3, a4, bt, bn, o, s) \
	(((void (*)(__AROS_LPA(a1), __AROS_LPA(a2), __AROS_LPA(a3), __AROS_LPA(a4), __AROS_LP_BASE(bt, bn)))__AROS_GETVECADDR(bn, o))(__AROS_LCA(a1), __AROS_LCA(a2), __AROS_LCA(a3), __AROS_LCA(a4), __AROS_LC_BASE(bt, bn)))
#define AROS_VOID_LC5(n, a1, a2, a3, a4, a5, bt, bn, o, s) \
	(((void (*)(__AROS_LPA(a1), __AROS_LPA(a2), __AROS_LPA(a3), __AROS_LPA(a4), __AROS_LPA(a5), __AROS_LP_BASE(bt, bn)))__AROS_GETVECADDR(bn, o))(__AROS_LCA(a1), __AROS_LCA(a2), __AROS_LCA(a3), __AROS_LCA(a4), __AROS_LCA(a5), __AROS_LC_BASE(bt, bn)))
#define AROS_VOID_LC6(n, a1, a2, a3, a4, a5, a6, bt, bn, o, s) \
	(((void (*)(__AROS_LPA(a1), __AROS_LPA(a2), __AROS_LPA(a3), __AROS_LPA(a4), __AROS_LPA(a5), __AROS_LPA(a6), __AROS_LP_BASE(bt, bn)))__AROS_GETVECADDR(bn, o))(__AROS_LCA(a1), __AROS_LCA(a2), __AROS_LCA(a3), __AROS_LCA(a4), __AROS_LCA(a5), __AROS_LCA(a6), __AROS_LC_BASE(bt, bn)))
#define AROS_VOID_LC7(n, a1, a2, a3, a4, a5, a6, a7, bt, bn, o, s) \
	(((void (*)(__AROS_LPA(a1), __AROS_LPA(a2), __AROS_LPA(a3), __AROS_LPA(a4), __AROS_LPA(a5), __AROS_LPA(a6), __AROS_LPA(a7), __AROS_LP_BASE(bt, bn)))__AROS_GETVECADDR(bn, o))(__AROS_LCA(a1), __AROS_LCA(a2), __AROS_LCA(a3), __AROS_LCA(a4), __AROS_LCA(a5), __AROS_LCA(a6), __AROS_LCA(a7), __AROS_LC_BASE(bt, bn)))

#endif /* DEFINES_AROS_VOIDCALL_H */
