/* 
 * File:   UFBW_fixed.cpp
 * Author: jonpetersen
 * 
 * Created on April 3, 2015, 8:33 AM
 */

#include "UFBW_fixed.hpp"
#include "ModelUtils.hpp"

UFBW_fixed::UFBW_fixed(const time_t startTime, const time_t endTime, const int lenBatchWindow, const double maxMatchDistKm, const double minOverlapThreshold, std::set<Request*, ReqComp> initRequests, std::set<OpenTrip*, EtaComp> initOpenTrips, const std::set<Driver*, DriverIndexComp> * drivers) : 
        _startTime(startTime), _endTime(endTime), _lenBatchWindowInSec(lenBatchWindow), _maxMatchDistInKm(maxMatchDistKm), _minOverlapThreshold(minOverlapThreshold), _allRequests(initRequests), _initOpenTrips(initOpenTrips), _allDrivers(drivers) {
    
    _batchCounter = 0;    
}

UFBW_fixed::~UFBW_fixed() {
}
#include "MitmModel.hpp"


bool UFBW_fixed::solve(bool printDebugFiles, Output * pOutput, bool populateInitOpenTrips, bool printToScreen) {
    std::cout << "\n\n---------------------------------------------------------\n" << std::endl;
    std::cout << "     SOLVING UP FRONT BATCHING OPT WITH FIXED PICKUPS\n" << std::endl;
    std::cout << "---------------------------------------------------------\n\n" << std::endl;   
    
    
    std::ofstream * pOutFile;
    
    // initialize output file that tracks match candidates
    if( printDebugFiles ) {
        const std::string outputBasePath = pOutput->getOutputBasePath();
        const std::string filename = "UFBW-match-candidates.txt";
        std::string outPath = outputBasePath + filename;
        pOutFile = new std::ofstream(outPath); 
        initEligMatchFile(*pOutFile);
    }        
    
    
    // initialize the queue of ALL requests
    std::deque<Request*> requestPool;
            
    // step 1: queue all requests in the simulation horizon
    for( std::set<Request*, ReqComp>::iterator reqItr = _allRequests.begin(); reqItr != _allRequests.end(); ++reqItr ) {
        requestPool.push_back(*reqItr);
    }
    
    // step 2: initialize all open trips
    std::set<OpenTrip*, EtaComp> currOpenTrips;
    if( populateInitOpenTrips ) {
        currOpenTrips = cloneOpenTrips(_initOpenTrips);
    }
    
    // step 3: initialize set of past unmatched requests (before open trip)
    std::set<Request*, ReqComp> unmatchedRequestsWithinWaitTime;
    
        
    std::cout << "\n\nbatch window length:  " << _lenBatchWindowInSec << " sec" << std::endl;
        
    while( true ) {

        // extract Request
        Request * pCurrRequest = requestPool.front();
        const time_t batchWindowBegin = pCurrRequest->getReqTime();
        const time_t batchWindowEnd   = batchWindowBegin + _lenBatchWindowInSec;
                       
        if( printToScreen ) {
            std::cout << "\n\n-------------------------------------------------------------------------\n" << std::endl;
            std::cout << "       CURRENT REQUEST AT TOP OF DEQUE: " << pCurrRequest->getRiderIndex() << " AT " << Utility::convertTimeTToString(pCurrRequest->getReqTime()) << std::endl;
            std::cout << "          batch " << Utility::intToStr(_batchCounter) << " from " << Utility::convertTimeTToString(batchWindowBegin) << " to " << Utility::convertTimeTToString(batchWindowEnd) << std::endl;
            std::cout << "\n-------------------------------------------------------------------------\n\n" << std::endl;  
        }
                              
        // step 1: convert any expired unmatched requests to open trips if they have been waiting more than the tolerance
        if( unmatchedRequestsWithinWaitTime.size() > 0 ) {            
            std::set<Request*, ReqComp>::iterator unmatchItr;            
            std::set<OpenTrip*,EtaComp> newOpenTrips = convertExpiredUnmatchedReqsToOpenTrips(&unmatchedRequestsWithinWaitTime, pCurrRequest->getReqTime());
            currOpenTrips.insert(newOpenTrips.begin(), newOpenTrips.end()); 
        }
        
        // step 2: convert all completed trips to unmatched trips   
        std::set<AssignedTrip*, AssignedTripIndexComp> completedTripsCurrBatch = ModelUtils::getCompletedOpenTrips(pCurrRequest->getReqTime(), currOpenTrips, _assignedTrips);
        
        if( printToScreen ) {
            std::cout << "\t" << completedTripsCurrBatch.size() << " open trips have been completed as unmatched" << std::endl;
        }
              
        // step 3: get ALL requests from the current within the batch window
        std::set<Request*, ReqComp> requestsInCurrBatch = getRequestsInInterval(requestPool, batchWindowBegin, batchWindowEnd);
                        
        // step 4: build candidate MASTERS and MINIONS
        std::pair<std::set<MasterCand*, MasterComp>, std::set<MinionCand*, MinionComp> > candMastersMinions = generateCandidateMastersAndMinions(currOpenTrips, requestsInCurrBatch);
        
           
        if( printToScreen ) {
            std::set<MinionCand*, MinionComp> minions = candMastersMinions.second;
            std::cout << "\t" << minions.size() << " candidate minions: " << std::endl;
            std::set<MinionCand*, MinionComp>::iterator itr;
            for( itr = minions.begin(); itr != minions.end(); ++itr ) {
                std::cout << "\t\tminion " << (*itr)->_riderIndex << " request at " << Utility::convertTimeTToString((*itr)->_reqTime) << std::endl;
            }
        }
        
        // step 5: build FEASIBLE candidate (master,minion) pairs
        if( printToScreen ) { 
            std::cout << "\ttrying to get feasible matches... " << std::endl;
        }
        std::set<UFBW_fixed::MasterMinionMatchCand*, UFBW_fixed::MasterMinionMatchComp> feasibleMatches = generateFeasibleMasterMinionMatches(candMastersMinions.first, candMastersMinions.second);
         
        if( printToScreen ) {
            std::cout << "\t\t" << feasibleMatches.size() << " feasible matches have been identified" << std::endl;                
            for( std::set<UFBW_fixed::MasterMinionMatchCand*, UFBW_fixed::MasterMinionMatchComp>::iterator matchCandItr = feasibleMatches.begin(); matchCandItr != feasibleMatches.end(); ++matchCandItr ) {
                const std::string dropTypeStr = ((*matchCandItr)->_dropType == UFBW_fixed::MasterMinionMatchCand::FIFO) ? "FIFO" : "FILO";
                std::cout << "\t\t\tmaster/minion/type  " << (*matchCandItr)->pMaster->_riderIndex << " / " << (*matchCandItr)->pMinion->_riderIndex << " / " << dropTypeStr << std::endl;
            }
        }
        
        bool isTopRequestMatched = false;               
        if( feasibleMatches.size() > 0 ) {
        
            // step 6: assign costs to each match candidate (& print candidates)
            assignWeightsForMatchCandidates(&feasibleMatches);             
            if( printDebugFiles ) {
                printCurrentBatchMatchCandidates(*pOutFile, pCurrRequest, &feasibleMatches);
            }
           
            // step 7: SOLVE OPTIMIZATION MODEL
            if( printToScreen ) {
                std::cout << "\tsolving optimization model... " << std::endl;
            }
            std::set<AssignedTrip*, AssignedTripIndexComp> matchingsForCurrBatch = solveMatchingOptimization(&feasibleMatches, &requestsInCurrBatch, printToScreen);        
            if( printToScreen ) {
                std::cout << "\t\tdone." << std::endl;
            }
            if( matchingsForCurrBatch.size() > 0 ) {                
                _assignedTrips.insert(matchingsForCurrBatch.begin(), matchingsForCurrBatch.end());  
            }

            if( printToScreen ) {
                std::cout << "\t" << matchingsForCurrBatch.size() << " matches have been found from optimization" << std::endl;
            }

            // step 8: remove matched masters & minions
            //std::set<const int> matchedRiderIndices = getAllIndicesAssociatedWithMatchedRiders(&matchingsForCurrBatch);
            std::multimap<const int, time_t> matchedRiderReqTimeMap = getAllIndicesAssociatedWithMatchedRiders(&matchingsForCurrBatch);
            int removedOpenTrips = removeMatchedOpenTrips(&matchedRiderReqTimeMap, &currOpenTrips);
            int removedFutureRequests = removeMatchedFutureRequests(&matchedRiderReqTimeMap, &requestPool, pCurrRequest);
                        
            //std::set<const int>::iterator topRequestItr = matchedRiderIndices.find(pCurrRequest->getRiderIndex());            
            //isTopRequestMatched = (topRequestItr != matchedRiderIndices.end());   
            std::multimap<const int, time_t>::iterator topRequestItr = matchedRiderReqTimeMap.find(pCurrRequest->getRiderIndex());
            isTopRequestMatched = (topRequestItr != matchedRiderReqTimeMap.end());
        } 
                                      
        // step 9: process the current request at top of deque
        
        //      step 9.1: the request was matched - remove from pool
        if( isTopRequestMatched ) {
            requestPool.erase(requestPool.begin());              
        }

        //      step 9.2: the request was not matched - pop from deque and add to unmatchedRequest pool
        else {
            unmatchedRequestsWithinWaitTime.insert(pCurrRequest);
            requestPool.pop_front(); // remove            
        }
                     
        if( requestPool.size() == 0 )
            break;
               
        _batchCounter++;
    }
    
    if( printToScreen ) {
        std::cout << "\n\nafter processing all " << _allRequests.size() << " there are " << _assignedTrips.size() << " assigned trips" << std::endl;
    }

    // convert any requests awaiting a match to an open trip
    for( std::set<Request*, ReqComp>::iterator waitingReqItr = unmatchedRequestsWithinWaitTime.begin(); waitingReqItr != unmatchedRequestsWithinWaitTime.end(); ++waitingReqItr ) {
        AssignedTrip * pAssignedTrip = convertWaitingRequestToAssignedTrip(*waitingReqItr);
        pAssignedTrip->setIndex(_assignedTrips.size());
        _assignedTrips.insert(pAssignedTrip);
    }
    
    // convert any remaining open trips to unmatched ones
    if( printToScreen ) {
        std::cout << "\tconverting final open trips to unmatched trips... " << std::endl;
    }
    std::set<OpenTrip*, EtaComp>::iterator openTripItr;
    for( openTripItr = currOpenTrips.begin(); openTripItr != currOpenTrips.end(); ++openTripItr ) {
        AssignedTrip * pAssignedTrip = ModelUtils::convertOpenTripToAssignedTrip(*openTripItr);
        pAssignedTrip->setIndex(_assignedTrips.size());
        _assignedTrips.insert(pAssignedTrip);               
    }
    if( printToScreen ) {
        std::cout << "\tdone." << std::endl;
        std::cout << "\n\nafter there are " << _assignedTrips.size() << " assigned trips" << std::endl;
    }

    // create Solution
    pSolution = new Solution(Solution::UFBW_fixedPickups, _startTime, _endTime, _allRequests.size(), _allDrivers->size(), _assignedTrips, _disqualifiedRequests);
    pSolution->buildSolutionMetrics();
    
    return true;    
}

