#include "dvsemulator.h"

dvsEmulator::dvsEmulator(unsigned screenWidth_, unsigned screenHeight_,
                         unsigned channelSize_, unsigned dvsThresh_):
    screenWidth(screenWidth_),
    screenHeight(screenHeight_),
    channelSize(channelSize_),
    dataSize(0),
    dvsThresh(dvsThresh_),
    aIsNew(true)
{

}

//extern int* pwritten;
//extern uint8_t* pdata;

//void dvsEmulator::emulate(unsigned char *oldFrame_, unsigned char *newFrame_, double timeOld_, double timeNew_)
//{
//    int temp_lum = 0;
//    int sizePic = screenWidth*screenHeight;
//    // compute luminance difference for each pixel
//    for (int ii=0; ii<sizePic; ++ii)
//    {
//        // currently not rounding correctly, just to get visualization through ROS
//        temp_lum = (int) abs(0.33*(newFrame_[4*ii] + newFrame_[4*ii+1] + newFrame_[4*ii+2]
//                                 - oldFrame_[4*ii] - oldFrame_[4*ii+1] - oldFrame_[4*ii+2]));

//        if (temp_lum>dvsThresh)
//        {
//            pdata[ii] = (unsigned char)temp_lum; // for now only use unsigned delta luminosity
//        }
//        else
//            pdata[ii] = 0;
//    }
//    *pwritten=1;

//}

