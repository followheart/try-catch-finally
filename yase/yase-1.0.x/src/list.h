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
#ifndef list_h
#define list_h

typedef struct ys_link_t
{
	struct ys_link_t *llink, *rlink ;
} ys_link_t ;

typedef struct
{
	ys_link_t  lhead ;
} ys_list_t ;

void   ys_list_init         ( ys_list_t *list );
void   ys_list_append       ( ys_list_t *list, void *link );
void   ys_list_prepend      ( ys_list_t *list, void *link );
void   ys_list_insert_after ( ys_list_t *list, void *anchor, void *link );
void   ys_list_insert_before( ys_list_t *list, void *anchor, void *link );
void * ys_list_first        ( ys_list_t *list );
void * ys_list_last         ( ys_list_t *list );
void * ys_list_next         ( ys_list_t *list, void *link );
void * ys_list_prev         ( ys_list_t *list, void *link );
void   ys_list_remove       ( ys_list_t *list, void *link );
void   ys_list_push         ( ys_list_t *list, void *link );
void * ys_list_pop          ( ys_list_t *list );

#define ys_list_push(list, link) ys_list_append((list), (link))

#endif
