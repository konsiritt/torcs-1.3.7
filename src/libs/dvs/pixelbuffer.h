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

// dvs implementation
#include "dvsemulator.h"

#include "config_dvs.h"


struct shared_mem_emul
{
    shared_mem_emul() :
        timeNew(0),
        timeRef(0),
        imageNew(),
        imageRef(),
        frameUpdated(false),
        mutex()
    {
    }
    double timeNew;
    double timeRef;
    unsigned char imageNew[image_width*image_height*4];
    double imageRef[image_width*image_height];

    //boolean updated when new frame was written
    bool frameUpdated;

    //Mutex to protect access to the queue
    boost::interprocess::interprocess_mutex mutex;

    //Condition to wait when the frame was not updated
    boost::interprocess::interprocess_condition  condNew;
};


class pixelBuffer
{
private:
//    dvsEmulator dvsE;
    const static int pboCount = pbo_count;
    unsigned pboIndex;
    unsigned screenWidth;
    unsigned screenHeight;
    unsigned channelSize;
    unsigned dataSize;
    //unsigned dvsThresh;
    bool twoFrames;
    unsigned readCount;
    GLenum pixFormat;
    GLuint  pboIds[pboCount];    
    unsigned char *imgA;
    unsigned char *imgB;
    Timer t1;
    double tRead;
    double tMap;
    double tUnmap;
    double tProcess;
    unsigned framesCount;
    shared_mem_emul *dataShrd;
    double simTime [pboCount];
    double linLogLim;

public:
    pixelBuffer(unsigned screenWidth_, unsigned screenHeight_);
    ~pixelBuffer();

    /**
     * @brief resize adapts sizing of arrays used for pbo data transfer
     * @param screenWidth_
     * @param screenHeight_
     * @return
     */
    int resize(unsigned screenWidth_, unsigned screenHeight_);

    /**
     * @brief process computes luminosity difference between two consecutive frames
     */
    void process(double currentTime_);

    /**
     * @brief linlog computes mix of linear and log response to avoid near zero problems of log
     * @param arg
     * @return
     */
    inline double linlog(double arg)
    {
        if (arg>linLogLim)
        {
            return log(arg);
        }
        else
            return log(linLogLim)/linLogLim*arg;
    }


};

#endif // PIXELBUFFER_H
