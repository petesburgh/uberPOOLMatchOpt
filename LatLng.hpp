/* 
 * File:   LatLng.hpp
 * Author: jonpetersen
 *
 * Created on March 24, 2015, 10:57 AM
 */

#ifndef LATLNG_HPP
#define	LATLNG_HPP

struct LatLng {
    LatLng(double lat, double lng) : _lat(lat), _lng(lng) {};
    const double getLat() const { return _lat; }
    const double getLng() const { return _lng; }
    const double _lat;
    const double _lng;
};



#endif	/* LATLNG_HPP */

