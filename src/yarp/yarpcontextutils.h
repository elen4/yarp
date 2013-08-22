/*
 * Copyright (C) 2013 Istituto Italiano di Tecnologia
 * Authors: Elena Ceseracciu
 * CopyPolicy: Released under the terms of the LGPLv2.1 or later, see LGPL.TXT
 *
 */
#ifndef YARPCONTEXTUTILS_H
#define YARPCONTEXTUTILS_H

#include <yarp/os/ConstString.h>
#include <yarp/os/ResourceFinder.h>
#include <iostream>
#include "diff_match_patch.h"

#if defined(WIN32)
    #define PATH_SEPERATOR      "\\"
#else
    #define PATH_SEPERATOR      "/"
#endif

enum folderType{CONTEXTS=0, ROBOTS=1};
int recursiveCopy(yarp::os::ConstString srcDirName, yarp::os::ConstString destDirName);
int recursiveRemove(yarp::os::ConstString dirName);
std::vector<std::string> listContentDirs(const yarp::os::ConstString &curPath);
void printContentDirs(const yarp::os::ConstString &curPath);
void printUserFolders(yarp::os::ResourceFinder &rf, folderType ftype);
void printSysadmFolders(yarp::os::ResourceFinder &rf, folderType ftype);
void printInstalledFolders(yarp::os::ResourceFinder &rf, folderType ftype);
void prepareHomeFolder(yarp::os::ResourceFinder &rf, folderType ftype);
int recursiveDiff(yarp::os::ConstString srcDirName, yarp::os::ConstString destDirName, std::ostream &output=std::cout);

#endif

