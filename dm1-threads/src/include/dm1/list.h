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

/**
 * 1-Sep-02: Created
 */

#ifndef dm1_list_h
#define dm1_list_h

#include "dm1/common.h"

namespace dm1 {

	class Linkable {

	public:
		virtual Linkable *getNext(int ID) const = 0;
		virtual void setNext(Linkable *next, int ID) = 0;
		virtual Linkable *getPrev(int ID) const = 0;
		virtual void setPrev(Linkable *prev, int ID) = 0;
	};

	template <int size = 1>
	class TLinkable : public Linkable {

	protected:
		Linkable *next[size];
		Linkable *prev[size];

	public:
		TLinkable() { 
			for (int i = 0; i < size; i++) {
				next[i] = prev[i] = 0;
			}
		}

		Linkable *getNext(int id = 0) const {
			return next[id];
		}

		void setNext(Linkable *link, int id = 0) {
			next[id] = link;
		}

		Linkable *getPrev(int id = 0) const {
			return prev[id];
		}

		void setPrev(Linkable *link, int id = 0) {
			prev[id] = link;
		}
		
		static int getLinkCount() {
			return size;
		}
	};

	typedef TLinkable<1> SLinkable;
	typedef TLinkable<2> DLinkable;

	class LinkList {

	private:
		Linkable *head;
		Linkable *tail;
		int count;
		int ID;

	public:
		LinkList(int ID = 0);
		virtual ~LinkList();

		Linkable *getFirst() const;
		Linkable *getLast() const;

		bool isEmpty() const;
		int getCount() const;
		int getID() const;
		bool contains(Linkable *link) const;

		void append(Linkable *link);
		void prepend(Linkable *link);
		void insertBefore(Linkable *anchor, Linkable *link);
		void insertAfter(Linkable *anchor, Linkable *link);
		void remove(Linkable *link);
		void removeAll();

		void push(Linkable *link);
		Linkable *pop();
	};

	inline LinkList::LinkList(int ID) {
		this->ID = ID;
		head = tail = 0;
		count = 0;
	}

	inline LinkList::~LinkList() {
		assert(count == 0);
//		while (count > 0)
//			remove(head);
	}

	inline void LinkList::removeAll() {
		while (count > 0)
			remove(head);
	}

	inline Linkable *LinkList::getFirst() const {
		return head;
	}

	inline Linkable *LinkList::getLast() const {
		return tail;
	}

	inline int LinkList::getCount() const {
		return count;
	}

	inline bool LinkList::isEmpty() const {
		return count == 0;
	}

	inline int LinkList::getID() const {
		return ID;
	}

	inline void LinkList::push(Linkable *link) {
		append(link);
	}

	inline Linkable *LinkList::pop() {
		Linkable *popped = tail;
		if (popped != 0) {
			remove(popped);
		}
		return popped;
	}

}

#endif
