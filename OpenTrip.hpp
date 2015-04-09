/* 
 * File:   OpenTrip.hpp
 * Author: jonpetersen
 *
 * Created on March 24, 2015, 10:47 AM
 */

#ifndef OPENTRIP_HPP
#define	OPENTRIP_HPP

#include "Driver.hpp"
#include "Rider.hpp"
#include "Event.hpp"
#include "LatLng.hpp"

#include <iostream>
#include <vector>

using namespace std;

class OpenTrip {
public:
       
    OpenTrip(const std::string driverID, const Driver * driver, const int driverIx, const std::string riderID, const int riderIx, const Event * masterReq, const Event * masterDispatch, const double actualPickupLat, const double actualPickupLng, const double dropReqLat, const double dropReqLng, time_t eta, time_t etd);
    virtual ~OpenTrip();
        
    // setters
    void setIsRiderInitPresent(bool isRiderPres) { _isRiderInitiallyPresent = isRiderPres; }

    // getters
    const std::string getDriverID() { return _driverID; }
    const int getDriverIndex() const { return _driverIndex; }
    const std::string getMasterID() { return _riderID; }
    const int getMasterIndex() const { return _riderIndex; }
    const Event * getMasterRequestEvent() { return pMasterReqEvent; }
    const Event * getMasterDispatcEvent() { return pMasterDispatchEvent; }
    const bool isRiderInitPresentAtStart() { return _isRiderInitiallyPresent; }
    const Driver * getDriver() const { return pDriver; }
    
    const time_t getETA() const { return _eta; } // est time of arrival to pickup master
    const time_t getETD() const { return _etd; } // est time to destination
    
    const double getActPickupLat() const { return _actualPickupLat; }
    const double getActPickupLng() const { return _actualPickupLng; }
    
    const double getDropRequestLat() const { return _dropRequestLat; }
    const double getDropRequestLng() const { return _dropRequestLng; }
    
private:
    
    std::string _driverID; 
    std::string _riderID;
    int _driverIndex;
    int _riderIndex;
    const Event * pMasterReqEvent;
    const Event * pMasterDispatchEvent;  
    //Event _masterPickupEvent;
    
    const Driver * pDriver;
    
    const double _actualPickupLat;
    const double _actualPickupLng;
    
    const double _dropRequestLat;
    const double _dropRequestLng;

    bool _isRiderInitiallyPresent;  
    const time_t _eta;  
    const time_t _etd;
    
};

#endif	/* OPENTRIP_HPP */

