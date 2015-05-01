/* 
 * File:   FlexDepSolution.hpp
 * Author: jonpetersen
 *
 * Created on April 28, 2015, 8:11 PM
 */

#ifndef FLEXDEPSOLUTION_HPP
#define	FLEXDEPSOLUTION_HPP

#include "Solution.hpp"

class FlexDepSolution : public Solution {
public:
    
    // summary metrics by Flex Departure Reqs
    struct RequestMetrics_FD {
        int     _totalRequests;
        int     _matchedRequests;
        int     _unmatchedRequests;
        double  _matchPercentage;     
    };    
    struct MatchMetrics_FD {
        int     _numpMatches;               
        int     _numExtendedMatches;
        double  _pctExtendedMatches;
        int     _numNonExtendedMatches;
        double  _pctNonExtendedMatches;       
    };    
    struct MatchInconvenienceMetrics_FD {
        double _avgPctAddedDistsForAll;
        double _avgPctAddedDistsForMasters;
        double _avgPctAddedDistsForMinions;          
    };
    
    // summary metrics by non-Flex Departure Reqs
    struct RequestMetrics_nonFD {
        int     _totalRequests;
        int     _matchedRequests;
        int     _unmatchedRequests;
        double  _matchPercentage;
    };
    struct MatchMetrics_nonFD {
        int     _numpMatches;               
        int     _numExtendedMatches;
        double  _pctExtendedMatches;
        int     _numNonExtendedMatches;
        double  _pctNonExtendedMatches; 
    };
    struct MatchInconvenienceMetrics_nonFD {
        double _avgPctAddedDistsForAll;
        double _avgPctAddedDistsForMasters;
        double _avgPctAddedDistsForMinions; 
    };
    
    FlexDepSolution( const time_t simStart, const time_t simEnd, const int totalReqs,
                     const int totalDrivers, std::set<AssignedTrip*, AssignedTripIndexComp> &assignedTrips, 
                     std::set<Request*, ReqComp> &disqualReqs, std::set<const int> * flexDepReqIndices );
    virtual ~FlexDepSolution();
    
    const std::set<const int> * getFlexDepReqIndices() const { return pFlexDepRequestIndices; }
    std::set<const int> getMatchedFDReqIndices();
    
    // metrics of flexible departure requests
    const FlexDepSolution::RequestMetrics_FD * getFlexDepReqMetrics() const { return &_requestMetrics_FD; }
    const FlexDepSolution::MatchMetrics_FD * getFlexDepMatchMetrics() const { return &_matchMetrics_FD; }
    const FlexDepSolution::MatchInconvenienceMetrics_FD * getFlexDepMatchInconvMetrics() const { return &_inconvenienceMetrics_FD; }
    
    // metrics of non-FD requests
    const FlexDepSolution::RequestMetrics_nonFD * getNonFlexDepReqMetrics() const { return &_requestMetrics_nonFD; }
    const FlexDepSolution::MatchMetrics_nonFD * getNonFlexDepMatchMetrics() const { return &_matchMetrics_nonFD; }
    const FlexDepSolution::MatchInconvenienceMetrics_nonFD * getNonFlexDepMatchInconvMetrics() const { return &_inconvenienceMetrics_nonFD; }    
    
    // main method to generate all data
    void buildSolutionMetrics();
    void buildAggregateSolutionMetrics(); // aggregate metrics
    void buildFDSolutionMetrics(); // metrics associated with FD vs. non-FD reqs
    
    
    
private:

    int _totalReques;
    int _totalNonFlexPoolTrips;
    
    // metrics for flex departures
    double _matchPct_flex;
    double _inconv_flex;
    int    _numTrips_flex;
    
    // indices associated with FlexDep request indices
    const std::set<const int> * pFlexDepRequestIndices;
    
    //  SOLUTION CONTAINERS FOR FLEX DEP REQUESTS
    FlexDepSolution::RequestMetrics_FD             _requestMetrics_FD;
    FlexDepSolution::MatchMetrics_FD               _matchMetrics_FD;
    FlexDepSolution::MatchInconvenienceMetrics_FD  _inconvenienceMetrics_FD;

    //  SOLUTION CONTAINERS FOR NON-FLEX DEP REQUESTS
    FlexDepSolution::RequestMetrics_nonFD             _requestMetrics_nonFD;
    FlexDepSolution::MatchMetrics_nonFD               _matchMetrics_nonFD;
    FlexDepSolution::MatchInconvenienceMetrics_nonFD  _inconvenienceMetrics_nonFD;    
};

#endif	/* FLEXDEPSOLUTION_HPP */

