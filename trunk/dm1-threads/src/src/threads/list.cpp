/***
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
*/

#include "dm1/list.h"

namespace dm1 {

	void LinkList::append( Linkable *link ) {
		if (head == 0)
			head = link;
		link->setPrev(tail, ID);
		if (tail != 0)
			tail->setNext(link, ID);
		tail = link;
		link->setNext(0, ID);
		count++;
	}

	void LinkList::prepend( Linkable *link ) {
		if (tail == 0)
			tail = link;
		link->setNext(head, ID);
		if (head != 0)
			head->setPrev(link, ID);
		head = link;
		link->setPrev(0, ID);
		count++;
	}

	void LinkList::insertBefore( Linkable *anchor, Linkable *link ) {

		if (anchor == 0) {
			prepend(link);
		}
		else {
			Linkable *prev = anchor->getPrev(ID);
			link->setNext(anchor, ID);
			link->setPrev(prev, ID);
			anchor->setPrev(link, ID);
			if (prev == 0) {
				head = link;
			}
			else {
				prev->setNext(link, ID);
			}
			count++;
		}
	}

	void LinkList::insertAfter( Linkable *anchor, Linkable *link ) {

		if (anchor == 0) {
			append(link);
		}
		else {
			Linkable *next = anchor->getNext(ID);
			link->setPrev(anchor, ID);
			link->setNext(next, ID);
			anchor->setNext(link, ID);
			if (next == 0) {
				tail = link;
			}
			else {
				next->setPrev(link, ID);
			}
			count++;
		}
	}

	void LinkList::remove( Linkable *link ) {

		Linkable *next = link->getNext(ID);
		Linkable *prev = link->getPrev(ID);
		if (next != 0) {
			next->setPrev(prev, ID);
		}
		else {
			tail = prev;
		}
		if (prev != 0) {
			prev->setNext(next, ID);
		}
		else {
			head = next;
		}
		link->setNext(0, ID);
		link->setPrev(0, ID);
		count--;
	}

	bool LinkList::contains(Linkable *link) const {
		Linkable *cursor = head;

		while (cursor != 0) {
			if (cursor == link) {
				return true;
			}
			else {
				cursor = cursor->getNext(ID);
			}
		}
		return false;
	}

}

/* #define STANDALONE */
#ifdef TEST_DM1_LIST

#include <assert.h>
#include <stdio.h>

using namespace dm1;

class Item : public SLinkable {
public:
	int a;
	Item(int a) { this->a = a; }
};

class DItem : public DLinkable {
public:
	int a;
	DItem(int a) : DLinkable() { 
		this->a = a; 
	}
};

int test1()
{
	LinkList list;

	Item a(1);
	Item b(2);
	Item c(3);
	Item d(4);
	Item e(5);
	Item f(0);

	Item *ptr = 0;

	list.append(&a);
	list.insertBefore(&b, &a);
	list.insertAfter(&b, &d);
	list.insertAfter(&b, &c);
	list.append(&e);
	list.prepend(&f);

	printf("expect 0,1,2,3,4,5\n");
	for (ptr = (Item *)list.getFirst();
		  ptr != 0;
		  ptr = (Item *)ptr->getNext())
	{
		printf("%d\n", ptr->a);
	}

	list.remove(&f);
	list.remove(&e);
	list.remove(&b);
	list.remove(&c);
	printf("expect 4,1\n");
	for (ptr = (Item *)list.getLast();
		  ptr != 0;
		  ptr = (Item *)ptr->getPrev())
	{
		printf("%d\n", ptr->a);
	}

	list.remove(&d);
	list.remove(&a);
	printf("expect none\n");
	for (ptr = (Item *)list.getFirst();
		  ptr != 0;
		  ptr = (Item *)ptr->getNext())
	{
		printf("%d\n", ptr->a);
	}
	assert(list.isEmpty());

	return 0;
}

int test2()
{
	LinkList list1(0);
	LinkList list2(1);

	DItem a(1);
	DItem b(2);
	DItem c(3);
	DItem d(4);
	DItem e(5);
	DItem f(0);

	DItem *ptr = 0;

	list1.append(&a);
	list1.insertBefore(&b, &a);
	list1.insertAfter(&b, &d);
	list1.insertAfter(&b, &c);
	list1.append(&e);
	list1.prepend(&f);

	list2.append(&a);
	list2.append(&b);
	list2.append(&d);
	list2.append(&c);
	list2.append(&e);
	list2.append(&f);

	printf("Dumping list1: expect 0,1,2,3,4,5\n");
	for (ptr = (DItem *)list1.getFirst();
		  ptr != 0;
		  ptr = (DItem *)ptr->getNext(0))
	{
		printf("%d\n", ptr->a);
	}

	printf("Dumping list2: expect 1,2,4,3,5,0\n");
	for (ptr = (DItem *)list2.getFirst();
		  ptr != 0;
		  ptr = (DItem *)ptr->getNext(1))
	{
		printf("%d\n", ptr->a);
	}

	return 0;
}

int main(int argc, const char *argv[])
{
	test1();
	test2();
	return 0;
}

#endif
