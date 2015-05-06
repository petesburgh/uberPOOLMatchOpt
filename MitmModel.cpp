/* 
 * File:   MitmModel.cpp
 * Author: jonpetersen
 * 
 * Created on March 24, 2015, 5:40 PM
 */

#include "MitmModel.hpp"

MitmModel::MitmModel(const time_t startTime, const time_t endTime, const double maxMatchDistKm, const double minOverlapThreshold, 
        std::set<Request*, ReqComp> initRequests, std::set<OpenTrip*, EtaComp> initOpenTrips, const std::set<Driver*, DriverIndexComp> * drivers,  bool inclInitPickupInSavings) 
        : _startTime(startTime), _endTime(endTime), _maxMatchDistInKm(maxMatchDistKm), _minOverlapThreshold(minOverlapThreshold), _allDrivers(drivers), _inclMinionPickupDistExtMatchesSavingsConstr(inclInitPickupInSavings) {
    _allRequests = initRequests;
    _initOpenTrips = initOpenTrips;
    pSolution = NULL;
}


MitmModel::~MitmModel() {
}

/*
 *  SOLVE MITM PROXY MODEL
 */
bool MitmModel::solve(bool printDebugFiles, Output * pOutput, bool populateInitOpenTrips) {
    
    std::cout << "\n\n--------------------------------------------------\n" << std::endl;
    std::cout << "                SOLVING MITM MODEL\n" << std::endl;
    std::cout << "--------------------------------------------------\n\n" << std::endl;
        
    // step 1: build sets to be updated
    std::set<Request*,  ReqComp> requests = cloneRequests();
    std::set<OpenTrip*, EtaComp> openTrips;
    if( populateInitOpenTrips ) {
        openTrips = cloneOpenTrips();
    }
    
    std::ofstream * pOutFile;
    
    // initialize output file that tracks match candidates
    if( printDebugFiles ) {
        const std::string outputScenPath = pOutput->getOutputScenarioPath();
        const std::string filename = "MITM-match-candidates.txt";
        std::string outPath = outputScenPath + filename;
        pOutFile = new std::ofstream(outPath); 
        initCandidateMatchFile(*pOutFile);
    }
       
    // step 2: loop through all requests
    std::set<Request*, ReqComp> requestsWithNoDrivers;
    
    std::set<Request*, ReqComp>::iterator reqItr;  
    for( reqItr = requests.begin(); reqItr != requests.end(); ++reqItr ) {
                                                  
        time_t currReqTime = (*reqItr)->getReqTime();
        std::set<AssignedTrip*, AssignedTripIndexComp> completedUnmatchedTrips = ModelUtils::getCompletedOpenTrips(currReqTime,openTrips,_assignedTrips);
        
        if( completedUnmatchedTrips.size() > 0 ) {           
            _assignedTrips.insert(completedUnmatchedTrips.begin(),completedUnmatchedTrips.end());
        }  
                   
        // 2.B: search for best open trip to match        
        AssignedTrip * matchedOpenTrip = getBestMatchForCurrMinionRequest(*reqItr,&openTrips,printDebugFiles,pOutFile); 
        
        // step 3: process the match                     
        // IF there is a match, remove the old master and append the set of assigned trips
        if( matchedOpenTrip != NULL ) {
            matchedOpenTrip->setIndex(_assignedTrips.size());            
            removeMasterFromOpenTrips(matchedOpenTrip,&openTrips);    
            _assignedTrips.insert(matchedOpenTrip);              
        } 
        
        // IF no match, then assign new open trip
        else {                  
            // check if the driver from the actual trip is free
            const Event * pMasterDispatchEvent = (*reqItr)->getActualDispatchEvent();              
            OpenTrip * pOpenTrip = createNewOpenTripForUnmatchedRequest((*reqItr)->getActualDriver(), *reqItr, (*reqItr)->getActualDispatchEvent());    
            openTrips.insert(pOpenTrip);
        }
        
        
        // ensure that no FUTURE requests are made
        for( std::set<OpenTrip*, EtaComp>::iterator it = openTrips.begin(); it != openTrips.end(); ++it ) {
            if( (*it)->getMasterDispatcEvent()->timeT > (*reqItr)->getReqTime() ) {
                if( (*it)->getMasterIndex() == (*reqItr)->getRiderIndex() ) {
                    continue;
                }               
            } 
        }   
    }
    
    std::cout << "\n\n\n--- finished processing all requests ---\n\n" << std::endl;
    
    // assign any unmatched (open) trips
    if( openTrips.size() > 0 ) {
        clearRemainingOpenTrips(openTrips, &_assignedTrips);
        assert( openTrips.empty() );        
    } 
    
    
    // assign disqualified trips due to no available driver
    _disqualifiedRequests.insert(requestsWithNoDrivers.begin(),requestsWithNoDrivers.end());
        
    if( printDebugFiles ) {
        pOutFile->close();
    }
    
    // BUILD SOLUTION
    buildSolution(_assignedTrips);
       
    return true;
}


