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
        aIsNew(true),
        timeA(0),
        timeB(0),
        imageA(),
        imageB(),
        frameUpdated(false),
        mutex()
    {
    }

    //boolean differentiating between old and new frame
    bool aIsNew;
    double timeA;
    double timeB;
    unsigned char imageA[image_width*image_height*4];
    unsigned char imageB[image_width*image_height*4];

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
    bool aIsNew;
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


};

#endif // PIXELBUFFER_H
