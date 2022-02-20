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
			Elem *Data;

			ListNode() : Prev(this), Next(this), Data(nullptr){}; // for dummy object
			ListNode(Elem const &data) : Prev(this), Next(this), Data(new Elem(data)) {}
			~ListNode()
			{
				if (Data)
				{
					delete Data;
					Data = nullptr;
				}
			}

			Elem &get_data()
			{
				assert(Data);
				return *Data;
			}

			const Elem &get_data() const
			{
				assert(Data);
				return *Data;
			}
		};

		template <typename T>
		struct ListIterator
		{
			typedef T Elem;
			typedef ListIterator<Elem> Self;
			typedef ListNode<Elem> Node;

			ListIterator(Node *current) : _current(current) {}
			ListIterator(const Self &other) : _current(other._current) {}
			ListIterator(Self &&other) : _current(other._current) { other._current = nullptr; }

			Elem &operator*() const
			{
				assert(_current != nullptr);
				return (_current->get_data());
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
			Node *_current;
		};

		template <typename T>
		struct ListConstIterator
		{
			typedef T Elem;
			typedef ListIterator<Elem> Iterator;
			typedef ListConstIterator<Elem> Self;
			typedef ListNode<Elem> Node;

			ListConstIterator(const Node *current) : _current(current) {}
			ListConstIterator(const Self &other) : _current(other._current) {}
			ListConstIterator(const Iterator &other) : _current(other._current) {} // convert iterator to const_iterator
			ListConstIterator(Self &&other) : _current(other._current) { other._current = nullptr; }

			const Elem &operator*() const
			{
				assert(_current != nullptr);
				return _current->get_data();
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
			typedef ListConstIterator<Elem> ConstIterator;
			typedef unsigned int size_type;

			List() : _node(Node()), _count(0) {}

			List(const Self &r) : _node(Node()), _count(0)
			{
				for (const Elem &elem : r)
				{
					push_back(elem);
				}
			}

			List(Self &&r) : _node(r._node), _count(r._count)
			{
				r._node.Prev = nullptr;
				r._node.Next = nullptr;
				r._node.Data = nullptr;
				r._count = 0;
			}

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
				Node *new_node = new Node(elem);
				new_node->Next = &_node;
				new_node->Prev = _node.Prev;

				_node.Prev->Next = new_node;
				_node.Prev = new_node;
				++_count;
			}

			void push_front(Elem const &elem)
			{
				Node *new_node = new Node(elem);
				new_node->Prev = &_node;
				new_node->Next = _node.Next;

				_node.Next->Prev = new_node;
				_node.Next = new_node;
				++_count;
			}

			void remove(Elem const &elem)
			{
				Node *ptr = _node.Next;
				while (!_is_dummy_node(ptr) && ptr->get_data() != elem)
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
				Elem ret = ptr->get_data();
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
				Elem ret = ptr->get_data();
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

			Iterator begin()
			{
				return Iterator(_node.Next);
			}

			ConstIterator begin() const
			{
				return ConstIterator(_node.Next);
			}

			Iterator end()
			{
				return Iterator(&_node);
			}

			ConstIterator end() const
			{
				return ConstIterator(&_node);
			}

			Elem &first()
			{
				assert(!_is_dummy_node(_node.Next));
				return _node.Next->get_data();
			}

			Elem const &first() const
			{
				assert(!_is_dummy_node(_node.Next));
				return _node.Next->get_data();
			}

			Elem &last()
			{
				assert(!_is_dummy_node(_node.Prev));
				return _node.Prev->get_data();
			}

			Elem const &last() const
			{
				assert(!_is_dummy_node(_node.Prev));
				return _node.Prev->get_data();
			}

			Elem &at(int index)
			{
				int counter = 0;
				Node *ptr = _node.Next;
				while (counter != index && !_is_dummy_node(ptr))
				{
					ptr = ptr->Next;
					++counter;
				}

				assert(!_is_dummy_node(ptr));
				return ptr->get_data();
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
				return ptr->get_data();
			}

			size_type count() const
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
			size_type _count;

			bool _is_dummy_node(Node *ptr) const
			{
				return ptr == &_node;
			}
		};
	}
}
