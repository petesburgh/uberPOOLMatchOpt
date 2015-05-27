/* 
 * File:   MultPickupSoln.cpp
 * Author: jonpetersen
 * 
 * Created on May 18, 2015, 1:36 PM
 */

#include "MultPickupSoln.hpp"

MultPickupSoln::MultPickupSoln(int model, const time_t simStart, const time_t simEnd, const int totalReqs, const int totalDrivers, std::set<AssignedRoute*, AssignedRouteIndexComp> &assignedRoutes, std::set<Request*,ReqComp> &disqualifiedReqs, int maxRiders) :
    Solution(MULTIPLE_PICKUPS, simStart, simEnd, totalReqs, totalDrivers, disqualifiedReqs), _allRoutesFromSolution(assignedRoutes), _maxRidersPerTrip(maxRiders) {};
   
MultPickupSoln::~MultPickupSoln() {
}

void MultPickupSoln::buildSolutionMetrics() {
        
    std::vector<AssignedRoute*> routes_1Rider;
    std::vector<AssignedRoute*> routes_2Riders;
    std::vector<AssignedRoute*> routes_3Riders;
    
    std::set<AssignedRoute*, AssignedRouteIndexComp>::const_iterator rItr;
    for( rItr = _allRoutesFromSolution.begin(); rItr != _allRoutesFromSolution.end(); ++rItr ) {
        int numRiders = (*rItr)->getNumRidersInRoute();
        assert( (1 <= numRiders) && (numRiders <= _maxRidersPerTrip) );
        switch( numRiders ) {
            case 1 :
                routes_1Rider.push_back(*rItr);
                break;
            case 2 :
                routes_2Riders.push_back(*rItr);
                break;
            case 3 :
                routes_3Riders.push_back(*rItr);
                break;
            default :
                throw new RiderCountException(*rItr, _maxRidersPerTrip);
        }
    }
    
    // populate priver members    
    _unmatchedRoutes.insert(routes_1Rider.begin(), routes_1Rider.end());
    _matchedRoutes.insert(routes_2Riders.begin(), routes_2Riders.end());
    _matchedRoutes.insert(routes_3Riders.begin(), routes_3Riders.end());
    
    int numTrips_1Rider  = (int)routes_1Rider.size();
    int numTrips_2Riders = (int)routes_2Riders.size();
    int numTrips_3Riders = (int)routes_3Riders.size();
    const int totalDisqualReqs = getNumDisqualReqs();
    
    const int totalRequests = numTrips_1Rider + 2*numTrips_2Riders + 3*numTrips_3Riders + totalDisqualReqs;
    const int totalMatchedRequests = 2*numTrips_2Riders + 3*numTrips_3Riders;
    const double matchRate = (double)100*((double)totalMatchedRequests/(double)totalRequests);
    const double unmatchedPercentage = (double)100*((double)numTrips_1Rider/(double)totalRequests);    
    const double disqualRequestRate = (double)100*((double)totalDisqualReqs/(double)totalRequests);
    
    // ---- POPULATE SOLUTION CONTAINERS ----
    // distribution of trips by rider count
    _riderCountDistn.numTrips_1Rider  = numTrips_1Rider;
    _riderCountDistn.numTrips_2Riders = numTrips_2Riders;
    _riderCountDistn.numTrips_3Riders = numTrips_3Riders;
    
    // request summary
    Solution::RequestMetrics _requestMetrics;
    _requestMetrics._totalRequests = totalRequests;
    _requestMetrics._numMatchedRequests = totalMatchedRequests;
    _requestMetrics._matchedPercentage = matchRate;
    _requestMetrics._numUnmatchedRequests = numTrips_1Rider;
    _requestMetrics._unmatchedPercentage = unmatchedPercentage;
    _requestMetrics._numDisqualifiedRequests = totalDisqualReqs;
    _requestMetrics._disqualPercentage = disqualRequestRate;
    setRequestMetrics(_requestMetrics); // populate private member in base class
        
    // ---- POPULATE SOLUTION CONTAINERS ----
    Solution::MatchMetrics _matchMetrics;
    _matchMetrics._numMatches = totalMatchedRequests;
    _matchMetrics._numExtendedMatches = -1;
    _matchMetrics._pctExtendedMatches = -1.0;
    _matchMetrics._numNonExtendedMatches = -1.0;
    _matchMetrics._pctNonExtendedMatches = -1.0;
    _matchMetrics._numFIFOMatches = -1;
    _matchMetrics._pctFIFOMatches = -1.0;
    _matchMetrics._numFILOMatches = -1;
    _matchMetrics._pctFILOMatches = -1.0;
    _matchMetrics._pctFIFOExtendedMatches = -1.0;
    _matchMetrics._pctFIFONonExtendedMatches = -1.0;
    _matchMetrics._pctFILOExtendedMatches = -1.0;
    _matchMetrics._pctFILONonExtendedMatches = -1.0;
    _matchMetrics._avgWaitTimeOfMatchesForAllMatchedRiders = 0.0;
    _matchMetrics._avgWaitTimeOfMatchesForMasters = 0.0;
    _matchMetrics._avgWaitTimeOfMatchesForMinions = 0.0;    
    setMatchMetrics(_matchMetrics); // populate private member in base class

    // define avg increase in trip length for all, masters, and minions 
    Solution::MatchInconvenienceMetrics _inconvenienceMetrics;
    _inconvenienceMetrics._avgPctAddedDistsForAll     = -1.0;
    _inconvenienceMetrics._avgPctAddedDistsForMasters = -1.0; 
    _inconvenienceMetrics._avgPctAddedDistsForMinions = -1.0; 
    setInconvenienceMetrics(_inconvenienceMetrics); 
    
    // overlap metrics
    Solution::MatchOverlapMetrics _overlapMetrics;
    _overlapMetrics._avgOverlapDist       = -1.0;
    _overlapMetrics._avgTripOverlapPct    = -1.0;
    _overlapMetrics._avgPctOverlapAll     = -1.0;
    _overlapMetrics._avgPctOverlapMasters = -1.0;
    _overlapMetrics._avgPctOverlapMinions = -1.0;
    setOverlapMetrics(_overlapMetrics);
    
    // define savings
    Solution::SavingsMetrics _savingsMetrics;
    _savingsMetrics._avgMatchedRiderSavingsPct = -1.0;
    _savingsMetrics._avgMasterSavingsPct = -1.0;
    _savingsMetrics._avgMinionSavingsPct = -1.0; 
    setSavingsMetrics(_savingsMetrics);

    
    // -----------------------
    //   INDIVIDUAL METRICS    
    // -----------------------
    Solution::IndivMatchedRiderMetrics _indivMatchedRiderMetrics;
    std::vector<double> dummyVec;
    std::vector<int> intDummyVec;
    
    _indivMatchedRiderMetrics._inconv_ALL              = dummyVec;
    _indivMatchedRiderMetrics._inconv_Masters          = dummyVec;
    _indivMatchedRiderMetrics._inconv_Minions          = dummyVec;

    _indivMatchedRiderMetrics._overlapDist             = dummyVec;
    _indivMatchedRiderMetrics._overlapPct_Trip         = dummyVec;
    _indivMatchedRiderMetrics._overlapPct_ALL          = dummyVec;
    _indivMatchedRiderMetrics._overlapPct_Masters      = dummyVec;
    _indivMatchedRiderMetrics._overlapPct_Minions      = dummyVec;

    _indivMatchedRiderMetrics._savings_ALL             = dummyVec;
    _indivMatchedRiderMetrics._savings_Masters         = dummyVec;
    _indivMatchedRiderMetrics._savings_Minions         = dummyVec;
    
    _indivMatchedRiderMetrics._waitTimeToMatch_ALL     = intDummyVec;
    _indivMatchedRiderMetrics._waitTimeToMatch_Masters = intDummyVec;
    _indivMatchedRiderMetrics._waitTimeToMatch_Minions = intDummyVec;  
    
    setIndivMatchedMetrics(_indivMatchedRiderMetrics);    
}