std::set<Request*, ReqComp> MitmModel::cloneRequests() {
    std::set<Request*, ReqComp> requests;
    std::set<Request*, ReqComp>::iterator reqItr;
    for( reqItr = _allRequests.begin(); reqItr != _allRequests.end(); ++reqItr ) {
        requests.insert(*reqItr);
    }
    
    return requests;
}
std::set<OpenTrip*, EtaComp> MitmModel::cloneOpenTrips() {
    std::set<OpenTrip*, EtaComp> openTrips;
    for( std::set<OpenTrip*>::iterator itr = _initOpenTrips.begin(); itr != _initOpenTrips.end(); ++itr ) {
        openTrips.insert(*itr);
    }
    return openTrips;
}

std::set<AssignedTrip*, AssignedTripIndexComp> MitmModel::getAllCompletedOpenTrips(time_t &tm, std::set<OpenTrip*,EtaComp> &openTrips) {
    std::set<AssignedTrip*, AssignedTripIndexComp> unmatchedTrips;
    
    // iterate over open trips and check for completion
    std::set<OpenTrip*, EtaComp>::iterator openTripItr;
    std::set<OpenTrip*, EtaComp>::iterator lastDeletionItr;
    for( openTripItr = openTrips.begin(); openTripItr != openTrips.end(); ++openTripItr ) {
        
        // terminate loop if ETA is after current time epoch (openTrips are sorted by ETA)
        if( (*openTripItr)->getETD() > tm ) {
            lastDeletionItr = openTripItr;
            break;
        }
        
        // convert expired open trip to AssignedTrip that is unmatched
        AssignedTrip * pAssignedTrip = ModelUtils::convertOpenTripToAssignedTrip(*openTripItr);
        pAssignedTrip->setIndex(_assignedTrips.size());
        _assignedTrips.insert(pAssignedTrip);
                
        unmatchedTrips.insert(pAssignedTrip);
        lastDeletionItr = openTripItr;
    }
    
    // now delete all expired open trips
    if( lastDeletionItr != openTrips.begin() ) {
        openTrips.erase(openTrips.begin(), lastDeletionItr);
    }
    
    return unmatchedTrips;
}
void MitmModel::clearRemainingOpenTrips(std::set<OpenTrip*, EtaComp> &openTrips, std::set<AssignedTrip*, AssignedTripIndexComp> * pAssignedTrips) {
    //std::set<AssignedTrip*, AssignedTripIndexComp> finalUnmatchedTrips;
    
    for( std::set<OpenTrip*, EtaComp>::iterator tripItr = openTrips.begin(); tripItr != openTrips.end(); ++tripItr ) {                
        AssignedTrip * pAssignedTrip = ModelUtils::convertOpenTripToAssignedTrip(*tripItr);               
        pAssignedTrip->setIndex(pAssignedTrips->size());    
        pAssignedTrips->insert(pAssignedTrip);
    }
    
    // remove all remaining open trips
    openTrips.erase(openTrips.begin(), openTrips.end());        
}

/*
 *  given minion request and current open trips (candidate masters), get best master to match with minion 
 *     if no feasible matches exist, return NULL
 */
AssignedTrip * MitmModel::getBestMatchForCurrMinionRequest(Request * pMinionRequest, std::set<OpenTrip*, EtaComp> * pMasterCandidates, bool printDebugFiles, std::ofstream * pOutFile) {
    
    // if there are no master candidate then a match is not possible
    if( pMasterCandidates->size() == 0 )
        return NULL;
    
    // search for every candidate master
    std::multimap<const double, FeasibleMatch*> candidateMatchMap; // key: objective, value: master with which may be matched to minion
    std::set<OpenTrip*, EtaComp>::iterator masterItr;
    
    if( printDebugFiles ) {
        *pOutFile << "\nMINION REQUEST:  " << Utility::convertTimeTToString(pMinionRequest->getReqTime()) << std::endl;
        *pOutFile << "RIDER INDEX:  " << Utility::intToStr(pMinionRequest->getRiderIndex()) << std::endl;
        *pOutFile << "ORIG:  (" << std::setprecision(15) << pMinionRequest->getPickupLat() << "," << std::setprecision(15) << pMinionRequest->getPickupLng() << ")" << std::endl;
        *pOutFile << "DEST:  (" << std::setprecision(15) << pMinionRequest->getDropoffLat() << "," << std::setprecision(15) << pMinionRequest->getDropoffLng() << ")" << std::endl;      
    }
    
    for( masterItr = pMasterCandidates->begin(); masterItr != pMasterCandidates->end(); ++masterItr ) {
        
        // step 0: ensure minion and master rider IDs differ
        if( pMinionRequest->getRiderIndex() == (*masterItr)->getMasterIndex() )
            continue;        
        
        // step 1: ensure that the open trip dispatch time is after the request dispatch 
        if( (*masterItr)->getMasterDispatcEvent()->timeT >= pMinionRequest->getReqTime() )
            continue;
                
        // step 2: ensure the minion request is in between master dispatch and dropoff
        if( pMinionRequest->getReqTime() <= (*masterItr)->getMasterDispatcEvent()->timeT ) {
            continue;
        }
                
        if( pMinionRequest->getReqTime() >= (*masterItr)->getETD() ) {
            continue;
        }
        
               
        // step 3: get feasible matches depending upon dropoff sequence (same master,minion pair)        
        std::vector<FeasibleMatch*> feasibleMatches = getFeasibleMatchesFromCurrPair(pMinionRequest, *masterItr);        
        if( feasibleMatches.size() > 0 ) {
            for( std::vector<FeasibleMatch*>::iterator iMatch = feasibleMatches.begin(); iMatch != feasibleMatches.end(); ++iMatch ) {
                const double currCost = computeCostOfMatch(*iMatch);
                candidateMatchMap.insert(make_pair(currCost,*iMatch));                
            }
        }
    }
    
    // if no feasible candidate found return NULL to indicate no match
    if( candidateMatchMap.empty() )
        return NULL;
    else {
        if( printDebugFiles ) {
            printMatchCandidatesForCurrRequest(&candidateMatchMap, pOutFile);
        }
        
        // convert FeasibleTrip* object into AssignedTrip* object and return
        AssignedTrip * pAssignedTrip = convertFeasibleMatchToAssignedTripObject(candidateMatchMap.begin()->second); 
          
        
        return pAssignedTrip;
    }
}

