/* 
 * File:   OpenTrip.cpp
 * Author: jonpetersen
 * 
 * Created on March 24, 2015, 10:47 AM
 */

#include "OpenTrip.hpp"

OpenTrip::OpenTrip(const std::string driverID, const Driver * driver, const int driverIx, const std::string riderID, const int riderIx, const Event * masterReq, const Event * masterDispatch, const double actualPickupLat, const double actualPickupLng, const double dropReqLat, const double dropReqLng, time_t eta, time_t etd) 
        : pDriver(driver), pMasterReqEvent(masterReq), pMasterDispatchEvent(masterDispatch), _actualPickupLat(actualPickupLat), _actualPickupLng(actualPickupLng), _dropRequestLat(dropReqLat), _dropRequestLng(dropReqLng), _eta(eta), _etd(etd) {
    _driverID = driverID;
    _driverIndex = driverIx;
    _riderID = riderID;
    _riderIndex = riderIx;   
    
    if( _riderIndex == 90 ) {
        std::cout << "\t** ADDING OPEN TRIP WITH MASTER INDEX 90 **" << std::endl;
    }
}

/*OpenTrip::OpenTrip(const OpenTrip& orig) : _eta(-1) {
}*/

OpenTrip::~OpenTrip() {
}
