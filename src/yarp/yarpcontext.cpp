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
#include <errno.h>

#include "yarpcontextutils.h"

#if defined(WIN32)
    #define PATH_SEPERATOR      "\\"
#else
    #define PATH_SEPERATOR      "/"
#endif

using namespace yarp::os;

// int recursiveCopy(ConstString srcDirName, ConstString destDirName)
// {
//     bool ok=true;
//     struct YARP_DIRENT **namelist;
//     int n = YARP_scandir(srcDirName.c_str(),&namelist,NULL,YARP_alphasort);
//     if (n<0)
//     {
//         printf("Could not read from  directory %s\n", srcDirName.c_str());
//         return -1;
//     }
//     if (yarp::os::mkdir((destDirName).c_str()) < 0)
//     {
//         if (errno == EEXIST)
//             printf("Directory %s already exist; remove it first, or use the diff/merge commands\n", destDirName.c_str());
//         else
//             printf("Could not create directory %s\n", destDirName.c_str());
//         return -1;
//     }
//     for (int i=0; i<n; i++)
//     {
// 
//         ConstString name = namelist[i]->d_name;
// 
//         if( name != "." && name != "..")
//         {
//             ConstString srcPath=srcDirName + PATH_SEPERATOR + name;
//             ACE_stat statbuf;
//             if( YARP_stat(srcPath.c_str(), &statbuf) ==-1)
//                 printf("Error in checking properties for %s\n", srcPath.c_str());
//             if ((statbuf.st_mode & S_IFMT)== S_IFDIR)
//                 recursiveCopy(srcPath, destDirName + PATH_SEPERATOR + name);
//             if ((statbuf.st_mode & S_IFMT)== S_IFREG)
//             {
//                 char buf[BUFSIZ];
//                 size_t size;
// 
//                 FILE* source = fopen(srcPath.c_str(), "rb");
//                 if (source == NULL)
//                 {
//                     ok=false;
//                     printf("Could not open source file %s\n", srcPath.c_str());
//                     continue;
//                 }
//                 FILE* dest = fopen((destDirName + PATH_SEPERATOR + name).c_str(), "wb");
//                 if(dest ==NULL)
//                 {
//                     ok=false;
//                     printf("Could not open target file %s\n",(destDirName + PATH_SEPERATOR + name).c_str());
//                     fclose(source);
//                     continue;
//                 }
//                 // clean and more secure
//                 // feof(FILE* stream) returns non-zero if the end of file indicator for stream is set
// 
//                 while ((size = fread(buf, 1, BUFSIZ, source)))
//                 {
//                     fwrite(buf, 1, size, dest);
//                 }
// 
//                 fclose(source);
//                 fclose(dest);
//             }
//         }
//         free(namelist[i]);
//     }
//     free(namelist);
// 
//     return (ok==true ? 0 :-1);
// }
// 
// int recursiveRemove(ConstString dirName)
// {
//     struct YARP_DIRENT **namelist;
//     int n = YARP_scandir(dirName.c_str(),&namelist,NULL,YARP_alphasort);
//     if (n<0)
//     {
//         printf("Could not read from  directory %s\n", dirName.c_str());
//         return yarp::os::rmdir(dirName.c_str()); // TODO check if this is useful...
//     }
// 
//     for (int i=0; i<n; i++)
//     {
//         ConstString name = namelist[i]->d_name;
//         ConstString path=dirName + PATH_SEPERATOR + name;
//         if( name != "." && name != "..")
//         {
//             ACE_stat statbuf;
//             if (YARP_stat(path.c_str(), &statbuf) == -1)
//                 printf("Error in checking properties for %s\n", path.c_str());
//             if ((statbuf.st_mode & S_IFMT)== S_IFDIR)
//                 recursiveRemove(path);
//             if ((statbuf.st_mode & S_IFMT)== S_IFREG)
//             {
//                 YARP_unlink(path.c_str());
//             }
//         }
//         free(namelist[i]);
//     }
//     free(namelist);
// 
//     return yarp::os::rmdir(dirName.c_str());
// };
// 
// void printContentDirs(ConstString curPath)
// {
//     struct YARP_DIRENT **namelist;
//     int n = YARP_scandir(curPath.c_str(),&namelist,NULL,YARP_alphasort);
//     if (n<0) {
//         return;
//     }
//     for (int i=0; i<n; i++) {
// 
//         ConstString name = namelist[i]->d_name;
//         if( name != "." && name != "..")
//         {
//             ACE_stat statbuf;
//             ConstString path=curPath + PATH_SEPERATOR + name;
//             if (YARP_stat(path.c_str(), &statbuf) == -1)
//                 printf("Error in checking properties for %s\n", path.c_str());
// 
//             if ((statbuf.st_mode & S_IFMT)== S_IFDIR)
//                 printf("%s\n",name.c_str());
//         }
//         free(namelist[i]);
//     }
//     free(namelist);
// };
// 
// 
// void printUserContexts(ResourceFinder &rf)
// {
//     ResourceFinderOptions opts;
//     opts.searchLocations=ResourceFinderOptions::User;
//     yarp::os::Bottle contextPaths=rf.findPaths("contexts", opts);
//     printf("**LOCAL USER DATA:\n");
//     for (int curPathId=0; curPathId<contextPaths.size(); ++curPathId)
//     {
//         printf("* Directory : %s\n", contextPaths.get(curPathId).asString().c_str());
//         printContentDirs(contextPaths.get(curPathId).asString());
//     }
// }
// 
// void printSysadmContexts(ResourceFinder &rf)
// {
//     ResourceFinderOptions opts;
//     opts.searchLocations=ResourceFinderOptions::Sysadmin;
//     yarp::os::Bottle contextPaths=rf.findPaths("contexts", opts);
//     printf("**SYSADMIN DATA:\n");
//     for (int curPathId=0; curPathId<contextPaths.size(); ++curPathId)
//     {
//         printf("* Directory : %s\n", contextPaths.get(curPathId).asString().c_str());
//         printContentDirs(contextPaths.get(curPathId).asString());
//     }
// }
// 
// void printInstalledContexts(ResourceFinder &rf)
// {
//     ResourceFinderOptions opts;
//     opts.searchLocations=ResourceFinderOptions::Installed;
//     yarp::os::Bottle contextPaths=rf.findPaths("contexts", opts);
//     printf("**INSTALLED DATA:\n");
//     for (int curPathId=0; curPathId<contextPaths.size(); ++curPathId)
//     {
//         printf("* Directory : %s\n", contextPaths.get(curPathId).asString().c_str());
//         printContentDirs(contextPaths.get(curPathId).asString());
//     }
// }
// 
// void prepareHomeFolder(ResourceFinder &rf)
// {
// 
//     ACE_DIR* dir= YARP_opendir((rf.getDataHome()).c_str());
//     if (dir!=NULL)
//         YARP_closedir(dir);
//     else
//     {
//         yarp::os::mkdir((rf.getDataHome()).c_str());
//     }
// 
//     dir= YARP_opendir((rf.getDataHome() + PATH_SEPERATOR + "contexts").c_str());
//     if (dir!=NULL)
//         YARP_closedir(dir);
//     else
//     {
//         yarp::os::mkdir((rf.getDataHome() + PATH_SEPERATOR + "contexts").c_str());
//     }
// }