// given minion,master pair check for feasibility
std::vector<FeasibleMatch*> MitmModel::getFeasibleMatchesFromCurrPair(Request* pMinionReq, OpenTrip * pMasterCand) {
    
    std::vector<FeasibleMatch*> feasibleAssignments; 
        
    // check 1: pickup distance is feasible
    const double pickupDistanceAtTimeOfMinionRequest = ModelUtils::getPickupDistanceAtTimeOfMinionRequest_maxPickupConstr(
            pMinionReq->getReqTime(), pMinionReq->getPickupLat(), pMinionReq->getPickupLng(), pMasterCand->getETA(), 
            pMasterCand->getActPickupLat(), pMasterCand->getActPickupLng(), pMasterCand->getETD(), 
            pMasterCand->getDropRequestLat(), pMasterCand->getDropRequestLng(), pMasterCand->getMasterDispatcEvent()->timeT,
            pMasterCand->getMasterDispatcEvent()->lat, pMasterCand->getMasterDispatcEvent()->lng);
        
    if( pickupDistanceAtTimeOfMinionRequest <= _maxMatchDistInKm ) {
            
        // check 2: trip overlap exceeds threshold
        const double pickupDistanceToMinion = ModelUtils::computePickupDistance_savingsConstr(pMasterCand->getETA(), pMasterCand->getActPickupLat(), pMasterCand->getActPickupLng(), pMasterCand->getETD(), pMasterCand->getDropRequestLat(), pMasterCand->getDropRequestLng(), pMinionReq->getReqTime(), pMinionReq->getPickupLat(), pMinionReq->getPickupLng(), _inclMinionPickupDistExtMatchesSavingsConstr);
        const double uberXdistanceForMaster = Utility::computeGreatCircleDistance(pMasterCand->getActPickupLat(), pMasterCand->getActPickupLng(), pMasterCand->getDropRequestLat(), pMasterCand->getDropRequestLng());
        const double uberXdistanceForMinion = Utility::computeGreatCircleDistance(pMinionReq->getPickupLat(), pMinionReq->getPickupLng(), pMinionReq->getDropoffLat(), pMinionReq->getDropoffLng());

        //      2.A:  check match with master dropoff first
        FeasibleMatch * pFeasMatchWithMasterDropoffFirst = checkIfFIFOMatchIsFeasible(pMinionReq->getRiderID(), pickupDistanceToMinion, uberXdistanceForMaster, uberXdistanceForMinion, pMinionReq, pMasterCand);
        //FeasibleMatch * pFeasMatchWithMasterDropoffFirst = ModelUtils::checkIfOverlapIsFeasWithforFIFOMatch(_minOverlapThreshold, pMinionReq->getRiderID(), distFromMasterToMinion, uberXdistanceForMaster, uberXdistanceForMinion, )
        if( pFeasMatchWithMasterDropoffFirst != NULL ) {
            feasibleAssignments.push_back(pFeasMatchWithMasterDropoffFirst);
        }    
        
        //      2.B:  check with minion dropoff first
        FeasibleMatch * pFeasMatchWithMinionDropoffFirst = checkIfFILOMatchIsFeasible(pickupDistanceToMinion, uberXdistanceForMaster, uberXdistanceForMinion, pMinionReq, pMasterCand);
        if( pFeasMatchWithMinionDropoffFirst != NULL ) {
            feasibleAssignments.push_back(pFeasMatchWithMinionDropoffFirst);
        }
    }
    
    return feasibleAssignments;
}
FeasibleMatch * MitmModel::checkIfFIFOMatchIsFeasible(const std::string minionId, const double distToMinion, const double masterUberXDist, const double minionUberXDist, Request* pMinionReq, OpenTrip * pMasterCand) {
    
    /*
     *   MASTER constraint: s1 + 0.5h <= (1-t)x 
     *   MINION constraint: 0.5h + s2 <= (1-t)y where
     *      s1 = distance of single leg to pick up minion 
     *      s2 = distance of single leg to drop off minion
     *      h  = distance of shared leg 
     *      x  = distance of uberX trip for master 
     *      y  = distance of uberX trip for minion
     *      t  = threshold [0,1] savings required for legality
     */
    
    // step 1: compute h: distance of shared leg with MASTER being dropped first (Haversine)
    const double minionPickupLat  = pMinionReq->getPickupLat();
    const double minionPickupLng  = pMinionReq->getPickupLng();
    const double masterDropoffLat = pMasterCand->getDropRequestLat();
    const double masterDropoffLng = pMasterCand->getDropRequestLng();
    const double sharedDistance   = Utility::computeGreatCircleDistance(minionPickupLat, minionPickupLng, masterDropoffLat, masterDropoffLng);
    const double dropDistance     = Utility::computeGreatCircleDistance(masterDropoffLat, masterDropoffLng, pMinionReq->getDropoffLat(), pMinionReq->getDropoffLng());
    
    // step 2: compute LHS representing cost of matched uberPOOL trip
    const double distance_pool_master = distToMinion + 0.5*sharedDistance; // distance of pooled trip for master (deadhead to pickup + 0.5*shared distance)
    const double distance_pool_minion = 0.5*sharedDistance + dropDistance; // distance of pooled trip for minion (0.5*shared distance + deadhead to drop)
    
    // step 3: compute RHS representing cost of unmatched uberPOOL trip
    const double distance_uberX_master = (1-_minOverlapThreshold)*masterUberXDist;
    const double distance_uberX_minion = (1-_minOverlapThreshold)*minionUberXDist;
    
    // step 4: check feasibility for both master and minion
    bool isFeasForMaster = ( distance_pool_master <= distance_uberX_master );
    bool isFeasForMinion = ( distance_pool_minion <= distance_uberX_minion );
    
    // step 5: check if feasible and if so create AssignedTrip object
    if( isFeasForMaster && isFeasForMinion ) {
        LatLng masterOrig = LatLng(pMasterCand->getActPickupLat(), pMasterCand->getActPickupLng());
        LatLng masterDest = LatLng(pMasterCand->getDropRequestLat(), pMasterCand->getDropRequestLng());
        LatLng minionOrig = LatLng(pMinionReq->getPickupLat(), pMinionReq->getPickupLng());
        LatLng minionDest = LatLng(pMinionReq->getDropoffLat(), pMinionReq->getDropoffLng());                
        const double dropDist = Utility::computeGreatCircleDistance(masterDropoffLat, masterDropoffLng, pMinionReq->getDropoffLat(), pMinionReq->getDropoffLng());
        const double totalDistMaster = distToMinion + sharedDistance;
        const double totalDistMinion = sharedDistance + dropDist;
        const double uberXDistMinion = Utility::computeGreatCircleDistance(pMinionReq->getPickupLat(), pMinionReq->getPickupLng(), pMinionReq->getDropoffLat(), pMinionReq->getDropoffLng());
        const bool masterPickedUpAtTimeOfMatch = (pMasterCand->getETA() <= pMinionReq->getReqTime());   
        const double addlDistMaster = (double)totalDistMaster - (double)masterUberXDist;
        const double addlDistMinion = (double)totalDistMinion - (double)uberXDistMinion;
        const double pctAddlDistMaster = (double)100*(double)addlDistMaster/(double)masterUberXDist;
        const double pctAddlDistMinion = (double)100*(double)addlDistMinion/(double)uberXDistMinion;
        
        const double masterSavings = abs(distance_pool_master - distance_uberX_master);
        const double minionSavings = abs(distance_pool_minion - distance_uberX_minion);
        const double avgSavings    = (masterSavings + minionSavings)/(double)2;
        
        // get driver & master location at time of minion request
        LatLng driverLocAtTimeOfMinionReq = ModelUtils::computeMasterDriverLocAtTimeOfMinionReq(pMasterCand->getMasterDispatcEvent(), pMasterCand->getMasterActualPickupEvent(), pMasterCand->getMasterActualDropEvent(), pMinionReq->getReqTime());
        LatLng masterLocAtTimeOfMinionReq = ModelUtils::computeMasterLocAtTimeOfMinionReq(pMasterCand->getMasterActualPickupEvent(), pMasterCand->getMasterActualDropEvent(), pMinionReq->getReqTime());
                
        FeasibleMatch * pFeasMatch = new FeasibleMatch(pMasterCand->getDriver(), pMasterCand->getMasterID(), pMasterCand->getMasterIndex(), pMasterCand->getRiderTripUUID(), minionId, pMinionReq->getRiderIndex(), pMinionReq->getRiderTripUUID(),
                    true, pMasterCand->getMasterRequestEvent(), pMasterCand->getMasterDispatcEvent(), masterPickedUpAtTimeOfMatch, distToMinion, sharedDistance, dropDist, totalDistMaster, totalDistMinion, masterUberXDist, 
                    uberXDistMinion, pMasterCand->getMasterRequestEvent()->timeT, pMasterCand->getMasterDispatcEvent()->timeT, pMasterCand->getETA(), pMasterCand->getETD(), pMasterCand->getETD(), pMasterCand->getMasterActualPickupEvent(), pMasterCand->getMasterActualDropEvent(), masterOrig, masterDest, pctAddlDistMaster,
                    pMinionReq->getReqTime(), -1, -1, -1, minionOrig, minionDest, pctAddlDistMinion,
                    masterSavings, minionSavings, avgSavings, -1, pMinionReq->getReqIndex(), driverLocAtTimeOfMinionReq, masterLocAtTimeOfMinionReq);  // note: -1 added at the end to indicate the master was from an OpenTrip        
        return pFeasMatch;
    } else {
        return NULL;
    }
}
FeasibleMatch * MitmModel::checkIfFILOMatchIsFeasible(const double distToMinion, const double masterUberXDist, const double minionUberXDist, Request* pMinionReq, OpenTrip * pMasterCand) {
    
    /*
     *   MASTER constraint: (s1 + 0.5h + s2) <= (1-t)x 
     *   MINION constraint: 0.5h <= (1-t)y where
     *      s1 = distance of single leg to pick up minion 
     *      h  = distance of shared leg 
     *      s2 = distance of single leg from minion drop to master drop
     *      x  = distance of uberX trip for master 
     *      y  = distance of uberX trip for minion
     *      t  = threshold [0,1] discount required for legality
     */    
    
    // step 1: compute h: distance of shared leg with MINION being dropped first (Haversine) 
    const double minionPickupLat  = pMinionReq->getPickupLat();
    const double minionPickupLng  = pMinionReq->getPickupLng();
    const double minionDropoffLat = pMinionReq->getDropoffLat();
    const double minionDropoffLng = pMinionReq->getDropoffLng();
    const double sharedDistance   = Utility::computeGreatCircleDistance(minionPickupLat, minionPickupLng, minionDropoffLat, minionDropoffLng);
    
    // step 2: compute s2: distance from minion drop to master drop
    const double masterDropLat    = pMasterCand->getDropRequestLat();
    const double masterDropLng    = pMasterCand->getDropRequestLng();
    const double distToMasterDrop = Utility::computeGreatCircleDistance(minionDropoffLat, minionDropoffLng, masterDropLat, masterDropLng);
    
    // step 3: compute LHS representing cost of matched uperPOOL trip
    const double distance_pool_master = distToMinion + 0.5*sharedDistance + distToMasterDrop;
    const double distance_pool_minion = 0.5*sharedDistance;
    
    // step 4: compute RHS representing cost of unmatched POOL trip
    const double distance_uberX_master = (1-_minOverlapThreshold)*masterUberXDist;
    const double distance_uberX_minion = (1-_minOverlapThreshold)*minionUberXDist;
    
    // step 5: check feasibility for both master and minion
    bool isFeasForMaster = ( distance_pool_master <= distance_uberX_master );
    bool isFeasForMinion = ( distance_pool_minion <= distance_uberX_minion );
    
    // step 5: return FeasibleMatch * object (if cost savings) or NULL (if match does not save)
    if ( isFeasForMaster && isFeasForMinion ) {
        LatLng masterOrig = LatLng(pMasterCand->getActPickupLat(), pMasterCand->getActPickupLng());
        LatLng masterDest = LatLng(pMasterCand->getDropRequestLat(), pMasterCand->getDropRequestLng());
        LatLng minionOrig = LatLng(pMinionReq->getPickupLat(), pMinionReq->getPickupLng());
        LatLng minionDest = LatLng(pMinionReq->getDropoffLat(), pMinionReq->getDropoffLng());
        const double totalDistMaster = distToMinion + sharedDistance + distToMasterDrop;
        const bool masterPickedUpAtTimeOfMatch = (pMasterCand->getETA() <= pMinionReq->getReqTime());
        const double minionUberXDist = Utility::computeGreatCircleDistance(pMinionReq->getPickupLat(), pMinionReq->getPickupLng(), pMinionReq->getDropoffLat(), pMinionReq->getDropoffLng());
        const double addlDistMaster = (double)totalDistMaster - (double)masterUberXDist;
        
        const double pctAddlDistMaster = (double)100*(double)addlDistMaster/(double)masterUberXDist;
        const double pctAddlDistMinion = 0.0;
        
        const double masterSavings = abs(distance_pool_master - distance_uberX_master);
        const double minionSavings = abs(distance_pool_minion - distance_uberX_minion);
        const double avgSavings    = (masterSavings+minionSavings)/(double)2;
        
        // get driver & master location at time of minion request
        LatLng driverLocAtTimeOfMinionReq = ModelUtils::computeMasterDriverLocAtTimeOfMinionReq(pMasterCand->getMasterDispatcEvent(), pMasterCand->getMasterActualPickupEvent(), pMasterCand->getMasterActualDropEvent(), pMinionReq->getReqTime());
        LatLng masterLocAtTimeOfMinionReq = ModelUtils::computeMasterLocAtTimeOfMinionReq(pMasterCand->getMasterActualPickupEvent(), pMasterCand->getMasterActualDropEvent(), pMinionReq->getReqTime());
        
        FeasibleMatch * pFeasMatch = new FeasibleMatch(pMasterCand->getDriver(), pMasterCand->getMasterID(), pMasterCand->getMasterIndex(), pMasterCand->getRiderTripUUID(), 
                pMinionReq->getRiderID(), pMinionReq->getRiderIndex(), pMinionReq->getRiderTripUUID(), false, pMasterCand->getMasterRequestEvent(), pMasterCand->getMasterDispatcEvent(), masterPickedUpAtTimeOfMatch,
                distToMinion, sharedDistance, distToMasterDrop, totalDistMaster, sharedDistance, masterUberXDist, minionUberXDist,
                pMasterCand->getMasterRequestEvent()->timeT, pMasterCand->getMasterDispatcEvent()->timeT, pMasterCand->getETA(), -1, pMasterCand->getETD(), pMasterCand->getMasterActualPickupEvent(), pMasterCand->getMasterActualDropEvent(), masterOrig, masterDest, pctAddlDistMaster,
                pMinionReq->getReqTime(), pMinionReq->getReqTime(), -1, -1, minionOrig, minionDest, pctAddlDistMinion,
                masterSavings, minionSavings, avgSavings, -1, pMinionReq->getReqIndex(), driverLocAtTimeOfMinionReq, masterLocAtTimeOfMinionReq );    
        return pFeasMatch;
    } else {
        return NULL;
    }
}

