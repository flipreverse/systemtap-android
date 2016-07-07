#ifndef __NET_HPP__
#define __NET_HPP__

/*
 * Copyright 2013 Alexander Lochmann, Michael Lenz
 *
 * This file is part of SystemTap4Android.
 *
 * SystemTap4Android is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * SystemTap4Android is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with SystemTap4Android.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "SystemTapMessage.pb.h"

#define DEFAULT_PORT 4711
#define DEFAULT_BUFLEN 1024

using SystemTapMessage::ModuleStatus;

/* The code for sending start, stop or delete is the same, except the concret message type.
 * Hence, a define for each case to a generic parameterized function will do.
 */
#define deleteModule(ip,port,name)	controlModule(ip,port,name,SystemTapMessage::DELETED)
#define startModule(ip,port,name)	controlModule(ip,port,name,SystemTapMessage::RUNNING)
#define stopModule(ip,port,name)	controlModule(ip,port,name,SystemTapMessage::STOPPED)

int sendModule(char *pIP, int pPort, char *pFileName, bool pIgnoreModuleSizeRestriction);
int controlModule(char *pIP, int pPort, char *pName, ModuleStatus pType);
int listModules(char *pIP, int pPort);

#endif
