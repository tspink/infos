/* SPDX-License-Identifier: MIT */

#pragma once

#include <arch/x86/irq.h>

namespace infos
{
	namespace kernel
	{
		class Thread;
	}
	
	namespace util
	{
		class Lock
		{
		public:
			virtual void lock() = 0;
			virtual void unlock() = 0;
		};
		
		template<typename TLock>
		class UniqueLock
		{
		public:
			explicit UniqueLock(TLock& l) : _l(l) {
				_l.lock();
			}
			
			~UniqueLock() {
				_l.unlock();
			}
			
		private:
			TLock& _l;
		};
		
		class Mutex : public Lock
		{
		public:
			Mutex() : _locked(0) { }
			
			void lock() override;
			void unlock() override;
			
			bool locked() { return !!_locked; }
			bool locked_by_me();
			
		private:
			Mutex(const Mutex& c);
			Mutex(const Mutex&& c);
			
			volatile unsigned long _locked;
			kernel::Thread *_owner;
		};
		
		class ConditionVariable
		{
		public:
			ConditionVariable() { }
			
			void wait(Mutex& mtx);
			void notify_one();
			void notify_all();
		};
		
		class IRQLock : public Lock
		{
		public:
			explicit IRQLock();
			
			void lock() override;
			void unlock() override;
			
		private:
			bool _were_interrupts_enabled;
		};
		
		class UniqueIRQLock
		{
		public:
			UniqueIRQLock() { _lock.lock(); }
			~UniqueIRQLock() { _lock.unlock(); }
			
		private:
			IRQLock _lock;
		};
	}
}
