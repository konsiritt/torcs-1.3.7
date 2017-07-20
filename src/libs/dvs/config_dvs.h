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

// define frames_float if the color values should be obtained in
// GL_FLOAT [0,1] format from GPU instead of GL_UNSIGNED_BYTE [0,255]
#define frames_float //TODO: NOT IMPLEMENTED

// use condition for processing frames without loss of potential frames
// not fit for real-time emulation
#define no_loss_frame_emulation

#endif // CONFIG_DVS_H