std::set<OpenTrip*, EtaComp> UFBW_fixed::convertExpiredUnmatchedReqsToOpenTrips(std::set<Request*, ReqComp> * pUnmatchedRequests, const time_t currTime) {
   
    std::set<OpenTrip*, EtaComp> newOpenTrips;
     
   // std::cout << "\t\t\tlooping through " << pUnmatchedRequests->size() << " unmatched reqs... " << std::endl;
    for( std::set<Request*, ReqComp>::iterator reqItr = pUnmatchedRequests->begin(); reqItr != pUnmatchedRequests->end(); ) {
     //   std::cout << "\t\t\t\treq by " << (*reqItr)->getRiderIndex() << " at " << Utility::convertTimeTToString((*reqItr)->getReqTime()) << std::endl;
        const time_t reqTime = (*reqItr)->getReqTime();
        const time_t currentWaitInSec = currTime - reqTime;
               
       /* std::cout << "\t\t\t\tcurr wait time: " << currentWaitInSec << " sec " << std::endl;
        std::cout << "\t\t\t\tmax wait time: " << _maxWaitTimeInSec << std::endl; */
        if( (int) currentWaitInSec > _lenBatchWindowInSec ) {
            
            //std::cout << "\t\t\t\t\texpired! processing... " << std::endl;
            
            const Event * pRequestEvent = new Event((*reqItr)->getReqTime(), (*reqItr)->getPickupLat(), (*reqItr)->getPickupLng());
                        
            OpenTrip * pOpenTrip = new OpenTrip((*reqItr)->getActualDriver()->getId(), (*reqItr)->getActualDriver(), (*reqItr)->getActualDriver()->getIndex(), 
                    (*reqItr)->getRiderID(), (*reqItr)->getRiderIndex(), (*reqItr)->getRiderTripUUID(), pRequestEvent,
                    (*reqItr)->getActualDispatchEvent(), (*reqItr)->getPickupLat(), (*reqItr)->getPickupLng(), (*reqItr)->getDropoffLat(), 
                    (*reqItr)->getDropoffLng(), (*reqItr)->getActTimeOfPickupFromTripActuals(), (*reqItr)->getActTimeOfDropoffFromTripActuals(),
                    (*reqItr)->getActualPickupEvent(), (*reqItr)->getActualDropEvent());
            
            newOpenTrips.insert(pOpenTrip);
            
            pUnmatchedRequests->erase(reqItr++);            
        } else {
            reqItr++;
        }
    }
    
    return newOpenTrips;
}
std::pair<std::set<UFBW_fixed::MasterCand*, UFBW_fixed::MasterComp>, std::set<UFBW_fixed::MinionCand*, UFBW_fixed::MinionComp> > UFBW_fixed::generateCandidateMastersAndMinions(std::set<OpenTrip*,EtaComp>& openTrips, std::set<Request*,ReqComp>& currBatchRequests) {
   
    // instantiate first and second sets to be returned in pair
    std::pair<std::set<UFBW_fixed::MasterCand*, UFBW_fixed::MasterComp>, std::set<UFBW_fixed::MinionCand*, UFBW_fixed::MinionComp> > mmPair;
    std::set<UFBW_fixed::MasterCand*, UFBW_fixed::MasterComp> candMasters;
    std::set<UFBW_fixed::MinionCand*, UFBW_fixed::MinionComp> candMinions;
    
    // first generate master candidates from current open trips
    for( std::set<OpenTrip*, EtaComp>::iterator tripItr = openTrips.begin(); tripItr != openTrips.end(); ++tripItr ) {
        LatLng reqOrig((*tripItr)->getActPickupLat(), (*tripItr)->getActPickupLng());
        LatLng reqDest((*tripItr)->getDropRequestLat(), (*tripItr)->getDropRequestLng());
        
        //const Event * pickupEvent = new Event((*tripItr)->getETA(), (*tripItr)->getActPickupLat(), (*tripItr)->getActPickupLng());
        //const Event * dropEvent = new Event((*tripItr)->getETD(), (*tripItr)->getDropRequestLat(), (*tripItr)->getDropRequestLng());
        
        UFBW_fixed::MasterCand * pMasterCand = new UFBW_fixed::MasterCand((*tripItr)->getMasterRequestEvent(), (*tripItr)->getMasterRequestEvent()->timeT, reqOrig, reqDest, (*tripItr)->getETA(), (*tripItr)->getETD(), (*tripItr)->getDriver(), (*tripItr)->getMasterDispatcEvent(), (*tripItr)->getMasterActualPickupEvent(), (*tripItr)->getMasterActualDropEvent(), (*tripItr)->getMasterIndex(), (*tripItr)->getMasterID(), (*tripItr)->getRiderTripUUID());
        candMasters.insert(pMasterCand);
    }
    
    // second generate master and minion candidates from batch
    for( std::set<Request*, ReqComp>::iterator reqItr = currBatchRequests.begin(); reqItr != currBatchRequests.end(); ++reqItr ) {
        LatLng reqOrig((*reqItr)->getPickupLat(), (*reqItr)->getPickupLng());
        LatLng reqDest((*reqItr)->getDropoffLat(), (*reqItr)->getDropoffLng());
        
       // const Event * pickupEvent = new Event((*reqItr)->getActTimeOfPickupFromTripActuals(), (*reqItr)->getPickupLat(), (*reqItr)->getPickupLng());
       // const Event * dropEvent = new Event((*reqItr)->getActTimeOfDropoffFromTripActuals(), (*reqItr)->getDropoffLat(), (*reqItr)->getDropoffLng());
        
        // MASTER candidate (note: since dispatch has not occurred by this stage a NULL dispatch event is passed in)        
            // JP; commenting the following line out 
        UFBW_fixed::MasterCand * pMasterCand = new UFBW_fixed::MasterCand((*reqItr)->getActualRequestEvent(),(*reqItr)->getReqTime(), reqOrig, reqDest, (*reqItr)->getActTimeOfPickupFromTripActuals(), (*reqItr)->getActTimeOfDropoffFromTripActuals(), (*reqItr)->getActualDriver(), NULL, (*reqItr)->getActualPickupEvent(), (*reqItr)->getActualDropEvent(), (*reqItr)->getRiderIndex(), (*reqItr)->getRiderID(), (*reqItr)->getRiderTripUUID());        
        candMasters.insert(pMasterCand);
        
        // MINION candidate
        UFBW_fixed::MinionCand * pMinionCand = new UFBW_fixed::MinionCand((*reqItr)->getReqTime(), reqOrig, reqDest, (*reqItr)->getRiderIndex(), (*reqItr)->getRiderID(), (*reqItr)->getRiderTripUUID());
        candMinions.insert(pMinionCand);
    }    
        
    mmPair.first  = candMasters;
    mmPair.second = candMinions;
    
    return mmPair;
}
AssignedTrip * UFBW_fixed::convertWaitingRequestToAssignedTrip(Request * pWaitingRequest) {
    AssignedTrip * pAssignedTrip = new AssignedTrip(pWaitingRequest->getActualDriver(), pWaitingRequest->getActualDispatchEvent(), pWaitingRequest->getRiderTripUUID(), pWaitingRequest->getActualRequestEvent(), pWaitingRequest->getActualPickupEvent(), pWaitingRequest->getActualDropEvent());
    
    // define request and dispatch events
    //Event reqEvent(pWaitingRequest->getReqTime(), pWaitingRequest->getPickupLat(), pWaitingRequest->getPickupLng());
    //Event dispatchEvent(pWaitingRequest->getActualDispatchEvent()->timeT, pWaitingRequest->getActualDispatchEvent()->lat, pWaitingRequest->getActualDispatchEvent()->lng);
    
    pAssignedTrip->setMasterId(pWaitingRequest->getRiderID());
    pAssignedTrip->setMasterIndex(pWaitingRequest->getRiderIndex());
   // pAssignedTrip->setMasterRequest(reqEvent);
   // pAssignedTrip->setMasterDispatch(dispatchEvent);
    
  //  Event actPickup(pWaitingRequest->getActTimeOfPickupFromTripActuals(),pWaitingRequest->getPickupLat(),pWaitingRequest->getPickupLng());
  //  Event actDropoff(pWaitingRequest->getActTimeOfDropoffFromTripActuals(),pWaitingRequest->getDropoffLat(),pWaitingRequest->getDropoffLng());
    
   // pAssignedTrip->setMasterPickupFromActuals(actPickup);   
   // pAssignedTrip->setMasterDropFromActuals(actDropoff); 
       
    return pAssignedTrip;
}

