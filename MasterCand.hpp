/* 
 * File:   MasterCand.hpp
 * Author: jonpetersen
 *
 * Created on April 23, 2015, 7:48 PM
 */

#ifndef MASTERCAND_HPP
#define	MASTERCAND_HPP

#include "Event.hpp"
#include "Request.hpp"
#include "LatLng.hpp"
#include "Driver.hpp"
#include <time.h>
#include <iostream>

struct MasterCand {

    MasterCand(const Event * reqEvent, const time_t reqTime, const LatLng reqOrig, const LatLng reqDest, const time_t eta, const time_t etd, const Driver * driver,
               const Event * dispatchEvent, const Event * pickupEvent, const Event * drop, const int riderIx, const std::string riderID, const std::string riderTripUUID, const int reqIndex) :
        pReqEvent(reqEvent), _reqTime(reqTime), _reqOrig(reqOrig), _reqDest(reqDest), _ETA(eta), _ETD(etd), 
        pDriver(driver), pDispatchEvent(dispatchEvent), pPickupEvent(pickupEvent), pDropEvent(drop), 
        _riderIndex(riderIx), _riderID(riderID), _riderTripUUID(riderTripUUID), _requestIndex(reqIndex) {};

    const Event * pReqEvent;
    const time_t _reqTime;
    const LatLng _reqOrig;
    const LatLng _reqDest;                
    const time_t _ETA;
    const time_t _ETD;     
    const Driver * pDriver;
    const Event * pDispatchEvent;
    const Event * pPickupEvent;
    const Event * pDropEvent;
    const int _riderIndex;
    const std::string _riderID;
    const std::string _riderTripUUID;
    const int _requestIndex;

    bool operator= (const MasterCand& other) {
        return (this->_riderIndex == other._riderIndex);
    }
};


#endif	/* MASTERCAND_HPP */

