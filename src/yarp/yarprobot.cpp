// -*- mode:C++; tab-width:4; c-basic-offset:4; indent-tabs-mode:nil -*-

/*
 * Copyright (C) 2013 Istituto Italiano di Tecnologia
 * Authors: Elena Ceseracciu
 * CopyPolicy: Released under the terms of the LGPLv2.1 or later, see LGPL.TXT
 *
 */


#include <yarp/os/Property.h>
#include <yarp/os/ResourceFinder.h>
#include <yarp/os/ResourceFinderOptions.h>
#include <yarp/os/Bottle.h>
#include <yarp/os/Os.h>
#include <yarp/os/impl/PlatformStdlib.h>
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>

#include "yarpcontextutils.h"

#if defined(WIN32)
    #define PATH_SEPERATOR      "\\"
#else
    #define PATH_SEPERATOR      "/"
#endif

using namespace yarp::os;

#ifndef WIN32
std::string readlink_malloc (const char *filename)
{
    int size = 100;
    char *buffer = NULL;
    while (1)
    {
        buffer = (char *) realloc (buffer, size);
        int nchars = readlink (filename, buffer, size);
        if (nchars < 0)
        {
            free (buffer);
            return std::string("");
        }
        if (nchars < size)
            return std::string(buffer).substr(0, nchars);
        size *= 2;
    }
}
#endif


std::string extractRobotNameFromPath(const std::string& fullpath)
{
    size_t n = fullpath.rfind('/');
    if (n == std::string::npos) 
    {
        n = fullpath.rfind('\\');
        if (n != std::string::npos)
            return fullpath.substr(n+2);
        else return std::string("");
    }
    else
    {
        if (n != std::string::npos)
            return fullpath.substr(n+1);
        else return std::string("");
    }
}

std::string extractPath(const std::string& fullpath)
{
    size_t n = fullpath.rfind('/');
    if (n == std::string::npos) 
    {
        n = fullpath.rfind('\\');
    }
    if (n != std::string::npos)
        return fullpath.substr(0, n);
    else return std::string("");
}

bool removeSymLinkFor(std::string robotPath, bool force=false) 
{
#ifndef WIN32
    //force should be true if the symlink should be removed even when it points to a different robot than robotname
    std::string dirPath=extractPath(robotPath);
    std::string defaultPath=dirPath + PATH_SEPERATOR + "default";
    printf("dirpath: %s defPath %s\n", dirPath.c_str() , defaultPath.c_str() );
    if (force)
    {
        return !YARP_unlink(defaultPath.c_str());
    }
    else
    {
        std::string realpath=readlink_malloc(defaultPath.c_str());
        printf("robotpath %s, realpath: %s \n", robotPath.c_str(), realpath.c_str());
        if (robotPath == defaultPath || realpath==robotPath)
            return !YARP_unlink(defaultPath.c_str());
    }
#endif
    return false;
}

int findCurrentRobot(bool verbose)
{
    yarp::os::ResourceFinder rf;
    if (verbose)
        rf.setVerbose(true);
    //check environment variable
    const char *result = ACE_OS::getenv("YARP_ROBOT_NAME");
    if (result != NULL)
    {
        ConstString robotpath = rf.findPath(ConstString("robots") + PATH_SEPERATOR + ConstString(result));
        printf("Current robot is %s, identified by the environment variable YARP_ROBOT_NAME\n", result);
        printf("Robot location: %s\n", robotpath.c_str());
        return 0;
    }

#if defined(WIN32)
    printf("Could not determine which is the current robot as the current OS does not support symbolic links, please set the YARP_ROBOT_NAME environment variable to set the current robot\n");
#else
    ConstString robotpath=rf.findPath((ConstString("robots") + PATH_SEPERATOR + "default").c_str());
    if (robotpath == "")
    {
        printf("Could not determine which is the current robot\n");
        return 0;
    }
    std::string realrobotname= readlink_malloc(robotpath.c_str());
    if (realrobotname != "")
    {
        printf("Current robot is %s\n", extractRobotNameFromPath(realrobotname).c_str());
        printf("Current robot location is %s\n", realrobotname.c_str());
    }
    else
    {
        printf("No current robot\n");
    }
#endif
    return 0;
}

