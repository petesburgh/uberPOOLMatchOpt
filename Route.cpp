/* 
 * File:   Route.cpp
 * Author: jonpetersen
 * 
 * Created on May 14, 2015, 9:02 AM
 */

#include "Route.hpp"

Route::Route(int ix, const Driver * driver, const Event * dispatch) : _routeIndex(ix), pDriver(driver), pDispatchEvent(dispatch) {
}

Route::~Route() {
}

void Route::addPickupEvent(const Request* pReq, time_t tm, double lat, double lng) {
    LatLng loc(lat,lng);
    RouteEvent * pRouteEvent = new RouteEvent(pReq,tm,loc,RouteEvent::PICKUP);
    _pickupEvents.push_back(pRouteEvent);
    _pickupEventTimeMap.insert(make_pair(pRouteEvent,tm));
}

void Route::addDropoffEvent(const Request* pReq, time_t tm, double lat, double lng) {
    LatLng loc(lat,lng);
    RouteEvent * pRouteEvent = new RouteEvent(pReq,tm,loc,RouteEvent::DROPOFF);
    _dropoffEvents.push_back(pRouteEvent);
    _dropoffEventTimeMap.insert(make_pair(pRouteEvent,tm));
}

void Route::insertPickupEvent(RouteEvent* pickupEvent, int position) {
    int index = position-1;
    std::vector<RouteEvent*>::iterator itr = _pickupEvents.begin()+index;
    _pickupEvents.insert(itr, pickupEvent);
    _pickupEventTimeMap.insert(make_pair(pickupEvent, pickupEvent->getEventTime()));
}

void Route::insertDropoffEvent(RouteEvent* dropoffEvent, int position) {
    int index = position-1;
    std::vector<RouteEvent*>::iterator itr = _dropoffEvents.begin()+index;
    _dropoffEvents.insert(itr, dropoffEvent);
    _dropoffEventTimeMap.insert(make_pair(dropoffEvent, dropoffEvent->getEventTime()));
}

