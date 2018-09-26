/* SPDX-License-Identifier: MIT */

/*
 * drivers/controller.cpp
 * 
 * InfOS
 * Copyright (C) University of Edinburgh 2016.  All Rights Reserved.
 * 
 * Tom Spink <tspink@inf.ed.ac.uk>
 */
#include <infos/drivers/controller.h>

using namespace infos::drivers;

Controller::Controller(kernel::DeviceManager& dm) : _dm(dm)
{

}

Controller::~Controller()
{

}
