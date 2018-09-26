/* SPDX-License-Identifier: MIT */

#pragma once

namespace infos
{
	namespace util
	{
		template<typename T>
		class Enumerator
		{
		public:
			T& operator *() const {	return _current; }
			
		protected:
			virtual bool has_next() = 0;
			virtual T get_next() = 0;
			
		private:
			T _current;
		};
		
		template<typename T>
		class Enumerable
		{
		public:
			virtual Enumerator<T> begin() = 0;
			virtual Enumerator<T> end() = 0;
		};
	}
}
