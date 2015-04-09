/* 
 * File:   Event.hpp
 * Author: jonpetersen
 *
 * Created on March 18, 2015, 3:20 PM
 */

#ifndef EVENT_HPP
#define	EVENT_HPP

#include<iostream>
#include<vector>
#include<time.h>

#include "Utility.hpp"

using namespace std;

struct Event{
    Event() {};
    Event(time_t currTime, const double currLat, const double currLng) : 
        timeT(currTime), lat(currLat), lng(currLng) {};
        
   const std::string getEventStr() const { 
       std::string eventStr = "";
       eventStr += Utility::convertTimeTToString(timeT); 
       eventStr += "/";
       eventStr += Utility::doubleToStr(lat);
       eventStr += "/";
       eventStr += Utility::doubleToStr(lng);
       return eventStr;
   }
        
    time_t timeT;
    double lat;
    double lng;
};


#endif	/* EVENT_HPP */

