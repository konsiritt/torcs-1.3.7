/***************************************************************************

    file                 : main.cpp
    created              : Sat Mar 18 23:54:30 CET 2000
    copyright            : (C) 2000 by Eric Espie
    email                : torcs@free.fr
    version              : $Id: main.cpp,v 1.14.2.3 2012/06/01 01:59:42 berniw Exp $

 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <stdlib.h>

#include <GL/glut.h>

#include <tgfclient.h>
#include <client.h>

#include "linuxspec.h"
#include <raceinit.h>

// boost shared mem
#include <boost/interprocess/shared_memory_object.hpp>
#include <boost/interprocess/mapped_region.hpp>
#include <boost/interprocess/sync/scoped_lock.hpp>
#include <boost/interprocess/sync/interprocess_mutex.hpp>
#include <boost/interprocess/sync/interprocess_condition.hpp>

#include <sys/shm.h> 

#include <iostream> 
#include <unistd.h> 

// config for shared memory setup: including screen resolution
#include "../libs/dvs/config_dvs.h"
//debug shmget error:
//#include <errno.h>
//#define image_width 640 //320 //
//#define image_height 480 //240 //


extern bool bKeepModules;

static void
init_args(int argc, char **argv, const char **raceconfig)
{
	int i;
	char *buf;
    
    setNoisy(false);
    setVersion("2013");

	i = 1;

	while(i < argc) {
		if(strncmp(argv[i], "-l", 2) == 0) {
			i++;

			if(i < argc) {
				buf = (char *)malloc(strlen(argv[i]) + 2);
				sprintf(buf, "%s/", argv[i]);
				SetLocalDir(buf);
				free(buf);
				i++;
			}
		} else if(strncmp(argv[i], "-L", 2) == 0) {
			i++;

			if(i < argc) {
				buf = (char *)malloc(strlen(argv[i]) + 2);
				sprintf(buf, "%s/", argv[i]);
				SetLibDir(buf);
				free(buf);
				i++;
			}
		} else if(strncmp(argv[i], "-D", 2) == 0) {
			i++;

			if(i < argc) {
				buf = (char *)malloc(strlen(argv[i]) + 2);
				sprintf(buf, "%s/", argv[i]);
				SetDataDir(buf);
				free(buf);
				i++;
			}
		} else if(strncmp(argv[i], "-s", 2) == 0) {
			i++;
			SetSingleTextureMode();
		} else if (strncmp(argv[i], "-t", 2) == 0) {
		    i++;
		    if (i < argc) {
			long int t;
			sscanf(argv[i],"%ld",&t);
			setTimeout(t);
			printf("UDP Timeout set to %ld 10E-6 seconds.\n",t);
			i++;
		    }
		} else if (strncmp(argv[i], "-nodamage", 9) == 0) {
		    i++;
		    setDamageLimit(false);
		    printf("Car damages disabled!\n");
		} else if (strncmp(argv[i], "-nofuel", 7) == 0) {
		    i++;
		    setFuelConsumption(false);
		    printf("Fuel consumption disabled!\n");
		} else if (strncmp(argv[i], "-noisy", 6) == 0) {
		    i++;
		    setNoisy(true);
		    printf("Noisy Sensors!\n");
		} else if (strncmp(argv[i], "-ver", 4) == 0) {
		    i++;
		    if (i < argc) {
					setVersion(argv[i]);
		    		printf("Set version: \"%s\"\n",getVersion());
		    		i++;
		    }
		} else if (strncmp(argv[i], "-nolaptime", 10) == 0) {
		    i++;
		    setLaptimeLimit(false);
		    printf("Laptime limit disabled!\n");   
		} else if(strncmp(argv[i], "-k", 2) == 0) {
			i++;
			// Keep modules in memory (for valgrind)
			printf("Unloading modules disabled, just intended for valgrind runs.\n");
			bKeepModules = true;
#ifndef FREEGLUT
		} else if(strncmp(argv[i], "-m", 2) == 0) {
			i++;
			GfuiMouseSetHWPresent(); /* allow the hardware cursor */
#endif
		} else if(strncmp(argv[i], "-r", 2) == 0) {
			i++;
			*raceconfig = "";

			if(i < argc) {
				*raceconfig = argv[i];
				i++;
			}

			if((strlen(*raceconfig) == 0) || (strstr(*raceconfig, ".xml") == 0)) {
				printf("Please specify a race configuration xml when using -r\n");
				exit(1);
			}
		} else {
			i++;		/* ignore bad args */
		}
	}

#ifdef FREEGLUT
	GfuiMouseSetHWPresent(); /* allow the hardware cursor (freeglut pb ?) */
#endif
}