// METHODS TO DETERMINE ALL REQUESTS WITHIN A GIVEN BATCH WINDOW
std::set<Request*, ReqComp> UFBW_fixed::getRequestsInInterval(std::deque<Request*>& requestsToProcess, const time_t& currBatchStartTime, const time_t& currBatchEndTime) {
    std::set<Request*, ReqComp> reqsInCurrentBatch;
    
    std::deque<Request*>::iterator reqItr;
    for( reqItr = requestsToProcess.begin(); reqItr != requestsToProcess.end(); ++reqItr ) {
        const time_t currReqTime = (*reqItr)->getReqTime();
        if( currReqTime > currBatchEndTime )
            break;
        
        reqsInCurrentBatch.insert(*reqItr);
    }
    
    return reqsInCurrentBatch;
}

// METHODS TO FIND ALL FEASIBLE MATCH COMBINATIONS
std::set<UFBW_fixed::MasterMinionMatchCand*, UFBW_fixed::MasterMinionMatchComp> UFBW_fixed::generateFeasibleMasterMinionMatches(std::set<UFBW_fixed::MasterCand*, UFBW_fixed::MasterComp> &candMasters, std::set<UFBW_fixed::MinionCand*, UFBW_fixed::MinionComp> &candMinions) {
    std::set<UFBW_fixed::MasterMinionMatchCand*, UFBW_fixed::MasterMinionMatchComp> matchCandidates;
    
    std::set<UFBW_fixed::MasterCand*, UFBW_fixed::MasterComp>::iterator masterItr;
            
    for( masterItr = candMasters.begin(); masterItr != candMasters.end(); ++masterItr ) {
                               
        // loop over all candidate minions
        for( std::set<UFBW_fixed::MinionCand*, UFBW_fixed::MinionComp>::iterator minionItr = candMinions.begin(); minionItr != candMinions.end(); ++minionItr ) {
            
            // ensure the master trip has not been completed
            if( (*masterItr)->_ETD <= (*minionItr)->_reqTime ) 
                continue;
            
            // ignore matching the same rider
            if( (*masterItr)->_riderIndex == (*minionItr)->_riderIndex )
                continue;
            
          //  std::cout << "\n\ncandidate master: " << (*masterItr)->_riderIndex << " req at " << Utility::convertTimeTToString((*masterItr)->_reqTime) << std::endl;
          //  std::cout << "candidate minion: " << (*minionItr)->_riderIndex << " req at " << Utility::convertTimeTToString((*minionItr)->_reqTime) << std::endl;
                                               
            // check if DISTANCE qualifies (check separate instances depending upon if the master has been dispatched)
            double pickupDistToMinionAtTimeOfReq = 0.0;
            if( (*masterItr)->pDispatchEvent == NULL ) {
                pickupDistToMinionAtTimeOfReq = Utility::computeGreatCircleDistance((*masterItr)->_reqOrig.getLat(), (*masterItr)->_reqOrig.getLng(), (*minionItr)->_reqOrig.getLat(), (*minionItr)->_reqOrig.getLng());
            } else {
               // std::cout << "\tcomputing pickup distance between master/minion pair " << (*masterItr)->_riderIndex << "/" << (*minionItr)->_riderIndex << std::endl;
               // std::cout << "\tmaster dispatched at " << Utility::convertTimeTToString((*masterItr)->pDispatchEvent->timeT) << std::endl;
                
                // if the minion request occurs after master dispatch
                if( (*minionItr)->_reqTime >= (*masterItr)->pDispatchEvent->timeT ) {
                    pickupDistToMinionAtTimeOfReq = ModelUtils::getPickupDistanceAtTimeOfMinionRequest(
                        (*minionItr)->_reqTime, (*minionItr)->_reqOrig.getLat(), (*minionItr)->_reqOrig.getLng(), 
                        (*masterItr)->_ETA, (*masterItr)->_reqOrig.getLat(), (*masterItr)->_reqOrig.getLng(), 
                        (*masterItr)->_ETD, (*masterItr)->_reqDest.getLat(), (*masterItr)->_reqDest.getLng(),
                        (*masterItr)->pDispatchEvent->timeT, (*masterItr)->pDispatchEvent->lat, 
                        (*masterItr)->pDispatchEvent->lng);                    
                }
                
                // if the minion request occurs before master dispatch
                else {
                    pickupDistToMinionAtTimeOfReq = Utility::computeGreatCircleDistance((*masterItr)->_reqOrig.getLat(), (*masterItr)->_reqOrig.getLng(), (*minionItr)->_reqOrig.getLat(), (*minionItr)->_reqOrig.getLng());
                }
            }
                        
            if( pickupDistToMinionAtTimeOfReq <= _maxMatchDistInKm ) {   
                
                //const double haversineDistFromMasterOrigToMinionOrig = Utility::computeGreatCircleDistance((*masterItr)->_reqOrig.getLat(), (*masterItr)->_reqOrig.getLng(), (*minionItr)->_reqOrig.getLat(), (*minionItr)->_reqOrig.getLng());
                const double pickupDistanceToMinion = ModelUtils::computePickupDistance((*masterItr)->_ETA, (*masterItr)->_reqOrig.getLat(), (*masterItr)->_reqOrig.getLng(), (*masterItr)->_ETD, (*masterItr)->_reqDest.getLat(), (*masterItr)->_reqDest.getLng(), (*minionItr)->_reqTime, (*minionItr)->_reqOrig.getLat(), (*minionItr)->_reqOrig.getLng()); // TODO: fix this!
                double uberX_dist_master = Utility::computeGreatCircleDistance((*masterItr)->_reqOrig.getLat(), (*masterItr)->_reqOrig.getLng(), (*masterItr)->_reqDest.getLat(), (*masterItr)->_reqDest.getLng());
                double uberX_dist_minion = Utility::computeGreatCircleDistance((*minionItr)->_reqOrig.getLat(), (*minionItr)->_reqOrig.getLng(), (*minionItr)->_reqDest.getLat(), (*minionItr)->_reqDest.getLng());
                                
                // check if FIFO match is feasible                           
                FeasibleMatch * pFIFOMatch = ModelUtils::checkIfOverlapIsFeasWithforFIFOMatch(_minOverlapThreshold, (*minionItr)->_riderID, pickupDistanceToMinion , uberX_dist_master, uberX_dist_minion, *minionItr, *masterItr  );                                
                if( pFIFOMatch != NULL ) {  
                    const int matchIndex = matchCandidates.size();
                    UFBW_fixed::MasterMinionMatchCand * pFIFOMatchCandidate = new UFBW_fixed::MasterMinionMatchCand(matchIndex, *masterItr, *minionItr, pickupDistanceToMinion, UFBW_fixed::MasterMinionMatchCand::FIFO, pFIFOMatch->_avgSavings, pFIFOMatch->_masterPickedUpAtTimeOfMatch, pFIFOMatch);
                    matchCandidates.insert(pFIFOMatchCandidate);
                }
                
                // check if FILO match is feasible
                FeasibleMatch * pFILOMatch = ModelUtils::checkIfOverlapIsFeasWithforFILOMatch(_minOverlapThreshold, pickupDistanceToMinion, uberX_dist_master, uberX_dist_minion, *minionItr, *masterItr);
                if( pFILOMatch != NULL ) {
                    const int matchIndex = matchCandidates.size();
                    UFBW_fixed::MasterMinionMatchCand * pFILOMatchCandidate = new UFBW_fixed::MasterMinionMatchCand(matchIndex, *masterItr, *minionItr, pickupDistanceToMinion, UFBW_fixed::MasterMinionMatchCand::FILO, pFILOMatch->_avgSavings, pFILOMatch->_masterPickedUpAtTimeOfMatch, pFILOMatch);
                    matchCandidates.insert(pFILOMatchCandidate);  
                }                                
            }            
        }
    }


    return matchCandidates;
}
void UFBW_fixed::assignWeightsForMatchCandidates(std::set<MasterMinionMatchCand*,MasterMinionMatchComp>* pCandidateMatches) {
    
    // loop through match candidates and assign weights to each 
    std::set<UFBW_fixed::MasterMinionMatchCand*, UFBW_fixed::MasterMinionMatchComp>::iterator matchItr;
    for( matchItr = pCandidateMatches->begin(); matchItr != pCandidateMatches->end(); ++matchItr ) {
        double edgeWeight = computeEdgeWeightOfCurrCandidateMatch(*matchItr);
        (*matchItr)->_matchWeight = edgeWeight;
    }
}
double UFBW_fixed::computeEdgeWeightOfCurrCandidateMatch(MasterMinionMatchCand * pCurrMatchCand) {
    return (pCurrMatchCand->_matchWeight = pCurrMatchCand->_avgSavings);
}

