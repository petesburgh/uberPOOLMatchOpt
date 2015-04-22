/* 
 * File:   MitmModel.hpp
 * Author: jonpetersen
 *
 * Created on March 24, 2015, 5:40 PM
 */

#ifndef MITMMODEL_HPP
#define	MITMMODEL_HPP

#include "Request.hpp"
#include "OpenTrip.hpp"
#include "Comparators.hpp"
#include "AssignedTrip.hpp"
#include "Event.hpp"
#include "LatLng.hpp"
#include "FeasibleMatch.hpp"
#include "Output.hpp"
#include "Solution.hpp"
#include "ModelUtils.hpp"

#include <iostream>
#include <set>
#include <vector>
#include <map>
#include <iostream>
#include <iomanip>
#include <exception>

using namespace std;

class MitmModel_new {
public:
    MitmModel_new(const time_t startTime, const time_t endTime, const double maxMatchDistKm, const double minOverlapThreshold, std::set<Request*, ReqComp> initRequests, std::set<OpenTrip*, EtaComp> initOpenTrips, const std::set<Driver*, DriverIndexComp> * drivers);
    virtual ~MitmModel_new();
    
    bool solve(bool printDebugFiles, Output * pOutput, bool populateInitOpenTrips);
    bool solve_deprecated(bool printDebugFiles, Output * pOutput, bool populateInitOpenTrips);
    
    // methods to update
    std::set<AssignedTrip*, AssignedTripIndexComp> getAllCompletedOpenTrips(time_t &tm, std::set<OpenTrip*,EtaComp> &openTrips);
    AssignedTrip * getBestMatchForCurrMinionRequest(Request * pMinionRequest, std::set<OpenTrip*, EtaComp> * pMasterCandidates, bool printDebugFiles, std::ofstream * pOutFile);
        
    // given minion,master pair, check if feasible
    std::vector<FeasibleMatch*> getFeasibleMatchesFromCurrPair(Request* pMinionReq, OpenTrip * pMasterCand);
    FeasibleMatch * checkIfFIFOMatchIsFeasible(const std::string minionId, const double distToMinion, const double masterUberXDist, const double minionUberXDist, Request* pMinionReq, OpenTrip * pMasterCand);
    FeasibleMatch * checkIfFILOMatchIsFeasible(const double distToMinion, const double masterUberXDist, const double minionUberXDist, Request* pMinionReq, OpenTrip * pMasterCand);
    
    const double computeCostOfMatch(FeasibleMatch * pMatch) const;    
    AssignedTrip * convertFeasibleMatchToAssignedTripObject(FeasibleMatch * pMatch);    
    bool removeMasterFromOpenTrips(AssignedTrip * pMatchedTrip, std::set<OpenTrip*, EtaComp> *pOpenTrips);
    
    // find nearest unassigned driver
    std::pair<Driver*, std::pair<double,double> > * getNearestDriverToDispatchRequest(std::set<OpenTrip*, EtaComp> * pOpenTrips, time_t reqTime, double reqLat, double reqLng, const Driver * pDriver);
    bool checkIfDriverAssignedToOpenTrip( time_t currTime, Driver * pDriver, std::set<OpenTrip*, EtaComp>* pOpenTrips );
    std::pair<const TripData*, const TripData*> * getAdjacentTrips(const Driver * pDriver, const time_t reqTime);
    LatLng getEstLocationOfOpenDriver(const Driver * pDriver, const time_t &reqTime);
    //LatLng * getEstLocationOfOpenDriver(const Driver * pDriver, const time_t &reqTime);
    //OpenTrip * createNewOpenTripForUnmatchedRequest(const Driver * pNearestDriver, std::pair<double,double> driverDispatchLoc, Request * pMinionRequest, const Event * pActualDispatchEvent);
    OpenTrip * createNewOpenTripForUnmatchedRequest(const Driver * pNearestDriver, Request * pMinionRequest, const Event * pActualDispatchEvent);
    OpenTrip * getOpenTripAssignedToDriver(std::set<OpenTrip*, EtaComp> * pOpenTrips, const Driver * pDriver);
    
    std::set<Request*,  ReqComp> cloneRequests();
    std::set<OpenTrip*, EtaComp> cloneOpenTrips();
    
    void clearRemainingOpenTrips(std::set<OpenTrip*, EtaComp> &openTrips, std::set<AssignedTrip*, AssignedTripIndexComp> * pAssignedTrips);
    
    // methods for building solution
    void buildSolution(std::set<AssignedTrip*, AssignedTripIndexComp> &assignedTrips);
    std::pair<std::set<AssignedTrip*, AssignedTripIndexComp>, std::set<AssignedTrip*, AssignedTripIndexComp> > partitionAssignedTrips(std::set<AssignedTrip*, AssignedTripIndexComp> &assignedTrips);
    Solution * getSolution() {return pSolution; } 
    
    // file I/O
    void initCandidateMatchFile(std::ofstream &outFile);
    void printMatchCandidatesForCurrRequest(std::multimap<const double, FeasibleMatch*> * pCandMatchMap, std::ofstream * pOut);
    
private:
  
    const time_t _startTime;
    const time_t _endTime;
    const double _maxMatchDistInKm;
    const double _minOverlapThreshold;
    std::set<Request*, ReqComp> _allRequests;
    std::set<OpenTrip*, EtaComp> _initOpenTrips; 
    std::set<AssignedTrip*, AssignedTripIndexComp> _assignedTrips;
    std::set<Request*, ReqComp> _disqualifiedRequests;
    const std::set<Driver*, DriverIndexComp> * _allDrivers;
    
    Solution * pSolution; 
   

};

#endif	/* MITMMODEL_HPP */

