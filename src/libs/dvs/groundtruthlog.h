#ifndef GROUNDTRUTHLOG_H
#define GROUNDTRUTHLOG_H

#include <fstream> //ofstream
#include <sstream> //patch
#include <vector>
#include <string>

#include <raceman.h>

namespace patch
{
    template < typename T > std::string to_string( const T& n )
    {
        std::ostringstream stm ;
        stm << n ;
        return stm.str() ;
    }
}

/*!
 * \brief The groundTruthLog class
 *
 * logs ground truth values to file,
 * to be later used in evaluating performance
 */
class groundTruthLog
{
public:
    groundTruthLog(int nCars_);
    ~groundTruthLog();

    //! set directory of logging files
    void setPlotdir(const std::string dir_) { outputDir = dir_; }
    //! set output_name:
    void setOutputName(std::string output_name_) {outputName = output_name_;}

    //! logs all relevant information for the current race situation
    int logContinuous(const tSituation * s);
    //! logs all relevant initial information
    int logInitial(const tSituation * s);


private:
    //! amount of cars involved -> determines amount of logging files
    int nCars;
    //! output stream for continuous logging, one entry per car
    std::ofstream logCont[2];
    //! output stream for initial logging (camera settings)
    std::ofstream logInit[2];

    //! directory for data logging
    std::string outputDir;
    //! filename of logging file
    std::string outputName;

};

#endif // GROUNDTRUTHLOG_H
