
#include "pixelbuffer.h"

namespace bip = boost::interprocess;

extern shared_mem_emul * dataShrdMain;

pixelBuffer::pixelBuffer (unsigned screenWidth_, unsigned screenHeight_) :
//    dvsE(screenWidth_,screenHeight_,
//         channelSize_,dvsThreshold_), //2DO: pass channelSize according to pixelFormat
    pboIndex(0),
    screenWidth(screenWidth_),
    screenHeight(screenHeight_),
    channelSize(0),
    dataSize(0),
    //dvsThresh(dvsThreshold_),
    twoFrames(false),
    readCount(0),
    pixFormat(pixel_format),
    tRead(0),
    tMap(0),
    tUnmap(0),
    tProcess(0),
    framesCount(0),
    linLogLim(lin_log_lim)
{
    for (int i=0; i<pboCount; ++i)
    {
        simTime[i] = 0;
    }

    if (GL_LUMINANCE == pixFormat)
    {
        channelSize = 1;
        dataSize = screenWidth * screenHeight;
    }
    else if (GL_RGB == pixFormat || GL_BGR == pixFormat)
    {
        channelSize = 3;
        dataSize = screenWidth * screenHeight * channelSize;
    }
    else if (GL_RGBA == pixFormat || GL_BGRA == pixFormat)
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

    //link the shared structure in memory (main.cpp)
    dataShrd = dataShrdMain;

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

    //Erase shared memory
    bip::shared_memory_object::remove("shared_memory");

    std::cout << "PBO-Object destroyed" << std::endl;
}

int pixelBuffer::resize(unsigned screenWidth_, unsigned screenHeight_)
{
    return 0;
}

void pixelBuffer::process(double currentTime_)
{
    unsigned char* gpuPtr;

    // make sure all our pbos are bound to make use of asynchronous read of VRAM without blocking calls
    if (readCount < pboCount)
    {
        t1.start();

        // set the framebuffer to read
        glReadBuffer(GL_FRONT);        

        // copy pixels from framebuffer to PBO
        // Use offset instead of ponter.
        // OpenGL should perform asynch DMA transfer, so glReadPixels() will return immediately.
        glBindBuffer(GL_PIXEL_PACK_BUFFER, pboIds[pboIndex]);

        // get the current simulation time as time stamp for event timing
        simTime[pboIndex] = currentTime_;

        // important: the internal format needs to match the pixFormat to return fast
        glReadPixels(0, 0, screenWidth, screenHeight, pixFormat, GL_UNSIGNED_BYTE, 0);

        t1.stop();
        tRead += t1.getElapsedTimeInMilliSec();

        readCount++;
    }
    // only when all pbo's have been bound, then start mapping (where blocking might still occur)
    else
    {
        t1.start();

        // map the PBO that contain framebuffer pixels before processing it
        glBindBuffer(GL_PIXEL_PACK_BUFFER, pboIds[pboIndex]);
        gpuPtr = (unsigned char*)glMapBuffer(GL_PIXEL_PACK_BUFFER, GL_READ_ONLY);

        t1.stop();
        tMap += t1.getElapsedTimeInMilliSec();
        t1.start();

        // copy from VRAM to RAM
        if (NULL != gpuPtr) {

            {
                bip::scoped_lock<bip::interprocess_mutex> lock(dataShrd->mutex);                
                t1.stop();
                tUnmap += t1.getElapsedTimeInMilliSec();
                t1.start();

                // check if enough frames exist to perform first event emulation
                if (twoFrames) {
                    std::memcpy(dataShrd->imageNew, gpuPtr, dataSize);
                    dataShrd->timeRef = dataShrd->timeNew;
                    dataShrd->timeNew = simTime[pboIndex];

                    // flag that frame data has been updated
                    dataShrd->frameUpdated = true;
                    // notify the emulating process of newly available data
                    dataShrd->condNew.notify_one();
                }
                else
                {
                    twoFrames = true;

                    int sizePic = screenWidth*screenHeight;
                    for (int ii=0; ii<sizePic; ++ii)
                    {
                        // currently not rounding correctly, just to get visualization through ROS
                        dataShrd->imageRef[ii] =  linlog(0.33*(gpuPtr[4*ii] + gpuPtr[4*ii+1] + gpuPtr[4*ii+2]));
                    }
                    dataShrd->timeRef = simTime[pboIndex];
                }
            }
            glUnmapBuffer(GL_PIXEL_PACK_BUFFER);
        }
        else {
            std::cout<<"Failed to map the buffer"<<std::endl;
        }

        t1.stop();
        tProcess += t1.getElapsedTimeInMilliSec();
        t1.start();

        // get the current simulation time as time stamp for event timing
        simTime[pboIndex] = currentTime_;

        // schedule new glReadPixels for next frame into the pbo
        glReadPixels(0, 0, screenWidth, screenHeight, pixFormat, GL_UNSIGNED_BYTE, 0);

        t1.stop();
        tRead += t1.getElapsedTimeInMilliSec();

    }
    glBindBuffer(GL_PIXEL_PACK_BUFFER, 0);

    // advance counters
    ++pboIndex;
    pboIndex = pboIndex % pboCount;
    ++framesCount;

    // output of runtimes for different sections
    if (framesCount == 60 ) //((tRead+tMap+tUnmap+tProcess) >= 2000)
    {

        std::cout << " Average times: \n read - \t" << tRead/framesCount
                  << "ms, \n map it - \t"           << tMap/framesCount
                  << "ms, \n mutexed  - \t"          << tUnmap/framesCount
                  << "ms, \n copied - \t"          << tProcess/framesCount
                  << "ms, \n total - \t"            << (tRead+tMap+tUnmap+tProcess)/framesCount
                  << std::endl;
        tRead = 0;
        tMap = 0;
        tUnmap = 0;
        tProcess = 0;
        framesCount = 0;
    }
}