// -------------------------------------------------
//
//      METHODS TO SOLVE MATCHING OPTIMIZATION
//
// -------------------------------------------------
std::set<AssignedTrip*, AssignedTripIndexComp> UFBW_fixed::solveMatchingOptimization(
        std::set<UFBW_fixed::MasterMinionMatchCand*, UFBW_fixed::MasterMinionMatchComp> * pEligMatches, 
        std::set<Request*, ReqComp> * pBatchRequests,
        bool printToScreen) {
    
    std::set<AssignedTrip*, AssignedTripIndexComp> currBatchMatches;
    
    // instantiate MPSolver object
    std::string batchStr = "UFBW_fixed_itr_" + Utility::intToStr(_batchCounter); 
    MPSolver solver(batchStr.c_str(), MPSolver::CBC_MIXED_INTEGER_PROGRAMMING);
    
    // instantiate mapping between rider indices and various constraints
    std::map<const int, MPConstraint*> riderIndexLeftDegreeConstraint;
    std::map<const int, MPConstraint*> riderIndexRightDegreeConstraint;
    std::map<const int, MPConstraint*> riderIndexAggregationConstraint;
    
    // get the set of riders who may be master OR minions
    instantiateConstraints(&solver, pEligMatches, pBatchRequests, &riderIndexLeftDegreeConstraint, &riderIndexRightDegreeConstraint, &riderIndexAggregationConstraint);
   
    // build variables
    if( printToScreen ) {
        std::cout << "\t\tbuilding model... " << std::endl;
    }
    std::map<MPVariable*,MasterMinionMatchCand*> edgeVariables = buildModelVariables(&solver,pEligMatches, &riderIndexLeftDegreeConstraint, &riderIndexRightDegreeConstraint, &riderIndexAggregationConstraint);
    
    if( printToScreen ) {
        std::cout << "\t\t\tdone." << std::endl;      
        std::cout << "\n\tINVOKING SOLVER... " << std::endl;
    }
    
    
    // solve
    solver.SuppressOutput();
    //solver.EnableOutput();

    const MPSolver::ResultStatus status = solver.Solve();
    
    std::vector<UFBW_fixed::MasterMinionMatchCand*> * pOptMatchings = NULL;
    
    switch( status ) {
        case MPSolver::OPTIMAL :
        {
            if( printToScreen ) {
                std::cout << "\t\tHOORAY! SOLUTION IS OPTIMAL" << std::endl;
            }
            const double optObjVal = solver.Objective().Value();
            pOptMatchings = getOptimalMatchings(&solver, &edgeVariables, printToScreen);
            break;
        }
        case MPSolver::INFEASIBLE : 
            std::cout << "\nSHIT... problem is INFEASIBLE" << std::endl;
            break;
        case MPSolver::UNBOUNDED : 
            std::cout << "\nOH SNAP... problem is UNBOUNDED" << std::endl;
            break;
        default:
            std::cout << "\nSOMETHING ELSE... " << std::endl;
    }
        
    if( pOptMatchings != NULL ) {
        currBatchMatches = buildAssignedTripsFromMatchingSolution(pOptMatchings);
    }
    
    
   // _batchCounter++; // counter to track the number of batches solved
    
    return currBatchMatches;
}
std::map<MPVariable*, UFBW_fixed::MasterMinionMatchCand*> UFBW_fixed::buildModelVariables(
        MPSolver * pSolver, std::set<UFBW_fixed::MasterMinionMatchCand*, 
        UFBW_fixed::MasterMinionMatchComp>* pEligMatches, 
        std::map<const int, MPConstraint*> * pLeftNodeConstrMap, 
        std::map<const int, MPConstraint*> * pRightNodeConstrMap, 
        std::map<const int, MPConstraint*> * pAggregationConstrMap) {
   
    MPObjective * const pObjective = pSolver->MutableObjective();
    pObjective->SetMaximization();
    
    std::map<MPVariable*, MasterMinionMatchCand*> modelVariables;
    
    std::set<UFBW_fixed::MasterMinionMatchCand*, UFBW_fixed::MasterMinionMatchComp>::iterator matchItr;
    for( matchItr = pEligMatches->begin(); matchItr != pEligMatches->end(); ++matchItr ) {
        
        const int masterIndex = (*matchItr)->pMaster->_riderIndex;
        const int minionIndex = (*matchItr)->pMinion->_riderIndex;
       
        // DEFINE VARIABLE
        std::string dropTypeStr = ((*matchItr)->_dropType == UFBW_fixed::MasterMinionMatchCand::FIFO) ? "FIFO" : "FILO";
        std::string varName = Utility::intToStr(masterIndex) + "-" + Utility::intToStr(minionIndex) + "-" + dropTypeStr;
        MPVariable * const currVar = pSolver->MakeIntVar(0,1,varName.c_str());
        
        modelVariables.insert(make_pair(currVar,*matchItr));  
        
        // ADD VARIABLE TO OBJECTIVE 
        pObjective->SetCoefficient(currVar, (*matchItr)->_matchWeight);
        //pObjective->SetCoefficient(currVar, 1.0);
        
        // ADD VARIABLE MASTER TO DEGREE CONSTRAINT
        std::map<const int, MPConstraint*>::iterator leftNodeConstrItr = pLeftNodeConstrMap->find(masterIndex);
        assert( leftNodeConstrItr != pLeftNodeConstrMap->end() );
        leftNodeConstrItr->second->SetCoefficient(currVar, 1.0);
        
        // ADD VARIABLE MINION TO DEGREE CONSTRAINT
        std::map<const int, MPConstraint*>::iterator rightNodeConstrItr = pRightNodeConstrMap->find(minionIndex);
        assert( rightNodeConstrItr != pRightNodeConstrMap->end() );
        rightNodeConstrItr->second->SetCoefficient(currVar, 1.0);
        
        // ADD VARIABLE TO MASTER AGGREGATION CONSTRAINT (for riders that may serve as either master or minion)
        std::map<const int, MPConstraint*>::iterator masterAggConstrItr = pAggregationConstrMap->find(masterIndex);
        if( masterAggConstrItr != pAggregationConstrMap->end() ) {
            masterAggConstrItr->second->SetCoefficient(currVar, 1.0);
        }
        
        // ADD VARIABLE TO MINION AGGREGATION CONSTRAINT (for riders that may serve as either master or minion)
        std::map<const int, MPConstraint*>::iterator minionAggConstrItr = pAggregationConstrMap->find(minionIndex);
        if( minionAggConstrItr != pAggregationConstrMap->end() ) {
            minionAggConstrItr->second->SetCoefficient(currVar, 1.0);
        }
                
    }
        
    
    return modelVariables;
}
void UFBW_fixed::instantiateConstraints(
        MPSolver* pSolver, 
        std::set<UFBW_fixed::MasterMinionMatchCand*,UFBW_fixed::MasterMinionMatchComp>* pEligMatches, 
        std::set<Request*, ReqComp> * pBatchRequests, 
        std::map<const int, MPConstraint*> * pRiderIxLeftDegreeConstrMap, 
        std::map<const int, MPConstraint*> * pRiderIxRightDegreeConstrMap, 
        std::map<const int, MPConstraint*> * pRiderIxAggregationConstrMap) {
    
    // instantiate degree constraints
    instantiateDegreeConstraints(pSolver, pEligMatches, pRiderIxLeftDegreeConstrMap, pRiderIxRightDegreeConstrMap);
    
    // instantiate aggregation degree constraints (for riders who may be master or minion)
    instantiateAggregationConstraints(pSolver, pEligMatches, pBatchRequests, pRiderIxAggregationConstrMap);
}
void UFBW_fixed::instantiateDegreeConstraints(MPSolver* pSolver, std::set<UFBW_fixed::MasterMinionMatchCand*,UFBW_fixed::MasterMinionMatchComp>* pEligMatches, std::map<const int, MPConstraint*> * pRiderIxLeftDegreeConstrMap, std::map<const int, MPConstraint*> * pRiderIxRightDegreeConstrMap) {
    
    // keep track of all master and minion nodes added
    std::set<const int> masterIndicesProcessed;
    std::set<const int> minionIndicesProcessed;
    
    std::set<UFBW_fixed::MasterMinionMatchCand*, UFBW_fixed::MasterMinionMatchComp>::iterator matchItr;
    for( matchItr = pEligMatches->begin(); matchItr != pEligMatches->end(); ++matchItr ) {
        
        // add master (if not already added)
        const int masterIndex = (*matchItr)->pMaster->_riderIndex;
        std::set<const int>::iterator masterItr = masterIndicesProcessed.find(masterIndex);
        bool masterConstrExists = (masterItr != masterIndicesProcessed.end());
        if( masterConstrExists == false ) {
            std::string masterDegreeConstrName = "master_degree_rider_" + Utility::intToStr(masterIndex);
            MPConstraint * pMasterConstraint = pSolver->MakeRowConstraint(0,1,masterDegreeConstrName);
            
            pRiderIxLeftDegreeConstrMap->insert(make_pair(masterIndex, pMasterConstraint));
            
            masterIndicesProcessed.insert(masterIndex);
        }
        
        // add minion (if not already added) 
        const int minionIndex = (*matchItr)->pMinion->_riderIndex;
        std::set<const int>::iterator minionItr = minionIndicesProcessed.find(minionIndex);
        bool minionConstrExists = (minionItr != minionIndicesProcessed.end());
        if( minionConstrExists == false ) {
            std::string minionDegreeConstrName = "minion_degree_rider_" + Utility::intToStr(minionIndex);
            MPConstraint * pMinionConstraint = pSolver->MakeRowConstraint(0,1,minionDegreeConstrName);
            
            pRiderIxRightDegreeConstrMap->insert(make_pair(minionIndex, pMinionConstraint));
            
            minionIndicesProcessed.insert(minionIndex);
        }                
    }    
}
void UFBW_fixed::instantiateAggregationConstraints(MPSolver * pSolver, std::set<UFBW_fixed::MasterMinionMatchCand*,UFBW_fixed::MasterMinionMatchComp>* pEligMatches, std::set<Request*, ReqComp> * pBatchRequests, std::map<const int, MPConstraint*> * pRiderIxAggregationConstrMap) {
    
    std::set<int> riderIndicesWithCopies = getRidersThanMayBeMastersOrMinions(pEligMatches, pBatchRequests);
    if( riderIndicesWithCopies.size() > 0 ) {
        for( std::set<int>::iterator indexItr = riderIndicesWithCopies.begin(); indexItr != riderIndicesWithCopies.end(); ++indexItr ) {
                std::string constrName = "aggregation_rider_" + Utility::intToStr(*indexItr);
                MPConstraint * constraint = pSolver->MakeRowConstraint(0,1,constrName.c_str());
                
                
                pRiderIxAggregationConstrMap->insert(make_pair(*indexItr, constraint));
        }        
    }    
}
std::set<int> UFBW_fixed::getRidersThanMayBeMastersOrMinions(std::set<UFBW_fixed::MasterMinionMatchCand*, UFBW_fixed::MasterMinionMatchComp> * pEligMatches, std::set<Request*, ReqComp> * pBatchRequests) {
    
    std::set<int> masterAndMinionRiderIndices;
    
    std::set<UFBW_fixed::MasterMinionMatchCand*, UFBW_fixed::MasterMinionMatchComp>::iterator matchItr;
    for( matchItr = pEligMatches->begin(); matchItr != pEligMatches->end(); ++matchItr ) {
        // extract left node (candidate master)
        const UFBW_fixed::MasterCand * pMaster =  (*matchItr)->pMaster;
       
        // check if there exists a right node
        Request * pRequest = getMinionRequest(pMaster->_riderIndex, pBatchRequests);
        if( pRequest != NULL ) {
            masterAndMinionRiderIndices.insert(pMaster->_riderIndex);            
        }
    } 
    
    return masterAndMinionRiderIndices;
}
MPConstraint * UFBW_fixed::getLeftNodeConstraint(MPSolver* pSolver, const UFBW_fixed::MasterCand* pMaster) {
    
    std::string leftNodeConstrName = "left_" + Utility::intToStr(pMaster->_riderIndex);
    
    std::vector<MPConstraint*>::const_iterator constrItr;
    const std::vector<MPConstraint*> pCurrConstraints = pSolver->constraints();
    for( constrItr = pCurrConstraints.begin(); constrItr != pCurrConstraints.end(); ++constrItr ) {
        if( leftNodeConstrName == (*constrItr)->name() ) 
            return *constrItr;
    }

    return NULL;
}
MPConstraint * UFBW_fixed::getRightNodeConstraint(MPSolver* pSolver, const UFBW_fixed::MinionCand* pMinion) {
    std::string leftNodeConstrName = "right_" + Utility::intToStr(pMinion->_riderIndex);
    
    std::vector<MPConstraint*>::const_iterator constrItr;
    const std::vector<MPConstraint*> pCurrConstraints = pSolver->constraints();
    for( constrItr = pCurrConstraints.begin(); constrItr != pCurrConstraints.end(); ++constrItr ) {
        if( leftNodeConstrName == (*constrItr)->name() ) 
            return *constrItr;
    }

    return NULL;
}
std::vector<MPVariable*> UFBW_fixed::getIncidentEdgeVariablesForMaster(const UFBW_fixed::MasterCand * pMaster, std::map<UFBW_fixed::MasterMinionMatchCand*,MPVariable*>* pMatchVarMap) {

    // instantiate vector to be returned
    std::vector<MPVariable*> incidentEdgeVariables;
    
    std::map<UFBW_fixed::MasterMinionMatchCand*, MPVariable*>::iterator mapItr;
    for( mapItr = pMatchVarMap->begin(); mapItr != pMatchVarMap->end(); ++mapItr ) {                
        if( pMaster->_riderIndex == mapItr->first->pMaster->_riderIndex ) {
            incidentEdgeVariables.push_back(mapItr->second);
        }        
    }
            
    return incidentEdgeVariables;    
}
std::vector<UFBW_fixed::MasterMinionMatchCand*> * UFBW_fixed::getOptimalMatchings(MPSolver* pSolver, std::map<MPVariable*,UFBW_fixed::MasterMinionMatchCand*>* pEdgeVariables, bool printToScreen) {
   
    // std::map<const MasterCand*, const MinionCand*> * pOptMatchingMap = new std::map<const MasterCand*, const MinionCand*>();
    std::vector<UFBW_fixed::MasterMinionMatchCand*> * pOptMatchings = new std::vector<UFBW_fixed::MasterMinionMatchCand*>();
    
    std::vector<MPVariable*>::const_iterator varItr; 
    for( varItr = pSolver->variables().begin(); varItr != pSolver->variables().end(); ++varItr ) {
        const double val = (*varItr)->solution_value();
        if( val > 0.01 ){
            if( printToScreen ) {
                std::cout << "var " << (*varItr)->name() << ":  " << (*varItr)->solution_value() << std::endl;   
            }
            std::map<MPVariable*, UFBW_fixed::MasterMinionMatchCand*>::iterator varMatchItr = pEdgeVariables->find(*varItr);
            if( varMatchItr != pEdgeVariables->end() ) {            
                pOptMatchings->push_back(varMatchItr->second);//
            } else {
                std::cout << "ERROR: VARIABLE " << (*varItr)->name() << " NOT FOUND IN MAPPING" << std::endl;
                exit(0);
            }
        }
    }
    
    
    return pOptMatchings;
}
std::set<AssignedTrip*, AssignedTripIndexComp> UFBW_fixed::buildAssignedTripsFromMatchingSolution(std::vector<UFBW_fixed::MasterMinionMatchCand*>* pOptMatchings) {
    std::set<AssignedTrip*, AssignedTripIndexComp> assignedTripsFromMatchingSoln;
    
    int assignedTripCounter = _assignedTrips.size();
    
    std::vector<UFBW_fixed::MasterMinionMatchCand*>::iterator matchItr;
    for( matchItr = pOptMatchings->begin(); matchItr != pOptMatchings->end(); ++matchItr ) {
        
        const UFBW_fixed::MasterCand * pMaster = (*matchItr)->pMaster;
        const UFBW_fixed::MinionCand * pMinion = (*matchItr)->pMinion;
        
      //  Event masterReqEvent(pMaster->_reqTime, pMaster->_reqOrig.getLat(), pMaster->_reqOrig.getLng());
      //  Event minionReqEvent(pMinion->_reqTime, pMinion->_reqOrig.getLat(), pMinion->_reqOrig.getLng());
      //  Event masterPickFromActuals(pMaster->pPickupEvent->timeT, pMaster->pPickupEvent->lat, pMaster->pPickupEvent->lng);
      //  Event masterDropFromActuals(pMaster->pDropEvent->timeT, pMaster->pDropEvent->lat, pMaster->pDropEvent->lng);
        
        // build AssignedTrip* object
            // 
        AssignedTrip * pAssignedTrip = new AssignedTrip(pMaster->pDriver, pMaster->pDispatchEvent, pMaster->_riderTripUUID, pMaster->pReqEvent, pMaster->pPickupEvent, pMaster->pDropEvent);
        pAssignedTrip->setIndex(assignedTripCounter);
        pAssignedTrip->setMasterId(pMaster->_riderID);
        pAssignedTrip->setMasterIndex(pMaster->_riderIndex);
        //pAssignedTrip->setMasterRequest(masterReqEvent);
        //pAssignedTrip->setMasterDispatch();
        //pAssignedTrip->setMasterDropFromActuals(masterPickFromActuals);
        //pAssignedTrip->setMasterPickupFromActuals(masterDropFromActuals);
        pAssignedTrip->setMinionId(pMinion->_riderID);
        pAssignedTrip->setMinionIndex(pMinion->_riderIndex);
        pAssignedTrip->setMinionTripUUID(pMinion->_riderTripUUID);
        //pAssignedTrip->setMinionDispatch();
       // pAssignedTrip->setMinionRequest(minionReqEvent);
                
        // set FeasibleMatch object which stores information about match metrics
        pAssignedTrip->setMatch((*matchItr)->pFeasMatch);  
        
        assignedTripsFromMatchingSoln.insert(pAssignedTrip);
        
        assignedTripCounter++;
    }

    return assignedTripsFromMatchingSoln;
}
MPConstraint * UFBW_fixed::getRiderCopyAggregationConstraint(MPSolver * pSolver, const int masterIndex) {
    std::string constrName = "masterMinion_card_rider_" + Utility::intToStr(masterIndex);
    
    const std::vector<MPConstraint*> constraints = pSolver->constraints();
    for( std::vector<MPConstraint*>::const_iterator constrItr = constraints.begin(); constrItr != constraints.end(); ++constrItr ) {
        if( (*constrItr)->name() == constrName ) {
            return (*constrItr);
        }
    }
    
    return NULL;
}

