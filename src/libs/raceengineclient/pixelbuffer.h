#ifndef PIXELBUFFER_H
#define PIXELBUFFER_H

#include "Timer.h"

class pixelBuffer
{
private:
    const static int pboCount = 2;
    unsigned screenWidth;
    unsigned screenHeight;
    unsigned dataSize;
    unsigned int dvsThresh;
    bool copiedImg;
    bool mappable;
    GLenum pixelFormat;
    GLuint  pboIds[pboCount];    
    unsigned char *imgOld;    
    Timer t1;



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
