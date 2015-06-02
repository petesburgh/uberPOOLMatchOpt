/* 
 * File:   MultiplePickupsModel.hpp
 * Author: jonpetersen
 *
 * Created on May 13, 2015, 6:48 PM
 */

#ifndef MULTIPLEPICKUPSMODEL_HPP
#define	MULTIPLEPICKUPSMODEL_HPP

#include "Request.hpp"
#include "OpenTrip.hpp"
#include "Comparators.hpp"
#include "AssignedTrip.hpp"
#include "AssignedRoute.hpp"
#include "Event.hpp"
#include "LatLng.hpp"
#include "FeasibleMatch.hpp" // TODO: delete this
#include "FeasibleRoute.hpp"
#include "Output.hpp"
//#include "Solution.hpp"
#include "MultPickupSoln.hpp"
#include "ModelUtils.hpp"
#include "Route.hpp"
#include "RiderMetrics.hpp"

#include <iostream>
#include <set>
#include <vector>
#include <map>
#include <iostream>
#include <iomanip>
#include <exception>

using namespace std;

class MultiplePickupsModel {
public:
    MultiplePickupsModel(const time_t startTime, const time_t endTime, const double maxMatchDistKm, const double minOverlapThreshold, std::set<Request*, ReqComp> initRequests, std::set<OpenTrip*, EtdComp> initOpenTrips, const std::set<Driver*, DriverIndexComp> * drivers, bool inclInitPickupInSavings, int maxAllowablePickups);
    virtual ~MultiplePickupsModel();
    
    bool solve(bool printDebugFiles, Output * pOutput, bool populateInitOpenTrips);
    
    // methods to update
    std::set<AssignedTrip*, AssignedTripIndexComp> getAllCompletedOpenTrips(time_t &tm, std::set<OpenTrip*,EtdComp> &openTrips);
    Route * getBestRouteForRequestInsertion(Request * pMinionRequest, std::set<Route*, RouteEndComp>  * pExistingRoutes, bool printDebugFiles, std::ofstream * pOutFile);
        
    // given minion,master pair, check if feasible
    std::vector<Route*> getFeasibleMatchesFromCurrPair(Request* pMinionReq, OpenTrip * pMasterCand); // TODO: delete
    std::vector<Route*> getFeasibleRoutesToInsertMinionReq(Request * pMinionReq, Route * pExistingRoute);
    Route * checkIfFIFORouteIsFeasible(const std::string minionId, const double distToMinion, const double masterUberXDist, const double minionUberXDist, Request* pMinionReq, Route * pSingleRiderExistingRoute);
    Route * checkIfFILORouteIsFeasible(const double distToMinion, const double masterUberXDist, const double minionUberXDist, Request* pMinionReq, Route * pSingleRiderExistingRoute);

    const double computeCostOfMatch(FeasibleMatch * pMatch) const; 
    const double computeCostOfRoute(Route * pRoute) const;
    AssignedTrip * convertFeasibleMatchToAssignedTripObject(FeasibleMatch * pMatch);    
    bool removeRouteFromAssignedRoutes(Route * pMatchedRoute, std::set<Route*, RouteEndComp> *pOpenRoutes);
    
    std::vector<Route*> getFeasibleRoutesForSingleExistingRiderInRoute(Request * pMinionReq, Route * pExistingRoute);
    std::vector<Route*> getFeasibleRoutesForSingleExistingRiderInRoute_deprecated(Request * pMinionReq, Route * pExistingRoute);
    std::vector<Route*> getFeasibleRoutesWithTwoExistingRiders(Request * pMinionReq, Route * pExistingRoute);
    
    // append modified route to set of open routes
    void appendToOpenRoute(std::set<Route*, RouteEndComp> * pExistingRoutes, Route * pAppendedRoute);
    
    // methods to append a third rider to a route
    std::vector<Route*> getFeasibleRoutesWithRiderPickedUpFirst(Request * pMinionReq, Route * pExistingRoute);
    std::vector<Route*> getFeasilbeRoutesWithRiderPickedUpSecond(Request * pMinionReq, Route * pExistingRoute);
    std::vector<Route*> getFeasibleRoutesWithRiderPickedUpThird(Request * pMinionReq, Route * pExistingRoute);
    
