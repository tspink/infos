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

            virtual void init() = 0;
			virtual void add_to_runqueue(SchedulingEntity& entity) = 0;
			virtual void remove_from_runqueue(SchedulingEntity& entity) = 0;
		};
		
		class Scheduler : public Subsystem
		{
		public:
			Scheduler(Kernel& owner);
			
			bool init();
			
			SchedulingAlgorithm& algorithm() const { return *_algorithm; }
			void algorithm(SchedulingAlgorithm& algorithm) { _algorithm = &algorithm; }
			
			__noreturn void run();
			
			void schedule();
			
			void set_entity_state(SchedulingEntity& entity, SchedulingEntityState::SchedulingEntityState state);

			SchedulingEntity& current_entity() const { return *_current; }
			
			void update_accounting();
			
		private:
			SchedulingAlgorithm *acquire_scheduler_algorithm();
			
			bool _active;
			SchedulingAlgorithm *_algorithm;
			SchedulingEntity *_current;
			SchedulingEntity *_idle_entity;
		};
		
		extern ComponentLog sched_log;
				
		#define RegisterScheduler(_class) static _class __sched_alg_##_class; __section(".schedalg") infos::kernel::SchedulingAlgorithm *__sched_alg_ptr_##_class = &__sched_alg_##_class
	}
}
