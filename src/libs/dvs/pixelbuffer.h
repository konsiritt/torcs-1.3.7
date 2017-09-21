#ifndef PIXELBUFFER_H
#define PIXELBUFFER_H

// Graphics
#include <GL/gl.h>

// IO
#include <iostream>

// memcpy
#include <cstring>

// boost shared mem
#include <boost/interprocess/shared_memory_object.hpp>
#include <boost/interprocess/mapped_region.hpp>
#include <boost/interprocess/sync/scoped_lock.hpp>
#include <boost/interprocess/sync/interprocess_mutex.hpp>
#include <boost/interprocess/sync/interprocess_condition.hpp>

// computation
#include <math.h>
#include <stdlib.h>

// execution time measurement
#include "Timer.h"

#include "config_dvs.h"


//****************************************************************
///! Structs
//****************************************************************

//! struct that is kept in shared memory
//! ATTENTION: changes here need to be performed in TORCS main.cpp
//! as well, where frames are saved to shared memory
struct shared_mem_emul
{
    shared_mem_emul() :
        timeNew(0),
        timeRef(0),
        imageNew(),
        imageRef(),
        frameUpdated(false),
        frameIndex(0),
        mutex()
    {
    }
    //! time stamp of newly acquired frame
    double timeNew;
    //! time stamp of last frame
    double timeRef;
    //! RGBA values of newly acquired frame
    unsigned char imageNew[image_width*image_height*4];
    //! log(luminance) value of reference frame (last event generated)
    double imageRef[width_out*height_out];

    //! true when new frame was written
    bool frameUpdated;
    //! frame index to keep track of loss of frames saved to memory
    double frameIndex;

    //! Mutex to protect access to the queue
    boost::interprocess::interprocess_mutex mutex;

    //! Condition to wait on when the frame was not updated
    boost::interprocess::interprocess_condition  condNew;
    //! Condition to wait on when the frame was not processed
    boost::interprocess::interprocess_condition  condProcess;
};


//****************************************************************
///! Class
//****************************************************************
//!
//! \brief The pixelBuffer class manages access to the gpu framebuffer
//!
//! the pbo is a openGL construct that asynchronously accesses
//! the gpu framebuffer. The frames can then be transfered to
//! regular RAM. This implementation directly transfers the obtained
//! rendered frames to shared memory to provide access to a subsequent
//! emulator.
class pixelBuffer
{
private:
    //****************************************************************
    ///! GPU access specific variables
    //****************************************************************
    //! amount of pbos used in turn
    const static int pboCount = pbo_count;
    //! index of currently used pbo
    unsigned pboIndex;
    unsigned screenWidth;
    unsigned screenHeight;
    //! color channel size: RGBA: 4
    unsigned channelSize;
    //! size of data per frame
    unsigned dataSize;
    //! read count to keep track of glReadPixels calls before mapping
    unsigned readCount;
    //! internal (openGL) pixel representation format
    GLenum pixFormat;
    //! handle for pbos
    GLuint  pboIds[pboCount];

    //****************************************************************
    ///! Performance evaluation variables
    //****************************************************************
    //! timer object taking timing measurements
    Timer t1;
    //! time for each part of the processing pipeline
    double tRead;
    double tMap;
    double tUnmap;
    double tProcess;
    //! evaluated frames, to perform averaging for timings
    unsigned framesCount;
    //! time point to limit display of performance to certain frequency
    double lastTimeDisplay;

    //! total frames, since start of race
    unsigned totalFrames;

    //****************************************************************
    ///! Frame output variables
    //****************************************************************
    //! directory where frames are saved to
    std::string outputDirAEDat;
    //! counter for frames put out
    int savedFrames;
    //! time of last screenshot to keep track of interval for new screen
    double lastTimeScreenshot;

    //****************************************************************
    ///! Shared memory access variables
    //****************************************************************
    //! struct in shared memory that contains data and access control
    shared_mem_emul *dataShrd;

    //****************************************************************
    ///! Emulation variables
    //****************************************************************
    //! array to save simulation time at time of glReadPixels for
    //! asynchronous access to pass later on for timestamping events
    double simTime [pboCount];

public:
    pixelBuffer(unsigned screenWidth_, unsigned screenHeight_);
    ~pixelBuffer();

    //****************************************************************
    ///! Class Methods
    //****************************************************************
    //! NOT IMPLEMENTED YET
    //!
    //! \brief resize adapts sizing of arrays used for pbo data transfer
    //! \param screenWidth_ new screen width
    //! \param screenHeight_ new screen height
    //! \return 0
    //!
    int resize(unsigned screenWidth_, unsigned screenHeight_);

    //!
    //! \brief process computes luminosity difference between two consecutive frames
    //! \param currentTime_ is the simulation time inherent to TORCS in sec
    //!
    void process(double currentTime_);
};

#endif // PIXELBUFFER_H
