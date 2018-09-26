/* SPDX-License-Identifier: MIT */

/*
 * util/cmdline.cpp
 * 
 * InfOS
 * Copyright (C) University of Edinburgh 2016.  All Rights Reserved.
 * 
 * Tom Spink <tspink@inf.ed.ac.uk>
 */
#include <infos/util/cmdline.h>
#include <infos/kernel/log.h>
#include <infos/util/string.h>

using namespace infos::util;
using namespace infos::kernel;

extern char _CMDLINE_ARGS_START, _CMDLINE_ARGS_END;

bool CommandLine::parse(const char* cmdline)
{
	char setting[128];
	
	int i = 0;
	while (*cmdline) {
		while (*cmdline && *cmdline == ' ') cmdline++;
		while (*cmdline && *cmdline != ' ') setting[i++] = *cmdline++;
		setting[i] = 0;
		
		if (i == 0) break;
		
		process(setting);
		i = 0;
	}
	
	return true;
}

void CommandLine::process(const char* setting)
{
	char key[64];
	char value[64];
	
	int i = 0;
	while (*setting && *setting != '=' && i < 63) {
		key[i++] = *setting++;
	}
	key[i] = 0;
	
	i = 0;
	while (*setting && i < 63) {
		if (*setting == '=') { setting++; continue; }
		value[i++] = *setting++;
	}
	value[i] = 0;
	
	auto reg = find_registration(key);
	if (reg) {
		reg->fn(value);
	}
}

const CommandLineArgumentRegistration* CommandLine::find_registration(const char* key)
{
	const CommandLineArgumentRegistration *rgn = (const CommandLineArgumentRegistration *)&_CMDLINE_ARGS_START;
	
	while (rgn < (const CommandLineArgumentRegistration *)&_CMDLINE_ARGS_END) {
		if (!rgn->match) {
			rgn++;
			continue;
		}
		
		if (strncmp(rgn->match, key, 256) == 0) return rgn;
		rgn++;
	}
	
	return NULL;
}
