/* SPDX-License-Identifier: MIT */

/*
 * drivers/timer/lapic-timer.cpp
 * 
 * InfOS
 * Copyright (C) University of Edinburgh 2016.  All Rights Reserved.
 * 
 * Tom Spink <tspink@inf.ed.ac.uk>
 */
#include <infos/drivers/timer/lapic-timer.h>
#include <infos/drivers/timer/pit.h>
#include <infos/drivers/irq/lapic.h>
#include <infos/drivers/irq/core.h>
#include <infos/kernel/kernel.h>
#include <infos/kernel/irq.h>
#include <infos/kernel/log.h>
#include <infos/util/time.h>
#include <arch/x86/context.h>
#include <arch/x86/irq.h>

using namespace infos::kernel;
using namespace infos::drivers;
using namespace infos::drivers::timer;
using namespace infos::drivers::irq;
using namespace infos::util;

const DeviceClass LAPICTimer::LAPICTimerDeviceClass(Timer::TimerDeviceClass, "lapic-timer");

ComponentLog lapic_timer_log(syslog, "lapic-timer");

/**
 * Constructs a new LAPIC timer instance, given the associated IRQ and the base address of the device.
 * @param irq The IRQ associated with the LAPIC timer
 * @param apic_base The base address of the APIC
 */
LAPICTimer::LAPICTimer() : _frequency(0)
{
}

void LAPICTimer::set_lapic_ptr(drivers::irq::LAPIC *lapic) {
    _lapic = lapic;
}

/**
 * Calibrate the LAPIC timer by measuring its tick rate with respect to a known tick rate.
 * @return Returns TRUE if the calibration suceeded, or FALSE otherwise.
 */
bool LAPICTimer::calibrate()
{
	PIT *pit;
	
	// Attempt to lookup the PIT (programmable interrupt timer), so that we have a
	// known timing base to measure the LAPIC.
	if (!sys.device_manager().try_get_device_by_class(PIT::PITDeviceClass, pit)) {
		lapic_timer_log.message(LogLevel::ERROR, "LAPIC requires the PIT");
		return false;
	}

	// Some useful constants for the calibration
	#define FACTOR					1000
	#define PIT_FREQUENCY			(1193180)
	#define CALIBRATION_PERIOD		(10)
	#define CALIBRATION_TICKS		(uint16_t)((PIT_FREQUENCY * CALIBRATION_PERIOD) / FACTOR)

	lapic_timer_log.messagef(LogLevel::DEBUG, "calibration ticks=%d", (uint32_t)CALIBRATION_TICKS);
	
	// Initialise the LAPIC and the PIT for one-shot operation.
//    pit->lock();
    this->init_oneshot(0xffffffff);
	pit->init_oneshot(CALIBRATION_TICKS);		// 10ms
	
	// Start the PIT and the LAPIC
	pit->start();
	this->start();
	
	// Spin until the PIT expires
	while (!pit->expired());
	
	// Stop the LAPIC
	this->stop();

	// Stop the PIT
	pit->stop();
//	pit->unlock();

	// Calculate the number of ticks per period (accounting for the LAPIC division)
	uint32_t ticks_per_period = (0xffffffff - count());
	ticks_per_period <<= 4;
	
	lapic_timer_log.messagef(LogLevel::DEBUG, "ticks-per-period=%u", ticks_per_period);

    // Determine the LAPIC base frequency
    _frequency = (ticks_per_period * (FACTOR/CALIBRATION_PERIOD));

	lapic_timer_log.messagef(LogLevel::DEBUG, "frequency=%lu", _frequency);
	
	return true;
}

/**
 * Initialises the LAPIC timer device
 * @param dm The device manager that manages this device.
 * @return Returns TRUE if the LAPIC timer initialised successfully, FALSE otherwise.
 */
bool LAPICTimer::init(kernel::DeviceManager& dm)
{
	_irq = &_lapic->timer_irq();
    _irq->attach(lapic_timer_irq_handler, this);

    // Initialise the timer controls
	_lapic->set_timer_divide(3);	
	_lapic->set_timer_initial_count(1);

	// Calibrate the timer.
	return calibrate();
}

/**
 * Starts the timer running.
 */
void LAPICTimer::start()
{
	_irq->enable();
}

/**
 * Stops the timer running.
 */
void LAPICTimer::stop()
{
	_irq->disable();
}

/**
 * Stops and resets the timer.
 */
void LAPICTimer::reset()
{
	_irq->disable();
	_lapic->set_timer_initial_count(0);
}

/**
 * Initialises the timer for one-shot mode.
 * @param period The duration of the one-shot timer.
 */
void LAPICTimer::init_oneshot(uint64_t period)
{
	reset();
	
	_lapic->set_timer_one_shot();
	_lapic->set_timer_initial_count(period);
}

/**
 * Initialises the timer for periodic mode.
 * @param period The duration of the period.
 */
void LAPICTimer::init_periodic(uint64_t period)
{
	reset();
	
	_lapic->set_timer_periodic();
	_lapic->set_timer_initial_count(period);
}

/**
 * Returns the raw counter value for the timer.
 * @return The raw counter value for the timer.
 */
uint64_t LAPICTimer::count() const
{
	return _lapic->get_timer_current_count();
}

/**
 * Determines whether or not the timer has expired.
 * @return 
 */
bool LAPICTimer::expired() const
{
	// TODO: Implement
//	return _lapic->get_timer_current_count() == 0;
	return false;
}

/**
 * The IRQ handler for the LAPIC timer.
 * @param nr The IRQ number that occurred.
 * @param priv IRQ specific data.  In this case, a pointer to the LAPIC timer device object.
 */
void LAPICTimer::lapic_timer_irq_handler(const IRQ *irq, void* priv)
{
	// HACK HACK HACK -- this shouldn't be hard-coded in
	Scheduler& sched = Core::get_current_core()->get_scheduler();

    // Only have the BSP update the system runtime
    if (Core::get_current_core()->get_state() == irq::Core::core_state::BOOTSTRAP) {
        sys.update_runtime(DurationCast<Nanoseconds>(Milliseconds(10)));		// Tell the kernel to update its internal runtime with +10mS
    }

	sched.update_accounting();		// Tell the scheduler to update process accounting
	sched.schedule();					// Cause a scheduling event to occur
}
