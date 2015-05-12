/* 
 * File:   ProblemInstance.hpp
 * Author: jonpetersen
 *
 * Created on April 27, 2015, 4:59 PM
 */

#ifndef PROBLEMINSTANCE_HPP
#define	PROBLEMINSTANCE_HPP

#include "Geofence.hpp"
#include <iostream>
#include <time.h>
#include <vector>

class ProblemInstance {
public:
    ProblemInstance(int cityIndex, const std::string simStartTimeString, int simLengthMin, std::string inputFileName, const Geofence * geofence, std::string scenString);
    virtual ~ProblemInstance();
    
    const int   getCityIndex() const { return _cityIndex; }
    const std::string getSimStartTimeString() const { return _simStartTimeString; }
    const int   getSimLengthInMin() const { return _simLengthInMin; }
    const std::string getInputCsvFilename() const { return _inputFileName; }
    const Geofence* getGeofence() const { return pGeofence; }
    const std::string getScenString() const { return _scenString; }
    
private:
    const int           _cityIndex;
    const std::string   _simStartTimeString;
    const int           _simLengthInMin;
    const std::string   _inputFileName;
    const Geofence*     pGeofence;
    const std::string   _scenString;
};

#endif	/* PROBLEMINSTANCE_HPP */

