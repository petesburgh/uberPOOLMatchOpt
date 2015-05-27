/* 
 * File:   Route.hpp
 * Author: jonpetersen
 *
 * Created on May 14, 2015, 9:02 AM
 */

#include "Driver.hpp"
#include "RouteEvent.hpp"
#include "LatLng.hpp"

#include <iostream>
#include <time.h>
#include <vector>

#ifndef ROUTE_HPP
#define	ROUTE_HPP

using namespace std;

class Route {
        
public:
    
    struct RiderProperties {
        Request * pRequest;
        double uberXDist;
        double pooledDist;
        double savings;
        double inconv;
    };
    
    Route(int ix, const Driver * driver, const Event * dispatch);
    virtual ~Route();
    
    void addRequest(Request * pCurrReq) { _requests.push_back(pCurrReq); }
    void addPickupEvent(const Request * pReq, time_t tm, double lat, double lng);
    void addDropoffEvent(const Request * pReq, time_t tm, double lat, double lng);
    void insertPickupEvent (RouteEvent * pickupEvent,  int position);
    void insertDropoffEvent(RouteEvent * dropoffEvent, int position);
        
    // getters
    const int getRouteIndex() const { return _routeIndex; }
    const std::vector<Request*> * getRequests() const { return &_requests; }
    const time_t getInitPickupTime() const { return _pickupEvents.front()->getEventTime(); }
    const time_t getRouteEndTime() const { return _dropoffEvents.back()->getEventTime(); }    
    const int getNumMatchedRiders() const { return _dropoffEvents.size(); }
    const time_t getDispatchTime() const { return pDispatchEvent->timeT; }
    const Request * getPickupRequest(int j) const { return _requests.at(j-1); }
    std::vector<RouteEvent*> * getPickupEvents() { return &_pickupEvents; }
    std::vector<RouteEvent*> * getDropoffEvents() { return &_dropoffEvents; }
    std::map<RouteEvent*, time_t> * getPickupEventTimeMap() { return &_pickupEventTimeMap; }
    std::map<RouteEvent*, time_t> * getDropoffEventTimeMap() { return &_dropoffEventTimeMap; }
    
    const Driver * getDriver() const { return pDriver; }
    const Event * getDispatchEvent() const { return pDispatchEvent; }
    
    // methods to be used in overriding an existing route to a newly appended route
    void setPickupEvents(std::vector<RouteEvent*> pickups) { _pickupEvents = pickups; }
    void setDropoffEvents(std::vector<RouteEvent*> dropoffs) { _dropoffEvents = dropoffs; }
    void setRequests(std::vector<Request*> reqs) { _requests = reqs; }
    
    bool updateTimes();
    
    void print();
    
private:
    const int _routeIndex;
    const Driver * pDriver;
    std::vector<Request*> _requests;
    const Event * pDispatchEvent;
    
    std::vector<RouteEvent*> _pickupEvents;
    std::vector<RouteEvent*> _dropoffEvents;        

    std::map<RouteEvent*, time_t> _pickupEventTimeMap;
    std::map<RouteEvent*, time_t> _dropoffEventTimeMap;
    
    // POOL metrics
    
    // individual rider metrics
    
};

#endif	/* ROUTE_HPP */

