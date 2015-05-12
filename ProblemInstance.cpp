/* 
 * File:   ProblemInstance.cpp
 * Author: jonpetersen
 * 
 * Created on April 27, 2015, 4:59 PM
 */

#include "ProblemInstance.hpp"

ProblemInstance::ProblemInstance(int cityIndex, const std::string simStartTimeString, int simLengthMin, std::string inputFileName, const Geofence* geofence, std::string scenString) :
        _cityIndex(cityIndex), _simStartTimeString(simStartTimeString), _simLengthInMin(simLengthMin), _inputFileName(inputFileName), pGeofence(geofence), _scenString(scenString) {
}

ProblemInstance::~ProblemInstance() {
}

