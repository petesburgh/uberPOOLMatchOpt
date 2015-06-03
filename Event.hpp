/* 
 * File:   Event.hpp
 * Author: jonpetersen
 *
 * Created on March 18, 2015, 3:20 PM
 */

#ifndef EVENT_HPP
#define	EVENT_HPP

//#include "Utility.hpp"

#include<iostream>
#include<vector>
#include<time.h>

//using namespace std;

struct Event{
    Event() {};
    Event(time_t currTime, const double currLat, const double currLng) : 
        timeT(currTime), lat(currLat), lng(currLng) {};
        
   const std::string getEventStr() const { 
       std::string eventStr = "";
       eventStr += "ABC"; //Utility::convertTimeTToString(timeT); 
       eventStr += "/";
       eventStr += "IJK"; //Utility::doubleToStr(lat);
       eventStr += "/";
       eventStr += "XYZ"; //Utility::doubleToStr(lng);
       return eventStr;
   }
        
    time_t timeT;
    double lat;
    double lng;
};


#endif	/* EVENT_HPP */

