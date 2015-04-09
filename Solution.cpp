/* 
 * File:   Solution.cpp
 * Author: jonpetersen
 * 
 * Created on March 31, 2015, 12:29 PM
 */

#include "Solution.hpp"

Solution::Solution(Model model, time_t simStart, time_t simEnd, int totalReqs, int totalDrivers, std::set<AssignedTrip*, AssignedTripIndexComp> &assignedTrips, std::set<Request*,ReqComp> &disqualifiedReqs) : 
        _model(model), _simStartTime(simStart), _simEndTime(simEnd), _totalRequests(totalReqs), _totalDrivers(totalDrivers), _allTripsFromSolution(assignedTrips), _disqualifiedRequests(disqualifiedReqs) {
}

Solution::~Solution() {
}

void Solution::buildSolutionMetrics() {
    
    // instantiate data to be populated
    std::vector<double> pctAddedDistancesForMasters;  // how much added distance for pool trip relative to X for masters
    std::vector<double> pctAddedDistancesForMinions;  // how much added distance for pool trip relative to X for minions
    
    int numExtendedMatches = 0;
    int numNonExtendedMatches = 0;
    
    int numFIFOMatches = 0;    
    int numFILOMatches = 0;
    
    int numFIFOExtendedMatches    = 0;
    int numFIFONonExtendedMatches = 0;
    int numFILOExtendedMatches    = 0;
    int numFILONonExtendedMatches = 0;
    
    // loop through trip assignments and update statistics
    std::set<AssignedTrip*, AssignedTripIndexComp>::const_iterator tripItr;
    for( tripItr = _allTripsFromSolution.begin(); tripItr != _allTripsFromSolution.end(); ++tripItr ) {
        if( (*tripItr)->isMatchedTrip() ) {
            _matchedTrips.insert(*tripItr);
            
            // track extended and FIFOs
            bool isExtended = (*tripItr)->getMatchDetails()->_masterPickedUpAtTimeOfMatch;
            bool isFIFO     = (*tripItr)->getMatchDetails()->_fixedDropoff;
            if( isExtended ) {
                numExtendedMatches++;           // increment ONLY extended matches
            } else {
                numNonExtendedMatches++;        // increment ONLY nonextended matches
            }
            if( isFIFO ) {
                numFIFOMatches++;               // increment ONLY FIFO matches
            } else {
                numFILOMatches++;               // increment ONLY FILO matches
            }
            
            if( isExtended && isFIFO ) {
                numFIFOExtendedMatches++;       // increment FIFO AND extended
            } else if( isExtended && !isFIFO ) {
                numFILOExtendedMatches++;       // increment FILO AND extended
            } else if( !isExtended && isFIFO ) {
                numFIFONonExtendedMatches++;    // increment FIFO AND nonextended
            } else if( !isExtended && !isFIFO ) {
                numFILONonExtendedMatches++;    // increment FILO AND nonextended
            }
                        
            
            // compute additional distance for master
            const double tripLength_master  = (*tripItr)->getMatchDetails()->_totalDistanceForMaster;
            const double uberXLength_master = (*tripItr)->getMatchDetails()->_uberXDistanceForMaster;
            const double addedDist_master   = tripLength_master - uberXLength_master;
            const double pctAdded_master = (double)100*addedDist_master/uberXLength_master;
            pctAddedDistancesForMasters.push_back(pctAdded_master);

            // compute additional distance for minion
            const double tripLength_minion  = (*tripItr)->getMatchDetails()->_totalDistanceForMinion;
            const double uberXLength_minion = (*tripItr)->getMatchDetails()->_uberXDistanceForMinion;
            const double addedDist_minion   = tripLength_minion - uberXLength_minion;
            const double pctAdded_minion    = (double)100*addedDist_minion/uberXLength_minion;
            pctAddedDistancesForMinions.push_back(pctAdded_minion);            
            
        }
        else {
            _unmatchedTrips.insert(*tripItr);
        }
    }
    
    // compute MATCH RATE
    int totalMatchedTrips   = (int)_matchedTrips.size();
    int totalUnmatchedTrips = (int)_unmatchedTrips.size();
    int totalDisqualReqs    = (int)_disqualifiedRequests.size();
    int totalMatchedRequests = 2*totalMatchedTrips;
    int totalRequests = totalMatchedRequests + totalUnmatchedTrips + totalDisqualReqs;
    const double matchRate = (double)100*(double)totalMatchedRequests/(double)totalRequests;
    const double unmatchedRequestRate = (double)totalUnmatchedTrips/(double)totalRequests;
    const double disqualRequestRate = (double)totalDisqualReqs/(double)totalRequests;
    
    // compute metrics involving percentages of extended, FIFO, and combinations
    double pctExtendedMatches = (double)100*(double)numExtendedMatches/(double)totalMatchedTrips;
    double pctNonExtendedMatches = (double)100*(double)numNonExtendedMatches/(double)totalMatchedTrips;
    double pctFIFOMatches = (double)100*(double)numFIFOMatches/(double)totalMatchedTrips;
    double pctFILOMatches = (double)100*(double)numFILOMatches/(double)totalMatchedTrips;
    double pctFIFOExtendedMatches = (double)100*(double)numFIFOExtendedMatches/(double)totalMatchedTrips;
    double pctFIFONonExtendedMatches = (double)100*(double)numFIFONonExtendedMatches/(double)totalMatchedTrips;
    double pctFILOExtendedMatches = (double)100*(double)numFILOExtendedMatches/(double)totalMatchedTrips;
    double pctFILONonExtendedMatches = (double)100*(double)numFILONonExtendedMatches/(double)totalMatchedTrips;
    
    // compute inconvenience 
    std::vector<double> pctAllAddedDistances; 
    pctAllAddedDistances.insert(pctAllAddedDistances.end(),pctAddedDistancesForMasters.begin(),pctAddedDistancesForMasters.end());
    pctAllAddedDistances.insert(pctAllAddedDistances.end(),pctAddedDistancesForMinions.begin(),pctAddedDistancesForMinions.end());
    const double meanAddedDist_masters = Utility::computeMean(pctAddedDistancesForMasters);
    const double meanAddedDist_minions = Utility::computeMean(pctAddedDistancesForMinions);
    const double meanAddedDist_all     = Utility::computeMean(pctAllAddedDistances);
    
    // ---- POPULATE SOLUTION CONTAINERS ----
    // request summary
    _requestMetrics._totalRequests = totalRequests;
    _requestMetrics._numMatchedRequests = totalMatchedRequests;
    _requestMetrics._matchedPercentage = matchRate;
    _requestMetrics._numUnmatchedRequests = totalUnmatchedTrips;
    _requestMetrics._unmatchedPercentage = unmatchedRequestRate;
    _requestMetrics._numDisqualifiedRequests = totalDisqualReqs;
    _requestMetrics._disqualPercentage = disqualRequestRate;
    
    // define match metrics
    _matchMetrics._numMatches = totalMatchedTrips;
    _matchMetrics._numExtendedMatches = numExtendedMatches;
    _matchMetrics._pctExtendedMatches = pctExtendedMatches;
    _matchMetrics._numNonExtendedMatches = numNonExtendedMatches;
    _matchMetrics._pctNonExtendedMatches = pctNonExtendedMatches;
    _matchMetrics._numFIFOMatches = numFIFOMatches;
    _matchMetrics._pctFIFOMatches = pctFIFOMatches;
    _matchMetrics._numFILOMatches = numFILOMatches;
    _matchMetrics._pctFILOMatches = pctFILOMatches;
    _matchMetrics._pctFIFOExtendedMatches = pctFIFOExtendedMatches;
    _matchMetrics._pctFIFONonExtendedMatches = pctFIFONonExtendedMatches;
    _matchMetrics._pctFILOExtendedMatches = pctFILOExtendedMatches;
    _matchMetrics._pctFILONonExtendedMatches = pctFILONonExtendedMatches;
    
    // define avg increase in trip length for all, masters, and minions  
    _inconvenienceMetrics._avgPctAddedDistsForAll     = meanAddedDist_all;
    _inconvenienceMetrics._avgPctAddedDistsForMasters = meanAddedDist_masters;
    _inconvenienceMetrics._avgPctAddedDistsForMinions = meanAddedDist_minions;

}

