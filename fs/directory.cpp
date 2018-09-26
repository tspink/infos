/* SPDX-License-Identifier: MIT */

/*
 * fs/directory.cpp
 * 
 * InfOS
 * Copyright (C) University of Edinburgh 2016.  All Rights Reserved.
 * 
 * Tom Spink <tspink@inf.ed.ac.uk>
 */
#include <infos/fs/directory.h>
#include <infos/kernel/log.h>

using namespace infos::fs;
using namespace infos::kernel;

bool SimpleDirectory::read_entry(DirectoryEntry& entry)
{
	if (_current_entry < _entries.count()) {
		entry = _entries.at(_current_entry);
		_current_entry++;
		
		return true;
	}
	
	return false;
}

void SimpleDirectory::close()
{
	_entries.clear();
}

void SimpleDirectory::add_entry(const DirectoryEntry& e)
{
	_entries.append(e);
}
