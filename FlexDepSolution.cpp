/* 
 * File:   FlexDepSolution.cpp
 * Author: jonpetersen
 * 
 * Created on April 28, 2015, 8:11 PM
 */

#include "FlexDepSolution.hpp"
        
FlexDepSolution::FlexDepSolution(const time_t simStart,  const time_t simEnd, const int totalReqs, const int totalDrivers, 
        std::set<AssignedTrip*, AssignedTripIndexComp> &assignedTrips, std::set<Request*, ReqComp> &disqualReqs, std::set<const int> * flexDepReqIndices) : 
        Solution(FLEX_DEPARTURE, simStart, simEnd, totalReqs, totalDrivers, assignedTrips, disqualReqs), pFlexDepRequestIndices(flexDepReqIndices) {
    
}

FlexDepSolution::~FlexDepSolution() {
}

void FlexDepSolution::buildSolutionMetrics() {

    buildAggregateSolutionMetrics(); 
    buildFDSolutionMetrics();
  
}

void FlexDepSolution::buildAggregateSolutionMetrics() {
    
    // instantiate data to be populated
    std::vector<double> pctAllAddedDistances; 
    std::vector<double> pctAddedDistancesForMasters;  // how much added distance for pool trip relative to X for masters
    std::vector<double> pctAddedDistancesForMinions;  // how much added distance for pool trip relative to X for minions    
        
    std::vector<double> pctSavingsAllMatchedRiders;
    std::vector<double> pctSavingsForMasters;
    std::vector<double> pctSavingsForMinions;
    
    std::vector<int>    waitTimeMatches_all;
    std::vector<int>    waitTimeMatchesForMasters;
    std::vector<int>    waitTimeMatchesForMinions;
    
    std::vector<double> overlapDistances;
    std::vector<double> pctSharedDist_ALL;
    std::vector<double> pctOverlapDist_Masters;
    std::vector<double> pctOverlapDist_Minions;
    
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
            
            // add match overlap 
            const double sharedDist = (*tripItr)->getMatchDetails()->_sharedDistance;
            overlapDistances.push_back(sharedDist);
            double pctSharedDist_MASTERS = (double)100*(sharedDist/tripLength_master);
            double pctSharedDist_MINIONS = (double)100*(sharedDist/tripLength_minion);            
            pctOverlapDist_Masters.push_back(pctSharedDist_MASTERS);
            pctOverlapDist_Minions.push_back(pctSharedDist_MINIONS);
            
            // populate savings vectors
            pctSavingsForMasters.push_back((*tripItr)->getMatchDetails()->_masterSavings);
            pctSavingsForMinions.push_back((*tripItr)->getMatchDetails()->_minionSavings);  
            
            // populate wait time of matches
            waitTimeMatchesForMasters.push_back((*tripItr)->getMatchDetails()->_masterWaitTimeInSec);
            waitTimeMatchesForMinions.push_back((*tripItr)->getMatchDetails()->_minionWaitTimeInSec);
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
    pctAllAddedDistances.insert(pctAllAddedDistances.end(),pctAddedDistancesForMasters.begin(),pctAddedDistancesForMasters.end());
    pctAllAddedDistances.insert(pctAllAddedDistances.end(),pctAddedDistancesForMinions.begin(),pctAddedDistancesForMinions.end());
    const double meanAddedDist_masters = Utility::computeMean(pctAddedDistancesForMasters);
    const double meanAddedDist_minions = Utility::computeMean(pctAddedDistancesForMinions);
    const double meanAddedDist_all     = Utility::computeMean(pctAllAddedDistances);
    
    // compute overlap metrics
    pctSharedDist_ALL.insert(pctSharedDist_ALL.end(), pctOverlapDist_Masters.begin(), pctOverlapDist_Masters.end());
    pctSharedDist_ALL.insert(pctSharedDist_ALL.end(), pctOverlapDist_Minions.begin(), pctOverlapDist_Minions.end());
    const double meanOverlapDist        = Utility::computeMean(overlapDistances);
    const double meanPctOverlap_ALL     = Utility::computeMean(pctSharedDist_ALL);
    const double meanPctOverlap_Masters = Utility::computeMean(pctOverlapDist_Masters);
    const double meanPctOverlap_Minions = Utility::computeMean(pctOverlapDist_Minions);
    
    // compute savings
    
    pctSavingsAllMatchedRiders.insert(pctSavingsAllMatchedRiders.end(),pctSavingsForMasters.begin(),pctSavingsForMasters.end());
    pctSavingsAllMatchedRiders.insert(pctSavingsAllMatchedRiders.end(),pctSavingsForMinions.begin(),pctSavingsForMinions.end());
    const double meanSavings_allMatchedRiders = Utility::computeMean(pctSavingsAllMatchedRiders);
    const double meanSavings_masters = Utility::computeMean(pctSavingsForMasters);
    const double meanSavings_minions = Utility::computeMean(pctSavingsForMinions);
    
    // wait time for matches
    waitTimeMatches_all.insert(waitTimeMatches_all.end(), waitTimeMatchesForMasters.begin(), waitTimeMatchesForMasters.end());
    waitTimeMatches_all.insert(waitTimeMatches_all.end(), waitTimeMatchesForMinions.begin(), waitTimeMatchesForMinions.end());
    const double avgWaitTimeMatch_all     = Utility::computeMean(waitTimeMatches_all);
    const double avgWaitTimeMatch_masters = Utility::computeMean(waitTimeMatchesForMasters);
    const double avgWaitTimeMatch_minions = Utility::computeMean(waitTimeMatchesForMinions); 
    
    // ---- POPULATE SOLUTION CONTAINERS ----
    // request summary
    setRequestMetrics(totalRequests, totalMatchedRequests, matchRate, totalUnmatchedTrips, unmatchedRequestRate, totalDisqualReqs, disqualRequestRate);

    // define match metrics
    setMatchMetrics(totalMatchedTrips,numExtendedMatches,pctExtendedMatches,numNonExtendedMatches,pctNonExtendedMatches,
            numFIFOMatches,pctFIFOMatches,numFILOMatches,pctFILOMatches,pctFIFOExtendedMatches,
            pctFIFONonExtendedMatches,pctFILOExtendedMatches,pctFILONonExtendedMatches,
            avgWaitTimeMatch_all, avgWaitTimeMatch_masters, avgWaitTimeMatch_minions);

    // define avg increase in trip length for all, masters, and minions 
    setInconvenienceMetrics(meanAddedDist_all,meanAddedDist_masters,meanAddedDist_minions);
    
    // set match overlap metrics
    setOverlapMetrics(meanOverlapDist, meanPctOverlap_ALL, meanPctOverlap_Masters, meanPctOverlap_Minions);
    
    // set rider savings metrics
    setSavingsMetrics(meanSavings_allMatchedRiders, meanSavings_masters, meanSavings_minions);
    
    setIndivMatchMetrics(pctAllAddedDistances, pctAddedDistancesForMasters, pctAddedDistancesForMinions,
                         pctSavingsAllMatchedRiders, pctSavingsForMasters, pctSavingsForMinions,
                         waitTimeMatches_all, waitTimeMatchesForMasters, waitTimeMatchesForMinions, 
                         overlapDistances, pctSharedDist_ALL, pctOverlapDist_Masters, pctOverlapDist_Minions);            
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