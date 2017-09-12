#ifndef CONFIG_DVS_H
#define CONFIG_DVS_H

//! DVS Emulation from TORCS configuration file:

//****************************************************************
///! FRAME INTERFACE SETTINGS (needs to be adapted in ros_dvs_emulator
///  include/ros_dvs_emulator/config_dvs.h as well, where frames are accessed)
//****************************************************************

// define a display mode where every simulation time step a frame is rendered:
#define RM_DISP_MODE_EVERY 5

// game resolution used: TODO: save to shared memory
#define image_width 640 //320 //
#define image_height 480 //240 //
// amount of pixel buffer objects (pbo) that asynchronously access the frames
#define pbo_count 2

// color data access format: BlueGreenRedAlpha
#define pixel_format GL_BGRA //Fastest option for map from vram to ram
#define channel_size 4

// use condition for processing frames without loss of potential frames
// not fit for real-time emulation -> implemented in raceengineclient/raceengine.cpp
#define no_frame_loss_emulation

// log ground truth data -> implementations in ssgraph/grmain.cpp & raceengineclient/raceengine.cpp
#define log_gt true

// define obstacle location: i.e. location of car -> implementation in simuv2/simu.cpp
#define obstacle_location true
//#undef obstacle_location

// define a maximum velocity, to allow operation at constant speeds -> implementation in simuv2/car.cpp
#define limit_maxvel 20.0
//#undef limit_maxvel

// define no steering condition: to provide pure translational flow -> implementation in drivers/bt/driver.cpp
#define no_steering
#undef no_steering


#endif // CONFIG_DVS_H

