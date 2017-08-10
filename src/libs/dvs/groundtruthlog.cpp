#include "groundtruthlog.h"
#include <iostream>


groundTruthLog::groundTruthLog(int nCars_):
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
                   " 14:DynGCg.acc.x [m/s^2] | 15:acc.y | 16:acc.z | 17:acc.ax [rad/s^2?] | 18:acc.ay | 19:acc.az" << std::endl;

        if( !logInit[i].is_open() )
        {
            logInit[i].open((outputDir + outputName + "car" + patch::to_string(i)
                             + "init.out").c_str());//,std::ios::app);
        }
        else
        {
            std::cerr << "Could not open file!\n" << std::endl;
        }
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
        logCont[i] << s->currentTime << " "
                   << pos.x << " " << pos.y << " " << pos.z << " " << pos.ax << " " << pos.ay << " " << pos.az << " "
                   << vel.x << " " << vel.y << " " << vel.z << " " << vel.ax << " " << vel.ay << " " << vel.az << " "
                   << acc.x << " " << acc.y << " " << acc.z << " " << acc.ax << " " << acc.ay << " " << acc.az << std::endl;
    }

    return 1;
}


int groundTruthLog::logInitial(const tSituation * s)
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
        logInit[i] << s->currentTime << " "
                   << curCar.bonnetPos.x << " " << curCar.bonnetPos.y << " " << curCar.bonnetPos.z << " "
                   << curCar.statGC.x << " " << curCar.statGC.y << " " << curCar.statGC.z << " "
                   << curCar.drvPos.x << " " << curCar.drvPos.y << " " << curCar.drvPos.z << " " << std::endl;
    }

    return 1;
}

int groundTruthLog::logInitial(const float camFovY)
{
    for (int i = 0; i < nCars; ++i)
    {
        if (i >= 2)
        {
            // currently only two cars logged
            break;
        }
        logInit[i] << camFovY << " " << std::endl;
    }

    return 1;
}
