/* 
 * File:   Geofence.hpp
 * Author: jonpetersen
 *
 * Created on April 27, 2015, 5:04 PM
 */

#ifndef GEOFENCE_HPP
#define	GEOFENCE_HPP

#include <iostream>
#include <vector>
#include <assert.h>

using namespace std;

class Geofence {
   
public:
    
    // specify whether the Geofence applies to request, origin, or entire trip
    enum Type{
        REQ_ONLY,
        ORIG_ONLY,
        ENTIRE_TRIP
    };     
    
   // Geofence( const Type geofenceType, const int geofenceID, const std::string cityStr );
    Geofence( const Type geofenceType );
    virtual ~Geofence();
            
    // append curr lat or lng
    void addLatLng(std::pair<double,double> &currVertex);
    
    // getters
    const Type getGeofenceType() const { return _geofenceType; }
   // const int getGeofenceID() const { return _geofenceID; }
   // const std::string getGeofenceCityString() const { return _cityString; }
    const std::vector<double> * getLatitudes() const  { return &_latitudes; }
    const std::vector<double> * getLongitudes() const { return &_longitudes; }
   
private:

    const Type   _geofenceType;
   // const int _geofenceID; 
  //  const std::string _cityString;
    
    std::vector<double> _latitudes;
    std::vector<double> _longitudes;
};

#endif	/* GEOFENCE_HPP */

