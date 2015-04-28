/* 
 * File:   Geofence.hpp
 * Author: jonpetersen
 *
 * Created on April 27, 2015, 5:04 PM
 */

#ifndef GEOFENCE_HPP
#define	GEOFENCE_HPP

class Geofence {
   
public:
    
    // specify whether the Geofence applies to request, origin, or entire trip
    enum Type{
        REQ_ONLY,
        ORIG_ONLY,
        ENTIRE_TRIP
    };     
    
    Geofence( Type geofenceType, double minLat, double maxLat, double minLng, double maxLng);
    virtual ~Geofence();
    
    const Type getGeofenceType() const { return _geofenceType; }
    const double getMinLat() const { return _minLat; }
    const double getMaxLat() const { return _maxLat; }
    const double getMinLng() const { return _minLng; }
    const double getMaxLng() const { return _maxLng; }
    
private:

    const Type   _geofenceType;
    const double _minLat;
    const double _maxLat;
    const double _minLng;
    const double _maxLng;
};

#endif	/* GEOFENCE_HPP */

