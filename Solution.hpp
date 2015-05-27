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
#include "ModelEnum.hpp"
#include <vector>
#include <set>

using namespace std;

class Solution {
 
public:
        
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
        
        // wait times
        double  _avgWaitTimeOfMatchesForMasters;
        double  _avgWaitTimeOfMatchesForMinions;
        double  _avgWaitTimeOfMatchesForAllMatchedRiders;
    };
    
    struct SavingsMetrics {
        double _avgMasterSavingsPct;
        double _avgMinionSavingsPct;
        double _avgMatchedRiderSavingsPct;
    };
    
    struct MatchInconvenienceMetrics {
        double _avgPctAddedDistsForAll;
        double _avgPctAddedDistsForMasters;
        double _avgPctAddedDistsForMinions;        
    };
    
    struct MatchOverlapMetrics {        
        double  _avgOverlapDist;
        double  _avgTripOverlapPct;
        double  _avgPctOverlapAll;
        double  _avgPctOverlapMasters;
        double  _avgPctOverlapMinions;
    };
    
    struct IndivMatchedRiderMetrics {
        std::vector<double> _inconv_ALL;
        std::vector<double> _inconv_Masters;
        std::vector<double> _inconv_Minions;
        
        std::vector<double> _overlapDist;
        std::vector<double> _overlapPct_Trip;
        std::vector<double> _overlapPct_ALL;
        std::vector<double> _overlapPct_Masters;
        std::vector<double> _overlapPct_Minions;
        
        std::vector<double> _savings_ALL;
        std::vector<double> _savings_Masters;
        std::vector<double> _savings_Minions;
        
        std::vector<int>    _waitTimeToMatch_ALL;
        std::vector<int>    _waitTimeToMatch_Masters;
        std::vector<int>    _waitTimeToMatch_Minions;          
    };
    
    // constructor for scenarios OTHER than 
    Solution(int model, const time_t simStart, const time_t simEnd, const int totalReqs, const int totalDrivers, std::set<AssignedTrip*, AssignedTripIndexComp> &assignedTrips, std::set<Request*,ReqComp> &disqualifiedReqs);   
    Solution(int model, const time_t simStart, const time_t simEnd, const int totalReqs, const int totalDrivers, std::set<Request*,ReqComp> &disqualifiedReqs);
    virtual ~Solution();
    
    // main method to generate all data
    void buildSolutionMetrics();    
    
    // setters
    void setMatchedTrips(std::set<AssignedTrip*, AssignedTripIndexComp> matchedTrips) { _matchedTrips = matchedTrips; }
    void setUnmatchedTrips(std::set<AssignedTrip*, AssignedTripIndexComp> unmatchedTrips) { _unmatchedTrips = unmatchedTrips; }
    void setDisqualifiedRequests(std::set<Request*,ReqComp> disqualReqs) { _disqualifiedRequests = disqualReqs; }
    
    // getters
    const int getModel() const { return _model; }
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
    const Solution::MatchOverlapMetrics * getOverlapMetrics() const { return &_overlapMetrics; }
    const Solution::SavingsMetrics * getSavingsMetrics() const { return &_savingsMetrics; }
    const Solution::IndivMatchedRiderMetrics * getIndivMatchedRidersMetrics() const { return &_indivMatchedRiderMetrics; }
    
    const int getTotalNumTripsFromSoln() const { return (int)_allTripsFromSolution.size(); }
    
    // methods used by derived classes
    const std::set<AssignedTrip*, AssignedTripIndexComp> * getAllTripsFromSoln() const { return &_allTripsFromSolution; }
    void addMatchedTrip(AssignedTrip * pAssignedTrip) { _matchedTrips.insert(pAssignedTrip); }
    void addUnmatchedTrip(AssignedTrip * pUnmatchedTrip) { _unmatchedTrips.insert(pUnmatchedTrip); }
    const int getNumberMatchedTrips() const { return (int)_matchedTrips.size(); }
    const int getNumberUnmatchedTrips() const { return (int)_unmatchedTrips.size(); }
    const int getNumDisqualRequests() const { return (int)_disqualifiedRequests.size(); }
    const std::set<AssignedTrip*, AssignedTripIndexComp> * getMatchedTrips() { return &_matchedTrips; }
    const int getTotalNumRequests() const { return _requestMetrics._totalRequests; }
    const int getTotalMatchedRequests() const { return _requestMetrics._numMatchedRequests; }
    
    // methods used by derived class(es))
    void setRequestMetrics(int totalRequests, int totalMatchedRequests, double matchRate, int totalUnmatchedTrips, double unmatchedRequestRate, int totalDisqualReqs, double disqualRequestRate);
    void setMatchMetrics(int totalMatchedTrips, int numExtendedMatches, double pctExtendedMatches, int numNonExtendedMatches, double pctNonExtendedMatches, 
                        int numFIFOMatches, double pctFIFOMatches, int numFILOMatches, double pctFILOMatches,
                        double pctFIFOExtendedMatches, double pctFIFONonExtendedMatches, double pctFILOExtendedMatches, double pctFILONonExtendedMatches,
                        double avgWaitTimeMatch_all, double avgWaitTimeMatch_masters, double avgWaitTimeMatch_minions);
    void setInconvenienceMetrics(double avgPctAddedAll, double avgPctAddedMasters, double avgPctAddedMinions);
    void setSavingsMetrics(double avgSavingsAll, double avgSavingsMasters, double avgSavingsMinions);
    void setOverlapMetrics(double avgSharedDist, double pctSharedDist_Trip, double avgPctSharedDist_ALL, double avgPctSharedDist_Masters, double avgPctSharedDist_Minions);
    void setIndivMatchMetrics(std::vector<double> pctAllAddedDistances, std::vector<double> pctAddedDistancesForMasters, std::vector<double> pctAddedDistancesForMinions,
                              std::vector<double> pctSavingsAllMatchedRiders, std::vector<double> pctSavingsForMasters, std::vector<double> pctSavingsForMinions,
                              std::vector<int> waitTimeMatches_all, std::vector<int> waitTimeMatchesForMasters, std::vector<int> waitTimeMatchesForMinions, 
                              std::vector<double> overlapDistances, std::vector<double> pctSharedDist_Trip, std::vector<double> pctSharedDist_ALL, std::vector<double> pctOverlapDist_Masters, std::vector<double> pctOverlapDist_Minions);
    
    const int getNumDisqualReqs() const { return (int)_disqualifiedRequests.size(); }
    
    // methods to populate Solution structures from base class(es)
    void setRequestMetrics(Solution::RequestMetrics reqMetrics) { _requestMetrics = reqMetrics; }
    void setMatchMetrics(Solution::MatchMetrics matchMetrics) { _matchMetrics = matchMetrics; }
    void setInconvenienceMetrics(Solution::MatchInconvenienceMetrics inconvMetrics) { _inconvenienceMetrics = inconvMetrics; }
    void setOverlapMetrics(Solution::MatchOverlapMetrics overlapMetrics) { _overlapMetrics = overlapMetrics; }
    void setSavingsMetrics(Solution::SavingsMetrics savingsMetrics) {_savingsMetrics = savingsMetrics; }
    void setIndivMatchedMetrics(Solution::IndivMatchedRiderMetrics indivMetrics) { _indivMatchedRiderMetrics = indivMetrics; }
    
private:
    
    const int _model;
    
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
    Solution::MatchOverlapMetrics       _overlapMetrics;
    Solution::SavingsMetrics            _savingsMetrics;    
    Solution::IndivMatchedRiderMetrics  _indivMatchedRiderMetrics;
    
};

#endif	/* SOLUTION_HPP */

