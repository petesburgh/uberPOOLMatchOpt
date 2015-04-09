/* 
 * File:   Driver.cpp
 * Author: jonpetersen
 * 
 * Created on March 18, 2015, 2:48 PM
 */

#include "Driver.hpp"

Driver::Driver() {
}

Driver::Driver(const Driver& orig) {
}

Driver::~Driver() {
}

void Driver::addTrip(TripData* pTrip) {
    if( pTrip != NULL ) {
        _trips.push_back(pTrip);
    }
}

