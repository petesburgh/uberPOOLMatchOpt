/* 
 * File:   AssignedTrip.cpp
 * Author: jonpetersen
 * 
 * Created on March 25, 2015, 10:01 AM
 */

#include "AssignedTrip.hpp"

AssignedTrip::AssignedTrip(const Driver * driver, const Event * masterDispatchEvent, const std::string masterTripUUID, const Event * masterRequest, const Event * actualPickupForMaster, const Event * actualDropForMaster, const int masterReqIndex) : 
        pDriver(driver), pMasterDispatchEvent(masterDispatchEvent), _masterTripUUID(masterTripUUID), pMasterRequestEvent(masterRequest), pMasterPickupEventFromActuals(actualPickupForMaster), pMasterDropEventFromActuals(actualDropForMaster), _masterReqIndex(masterReqIndex) {
    _minionId = "none";
    _minionIndex = -1;
    pMatch = NULL;
    _minionTripUUID = "N/A";
}

AssignedTrip::~AssignedTrip() {
}