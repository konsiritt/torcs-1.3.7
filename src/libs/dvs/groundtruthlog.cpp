#include "groundtruthlog.h"
#include <iostream>


groundTruthLog::groundTruthLog(int nCars_):
    focalPixel(0),
    minVelFoe(0.2),
    nCars(nCars_),
    outputDir ("/home/rittk/Documents/torcsGT/"),
    outputName("")
{
    for (int i = 0; i < nCars; ++i)
    {
        if (i >= 2)
        {
            std::cerr << "more cars involved than can be logged" << std::endl;
            break;
        }

        if( !logCont[i].is_open() )
        {
            logCont[i].open((outputDir + outputName + "car" + patch::to_string(i)
                             + "cont.out").c_str());//,std::ios::app);
        }
        else
        {
            std::cout << "Could not open file!\n" << std::endl;
        }
        /*
         * format: time [s]| DynGCg.pos.x [m] | y | z | ax [rad?] | ay | az |
         *                   DynGCg.vel.x [m/s] | y | z | ax [rad/s?] | ay | az |
         *                   DynGCg.acc.x [m/s²] | y | z | ax [rad/s²?] | ay | az |
         */
        logCont[i] << "# 1:time[s] | 2:DynGCg.pos.x [m] | 3:pos.y | 4:pos.z | 5:pos.ax [rad?] | 6:pos.ay | 7:pos.az |" <<
                   " 8:DynGCg.vel.x [m/s] | 9:vel.y | 10:vel.z | 11:vel.ax [rad/s?] | 12:vel.ay | 13:vel.az |" <<
                   " 14:DynGCg.acc.x [m/s^2] | 15:acc.y | 16:acc.z | 17:acc.ax [rad/s^2?] | 18:acc.ay | 19:acc.az |" <<
                   " 20:DynGC.vel.x [m] | 21:vel.y | 22:vel.z | 23:foeX | 24:foeY | 25:DynGC.vel.az"<< std::endl;

        if( !logInit[i].is_open() )
        {
            logInit[i].open((outputDir + outputName + "car" + patch::to_string(i)
                             + "init.out").c_str());//,std::ios::app);
        }
        else
        {
            std::cerr << "Could not open file!\n" << std::endl;
        }
        logInit[i] << "# 1:time[s] | 2:bonnetPos.x | 3:bonnetPos.y | 4:bonnetPos.z |" <<
                      " 5:statGC.x | 6:statGC.y | 7:statGC.z | 8:drvPos.x | 9:drvPos.y | 10:drvPos.z |"
                      " 11:focal length [pixels]" << std::endl;
    }
}

groundTruthLog::~groundTruthLog()
{
    for (int i = 0; i < nCars; ++i)
    {
        if (i >= 2)
        {
            // currently only two cars logged
            break;
        }
        logCont[i].close();
        logInit[i].close();
    }
}


int groundTruthLog::logContinuous(const tSituation * s)
{
    for (int i = 0; i < nCars; ++i)
    {
        if (i >= 2)
        {
            // currently only two cars logged
            break;
        }
        tPosd pos = s->cars[i]->pub.DynGCg.pos;
        tPosd vel = s->cars[i]->pub.DynGCg.vel;
        tPosd acc = s->cars[i]->pub.DynGCg.acc;
        tPosd velCar = s->cars[i]->pub.DynGC.vel;
        double foeX, foeY;
        if (focalPixel==0)
        {
            foeX = image_width/2;
            foeY = image_height/2;
        }
        else
        {
            // compute the foe from the reprojection of the cars/cameras velocity to the image plane
            // using pinhole camera model (http://docs.opencv.org/2.4/modules/calib3d/doc/camera_calibration_and_3d_reconstruction.html)
            // note the orientation of the cars axes is not in the direction of the camera axes assumed by the above model
            // the output pixel coordinate system is with the origin in the lower left corner (to match jAER)
            if (sqrt(velCar.x*velCar.x + velCar.y*velCar.y + velCar.z*velCar.z) > minVelFoe)
            {
                foeX = focalPixel * (-velCar.y)/velCar.x + image_width/2;
                foeY = image_height - ( focalPixel * (-velCar.z)/velCar.x + image_height/2 );
            }
            else
            {
                foeX = image_width/2;
                foeY = image_height/2;
            }
        }
        logCont[i] << s->currentTime << " "
                   << pos.x << " " << pos.y << " " << pos.z << " " << pos.ax << " " << pos.ay << " " << pos.az << " "
                   << vel.x << " " << vel.y << " " << vel.z << " " << vel.ax << " " << vel.ay << " " << vel.az << " "
                   << acc.x << " " << acc.y << " " << acc.z << " " << acc.ax << " " << acc.ay << " " << acc.az << " "
                   << velCar.x << " " << velCar.y << " " << velCar.z << " " << foeX << " " << foeY << " " << velCar.az << std::endl;
    }

    return 1;
}


int groundTruthLog::logInitial(const tSituation * s, const float camFovY)
{
    //TODO: find out what other camera settings are necessary
    for (int i = 0; i < nCars; ++i)
    {
        if (i >= 2)
        {
            // currently only two cars logged
            break;
        }
        tInitCar curCar = s->cars[i]->info;
        focalPixel = image_height/2/tan(camFovY*M_PI/360);
        logInit[i] << s->currentTime << " "
                   << curCar.bonnetPos.x << " " << curCar.bonnetPos.y << " " << curCar.bonnetPos.z << " "
                   << curCar.statGC.x << " " << curCar.statGC.y << " " << curCar.statGC.z << " "
                   << curCar.drvPos.x << " " << curCar.drvPos.y << " " << curCar.drvPos.z << " "
                   << focalPixel << std::endl;
    }

    return 1;
}
