#ifndef DVSEMULATOR_H
#define DVSEMULATOR_H


// computation
#include <math.h>
#include <stdlib.h>

// uint8_t
#include <stdint.h>

class dvsEmulator
{
private:
    unsigned screenWidth;
    unsigned screenHeight;
    unsigned channelSize;
    unsigned dataSize;
    unsigned dvsThresh;
    bool aIsNew;
//    Timer t1;
public:
    dvsEmulator(unsigned screenWidth_, unsigned screenHeight_,
                unsigned channelSize_, unsigned dvsThresh_);
    ~dvsEmulator();

    void emulate(unsigned char * oldFrame_, unsigned char * newFrame_, double timeOld_, double timeNew_);



};

#endif // DVSEMULATOR_H
