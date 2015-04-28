/* 
 * File:   Solution.hpp
 * Author: jonpetersen
 *
 * Created on March 31, 2015, 12:29 PM
 */

#ifndef SOLUTION_HPP
#define	SOLUTION_HPP

#include "Driver.hpp"
#include "AssignedTrip.hpp"
#include "Comparators.hpp"
#include <vector>
#include <set>

using namespace std;

class Solution {
 
public:
    
    enum Model {
        MITM,
        UFBW_fixedPickups,
        UFBW_pickupSwaps,
        UFBW_perfectInfo
    };
        
    struct RequestMetrics {
        int     _totalRequests;
        int     _numMatchedRequests;
        double  _matchedPercentage;
        int     _numUnmatchedRequests;
        double  _unmatchedPercentage;
        int     _numDisqualifiedRequests;
        double  _disqualPercentage;
    };
    
    struct MatchMetrics {
        int     _numMatches;               
        int     _numExtendedMatches;
        double  _pctExtendedMatches;
        int     _numNonExtendedMatches;
        double  _pctNonExtendedMatches;
        int     _numFIFOMatches;
        double  _pctFIFOMatches;
        int     _numFILOMatches;
        double  _pctFILOMatches;
        double  _pctFIFOExtendedMatches;
        double  _pctFIFONonExtendedMatches;
        double  _pctFILOExtendedMatches;
        double  _pctFILONonExtendedMatches;
    };
    
    struct MatchInconvenienceMetrics{
        double _avgPctAddedDistsForAll;
        double _avgPctAddedDistsForMasters;
        double _avgPctAddedDistsForMinions;        
    };
    
    
    Solution(Model model, const time_t simStart, const time_t simEnd, const int totalReqs, const int totalDrivers, std::set<AssignedTrip*, AssignedTripIndexComp> &assignedTrips, std::set<Request*,ReqComp> &disqualifiedReqs);
    virtual ~Solution();
    
    // main method to generate all data
    void buildSolutionMetrics();
    
    // methods to compute interesting metrics
    //const double getMatchRate() const;
    //AddedDistances getAddedDistancesOfMatchedTrips();
   // MatchMetrics getMatchMetrics();
    
    // setters
    void setMatchedTrips(std::set<AssignedTrip*, AssignedTripIndexComp> matchedTrips) { _matchedTrips = matchedTrips; }
    void setUnmatchedTrips(std::set<AssignedTrip*, AssignedTripIndexComp> unmatchedTrips) { _unmatchedTrips = unmatchedTrips; }
    void setDisqualifiedRequests(std::set<Request*,ReqComp> disqualReqs) { _disqualifiedRequests = disqualReqs; }
    
    // getters
    const Model getModel() const { return _model; }
    const time_t getStartTime() const { return _simStartTime; }
    const time_t getEndTime() const { return _simEndTime; }
    const int getTotalRequests() const { return _totalRequests; }
    const int getTotalDrivers() const { return _totalDrivers; }    
    const std::set<AssignedTrip*, AssignedTripIndexComp> * getMatchedTrips() const { return &_matchedTrips; }
    const std::set<AssignedTrip*, AssignedTripIndexComp> * getUnmatchedTrips() const { return &_unmatchedTrips; }
    const std::set<Request*, ReqComp> * getDisqualifiedRequests() const { return &_disqualifiedRequests; }
    
    // return solution containers
    const Solution::RequestMetrics * getRequestMetrics() const { return &_requestMetrics; }
    const Solution::MatchMetrics * getMatchMetrics() const { return &_matchMetrics; }
    const Solution::MatchInconvenienceMetrics * getInconvenienceMetrics() const { return &_inconvenienceMetrics; }
    
    const int getTotalNumTripsFromSoln() const { return (int)_allTripsFromSolution.size(); }
    
  
private:
    
    const Model _model;
    
    // input data
    const time_t _simStartTime;
    const time_t _simEndTime;
    const int _totalRequests;
    const int _totalDrivers;
    
    // trips types (matched, unmatched, disqualified requests)
    const std::set<AssignedTrip*, AssignedTripIndexComp> _allTripsFromSolution;
    
    std::set<AssignedTrip*, AssignedTripIndexComp> _matchedTrips;
    std::set<AssignedTrip*, AssignedTripIndexComp> _unmatchedTrips;
    std::set<Request*, ReqComp> _disqualifiedRequests;

    //  SOLUTION CONTAINERS
    Solution::RequestMetrics            _requestMetrics;
    Solution::MatchMetrics              _matchMetrics;
    Solution::MatchInconvenienceMetrics _inconvenienceMetrics;
    
};

#endif	/* SOLUTION_HPP */