int makeCurrentRobot(std::string robotName, bool verbose)
{
                //check environment variable
            const char *result = ACE_OS::getenv("YARP_ROBOT_NAME");
            if (result != NULL)
            {
                printf("YARP_ROBOT_NAME environment variable is set to %s, which identifies the current robot\n", result);
#ifdef WIN32
                printf("Please modify the environment variable to change the current robot");
#else
                printf("Please unset it if you want to change the current robot with 'yarp robot'");
#endif
                return 0;
            }
#if defined(WIN32)
            printf("Cannot set the current robot as the current OS does not support symbolic links, please set the YARP_ROBOT_NAME environment variable to set the current robot\n");
            return 0;
#else
            yarp::os::ResourceFinder rf;
            if (verbose)
                rf.setVerbose(true);
//                         ResourceFinderOptions opts;
//             opts.searchLocations=ResourceFinderOptions::User;
            ConstString robotpath=rf.findPath((ConstString("robots") + PATH_SEPERATOR + robotName).c_str()/*, opts*/);
            if (robotpath=="")
            {
                printf("ERROR: Could not find a directory for robot %s\n", robotName.c_str());
                return 0;
            }
             std::string dirPath=extractPath(robotpath);
             std::string defaultPath=dirPath + PATH_SEPERATOR + "default";
//             YARP_unlink(defaultPath);
            removeSymLinkFor(robotpath, true);
            int notok=symlink (robotpath.c_str(), defaultPath.c_str());
            if (notok)
                printf("ERROR: An error occurred while creating symbolic link from %s to %s", robotpath.c_str(), defaultPath.c_str());
            else
            {
                findCurrentRobot(verbose);
                //printf("Current robot is %s\n", robotName.c_str());
                //printf("Current robot location is %s\n", robotpath.c_str());
            }
            return 0;

#endif
}
int import(std::string robotName, bool verbose)
{
    yarp::os::ResourceFinder rf;
    if (verbose)
        rf.setVerbose(true);
    ResourceFinderOptions opts;
    opts.searchLocations=ResourceFinderOptions::Installed;
    ConstString originalpath=rf.findPath((ConstString("robots") + PATH_SEPERATOR +robotName).c_str(), opts);
    ConstString destDirname=rf.getDataHome() + PATH_SEPERATOR + "robots" + PATH_SEPERATOR + robotName;

    prepareHomeFolder(rf, ROBOTS);

    int result= recursiveCopy(originalpath, destDirname);
    if (result < 0)
        printf("ERRORS OCCURRED WHILE IMPORTING ROBOT %s\n", robotName.c_str());
    else
    {
        printf("Copied directory for robot %s from %s to %s .\nCurrent locations for this robot:\n", robotName.c_str(), originalpath.c_str(), destDirname.c_str());
        yarp::os::Bottle paths=rf.findPaths((ConstString("robots") + PATH_SEPERATOR +robotName).c_str());
        for (int curCont=0; curCont<paths.size(); ++curCont)
            printf("%s\n", paths.get(curCont).asString().c_str());
    }

    return result;
}

int importSymLinkToHome(bool verbose)
{
#ifndef WIN32
    ResourceFinderOptions opts;
    opts.searchLocations=ResourceFinderOptions::Installed;
    yarp::os::ResourceFinder rf;
    if (verbose)
        rf.setVerbose(true);

    ConstString defaultPath=rf.findPath(std::string("robots") + PATH_SEPERATOR + "default", opts);
    std::string realDefaultPath=readlink_malloc(defaultPath.c_str());
    std::string robotName= extractRobotNameFromPath(realDefaultPath);

    opts.searchLocations=ResourceFinderOptions::User;
    if (rf.findPath(std::string("robots") + PATH_SEPERATOR + robotName, opts) == "")
        import(robotName, verbose);

    return makeCurrentRobot(robotName, verbose);
#endif
    return 1;
}

void show_yarprobot_help() {
    printf("Usage: yarp-robot [OPTION]\n\n");
    printf("Known values for OPTION are:\n\n");
    printf("  --help       display this help and exit\n");
    printf("  --list  list robots that are available; add optional '--user', '--sysadm' or '--installed' parameters to limit the search locations\n");
    printf("  --current <robot_name> print name and location of current robot in use if no robotname is provided; otherwise, make <robot_name> the current robot\n");
    printf("  --import <robot_name>  import specified robot to home directory\n");
    printf("  --import-all import all robots to home directory\n");
    printf("  --remove  <robot_name>  remove specified robot from home directory\n");
    printf("  --diff  <robot_name>  find differences from the robot in the home directory with respect to the installation directory\n");
    printf("  --diff-list  list the robots in the home directory that are different from the installation directory\n");
    printf("  --where  <robot_name>  print full paths to the robots that are found for <robot_name> (the first one is the default one)\n");
    printf("  --merge  <robot_name>  merge differences\n");
    printf("\n");

}

