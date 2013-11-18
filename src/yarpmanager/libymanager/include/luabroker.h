// -*- mode:C++; tab-width:4; c-basic-offset:4; indent-tabs-mode:nil -*-

/*
 *  Yarp Modules Manager
 *  Copyright: (C) 2013 Istituto Italiano di Tecnologia (IIT)
 *  Authors: Ali Paikan <ali.paikan@iit.it>, Elena Ceseracciu
 * 
 *  Copy Policy: Released under the terms of the LGPLv2.1 or later, see LGPL.TXT
 *
 */


#ifndef __LUABROKER__
#define __LUABROKER__

#include "yarpbroker.h" 
#include "localbroker.h"

#include <string>

class LuaLocalBroker: public LocalBroker
{

public: 
     LuaLocalBroker() : LocalBroker() {}
     virtual ~LuaLocalBroker() {}
     bool init(const char* szcmd, const char* szparam,
            const char* szhost, const char* szstdio,
            const char* szworkdir, const char* szenv ) {

            OSTRINGSTREAM strDevParam;
            std::string strParam;
            std::string strCmd;
            if(szcmd) strCmd = szcmd;
            if(szparam) strParam = szparam;
            strDevParam<<strCmd<<" "<<strParam;
            return LocalBroker::init("lua", strDevParam.str().c_str(), 
                                     szhost, szstdio, szworkdir, szenv);
     }
};
 

class LuaYarprunBroker: public YarpBroker
{

public: 
    LuaYarprunBroker() : YarpBroker() {}
     virtual ~LuaYarprunBroker() {}
     bool init(const char* szcmd, const char* szparam,
            const char* szhost, const char* szstdio,
            const char* szworkdir, const char* szenv ) {

            OSTRINGSTREAM strDevParam;
            std::string strParam;
            std::string strCmd;
            if(szcmd) strCmd = szcmd;
            if(szparam) strParam = szparam;
            strDevParam<<strCmd<<" "<<strParam;
            return YarpBroker::init("lua", strDevParam.str().c_str(), 
                                     szhost, szstdio, szworkdir, szenv);
     }
};
 

#endif //__LUABROKER__
