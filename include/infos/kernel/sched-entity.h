/* SPDX-License-Identifier: MIT */

#pragma once

#include <infos/util/time.h>
#include <infos/util/event.h>
#include <infos/util/string.h>

namespace infos
{
	namespace kernel
	{
		class Scheduler;
		
		namespace SchedulingEntityState
		{
			enum SchedulingEntityState
			{
				STOPPED,
				SLEEPING,
				RUNNABLE,
				RUNNING,
			};
		}

        namespace SchedulingEntityPriority
        {
            enum SchedulingEntityPriority
            {
                REALTIME,
                INTERACTIVE,
                NORMAL,
                DAEMON,
            };
        }
		
		class SchedulingEntity
		{
			friend class Scheduler;
		public:
			typedef util::Nanoseconds EntityRuntime;
			typedef util::KernelRuntimeClock::Timepoint EntityStartTime;

			SchedulingEntity(SchedulingEntityPriority::SchedulingEntityPriority priority, const util::String& name)
			: _cpu_runtime(0), _exec_start_time(0), _name(name), _state(SchedulingEntityState::STOPPED), _priority(priority) { }
			virtual ~SchedulingEntity() { }
			
			virtual bool activate(SchedulingEntity *prev) = 0;
			
			EntityRuntime cpu_runtime() const { return _cpu_runtime; }
			
			void increment_cpu_runtime(EntityRuntime delta) { _cpu_runtime += delta; }
			void update_exec_start_time(EntityStartTime exec_start_time) { _exec_start_time = exec_start_time; }

            const util::String& name() const { return _name; }
            SchedulingEntityState::SchedulingEntityState state() const { return _state; }
            SchedulingEntityPriority::SchedulingEntityPriority priority() const { return _priority; }

			bool stopped() const { return _state == SchedulingEntityState::STOPPED; }
			
			util::Event& state_changed() { return _state_changed; }
			
		private:
			EntityRuntime _cpu_runtime;
			EntityStartTime _exec_start_time;

            const util::String _name;
            SchedulingEntityState::SchedulingEntityState _state;
            SchedulingEntityPriority::SchedulingEntityPriority _priority;
            util::Event _state_changed;
		};
	}
}