// METHODS TO UPDATE DYNAMIC STRUCTURES
int UFBW_fixed::removeMatchedOpenTrips(std::multimap<const int, time_t> * pMatchedRiderReqTimeMap, std::set<OpenTrip*, EtaComp> * pOpenTrips) {
    
    int removedOpenTrips = 0;
    
    std::set<OpenTrip*, EtaComp>::iterator openTripItr;
    for( openTripItr = pOpenTrips->begin(); openTripItr != pOpenTrips->end(); ) {
        const int masterIndex = (*openTripItr)->getMasterIndex();
        std::multimap<const int, time_t>::iterator matchingRiderIndexReqTimeItr = pMatchedRiderReqTimeMap->find(masterIndex); // = pMatchedRiderIndices->find(masterIndex);
        if( matchingRiderIndexReqTimeItr != pMatchedRiderReqTimeMap->end() ) {
            pOpenTrips->erase(openTripItr++);     
            removedOpenTrips++;            
            continue;
        } else {
            openTripItr++;
        }  
    }
    
    return removedOpenTrips;    
}
int UFBW_fixed::removeMatchedFutureRequests(std::multimap<const int, time_t>* pMatchedRiderReqTimeMap, std::deque<Request*> * pFutureRequests, Request * pCurrRequestInQueue) {
    
    int removedFutureRequests = 0;
    
    //std::cout << "\n\nremoving matched future requests... " << std::endl;
        
    std::deque<Request*>::iterator reqItr;
    for( reqItr = pFutureRequests->begin(); reqItr != pFutureRequests->end(); ) {
        const int riderIndex = (*reqItr)->getRiderIndex();
        std::multimap<const int, time_t>::iterator matchingIndexReqTimeItr = pMatchedRiderReqTimeMap->find(riderIndex);
        if( matchingIndexReqTimeItr != pMatchedRiderReqTimeMap->end() ) {
            if( pCurrRequestInQueue->getRiderIndex() != (*reqItr)->getRiderIndex() ) { // ignore the top request in the deque since it will 'pop' from the front later
                if( matchingIndexReqTimeItr->second == (*reqItr)->getReqTime() ) {
                    pFutureRequests->erase(reqItr);
                    removedFutureRequests++;
                    continue;
                } else {
                    reqItr++;
                }
            } else {
                reqItr++;
            }
        } else {
            reqItr++;
        }
    }    
    return removedFutureRequests;    
}
std::multimap<const int, time_t> UFBW_fixed::getAllIndicesAssociatedWithMatchedRiders(std::set<AssignedTrip*, AssignedTripIndexComp> * pAssignedTrips) {

    std::multimap<const int, time_t> indexTimeMapOfMatchedRiders;
    
    std::set<AssignedTrip*, AssignedTripIndexComp>::iterator tripItr;
    for( tripItr = pAssignedTrips->begin(); tripItr != pAssignedTrips->end(); ++tripItr ) {
        indexTimeMapOfMatchedRiders.insert(make_pair((*tripItr)->getMasterIndex(), (*tripItr)->getMasterRequestEvent()->timeT));
        indexTimeMapOfMatchedRiders.insert(make_pair((*tripItr)->getMinionIndex(), (*tripItr)->getMatchDetails()->_minionRequest));
    }
    
    //return indicesMatchedRiders;
    return indexTimeMapOfMatchedRiders;
}

