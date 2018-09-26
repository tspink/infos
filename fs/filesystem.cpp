/* SPDX-License-Identifier: MIT */

/*
 * fs/filesystem.cpp
 * 
 * InfOS
 * Copyright (C) University of Edinburgh 2016.  All Rights Reserved.
 * 
 * Tom Spink <tspink@inf.ed.ac.uk>
 */
#include <infos/fs/filesystem.h>

using namespace infos::fs;
using namespace infos::kernel;

ComponentLog infos::fs::fs_log(syslog, "fs");