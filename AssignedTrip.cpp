/* 
 * File:   AssignedTrip.cpp
 * Author: jonpetersen
 * 
 * Created on March 25, 2015, 10:01 AM
 */

#include "AssignedTrip.hpp"

AssignedTrip::AssignedTrip(const Driver * driver) : pDriver(driver)  {
    _minionId = "none";
    _minionIndex = -1;
    pMatch = NULL;
}

AssignedTrip::~AssignedTrip() {
}

void AssignedTrip::setMasterIndex(const int ix) {
    if( ix == 90 ) {
        std::cout << "\t*** ASSIGNING TRIP WITH MASTER INDEX " << ix << " ***" << std::endl;
    }
    this->_masterIndex = ix;
}