/***
*    YASE (Yet Another Search Engine)
*    Copyright (C) 2000-2002  Dibyendu Majumdar.
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

/**
 * The circular doubly linked list implementation is based upon algorithm
 * described in Knuth's The Art of Computer Programing, Vol 1,
 * section 2.2.5. 
 */

#include "list.h"

#define ys_list_empty(list)    (list->lhead.rlink == &list->lhead)
#define ys_list_top(list)      (list->lhead.rlink)
#define ys_list_bottom(list)   (list->lhead.llink)

void
ys_list_init(ys_list_t *list)
{
	list->lhead.rlink = &list->lhead;
	list->lhead.llink = &list->lhead;
}

void  
ys_list_insert_after(ys_list_t *list, void *anchorp, void *linkp)
{
	ys_link_t *anchor = (ys_link_t *)anchorp;
	ys_link_t *link = (ys_link_t *)linkp; 

	if (ys_list_empty(list)) {
		anchor = &list->lhead;
	}
	/* LLINK(P) <- X (Knuth) */
	link->llink = anchor;
	/* RLINK(P) <- RLINK(X) (Knuth) */
	link->rlink = anchor->rlink;
	/* LLINK(RLINK(X)) <- P (Knuth) */
	anchor->rlink->llink = link;
	/* RLINK(X) <- P (Knuth) */
	anchor->rlink = link;
}

void  
ys_list_insert_before(ys_list_t *list, void *anchorp, void *linkp)
{
	ys_link_t *anchor = (ys_link_t *)anchorp;
	ys_link_t *link = (ys_link_t *)linkp; 

	if (ys_list_empty(list)) {
		anchor = &list->lhead;
	}
	link->rlink = anchor;
	link->llink = anchor->llink;
	anchor->llink->rlink = link;
	anchor->llink = link;
}

void  
ys_list_append(ys_list_t *list, void *link)
{
	ys_list_insert_after(list, ys_list_bottom(list), link);
}

void
ys_list_prepend(ys_list_t *list, void *link)
{
	ys_list_insert_before(list, ys_list_top(list), link);
}

void  
ys_list_remove(ys_list_t *list, void *linkp)
{
	ys_link_t *link = (ys_link_t *)linkp; 

	/* RLINK(LLINK(X)) <- RLINK(X) (Knuth) */
	link->llink->rlink = link->rlink;
	/* LLINK(RLINK(X)) <- LLINK(X) (Knuth) */
	link->rlink->llink = link->llink;
	link->rlink = link->llink = 0;
}

void *
ys_list_pop(ys_list_t *list) 
{
	void *link = ys_list_last(list);
	if (link != 0)
		ys_list_remove(list, link);
	return link;
}	

void * 
ys_list_first(ys_list_t *list)
{
	if (ys_list_empty(list)) {
		return 0;
	}
	return ys_list_top(list);
}

void * 
ys_list_last(ys_list_t *list)
{
	if (ys_list_empty(list)) {
		return 0;
	}
	return ys_list_bottom(list);
}

void * 
ys_list_next(ys_list_t *list, void *linkp)
{
	ys_link_t *link = (ys_link_t *)linkp; 
	if (link == ys_list_bottom(list)) {
		return 0;
	}
	return link->rlink;
}

void * 
ys_list_prev(ys_list_t *list, void *linkp)
{
	ys_link_t *link = (ys_link_t *)linkp; 
	if (link == ys_list_top(list)) {
		return 0;
	}
	return link->llink;
}

/* #define STANDALONE */
#ifdef STANDALONE

int main(void)
{
	typedef struct {
		ys_link_t l;
		int a;
	} Item;

	Item a = { {0,0}, 1 };
	Item b = { {0,0}, 2 };
	Item c = { {0,0}, 3 };
	Item d = { {0,0}, 4 };
	Item e = { {0,0}, 5 };
	Item f = { {0,0}, 0 };

	ys_list_t l = { {0,0} };

	Item *ptr = 0;

	ys_list_init(&l);
	ys_list_append(&l, &b);
	ys_list_insert_before(&l, &b, &a);
	ys_list_insert_after(&l, &b, &d);
	ys_list_insert_after(&l, &b, &c);
	ys_list_append(&l, &e);
	ys_list_prepend(&l, &f);

	for (ptr = (Item *)ys_list_first(&l);
		  ptr != 0;
		  ptr = (Item *)ys_list_next(&l, ptr))
	{
		printf("%d\n", ptr->a);
	}

	ys_list_remove(&l, &f);
	ys_list_remove(&l, &e);
	ys_list_remove(&l, &b);
	ys_list_remove(&l, &c);
	for (ptr = (Item *)ys_list_last(&l);
		  ptr != 0;
		  ptr = (Item *)ys_list_prev(&l, ptr))
	{
		printf("%d\n", ptr->a);
	}
	ys_list_remove(&l, &d);
	ys_list_remove(&l, &a);
	for (ptr = (Item *)ys_list_first(&l);
		  ptr != 0;
		  ptr = (Item *)ys_list_next(&l, ptr))
	{
		printf("%d\n", ptr->a);
	}

	return 0;
}

#endif