void show_help() {
    printf("Usage: yarp-context [OPTION]\n\n");
    printf("Known values for OPTION are:\n\n");
    printf("  --help       display this help and exit\n");
    printf("  --list  list contexts that are available; add optional '--user', '--sysadm' or '--installed' parameters to limit the search locations\n");
//    printf("  --show <context-name>  show files that make up a context, and the location of each\n");
    printf("  --import <context_name> file1 file2 import specified context files to home directory\n");
    printf("  --import-all import all contexts to home directory\n");
    printf("  --remove  <context_name>  remove specified context from home directory\n");
    printf("  --diff  <context_name>  find differences from the context in the home directory with respect to the installation directory\n");
    printf("  --diff-list  list the contexts in the home directory that are different from the installation directory\n");
    //printf("  --where  <context_name>  print full paths to the contexts that are found for <context_name> (the first one is the default one)\n");
    printf("  --merge  <context_name>  file1 file2 ... merge differences in selected files-directories\n");
    printf("\n");

}

int yarp_context_main(int argc, char *argv[]) {
    yarp::os::Property options;
    options.fromCommand(argc,argv);
    if (options.check("help")) {
        show_help();
        return 0;
    }
    if (options.check("list")) {
        yarp::os::ResourceFinder rf;
        if (options.check("verbose"))
            rf.setVerbose(true);
        if(options.check("user") || options.check("sysadm") || options.check("installed"))
        {
            if (options.check("user"))
                printUserFolders(rf, CONTEXTS);
            if (options.check("sysadm"))
                printSysadmFolders(rf, CONTEXTS);
            if (options.check("installed"))
                printInstalledFolders(rf, CONTEXTS);
        }
        else
        {
            printUserFolders(rf, CONTEXTS);
            printSysadmFolders(rf, CONTEXTS);
            printInstalledFolders(rf, CONTEXTS);
        }
        return 0;
    }

    if(options.check("import"))
    {
        Bottle importArg=options.findGroup("import");
        ConstString contextName;
        if (importArg.size() >1 )
            contextName=importArg.get(1).asString().c_str();
        if (contextName=="")
        {
            printf("No context name provided\n");
            return 0;
        }
        yarp::os::ResourceFinder rf;
        if (options.check("verbose"))
            rf.setVerbose(true);
        ResourceFinderOptions opts;
        opts.searchLocations=ResourceFinderOptions::Installed;
        ConstString originalpath=rf.findPath((ConstString("contexts") + PATH_SEPERATOR +contextName).c_str(), opts);
        ConstString destDirname=rf.getDataHome() + PATH_SEPERATOR + "contexts" + PATH_SEPERATOR + contextName;
        //tmp:
        ConstString hiddenDirname=rf.getDataHome() + PATH_SEPERATOR + ".contexts" + PATH_SEPERATOR + contextName;
        prepareHomeFolder(rf, CONTEXTS);
        if (importArg.size() >2 )
        {

            yarp::os::mkdir((destDirname).c_str());
            yarp::os::mkdir((hiddenDirname).c_str());
            bool ok=true;
            for (int i=2; i<importArg.size(); ++i)
            {
                ConstString fileName=importArg.get(i).asString();
                if(fileName != "")
                {
                    ok = (recursiveCopy(originalpath+ PATH_SEPERATOR + fileName, destDirname + PATH_SEPERATOR + fileName) >=0 ) && ok;
                    ok = ok && (recursiveCopy(originalpath+ PATH_SEPERATOR + fileName, hiddenDirname + PATH_SEPERATOR + fileName) >=0 );
                }
            }
            if (ok)
            {
                printf("Copied selected files to %s\n", destDirname.c_str());
                return 0;
            }
            else
            {
                printf("ERRORS OCCURRED WHILE IMPORTING FILES FOR CONTEXT %s\n", contextName.c_str());
                return 1;
            }
        }
        else
        {

            int result= recursiveCopy(originalpath, destDirname);
            recursiveCopy(originalpath, hiddenDirname);

            if (result < 0)
                printf("ERRORS OCCURRED WHILE IMPORTING CONTEXT %s\n", contextName.c_str());
            else
            {
                printf("Copied context %s from %s to %s .\nCurrent locations for this context:\n", contextName.c_str(), originalpath.c_str(), destDirname.c_str());
                yarp::os::Bottle paths=rf.findPaths((ConstString("contexts") + PATH_SEPERATOR +contextName).c_str());
                for (int curCont=0; curCont<paths.size(); ++curCont)
                    printf("%s\n", paths.get(curCont).asString().c_str());
            }
            return result;
        }
    }

    if(options.check("import-all"))
    {

        yarp::os::ResourceFinder rf;
        if (options.check("verbose"))
            rf.setVerbose(true);

        prepareHomeFolder(rf, CONTEXTS);
        ResourceFinderOptions opts;
        opts.searchLocations=ResourceFinderOptions::Installed;
        yarp::os::Bottle contextPaths=rf.findPaths("contexts", opts);
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

                if( name != "." && name != "..")
                {
                    ACE_stat statbuf;
                    ConstString originalpath=curPath + PATH_SEPERATOR + name;
                    YARP_stat(originalpath.c_str(), &statbuf);
                    if ((statbuf.st_mode & S_IFMT)== S_IFDIR)
                    {
                        ConstString destDirname=rf.getDataHome() + PATH_SEPERATOR + "contexts" + PATH_SEPERATOR + name;
                        recursiveCopy(originalpath, destDirname);
                        ConstString hiddenDirname=rf.getDataHome() + PATH_SEPERATOR + ".contexts" + PATH_SEPERATOR + name;
                        recursiveCopy(originalpath, hiddenDirname);// TODO: check result!
                    }
                }
                free(namelist[i]);
            }
            free(namelist);
        }

        printf("New user contexts:\n");
        printContentDirs(rf.getDataHome() + PATH_SEPERATOR + "contexts");
        return 0;
    }

    if(options.check("remove"))
    {

        ConstString contextName=options.find("remove").asString().c_str();
        if (contextName=="")
        {
            printf("No context name provided\n");
            return 0;
        }
        yarp::os::ResourceFinder rf;
        if (options.check("verbose"))
            rf.setVerbose(true);
        ResourceFinderOptions opts;
        opts.searchLocations=ResourceFinderOptions::User;
        ConstString targetPath=rf.findPath((ConstString("contexts") + PATH_SEPERATOR +contextName).c_str(), opts);
        if (targetPath=="")
        {
            printf("Could not find context %s !\n", contextName.c_str());
            return 0;
        }
        else
        {
            char choice='n';
            printf("Are you sure you want to remove this folder: %s ? (y/n): ", targetPath.c_str());
            scanf("%c",&choice);
            if (choice=='y')
            {
                int result= recursiveRemove(targetPath.c_str());
                if (result < 0)
                    printf("ERRORS OCCURRED WHILE REMOVING %s\n", targetPath.c_str());
                else
                    printf("Removed folder %s\n", targetPath.c_str());
                //remove hidden folder:
                ConstString hiddenPath=   rf.findPath((ConstString(".contexts") + PATH_SEPERATOR +contextName).c_str(), opts);
                if (hiddenPath != "")
                    recursiveRemove(hiddenPath.c_str());
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
        yarp::os::Bottle paths=rf.findPaths((ConstString("contexts") + PATH_SEPERATOR +contextName).c_str());
        for (int curCont=0; curCont<paths.size(); ++curCont)
            printf("%s\n", paths.get(curCont).asString().c_str());
        return 0;
    }

//         if(options.check("show"))
//     {
//         ConstString contextName=options.find("show").asString().c_str();
//         if (contextName=="")
//         {
//             printf("No context name provided\n");
//             return 0;
//         }
//         yarp::os::ResourceFinder rf;
//         if (options.check("verbose"))
//             rf.setVerbose(true);
//         yarp::os::Bottle paths=rf.findPaths((ConstString("contexts") + PATH_SEPERATOR +contextName).c_str());
//         std::vector<std::string> fileList;
//         for (int curCont=0; curCont<paths.size(); ++curCont)
//         {
//             std::vector<std::string> newList;
//             fileList.resize(fileList.size()+);
//         }
//             printf("%s\n", paths.get(curCont).asString().c_str());
//         return 0;
//     }

    if(options.check("diff"))
    {
        ConstString contextName=options.find("diff").asString().c_str();
        if (contextName=="")
        {
            printf("No context name provided\n");
            return 0;
        }
        yarp::os::ResourceFinder rf;
        if (options.check("verbose"))
            rf.setVerbose(true);

        ResourceFinderOptions opts;
        opts.searchLocations=ResourceFinderOptions::User;
        ConstString userPath=rf.findPath((ConstString("contexts") + PATH_SEPERATOR +contextName).c_str(), opts);

        opts.searchLocations=ResourceFinderOptions::Installed;
        ConstString installedPath=rf.findPath((ConstString("contexts") + PATH_SEPERATOR +contextName).c_str(), opts);

        recursiveDiff(installedPath, userPath);
        return 0;
    }
    if(options.check("diff-list"))
    {
        yarp::os::ResourceFinder rf;
        if (options.check("verbose"))
            rf.setVerbose(true);
        ResourceFinderOptions opts;
        opts.searchLocations=ResourceFinderOptions::Installed;
        Bottle installedPaths=rf.findPaths(ConstString("contexts").c_str(), opts);
        for(int n=0; n <installedPaths.size(); ++n)
        {
            std::string installedPath=installedPaths.get(n).asString();
            std::vector<std::string> subDirs=listContentDirs(installedPath);
            for (std::vector<std::string>::iterator subDirIt= subDirs.begin(); subDirIt!=subDirs.end(); ++subDirIt)
            {
                ostream tmp(0);
                opts.searchLocations=ResourceFinderOptions::User;
                rf.setQuiet();
                ConstString userPath=rf.findPath((ConstString("contexts") + PATH_SEPERATOR +(*subDirIt)).c_str(), opts);
                if (userPath == "")
                    continue;
                if ( recursiveDiff(installedPath + PATH_SEPERATOR + (*subDirIt), userPath, tmp)>0)
                    std::cout<< (*subDirIt) <<std::endl;
            }
        }

        return 0;
    }
    if(options.check("merge"))
    {
        Bottle mergeArg=options.findGroup("merge");
        ConstString contextName;
        if (mergeArg.size() >1 )
            contextName=mergeArg.get(1).asString().c_str();
        if (contextName=="")
        {
            printf("No context name provided\n");
            return 0;
        }
        yarp::os::ResourceFinder rf;
        if (options.check("verbose"))
            rf.setVerbose(true);

        if (mergeArg.size() >2 )
        {
            for (int i=2; i<mergeArg.size(); ++i)
            {
                ConstString fileName=mergeArg.get(i).asString();
                if(fileName != "")
                {
                    ResourceFinderOptions opts;
                    opts.searchLocations=ResourceFinderOptions::User;
                    ConstString userFileName=rf.findPath((ConstString("contexts") + PATH_SEPERATOR +contextName + PATH_SEPERATOR + fileName).c_str(), opts);

                    ConstString hiddenFileName=rf.findPath((ConstString(".contexts") + PATH_SEPERATOR +contextName+ PATH_SEPERATOR + fileName).c_str(), opts);

                    opts.searchLocations=ResourceFinderOptions::Installed;
                    ConstString installedFileName=rf.findPath((ConstString("contexts") + PATH_SEPERATOR +contextName+ PATH_SEPERATOR + fileName).c_str(), opts);

                    if (userFileName!="" && hiddenFileName != "" && installedFileName !="")
                        fileMerge(installedFileName, userFileName, hiddenFileName);
                    else if (userFileName!=""  && installedFileName !="")
                        printf("Need to use mergetool\n");
                    else
                        printf("Could not merge file %s\n", fileName.c_str());
                }
            }
        }
        else
        {
            ResourceFinderOptions opts;
            opts.searchLocations=ResourceFinderOptions::User;
            ConstString userPath=rf.findPath((ConstString("contexts") + PATH_SEPERATOR +contextName).c_str(), opts);

            ConstString hiddenUserPath=rf.findPath((ConstString(".contexts") + PATH_SEPERATOR +contextName).c_str(), opts);

            opts.searchLocations=ResourceFinderOptions::Installed;
            ConstString installedPath=rf.findPath((ConstString("contexts") + PATH_SEPERATOR +contextName).c_str(), opts);

            recursiveMerge(installedPath, userPath, hiddenUserPath);
        }
        return 0;
    }
    show_help();
    return 1;
}

