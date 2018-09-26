/* SPDX-License-Identifier: MIT */

/*
 * include/assert.h
 * 
 * InfOS
 * Copyright (C) University of Edinburgh 2016.  All Rights Reserved.
 * 
 * Tom Spink <tspink@inf.ed.ac.uk>
 */
#pragma once

extern void __assertion_failure(const char *filename, int lineno, const char *expression) __attribute__((noreturn));
#define assert(_expr) do { if (!(_expr)) { __assertion_failure(__FILE__, __LINE__, #_expr); __builtin_unreachable(); } } while(0)

#define not_implemented() assert(false && "NOT IMPLEMENTED")
