/* SPDX-License-Identifier: MIT */
#pragma once

#include <infos/define.h>

namespace infos
{
	namespace kernel
	{
		class IRQ
		{
		public:
			typedef void (*irq_handler_t)(const IRQ *irq, void *priv);
			
			IRQ() : _nr(0), _handler(NULL), _priv(NULL) { }
			
			uint32_t nr() const { return _nr; }
			void assign(uint32_t nr) { _nr = nr; }
			
			void attach(irq_handler_t handler, void *priv) { _handler = handler; _priv = priv; }
			
			virtual void handle() const = 0;
			
			virtual void enable()   = 0;
			virtual void disable() = 0;
			
		protected:
			bool invoke() const;
			
		private:
			uint32_t _nr;
			irq_handler_t _handler;
			void *_priv;
		};
	}
}
