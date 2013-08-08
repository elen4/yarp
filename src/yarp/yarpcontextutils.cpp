/*
 * Copyright (C) 2013 Istituto Italiano di Tecnologia
 * Authors: Elena Ceseracciu
 * CopyPolicy: Released under the terms of the LGPLv2.1 or later, see LGPL.TXT
 *
 */

#include "yarpcontextutils.h"
#include <yarp/os/Os.h>
#include <yarp/os/impl/PlatformStdlib.h>
#include <stdio.h>

using namespace yarp::os;

ConstString getFolderStringName(folderType ftype)
{
    switch(ftype)
    {
        case 0:
            return ConstString("contexts");
        case 1:
            return ConstString("robots");
        default:
            return ConstString("");
    }
}

int recursiveCopy(ConstString srcDirName, ConstString destDirName)
{
    bool ok=true;
    struct YARP_DIRENT **namelist;
    int n = YARP_scandir(srcDirName.c_str(),&namelist,NULL,YARP_alphasort);
    if (n<0)
    {
        printf("Could not read from  directory %s\n", srcDirName.c_str());
        return -1;
    }
    if (yarp::os::mkdir((destDirName).c_str()) < 0)
    {
        if (errno == EEXIST)
            printf("Directory %s already exist; remove it first, or use the diff/merge commands\n", destDirName.c_str());
        else
            printf("Could not create directory %s\n", destDirName.c_str());
        return -1;
    }
    for (int i=0; i<n; i++)
    {

        ConstString name = namelist[i]->d_name;

        if( name != "." && name != "..")
        {
            ConstString srcPath=srcDirName + PATH_SEPERATOR + name;
            ACE_stat statbuf;
            if( YARP_stat(srcPath.c_str(), &statbuf) ==-1)
                printf("Error in checking properties for %s\n", srcPath.c_str());
            if ((statbuf.st_mode & S_IFMT)== S_IFDIR)
                recursiveCopy(srcPath, destDirName + PATH_SEPERATOR + name);
            if ((statbuf.st_mode & S_IFMT)== S_IFREG)
            {
                char buf[BUFSIZ];
                size_t size;

                FILE* source = fopen(srcPath.c_str(), "rb");
                if (source == NULL)
                {
                    ok=false;
                    printf("Could not open source file %s\n", srcPath.c_str());
                    continue;
                }
                FILE* dest = fopen((destDirName + PATH_SEPERATOR + name).c_str(), "wb");
                if(dest ==NULL)
                {
                    ok=false;
                    printf("Could not open target file %s\n",(destDirName + PATH_SEPERATOR + name).c_str());
                    fclose(source);
                    continue;
                }
                // clean and more secure
                // feof(FILE* stream) returns non-zero if the end of file indicator for stream is set

                while ((size = fread(buf, 1, BUFSIZ, source)))
                {
                    fwrite(buf, 1, size, dest);
                }

                fclose(source);
                fclose(dest);
            }
        }
        free(namelist[i]);
    }
    free(namelist);

    return (ok==true ? 0 :-1);
}

int recursiveRemove(ConstString dirName)
{
    struct YARP_DIRENT **namelist;
    int n = YARP_scandir(dirName.c_str(),&namelist,NULL,YARP_alphasort);
    if (n<0)
    {
        printf("Could not read from  directory %s\n", dirName.c_str());
        return yarp::os::rmdir(dirName.c_str()); // TODO check if this is useful...
    }

    for (int i=0; i<n; i++)
    {
        ConstString name = namelist[i]->d_name;
        ConstString path=dirName + PATH_SEPERATOR + name;
        if( name != "." && name != "..")
        {
            ACE_stat statbuf;
            if (YARP_stat(path.c_str(), &statbuf) == -1)
                printf("Error in checking properties for %s\n", path.c_str());
            if ((statbuf.st_mode & S_IFMT)== S_IFDIR)
                recursiveRemove(path);
            if ((statbuf.st_mode & S_IFMT)== S_IFREG)
            {
                YARP_unlink(path.c_str());
            }
        }
        free(namelist[i]);
    }
    free(namelist);

    return yarp::os::rmdir(dirName.c_str());
};

void printContentDirs(ConstString curPath)
{
    struct YARP_DIRENT **namelist;
    int n = YARP_scandir(curPath.c_str(),&namelist,NULL,YARP_alphasort);
    if (n<0) {
        return;
    }
    for (int i=0; i<n; i++) {

        ConstString name = namelist[i]->d_name;
        if( name != "." && name != "..")
        {
            ACE_stat statbuf;
            ConstString path=curPath + PATH_SEPERATOR + name;
            if (YARP_stat(path.c_str(), &statbuf) == -1)
                printf("Error in checking properties for %s\n", path.c_str());

            if ((statbuf.st_mode & S_IFMT)== S_IFDIR)
                printf("%s\n",name.c_str());
        }
        free(namelist[i]);
    }
    free(namelist);
};


void printUserFolders(yarp::os::ResourceFinder &rf, folderType ftype)
{
    ResourceFinderOptions opts;
    opts.searchLocations=ResourceFinderOptions::User;
    yarp::os::Bottle contextPaths=rf.findPaths(getFolderStringName(ftype), opts);
    printf("**LOCAL USER DATA:\n");
    for (int curPathId=0; curPathId<contextPaths.size(); ++curPathId)
    {
        printf("* Directory : %s\n", contextPaths.get(curPathId).asString().c_str());
        printContentDirs(contextPaths.get(curPathId).asString());
    }
}

void printSysadmFolders(yarp::os::ResourceFinder &rf, folderType ftype)
{
    ResourceFinderOptions opts;
    opts.searchLocations=ResourceFinderOptions::Sysadmin;
    yarp::os::Bottle contextPaths=rf.findPaths(getFolderStringName(ftype), opts);
    printf("**SYSADMIN DATA:\n");
    for (int curPathId=0; curPathId<contextPaths.size(); ++curPathId)
    {
        printf("* Directory : %s\n", contextPaths.get(curPathId).asString().c_str());
        printContentDirs(contextPaths.get(curPathId).asString());
    }
}

void printInstalledFolders(yarp::os::ResourceFinder &rf, folderType ftype)
{
    ResourceFinderOptions opts;
    opts.searchLocations=ResourceFinderOptions::Installed;
    yarp::os::Bottle contextPaths=rf.findPaths(getFolderStringName(ftype), opts);
    printf("**INSTALLED DATA:\n");
    for (int curPathId=0; curPathId<contextPaths.size(); ++curPathId)
    {
        printf("* Directory : %s\n", contextPaths.get(curPathId).asString().c_str());
        printContentDirs(contextPaths.get(curPathId).asString());
    }
}

void prepareHomeFolder(yarp::os::ResourceFinder &rf, folderType ftype)
{
    ACE_DIR* dir= YARP_opendir((rf.getDataHome()).c_str());
    if (dir!=NULL)
        YARP_closedir(dir);
    else
    {
        yarp::os::mkdir((rf.getDataHome()).c_str());
    }

    dir= YARP_opendir((rf.getDataHome() + PATH_SEPERATOR + getFolderStringName(ftype)).c_str());
    if (dir!=NULL)
        YARP_closedir(dir);
    else
    {
        yarp::os::mkdir((rf.getDataHome() + PATH_SEPERATOR + getFolderStringName(ftype)).c_str());
    }
}