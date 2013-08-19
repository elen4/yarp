// -*- mode:C++; tab-width:4; c-basic-offset:4; indent-tabs-mode:nil -*-

/*
 * Copyright (C) 2010 RobotCub Consortium
 * Authors: Paul Fitzpatrick
 * CopyPolicy: Released under the terms of the LGPLv2.1 or later, see LGPL.TXT
 *
 */

#include "XXXStream.h"
#include <yarp/os/NetType.h>

class XXXCarrier : public yarp::os::Carrier {
public:

    // First, the easy bits...

    virtual yarp::os::Carrier *create() {
        return new XXXCarrier();
    }

    virtual yarp::os::ConstString getName() {
        return "xxx";
    }

    virtual bool isConnectionless() {
        return true;
    }

    virtual bool canAccept() {
        return true;
    }

    virtual bool canOffer() {
        return true;
    }

    virtual bool isTextMode() {
        // let's be text mode, for xxx-friendliness
        return true;
    }

    virtual bool canEscape() {
        return true;
    }

    virtual bool requireAck() {
        return true;
    }

    virtual bool supportReply() {
        return true;
    }

    virtual bool isLocal() {
        return false;
    }

    virtual yarp::os::ConstString toString() {
        return "xxxs are handy";
    }

    virtual void getHeader(const yarp::os::Bytes& header) {
        const char *target = "XXXITY";
        for (size_t i=0; i<8 && i<header.length(); i++) {
            header.get()[i] = target[i];
        }
    }

    virtual bool checkHeader(const yarp::os::Bytes& header) {
        if (header.length()!=8) {
            return false;
        }
        const char *target = "XXXITY";
        for (size_t i=0; i<8; i++) {
            if (header.get()[i] != target[i]) {
                return false;
            }
        }
        return true;
    }

    virtual void setParameters(const yarp::os::Bytes& header) {
        // no parameters - no carrier variants
    }


    // Now, the initial hand-shaking

    virtual bool prepareSend(yarp::os::ConnectionState& proto) {
        // nothing special to do
        return true;
    }

    virtual bool sendHeader(yarp::os::ConnectionState& proto);

    virtual bool expectSenderSpecifier(yarp::os::ConnectionState& proto) {
        // interpret everything that sendHeader wrote
        proto.setRoute(proto.getRoute().addFromName(proto.is().readLine()));
        return proto.is().isOk();
    }

    virtual bool expectExtraHeader(yarp::os::ConnectionState& proto) {
        // interpret any extra header information sent - optional
        return true;
    }

    virtual bool respondToHeader(yarp::os::ConnectionState& proto) {
        // SWITCH TO NEW STREAM TYPE
        XXXStream *stream = new XXXStream();
        if (stream==NULL) { return false; }
        proto.takeStreams(stream);
        return true;
    }

    virtual bool expectReplyToHeader(yarp::os::ConnectionState& proto) {
        // SWITCH TO NEW STREAM TYPE
        XXXStream *stream = new XXXStream();
        if (stream==NULL) { return false; }
        proto.takeStreams(stream);
        return true;
    }

    virtual bool isActive() {
        return true;
    }


    // Payload time!

    virtual bool write(yarp::os::ConnectionState& proto, yarp::os::SizedWriter& writer) {
        bool ok = sendIndex(proto);
        if (!ok) return false;
        writer.write(proto.os());
        return proto.os().isOk();
    }

    virtual bool sendIndex(yarp::os::ConnectionState& proto) {
        yarp::os::ConstString prefix = "xxx says ";
        yarp::os::Bytes b2((char*)prefix.c_str(),prefix.length());
        proto.os().write(b2);
        return true;
    }

    virtual bool expectIndex(yarp::os::ConnectionState& proto) {
        yarp::os::ConstString prefix = "xxx says ";
        yarp::os::ConstString compare = prefix;
        yarp::os::Bytes b2((char*)prefix.c_str(),prefix.length());
        proto.is().read(b2);
        bool ok = proto.is().isOk() && (prefix==compare);
        if (!ok) std::cout << "YOU DID NOT SAY 'xxx says '" << std::endl;
        return ok;
    }

    // Acknowledgements, we don't do them

    virtual bool sendAck(yarp::os::ConnectionState& proto) {
        yarp::os::ConstString prefix = "computers rule!\r\n";
        yarp::os::Bytes b2((char*)prefix.c_str(),prefix.length());
        proto.os().write(b2);
        return true;
    }

    virtual bool expectAck(yarp::os::ConnectionState& proto) {
        yarp::os::ConstString prefix = "computers rule!\r\n";
        yarp::os::ConstString compare = prefix;
        yarp::os::Bytes b2((char*)prefix.c_str(),prefix.length());
        proto.is().read(b2);
        bool ok = proto.is().isOk() && (prefix==compare);
        if (!ok) std::cout << "YOU DID NOT SAY 'computers rule!'" << std::endl;
        return ok;
    }

};
