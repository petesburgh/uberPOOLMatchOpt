/* 
 * File:   RouteEvent.cpp
 * Author: jonpetersen
 * 
 * Created on May 14, 2015, 9:32 AM
 */

#include "RouteEvent.hpp"

RouteEvent::RouteEvent(const Request* req, time_t tm, LatLng loc, RouteEventType rtEventType) : pRequest(req), _location(-9999.0,-9999.0) {
    _eventTime = tm;
    _location  = loc;
    _type      = rtEventType;
}
RouteEvent::~RouteEvent() {
}

