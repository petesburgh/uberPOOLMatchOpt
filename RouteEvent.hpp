/* 
 * File:   RouteEvent.hpp
 * Author: jonpetersen
 *
 * Created on May 14, 2015, 9:32 AM
 */

#ifndef ROUTEEVENT_HPP
#define	ROUTEEVENT_HPP

#include "LatLng.hpp"
#include "Request.hpp"
#include <iostream>
#include <time.h>

using namespace std;

class RouteEvent {
public:
    
    enum RouteEventType {PICKUP,DROPOFF};
    
    RouteEvent(const Request* req, time_t tm, LatLng loc, RouteEventType rtEventType);
    virtual ~RouteEvent();
    
    // method may be used to reset event time 
    void resetEventTime(time_t newEventTime) { _eventTime = newEventTime; }
             
    // getters
    const Request * getRequest() const { return pRequest; }
    time_t getEventTime() const { return _eventTime; }
    const LatLng * getLocation() const { return &_location; }
    const double getLat() const { return _location.getLat(); }
    const double getLng() const { return _location.getLng(); }
    const RouteEventType getRouteEventType() const { return _type; }
    
private:
    const Request * pRequest;
    time_t _eventTime;
    LatLng _location;
    enum RouteEventType _type;
};

#endif	/* ROUTEEVENT_HPP */

