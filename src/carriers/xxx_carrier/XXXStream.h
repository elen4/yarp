// -*- mode:C++; tab-width:4; c-basic-offset:4; indent-tabs-mode:nil -*-

/*
 * Copyright (C) 2010 RobotCub Consortium
 * Authors: Paul Fitzpatrick
 * CopyPolicy: Released under the terms of the LGPLv2.1 or later, see LGPL.TXT
 *
 */

#include <stdio.h>
#include <yarp/os/all.h>

#include <yarp/os/Carrier.h>
// #include <yarp/os/impl/Carrier.h>
// #include <yarp/os/impl/Carriers.h>
// #include <yarp/os/impl/String.h>
// #include <yarp/os/Bytes.h>
// #include <yarp/os/ManagedBytes.h>
// #include <yarp/os/impl/NetType.h>
// #include <yarp/os/impl/Protocol.h>

#include <iostream>
#include <string>

// using namespace std;
// using namespace yarp::os;
//using namespace yarp::os::impl;


class XXXStream : public yarp::os::TwoWayStream, public yarp::os::InputStream, public yarp::os::OutputStream {
private:
    bool interrupting;
    bool needInterrupt;
    std::string inputCache;
    std::string outputCache;
public:
    XXXStream() {
        interrupting = false;
        needInterrupt = false;
        inputCache = outputCache = "";
    }

    virtual void close() {
        std::cout << "Bye bye" << std::endl;
    }

    virtual bool isOk() {
        return true;
    }

    virtual void interrupt() {
        interrupting = true;
        while (needInterrupt) {
            std::cout << "*** INTERRUPT: Please hit enter ***" << std::endl;
            for (int i=0; i<10 && needInterrupt; i++) {
                yarp::os::Time::delay(0.1);
            }
        }
    }

    // InputStream

    virtual ssize_t read(const yarp::os::Bytes& b);

    // OutputStream

    virtual void write(const yarp::os::Bytes& b);

    // TwoWayStream

    virtual yarp::os::InputStream& getInputStream() {
        return *this;
    }

    virtual yarp::os::OutputStream& getOutputStream() {
        return *this;
    }

    virtual const yarp::os::Contact& getLocalAddress() {
        // left undefined
        return local;
    }

    virtual const yarp::os::Contact& getRemoteAddress() {
        // left undefined
        return remote;
    }

    virtual void reset() {
        inputCache = outputCache = "";
        std::cout << "Stream reset" << std::endl;
    }

    virtual void beginPacket() {
        std::cout << "Packet begins" << std::endl;
        inputCache = "";
        outputCache = "";
    }

    virtual void endPacket() {
        std::cout << "Packet ends" << std::endl;
    }

private:
    yarp::os::Contact local, remote;
};