int yarp_robot_main(int argc, char *argv[]) {
    yarp::os::Property options;
    options.fromCommand(argc,argv);
    if (options.check("help")) {
        show_yarprobot_help();
        return 0;
    }
    if (options.check("list")) {
        yarp::os::ResourceFinder rf;
        if (options.check("verbose"))
            rf.setVerbose(true);
        if(options.check("user") || options.check("sysadm") || options.check("installed"))
        {
            if (options.check("user"))
                printUserFolders(rf, ROBOTS);
            if (options.check("sysadm"))
                printSysadmFolders(rf, ROBOTS);
            if (options.check("installed"))
                printInstalledFolders(rf, ROBOTS);
        }
        else
        {
            printUserFolders(rf, ROBOTS);
            printSysadmFolders(rf, ROBOTS);
            printInstalledFolders(rf, ROBOTS);
        }
        return 0;
    }

    if (options.check("current"))
    {
        ConstString robotName=options.find("current").asString().c_str();
         bool verbose=options.check("verbose");
        if (robotName=="") //find current robot
        {
            return findCurrentRobot(verbose);
        }
        else //set current robot
        {
            return makeCurrentRobot(robotName, verbose);
        }

    }
    if(options.check("import"))
    {
        ConstString robotName=options.find("import").asString().c_str();
        bool verbose=options.check("verbose");
        if (robotName=="")
        {
            printf("No robot name provided\n");
            return 0;
        }
        if (robotName=="default")
        {
           return importSymLinkToHome(verbose);
        }

        return import(robotName, verbose);

    }

    if(options.check("import-all"))
    {

        yarp::os::ResourceFinder rf;
        bool verbose=options.check("verbose");
        if (verbose)
            rf.setVerbose(true);

        prepareHomeFolder(rf, ROBOTS);
        ResourceFinderOptions opts;
        opts.searchLocations=ResourceFinderOptions::Installed;
        yarp::os::Bottle contextPaths=rf.findPaths("robots", opts);
        for (int curPathId=0; curPathId<contextPaths.size(); ++curPathId)
        {

            ConstString curPath= contextPaths.get(curPathId).toString();

            struct YARP_DIRENT **namelist;
            int n = YARP_scandir(curPath.c_str(),&namelist,NULL,YARP_alphasort);
            if (n<0) {
               continue;
            }
            for (int i=0; i<n; i++) {

                ConstString name = namelist[i]->d_name;

                if( name != "." && name != ".." && name !="default")
                {
                    ACE_stat statbuf;
                    ConstString originalpath=curPath + PATH_SEPERATOR + name;
                    YARP_stat(originalpath.c_str(), &statbuf);
                    if ((statbuf.st_mode & S_IFMT)== S_IFDIR)
                    {
                        ConstString destDirname=rf.getDataHome() + PATH_SEPERATOR + "robots" + PATH_SEPERATOR + name;
                        recursiveCopy(originalpath, destDirname);// TODO: check result!
                    }
                }
                free(namelist[i]);
            }
            free(namelist);
        }


//         ConstString defaultPath=rf.findPath(std::string("robots") + PATH_SEPERATOR + "default");
//         std::string realDefaultPath=readlink_malloc(defaultPath.c_str());
//         std::string robotName= extractRobotNameFromPath(realDefaultPath);
//
//         options.put("current", yarp::os::Value(robotName));
//         makeCurrentRobot(options);
        importSymLinkToHome(verbose);
        printf("New user robots:\n");
        printContentDirs(rf.getDataHome() + PATH_SEPERATOR + "robots");
        return 0;
    }

    if(options.check("remove"))
    {

        ConstString contextName=options.find("remove").asString().c_str();
        if (contextName=="")
        {
            printf("No robot name provided\n");
            return 0;
        }
        yarp::os::ResourceFinder rf;
        if (options.check("verbose"))
            rf.setVerbose(true);
        ResourceFinderOptions opts;
        opts.searchLocations=ResourceFinderOptions::User;
        ConstString targetPath=rf.findPath((ConstString("robots") + PATH_SEPERATOR +contextName).c_str(), opts);
        if (targetPath=="")
        {
            printf("Could not find robot %s !\n", contextName.c_str());
            return 0;
        }
        else
        {
            char choice='n';
            printf("Are you sure you want to remove this folder: %s ? (y/n): ", targetPath.c_str());
            scanf("%c",&choice);
            if (choice=='y')
            {
                int result=0;
                if(contextName != "default")
                {
                    result= recursiveRemove(targetPath.c_str());
                    if (result < 0)
                        printf("ERRORS OCCURRED WHILE REMOVING %s\n", targetPath.c_str());
                    else
                        printf("Removed folder %s\n", targetPath.c_str());
                }
                removeSymLinkFor(targetPath);
                return result;
            }
            else
            {
                printf("Skipped\n");
                return 0;
            }
        }
    }

    if(options.check("where"))
    {
        ConstString contextName=options.find("where").asString().c_str();
        if (contextName=="")
        {
            printf("No context name provided\n");
            return 0;
        }
        yarp::os::ResourceFinder rf;
        if (options.check("verbose"))
            rf.setVerbose(true);
        yarp::os::Bottle paths=rf.findPaths((ConstString("robots") + PATH_SEPERATOR +contextName).c_str());
        for (int curCont=0; curCont<paths.size(); ++curCont)
            printf("%s\n", paths.get(curCont).asString().c_str());
        return 0;
    }
    if(options.check("diff"))
    {
        printf("Not implemented yet\n");
        return 0;
    }
    if(options.check("diff-list"))
    {
        printf("Not implemented yet\n");
        return 0;
    }
    if(options.check("merge"))
    {
        printf("Not implemented yet\n");
        return 0;
    }
    show_yarprobot_help();
    return 1;
}

