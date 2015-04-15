/* 
 * File:   AssignedTrip.hpp
 * Author: jonpetersen
 *
 * Created on March 25, 2015, 10:01 AM
 */

#ifndef ASSIGNEDTRIP_HPP
#define	ASSIGNEDTRIP_HPP

#include "Request.hpp"
#include "Rider.hpp"
#include "Driver.hpp"
#include "FeasibleMatch.hpp"  /* computing metrics for match */

#include <iostream>
#include <set>
#include <vector>
#include <map>

using namespace std;

class AssignedTrip {
public:
    AssignedTrip(const Driver * driver);
    virtual ~AssignedTrip();
    
    // setters
    void setIndex(int ix) { _index = ix; }
    void setDriver(Driver * driver) { pDriver = driver; }
    
    // set master info
    void setMasterId(const std::string id) { _masterId = id; }
    void setMasterIndex(const int ix) { _masterIndex = ix; }
    void setMasterRequest(Event req) { _masterRequest = req; }
    void setMasterDispatch(Event disp) { _masterDispatch = disp; }
    void setMasterPickupFromActuals(Event pickup) { _masterPickupFromActuals = pickup; }
    void setMasterDropFromActuals(Event drop) { _masterDropFromActuals = drop; }
    //void setMasterPickup(Event pick) { _masterPickup = pick; }
    //void setMasterDrop(Event drop) { _masterDrop = drop; }
    
    // set minion info
    void setMinionId(const std::string id) { _minionId = id; }
    void setMinionIndex(const int ix) { _minionIndex = ix; }
    void setMinionRequest(Event req) { _minionRequest = req; }
    void setMinionDispatch(Event disp) { _minionDispatch = disp; }
    //void setMinionPickup(Event pick) { _minionPickup = pick; }
    //void setMinionDrop(Event drop) { _minionDrop = drop; }
    
    // getters
    const int getIndex() const { return _index; }
    const Driver * getDriver() const { return pDriver; }
    
    // get master info
    const std::string getMasterId() const { return _masterId; }
    const int getMasterIndex() const { return _masterIndex; }
    const Event * getMasterRequestEvent() const { return &_masterRequest; }
    const Event * getMasterDispatchEvent() const { return &_masterDispatch; }
    const Event * getMasterPickupEventFromActuals() const { return &_masterPickupFromActuals; }
    const Event * getMasterDropEventFromActuals() const { return &_masterDropFromActuals; }
    
    // get minion info
    const std::string getMinionId() const { return _minionId; }
    const int getMinionIndex() const { return _minionIndex; }
    const Event * getMinionRequestEvent() const { return &_minionRequest; }
    const Event * getMinionDispatchEvent() const { return &_minionDispatch; }
    
    // check if trip is matched   
    const bool isMatchedTrip() const { return (_minionIndex != -1); }
    
    // set match containing details
    void setMatch(FeasibleMatch * match) { pMatch = match; }
    const FeasibleMatch * getMatchDetails() const { return pMatch; }
    
private:
    
    int _index; 
    
    const Driver * pDriver;
    
    // master info
    std::string _masterId;
    int _masterIndex;
    Event _masterRequest;
    Event _masterDispatch;
    Event _masterPickupFromActuals;
    Event _masterDropFromActuals;
    
    // minion info
    std::string _minionId;
    int _minionIndex;
    Event _minionRequest;
    Event _minionDispatch;
    
    FeasibleMatch * pMatch;
};

#endif	/* ASSIGNEDTRIP_HPP */

