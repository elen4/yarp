// -*- mode:C++; tab-width:4; c-basic-offset:4; indent-tabs-mode:nil -*-

/*
 * Copyright (C) 2010 RobotCub Consortium
 * Authors: Paul Fitzpatrick
 * CopyPolicy: Released under the terms of the LGPLv2.1 or later, see LGPL.TXT
 *
 */

#include "XXXCarrier.h"
#include <yarp/os/SharedLibraryClass.h>

bool XXXCarrier::sendHeader(Protocol& proto) {
    // Send the "magic number" for this carrier
    ManagedBytes header(8);
    getHeader(header.bytes());
    proto.os().write(header.bytes());
    if (!proto.os().isOk()) return false;

    // Now we can do whatever we want, as long as somehow
    // we also send the name of the originating port

    // let's just send the port name in plain text terminated with a
    // carriage-return / line-feed
    String from = proto.getRoute().getFromName();
    Bytes b2((char*)from.c_str(),from.length());
    proto.os().write(b2);
    proto.os().write('\r');
    proto.os().write('\n');
    proto.os().flush();
    return proto.os().isOk();
}

YARP_DEFINE_SHARED_SUBCLASS(xxx_carrier, XXXCarrier, Carrier)
