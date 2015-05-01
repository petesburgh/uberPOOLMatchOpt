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
    AssignedTrip(const Driver * driver, const Event * masterDispatchEvent, const std::string masterTripUUID, const Event * masterRequest, const Event * actualPickupForMaster, const Event * actualDropForMaster, const int masterReqIndex);
    virtual ~AssignedTrip();
    
    // setters
    void setIndex(int ix) { _index = ix; }
    void setDriver(Driver * driver) { pDriver = driver; }
    
    // set master info
    void setMasterId(const std::string id) { _masterId = id; }
    void setMasterIndex(const int ix) { _masterIndex = ix; }
        
    // set minion info
    void setMinionId(const std::string id) { _minionId = id; }
    void setMinionIndex(const int ix) { _minionIndex = ix; }
    void setMinionDispatch(Event disp) { _minionDispatch = disp; }    
    void setMinionTripUUID(const std::string uuid) { _minionTripUUID = uuid; }
    
    // getters
    const int getIndex() const { return _index; }
    const Driver * getDriver() const { return pDriver; }
    
    // get master info
    const std::string getMasterId() const { return _masterId; }
    const int getMasterIndex() const { return _masterIndex; }
    const Event * getMasterRequestEvent() const { return pMasterRequestEvent; }
    const Event * getMasterDispatchEvent() const { return pMasterDispatchEvent; }
    const Event * getMasterPickupEventFromActuals() const { return pMasterPickupEventFromActuals; }
    const Event * getMasterDropEventFromActuals() const { return pMasterDropEventFromActuals; }
    const std::string getMasterTripUUID() const { return _masterTripUUID; }
    
    // get minion info
    const std::string getMinionId() const { return _minionId; }
    const int getMinionIndex() const { return _minionIndex; }
    const Event * getMinionDispatchEvent() const { return &_minionDispatch; }
    const std::string getMinionTripUUID() const { return _minionTripUUID; }
    
    // check if trip is matched   
    const bool isMatchedTrip() const { return (_minionIndex != -1); }
    
    // set match containing details
    void setMatch(FeasibleMatch * match) { pMatch = match; }
    const FeasibleMatch * getMatchDetails() const { return pMatch; }
    
    const int getMasterReqIndex() const { return _masterReqIndex; }
    
private:
    
    int _index; 
    
    const Driver * pDriver;
    
    // master info
    std::string _masterId;
    int _masterIndex;
    const Event * pMasterRequestEvent;
    const Event * pMasterDispatchEvent;
    const Event * pMasterPickupEventFromActuals;
    const Event * pMasterDropEventFromActuals;
    std::string _masterTripUUID;
    const int   _masterReqIndex;
    
    // minion info
    std::string _minionId;
    int _minionIndex;
    Event _minionDispatch;
    std::string _minionTripUUID;
    
    FeasibleMatch * pMatch;
};

#endif	/* ASSIGNEDTRIP_HPP */

