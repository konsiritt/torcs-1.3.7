
#include "pixelbuffer.h"

namespace bip = boost::interprocess;

extern shared_mem_emul * dataShrdMain;

// configure pixelBuffer and dvsEmulator:
GLenum pixelFormat_ = GL_BGRA; //Fastest option for map from vram to ram
unsigned channelSize_ = 4;
unsigned dvsThreshold_ = 35;


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
    aIsNew(true),
    readCount(0),
    pixFormat(pixelFormat_),
    tRead(0),
    tMap(0),
    tUnmap(0),
    tProcess(0),
    framesCount(0)
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

    //link the shared structure in memory
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

extern int* pwritten;
extern uint8_t* pdata;

void pixelBuffer::process(double currentTime_)
{
    unsigned char* gpuPtr;

    // make sure all our pbos are bound to make use of asynchronous read of VRAM without blocking calls
    if (readCount < pboCount)
    {
        // set the framebuffer to read
        glReadBuffer(GL_FRONT);

        t1.start();

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

            if (aIsNew)
            {
//                std::memcpy(imgA, gpuPtr, dataSize);
                bip::scoped_lock<bip::interprocess_mutex> lock(dataShrd->mutex);
                std::memcpy(dataShrd->imageA, gpuPtr, dataSize);
            }
            else
            {
//                std::memcpy(imgB, gpuPtr, dataSize);
                bip::scoped_lock<bip::interprocess_mutex> lock(dataShrd->mutex);
                std::memcpy(dataShrd->imageB, gpuPtr, dataSize);
            }
            glUnmapBuffer(GL_PIXEL_PACK_BUFFER);
        }
        else {
            std::cout<<"Failed to map the buffer"<<std::endl;
        }

        t1.stop();
        tUnmap += t1.getElapsedTimeInMilliSec();
        t1.start();

        // check if enough frames exist to perform first event emulation
        if (twoFrames)
        {
//            // call the emulator class to perform event generation
//            if (aIsNew)
//            {
//                dvsE.emulate(imgB,imgA,0,0);
//            }
//            else
//            {
//                dvsE.emulate(imgA,imgB,0,0);
//            }
            t1.stop();
            tProcess += t1.getElapsedTimeInMilliSec();
            t1.start();
        }
        else
        {
            twoFrames = true;
        }

        // get the current simulation time as time stamp for event timing
        simTime[pboIndex] = currentTime_;

        // schedule new glReadPixels for next frame into the pbo
        glReadPixels(0, 0, screenWidth, screenHeight, pixFormat, GL_UNSIGNED_BYTE, 0);

        t1.stop();
        tRead += t1.getElapsedTimeInMilliSec();

    }
    glBindBuffer(GL_PIXEL_PACK_BUFFER, 0);

    // advance counters and booleans
    aIsNew = !aIsNew;
    ++pboIndex;
    pboIndex = pboIndex % pboCount;
    ++framesCount;

    // output of runtimes for different sections
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
