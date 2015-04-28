/* 
 * File:   GenerateInstanceScenarios.hpp
 * Author: jonpetersen
 *
 * Created on April 27, 2015, 5:23 PM
 */

#ifndef GENERATEINSTANCESCENARIOS_HPP
#define	GENERATEINSTANCESCENARIOS_HPP

#include "ProblemInstance.hpp"
//#include "Geofence.hpp"
#include <iostream>
#include <time.h>
#include <map>

using namespace std;

enum {
    SF = 1,
    CHENGDU = 1774
} CityIndex;

ProblemInstance * generateInstanceScenarios(const int scenIndex);

#endif	/* GENERATEINSTANCESCENARIOS_HPP */

