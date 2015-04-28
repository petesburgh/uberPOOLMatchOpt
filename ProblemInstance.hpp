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
    ProblemInstance(int cityIndex, const std::string simStartTimeString, int simLengthMin, std::string inputFileName, std::vector<Geofence*> geofences, std::string scenString);
    virtual ~ProblemInstance();
    
    const int   getCityIndex() const { return _cityIndex; }
    const std::string getSimStartTimeString() const { return _simStartTimeString; }
    const int   getSimLengthInMin() const { return _simLengthInMin; }
    const std::string getInputCsvFilename() const { return _inputFileName; }
    const std::vector<Geofence*>* getGeofences() const { return &_geofenceVec; }
    const std::string getScenString() const { return _scenString; }
    
private:
    const int           _cityIndex;
    const std::string   _simStartTimeString;
    const int           _simLengthInMin;
    const std::string   _inputFileName;
    const std::vector<Geofence*> _geofenceVec;
    const std::string   _scenString;
};

#endif	/* PROBLEMINSTANCE_HPP */

