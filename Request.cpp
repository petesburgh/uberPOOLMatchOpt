/* 
 * File:   Request.cpp
 * Author: jonpetersen
 * 
 * Created on March 23, 2015, 5:05 PM
 */

#include "Request.hpp"

Request::Request(const std::string riderID, const int riderIndex, std::string riderTripUUID, const Driver * driver, const Event * pickupRequest, LatLng dropLocation, time_t actPickupTimeFromSched, time_t actDropoffTimeFromSched, const Event * actReqEvent, const Event * actDispatchEvent, const Event * actPickupEvent, const Event * actualDropEvent) : 
        _riderIndex(riderIndex), _riderTripUUID(riderTripUUID), pDriver(driver), _pickupTimeFromSched(actPickupTimeFromSched), _dropoffTimeFromSched(actDropoffTimeFromSched), pActualReqEvent(actReqEvent),  pActualDispatchEvent(actDispatchEvent), pActualPickupEvent(actPickupEvent), pActualDropEvent(actualDropEvent) {
    
    this->_riderID       = riderID;    
    this->_requestTime   = pickupRequest->timeT;
    this->_pickupLat     = pickupRequest->lat;
    this->_pickupLng     = pickupRequest->lng;
    this->_dropoffLat    = dropLocation.getLat();
    this->_dropoffLng    = dropLocation.getLng();  
}


Request::~Request() {
}

