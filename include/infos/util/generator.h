/* SPDX-License-Identifier: MIT */
#pragma once

#include <infos/util/list.h>

namespace infos
{
	namespace util
	{
		template<typename TElem>
		struct GeneratorIterator
		{
			typedef TElem Elem;
			typedef const TElem ConstElem;
			typedef List<Elem> ListType;
			typedef bool (*Callback)(ConstElem& elem);

			typedef GeneratorIterator<Elem> Self;
			typedef typename ListType::Iterator ListTypeIterator;
			
			GeneratorIterator(ListTypeIterator iter, Callback cb) : _iter(iter), _cb(cb) { }
			
			Elem& operator*() const {
				return *_iter;
			}
			
			void operator++() {
				do {
					++_iter;
				} while (!_cb(*_iter));
			}
			
			bool operator==(const Self other) const {
				return _iter == other._iter;
			}
		
			bool operator!=(const Self other) const {
				return _iter != other._iter;
			}
		
		private:
			ListTypeIterator _iter;
			Callback _cb;
		};
		
		template<typename TElem>
		class Generator
		{
		public:
			typedef GeneratorIterator<TElem> Iterator;
			typedef typename Iterator::ListType ListType;
			typedef typename Iterator::Callback Callback;
			typedef typename ListType::Iterator ListTypeIterator;
			
			Generator(ListType list, Callback cb) : _list(list), _cb(cb) { }
						
			Iterator begin()
			{
				ListTypeIterator iter = _list.begin();
				while (!_cb(*iter)) ++iter;
				
				return Iterator(iter, _cb);
			}

			Iterator end()
			{
				return Iterator(_list.end(), _cb);
			}
			
		private:
			ListType _list;
			Callback _cb;
		};
	}
}
