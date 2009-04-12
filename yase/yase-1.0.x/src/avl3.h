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
#ifndef avl_h
#define avl_h

/* avl tree manager */
/* dibyendu majumdar */
/* version 3.0 */
/* date started oct 21 1995 */
/* revised oct 24 1995 */
/* revised jan 27 1997 */
/* revised feb 17 1997 */

#include "alloc.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef int       (*pfn_comparekeys)  (void *key, void *object) ;
typedef void      (*pfn_createobject) (void *object, void *key) ;
typedef void      (*pfn_destroyobject)(void *object) ;
typedef void      (*pfn_assignobject) (void *dstobj, void *srcobj) ;

typedef struct avl_vtbl
{
	 pfn_comparekeys        comparekeys ;
	 pfn_createobject       createobject ;
	 pfn_assignobject       assignobject ;
	 pfn_destroyobject      destroyobject ;
} AVL_vtbl ;

typedef struct avltree
{
	AVL_vtbl *vptr ;
	void *root ;
	int n ;
	size_t size ;
	ys_allocator_t *a;
} AVLTree ;

AVLTree * AVLTree_New               (AVL_vtbl *vtbl, size_t objectsize,
						     size_t growby) ;

void *    AVLTree_Search            (AVLTree *tree, void *key) ;
void *    AVLTree_Insert            (AVLTree *tree, void *key) ;
int       AVLTree_Delete            (AVLTree *tree, void *key) ;
int       AVLTree_DeleteObject      (AVLTree *tree, void *object) ;

void *    AVLTree_FindFirst         (AVLTree *tree) ;
void *    AVLTree_FindLast          (AVLTree *tree) ;
void *    AVLTree_FindNext          (AVLTree *tree, void *currentobject) ;
void *    AVLTree_FindPrevious      (AVLTree *tree, void *currentobject) ;

int       AVLTree_Height            (AVLTree *tree) ;

void      AVLTree_Foreach           (AVLTree *tree, void (*funcptr)(void *));
void      AVLTree_Backeach          (AVLTree *tree, void (*funcptr)(void *));

void      AVLTree_Destroy           (AVLTree *tree) ;

#ifdef __cplusplus
}
#endif

#endif

