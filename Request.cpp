/* 
 * File:   Request.cpp
 * Author: jonpetersen
 * 
 * Created on March 23, 2015, 5:05 PM
 */

#include "Request.hpp"

Request::Request(const std::string riderID, const int riderIndex, const Driver * driver, const Event * pickupRequest, LatLng dropLocation, time_t actPickupTimeFromSched, time_t actDropoffTimeFromSched, const Event * actDispatchEvent) : 
        _riderIndex(riderIndex), pDriver(driver), _pickupTimeFromSched(actPickupTimeFromSched), _dropoffTimeFromSched(actDropoffTimeFromSched), pActualDispatchEvent(actDispatchEvent) {
    
    this->_riderID     = riderID;    
    this->_requestTime = pickupRequest->timeT;
    this->_pickupLat   = pickupRequest->lat;
    this->_pickupLng   = pickupRequest->lng;
    this->_dropoffLat  = dropLocation.getLat();
    this->_dropoffLng  = dropLocation.getLng();  
}


Request::~Request() {
}