// METHODS
std::queue<Request*> UFBW_fixed::cloneRequests(std::set<Request*, ReqComp> requests) {
    std::queue<Request*> requestQueue;
    
    for( std::set<Request*, ReqComp>::iterator reqItr = requests.begin(); reqItr != requests.end(); ++reqItr ) {
        requestQueue.push(*reqItr);
    }
    
    return requestQueue;
}
std::set<OpenTrip*, EtaComp> UFBW_fixed::cloneOpenTrips(std::set<OpenTrip*, EtaComp> openTrips) {
    std::set<OpenTrip*, EtaComp> openTripClones;
    for( std::set<OpenTrip*, EtaComp>::iterator itr = openTrips.begin(); itr != openTrips.end(); ++itr ) {
        //if( (_startTime <= (*itr)->getETD()) && ((*itr)->getETD() <= _endTime) ) {
            
            openTripClones.insert(*itr);
            
        //}
    }        
    return openTripClones;
}
Request*  UFBW_fixed::getMinionRequest(const int riderIndex, std::set<Request*,ReqComp>* pRequests) {
    
    for( std::set<Request*,ReqComp>::iterator reqItr = pRequests->begin(); reqItr != pRequests->end(); ++reqItr ) {
        if( (*reqItr)->getRiderIndex() == riderIndex )
            return (*reqItr);
    }
    
    return NULL;
}