/*
 *   compute the cost of a candidate match
 *     - computed as difference in driver distance from master pickup to final drop
 *     - note: when pickup swaps are to be considered, then we need to also consider distance to master pickup (if pickup has not occurred)
 */
const double MitmModel::computeCostOfMatch(FeasibleMatch * pMatch) const {
    
    // get the estimated location of the master at the time of 
    
    
    // get the distance from the current location of the car to the master origin
    double distFromDispatchToMasterPickup = 0.0;
    if( pMatch->_masterPickedUpAtTimeOfMatch == false ) {  
        LatLng estLocOfCarAtTimeOfMinionReq = Utility::estLocationByLinearProxy(pMatch->_minionRequest, pMatch->_masterDispatch, pMatch->_masterDispatchEvent->lat, pMatch->_masterDispatchEvent->lng, pMatch->_masterPickup, pMatch->_masterOrig.getLat(), pMatch->_masterOrig.getLng());
        distFromDispatchToMasterPickup = Utility::computeGreatCircleDistance(estLocOfCarAtTimeOfMinionReq.getLat(), estLocOfCarAtTimeOfMinionReq.getLng(), pMatch->_masterOrig.getLat(), pMatch->_masterOrig.getLng());
    }

    const double distToMinionPickup = pMatch->_distToMinionPickup;  // from current location to minion origin
    const double sharedDist         = pMatch->_sharedDistance;
    const double dropDist = pMatch->_distFromFirstToSecondDrop;
    
    const double totalPooledDistance =  distFromDispatchToMasterPickup + distToMinionPickup + sharedDist + dropDist;
    const double uberXDistMaster     = pMatch->_uberXDistanceForMaster;
    
    double addedDistance = totalPooledDistance - uberXDistMaster;
    
    /*if( addedDistance < -0.1 ) {
        std::cout << "\n\nuberX dist: " << Utility::doubleToStr(uberXDistMaster) << std::endl;
        std::cout << "total pooled dist: " << Utility::doubleToStr(totalPooledDistance) << std::endl;
        std::cout << "\tdist to minion pickup: " << Utility::doubleToStr(distToMinionPickup) << std::endl;
        std::cout << "\tshared dist: " << Utility::doubleToStr(sharedDist) << std::endl;
        std::cout << "\tdrop dist: " << Utility::doubleToStr(dropDist) << std::endl;     
        std::cout << "\tadded dist: " << Utility::doubleToStr(addedDistance) << std::endl;
    }
    assert(addedDistance >= -0.1);*/
    
    if( 0.0 < abs(addedDistance) < 0.0001 ) {
        addedDistance = 0.0;
    }
    
    const double cost = addedDistance;
      
    return cost;
}

