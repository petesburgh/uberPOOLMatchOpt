/* 
 * File:   Solution.cpp
 * Author: jonpetersen
 * 
 * Created on March 31, 2015, 12:29 PM
 */

#include "Solution.hpp"

Solution::Solution(int model, time_t simStart, time_t simEnd, int totalReqs, int totalDrivers, std::set<AssignedTrip*, AssignedTripIndexComp> &assignedTrips, std::set<Request*,ReqComp> &disqualifiedReqs) : 
        _model(model), _simStartTime(simStart), _simEndTime(simEnd), _totalRequests(totalReqs), _totalDrivers(totalDrivers), _allTripsFromSolution(assignedTrips), _disqualifiedRequests(disqualifiedReqs) {
}

Solution::~Solution() {
}

void Solution::buildSolutionMetrics() {
       
    // instantiate data to be populated
    std::vector<double> pctAllAddedDistances;         // inconvenience for ALL 
    std::vector<double> pctAddedDistancesForMasters;  // how much added distance for pool trip relative to X for masters
    std::vector<double> pctAddedDistancesForMinions;  // how much added distance for pool trip relative to X for minions
    
    std::vector<double> pctSavingsForMatchedRiders;
    std::vector<double> pctSavingsForMasters;
    std::vector<double> pctSavingsForMinions;
    
    std::vector<int>    waitTimesAllMatchedRiders;
    std::vector<int>    waitTimesOfMatchMasters;
    std::vector<int>    waitTimesOfMatchMinions;
    
    std::vector<double> sharedDistancesMatches;
    std::vector<double> pctSharedDist_ALL;
    std::vector<double> pctSharedDist_Masters;
    std::vector<double> pctSharedDist_Minions;
    
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
            
            // shared distances
            const double sharedDist = (*tripItr)->getMatchDetails()->_sharedDistance;
            sharedDistancesMatches.push_back(sharedDist);
            double pctSharedDist_MASTERS = (double)100*(sharedDist/tripLength_master);
            double pctSharedDist_MINIONS = (double)100*(sharedDist/tripLength_minion);
            pctSharedDist_Masters.push_back(pctSharedDist_MASTERS);
            pctSharedDist_Minions.push_back(pctSharedDist_MINIONS);

            // savings (distance in which rider is responsible for cost)
            const double savings_masters = (*tripItr)->getMatchDetails()->_masterSavings;
            const double savings_minions = (*tripItr)->getMatchDetails()->_minionSavings;            
            pctSavingsForMasters.push_back(savings_masters);
            pctSavingsForMinions.push_back(savings_minions);
            
            // wait time of match for masters, minions (only check FUTURE matches, i.e. those not matched to an OpenTrip)
            if( (*tripItr)->getMatchDetails()->_masterWaitTimeInSec != -1 ) {
                waitTimesOfMatchMasters.push_back((*tripItr)->getMatchDetails()->_masterWaitTimeInSec);
                waitTimesOfMatchMinions.push_back((*tripItr)->getMatchDetails()->_minionWaitTimeInSec);
            }
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
    
    // compute overlap 
    pctSharedDist_ALL.insert(pctSharedDist_ALL.end(), pctSharedDist_Masters.begin(), pctSharedDist_Masters.end());
    pctSharedDist_ALL.insert(pctSharedDist_ALL.end(), pctSharedDist_Minions.begin(), pctSharedDist_Minions.end());
    const double avgOverlapDist = Utility::computeMean(sharedDistancesMatches);
    const double avgPctSharedDist_ALL = Utility::computeMean(pctSharedDist_ALL);
    const double avgPctSharedDist_Masters = Utility::computeMean(pctSharedDist_Masters);
    const double avgPctSharedDist_Minions = Utility::computeMean(pctSharedDist_Minions);
    
    
    // compute inconvenience     
    pctAllAddedDistances.insert(pctAllAddedDistances.end(),pctAddedDistancesForMasters.begin(),pctAddedDistancesForMasters.end());
    pctAllAddedDistances.insert(pctAllAddedDistances.end(),pctAddedDistancesForMinions.begin(),pctAddedDistancesForMinions.end());
    const double meanAddedDist_masters = Utility::computeMean(pctAddedDistancesForMasters);
    const double meanAddedDist_minions = Utility::computeMean(pctAddedDistancesForMinions);
    const double meanAddedDist_all     = Utility::computeMean(pctAllAddedDistances);
    
    // compute savings    
    pctSavingsForMatchedRiders.insert(pctSavingsForMatchedRiders.end(), pctSavingsForMasters.begin(), pctSavingsForMasters.end());
    pctSavingsForMatchedRiders.insert(pctSavingsForMatchedRiders.end(), pctSavingsForMinions.begin(), pctSavingsForMinions.end());
    const double meanSavings_masters = Utility::computeMean(pctSavingsForMasters);
    const double meanSavings_minions = Utility::computeMean(pctSavingsForMinions);
    const double meanSavings_allMatchedRiders = Utility::computeMean(pctSavingsForMatchedRiders);
    
    // compute wait time of match for master, minion, all    
    waitTimesAllMatchedRiders.insert(waitTimesAllMatchedRiders.end(), waitTimesOfMatchMasters.begin(), waitTimesOfMatchMasters.end());
    waitTimesAllMatchedRiders.insert(waitTimesAllMatchedRiders.end(), waitTimesOfMatchMinions.begin(), waitTimesOfMatchMinions.end());
    const double meanWaitTimeMatch_all     = Utility::computeMean(waitTimesAllMatchedRiders);
    const double meanWaitTimeMatch_masters = Utility::computeMean(waitTimesOfMatchMasters);
    const double meanWaitTimeMatch_minions = Utility::computeMean(waitTimesOfMatchMinions);
    
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
    _matchMetrics._avgWaitTimeOfMatchesForAllMatchedRiders = meanWaitTimeMatch_all;
    _matchMetrics._avgWaitTimeOfMatchesForMasters = meanWaitTimeMatch_masters;
    _matchMetrics._avgWaitTimeOfMatchesForMinions = meanWaitTimeMatch_minions;
    
    // define avg increase in trip length for all, masters, and minions  
    _inconvenienceMetrics._avgPctAddedDistsForAll     = meanAddedDist_all;
    _inconvenienceMetrics._avgPctAddedDistsForMasters = meanAddedDist_masters;
    _inconvenienceMetrics._avgPctAddedDistsForMinions = meanAddedDist_minions;
    
    // overlap metrics
    _overlapMetrics._avgOverlapDist       = avgOverlapDist;
    _overlapMetrics._avgPctOverlapAll     = avgPctSharedDist_ALL;
    _overlapMetrics._avgPctOverlapMasters = avgPctSharedDist_Masters;
    _overlapMetrics._avgPctOverlapMinions = avgPctSharedDist_Minions;
    
    // define savings
    _savingsMetrics._avgMatchedRiderSavingsPct = meanSavings_allMatchedRiders;
    _savingsMetrics._avgMasterSavingsPct = meanSavings_masters;
    _savingsMetrics._avgMinionSavingsPct = meanSavings_minions;
    
    // -----------------------
    //   INDIVIDUAL METRICS    
    // -----------------------
    _indivMatchedRiderMetrics._inconv_ALL              = pctAllAddedDistances;
    _indivMatchedRiderMetrics._inconv_Masters          = pctAddedDistancesForMasters;
    _indivMatchedRiderMetrics._inconv_Minions          = pctAddedDistancesForMinions;

    _indivMatchedRiderMetrics._overlapDist             = sharedDistancesMatches;
    _indivMatchedRiderMetrics._overlapPct_ALL          = pctSharedDist_ALL;
    _indivMatchedRiderMetrics._overlapPct_Masters      = pctSharedDist_Masters;
    _indivMatchedRiderMetrics._overlapPct_Minions      = pctSharedDist_Minions;

    _indivMatchedRiderMetrics._savings_ALL             = pctSavingsForMatchedRiders;
    _indivMatchedRiderMetrics._savings_Masters         = pctSavingsForMasters;
    _indivMatchedRiderMetrics._savings_Minions         = pctSavingsForMinions;
    
    _indivMatchedRiderMetrics._waitTimeToMatch_ALL     = waitTimesAllMatchedRiders;
    _indivMatchedRiderMetrics._waitTimeToMatch_Masters = waitTimesOfMatchMasters;
    _indivMatchedRiderMetrics._waitTimeToMatch_Minions = waitTimesOfMatchMinions;    
}

