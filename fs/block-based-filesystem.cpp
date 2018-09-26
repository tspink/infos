/* SPDX-License-Identifier: MIT */

/*
 * fs/block-based-filesystem.cpp
 * 
 * InfOS
 * Copyright (C) University of Edinburgh 2016.  All Rights Reserved.
 * 
 * Tom Spink <tspink@inf.ed.ac.uk>
 */
#include <infos/fs/block-based-filesystem.h>

using namespace infos::fs;

BlockBasedFilesystem::BlockBasedFilesystem(drivers::block::BlockDevice& bdev) : _bdev(bdev)
{

}
