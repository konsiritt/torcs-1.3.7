#ifndef PIXELBUFFER_H
#define PIXELBUFFER_H

// Graphics
#include <GL/gl.h>

// IO
#include <iostream>

// memcpy
#include <cstring>

// computation
#include <math.h>
#include <stdlib.h>

// execution time measurement
#include "Timer.h"

// dvs implementation
#include "dvsemulator.h"

class pixelBuffer
{
private:
    dvsEmulator dvsE;
    const static int pboCount = 2;
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
    void process();


};

#endif // PIXELBUFFER_H