bool Route::updateTimes() {
    
    bool timesUpdated = false; 
    
    // compute average speed based on master trip
    const Request * pFirstReq = _requests.front();
    const Event * pPickupEvent = pFirstReq->getActualPickupEvent();
    const Event * pDropEvent   = pFirstReq->getActualDropEvent();
        
    double avgSpeed = Utility::computeAvgSpeed(pPickupEvent->timeT, pPickupEvent->lat, pPickupEvent->lng, pDropEvent->timeT, pDropEvent->lat, pDropEvent->lng);  
    
    // modify pickup times
    std::vector<RouteEvent*>::iterator pickupItr;
    for( int ii = 1; ii < _pickupEvents.size(); ii++ ) {
        RouteEvent * previousEvent = _pickupEvents.at(ii-1);
        RouteEvent * currEvent = _pickupEvents.at(ii);
                
        std::map<RouteEvent*, time_t>::iterator previousPickupItr = _pickupEventTimeMap.find(previousEvent);
        if( previousPickupItr != _pickupEventTimeMap.end() ) {
            time_t previousPickupTime = previousPickupItr->second;
            
            const double distToPickupKm = Utility::computeGreatCircleDistance(previousEvent->getLat(), previousEvent->getLng(), currEvent->getLat(), currEvent->getLng());
            const int estTransitTime = (int)(avgSpeed * distToPickupKm);
            time_t estPickupTime = previousEvent->getEventTime() + estTransitTime;
            
            // if the estimated pickup time is later than the actual pickup event time, reset the time accordingly
            if( estPickupTime > currEvent->getEventTime() ) {
                std::map<RouteEvent*, time_t>::iterator currEventPickupItr = _pickupEventTimeMap.find(currEvent);
                assert( currEventPickupItr != _pickupEventTimeMap.end() );
                currEventPickupItr->second = estPickupTime;
                currEvent->resetEventTime(estPickupTime);
                timesUpdated = true;
            }
        }
        
    }
    
    // update the initial drop time if necessary
    RouteEvent * pLastPickup = _pickupEvents.back();
    std::map<RouteEvent*, time_t>::iterator lastPickupItr = _pickupEventTimeMap.find(pLastPickup);
    assert( lastPickupItr != _pickupEventTimeMap.end() );
    time_t lastPickupTime = lastPickupItr->second;
    
    RouteEvent * pFirstDrop = _dropoffEvents.front();
    const double sharedDist = Utility::computeGreatCircleDistance(pLastPickup->getLat(), pLastPickup->getLng(), pFirstDrop->getLat(), pFirstDrop->getLng());
    
    const int estTimeOnSharedDist = (int)(avgSpeed * sharedDist);
    time_t estTimeFirstDrop = lastPickupTime + estTimeOnSharedDist;
    
    std::map<RouteEvent*, time_t>::iterator initDropItr = _dropoffEventTimeMap.find(pFirstDrop);
    assert( initDropItr != _dropoffEventTimeMap.end() );    
    if( estTimeFirstDrop > initDropItr->second ) {
        initDropItr->second = estTimeFirstDrop;
        initDropItr->first->resetEventTime(estTimeFirstDrop);
        timesUpdated = true;
    }
    
    // update future drop times
    for( int jj = 1; jj < _dropoffEvents.size(); jj++ ) {
        RouteEvent * pPrevDrop = _dropoffEvents.at(jj-1);
        RouteEvent * pCurrDrop = _dropoffEvents.at(jj);
        
        std::map<RouteEvent*, time_t>::iterator previousDropItr = _dropoffEventTimeMap.find(pPrevDrop);
        assert( previousDropItr != _dropoffEventTimeMap.end() );
        time_t previousDropTime = previousDropItr->second;
        
        const double distBetweenDrops = Utility::computeGreatCircleDistance(pPrevDrop->getLat(), pPrevDrop->getLng(), pCurrDrop->getLat(), pCurrDrop->getLng());
        int estTimeBetweenDrops = (int)(avgSpeed * distBetweenDrops);
       
        time_t estTimeAtCurrDrop = previousDropTime + estTimeBetweenDrops;
        
        // update if est time is after actual time
        std::map<RouteEvent*, time_t>::iterator currDropItr = _dropoffEventTimeMap.find(pCurrDrop);
        assert( currDropItr != _dropoffEventTimeMap.end() );
        if( estTimeAtCurrDrop > currDropItr->second ) {
            currDropItr->second = estTimeAtCurrDrop;
            currDropItr->first->resetEventTime(estTimeAtCurrDrop);
            timesUpdated = true;
        }                
    }
    
    return timesUpdated;
}

void Route::print() {
    std::vector<RouteEvent*> _routeEvents;
    _routeEvents.insert(_routeEvents.end(), _pickupEvents.begin(), _pickupEvents.end());
    _routeEvents.insert(_routeEvents.end(), _dropoffEvents.begin(), _dropoffEvents.end());
    
    std::cout << "\n\troute " << Utility::intToStr(_routeIndex) << std::endl;
    std::cout << "\t  dispatched at " << Utility::convertTimeTToString(getDispatchTime()) << std::endl;
    std::cout << "\t  path: " << std::endl;
    for( std::vector<RouteEvent*>::iterator itr = _routeEvents.begin(); itr != _routeEvents.end(); ++itr ) {
        std::string eventStr = ((*itr)->getRouteEventType() == RouteEvent::PICKUP ) ? "pickup" : "dropoff";        
        std::cout << "\t    " << left << setw(9) << eventStr << 
                left << setw(5) << Utility::intToStr((*itr)->getRequest()->getRiderIndex()) << " / " << 
                left << Utility::convertTimeTToString((*itr)->getEventTime()) << " / " << 
                left << Utility::convertToLatLngStr(*((*itr)->getLocation()), 4) << std::endl; 
    }
}