/* SPDX-License-Identifier: MIT */

/*
 * arch/x86/irq.cpp
 * 
 * InfOS
 * Copyright (C) University of Edinburgh 2016.  All Rights Reserved.
 * 
 * Tom Spink <tspink@inf.ed.ac.uk>
 */
#include <arch/x86/irq.h>
#include <arch/x86/irq-entry-points.h>
#include <arch/x86/dt.h>
#include <arch/x86/init.h>
#include <arch/x86/x86-arch.h>
#include <infos/kernel/log.h>

using namespace infos::arch;
using namespace infos::arch::x86;
using namespace infos::kernel;

// An array containing pointers to the IRQ entry-point functions
static irq_entry_point_t irq_entry_points[] = {
	__irq0,		__irq1,		__irq2,		__irq3,		__irq4,
	__irq5,		__irq6,		__irq7,		__irq8,		__irq9,
	__irq10, 	__irq11, 	__irq12, 	__irq13, 	__irq14, 
	__irq15, 	__irq16, 	__irq17, 	__irq18, 	__irq19, 
	__irq20, 	__irq21, 	__irq22, 	__irq23, 	__irq24, 
	__irq25, 	__irq26, 	__irq27, 	__irq28, 	__irq29, 
	__irq30, 	__irq31, 	__irq32, 	__irq33, 	__irq34, 
	__irq35, 	__irq36, 	__irq37, 	__irq38, 	__irq39, 
	__irq40, 	__irq41, 	__irq42, 	__irq43, 	__irq44, 
	__irq45, 	__irq46, 	__irq47, 	__irq48, 	__irq49, 
	__irq50, 	__irq51, 	__irq52, 	__irq53, 	__irq54, 
	__irq55, 	__irq56, 	__irq57, 	__irq58, 	__irq59, 
	__irq60, 	__irq61, 	__irq62, 	__irq63, 	__irq64, 
	__irq65, 	__irq66, 	__irq67, 	__irq68, 	__irq69, 
	__irq70, 	__irq71, 	__irq72, 	__irq73, 	__irq74, 
	__irq75, 	__irq76, 	__irq77, 	__irq78, 	__irq79, 
	__irq80, 	__irq81, 	__irq82, 	__irq83, 	__irq84, 
	__irq85, 	__irq86, 	__irq87, 	__irq88, 	__irq89, 
	__irq90, 	__irq91, 	__irq92, 	__irq93, 	__irq94, 
	__irq95, 	__irq96, 	__irq97, 	__irq98, 	__irq99, 
	__irq100, 	__irq101, 	__irq102, 	__irq103, 	__irq104, 
	__irq105, 	__irq106, 	__irq107, 	__irq108, 	__irq109, 
	__irq110, 	__irq111, 	__irq112, 	__irq113, 	__irq114, 
	__irq115, 	__irq116, 	__irq117, 	__irq118, 	__irq119, 
	__irq120, 	__irq121, 	__irq122, 	__irq123, 	__irq124, 
	__irq125, 	__irq126, 	__irq127, 	__irq128, 	__irq129, 
	__irq130, 	__irq131, 	__irq132, 	__irq133, 	__irq134, 
	__irq135, 	__irq136, 	__irq137, 	__irq138, 	__irq139, 
	__irq140, 	__irq141, 	__irq142, 	__irq143, 	__irq144, 
	__irq145, 	__irq146, 	__irq147, 	__irq148, 	__irq149, 
	__irq150, 	__irq151, 	__irq152, 	__irq153, 	__irq154, 
	__irq155, 	__irq156, 	__irq157, 	__irq158, 	__irq159, 
	__irq160, 	__irq161, 	__irq162, 	__irq163, 	__irq164, 
	__irq165, 	__irq166, 	__irq167, 	__irq168, 	__irq169, 
	__irq170, 	__irq171, 	__irq172, 	__irq173, 	__irq174, 
	__irq175, 	__irq176, 	__irq177, 	__irq178, 	__irq179, 
	__irq180, 	__irq181, 	__irq182, 	__irq183, 	__irq184, 
	__irq185, 	__irq186, 	__irq187, 	__irq188, 	__irq189, 
	__irq190, 	__irq191, 	__irq192, 	__irq193, 	__irq194, 
	__irq195, 	__irq196, 	__irq197, 	__irq198, 	__irq199, 
	__irq200, 	__irq201, 	__irq202, 	__irq203, 	__irq204, 
	__irq205, 	__irq206, 	__irq207, 	__irq208, 	__irq209, 
	__irq210, 	__irq211, 	__irq212, 	__irq213, 	__irq214, 
	__irq215, 	__irq216, 	__irq217, 	__irq218, 	__irq219, 
	__irq220, 	__irq221, 	__irq222, 	__irq223, 	__irq224, 
	__irq225, 	__irq226, 	__irq227, 	__irq228, 	__irq229, 
	__irq230, 	__irq231, 	__irq232, 	__irq233, 	__irq234, 
	__irq235, 	__irq236, 	__irq237, 	__irq238, 	__irq239, 
	__irq240, 	__irq241, 	__irq242, 	__irq243, 	__irq244, 
	__irq245, 	__irq246, 	__irq247, 	__irq248, 	__irq249, 
	__irq250, 	__irq251, 	__irq252, 	__irq253, 	__irq254, 
	__irq255
};

