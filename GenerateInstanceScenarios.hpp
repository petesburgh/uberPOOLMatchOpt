/* 
 * File:   GenerateInstanceScenarios.hpp
 * Author: jonpetersen
 *
 * Created on April 27, 2015, 5:23 PM
 */

#ifndef GENERATEINSTANCESCENARIOS_HPP
#define	GENERATEINSTANCESCENARIOS_HPP

#include "ProblemInstance.hpp"
#include "CSVRow.hpp"
#include <iostream>
#include<fstream>
#include<sstream>
#include<string>
#include<stdlib.h>  /* atof */
#include <time.h>
#include <map>


using namespace std;

class GenerateInstanceScenarios {
    
public:
    
    enum {
    SF      = 1,
    CHENGDU = 485,
    LA      = 12,
    AUSTIN  = 4, 
    NJ      = 198,
    SD      = 21
    } CityIndex;
    
    GenerateInstanceScenarios(){};
    ~GenerateInstanceScenarios(){};
    
    ProblemInstance * generateInstanceScenarios(const int scenIndex, const std::string &geofenceFolderPath);
    const Geofence * extractGeofence(const Geofence::Type type, const std::string &geofenceDataFile);
   

private:

};

#endif	/* GENERATEINSTANCESCENARIOS_HPP */

