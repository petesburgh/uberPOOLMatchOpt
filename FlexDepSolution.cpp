/* 
 * File:   FlexDepSolution.cpp
 * Author: jonpetersen
 * 
 * Created on April 28, 2015, 8:11 PM
 */

#include "FlexDepSolution.hpp"
        
FlexDepSolution::FlexDepSolution(const time_t simStart,  const time_t simEnd, const int totalReqs, const int totalDrivers, 
        std::set<AssignedTrip*, AssignedTripIndexComp> &assignedTrips, std::set<Request*, ReqComp> &disqualReqs, std::set<const int> * flexDepReqIndices) : 
        Solution(Solution::FlexDepartures, simStart, simEnd, totalReqs, totalDrivers, assignedTrips, disqualReqs), pFlexDepRequestIndices(flexDepReqIndices) {
    
}

FlexDepSolution::~FlexDepSolution() {
}

void FlexDepSolution::buildSolutionMetrics() {

    buildAggregateSolutionMetrics(); 
    buildFDSolutionMetrics();
  
}

void FlexDepSolution::buildAggregateSolutionMetrics() {
    
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
    for( tripItr = getAllTripsFromSoln()->begin(); tripItr != getAllTripsFromSoln()->end(); ++tripItr ) {
        if( (*tripItr)->isMatchedTrip() ) {
            addMatchedTrip(*tripItr); 
                        
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
            addUnmatchedTrip(*tripItr); // _unmatchedTrips.insert(*tripItr);
        }
    }
    
    // compute MATCH RATE
    int totalMatchedTrips   = getNumberMatchedTrips();
    int totalUnmatchedTrips = getNumberUnmatchedTrips();
    int totalDisqualReqs    = getNumDisqualRequests();
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
    setRequestMetrics(totalRequests, totalMatchedRequests, matchRate, totalUnmatchedTrips, unmatchedRequestRate, totalDisqualReqs, disqualRequestRate);

    // define match metrics
    setMatchMetrics(totalMatchedTrips,numExtendedMatches,pctExtendedMatches,numNonExtendedMatches,pctNonExtendedMatches,
            numFIFOMatches,pctFIFOMatches,numFILOMatches,pctFILOMatches,pctFIFOExtendedMatches,
            pctFIFONonExtendedMatches,pctFILOExtendedMatches,pctFILONonExtendedMatches);

    // define avg increase in trip length for all, masters, and minions 
    setInconvenienceMetrics(meanAddedDist_all,meanAddedDist_masters,meanAddedDist_minions);
    
}

void FlexDepSolution::buildFDSolutionMetrics() {
   
    // step 1: compute # of requests and # of matched requests
    int totalFDRequests = (int)pFlexDepRequestIndices->size(); // num FD reqs
    std::set<const int> matchedFDReqIndices = getMatchedFDReqIndices();
    int matchedFDRequests = (int)matchedFDReqIndices.size();
    int unmatchedFDRequests = totalFDRequests - matchedFDRequests;
    
    // step 2: compute FD metrics            
    double matchRate_FDReqs = ((double)100*(double)matchedFDRequests)/(double)totalFDRequests;
    
    
    _requestMetrics_FD._totalRequests   = totalFDRequests;
    _requestMetrics_FD._matchedRequests = matchedFDRequests;
    _requestMetrics_FD._unmatchedRequests = unmatchedFDRequests;
    _requestMetrics_FD._matchPercentage = matchRate_FDReqs;
    
    // step 3: compute non-FD metrics
    int totalRequests = getTotalNumRequests();
    int totalNonFDReqs = totalRequests - totalFDRequests;
    int totalMatchedReqs = getTotalMatchedRequests();
    int totalMatchedReqs_nonFDReqs = totalMatchedReqs - matchedFDRequests; 
    int totalUnmatchedReqs_nonFDReqs = totalNonFDReqs - totalMatchedReqs_nonFDReqs;
    double matchRate_nonFDReqs = ((double)100*(double)totalMatchedReqs_nonFDReqs)/(double)totalNonFDReqs; 
    
    _requestMetrics_nonFD._totalRequests = totalNonFDReqs;
    _requestMetrics_nonFD._matchedRequests = totalMatchedReqs_nonFDReqs;
    _requestMetrics_nonFD._unmatchedRequests = totalUnmatchedReqs_nonFDReqs;
    _requestMetrics_nonFD._matchPercentage = matchRate_nonFDReqs;
    
}

std::set<const int> FlexDepSolution::getMatchedFDReqIndices() {
    
    std::set<const int> matchedFDReqIndices;
    
    // extract all matched trips
    const std::set<AssignedTrip*, AssignedTripIndexComp> * pMatchedTrips = getMatchedTrips();
    std::set<AssignedTrip *, AssignedTripIndexComp>::const_iterator itr;
    for( itr = pMatchedTrips->begin(); itr != pMatchedTrips->end(); ++itr ) {
        const int masterReqIndex = (*itr)->getMatchDetails()->_masterReqIndex;
        const int minionReqIndex = (*itr)->getMatchDetails()->_minionReqIndex;
        
        // check if MASTER is FD
        std::set<const int>::const_iterator fdItr_master = pFlexDepRequestIndices->find(masterReqIndex);
        if( fdItr_master != pFlexDepRequestIndices->end() ) {
            matchedFDReqIndices.insert(masterReqIndex);
        }
        
        // check if MINION is FD
        std::set<const int>::const_iterator fdItr_minion = pFlexDepRequestIndices->find(minionReqIndex);
        if( fdItr_minion != pFlexDepRequestIndices->end() ) {
            matchedFDReqIndices.insert(minionReqIndex);
        }
        
    }
    
    return matchedFDReqIndices;
    
}