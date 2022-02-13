/* SPDX-License-Identifier: MIT */

/*
 * kernel/sched.cpp
 *
 * InfOS
 * Copyright (C) University of Edinburgh 2016.  All Rights Reserved.
 *
 * Tom Spink <tspink@inf.ed.ac.uk>
 */
#include <infos/kernel/sched.h>
#include <infos/kernel/sched-entity.h>
#include <infos/kernel/process.h>
#include <infos/kernel/kernel.h>
#include <infos/util/time.h>
#include <infos/util/cmdline.h>
#include <arch/arch.h>
#include <arch/x86/context.h>

using namespace infos::kernel;
using namespace infos::util;

ComponentLog infos::kernel::sched_log(syslog, "sched");

static char sched_algorithm[32];

RegisterCmdLineArgument(SchedAlgorithm, "sched.algorithm") {
	strncpy(sched_algorithm, value, sizeof(sched_algorithm)-1);
}

RegisterCmdLineArgument(SchedDebug, "sched.debug") {
	if (strncmp(value, "1", 1) == 0) {
		sched_log.enable();
	} else {
		sched_log.disable();
	}
}

Scheduler::Scheduler(Kernel& owner) : Subsystem(owner), _active(false), _current(NULL)
{

}

/**
 * The idle task thread proc.  It just spins in a loop, relaxing the processor.
 */
static void idle_task()
{
	for (;;) asm volatile("pause");
}

bool Scheduler::init()
{
	sched_log.message(LogLevel::INFO, "Creating idle process");

	Process *idle_process = new Process("idle", true, (Thread::thread_proc_t)idle_task);
	_idle_entity = &idle_process->main_thread();

	SchedulingAlgorithm *algo = acquire_scheduler_algorithm();
	if (!algo) {
		syslog.messagef(LogLevel::ERROR, "No scheduling algorithm available");
		return false;
	}

	syslog.messagef(LogLevel::IMPORTANT, "*** USING SCHEDULER ALGORITHM: %s", algo->name());

	// Install the discovered algorithm.
	_algorithm = algo;
    _algorithm->init();

	// Set the idle entity to be runnable, and forcibly activate it.  This is so that
	// when interrupts are enabled, the idle thread becomes the context that is saved and restored.
	// We don't call set_entity_state() here, because that would add the idle task to the algorithm
	// runqueue, meaning that the scheduler would schedule the idle task along with the regular tasks.
    _idle_entity->_state = SchedulingEntityState::RUNNABLE;
	idle_process->main_thread().activate(NULL);

	return true;
}

void Scheduler::run()
{
	// This is now the point of no return.  Once the scheduler is activated, it will schedule the first
	// eligible process.  Which may or may not be the idle task.  But, non-scheduled control-flow will cease
	// to be, and the kernel will only run processes.
	sched_log.message(LogLevel::INFO, "Activating scheduler...");
	_active = true;

	// Enable interrupts
	syslog.messagef(LogLevel::DEBUG, "Enabling interrupts");
	owner().arch().enable_interrupts();

	// From this point onwards, the scheduler is now live.

	// This infinite loop is required for two reasons:
	// 1. The ::run() method is marked as __noreturn, and gcc can figure out if that's really true, and so will complain
	//    if our "noreturn" method actually returns.
	//
	// 2. Once the scheduler is activated, it will only begin scheduling on the next timer tick -- which, of course,
	//    is asynchronous to this control-flow.
	for (;;)
		asm volatile("nop");
}

/**
 * Called during an interrupt to (possibly) switch processes.
 */
void Scheduler::schedule()
{
	if (!_active) return;
	if (!_algorithm) return;

	// Ask the scheduling algorithm for the next process.
	SchedulingEntity *next = _algorithm->pick_next_entity();

	// If the algorithm refused to return a process, then schedule
	// the idle entity.
	if (!next) {
		next = _idle_entity;
	}

	// If the next task to run, is NOT the currently running task...
	if (next != _current) {
		// Activate the next task.
		if (next->activate(_current)) {
			// Update the current task pointer.
			_current = next;

		} else {
			// If the task failed to activate, try and forcibly activate the idle entity.
			if (!_idle_entity->activate(_current)) {
				// We're in big trouble if even the idle thread won't activate.
				arch_abort();
			}

			// Update the current task pointer.
			_current = _idle_entity;
		}
	}

	// Update the execution start time for the task that's about to run.
	_current->update_exec_start_time(owner().runtime());
}

/**
 * Updates process accounting.
 */
void Scheduler::update_accounting()
{
	if (_current) {
		auto now = owner().runtime();

		// Calculate the delta.
		SchedulingEntity::EntityRuntime delta = now - _current->_exec_start_time;

		// Increment the CPU runtime.
		_current->increment_cpu_runtime(delta);

		// Update the exec start time.
		_current->update_exec_start_time(now);
	}
}

/**
 * Changes the state of a scheduling entity.
 * @param entity The scheduling entity being changed.
 * @param state The new state for the entity.
 */
void Scheduler::set_entity_state(SchedulingEntity& entity, SchedulingEntityState::SchedulingEntityState state)
{
	assert(_algorithm);

	// If the state is not being changed -- do nothing.
	if (entity._state == state) return;

	// If the new state is runnable...
	if (state == SchedulingEntityState::RUNNABLE) {
		// Add the entity to the runqueue only if it is transitioning from STOPPED or SLEEPING
		if (entity._state == SchedulingEntityState::STOPPED || entity._state == SchedulingEntityState::SLEEPING) {
			_algorithm->add_to_runqueue(entity);
		}
	} else if (state == SchedulingEntityState::STOPPED || state == SchedulingEntityState::SLEEPING) {
		// Remove the entity from the runqueue only if it is transitioning from RUNNABLE or RUNNING
		if (entity._state == SchedulingEntityState::RUNNABLE || entity._state == SchedulingEntityState::RUNNING) {
			_algorithm->remove_from_runqueue(entity);
		}
	} else if (state == SchedulingEntityState::RUNNING) {
		// The entity can only transition into RUNNING if it is currently RUNNABLE
		assert(entity._state == SchedulingEntityState::RUNNABLE);
	}

	// Record the new state in the entity.
	entity._state = state;
	entity._state_changed.trigger();
}

extern char _SCHED_ALG_PTR_START, _SCHED_ALG_PTR_END;

SchedulingAlgorithm* Scheduler::acquire_scheduler_algorithm()
{
	if (strlen(sched_algorithm) == 0) {
		sched_log.messagef(LogLevel::ERROR, "Scheduling allocation algorithm not chosen on command-line");
		return NULL;
	}

	SchedulingAlgorithm *candidate = NULL;
	SchedulingAlgorithm **schedulers = (SchedulingAlgorithm **)&_SCHED_ALG_PTR_START;

	sched_log.messagef(LogLevel::DEBUG, "Searching for '%s' algorithm...", sched_algorithm);
	while (schedulers < (SchedulingAlgorithm **)&_SCHED_ALG_PTR_END) {
		if (strncmp((*schedulers)->name(), sched_algorithm, sizeof(sched_algorithm)-1) == 0) {
			candidate = *schedulers;
		}

		schedulers++;
	}

	return candidate;
}
