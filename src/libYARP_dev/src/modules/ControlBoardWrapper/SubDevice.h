// -*- mode:C++; tab-width:4; c-basic-offset:4; indent-tabs-mode:nil -*-
#ifndef __SUBDEVICE__
#define __SUBDEVICE__

/*
* Copyright (C) 2013 iCub Facility - Istituto Italiano di Tecnologia
* Author: Lorenzo Natale
* CopyPolicy: Released under the terms of the LGPLv2.1 or later, see LGPL.TXT
*
*/

// ControlBoardWrapper
// A modified version of the remote control board class
// which remaps joints, it can also merge networks into a single part.
//

#include <yarp/os/PortablePair.h>
#include <yarp/os/BufferedPort.h>
#include <yarp/os/Time.h>
#include <yarp/os/Network.h>
#include <yarp/os/RateThread.h>
#include <yarp/os/Stamp.h>
#include <yarp/os/Vocab.h>

#include <yarp/dev/ControlBoardInterfaces.h>
#include <yarp/dev/PolyDriver.h>
#include <yarp/dev/ControlBoardInterfacesImpl.h>
#include <yarp/dev/PreciselyTimed.h>
#include <yarp/sig/Vector.h>
#include <yarp/os/Semaphore.h>
#include <yarp/dev/Wrapper.h>

#include <string>
#include <vector>

#include "ControlBoardWrapper.h"
#include "StreamingMessagesParser.h"
#include "RPCMessagesParser.h"

#ifdef MSVC
    #pragma warning(disable:4355)
#endif

/*
 * To optimize memory allocation, for group of joints we can have one mem reserver for rpc port
 * and on e for streaming. The size could be numOfSubDevices*maxNumOfjointForSubdevice.
 * (we could also use the actual joint number for each subdevice using a for loop). TODO
 */

/* Using yarp::dev::impl namespace for all helper class inside yarp::dev to reduce
 * name conflicts
 */

namespace yarp {
    namespace dev {
        class ControlBoardWrapper;
        namespace impl {
            class SubDevice;
            class WrappedDevice;
        }
    }
}


#ifndef DOXYGEN_SHOULD_SKIP_THIS

/*
* An Helper class for the controlBoardWrapper
* It maps only a subpart of the underlying device.
*/

class  yarp::dev::impl::SubDevice
{
public:
    std::string id;
    int base;
    int top;
    int axes;

    bool configuredF;

    yarp::dev::PolyDriver            *subdevice;
    yarp::dev::IPidControl           *pid;
    yarp::dev::IPositionControl      *pos;
    yarp::dev::IPositionControl2     *pos2;
    yarp::dev::IVelocityControl      *vel;
    yarp::dev::IVelocityControl2     *vel2;
    yarp::dev::IEncodersTimed        *enc;
    yarp::dev::IAmplifierControl     *amp;
    yarp::dev::IControlLimits2       *lim2;
    yarp::dev::IControlCalibration   *calib;
    yarp::dev::IControlCalibration2  *calib2;
    yarp::dev::IPreciselyTimed       *iTimed;
    yarp::dev::ITorqueControl        *iTorque;
    yarp::dev::IImpedanceControl     *iImpedance;
    yarp::dev::IOpenLoopControl      *iOpenLoop;
    yarp::dev::IControlMode          *iMode;
    yarp::dev::IControlMode2         *iMode2;
    yarp::dev::IAxisInfo             *info;
    yarp::dev::IPositionDirect       *posDir;
    yarp::dev::IInteractionMode      *iInteract;

    yarp::sig::Vector subDev_encoders;
    yarp::sig::Vector encodersTimes;

    SubDevice();

    bool attach(yarp::dev::PolyDriver *d, const std::string &id);
    void detach();
    inline void setVerbose(bool _verbose) {_subDevVerbose = _verbose; }

    bool configure(int base, int top, int axes, const std::string &id);

    inline void refreshEncoders()
    {
    	int idx = 0;
    	double tmp = 0;
        for(int j=base, idx=0; j<(base+axes); j++, idx++)
        {
            if(enc)
                enc->getEncoderTimed(j, &subDev_encoders[idx], &encodersTimes[idx]);
        }
    }

    bool isAttached()
    { return attachedF; }

private:
    bool _subDevVerbose;
    bool attachedF;
};

typedef std::vector<yarp::dev::impl::SubDevice> SubDeviceVector;

struct DevicesLutEntry
{
    int offset; //an offset, the device is mapped starting from this joint
    int deviceEntry; //index to the joint corresponding subdevice in the list
};


class yarp::dev::impl::WrappedDevice
{
public:
    SubDeviceVector subdevices;
    std::vector<DevicesLutEntry> lut;

    inline yarp::dev::impl::SubDevice *getSubdevice(unsigned int i)
    {
        if (i>=subdevices.size())
            return 0;

        return &subdevices[i];
    }
};

#endif // DOXYGEN_SHOULD_SKIP_THIS

#endif  //__SUBDEVICE__
