/* SPDX-License-Identifier: MIT */

/*
 * include/infos/util/map.h
 * 
 * InfOS
 * Copyright (C) University of Edinburgh 2016.  All Rights Reserved.
 * 
 * Tom Spink <tspink@inf.ed.ac.uk>
 */
#pragma once

#include <infos/define.h>
#include <infos/util/list.h>

namespace infos {
	namespace util {

		template<typename TKey, typename TValue>
		struct MapNode {

			enum Colour {
				RED,
				BLACK
			};

			typedef MapNode<TKey, TValue> Self;
			typedef TKey KeyType;
			typedef TValue ValueType;

			KeyType Key;
			ValueType Value;
			Colour Colour;
			
			MapNode(KeyType key, ValueType value) 
			: Key(key),
				Value(value),
				Colour(RED), 
				Parent(NULL), Left(NULL), Right(NULL) {
				
			}

			~MapNode() {
				if (Left) delete Left;
				if (Right) delete Right;
			}
			
			inline bool i_am_left() const {
				assert(parent());
				return this == parent()->Left;
			}

			inline bool i_am_right() const {
				assert(parent());
				return this == parent()->Right;
			}

			inline Self *parent() const {
				return Parent;
			}

			inline Self *grandparent() const {
				if (parent() == NULL) return NULL;
				return parent()->parent();
			}

			inline Self *uncle() const {
				Self *gp = grandparent();

				if (gp == NULL) return NULL;

				if (this->parent()->i_am_left()) {
					return gp->Right;
				} else {
					return gp->Left;
				}
			}
			
			inline Self *left() const {
				return Left;
			}

			inline void left(Self *n) {
				Left = n;
				
				if (n) {
					n->Parent = this;
				}
			}

			inline Self *right() const {
				return Right;
			}
			
			inline void right(Self *n) {
				Right = n;
				
				if (n) {
					n->Parent = this;
				}
			}
			
			inline void clear_parent() {
				Parent = NULL;
			}
			
			inline bool red() const { 
				return Colour == RED;
			}
			
			inline bool black() const {
				return Colour == BLACK;
			}
			
		private:
			Self *Parent;
			Self *Left, *Right;
		};

		template<typename TNode>
		struct MapIteratorPair {

			MapIteratorPair(const TNode& node) : key(node.Key), value(node.Value) {
			}

			const typename TNode::KeyType& key;
			const typename TNode::ValueType& value;
		};

		template<typename TNode>
		class MapIterator {
		public:
			typedef MapIteratorPair<TNode> Pair;
			typedef MapIterator<TNode> Self;

			MapIterator(const TNode *root) : _current(root) {
				if (root) {
					_queue.enqueue(root);
					advance();
				}
			}

			const Pair operator*() const {
				assert(_current);
				return Pair(*_current);
			}

			void operator++() {
				advance();
			}

			bool operator==(const Self& other) const {
				return other._current == _current;
			}

			bool operator!=(const Self& other) const {
				return other._current != _current;
			}

		private:
			List<const TNode *> _queue;
			const TNode *_current;

			void advance() {
				if (!_queue.empty()) {
					_current = _queue.dequeue();

					if (_current->left()) {
						_queue.enqueue(_current->left());
					}

					if (_current->right()) {
						_queue.enqueue(_current->right());
					}
				} else {
					_current = NULL;
				}
			}
		};

		template<typename TKey, typename TValue>
		class Map {
		public:
			typedef MapNode<TKey, TValue> Node;
			typedef MapIterator<Node> Iterator;
			typedef const MapIterator<Node> ConstIterator;
			typedef Map<TKey, TValue> Self;
			
			Map(const Self&) = delete;
			Map(Self&&) = delete;
			
			Map() : _root(NULL), _count(0) {
			}

			~Map() {
				if (_root) delete _root;
			}

			void add(TKey const& key, TValue const& value) {
				Node *owner = _root;

				while (owner) {
					if (key < owner->Key) {
						if (!owner->left()) break;
						owner = owner->left();
					} else if (key > owner->Key) {
						if (!owner->right()) break;
						owner = owner->right();
					} else {
						break;
					}
				}

				if (owner) {
					if (key == owner->Key) {
						owner->Value = value;
					} else {
						Node *nw = new Node(key, value);

						if (key < owner->Key) {
							owner->left(nw);
						} else {
							owner->right(nw);
						}

						rebalance_insert(nw);

						_count++;
					}
				} else {
					_root = new Node(key, value);
					_root->Colour = Node::BLACK;
					
					_count++;
				}
			}

			void remove(TKey const& key) {
				// TODO
			}

			void clear() {
				if (_root) delete _root;
				_root = NULL;
				_count = 0;
			}

			bool contains_key(TKey const& key) {
				Node *node = _root;

				while (node) {
					if (key < node->Key) {
						node = node->left();
					} else if (key > node->Key) {
						node = node->right();
					} else {
						return true;
					}
				}

				return false;
			}

			bool try_get_value(TKey const& key, TValue& value) const {
				Node *node = _root;

				while (node) {
					if (key < node->Key) {
						node = node->left();
					} else if (key > node->Key) {
						node = node->right();
					} else {
						value = node->Value;
						return true;
					}
				}

				return false;
			}

			ConstIterator begin() const {
				return Iterator(_root);
			}

			ConstIterator end() const {
				return Iterator(NULL);
			}
			
			unsigned int count() const { return _count; }
			
			Node *root() const { return _root; }

		private:
			Node *_root;
			unsigned int _count;
						
			void rotate_right(Node *n)
			{
				assert(n);
				
				Node *pivot = n->left();
				assert(pivot);
				
				if (n->parent() == NULL) {
					_root = pivot;
					_root->clear_parent();
				} else {
					if (n->i_am_left()) {
						n->parent()->left(pivot);
					} else {
						n->parent()->right(pivot);
					}
				}

				n->left(pivot->right());
				pivot->right(n);
			}
			
			void rotate_left(Node *n)
			{
				assert(n);
				
				Node *pivot = n->right();
				assert(pivot);

				if (n->parent() == NULL) {
					_root = pivot;
					_root->clear_parent();
				} else {
					if (n->i_am_left()) {
						n->parent()->left(pivot);
					} else {
						n->parent()->right(pivot);
					}
				}

				n->right(pivot->left());
				pivot->left(n);
			}

			void rebalance_insert(Node *nw)
			{
				nw->Colour = Node::RED;
				
				Node *x = nw, *y;
				while (x->parent() && x->parent()->red()) {
					if (x->parent() == x->grandparent()->left()) {
						y = x->grandparent()->right();
						if (y && y->red()) {
							x->parent()->Colour = Node::BLACK;
							y->Colour = Node::BLACK;
							x->grandparent()->Colour = Node::RED;
							x = x->grandparent();
						} else {
							if (x == x->parent()->right()) {
								x = x->parent();
								rotate_left(x);
							}
							
							x->parent()->Colour = Node::BLACK;
							x->grandparent()->Colour = Node::RED;
							rotate_right(x->grandparent());
						}
					} else {
						y = x->grandparent()->left();
						if (y && y->red()) {
							x->parent()->Colour = Node::BLACK;
							y->Colour = Node::BLACK;
							x->grandparent()->Colour = Node::RED;
							x = x->grandparent();
						} else {
							if (x == x->parent()->left()) {
								x = x->parent();
								rotate_right(x);
							}
							
							x->parent()->Colour = Node::BLACK;
							x->grandparent()->Colour = Node::RED;
							rotate_left(x->grandparent());
						}
					}
				}
				
				_root->Colour = Node::BLACK;
			}
		};
	}
}
