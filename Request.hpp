/* 
 * File:   Request.hpp
 * Author: jonpetersen
 *
 * Created on March 23, 2015, 5:05 PM
 */

#ifndef REQUEST_HPP
#define	REQUEST_HPP

#include "Driver.hpp"
#include "Event.hpp"
#include "LatLng.hpp"

#include <iostream>
#include <vector>
#include <set>
#include <time.h>

using namespace std;

class Request {
public:
    Request(const std::string riderID, const int riderIndex, const std::string riderUUID, const Driver * actDriver, const Event * pickupRequest, LatLng dropLocation, time_t actPickupTimeFromSched, time_t actDropoffTimeFromSched, const Event * actReqEvent, const Event * actDispatchEvent, const Event * actPickupEvent, const Event * actualDropEvent);
    virtual ~Request();
    
    // setters
    void setInitRequest(bool isInit) { _isInitReq = isInit; }
    void setIndex(int ix) { _reqIndex = ix; }
    
    // getters    
    const int getReqIndex() const { return _reqIndex; }
    const int getRiderIndex() const { return _riderIndex; }
    const std::string getRiderID() const { return _riderID; }    
    const Driver * getActualDriver() const { return pDriver; }
    const time_t getReqTime() const {return _requestTime; }
    const double getPickupLat() const { return _pickupLat; }
    const double getPickupLng() const { return _pickupLng; }
    const double getDropoffLat() const { return _dropoffLat; }
    const double getDropoffLng() const { return _dropoffLng; }
    const bool isInitRequest() const { return _isInitReq; }
    
    const time_t getActTimeOfPickupFromTripActuals() const { return _pickupTimeFromSched; }
    const time_t getActTimeOfDropoffFromTripActuals() const { return _dropoffTimeFromSched; }
    
    const Event * getActualDispatchEvent() const { return pActualDispatchEvent; }
    
    const std::string getRiderTripUUID() const { return _riderTripUUID; }
    
    const Event * getActualRequestEvent() const { return pActualReqEvent; }
    const Event * getActualPickupEvent() const { return pActualPickupEvent; }
    const Event * getActualDropEvent() const { return pActualDropEvent; }
  
    // define ordering by request time
    bool operator<(Request other) const {
        return (_requestTime < other.getReqTime());
    }
    
private:
    int _reqIndex;
    const int _riderIndex;    
    std::string _riderID;
    time_t _requestTime;    
    double _pickupLat;
    double _pickupLng;
    double _dropoffLat;
    double _dropoffLng;
    bool _isInitReq;
    const std::string _riderTripUUID;
    
    const time_t _pickupTimeFromSched;
    const time_t _dropoffTimeFromSched;
    
    const Driver * pDriver;
    const Event * pActualReqEvent;
    const Event * pActualDispatchEvent;
    const Event * pActualPickupEvent;
    const Event * pActualDropEvent;
    
};

#endif	/* REQUEST_HPP */

