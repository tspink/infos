/* SPDX-License-Identifier: MIT */

/*
 * include/util/list.h
 * 
 * InfOS
 * Copyright (C) University of Edinburgh 2016.  All Rights Reserved.
 * 
 * Tom Spink <tspink@inf.ed.ac.uk>
 */
#pragma once

#include <infos/define.h>
#include <infos/util/support.h>

namespace infos
{
	namespace util
	{
		template<typename T>
		struct ListNode
		{
			typedef T Elem;
			typedef ListNode<T> Self;
			
			Self *Next;
			Elem Data;
		};
		
		template<typename T>
		struct ListIterator
		{
			typedef T Elem;
			typedef ListIterator<Elem> Self;
			typedef ListNode<Elem> Node;
			
			ListIterator(Node *current) : _current(current) { }
			ListIterator(const Self& other) : _current(other._current) { }
			ListIterator(Self&& other) : _current(other._current) { other._current = NULL; }
			
			const Elem& operator*() const {
				return _current->Data;
			}
			
			void operator++() {
				if (_current)
					_current = _current->Next;
			}
			
			bool operator==(const Self& other) const {
				return _current == other._current;
			}
		
			bool operator!=(const Self& other) const {
				return _current != other._current;
			}
		
		private:
			Node *_current;
		};
		
		template<typename T>
		class List
		{
		public:
			typedef T Elem;
			typedef const T ConstElem;
			
			typedef List<Elem> Self;
			typedef ListNode<Elem> Node;
			typedef ListIterator<Elem> Iterator;
			
			List() : _elems(NULL), _count(0) { }
			
			// Copy
			List(const Self& r) : _elems(NULL), _count(0) {
				for (const auto& elem : r) {
					append(elem);
				}
			}

			// Move
			List(Self&& r) : _elems(r._elems), _count(r._count) { r._elems = NULL; r._count = 0; }
			
			~List() {
				Node *node = _elems;
				while (node) {
					Node *next = node->Next;
					delete node;
					node = next;
				}
			}
			
			void append(Elem const& elem) {
				Node **slot = &_elems;
				
				while (*slot != NULL) {
					slot = &(*slot)->Next;
				}
				
				*slot = new Node();
				(*slot)->Data = elem;
				(*slot)->Next = NULL;				
				
				_count++;
			}
			
			void remove(Elem const& elem) {
				Node **slot = &_elems;
				
				while (*slot && (*slot)->Data != elem) {
					slot = &(*slot)->Next;
				}
				
				if (*slot) {
					Node *candidate = *slot;
					assert(candidate->Data == elem);
					
					*slot = candidate->Next;
										
					delete candidate;
					_count--;
				}	
			}
			
			Elem dequeue() {
				assert(_elems);
				
				Node *front = _elems;
				
				Elem ret = front->Data;
				_elems = front->Next;
				delete front;
				_count--;
				
				return ret;
			}
			
			void enqueue(Elem const& elem)
			{
				append(elem);
			}
			
			void push(Elem const& elem)
			{
				Node *node = new Node();
				node->Data = elem;
				node->Next = _elems;
				_elems = node;
				
				_count++;
			}
			
			Elem pop() {
				return dequeue();
			}
			
			Elem const& first() const {
				assert(_elems);
				return _elems->Data;
			}

			Elem const& last() const {
				assert(_elems);
				
				Node *last = _elems;
				while (last->Next) {
					last = last->Next;
				}
				
				return last->Data;
			}
			
			Elem const& at(int index) const {
				int ctr = 0;
				Node *n = _elems;
				
				while (ctr != index && n) {
					n = n->Next;
					ctr++;
				}
				
				assert(n);
				return n->Data;
			}
			
			unsigned int count() const {
				return _count;
			}
			
			bool empty() const { return _count == 0; }
			
			void clear()
			{
				Node *cur = _elems;
				while (cur) {
					Node *tmp = cur;
					cur = tmp->Next;
					delete tmp;
				}
				
				_elems = NULL;
				_count = 0;
			}
			
			Iterator begin() const
			{
				return Iterator(_elems);
			}
			
			Iterator end() const
			{
				return Iterator(NULL);
			}
			
		private:			
			Node *_elems;
			unsigned int _count;
		};
	}
}
