// -*- mode:C++; tab-width:4; c-basic-offset:4; indent-tabs-mode:nil -*-

/*
 * Copyright (C) 2010 RobotCub Consortium
 * Authors: Paul Fitzpatrick
 * CopyPolicy: Released under the terms of the LGPLv2.1 or later, see LGPL.TXT
 *
 */

#include "XXXStream.h"
#include <memory.h>

using namespace yarp::os;
using namespace std;
YARP_SSIZE_T XXXStream::read(const Bytes& b) {
    if (interrupting) { return -1; }
    while (inputCache.size() < (unsigned int)b.length()) {
        std::cout << "*** CHECK OTHER TERMINAL FOR SOMETHING TO TYPE:"
             << std::endl;
        char buf[1000];
        needInterrupt = true;  // should be mutexed, in real implementation
        std::cin.getline(buf,1000);
        needInterrupt = false;
        if (interrupting) { return -1; }
        inputCache += buf;
        inputCache += "\r\n";
        std::cout << "Thank you" << std::endl;
    }
    memcpy(b.get(),inputCache.c_str(),b.length());
    inputCache = inputCache.substr(b.length());
    return b.length();
}

void XXXStream::write(const Bytes& b) {
    outputCache.append(b.get(),b.length());
    while (outputCache.find("\n")!=std::string::npos) {
        size_t idx = outputCache.find("\n");
        string show;
        show.append(outputCache.c_str(),idx);
        std::cout << "*** TYPE THIS ON THE OTHER TERMINAL: " << show << std::endl;
        outputCache = outputCache.substr(idx+1);
        yarp::os::Time::delay(1);
    }
}
