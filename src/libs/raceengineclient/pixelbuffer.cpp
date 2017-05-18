#include <GL/gl.h>
#include <stdlib.h>
#include <iostream>

#include "Timer.h"
#include "pixelbuffer.h"

pixelBuffer::pixelBuffer (unsigned screenWidth_, unsigned screenHeight_) :
    screenWidth(screenWidth_),
    screenHeight(screenHeight_),
    dataSize(0),
    dvsThresh(35),
    copiedImg(false),
    mappable(false),
    pixelFormat(GL_LUMINANCE) //GL_BGRA, GL_RGBA

{
    if (GL_LUMINANCE == pixelFormat)
    {
        dataSize = screenWidth * screenHeight;
    }
    else if (GL_RGB == pixelFormat || GL_BGR == pixelFormat)
    {
        dataSize = screenWidth * screenHeight * 3;
    }
    else if (GL_RGBA == pixelFormat || GL_BGRA == pixelFormat)
    {
        dataSize = screenWidth * screenHeight * 4;
    }

    imgOld = new unsigned char [dataSize];

    std::cout << "PBO IDs: "<< pboIds[0] << " and " << pboIds[1] << std::endl;
    // Generate pixel buffer objects
    glGenBuffers(pboCount, pboIds);
    glBindBuffer(GL_PIXEL_PACK_BUFFER, pboIds[0]);
    glBufferData(GL_PIXEL_PACK_BUFFER, dataSize, 0, GL_STREAM_READ);
    glBindBuffer(GL_PIXEL_PACK_BUFFER, pboIds[1]);
    glBufferData(GL_PIXEL_PACK_BUFFER, dataSize, 0, GL_STREAM_READ);

    glBindBuffer(GL_PIXEL_PACK_BUFFER, 0);

}


pixelBuffer::~pixelBuffer()
{
    glDeleteBuffers(pboCount, pboIds);
    delete [] imgOld;
    imgOld = 0;
}

int pixelBuffer::resize(unsigned screenWidth_, unsigned screenHeight_)
{
    return 0;
}

extern int* pwritten;
extern uint8_t* pdata;

void pixelBuffer::process()
{
    static int index = 0;
    int nextIndex = 0;                  // pbo index used for next frame
    // increment current index first then get the next index
    // "index" is used to read pixels from a framebuffer to a PBO
    // "nextIndex" is used to process pixels in the other PBO
    index = (index + 1) % 2;
    nextIndex = (index + 1) % 2;

    // set the framebuffer to read
    glReadBuffer(GL_FRONT);

    t1.start();

    // copy pixels from framebuffer to PBO
    // Use offset instead of ponter.
    // OpenGL should perform asynch DMA transfer, so glReadPixels() will return immediately.
    glBindBuffer(GL_PIXEL_PACK_BUFFER, pboIds[index]);
    std::cout << "pbo "<<index<< " is bound" << std::endl;
    glReadPixels(0, 0, screenWidth, screenHeight, pixelFormat, GL_UNSIGNED_BYTE, 0);
    std::cout << "pbo "<<index<< " is read" << std::endl;

    // map the PBO that contain framebuffer pixels before processing it
    glBindBuffer(GL_PIXEL_PACK_BUFFER, pboIds[nextIndex]);
    std::cout << "pbo "<<nextIndex<< " is bound" << std::endl;

    unsigned char* imgCurr = NULL;
    if (mappable)
    {
        std::cout << "pbo "<<nextIndex<< " is mapped" << std::endl;
        imgCurr = (unsigned char*)glMapBuffer(GL_PIXEL_PACK_BUFFER, GL_READ_ONLY);
    }

    t1.stop();
    std::cout << "Time till after map: " << t1.getElapsedTimeInMilliSec()<<std::endl;
    t1.start();

    // first call of MapBuffer is not successful, no prior read to that pbo
    if (mappable)
    {
        // on the first pass there is nothing copied to imgOld
        if (copiedImg)
        {
            std::cout << "pbo "<<nextIndex<< " is compared to imgOld" << std::endl;
            // compute luminance difference for each pixel
            for (unsigned ii=0; ii<dataSize; ++ii)
            {
                int temp_lum = - imgCurr[ii] + imgOld[ii];
                // copy new image values for next frame
                imgOld[ii] = imgCurr[ii];

                if (abs(temp_lum)>dvsThresh)
                {
                    pdata[ii] = - temp_lum;
                }
                else
                    pdata[ii] = 0;
            }
            *pwritten=1;
        }
        else
        {
            std::cout << "pbo "<<nextIndex<< " is copied to imgOld" << std::endl;
            // copy values to imgOld for comparison in next call of process
            for (unsigned ii=0; ii<dataSize; ++ii)
            {
                imgOld[ii] = imgCurr[ii];
            }
            copiedImg = true;
        }
    }
    else
    {
        mappable = true;
    }
    glUnmapBuffer(GL_PIXEL_PACK_BUFFER);     // release pointer to the mapped buffer
    glBindBuffer(GL_PIXEL_PACK_BUFFER, 0);

    t1.stop();
    std::cout << "Time till after unmap: " << t1.getElapsedTimeInMilliSec()<<std::endl;
}
