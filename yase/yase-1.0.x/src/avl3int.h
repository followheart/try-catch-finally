/***
*    YASE (Yet Another Search Engine) 
*    Copyright (C) 1995-2002  Dibyendu Majumdar.
*
*    This program is free software; you can redistribute it and/or modify
*    it under the terms of the GNU General Public License as published by
*    the Free Software Foundation; either version 2 of the License, or
*    (at your option) any later version.
*
*    This program is distributed in the hope that it will be useful,
*    but WITHOUT ANY WARRANTY; without even the implied warranty of
*    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*    GNU General Public License for more details.
*
*    You should have received a copy of the GNU General Public License
*    along with this program; if not, write to the Free Software
*    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*
*    Author : Dibyendu Majumdar
*    Email  : dibyendu@mazumdar.demon.co.uk
*    Website: www.mazumdar.demon.co.uk/yase_index.html
*/
#ifndef avl3int_h
#define avl3int_h

typedef enum BalanceType {
	Equal, LeftHigh, RightHigh
} BalanceType;

struct AVLNode_st {
	struct AVLNode_st        *parent, *left, *right;
	BalanceType     balance;
};
typedef struct AVLNode_st AVLNode;

#define CONSTRUCT_NODE(Obj,Key) \
	if (vtbl->createobject != NULL) { \
		(*(vtbl->createobject))((void *)(Obj+1),Key) ; \
	} else 

#define DESTRUCT_NODE(Node) \
	if (vtbl->destroyobject != NULL) { \
		(*(vtbl->destroyobject))((void *)(Node+1)) ; \
	} else

#define COPY_NODE(Node1, Node2, n) \
	if (vtbl->assignobject != NULL) { \
		(*(vtbl->assignobject))((void *)(Node1+1),(void *)(Node2+1)) ; \
	} else { \
		DESTRUCT_NODE(Node1) ; \
		memcpy((void *)(Node1+1),(void *)(Node2+1),n) ; \
	} 

#define COMP(Key, Obj)      (*(vtbl->comparekeys))(Key, (Obj+1))

AVLNode        *
AVL_new(AVLTree * tree, void * key);
void 
AVL_SetRight(AVLNode * self, AVLNode * r);
void 
AVL_SetLeft(AVLNode * self, AVLNode * l);
AVLNode        *
AVL_RotateLeft(AVLNode * self);
AVLNode        *
AVL_RotateRight(AVLNode * self);
AVLNode        *
AVL_DoubleRotateRight(AVLNode * self);
AVLNode        *
AVL_DoubleRotateLeft(AVLNode * self);
AVLNode        *
AVL_RebalanceHeavierLeft(AVLNode * self, int *height_changed);
AVLNode        *
AVL_RebalanceHeavierRight(AVLNode * self, int *height_changed);
AVLNode        *
AVL_RebalanceShorterLeft(AVLNode * self, int *height_changed);
AVLNode        *
AVL_RebalanceShorterRight(AVLNode * self, int *height_changed);
int 
AVL_Height(AVLNode * self);
void
AVLTree_BackwardApply(AVLNode * root, void (*funcptr)(void *));
void
AVLTree_ForwardApply(AVLNode * root, void (*funcptr)(void *));

#endif
