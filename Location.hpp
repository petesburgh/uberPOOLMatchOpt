/* 
 * File:   Location.hpp
 * Author: jonpetersen
 *
 * Created on March 18, 2015, 2:37 PM
 */

#ifndef LOCATION_HPP
#define	LOCATION_HPP

struct Event{
    Event() {};
    Event(int currTime, double currLat, double currLng) : 
        time(currTime), lat(currLat), lng(currLng) {};
        
    int    time;
    double lat;
    double lng;
};



#endif	/* LOCATION_HPP */

