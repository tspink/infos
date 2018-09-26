/* SPDX-License-Identifier: MIT */

#pragma once

namespace infos {
	namespace util {

		template<typename _Tp>
		struct RemoveReference {
			typedef _Tp type;
		};

		template<typename _Tp>
		struct RemoveReference<_Tp&> {
			typedef _Tp type;
		};

		template<typename _Tp>
		struct RemoveReference<_Tp&&> {
			typedef _Tp type;
		};

		template<typename _Tp>
		constexpr typename RemoveReference<_Tp>::type&& Move(_Tp&& __t) noexcept {
			return static_cast<typename RemoveReference<_Tp>::type&&> (__t);
		}
	}
}
