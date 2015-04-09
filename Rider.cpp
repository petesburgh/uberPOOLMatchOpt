/* 
 * File:   Rider.cpp
 * Author: jonpetersen
 * 
 * Created on March 20, 2015, 2:47 PM
 */

#include "Rider.hpp"

Rider::Rider() {
}

Rider::Rider(const Rider& orig) {
}

Rider::~Rider() {
}

void Rider::addTrip(TripData* pTrip) {
    _trips.push_back(pTrip);
}
