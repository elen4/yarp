/*
 * Copyright (C) 2013 Istituto Italiano di Tecnologia
 * Authors: Elena Ceseracciu
 * CopyPolicy: Released under the terms of the LGPLv2.1 or later, see LGPL.TXT
 *
 */
#include <set>
#include <string>
#include <fstream>
#include <iostream>

#include <yarp/os/Os.h>
#include <yarp/os/impl/PlatformStdlib.h>
#include "yarpcontextutils.h"
//#include <stdio>

using namespace yarp::os;
using namespace std;

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
        std::cerr << "Could not read from  directory "<< srcDirName <<std::endl;
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

std::vector<std::string> listContentDirs(const ConstString &curPath)
{
    std::vector<std::string> dirsList;
    struct YARP_DIRENT **namelist;
    int n = YARP_scandir(curPath.c_str(),&namelist,NULL,YARP_alphasort);
    if (n<0) {
        return dirsList;
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
                dirsList.push_back(name);
        }
        free(namelist[i]);
    }
    free(namelist);
    return dirsList;
}

void printContentDirs(const ConstString &curPath)
{
    std::vector<std::string> dirsList=listContentDirs(curPath);
    for(std::vector<std::string>::iterator dirsIt=dirsList.begin(); dirsIt!=dirsList.end(); ++dirsIt)
        printf("%s\n",(*dirsIt).c_str());
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

bool recursiveFileList(const char* basePath, const char* suffix, std::set<std::string>& filenames)
{
    string strPath = string (basePath);
    if((strPath.rfind(PATH_SEPERATOR)==string::npos) ||
            (strPath.rfind(PATH_SEPERATOR)!=strPath.size()-1))
            strPath = strPath + string(PATH_SEPERATOR);
    
     string mySuffix=string(suffix);
    
    if(((mySuffix.rfind(PATH_SEPERATOR)==string::npos) ||
            (mySuffix.rfind(PATH_SEPERATOR)!=mySuffix.size()-1)) && mySuffix!="")
            mySuffix = mySuffix + string(PATH_SEPERATOR);
    
    strPath += mySuffix;

    struct YARP_DIRENT **namelist;
    int n = YARP_scandir(strPath.c_str(),&namelist,NULL,YARP_alphasort);
    if (n<0)
    {
        std::cerr << "Could not read from  directory " << strPath <<std::endl;
        return false;
    }
    bool ok=true;
    for (int i=0; i<n; i++)
    {

        ConstString name = namelist[i]->d_name;
        if( name != "." && name != "..")
        {
            if (namelist[i]->d_type == DT_REG)
            {
                filenames.insert(mySuffix+name);
            }
            else
                if(namelist[i]->d_type == DT_DIR)
                {
                    ok=ok && recursiveFileList(basePath, (mySuffix+ name).c_str(), filenames);
                }
        }
        free(namelist[i]);
    }
    free(namelist);
    return ok;
}


int recursiveDiff(yarp::os::ConstString srcDirName, yarp::os::ConstString destDirName, std::ostream &output)
{
    std::set<std::string> srcFileList;
    bool ok = recursiveFileList(srcDirName.c_str(), "", srcFileList);
    std::set<std::string> destFileList;
    ok=ok && recursiveFileList(destDirName.c_str(), "", destFileList);

    if (!ok)
        return -1;
    
    size_t nModifiedFiles=0;
    for(std::set<std::string>::iterator srcIt=srcFileList.begin(); srcIt !=srcFileList.end(); ++srcIt)
    {
        std::set<std::string>::iterator destPos=destFileList.find(*srcIt);
        if (destPos!=destFileList.end())
        {
            
            diff_match_patch<std::string> dmp;
            std::ifstream in((srcDirName+ PATH_SEPERATOR + (*srcIt)).c_str());
            std::string srcStr((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());
            in.close();
            in.open((destDirName+ PATH_SEPERATOR +(*destPos)).c_str());
            std::string destStr((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());
            std::string patchString = dmp.patch_toText(dmp.patch_make(srcStr, destStr));
            if (patchString!= "")
            {
                output << "- " << srcDirName + PATH_SEPERATOR + (*srcIt)<<endl;
                output << "+ " << destDirName + PATH_SEPERATOR + (*destPos) <<endl;
                output << dmp.patch_toText(dmp.patch_make(srcStr, destStr))<<std::endl;
                nModifiedFiles++;
            }
            destFileList.erase(destPos);
        }
        else
        {
            output << "Added file  " << srcDirName+PATH_SEPERATOR +(*srcIt) <<endl;
            nModifiedFiles++;
//             std::ifstream in((srcDirName+PATH_SEPERATOR +(*srcIt)).c_str());
//             std::string srcStr((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());
//             cout << "str size " << srcStr.size() << endl;
//             cout <<srcStr <<endl;
//             output << srcStr<<std::endl;
//             in.close();
        }
    }

    for(std::set<std::string>::iterator destIt=destFileList.begin(); destIt !=destFileList.end(); ++destIt)
    {
        output << "Removed file " << destDirName+PATH_SEPERATOR +(*destIt) <<endl;
        nModifiedFiles++;
//         std::ifstream in((destDirName+PATH_SEPERATOR +(*destIt)).c_str());
//         std::string destStr((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());
//         output << destStr<<std::endl;
//         in.close();

    }

    return nModifiedFiles;//tbm
}

