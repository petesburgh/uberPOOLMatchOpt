/* 
 * File:   TripData.hpp
 * Author: jonpetersen
 *
 * Created on March 24, 2015, 10:18 AM
 */

#ifndef TRIPDATA_HPP
#define	TRIPDATA_HPP

#include "Event.hpp"

#include <iostream>
#include <vector>
#include <map>
#include <assert.h>

class TripData {
public:
    
    enum Status {
        COMPLETE,
        PREDISPATCH,
        DEADHEAD,
        ENROUTE,
        FUTURE,
        UNKNOWN, 
        UNINITIALIZED // initial value in constructor
    };
    
    TripData();
    TripData(const TripData& orig);
    virtual ~TripData();
    
   // methods to compute trip metrics
    const int getWaitTimeInSec() const;
    const int getDriverPickupTimeInSec() const;
    const int getEnRouteTimeInSec() const;
    const int getTotalTripTimeInSec() const;
    
    // get status of trip given time
    TripData::Status getTripStatusDuringTime(time_t tm);
    
    // define if a trip is POOL
    void definePOOLTrip() { _isPOOL = true; }
    
    // validate trip data (ensure sequence of events makes sense)
    bool validateTrip();
    
    // setters
    void setIndex(int index) { _index = index; }
    void setUniqueTripID(std::string id) { _uniqueTripID = id; }
    void setRiderID(std::string id) { _riderID = id; }
    void setDriverID(std::string id) { _driverID = id; }
    void setRiderIndex(int ix) { _riderIndex = ix; }
    void setDriverIndex(int ix) { _driverIndex = ix; }
    void setRequestEvent(Event req) { _requestEvent = req; }
    void setDispatchEvent(Event dispatch) { _dispatchEvent = dispatch; }
    void setPickupEvent(Event pickup) { _pickupEvent = pickup; }
    void setDropoffEvent(Event dropoff) { _dropoffEvent = dropoff; }
    void setTripStatus(Status status) { _tripStatus = status; }
        
    // getters
    const int getIndex() const { return _index; }
    const std::string getUniqueTripID() const { return _uniqueTripID; }
    const std::string getRiderID() const { return _riderID; }
    const std::string getDriverID() const { return _driverID; }
    const int getRiderIndex() const { return _riderIndex; }
    const int getDriverIndex() const { return _driverIndex; }
    const Event* getRequestEvent() const { return &_requestEvent; }
    const Event* getDispatchEvent() const { return &_dispatchEvent; }
    const Event* getPickupEvent() const { return &_pickupEvent; }
    const Event* getDropoffEvent() const { return &_dropoffEvent; }
    const Status getStatus() const { return _tripStatus; }
    const bool isPOOL() const { return _isPOOL; }
    
    const std::string getStatusString() const;
        
private:
    
    int _index;
    std::string _uniqueTripID;
    std::string _riderID;
    std::string _driverID;
    int _riderIndex;
    int _driverIndex;
    Event _requestEvent;
    Event _pickupEvent;
    Event _dropoffEvent; 
    Event _dispatchEvent;
    bool _isPOOL;
    Status _tripStatus;
};

#endif	/* TRIPDATA_HPP */

