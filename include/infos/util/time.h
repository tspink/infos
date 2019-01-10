#ifndef TIME_H
#define TIME_H

#include <infos/util/ratio.h>

namespace infos {
	namespace util {

		template<typename R, typename P = uint64_t>
		struct Duration {
			typedef Duration<R, P> Self;
			typedef R _R;
			typedef P _P;

			P _val;

			Duration() : _val(0) {
			}

			Duration(P val) : _val(val) {
			}

			Self& operator+=(const Self& amt) {
				_val += amt._val;
				return *this;
			}

			Self& operator-=(const Self& amt) {
				_val -= amt._val;
				return *this;
			}

			friend Self operator+(const Self& l, const Self& r) {
				return Self(l._val + r._val);
			}

			friend Self operator+(const Self& l, const P& r) {
				return Self(l._val + r);
			}

			friend Self operator-(const Self& l, const Self& r) {
				return Self(r._val - l._val);
			}
			
			friend bool operator<(const Self& l, const Self& r) {
				return l._val < r._val;
			}

			friend bool operator>(const Self& l, const Self& r) {
				return l._val > r._val;
			}

			P count() const {
				return _val;
			}
		};

		typedef Duration<Ratio<1, 1> > Seconds;
		typedef Duration<Ratio < 1, (int) 1e3 > > Milliseconds;
		typedef Duration<Ratio < 1, (int) 1e6 > > Microseconds;
		typedef Duration<Ratio < 1, (int) 1e9 > > Nanoseconds;

		template<typename Dur = Nanoseconds>
		struct TimepointImpl {
			typedef TimepointImpl<Dur> Self;
			typedef Dur _Dur;

			Dur _val;

			TimepointImpl() : _val(0) {
			}

			TimepointImpl(typename Dur::_P val) : _val(val) {
			}

			const Dur& time_since_epoch() const {
				return _val;
			}

			Self& operator+=(const Dur& amt) {
				_val += amt._val;
				return *this;
			}

			friend Dur operator-(const Self& lhs, const Self& rhs) {
				return Dur(rhs._val - lhs._val);
			}

			friend bool operator<(const Self& l, const Self& r) {
				return l._val._val < r._val._val;
			}

			TimepointImpl(Dur dur) : _val(dur) {
			}
		};

		template<typename Ratio, typename REP>
		struct DurationCastImpl {

			__cxp static REP DoCast(REP in) {

				return REP(static_cast<REP> (static_cast<REP> (in)
						* static_cast<REP> (Ratio::num)
						/ static_cast<REP> (Ratio::den)));

				return in;
			}
		};

		template<typename T, typename F>
		constexpr T DurationCast(const Duration<F, typename T::_P> &v) {
			typedef F InputRatio;
			typedef typename T::_R OutputRatio;
			typedef RatioDivide<InputRatio, OutputRatio> TranslationRatio;
			typedef typename T::_P Rep;

			return DurationCastImpl<TranslationRatio, Rep>::DoCast(v._val);
		}

		template<typename X, typename P>
		TimepointImpl<P> operator+(const TimepointImpl<P>& lhs, const X& rhs) {
			return TimepointImpl<P>(lhs._val + DurationCast<typename TimepointImpl<P>::_Dur > (rhs));
		}

		struct KernelRuntimeClock {
			typedef TimepointImpl<> Timepoint;
		};
		
		struct TimeOfDay {
			unsigned short hours, minutes, seconds;
			unsigned short day, month, year;
		};
	}
}

#endif /* TIME_H */

