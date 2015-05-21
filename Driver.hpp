/* 
 * File:   Driver.hpp
 * Author: jonpetersen
 *
 * Created on March 18, 2015, 2:48 PM
 */

#ifndef DRIVER_HPP
#define	DRIVER_HPP

#include <iostream>
#include <vector>
#include "TripData.hpp"
using namespace std;

class Driver {
public:
    Driver();
    Driver(const Driver& orig);
    virtual ~Driver();
    
    void addTrip(TripData* pTrip);
    
    // setters
    void setIndex(int ix) { _index = ix; }
    void setId(std::string id) { _driverID = id; }
    void setCapacity(int cap) { _capacity = cap; }
        
    // getters
    const int getIndex() const { return _index; }
    const std::string getId() const { return _driverID; }
    const std::vector<TripData*>* getTrips() const { return &_trips; }
    const int getCapacity() const { return _capacity; }
    
private:
    
    int _index;
    std::string _driverID;
    std::vector<TripData*> _trips;
    int _capacity;

};

#endif	/* DRIVER_HPP */