// methods used by derived class to set base class private members
void Solution::setRequestMetrics(int totalRequests, int totalMatchedRequests, double matchRate, int totalUnmatchedTrips, double unmatchedRequestRate, int totalDisqualReqs, double disqualRequestRate) {
    _requestMetrics._totalRequests = totalRequests;
    _requestMetrics._numMatchedRequests = totalMatchedRequests;
    _requestMetrics._matchedPercentage = matchRate;
    _requestMetrics._numUnmatchedRequests = totalUnmatchedTrips;
    _requestMetrics._unmatchedPercentage = unmatchedRequestRate;
    _requestMetrics._numDisqualifiedRequests = totalDisqualReqs;
    _requestMetrics._disqualPercentage = disqualRequestRate;
}
void Solution::setMatchMetrics(int totalMatchedTrips, int numExtendedMatches, double pctExtendedMatches, int numNonExtendedMatches, double pctNonExtendedMatches, 
                    int numFIFOMatches, double pctFIFOMatches, int numFILOMatches, double pctFILOMatches,
                    double pctFIFOExtendedMatches, double pctFIFONonExtendedMatches, double pctFILOExtendedMatches, double pctFILONonExtendedMatches,
                    double avgWaitTimeMatch_all, double avgWaitTimeMatch_masters, double avgWaitTimeMatch_minions) {
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
    _matchMetrics._avgWaitTimeOfMatchesForAllMatchedRiders = avgWaitTimeMatch_all;
    _matchMetrics._avgWaitTimeOfMatchesForMasters = avgWaitTimeMatch_masters;
    _matchMetrics._avgWaitTimeOfMatchesForMinions = avgWaitTimeMatch_minions;
}
void Solution::setInconvenienceMetrics(double avgPctAddedAll, double avgPctAddedMasters, double avgPctAddedMinions) {
    _inconvenienceMetrics._avgPctAddedDistsForAll = avgPctAddedAll;
    _inconvenienceMetrics._avgPctAddedDistsForMasters = avgPctAddedMasters;
    _inconvenienceMetrics._avgPctAddedDistsForMinions = avgPctAddedMinions;
}
void Solution::setSavingsMetrics(double avgSavingsAll, double avgSavingsMasters, double avgSavingsMinions) {
    _savingsMetrics._avgMatchedRiderSavingsPct = avgSavingsAll;
    _savingsMetrics._avgMasterSavingsPct = avgSavingsMasters;
    _savingsMetrics._avgMinionSavingsPct = avgSavingsMinions;    
}
void Solution::setOverlapMetrics(double avgSharedDist, double avgPctSharedDist_ALL, double avgPctSharedDist_Masters, double avgPctSharedDist_Minions) {
    _overlapMetrics._avgOverlapDist = avgSharedDist;
    _overlapMetrics._avgPctOverlapAll = avgPctSharedDist_ALL;
    _overlapMetrics._avgPctOverlapMasters = avgPctSharedDist_Masters;
    _overlapMetrics._avgPctOverlapMinions = avgPctSharedDist_Minions;
}
void Solution::setIndivMatchMetrics(std::vector<double> pctAllAddedDistances, std::vector<double> pctAddedDistancesForMasters, std::vector<double> pctAddedDistancesForMinions,
                          std::vector<double> pctSavingsAllMatchedRiders, std::vector<double> pctSavingsForMasters, std::vector<double> pctSavingsForMinions,
                          std::vector<int> waitTimeMatches_all, std::vector<int> waitTimeMatchesForMasters, std::vector<int> waitTimeMatchesForMinions, 
                          std::vector<double> overlapDistances, std::vector<double> pctSharedDist_ALL, std::vector<double> pctOverlapDist_Masters, std::vector<double> pctOverlapDist_Minions) {
    
    _indivMatchedRiderMetrics._inconv_ALL              = pctAllAddedDistances;
    _indivMatchedRiderMetrics._inconv_Masters          = pctAddedDistancesForMasters;
    _indivMatchedRiderMetrics._inconv_Minions          = pctAddedDistancesForMinions;
    
    _indivMatchedRiderMetrics._overlapDist             = overlapDistances;
    _indivMatchedRiderMetrics._overlapPct_ALL          = pctSharedDist_ALL;
    _indivMatchedRiderMetrics._overlapPct_Masters      = pctOverlapDist_Masters;
    _indivMatchedRiderMetrics._overlapPct_Minions      = pctOverlapDist_Minions;
    
    _indivMatchedRiderMetrics._savings_ALL             = pctSavingsAllMatchedRiders;
    _indivMatchedRiderMetrics._savings_Masters         = pctSavingsForMasters;
    _indivMatchedRiderMetrics._savings_Minions         = pctSavingsForMinions;
    
    _indivMatchedRiderMetrics._waitTimeToMatch_ALL     = waitTimeMatches_all;
    _indivMatchedRiderMetrics._waitTimeToMatch_Masters = waitTimeMatchesForMasters;
    _indivMatchedRiderMetrics._waitTimeToMatch_Minions = waitTimeMatchesForMinions;
}