/* SPDX-License-Identifier: MIT */

/*
 * util/string.cpp
 * 
 * InfOS
 * Copyright (C) University of Edinburgh 2016.  All Rights Reserved.
 * 
 * Tom Spink <tspink@inf.ed.ac.uk>
 */
#include <infos/util/string.h>

using namespace infos::util;

size_t infos::util::strlen(const char *str)
{
	if (!str) return 0;
	
	size_t counter = 0;
	
	while (*str++) {
		counter++;
	}
	
	return counter;
}

int infos::util::strncmp(const char *str1, const char *str2, size_t n)
{
	if (strlen(str1) != strlen(str2)) return -1;
	
	while (*str1 && *str2 && n--) {
		if (*str1 != *str2) return -1;
		
		str1++;
		str2++;
	}
	
	return 0;
}

char *infos::util::strncpy(char *dest, const char *src, size_t n)
{
	while (--n && *src) {
		*dest++ = *src++;
	}
	
	// HMMM
	*dest = 0;
	return dest;
}

String infos::util::ToString(unsigned int v)
{
#define BUFFER_SIZE	16
	char buffer[BUFFER_SIZE + 1];
	buffer[BUFFER_SIZE] = 0;
	
	int i = BUFFER_SIZE;
	if (v == 0) {
		buffer[--i] = '0';
	} else {
		while (v > 0 && i >= 0) {
			buffer[--i] = '0' + (v % 10);
			v /= 10;
		}
	}
	
	return String(&buffer[i]);
}

List<String> String::split(char delim, bool remove_empty)
{
	List<String> r;
	
	const char *start = _data;
	String current_part;
	
	while (start < (_data + _size)) {
		if (*start == delim) {
			if (current_part.length() == 0 && remove_empty) {
				continue;
			}
			
			r.append(current_part);
			current_part = "";
		} else {
			current_part = current_part + *start;
		}

		start++;
	}
	
	if (current_part.length() > 0 || !remove_empty) {
		r.append(current_part);
	}
	
	return r;
}
