/* 
 * File:   Geofence.cpp
 * Author: jonpetersen
 * 
 * Created on April 27, 2015, 5:04 PM
 */

#include "Geofence.hpp"

#include <iostream>

Geofence::Geofence(Geofence::Type geofenceType, double minLat, double maxLat, double minLng, double maxLng) :
        _geofenceType(geofenceType), _minLat(minLat), _maxLat(maxLat), _minLng(minLng), _maxLng(maxLng) {
}

Geofence::~Geofence() {
}

