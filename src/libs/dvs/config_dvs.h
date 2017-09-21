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
#define image_width 320 //640 //1280//
#define image_height 240 //480 //960//
// for when supersampling is used, define downsampled size
#define width_out 320//640//1280//
#define height_out 240//480 //960//
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
#define limit_maxvel 30.0
//#undef limit_maxvel

// define no steering condition: to provide pure translational flow -> implementation in drivers/bt/driver.cpp
#define no_steering
//#undef no_steering

// save periodic screenshots to png file
#define save_screenshot true//false //
// time interval at which screenshots are saved
#define screenshot_interval 0.2

// set minimum zoom field of view (fov) (lower means more zoom, default 50)
#define min_fov_noCarInside 35.0


#endif // CONFIG_DVS_H

