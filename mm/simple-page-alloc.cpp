/* SPDX-License-Identifier: MIT */

/*
 * mm/simple-page-alloc.cpp
 *
 * InfOS
 * Copyright (C) University of Edinburgh 2016.  All Rights Reserved.
 *
 * Tom Spink <tspink@inf.ed.ac.uk>
 */
#include <infos/mm/page-allocator.h>
#include <infos/mm/mm.h>
#include <infos/kernel/kernel.h>
#include <infos/util/list.h>

using namespace infos::kernel;
using namespace infos::mm;
using namespace infos::util;

#define MAX_ORDER 17

/**
 * A simple page allocation algorithm.
 */
class SimplePageAllocator : public PageAllocatorAlgorithm
{
private:
	PageDescriptor *_pgd_base;
	uint64_t _nr_pgds;

public:
	bool init(PageDescriptor *page_descriptors, uint64_t nr_page_descriptors) override
	{
		mm_log.messagef(LogLevel::DEBUG, "Simple Page Allocator online");
		_pgd_base = page_descriptors;
		_nr_pgds = nr_page_descriptors;

		return true;
	}

	PageDescriptor *allocate_pages(int order) override
	{
		const int nr_pages = (1 << order);
		for (uint64_t idx = 0; idx < _nr_pgds; idx++)
		{
			bool found = true;
			for (uint64_t subidx = idx; subidx < idx + nr_pages; subidx++)
			{
				if (_pgd_base[subidx].type != PageDescriptorType::AVAILABLE)
				{
					found = false;
					idx = subidx;
					break;
				}
			}

			if (found)
			{
				return &_pgd_base[idx];
			}
		}

		return NULL;
	}

	void free_pages(PageDescriptor *pgd, int order) override
	{
		PageDescriptor *base = pgd;
		for (unsigned int i = 0; i < ((unsigned int)1 << (unsigned int)order); i++)
		{
			assert(base[i].type == PageDescriptorType::ALLOCATED);
		}
	}

	virtual void insert_page_range(PageDescriptor *start, uint64_t count) override
	{
		mm_log.messagef(LogLevel::DEBUG, "Inserting available page range from %lx -- %lx", sys.mm().pgalloc().pgd_to_pfn(start), sys.mm().pgalloc().pgd_to_pfn(start + count));
	}

	virtual void remove_page_range(PageDescriptor *start, uint64_t count) override
	{
		mm_log.messagef(LogLevel::DEBUG, "Removing available page range from %lx -- %lx", sys.mm().pgalloc().pgd_to_pfn(start), sys.mm().pgalloc().pgd_to_pfn(start + count));
	}

	const char *name() const override { return "simple"; }

	void dump_state() const override
	{
	}
};

RegisterPageAllocator(SimplePageAllocator);