//struct shared_use_st
//{
//    int written;
//    uint8_t data[image_width*image_height*3];
//    int pause;
//    int zmq_flag;
//    int save_flag;
//};

//int* pwritten = NULL;
//uint8_t* pdata = NULL;
//int* ppause = NULL;
//int* pzmq_flag = NULL;
//int* psave_flag = NULL;

//void *shm = NULL;

//****************************************************************
///! Structs
//****************************************************************

//! struct that is kept in shared memory
//! ATTENTION: changes here need to be performed in emulator.h
//! in my_ros_dvs_emulator where frames are read from shared memory
struct shared_mem_emul
{
    shared_mem_emul() :
        timeNew(0),
        timeRef(0),
        imageNew(),
        imageRef(),
        frameUpdated(false),
        frameIndex(0),
        mutex()
    {
    }
    //! time stamp of newly acquired frame
    double timeNew;
    //! time stamp of last frame
    double timeRef;
    //! RGBA values of newly acquired frame
    unsigned char imageNew[image_width*image_height*4];
    //! log(luminance) value of reference frame (last event generated)
    double imageRef[width_out*height_out];

    //! true when new frame was written
    bool frameUpdated;
    //! frame index to keep track of loss of frames saved to memory
    double frameIndex;

    //! Mutex to protect access to the queue
    boost::interprocess::interprocess_mutex mutex;

    //! Condition to wait on when the frame was not updated
    boost::interprocess::interprocess_condition  condNew;
    //! Condition to wait on when the frame was not processed
    boost::interprocess::interprocess_condition  condProcess;
};

namespace bip = boost::interprocess;

//! pointer to shared memory struct that will be accessed later
shared_mem_emul * dataShrdMain = NULL;

/*
 * Function
 *	main
 *
 * Description
 *	LINUX entry point of TORCS
 *
 * Parameters
 *
 *
 * Return
 *
 *
 * Remarks
 *
 */
int
main(int argc, char *argv[])
{
//	struct shared_use_st *shared = NULL;
//    int shmid;
//    // establish memory sharing
//    shmid = shmget((key_t)1235, sizeof(struct shared_use_st), 0666|IPC_CREAT);
//    if(shmid == -1)
//    {
//        int errsv = errno;
//        std::cout << "shmget error number: " << errsv << std::endl;
//        fprintf(stderr, "shmget failed\n");

//        exit(EXIT_FAILURE);
//    }
  
//    shm = shmat(shmid, 0, 0);
//    if(shm == (void*)-1)
//    {
//        fprintf(stderr, "shmat failed\n");
//        exit(EXIT_FAILURE);
//    }
//    printf("\n********** Memory sharing started, attached at %X **********\n \n", shm);
//    // set up shared memory
//    shared = (struct shared_use_st*)shm;
//    shared->written = 0;
//    shared->pause = 0;
//    shared->zmq_flag = 0;
//    shared->save_flag = 0;

 
//    pwritten=&shared->written;
//    pdata=shared->data;
//    ppause=&shared->pause;
//    pzmq_flag = &shared->zmq_flag;
//	psave_flag = &shared->save_flag;

    // ---- memory sharing using boost library to use in emulator to ros interface ---//
    //Remove shared memory on construction and destruction
    struct shm_remove
    {
        shm_remove() { bip::shared_memory_object::remove("shared_memory"); }
        ~shm_remove(){ bip::shared_memory_object::remove("shared_memory"); }
    } remover;

    //Create a shared memory object.
    bip::shared_memory_object shm (bip::open_or_create, "shared_memory", bip::read_write);

    //Set size
    shm.truncate(sizeof(shared_mem_emul));

    //Map the whole shared memory in this process
    bip::mapped_region region(shm, bip::read_write);

    //Get the address of the mapped region
    void * addr       = region.get_address();

    //Construct the shared structure in memory
    dataShrdMain = new (addr) shared_mem_emul;

    memset(dataShrdMain->imageNew, 0, sizeof(dataShrdMain->imageNew));
    memset(dataShrdMain->imageRef, 0, sizeof(dataShrdMain->imageRef));

    std::cout << "Memory sharing for emulator events initialized" << std::endl;


    const char *raceconfig = "";

    init_args(argc, argv, &raceconfig);
	LinuxSpecInit();			/* init specific linux functions */

	if(strlen(raceconfig) == 0) {
		GfScrInit(argc, argv);	/* init screen */
		TorcsEntry();			/* launch TORCS */
		glutMainLoop();			/* event loop of glut */
	} else {
		// Run race from console, no Window, no OpenGL/OpenAL etc.
		// Thought for blind scripted AI training
		ReRunRaceOnConsole(raceconfig);
	}
	return 0;					/* just for the compiler, never reached */
}