AssignedTrip * MitmModel::convertFeasibleMatchToAssignedTripObject(FeasibleMatch * pMatch) {
    
    AssignedTrip * pTrip = new AssignedTrip(pMatch->pDriver, pMatch->_masterDispatchEvent, pMatch->_masterTripUUID, pMatch->_masterRequestEvent, pMatch->_masterPickupEventFromActuals, pMatch->_masterDropEventFromActuals, pMatch->_masterReqIndex);
    pTrip->setMasterId(pMatch->_masterId);
    pTrip->setMinionId(pMatch->_minionId);
    pTrip->setMasterIndex(pMatch->_masterIndex);
    pTrip->setMinionIndex(pMatch->_minionIndex);
    pTrip->setMinionTripUUID(pMatch->_minionTripUUID);
    
    pTrip->setMatch(pMatch);

    return pTrip;
}
bool MitmModel::removeMasterFromOpenTrips(AssignedTrip* pMatchedTrip, std::set<OpenTrip*,EtaComp>* pOpenTrips) {
    
    for( std::set<OpenTrip*, EtaComp>::iterator iTrip = pOpenTrips->begin(); iTrip != pOpenTrips->end(); ) {   
        if( (*iTrip)->getMasterID() == pMatchedTrip->getMasterId() ) {             
            pOpenTrips->erase(iTrip++);                        
            return true;
        } else {
            iTrip++;
        }
    }
    
    return false;
}