// IO STUFF
void UFBW_fixed::initEligMatchFile(std::ofstream &outFile) {
    outFile << "\n----------------------------------------\n" << std::endl;
    outFile << "    Up Front Batch Window Candidates" << std::endl;
    outFile << "        (fixed pickup sequence)" << std::endl;
    outFile << "\n----------------------------------------\n\n" << std::endl;    
}
void UFBW_fixed::printCurrentBatchMatchCandidates(std::ofstream& outFile, Request* pRequest, std::set<UFBW_fixed::MasterMinionMatchCand*, UFBW_fixed::MasterMinionMatchComp>* pEligMatches) {
    LatLng reqOrig(pRequest->getPickupLat(),pRequest->getPickupLng());
    LatLng reqDest(pRequest->getDropoffLat(),pRequest->getDropoffLng());
    
    outFile << "current request: " << Utility::convertTimeTToString(pRequest->getReqTime()) << std::endl;
    outFile << "request origin:  " << Utility::convertToLatLngStr(reqOrig, 5) << std::endl;
    outFile << "request dest:    " << Utility::convertToLatLngStr(reqDest, 5) << std::endl;
    outFile << "\n";

    int ixBuff = 8;
    int riderBuff = 12;
    int savBuff = 15;
    int dropBuff = 10;
    int extBuff = 14;
    int locBuff = 28;
    int timeBuff = 32;
    
    outFile << "\t" << left << setw(ixBuff) << "index" << 
            left << setw(riderBuff) << "masterIx" << 
            left << setw(riderBuff) << "minionIx" << 
            left << setw(savBuff) << "avgSavings" << 
            left << setw(extBuff) << "isExtended?" << 
            left << setw(dropBuff) << "drop" << 
            left << setw(timeBuff) << "masterReqTime" << 
            left << setw(locBuff) << "masterOrigin" << 
            left << setw(locBuff) << "masterDest" << 
            left << setw(timeBuff) << "minionReqTime" << 
            left << setw(locBuff) << "minionOrigin" <<
            left << setw(locBuff) << "minionDest" << std::endl;
    
    
    std::set<UFBW_fixed::MasterMinionMatchCand*, UFBW_fixed::MasterMinionMatchComp>::iterator itr;
    for( itr = pEligMatches->begin(); itr != pEligMatches->end(); ++itr ) {
        
        std::string dropType = ((*itr)->_dropType == UFBW_fixed::MasterMinionMatchCand::FIFO) ? "FIFO" : "FILO";
        std::string isExt = ((*itr)->_isExtended) ? "yes" : "no";
        std::string avgSavingsStr = Utility::truncateDouble((*itr)->_avgSavings, 4);
        
        outFile << "\t" << left << setw(ixBuff) << Utility::intToStr((*itr)->_matchIndex) << 
                left << setw(riderBuff) << Utility::intToStr((*itr)->pMaster->_riderIndex) << 
                left << setw(riderBuff) << Utility::intToStr((*itr)->pMinion->_riderIndex) << 
                left << setw(savBuff) << avgSavingsStr << 
                left << setw(extBuff) << isExt << 
                left << setw(dropBuff) << dropType << 
                left << setw(timeBuff) << Utility::convertTimeTToString((*itr)->pMaster->_reqTime) <<
                left << setw(locBuff) << Utility::convertToLatLngStr((*itr)->pMaster->_reqOrig, 5) << 
                left << setw(locBuff) << Utility::convertToLatLngStr((*itr)->pMaster->_reqDest, 5) <<
                left << setw(timeBuff) << Utility::convertTimeTToString((*itr)->pMinion->_reqTime) <<
                left << setw(locBuff) << Utility::convertToLatLngStr((*itr)->pMinion->_reqOrig, 5) << 
                left << setw(locBuff) << Utility::convertToLatLngStr((*itr)->pMinion->_reqDest, 5) << std::endl;
    }
    
    outFile << "\n" << std::endl;
}