/**
 * Initialises the IRQ manager.
 * @return Returns TRUE if the IRQ manager was successfully initialised, or FALSE otherwise.
 */
bool IRQManager::init()
{
	// Initialise all IDT entries to their corresponding entry points
	for (unsigned int i = 0; i < MAX_NR_IDT_ENTRIES && i < MAX_IRQS; i++) {
		idt.register_interrupt_gate(i, (uintptr_t)irq_entry_points[i], 0x08, 0);
	}
	
	// Allow TRAP + USER_SYSCALL from userspace, by modifiying the IDT
	// entry to permit invocation from ring 3.
	idt.register_interrupt_gate(IRQ_TRAP, (uintptr_t)irq_entry_points[IRQ_TRAP], 0x08, 3);
	idt.register_interrupt_gate(IRQ_USER_SYSCALL, (uintptr_t)irq_entry_points[IRQ_USER_SYSCALL], 0x08, 3);
	
	// Reload the IDT
	idt.reload();
	
	// Now, each logical IRQ vector has an associated "handler" object, depending on what
	// type of IRQ that particular vector is.  When the IRQ vector is invoked (by whatever)
	// the associated IRQ descriptor is looked up, and that contains the associated
	// handler object, that knows how to dispatch the behaviour.
	
	// The first 32 IRQs are actually exception vectors, so initialise the IRQ handler with an
	// ExceptionIRQ handler object.
	for (unsigned int i = 0; i < 32; i++) {
		// Create the ExceptionIRQ handler object
		IRQ *irq = new ExceptionIRQ();
		
		// Associate the object with the IRQ descriptor, and assign the number.
		irq_descriptors[i].irq(irq);
		irq->assign(i);
	}
	
	return true;
}

/**
 * Installs a particular handler function into an IRQ descriptor, representing the given IRQ
 * vector, associated with a particular handler type.
 * @param nr The IRQ vector number
 * @param handler The handler function
 * @param priv Private data associated with the IRQ
 * @return Returns TRUE if the handler was successfully installed.
 */
template<typename T>
bool IRQManager::install_handler(uint8_t nr, IRQ::irq_handler_t handler, void *priv)
{
	// Make sure the IRQ vector number is in range.
	if (nr >= MAX_IRQS) return false;

	// Take a look at the IRQ handler object associated with this vector number.  If
	// it is NULL, then create a new handler object, and connect it up.
	if (irq_descriptors[nr].irq() == NULL) {
		IRQ *irq = new T();
		irq_descriptors[nr].irq(irq);
		irq->assign(nr);
	}
	
	// Attach the handler function to the IRQ object.
	irq_descriptors[nr].irq()->attach(handler, priv);
	return true;
}

