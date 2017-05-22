
#include "pixelbuffer.h"

pixelBuffer::pixelBuffer (unsigned screenWidth_, unsigned screenHeight_) :
    pboIndex(0),
    screenWidth(screenWidth_),
    screenHeight(screenHeight_),
    channelSize(0),
    dataSize(0),
    dvsThresh(35),
    twoFrames(false),
    aIsNew(true),
    readCount(0),
    pixelFormat(GL_BGRA), //Fastest option for map from vram to ram
    tRead(0),
    tMap(0),
    tUnmap(0),
    tProcess(0),
    framesCount(0)

{
    if (GL_LUMINANCE == pixelFormat)
    {
        channelSize = 1;
        dataSize = screenWidth * screenHeight;
    }
    else if (GL_RGB == pixelFormat || GL_BGR == pixelFormat)
    {
        channelSize = 3;
        dataSize = screenWidth * screenHeight * channelSize;
    }
    else if (GL_RGBA == pixelFormat || GL_BGRA == pixelFormat)
    {
        channelSize = 4;
        dataSize = screenWidth * screenHeight * channelSize;
    }

    imgA = new unsigned char [dataSize];
    imgB = new unsigned char [dataSize];

    // Generate pixel buffer objects
    glGenBuffers(pboCount, pboIds);
    for (unsigned ii=0; ii < pboCount; ++ii)
    {
        glBindBuffer(GL_PIXEL_PACK_BUFFER, pboIds[ii]);
        glBufferData(GL_PIXEL_PACK_BUFFER, dataSize, 0, GL_STREAM_READ);
    }
    glBindBuffer(GL_PIXEL_PACK_BUFFER, 0);

}


pixelBuffer::~pixelBuffer()
{
    glDeleteBuffers(pboCount, pboIds);
    if (NULL != imgA)
    {
        delete [] imgA;
        imgA = NULL;
    }
    if (NULL != imgB)
    {
        delete [] imgB;
        imgB = NULL;
    }
}

int pixelBuffer::resize(unsigned screenWidth_, unsigned screenHeight_)
{
    return 0;
}

extern int* pwritten;
extern uint8_t* pdata;

void pixelBuffer::process()
{
    unsigned char* gpuPtr;

    // make sure all our pbos are bound
    if (readCount < pboCount)
    {
        // set the framebuffer to read
        glReadBuffer(GL_FRONT);

        t1.start();

        // copy pixels from framebuffer to PBO
        // Use offset instead of ponter.
        // OpenGL should perform asynch DMA transfer, so glReadPixels() will return immediately.
        glBindBuffer(GL_PIXEL_PACK_BUFFER, pboIds[pboIndex]);
        glReadPixels(0, 0, screenWidth, screenHeight, pixelFormat, GL_UNSIGNED_BYTE, 0);

        t1.stop();
        tRead += t1.getElapsedTimeInMilliSec();

        readCount++;
    }
    else
    {
        t1.start();

        // map the PBO that contain framebuffer pixels before processing it
        glBindBuffer(GL_PIXEL_PACK_BUFFER, pboIds[pboIndex]);
        gpuPtr = (unsigned char*)glMapBuffer(GL_PIXEL_PACK_BUFFER, GL_READ_ONLY);

        t1.stop();
        tMap += t1.getElapsedTimeInMilliSec();

        t1.start();

        if (NULL != gpuPtr) {
            if (aIsNew)
            {
                std::memcpy(imgA, gpuPtr, dataSize);
            }
            else
            {
                std::memcpy(imgB, gpuPtr, dataSize);
            }
            glUnmapBuffer(GL_PIXEL_PACK_BUFFER);
        }
        else {
            std::cout<<"Failed to map the buffer"<<std::endl;
        }

        t1.stop();
        tUnmap += t1.getElapsedTimeInMilliSec();
        t1.start();

        if (twoFrames)
        {
            int temp_lum = 0;
            int sizePic = screenWidth*screenHeight;
            // compute luminance difference for each pixel
            for (int ii=0; ii<sizePic; ++ii)
            {
                // currently not rounding correctly, just to get visualization through ROS
                temp_lum = (int) abs(0.33*(imgA[4*ii] + imgA[4*ii+1] + imgA[4*ii+2]
                                         - imgB[4*ii] - imgB[4*ii+1] - imgB[4*ii+2]));

                if (temp_lum>dvsThresh)
                {
                    pdata[ii] = (unsigned char)temp_lum; // for now only use unsigned delta luminosity
                }
                else
                    pdata[ii] = 0;
            }
            *pwritten=1;
            t1.stop();
            tProcess += t1.getElapsedTimeInMilliSec();
            t1.start();
        }
        else
        {
            twoFrames = true;
        }

        glReadPixels(0, 0, screenWidth, screenHeight, pixelFormat, GL_UNSIGNED_BYTE, 0);

        t1.stop();
        tRead += t1.getElapsedTimeInMilliSec();

    }
    glBindBuffer(GL_PIXEL_PACK_BUFFER, 0);

    aIsNew = !aIsNew;
    ++pboIndex;
    pboIndex = pboIndex % pboCount;
    ++framesCount;

    if (framesCount == 100)
    {
        std::cout << " Average times: \n read - \t" << tRead/framesCount
                  << "ms, \n map it - \t"           << tMap/framesCount
                  << "ms, \n copy it - \t"          << tUnmap/framesCount
                  << "ms, \n process - \t"          << tProcess/framesCount
                  << "ms, \n total - \t"            << (tRead+tMap+tUnmap+tProcess)/framesCount
                  << std::endl;
        tRead = 0;
        tMap = 0;
        tUnmap = 0;
        tProcess = 0;
        framesCount = 0;
    }
}
