/* 
 * File:   Geofence.cpp
 * Author: jonpetersen
 * 
 * Created on April 27, 2015, 5:04 PM
 */

#include "Geofence.hpp"

#include <iostream>

/*Geofence::Geofence(const Geofence::Type geofenceType, const int geofenceID, const std::string cityStr) :
         _geofenceType(geofenceType), _geofenceID(geofenceID), _cityString(cityStr) {
}*/

Geofence::Geofence( const Type geofenceType ) : _geofenceType(geofenceType) {    
}

Geofence::~Geofence() {
}

void Geofence::addLatLng(std::pair<double,double> &currVertex) {
    _latitudes.push_back(currVertex.first);
    _longitudes.push_back(currVertex.second);
}