/**
 * Installs a handler function for an exception IRQ.
 * @param nr
 * @param handler
 * @param priv
 * @return 
 */
bool IRQManager::install_exception_handler(uint8_t nr, IRQ::irq_handler_t handler, void *priv)
{
	return install_handler<ExceptionIRQ>(nr, handler, priv);
}

/**
 * Installs a handler function for a software IRQ.
 * @param nr
 * @param handler
 * @param priv
 * @return 
 */
bool IRQManager::install_software_handler(uint8_t nr, IRQ::irq_handler_t handler, void *priv)
{
	return install_handler<SoftwareIRQ>(nr, handler, priv);
}

/**
 * Attaches an IRQ object to the next available IRQ vector.
 * @param irq The IRQ object to hook-up
 * @return Returns TRUE if the IRQ object was attached, and FALSE otherwise.
 */
bool IRQManager::attach_irq(kernel::IRQ* irq)
{
	// The caller MUST supply an IRQ object.
	if (!irq) return false;

	// Starting at 32, and onwards, try to find a free IRQ vector by
	// finding a descriptor that doesn't have an associated IRQ object.
	for (unsigned int i = 0x20; i < 0x100; i++) {
		if (irq_descriptors[i].irq() == NULL) {
			// If one is found, connect the IRQ object to the descriptor,
			// and assign its number.
			irq_descriptors[i].irq(irq);
			irq->assign(i);

			return true;
		}
	}
	
	return false;
}

/**
 * Handles an exception IRQ.
 */
void ExceptionIRQ::handle() const
{
	// Invoke the handler function, but if it failed to invoke, halt the system.
	if (!invoke()) {
		x86_log.messagef(LogLevel::FATAL, "Unhandled Exception %u", nr());
		arch_abort();
	}
}

/**
 * Enables the exception IRQ.
 */
void ExceptionIRQ::enable()
{
	// Exception IRQs cannot be enabled/disabled.
}

/**
 * Disables the exception IRQ.
 */
void ExceptionIRQ::disable()
{
	// Exception IRQs cannot be enabled/disabled.
}

/**
 * Handles a software IRQ.
 */
void SoftwareIRQ::handle() const
{
	// Invoke the handler function, but if it failed to invoke, halt the system.
	if (!invoke()) {
		x86_log.messagef(LogLevel::FATAL, "Unhandled Software IRQ %u", nr());
		arch_abort();
	}
}

/**
 * Enables the software IRQ.
 */
void SoftwareIRQ::enable()
{
	// Software IRQs cannot be enabled/disabled.
	// (TODO: Maybe they can by modifying the IDT -- but do we really want to support this?)
}

/**
 * Disables the software IRQ.
 */
void SoftwareIRQ::disable()
{
	// Software IRQs cannot be enabled/disabled.
	// (TODO: Maybe they can by modifying the IDT -- but do we really want to support this?)
}

/**
 * This is the native IRQ handling function, called after the current context has been saved.
 * It is responsible for dispatching handling functionality to the associated handler object.
 * @param irq_nr
 */
extern "C" void __handle_raw_irq(uint32_t irq_nr)
{
	// Sanity checking.
	assert(irq_nr < MAX_IRQS);
	
	// Lookup the IRQ descriptor for the IRQ vector number, and retrieve the IRQ
	// handler object.
	IRQ *irq = x86arch.irq_manager().get_irq_descriptor(irq_nr)->irq();
	
	// If there was an object... handle the IRQ.
	if (irq) {
		irq->handle();
	} else {
		// No object?  Print out a warning.
		x86_log.messagef(LogLevel::WARNING, "IRQ %u -- but nobody cared", irq_nr);
	}
}
