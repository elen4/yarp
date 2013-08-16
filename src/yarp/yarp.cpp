// -*- mode:C++; tab-width:4; c-basic-offset:4; indent-tabs-mode:nil -*-

/*
 * Copyright (C) 2006, 2008 RobotCub Consortium
 * Authors: Paul Fitzpatrick
 * CopyPolicy: Released under the terms of the LGPLv2.1 or later, see LGPL.TXT
 *
 */

#include <stdio.h>
#include <yarp/os/Network.h>
using namespace yarp::os;

#if YARP_USE_PERSISTENT_NAMESERVER
#  include <yarp/yarpserversql/yarpserversql.h>
#endif

#include "yarpcontext.h"
#include "yarprobot.h"

int main(int argc, char *argv[]) {
#if YARP_USE_PERSISTENT_NAMESERVER
    // intercept "yarp server" if needed
    if (argc>=2) {
        if (ConstString(argv[1])=="server") {
            return yarpserver3_main(argc,argv);
        }
    }
#endif
    if (argc>=2) {
        if (ConstString(argv[1])=="context") {
            return yarp_context_main(argc,argv);
        }
        if (ConstString(argv[1])=="robot") {
            return yarp_robot_main(argc,argv);
        }
        if (ConstString(argv[1])=="help") {
            Network::main(argc, argv);
            printf("context      Manage context data\n");
            printf("robot        Manage robot data\n");
            return 0;
        }
    }

    // call the yarp standard companion
    Network yarp;
    return Network::main(argc,argv);
}

