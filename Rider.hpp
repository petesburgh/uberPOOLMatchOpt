/* 
 * File:   Rider.hpp
 * Author: jonpetersen
 *
 * Created on March 20, 2015, 2:47 PM
 */

#ifndef RIDER_HPP
#define	RIDER_HPP

#include "TripData.hpp"
#include <iostream>
#include <vector>
#include <map>

using namespace std;

class Rider {
public:
    Rider();
    Rider(const Rider& orig);
    virtual ~Rider();
    
    bool operator<(Rider other) {
        return _riderID < other._riderID;
    }
    
    void addTrip(TripData* pTrip);
    
    // setters
    void setIndex(int ix) { _index = ix; }
    void setID(const std::string id) { _riderID = id; }
    
    // getters
    const int getIndex() const { return _index; }
    const std::string getRiderID() const { return _riderID; }
    const std::vector<TripData*>* getTrips() const { return &_trips; }
    
private:
    std::string _riderID;           // string index as defined in DB snapshot
    int _index;                     // numeric identifier
    std::vector<TripData*> _trips;
    
};

#endif	/* RIDER_HPP */

