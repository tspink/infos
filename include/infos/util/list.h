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
		template <typename T>
		struct ListNode
		{
			typedef T Elem;
			typedef ListNode<T> Self;

			Self *Prev;
			Self *Next;
			Elem Data;
		};

		template <typename T>
		struct ListIterator
		{
			typedef T Elem;
			typedef ListIterator<Elem> Self;
			typedef ListNode<Elem> Node;

			ListIterator(const Node *current) : _current(current) {}
			ListIterator(const Self &other) : _current(other._current) {}
			ListIterator(Self &&other) : _current(other._current) { other._current = nullptr; }

			const Elem &operator*() const
			{
				assert(_current != nullptr);
				return _current->Data;
			}

			void operator++()
			{
				if (_current)
					_current = _current->Next;
			}

			bool operator==(const Self &other) const
			{
				return _current == other._current;
			}

			bool operator!=(const Self &other) const
			{
				return _current != other._current;
			}

		private:
			const Node *_current;
		};

		template <typename T>
		class List
		{
		public:
			typedef T Elem;
			typedef const T ConstElem;

			typedef List<Elem> Self;
			typedef ListNode<Elem> Node;
			typedef ListIterator<Elem> Iterator;

			List()
			{
				_construct();
			}

			List(const Self &r)
			{
				_construct();
				for (const Elem &elem : r)
				{
					push_back(elem);
				}
			}

			List(Self &&r) : _node(r._node), _count(r._count) {}

			~List()
			{
				Node *ptr = _node.Next;
				while (!_is_dummy_node(ptr))
				{
					Node *next = ptr->Next;
					delete ptr;
					ptr = next;
				}
			}

			void push_back(Elem const &elem)
			{
				Node *new_node = new Node();
				new_node->Data = elem;
				new_node->Next = &_node;
				new_node->Prev = _node.Prev;

				_node.Prev->Next = new_node;
				_node.Prev = new_node;
				++_count;
			}

			void push_front(Elem const &elem)
			{
				Node *new_node = new Node();
				new_node->Data = elem;
				new_node->Prev = &_node;
				new_node->Next = _node.Next;

				_node.Next->Prev = new_node;
				_node.Next = new_node;
				++_count;
			}

			void remove(Elem const &elem)
			{
				Node *ptr = _node.Next;
				while (!_is_dummy_node(ptr) && ptr->Data != elem)
				{
					ptr = ptr->Next;
				}

				if (!_is_dummy_node(ptr))
				{
					ptr->Prev->Next = ptr->Next;
					ptr->Next->Prev = ptr->Prev;
					delete ptr;
					--_count;
				}
			}

			Elem pop_back()
			{
				assert(!_is_dummy_node(_node.Prev));

				Node *ptr = _node.Prev;
				Elem ret = ptr->Data;
				ptr->Prev->Next = ptr->Next;
				ptr->Next->Prev = ptr->Prev;
				delete ptr;
				--_count;

				return ret;
			}

			Elem pop_front()
			{
				assert(!_is_dummy_node(_node.Next));

				Node *ptr = _node.Next;
				Elem ret = ptr->Data;
				ptr->Prev->Next = ptr->Next;
				ptr->Next->Prev = ptr->Prev;
				delete ptr;
				--_count;

				return ret;
			}

			// legacy support
			void append(Elem const &elem)
			{
				push_back(elem);
			}

			// legacy support
			Elem dequeue()
			{
				return pop_front();
			}

			// legacy support
			void enqueue(Elem const &elem)
			{
				push_back(elem);
			}

			// legacy support
			void push(Elem const &elem)
			{
				push_front(elem);
			}

			// legacy support
			Elem pop()
			{
				return pop_front();
			}

			Iterator begin() const
			{
				return Iterator(_node.Next);
			}

			Iterator end() const
			{
				return Iterator(&_node);
			}

			Elem const &first() const
			{
				assert(!_is_dummy_node(_node.Next));
				return _node.Next->Data;
			}

			Elem const &last() const
			{
				assert(!_is_dummy_node(_node.Prev));
				return _node.Prev->Data;
			}

			Elem const &at(int index) const
			{
				int counter = 0;
				Node *ptr = _node.Next;
				while (counter != index && !_is_dummy_node(ptr))
				{
					ptr = ptr->Next;
					++counter;
				}

				assert(!_is_dummy_node(ptr));
				return ptr->Data;
			}

			unsigned int count() const
			{
				return _count;
			}

			void clear()
			{
				Node *ptr = _node.Next;
				while (!_is_dummy_node(ptr))
				{
					Node *tmp = ptr;
					ptr = tmp->Next;
					delete tmp;
				}

				_node.Next = &_node;
				_node.Prev = &_node;
				_count = 0;
			}

			bool empty() const { return _count == 0; }

		private:
			Node _node; // dummy node
			unsigned int _count;

			bool _is_dummy_node(Node *ptr) const
			{
				return ptr == &_node;
			}

			void _construct()
			{
				_node.Prev = &_node;
				_node.Next = &_node;
				_count = 0;
			}
		};
	}
}
