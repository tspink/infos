/* SPDX-License-Identifier: MIT */

/*
 * include/kernel/sched.h
 * 
 * InfOS
 * Copyright (C) University of Edinburgh 2016.  All Rights Reserved.
 * 
 * Tom Spink <tspink@inf.ed.ac.uk>
 */
#pragma once

#include <infos/kernel/subsystem.h>
#include <infos/kernel/sched-entity.h>
#include <infos/kernel/log.h>
#include <infos/util/list.h>

namespace infos
{
	namespace kernel
	{
		class Scheduler;
		
		class SchedulingAlgorithm
		{
			friend class Scheduler;
			
		public:
			virtual const char *name() const = 0;
			virtual SchedulingEntity *pick_next_entity();
			
			virtual void add_to_runqueue(SchedulingEntity& entity) = 0;
			virtual void remove_from_runqueue(SchedulingEntity& entity) = 0;
		};

		typedef SchedulingAlgorithm *(*SchedulingAlgorithmFactory)(void);

        class Scheduler : public Subsystem
		{
		public:
			Scheduler(Kernel& owner);

            Scheduler(Scheduler&&) = delete;
            Scheduler(const Scheduler&) = delete;
			
			bool init();
			
			SchedulingAlgorithm& algorithm() const { return *_algorithm; }
			void algorithm(SchedulingAlgorithm& algorithm) { _algorithm = &algorithm; }
			
			__noreturn void run();
			
			void schedule();
			
			void set_entity_state(SchedulingEntity& entity, SchedulingEntityState::SchedulingEntityState state);
            void set_current_thread(Thread& thread);

			SchedulingEntity& current_entity() const { return *_current; }
			Thread* current_thread() const { return _current_thread; }
			
			void update_accounting();
			
		private:
			SchedulingAlgorithm *acquire_scheduler_algorithm();
			
			bool _active;
			SchedulingAlgorithm *_algorithm;
			SchedulingEntity *_current;
			SchedulingEntity *_idle_entity;
            Thread *_current_thread;
		};
		
		extern ComponentLog sched_log;

        #define RegisterScheduler(_class, _name)                                                        \
	    static infos::kernel::SchedulingAlgorithm *__sched_alg_factory_##_class() { return new _class(); } \
	    __section(".schedalg") infos::kernel::SchedulingAlgorithmFactory _sched_alg_factor_ptr_##_class = __sched_alg_factory_##_class;
	}
}
