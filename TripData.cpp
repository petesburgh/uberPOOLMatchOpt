/* 
 * File:   TripData.cpp
 * Author: jonpetersen
 * 
 * Created on March 24, 2015, 10:18 AM
 */

#include "TripData.hpp"

TripData::TripData() {
    _isPOOL = false;
    _tripStatus = TripData::UNINITIALIZED;    
}

TripData::TripData(const TripData& orig) {
}

TripData::~TripData() {
}

bool TripData::validateTrip() {
    assert( _requestEvent.timeT <= _dispatchEvent.timeT );
    assert( _dispatchEvent.timeT <= _pickupEvent.timeT );
    assert( _pickupEvent.timeT < _pickupEvent.timeT );
    
    return true;
}

const int TripData::getWaitTimeInSec() const {
    return ((int)(_pickupEvent.timeT - _requestEvent.timeT));
}
const int TripData::getDriverPickupTimeInSec() const {
    return ((int)(_pickupEvent.timeT - _dispatchEvent.timeT));
}
const int TripData::getEnRouteTimeInSec() const {
    return((int)(_dropoffEvent.timeT - _pickupEvent.timeT));
}
const int TripData::getTotalTripTimeInSec() const {
    return((int)(_dropoffEvent.timeT - _requestEvent.timeT));
}

TripData::Status TripData::getTripStatusDuringTime(time_t tm) {
    
    // case 1: check if trip in completed (i.e. tm < request)
    if( tm < _requestEvent.timeT ) {
        return TripData::FUTURE;
    } 
    
    // case 2: check if trip is pre-dispatch (i.e. request <= tm < dispatch)
    if( (_requestEvent.timeT <= tm) && (tm < _dispatchEvent.timeT) ){
        return TripData::PREDISPATCH;
    }
    
    // case 3: check if trip is deadhead (i.e. dispatch <= tm < pickup )
    if( (_dispatchEvent.timeT <= tm) && (tm < _pickupEvent.timeT) ) {
        return TripData::DEADHEAD;
    }
    
    // case 4: check if trip is en route (i.e. pickup <= tm < dropoff)
    if( (_pickupEvent.timeT <= tm) && (tm < _dropoffEvent.timeT) ) {
        return TripData::ENROUTE;
    }
    
    // case 5: check if trip is completed (i.e. tm >= dropoff)
    if( tm >= _dropoffEvent.timeT ) {
        return TripData::COMPLETE;
    }
    
    return TripData::UNKNOWN;
}

const std::string TripData::getStatusString() const {  
    switch( _tripStatus ) {
        case TripData::COMPLETE :
            return "Completed";
        case TripData::DEADHEAD :
            return "Deadhead";
        case TripData::ENROUTE : 
            return "EnRoute";            
        case TripData::FUTURE : 
            return "Future";
        case TripData::PREDISPATCH : 
            return "Pre-Dispatch";            
        case TripData::UNINITIALIZED : 
            return "Uninitialized";            
        case TripData::UNKNOWN :
            return "UNKNOWN";
        default :
            return "** ERROR **";
    }    
}