    Route * constructInsertedRoute(Route * pOrigRoute, Request * reqToInsert, int pickupOrder, int dropoffOrder);
    bool checkIfCandRouteMeetsSavings(Route * pCandRoute);
    RouteEvent * getDropEventInRouteForRider(const int riderIndex, Route * pCandRoute);
    std::pair<const double, const double> computeTripCostAndDistanceForRider(Route * pRoute, RouteEvent * pRiderPickup, RouteEvent * pRiderDropoff, const int riderIndexInserted, double sharedDistance, double pickupDistToInsertedReq);
    Route::RouteMetrics * computeRouteMetrics(Route * pRoute);
    const double computeTotalTripDistance(Route * pRoute, const double &sharedDist);
    
    std::pair<RouteEvent*, time_t> getRouteEventPreceedingInsertedEvent(Route * pOrigRoute, int seqInsertedEvent);
    std::pair<RouteEvent*, time_t> getRouteEventFollowingInsertedEvent(Route * pOrigRoute, int seqInsertedEvent);
    
    // given an unmatched request create a new Route object
    Route * createNewRouteWithUnmatchedRider(const Driver * pDriver, Request * pMinionRequest, const Event * pActualDispatchEvent);
    
    OpenTrip * getOpenTripAssignedToDriver(std::set<OpenTrip*, EtdComp> * pOpenTrips, const Driver * pDriver);
    
    std::set<Request*,  ReqComp> cloneRequests();
    
    //void clearRemainingOpenTrips(std::set<OpenTrip*, EtdComp> &openTrips, std::set<AssignedTrip*, AssignedTripIndexComp> * pAssignedTrips);
    void clearRemainingOpenRoutes(std::set<Route*, RouteEndComp> &openRoutes, std::set<AssignedRoute*, AssignedRouteIndexComp> * pAssignedRoutes);
    
    // methods for building solution
    void buildSolution(std::set<AssignedRoute*, AssignedRouteIndexComp> &assignedRoutes);
    std::pair<std::set<AssignedTrip*, AssignedTripIndexComp>, std::set<AssignedTrip*, AssignedTripIndexComp> > partitionAssignedTrips(std::set<AssignedTrip*, AssignedTripIndexComp> &assignedTrips);
    MultPickupSoln * getSolution() {return pSolution; } 
    
    const int getMaxAllowablePickups() const { return _maxAllowablePickups; }
    
    // define metrics for given rider
    RiderMetrics * computeRiderMetrics(Route * pRoute, int riderIndexPickup, double pickupDistToInsertedReq, Request * pInsertedReq);
    void printRiderMetrics(const RiderMetrics * pRiderMetrics);
    void printRouteMetrics(const Route::RouteMetrics * pRouteMetrics);

    // file I/O
    void initCandidateMatchFile(std::ofstream &outFile);
    void printRouteCandidatesForCurrRequest(std::multimap<const double, Route*> * pCandRouteMap, std::ofstream * pOut);
        
    
private:
  
    const time_t _startTime;
    const time_t _endTime;
    const double _maxMatchDistInKm;
    const double _minOverlapThreshold;
    std::set<Request*, ReqComp> _allRequests;
    std::set<OpenTrip*, EtdComp> _initOpenTrips; 
    std::set<AssignedTrip*, AssignedTripIndexComp> _assignedTrips;
    std::set<AssignedRoute*, AssignedRouteIndexComp> _assignedRoutes;
    std::set<Request*, ReqComp> _disqualifiedRequests;
    const std::set<Driver*, DriverIndexComp> * _allDrivers;
    
    const int _maxAllowablePickups;
    
    MultPickupSoln * pSolution; 
    
    const bool _inclMinionPickupDistExtMatchesSavingsConstr;
    
    int _numRoutesCreated;
    
};

#endif	/* MULTIPLEPICKUPSMODEL_HPP */