// methods to determine nearest driver
std::pair<Driver*, std::pair<double,double> > * MitmModel::getNearestDriverToDispatchRequest(std::set<OpenTrip*, EtaComp> * pOpenTrips, time_t reqTime, double reqLat, double reqLng, const Driver * pActualDriver) {
    
    // instantiate map whose key is distance from minion request and value is a pair of Driver and location info
    std::multimap<const double, std::pair<Driver*,LatLng> > eligDriverMap;
        
    for( std::set<Driver*>::iterator iDriver = _allDrivers->begin(); iDriver != _allDrivers->end(); ++iDriver ) {
        bool isDriverAssignedToOpenTrip = checkIfDriverAssignedToOpenTrip(reqTime,*iDriver,pOpenTrips);
        if( !isDriverAssignedToOpenTrip ) {
            
            // estimate location from trip actuals
            LatLng estLocation = getEstLocationOfOpenDriver(*iDriver, reqTime);
         
            try {
                const double distToReq = Utility::computeGreatCircleDistance(estLocation.getLat(), estLocation.getLng(), reqLat, reqLng);
                eligDriverMap.insert(make_pair(distToReq, make_pair(*iDriver,estLocation)));
            } catch ( TimeAdjacencyException &ex ) {
                std::cerr << "\n\n*** TimeAdjacencyException caught ***" << std::endl;
                std::cerr << ex.what() << std::endl;
                std::cerr << "\texiting... " << std::endl;
                exit(1);
            }        
        }
    }
    
    // case 1: there is at least one driver: return the nearest one
    if( eligDriverMap.size() > 0 ) {
       
        std::pair<Driver*, std::pair<double,double> > * pNearestDriver = new std::pair<Driver*, std::pair<double, double> >();
        pNearestDriver->first  = eligDriverMap.begin()->second.first;
        double estDriverLat = eligDriverMap.begin()->second.second.getLat(); 
        double estDriverLng = eligDriverMap.begin()->second.second.getLng();
       
        pNearestDriver->second = make_pair(estDriverLat, estDriverLng);
        return pNearestDriver;
    } 
    
    // case 2: no drivers (return null and remove from consideration)
    else {
        return NULL;
    }

}
bool MitmModel::checkIfDriverAssignedToOpenTrip( time_t currTime, Driver * pDriver, std::set<OpenTrip*, EtaComp>* pOpenTrips ) {
    std::set<OpenTrip*, EtaComp>::iterator openTripItr;
    for( openTripItr = pOpenTrips->begin(); openTripItr != pOpenTrips->end(); ++openTripItr ) {
        
        // first check if driver has an open trip
        if( (*openTripItr)->getDriverID() == pDriver->getId() ) {
            
            // now check if open trip is ongoing
            bool isAfterMasterDispatch = ((*openTripItr)->getMasterDispatcEvent()->timeT <= currTime);
            bool isBeforeETD  = (currTime < (*openTripItr)->getETD());
            if( isAfterMasterDispatch && isBeforeETD ) {
                return true;
            }
            
        }
    }
    
    return false;
}
std::pair<const TripData*, const TripData*> * MitmModel::getAdjacentTrips(const Driver * pDriver, const time_t reqTime) {
    
    const std::vector<TripData*> * pTrips = pDriver->getTrips();
    
    if( pTrips->size() > 1 ) {
        std::vector<TripData*>::const_iterator iTrip;
        try {
        for( int ii = 0; ii < pTrips->size() - 1; ii++ ) {
            TripData * earlierTrip = pTrips->at(ii);
            TripData * nextTrip = pTrips->at(ii+1);
            
            time_t previousTripEnd = earlierTrip->getDropoffEvent()->timeT;
            time_t nextTripBegin = nextTrip->getPickupEvent()->timeT;
            if( (previousTripEnd <= reqTime) && (reqTime <= nextTripBegin) ) {
                std::pair<const TripData*, const TripData*> * adjTrips = new std::pair<const TripData*, const TripData*>();
                adjTrips->first  = pTrips->at(ii);
                adjTrips->second = pTrips->at(ii+1);
                return adjTrips;
            }
        }
        } catch( exception &e ) {
            std::cout << "\n\n*** ERROR ***" << std::endl;
            std::cout << e.what() << std::endl;
            exit(1);
        }
    }
    
    return NULL;
}
LatLng MitmModel::getEstLocationOfOpenDriver(const Driver * pDriver, const time_t &reqTime) {
    
    // extract adjacent trips
    std::pair<const TripData *, const TripData *> * adjacentTrips = getAdjacentTrips(pDriver, reqTime);
    if( adjacentTrips != NULL ) {
       
        // extract information used to estimate location
        const double previousEndTime = adjacentTrips->first->getDropoffEvent()->timeT;
        const double previousEndLat  = adjacentTrips->first->getDropoffEvent()->lat;
        const double previousEndLng  = adjacentTrips->first->getDropoffEvent()->lng;
        const double nextStartTime   = adjacentTrips->second->getPickupEvent()->timeT;
        const double nextStartLat    = adjacentTrips->second->getPickupEvent()->lat;
        const double nextStartLng    = adjacentTrips->second->getPickupEvent()->lng;
                

        try {
            // estimate driver location (linear approx)
            LatLng estLocation = Utility::estLocationByLinearProxy(reqTime, previousEndTime, previousEndLat, previousEndLng, nextStartTime, nextStartLat, nextStartLng);                    
            return estLocation;

        } catch( TimeAdjacencyException &ex ) {
            std::cerr << "\n*** TimeAdjacencyException thrown ***" << std::endl;
            std::cerr << ex.what() << std::endl;
            exit(-1);                    
        }
    } else {
        // assume initial driver location for now
        const double initDriverLat = pDriver->getTrips()->front()->getPickupEvent()->lat;
        const double initDriverLng = pDriver->getTrips()->front()->getPickupEvent()->lng;
        LatLng initLoc = LatLng(initDriverLat,initDriverLng);
        return initLoc;              
    }    
}
OpenTrip * MitmModel::createNewOpenTripForUnmatchedRequest(const Driver * pNearestDriver, Request * pMinionRequest, const Event * pActualDispatchEvent) {
    
    const Event * pReqEvent    = new Event(pMinionRequest->getReqTime(), pMinionRequest->getPickupLat(), pMinionRequest->getPickupLng());
 //   const Event * pDispEvent   = new Event(pMinionRequest->getReqTime(), driverDispatchLoc.first, driverDispatchLoc.second );
    assert( pActualDispatchEvent != NULL );
   
    OpenTrip * pOpenTrip = new OpenTrip(pNearestDriver->getId(), pNearestDriver, pNearestDriver->getIndex(), pMinionRequest->getRiderID(), 
            pMinionRequest->getRiderIndex(), pMinionRequest->getRiderTripUUID(), pReqEvent, pActualDispatchEvent, pMinionRequest->getPickupLat(), 
            pMinionRequest->getPickupLng(), pMinionRequest->getDropoffLat(), pMinionRequest->getDropoffLng(), pMinionRequest->getActTimeOfPickupFromTripActuals(), 
            pMinionRequest->getActTimeOfDropoffFromTripActuals(),pMinionRequest->getActualPickupEvent(), pMinionRequest->getActualDropEvent(), pMinionRequest->getReqIndex());
    
    return pOpenTrip;
}


// --------------------
//    build solution
// --------------------
void MitmModel::buildSolution(std::set<AssignedTrip*, AssignedTripIndexComp> &assignedTrips) {
    
    // instantiate solution
    pSolution = new Solution(MITM_SEQ_PICKUPS, _startTime, _endTime, _allRequests.size(), _allDrivers->size(), assignedTrips, _disqualifiedRequests);
    
    // compute solution metrics
    pSolution->buildSolutionMetrics();    
}

// --------------------
//     I/O goodies
// --------------------
void MitmModel::initCandidateMatchFile(std::ofstream &outFile) {    
    outFile << "\n----------------------------------------\n" << std::endl;
    outFile << "       MITM Match Candidates" << std::endl;
    outFile << "\n----------------------------------------\n\n" << std::endl;      
}
void MitmModel::printMatchCandidatesForCurrRequest(std::multimap<const double, FeasibleMatch*> * pCandMatchMap, std::ofstream * pOut) {
    
    int costBuff = 20;
    int masterIdBuff = 11;
    int extBuff = 13;
    int dropBuff = 12;
    int distBuff = 22;
    int locBuff = 32;
    
    *pOut << left << setw(costBuff) << "cost" << 
            left << setw(masterIdBuff) << "masterID" << 
            left << setw(extBuff) << "isExtended" << 
            left << setw(dropBuff) << "firstDrop" << 
            left << setw(locBuff) << "masterPickup" << 
            left << setw(locBuff) << "masterDrop" <<            
            left << setw(distBuff) << "distToMinionPickup" << 
            left << setw(distBuff) << "sharedDist" << 
            left << setw(distBuff) << "dropDist" << 
            left << setw(distBuff) << "totalDistMaster" << 
            left << setw(distBuff) << "uberXDistMaster" << 
            left << setw(distBuff) << "totalDistMinion" << 
            left << setw(distBuff) << "uberXDistMinion" <<
            std::endl;
    
    std::multimap<const double, FeasibleMatch*>::iterator matchItr;
    for( matchItr = pCandMatchMap->begin(); matchItr != pCandMatchMap->end(); ++matchItr ) {
        
        const bool isSeqDrop = matchItr->second->_fixedDropoff;
        std::string firstDropStr = (isSeqDrop) ? "MASTER" : "MINION";
        std::string isExtStr = (matchItr->second->_masterPickedUpAtTimeOfMatch) ? "yes" : "no";
        
        std::string masterPickupLoc = "(";
        masterPickupLoc += Utility::doubleToStr(matchItr->second->_masterOrig.getLat());
        masterPickupLoc += ",";
        masterPickupLoc += Utility::doubleToStr(matchItr->second->_masterOrig.getLng());
        masterPickupLoc += ")";
        
        std::string masterDropLoc = "(";
        masterDropLoc += Utility::doubleToStr(matchItr->second->_masterDest.getLat());
        masterDropLoc += ",";
        masterDropLoc += Utility::doubleToStr(matchItr->second->_masterDest.getLng());
        masterDropLoc += ")";
        
        *pOut << left << setw(costBuff) << std::setprecision(7) << Utility::doubleToStr(matchItr->first) <<
                left << setw(masterIdBuff) << Utility::intToStr(matchItr->second->_masterIndex) << 
                left << setw(extBuff) << isExtStr << 
                left << setw(dropBuff) << firstDropStr <<
                left << setw(locBuff) << masterPickupLoc <<
                left << setw(locBuff) << masterDropLoc <<
                left << setw(distBuff) << Utility::doubleToStr(matchItr->second->_distToMinionPickup) << 
                left << setw(distBuff) << Utility::doubleToStr(matchItr->second->_sharedDistance) << 
                left << setw(distBuff) << Utility::doubleToStr(matchItr->second->_distFromFirstToSecondDrop) << 
                left << setw(distBuff) << Utility::doubleToStr(matchItr->second->_totalDistanceForMaster) <<               
                left << setw(distBuff) << Utility::doubleToStr(matchItr->second->_uberXDistanceForMaster) << 
                left << setw(distBuff) << Utility::doubleToStr(matchItr->second->_totalDistanceForMinion) << 
                left << setw(distBuff) << Utility::doubleToStr(matchItr->second->_uberXDistanceForMinion) << 
               std::endl; 
    }
    *pOut << "\n" << std::endl;
    
}
