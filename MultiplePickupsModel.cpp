/* 
 * File:   MitmModel.cpp
 * Author: jonpetersen
 * 
 * Created on March 24, 2015, 5:40 PM
 */

#include "MultiplePickupsModel.hpp"

MultiplePickupsModel::MultiplePickupsModel(const time_t startTime, const time_t endTime, const double maxMatchDistKm, const double minOverlapThreshold, 
        std::set<Request*, ReqComp> initRequests, std::set<OpenTrip*, EtdComp> initOpenTrips, const std::set<Driver*, DriverIndexComp> * drivers,  bool inclInitPickupInSavings, int maxAllowablePickups) 
        : _startTime(startTime), _endTime(endTime), _maxMatchDistInKm(maxMatchDistKm), _minOverlapThreshold(minOverlapThreshold), _allDrivers(drivers), _inclMinionPickupDistExtMatchesSavingsConstr(inclInitPickupInSavings), _maxAllowablePickups(maxAllowablePickups) {
    _allRequests = initRequests;
    _initOpenTrips = initOpenTrips;
    pSolution = NULL;
    
    _numRoutesCreated = 0;
}


MultiplePickupsModel::~MultiplePickupsModel() {
}

/*
 *  SOLVE n PICKUPS BY RECURSIVELY INVOKING MITM MODEL
 */
bool MultiplePickupsModel::solve(bool printDebugFiles, Output * pOutput, bool populateInitOpenTrips) {
    
    std::cout << "\n\n--------------------------------------------------\n" << std::endl;
    std::cout << "         SOLVING MULTIPLE PICKUPS MODEL\n" << std::endl;
    std::cout << "--------------------------------------------------\n\n" << std::endl;
        
    // step 1: build sets to be updated
    std::set<Request*,  ReqComp> requests = cloneRequests();  
    std::set<Route*, RouteEndComp> openRoutes;
    
    std::ofstream * pOutFile;
    
    // initialize output file that tracks match candidates
    if( printDebugFiles ) {
        const std::string outputScenPath = pOutput->getOutputScenarioPath();
        const std::string filename = "MultiplePickups-match-candidates.txt";
        std::string outPath = outputScenPath + filename;
        pOutFile = new std::ofstream(outPath); 
        initCandidateMatchFile(*pOutFile);
    }
    
    // step 2: loop through all requests
    std::set<Request*, ReqComp>::iterator reqItr;  
    for( reqItr = requests.begin(); reqItr != requests.end(); ++reqItr ) {
        time_t currReqTime = (*reqItr)->getReqTime();
        
        // TODO: delete
         //std::cout << "\n** processing request " << (*reqItr)->getRiderIndex() << " **" << std::endl;
         //cout << "\trequest at " << Utility::convertTimeTToString(currReqTime) << endl;
                               
        int numCompletedRoutes = ModelUtils::assignCompletedOpenRoutes(currReqTime,openRoutes,_assignedRoutes);
        
        // 2.B: search for best open trip to match  
        Route * pMatchedRoute = getBestRouteForRequestInsertion(*reqItr,&openRoutes,printDebugFiles,pOutFile); 
               
        // step 3: process the match                     
        //     3.A: IF there is a match, then assign the Route if the max allowable pickups are achieved
        if( pMatchedRoute != NULL ) {
            assert( pMatchedRoute->getRequests()->size() > 1 );
            
            if( pMatchedRoute->getNumMatchedRiders() == _maxAllowablePickups ) {
                removeRouteFromAssignedRoutes(pMatchedRoute,&openRoutes);   
                AssignedRoute * pAssignedRoute = new AssignedRoute(pMatchedRoute);
                _assignedRoutes.insert(pAssignedRoute);              
            } else {           
                appendToOpenRoute(&openRoutes,pMatchedRoute);
            }
        }                
        //     3.B: IF no match, then assign new open trip
        else {      
            Route * pOpenRoute = createNewRouteWithUnmatchedRider((*reqItr)->getActualDriver(), *reqItr, (*reqItr)->getActualDispatchEvent());    
            openRoutes.insert(pOpenRoute);
        }
        
       // std::cout << "there are " << openRoutes.size() << " open routes" << std::endl;
    }
               
    // assign any unmatched (open) trips
    if( openRoutes.size() > 0 ) {
        clearRemainingOpenRoutes(openRoutes, &_assignedRoutes);
        assert( openRoutes.empty() );        
    } 
   
    std::cout << "\n\n\n--- finished processing all requests ---\n\n" << std::endl;
   /* std::cout << "\n\n_assignedRoutes.size() = " << _assignedRoutes.size() << std::endl;
    std::cout << "let's see how many riders are in each route... " << std::endl;
    for( std::set<AssignedRoute*, AssignedRouteIndexComp>::iterator it = _assignedRoutes.begin(); it != _assignedRoutes.end(); ++it ) {
        std::cout << "\troute " << (*it)->getIndex() << ":  " << (*it)->getNumRidersInRoute() << std::endl;
    } */
        
    if( printDebugFiles ) {
        pOutFile->close();
    }
    
    int numMatchedTrips = 0;
    std::map<int,int> numTripsMap;
    for( std::set<AssignedRoute*, AssignedRouteIndexComp>::iterator it = _assignedRoutes.begin(); it != _assignedRoutes.end(); ++it ) {
        std::map<int,int>::iterator keyValItr = numTripsMap.find((*it)->getNumRidersInRoute());
        if( keyValItr != numTripsMap.end() ) {
            keyValItr->second++;
        } else {
            numTripsMap.insert(make_pair((*it)->getNumRidersInRoute(),1));
        }
    }
    std::cout << "\n\nhere is the distribution of trips... " << std::endl;
    std::cout << "\n\t" << left << setw(12) << "numRiders" << left << setw(10) << "count" << std::endl;
    int totalTrips = 0;
    int totalMatches = 0;
    for( std::map<int,int>::iterator solnIt = numTripsMap.begin(); solnIt != numTripsMap.end(); ++solnIt ) {
        std::cout << "\t" << left << setw(12) << Utility::intToStr(solnIt->first) << left << setw(10) << Utility::intToStr(solnIt->second) << endl;
        if( solnIt->first >= 2 ) {
            totalMatches += solnIt->first*solnIt->second;
        }
        totalTrips += solnIt->first*solnIt->second;
    }
    const double matchRate = (double)100*((double)totalMatches/(double)totalTrips);
    std::cout << "\nmatch rate: " << Utility::truncateDouble(matchRate,4) << endl;
    
    
    
    // BUILD SOLUTION
    buildSolution(_assignedRoutes); 
       
    return true;
}

void MultiplePickupsModel::appendToOpenRoute(std::set<Route*,RouteEndComp>* pExistingRoutes, Route* pAppendedRoute) {
    for( std::set<Route*, RouteEndComp>::iterator routeItr = pExistingRoutes->begin(); routeItr != pExistingRoutes->end(); ++routeItr ) {
        if( (*routeItr)->getRouteIndex() == pAppendedRoute->getRouteIndex() ) {
            (*routeItr)->setPickupEvents(*(pAppendedRoute->getPickupEvents()));
            (*routeItr)->setDropoffEvents(*(pAppendedRoute->getDropoffEvents()));
            (*routeItr)->setRequests(*(pAppendedRoute->getRequests()));
        }
    }
}

std::set<Request*, ReqComp> MultiplePickupsModel::cloneRequests() {
    std::set<Request*, ReqComp> requests;
    std::set<Request*, ReqComp>::iterator reqItr;
    for( reqItr = _allRequests.begin(); reqItr != _allRequests.end(); ++reqItr ) {
        requests.insert(*reqItr);
    }
    
    return requests;
}

std::set<AssignedTrip*, AssignedTripIndexComp> MultiplePickupsModel::getAllCompletedOpenTrips(time_t &tm, std::set<OpenTrip*,EtdComp> &openTrips) {
    std::set<AssignedTrip*, AssignedTripIndexComp> unmatchedTrips;
    
    // iterate over open trips and check for completion
    std::set<OpenTrip*, EtdComp>::iterator openTripItr;
    std::set<OpenTrip*, EtdComp>::iterator lastDeletionItr;
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
void MultiplePickupsModel::clearRemainingOpenRoutes(std::set<Route*, RouteEndComp> &openRoutes, std::set<AssignedRoute*, AssignedRouteIndexComp> * pAssignedRoutes) {
    for( std::set<Route*, RouteEndComp>::iterator routeItr = openRoutes.begin(); routeItr != openRoutes.end(); ++routeItr ) {
        AssignedRoute * pAssignedRoute = new AssignedRoute(*routeItr);             
        pAssignedRoutes->insert(pAssignedRoute); 
    }
    openRoutes.erase(openRoutes.begin(),openRoutes.end());
}

/*
 *  given minion request and current open trips (candidate masters), get best master to match with minion 
 *     if no feasible matches exist, return NULL
 */
Route * MultiplePickupsModel::getBestRouteForRequestInsertion(Request * pMinionRequest, std::set<Route*, RouteEndComp>  * pExistingRoutes, bool printDebugFiles, std::ofstream * pOutFile) {
           
    // if there are no master candidate then a match is not possible
    if( pExistingRoutes->size() == 0 )
        return NULL;
            
    // search for every candidate master
    std::multimap<const double, Route*> candidateRouteInsertionMap; // key: objective, value: master with which may be matched to minion
    std::set<Route*, RouteEndComp>::iterator routeItr;
    
    if( printDebugFiles ) {
        *pOutFile << "\nMINION REQUEST:  " << Utility::convertTimeTToString(pMinionRequest->getReqTime()) << std::endl;
        *pOutFile << "RIDER INDEX:  " << Utility::intToStr(pMinionRequest->getRiderIndex()) << std::endl;
        *pOutFile << "ORIG:  (" << std::setprecision(15) << pMinionRequest->getPickupLat() << "," << std::setprecision(15) << pMinionRequest->getPickupLng() << ")" << std::endl;
        *pOutFile << "DEST:  (" << std::setprecision(15) << pMinionRequest->getDropoffLat() << "," << std::setprecision(15) << pMinionRequest->getDropoffLng() << ")" << std::endl;      
    }
    
    for( routeItr = pExistingRoutes->begin(); routeItr != pExistingRoutes->end(); ++routeItr ) {
               
        // step 0: ensure minion index is never contained in an existing route
        bool isMinionAssigned = false;
        const std::vector<Request*> * pRequests = (*routeItr)->getRequests();
        for( std::vector<Request*>::const_iterator iReqInRt = pRequests->begin(); iReqInRt != pRequests->end(); ++iReqInRt ) {
            if( pMinionRequest->getRiderIndex() == (*iReqInRt)->getRiderIndex() ) {
                isMinionAssigned = true;
                break;
            }
        }
        if( isMinionAssigned ) 
            continue;
        
        // step 1: ensure that the candidate route has been dispatched by time of minion request
        if( (*routeItr)->getDispatchTime() >= pMinionRequest->getReqTime() ) 
            continue;
                
        // step 3: ensure that the minion request is before the route end time
        if( pMinionRequest->getReqTime() >= (*routeItr)->getRouteEndTime() ) {
            continue;
        }
                               
        // step 4: get feasible matches depending upon dropoff sequence (same master,minion pair)    
        std::vector<Route*> feasibleRoutes = getFeasibleRoutesToInsertMinionReq(pMinionRequest, *routeItr); 
        if( feasibleRoutes.size() > 0 ) { 
           // cout << "-> " << feasibleRoutes.size() << " feasible routes have been identified" << endl;          
            for( std::vector<Route*>::iterator iRoute = feasibleRoutes.begin(); iRoute != feasibleRoutes.end(); ++iRoute ) {
              //  std::cout << "feas route has " << (*iRoute)->getRequests()->size() << " requests" << endl;                
                const double currCost = computeCostOfRoute(*iRoute);
                candidateRouteInsertionMap.insert(make_pair(currCost,*iRoute)); 
            }
        }
    }

    // if no feasible candidate found return NULL to indicate no match
    if( candidateRouteInsertionMap.empty() )
        return NULL;
    else {
        if( printDebugFiles ) {
            printRouteCandidatesForCurrRequest(&candidateRouteInsertionMap, pOutFile);
        }
        
        return candidateRouteInsertionMap.begin()->second;
    } 
}

std::vector<Route*> MultiplePickupsModel::getFeasibleRoutesToInsertMinionReq(Request * pMinionReq, Route * pExistingRoute) {
    std::vector<Route*> feasRoutesForMinionInsertion;

    // check each combination of # riders
    const int numRidersInExistingRoute = pExistingRoute->getNumMatchedRiders();
    assert( numRidersInExistingRoute < _maxAllowablePickups );
    
    // get all matches that have a single rider present
    std::vector<Route*> feasRoutesWithSingleRider;
    if( numRidersInExistingRoute == 1 ) {
        // JPJP
        //cout << "\tprocessing single rider match for minion req " << pMinionReq->getRiderIndex() << endl;
        //feasRoutesWithSingleRider = getFeasibleRoutesForSingleExistingRiderInRoute(pMinionReq, pExistingRoute);
        feasRoutesWithSingleRider = getFeasibleRoutesForSingleExistingRiderInRoute_deprecated(pMinionReq, pExistingRoute);      
    }

    std::vector<Route*> feasRoutesWithMultipleExistingRiders;
    if( numRidersInExistingRoute >= 2 ) {
        feasRoutesWithMultipleExistingRiders = getFeasibleRoutesWithTwoExistingRiders(pMinionReq, pExistingRoute);
    }
    
    // append routes with a single rider present
    if( feasRoutesWithSingleRider.size() > 0 ) {
        feasRoutesForMinionInsertion.insert(feasRoutesForMinionInsertion.end(), feasRoutesWithSingleRider.begin(), feasRoutesWithSingleRider.end());
    }
    
    // append routes with multiple existing riders present
    if( feasRoutesWithMultipleExistingRiders.size() > 0 ) {
        feasRoutesForMinionInsertion.insert(feasRoutesForMinionInsertion.end(), feasRoutesWithMultipleExistingRiders.begin(), feasRoutesWithMultipleExistingRiders.end());
    }
       
    return feasRoutesForMinionInsertion;
} 
std::vector<Route*> MultiplePickupsModel::getFeasibleRoutesForSingleExistingRiderInRoute(Request* pMinionReq, Route * pExistingRoute) {
    std::vector<Route*> feasTwoRiderRoutes;

    // check 1: pickup distance is feasible (if there is only one existing rider the current request is the candidate minion)
    const Request * pMasterRequest = pExistingRoute->getPickupRequest(1);
         
    const double pickupDistanceAtTimeOfMinionRequest = ModelUtils::getPickupDistanceAtTimeOfMinionRequest_maxPickupConstr(
            pMinionReq->getReqTime(), pMinionReq->getPickupLat(), pMinionReq->getPickupLng(), 
            pMasterRequest->getActualPickupEvent()->timeT, pMasterRequest->getActualPickupEvent()->lat, pMasterRequest->getActualPickupEvent()->lng, 
            pMasterRequest->getActualDropEvent()->timeT, pMasterRequest->getActualDropEvent()->lat, pMasterRequest->getActualDropEvent()->lng, 
            pMasterRequest->getActualDispatchEvent()->timeT, pMasterRequest->getActualDispatchEvent()->lat, pMasterRequest->getActualDispatchEvent()->lng);
    
    if( pickupDistanceAtTimeOfMinionRequest <= _maxMatchDistInKm ) {
                
        // check FIFO
        Route * pRoute_FIFO = constructInsertedRoute(pExistingRoute,pMinionReq,2,2);
        bool isFIFOFeas = checkIfCandRouteMeetsSavings(pRoute_FIFO);
        if( isFIFOFeas ) {
            feasTwoRiderRoutes.push_back(pRoute_FIFO);
        }
        
        // check FILO
        Route * pRoute_FILO = constructInsertedRoute(pExistingRoute,pMinionReq,2,1);
        bool isFILOFeas = checkIfCandRouteMeetsSavings(pRoute_FILO);
        if( isFILOFeas ) {
            feasTwoRiderRoutes.push_back(pRoute_FILO);
        }        
    }    

    return feasTwoRiderRoutes;
}
std::vector<Route*> MultiplePickupsModel::getFeasibleRoutesForSingleExistingRiderInRoute_deprecated(Request * pMinionReq, Route * pExistingRoute) {
    std::vector<Route*> feasRoutesForTwoRiders;
       
    // check 1: pickup distance is feasible (if there is only one existing rider the current request is the candidate minion)
    const Request * pMasterRequest = pExistingRoute->getPickupRequest(1);
     
    const double pickupDistanceAtTimeOfMinionRequest = ModelUtils::getPickupDistanceAtTimeOfMinionRequest_maxPickupConstr(
            pMinionReq->getReqTime(), pMinionReq->getPickupLat(), pMinionReq->getPickupLng(), 
            pMasterRequest->getActualPickupEvent()->timeT, pMasterRequest->getActualPickupEvent()->lat, pMasterRequest->getActualPickupEvent()->lng, 
            pMasterRequest->getActualDropEvent()->timeT, pMasterRequest->getActualDropEvent()->lat, pMasterRequest->getActualDropEvent()->lng, 
            pMasterRequest->getActualDispatchEvent()->timeT, pMasterRequest->getActualDispatchEvent()->lat, pMasterRequest->getActualDispatchEvent()->lng);
        
    if( pickupDistanceAtTimeOfMinionRequest <= _maxMatchDistInKm ) {
        
        // check 2: trip overlap exceeds threshold
        const double pickupDistanceToMinion = ModelUtils::computePickupDistance_savingsConstr(pMasterRequest->getActualPickupEvent()->timeT, pMasterRequest->getActualPickupEvent()->lat, pMasterRequest->getActualPickupEvent()->lng, pMasterRequest->getActualDropEvent()->timeT, pMasterRequest->getActualDropEvent()->lat, pMasterRequest->getActualDropEvent()->lng, pMinionReq->getReqTime(), pMinionReq->getPickupLat(), pMinionReq->getPickupLng(), _inclMinionPickupDistExtMatchesSavingsConstr);
        const double uberXdistanceForMaster = Utility::computeGreatCircleDistance(pMasterRequest->getActualPickupEvent()->lat, pMasterRequest->getActualPickupEvent()->lng, pMasterRequest->getActualDropEvent()->lat, pMasterRequest->getActualDropEvent()->lng);
        const double uberXdistanceForMinion = Utility::computeGreatCircleDistance(pMinionReq->getPickupLat(), pMinionReq->getPickupLng(), pMinionReq->getDropoffLat(), pMinionReq->getDropoffLng());
        
        // check if FIFO route is feasible
        Route * pFeasRoute_FIFO = checkIfFIFORouteIsFeasible(pMinionReq->getRiderID(), pickupDistanceToMinion, uberXdistanceForMaster, uberXdistanceForMinion, pMinionReq, pExistingRoute);
        if( pFeasRoute_FIFO != NULL ) {
            feasRoutesForTwoRiders.push_back(pFeasRoute_FIFO);
        }
                
        // check if FILO route is feasible  
        Route * pFeasRoute_FILO = checkIfFILORouteIsFeasible(pickupDistanceToMinion, uberXdistanceForMaster, uberXdistanceForMinion, pMinionReq, pExistingRoute);
        if( pFeasRoute_FILO != NULL ) {
            feasRoutesForTwoRiders.push_back(pFeasRoute_FILO);
        }        
    }
    
    return feasRoutesForTwoRiders;
}
std::vector<Route*> MultiplePickupsModel::getFeasibleRoutesWithTwoExistingRiders(Request * pMinionReq, Route * pExistingRoute) {
    std::vector<Route*> feasRoutesForThreeRiders;
    
    assert(pExistingRoute->getPickupEvents()->size() == 2);
    
    // first get all routes where rider may be picked up second
    const time_t currReqTime   = pMinionReq->getReqTime();
    RouteEvent * pFirstPickup  = pExistingRoute->getPickupEvents()->at(0);
    RouteEvent * pSecondPickup = pExistingRoute->getPickupEvents()->at(1);
    RouteEvent * pFirstDrop    = pExistingRoute->getDropoffEvents()->front();
    
    // get all feasible routes with new request inserted as FIRST pickup
    const time_t firstPickupTime = pFirstPickup->getEventTime();
    if( firstPickupTime > currReqTime ) {
        std::vector<Route*> candRoutesWithFirstPickup = getFeasibleRoutesWithRiderPickedUpFirst(pMinionReq, pExistingRoute);
        if( candRoutesWithFirstPickup.size() > 0 ) {
            feasRoutesForThreeRiders.insert(feasRoutesForThreeRiders.end(), candRoutesWithFirstPickup.begin(), candRoutesWithFirstPickup.end());
        }
    }
    
    // get all feasible routes with new request inserted as SECOND pickup
    const time_t secondPickupTime = pSecondPickup->getEventTime();   
    if( secondPickupTime > currReqTime ) {
        std::vector<Route*> candRoutesWithSecondPickup = getFeasilbeRoutesWithRiderPickedUpSecond(pMinionReq, pExistingRoute);
        if( candRoutesWithSecondPickup.size() > 0 ) {
            feasRoutesForThreeRiders.insert(feasRoutesForThreeRiders.end(), candRoutesWithSecondPickup.begin(), candRoutesWithSecondPickup.end());
        }
    }
        
    // get all feasible routes with new request inserted as THIRD pickup   
    if( pFirstDrop->getEventTime() > currReqTime ) {
        std::vector<Route*> candRoutesWithThirdPickup = getFeasibleRoutesWithRiderPickedUpThird(pMinionReq, pExistingRoute);
        if( candRoutesWithThirdPickup.size() > 0 ) {
            feasRoutesForThreeRiders.insert(feasRoutesForThreeRiders.end(), candRoutesWithThirdPickup.begin(), candRoutesWithThirdPickup.end());
        }
    }
                    
    return feasRoutesForThreeRiders;
}

// methods to construct candidate Route objects for TWO riders
Route * MultiplePickupsModel::checkIfFIFORouteIsFeasible(const std::string minionId, const double distToMinion, const double masterUberXDist, const double minionUberXDist, Request* pMinionReq, Route * pSingleRiderExistingRoute) {
    
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
    
    const Request * pMasterReq = pSingleRiderExistingRoute->getPickupRequest(1);
        
    // step 1: compute h: distance of shared leg with MASTER being dropped first (Haversine)
    const double minionPickupLat  = pMinionReq->getPickupLat();
    const double minionPickupLng  = pMinionReq->getPickupLng();
    const double masterDropoffLat = pMasterReq->getDropoffLat();
    const double masterDropoffLng = pMasterReq->getDropoffLng();
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
        //Route * pAugmentedRoute = buildCandidateRouteWithInsertion(pSingleRiderExistingRoute, pMinionReq, 2, 2);
        Route * pAugmentedRoute = constructInsertedRoute(pSingleRiderExistingRoute, pMinionReq, 2, 2);              
        return pAugmentedRoute;
    } else {
        return NULL;
    }
}
Route * MultiplePickupsModel::checkIfFILORouteIsFeasible(const double distToMinion, const double masterUberXDist, const double minionUberXDist, Request* pMinionReq, Route * pSingleRiderExistingRoute) {
    
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
    
    const Request * pMasterReq = pSingleRiderExistingRoute->getPickupRequest(1);
    
    // step 1: compute h: distance of shared leg with MINION being dropped first (Haversine) 
    const double minionPickupLat  = pMinionReq->getPickupLat();
    const double minionPickupLng  = pMinionReq->getPickupLng();
    const double minionDropoffLat = pMinionReq->getDropoffLat();
    const double minionDropoffLng = pMinionReq->getDropoffLng();
    const double sharedDistance   = Utility::computeGreatCircleDistance(minionPickupLat, minionPickupLng, minionDropoffLat, minionDropoffLng);
    
    // step 2: compute s2: distance from minion drop to master drop
    const double masterDropLat    = pMasterReq->getActualDropEvent()->lat; 
    const double masterDropLng    = pMasterReq->getActualDropEvent()->lng;
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
        //Route * pFeasRoute = buildCandidateRouteWithInsertion(pSingleRiderExistingRoute, pMinionReq, 2, 1);
        Route * pFeasRoute = constructInsertedRoute(pSingleRiderExistingRoute, pMinionReq, 2, 1);
        return pFeasRoute;
    } else {
        return NULL;
    }
}

// methods to construct candidate Route objects for THREE riders
std::vector<Route*> MultiplePickupsModel::getFeasibleRoutesWithRiderPickedUpFirst(Request * pMinionReq, Route * pExistingRoute) {
    std::vector<Route*> feasRoutesWithReqFirstPickup;
    
    // compute pickup distance from most recent pickup to current request
    const RouteEvent * pFirstPickupEvent  = pExistingRoute->getPickupEvents()->at(0);
    const RouteEvent * pSecondPickupEvent = pExistingRoute->getPickupEvents()->at(1);
        
    const double pickupDistanceAtTimeOfMinionRequest = ModelUtils::getPickupDistance_maxPickupConstr_route(pExistingRoute, pMinionReq, 1); 
    
    // TODO: delete
    if( pExistingRoute->getPickupEvents()->size() != 2 ) {
        cout << "\n\n** ERROR: " << pExistingRoute->getPickupEvents()->size() << " pickup events defined (rider picked up first) **\n" << endl;
        cout << "\tminion request:  rider " << pMinionReq->getRiderIndex() << " req at " << Utility::convertTimeTToString(pMinionReq->getReqTime()) << endl;
        cout << "\texisting route: " << pExistingRoute->getRouteIndex() << endl;
        pExistingRoute->print();
        exit(1);
    }      
    
    if( pickupDistanceAtTimeOfMinionRequest <= _maxAllowablePickups ) { 
     
        // first check: second pickup, first drop
        Route * pRoute_11 = constructInsertedRoute(pExistingRoute,pMinionReq,1,1);
        bool isRouteFeas_11 = checkIfCandRouteMeetsSavings(pRoute_11);
        if( isRouteFeas_11 ) {
            feasRoutesWithReqFirstPickup.push_back(pRoute_11);           
        }
        
        // second check: second drop
        Route * pRoute_12 = constructInsertedRoute(pExistingRoute,pMinionReq,1,2);
        bool isRouteFeas_12 = checkIfCandRouteMeetsSavings(pRoute_12);
        if( isRouteFeas_12 ) {
            feasRoutesWithReqFirstPickup.push_back(pRoute_12);
        }
        
        // third check: third drop
        Route * pRoute_13 = constructInsertedRoute(pExistingRoute,pMinionReq,1,3);
        bool isRouteFeas_13 = checkIfCandRouteMeetsSavings(pRoute_13);
        if( isRouteFeas_13 ) {
            feasRoutesWithReqFirstPickup.push_back(pRoute_13);
        }             
    }    

    return feasRoutesWithReqFirstPickup;
}
std::vector<Route*> MultiplePickupsModel::getFeasilbeRoutesWithRiderPickedUpSecond(Request * pMinionReq, Route * pExistingRoute) {
    std::vector<Route*> feasRoutesWithReqSecondPickup;
        
    // compute pickup distance from most recent pickup to current request
    const RouteEvent * pFirstPickupEvent  = pExistingRoute->getPickupEvents()->at(0);
    const RouteEvent * pSecondPickupEvent = pExistingRoute->getPickupEvents()->at(1);
            
    // TODO: delete
    if( pExistingRoute->getPickupEvents()->size() != 2 ) {
        cout << "\n\n** ERROR: " << pExistingRoute->getPickupEvents()->size() << " pickup events defined (rider picked up second) **\n" << endl;
        cout << "\tminion request:  rider " << pMinionReq->getRiderIndex() << " req at " << Utility::convertTimeTToString(pMinionReq->getReqTime()) << endl;
        cout << "\texisting route: " << pExistingRoute->getRouteIndex() << endl;
        pExistingRoute->print();
        exit(1);
    }    
    
    const double pickupDistanceAtTimeOfMinionRequest = ModelUtils::getPickupDistance_maxPickupConstr_route(pExistingRoute, pMinionReq, 2);
    
    if( pickupDistanceAtTimeOfMinionRequest <= _maxAllowablePickups ) { 
        
        // first check: second pickup, first drop
        Route * pRoute_21 = constructInsertedRoute(pExistingRoute,pMinionReq,2,1);
        bool isRouteFeas_21 = checkIfCandRouteMeetsSavings(pRoute_21);
        if( isRouteFeas_21 ) {
            feasRoutesWithReqSecondPickup.push_back(pRoute_21);
        }
        
        // second check: second drop
        Route * pRoute_22 = constructInsertedRoute(pExistingRoute,pMinionReq,2,2);
        bool isRouteFeas_22 = checkIfCandRouteMeetsSavings(pRoute_22);
        if( isRouteFeas_22 ) {
            feasRoutesWithReqSecondPickup.push_back(pRoute_22);
        }
        
        // third check: third drop
        Route * pRoute_23 = constructInsertedRoute(pExistingRoute,pMinionReq,2,3);
        bool isRouteFeas_23 = checkIfCandRouteMeetsSavings(pRoute_23);
        if( isRouteFeas_23 ) {
            feasRoutesWithReqSecondPickup.push_back(pRoute_23);
        }
                        
    }

    return feasRoutesWithReqSecondPickup;
}
std::vector<Route*> MultiplePickupsModel::getFeasibleRoutesWithRiderPickedUpThird(Request * pMinionReq, Route * pExistingRoute) {
    std::vector<Route*> feasRoutesWithReqThirdPickup;
    
    // compute pickup distance from most recent pickup to current request
    const RouteEvent * pFirstPickupEvent  = pExistingRoute->getPickupEvents()->at(0);
    const RouteEvent * pSecondPickupEvent = pExistingRoute->getPickupEvents()->at(1);
    
    // TODO: delete
    if( pExistingRoute->getPickupEvents()->size() != 2 ) {
        cout << "\n\n** ERROR: " << pExistingRoute->getPickupEvents()->size() << " pickup events defined (rider picked up third) **\n" << endl;
        cout << "\tminion request:  rider " << pMinionReq->getRiderIndex() << " req at " << Utility::convertTimeTToString(pMinionReq->getReqTime()) << endl;
        cout << "\texisting route: " << pExistingRoute->getRouteIndex() << endl;
        pExistingRoute->print();
        exit(1);
    }
       
    const double pickupDistanceAtTimeOfMinionRequest = ModelUtils::getPickupDistance_maxPickupConstr_route(pExistingRoute, pMinionReq, 3);
        
    if( pickupDistanceAtTimeOfMinionRequest <= _maxAllowablePickups ) { 
     
        // first check: second pickup, first drop
        Route * pRoute_31 = constructInsertedRoute(pExistingRoute,pMinionReq,3,1);
        bool isRouteFeas_31 = checkIfCandRouteMeetsSavings(pRoute_31);
        if( isRouteFeas_31 ) {
            feasRoutesWithReqThirdPickup.push_back(pRoute_31);
        }
        
        // second check: second drop
        Route * pRoute_32 = constructInsertedRoute(pExistingRoute,pMinionReq,3,2);
        bool isRouteFeas_32 = checkIfCandRouteMeetsSavings(pRoute_32);
        if( isRouteFeas_32 ) {
            feasRoutesWithReqThirdPickup.push_back(pRoute_32);
        }
        
        // third check: third drop
        Route * pRoute_33 = constructInsertedRoute(pExistingRoute,pMinionReq,3,3);
        bool isRouteFeas_33 = checkIfCandRouteMeetsSavings(pRoute_33);
        if( isRouteFeas_33 ) {
            feasRoutesWithReqThirdPickup.push_back(pRoute_33);
        }           
    }
    
    return feasRoutesWithReqThirdPickup;
}
Route * MultiplePickupsModel::constructInsertedRoute(Route* pOrigRoute, Request* reqToInsert, int pickupOrder, int dropoffOrder) {
    
    Route * pRouteCopy = new Route(pOrigRoute->getRouteIndex(), pOrigRoute->getDriver(), pOrigRoute->getDispatchEvent());
    
    // extract the current events and times associated with the adjacent RouteEvents 
    std::pair<RouteEvent*, time_t> priorEvent  = getRouteEventPreceedingInsertedEvent(pOrigRoute,pickupOrder);
    std::pair<RouteEvent*, time_t> subseqEvent = getRouteEventFollowingInsertedEvent(pOrigRoute,pickupOrder); 
    
    // re-create requests and pick & drop events
    for( std::vector<Request*>::const_iterator reqItr = pOrigRoute->getRequests()->begin(); reqItr != pOrigRoute->getRequests()->end(); ++reqItr) {
        pRouteCopy->addRequest(*reqItr);
    }
    for( std::vector<RouteEvent*>::iterator pickItr = pOrigRoute->getPickupEvents()->begin(); pickItr != pOrigRoute->getPickupEvents()->end(); ++pickItr ) {
        pRouteCopy->addPickupEvent((*pickItr)->getRequest(), (*pickItr)->getEventTime(), (*pickItr)->getLat(), (*pickItr)->getLng());
    }
    for( std::vector<RouteEvent*>::iterator dropItr = pOrigRoute->getDropoffEvents()->begin(); dropItr != pOrigRoute->getDropoffEvents()->end(); ++dropItr ) {
        pRouteCopy->addDropoffEvent((*dropItr)->getRequest(), (*dropItr)->getEventTime(), (*dropItr)->getLat(), (*dropItr)->getLng());
    }

    // append current request to route copy
    pRouteCopy->addRequest(reqToInsert);

    // insert the PICKUP event
    int pickupIndex = pickupOrder-1;
    std::vector<RouteEvent*>::iterator itr = pRouteCopy->getPickupEvents()->begin()+pickupIndex;
    LatLng pickupLoc(reqToInsert->getActualRequestEvent()->lat, reqToInsert->getActualRequestEvent()->lng);
    RouteEvent * pNewPickupEvent = new RouteEvent(reqToInsert, reqToInsert->getActualPickupEvent()->timeT, pickupLoc, RouteEvent::PICKUP);
    pRouteCopy->insertPickupEvent(pNewPickupEvent, pickupOrder);

    // insert the DROPOFF event
    int dropoffIndex = dropoffOrder-1;
    std::vector<RouteEvent*>::iterator dropItr = pRouteCopy->getDropoffEvents()->begin()+dropoffIndex;
    LatLng dropLoc(reqToInsert->getActualDropEvent()->lat, reqToInsert->getActualDropEvent()->lng);
    RouteEvent * pNewDropEvent = new RouteEvent(reqToInsert, reqToInsert->getActualDropEvent()->timeT, dropLoc, RouteEvent::DROPOFF);
    pRouteCopy->insertDropoffEvent(pNewDropEvent,dropoffOrder);

    // update times
    pRouteCopy->updateTimes();

    // compute the distance traveled to pick up the request 
    double distToInsertedPickup = 0.0;
    RouteEvent * priorPickupEvent = priorEvent.first;     // prior pickup before insertion
    if( priorPickupEvent != NULL ) {
        RouteEvent * nextEvent = subseqEvent.first; // subsequent pickup before insertion
        distToInsertedPickup = ModelUtils::computePickupDistance_savingsConstr(priorPickupEvent->getEventTime(), priorPickupEvent->getLat(), priorPickupEvent->getLng(),
                                                        nextEvent->getEventTime(), nextEvent->getLat(), nextEvent->getLng(), 
                                                        reqToInsert->getReqTime(), reqToInsert->getActualRequestEvent()->lat, reqToInsert->getActualRequestEvent()->lng,
                                                        _inclMinionPickupDistExtMatchesSavingsConstr);
    }

    // compute the MASTER metrics
    RiderMetrics * master_metrics = computeRiderMetrics(pRouteCopy,1,distToInsertedPickup,reqToInsert);
    pRouteCopy->setMasterMetrics(master_metrics);
 
    // compute the MINION metrics
    if( pRouteCopy->getNumMatchedRiders() >= 2 ) {
        RiderMetrics * minion_metrics = computeRiderMetrics(pRouteCopy,2,distToInsertedPickup,reqToInsert);
        pRouteCopy->setMinionMetrics(minion_metrics);
    }

    // compute the PARASITE metrics
    if( pRouteCopy->getNumMatchedRiders() >= 3 ) {
        RiderMetrics * parasite_metrics = computeRiderMetrics(pRouteCopy,3,distToInsertedPickup,reqToInsert);
        pRouteCopy->setParasiteMetrics(parasite_metrics);
    }
    
    Route::RouteMetrics * pRouteMetrics = computeRouteMetrics(pRouteCopy);
    pRouteCopy->setRouteMetrics(pRouteMetrics);
    
    return pRouteCopy;
}

std::pair<RouteEvent*, time_t> MultiplePickupsModel::getRouteEventPreceedingInsertedEvent(Route* pOrigRoute, int seqInsertedEvent) {
    std::pair<RouteEvent*, time_t> preceedingEventAndTimePair;
    assert(seqInsertedEvent >= 1);
    
    // only proceed if the event is NOT first (else, empty pair is returned and handled separately)
    if( seqInsertedEvent > 1) {
        int arrayIndexOfNewEvent = seqInsertedEvent-1; // e.g. if 2nd event, this is index 1
        RouteEvent * pPrecedingEvent = pOrigRoute->getPickupEvents()->at(arrayIndexOfNewEvent-1);
        time_t timeOfPrecedingEvent = pPrecedingEvent->getEventTime();
        preceedingEventAndTimePair = make_pair(pPrecedingEvent, timeOfPrecedingEvent);
    } else {
        preceedingEventAndTimePair.first = NULL;
        preceedingEventAndTimePair.second = 0;
    }
    
    return preceedingEventAndTimePair;
}
std::pair<RouteEvent*, time_t> MultiplePickupsModel::getRouteEventFollowingInsertedEvent(Route* pOrigRoute, int seqInsertedEvent) {
    std::pair<RouteEvent*, time_t> subseqEventAndTimePair;
   
    std::vector<RouteEvent*> * pickupEvents  = pOrigRoute->getPickupEvents();
    std::vector<RouteEvent*> * dropoffEvents = pOrigRoute->getDropoffEvents();
    
    bool isInsertedEventLast = (pickupEvents->size()+1 == seqInsertedEvent);
    
    // case 1: the preceding pickup was NOT the final pickup... so next event is the following pickup
    if( isInsertedEventLast == false ) {
        subseqEventAndTimePair = make_pair(pickupEvents->at(seqInsertedEvent-1), pickupEvents->at(seqInsertedEvent-1)->getEventTime());
    }
    
    // case 2: the preceding pickup route WAS the final pickup... so next event is first drop
    else {
        subseqEventAndTimePair = make_pair(dropoffEvents->front(),dropoffEvents->front()->getEventTime());
    }
    
    return subseqEventAndTimePair;
}

void MultiplePickupsModel::printRiderMetrics(const RiderMetrics * pRiderMetrics) {
    cout << "\t" << left << setw(15) << "rider: " << Utility::intToStr(pRiderMetrics->pRequest->getRiderIndex()) << endl;
    cout << "\t" << left << setw(15) << "dist_X: " << Utility::truncateDouble(pRiderMetrics->uberXDist,4) << endl;
    cout << "\t" << left << setw(15) << "dist_pool:" << Utility::truncateDouble(pRiderMetrics->pooledDist,4) << endl;
    cout << "\t" << left << setw(15) << "inconv:" << Utility::truncateDouble(pRiderMetrics->inconv,4) << endl;
    cout << "\t" << left << setw(15) << "cost: " << Utility::truncateDouble(pRiderMetrics->pooledCost,4) << endl;
    cout << "\t" << left << setw(15) << "savings: " << Utility::truncateDouble(pRiderMetrics->savings,4) << endl;
}
void MultiplePickupsModel::printRouteMetrics(const Route::RouteMetrics * pRouteMetrics) {
    cout << "\t" << left << setw(15) << "totalTripDist: " << Utility::truncateDouble(pRouteMetrics->_totalTripDist,4) << endl;
    cout << "\t" << left << setw(15) << "sharedDist: " << Utility::truncateDouble(pRouteMetrics->_sharedDist,4) << endl;
    cout << "\t" << left << setw(15) << "avgInconv: " << Utility::truncateDouble(pRouteMetrics->_avgRiderInconv,4) << endl;
    cout << "\t" << left << setw(15) << "avgSavings: " << Utility::truncateDouble(pRouteMetrics->_avgRiderSavings,4) << endl;
}

RiderMetrics * MultiplePickupsModel::computeRiderMetrics(Route* pRoute, int riderIndexPickup, double pickupDistToInsertedReq, Request * pInsertedReq) {
    assert( riderIndexPickup <= this->_maxAllowablePickups );
    assert( riderIndexPickup <= pRoute->getPickupEvents()->size() );
    
    int pickupIx = riderIndexPickup-1;    
    RouteEvent * pRiderPickup  = pRoute->getPickupEvents()->at(pickupIx);
    RouteEvent * pRiderDropoff = getDropEventInRouteForRider(pRiderPickup->getRequest()->getRiderIndex(), pRoute);
    
    // compute shared distance
    RouteEvent * pLastPickup = pRoute->getPickupEvents()->back();
    RouteEvent * pFirstDrop  = pRoute->getDropoffEvents()->front();    
    const double sharedDist = Utility::computeGreatCircleDistance(pLastPickup->getLat(), pLastPickup->getLng(), pFirstDrop->getLat(), pFirstDrop->getLng());
    
    // compute total trip distance AND trip cost together
    std::pair<const double, const double> tripDistCostPair = computeTripCostAndDistanceForRider(pRoute, pRiderPickup, pRiderDropoff, pInsertedReq->getRiderIndex(), sharedDist, pickupDistToInsertedReq);
    const double riderPoolDist = tripDistCostPair.first;
    const double riderPoolCost = tripDistCostPair.second;
    
    // compute savings
    const double uberXDist = Utility::computeGreatCircleDistance(pRiderPickup->getRequest()->getActualPickupEvent()->lat, pRiderPickup->getRequest()->getActualPickupEvent()->lng, pRiderPickup->getRequest()->getActualDropEvent()->lat, pRiderPickup->getRequest()->getActualDropEvent()->lng);
    const double excessDist = riderPoolDist - uberXDist;
    const double inconv = excessDist/uberXDist;
    //const double inconv = (double)100*(excessDist/uberXDist);
    const double costDiff = uberXDist - riderPoolCost;
    //const double savings = (double)100*(costDiff/uberXDist);
    const double savings = costDiff/uberXDist;
        
    RiderMetrics * pRiderMetrics = new RiderMetrics(pRiderPickup->getRequest(), uberXDist, riderPoolDist, riderPoolCost, savings, inconv);
    return pRiderMetrics;
}
std::pair<const double, const double> MultiplePickupsModel::computeTripCostAndDistanceForRider(Route * pRoute, RouteEvent * pRiderPickup, RouteEvent * pRiderDropoff, const int riderIndexInserted, double sharedDistance, double pickupDistToInsertedReq) {
    
    assert( pRiderPickup->getRequest()->getRiderIndex() == pRiderDropoff->getRequest()->getRiderIndex() );
    
    std::vector<RouteEvent*> * pickups  = pRoute->getPickupEvents();
    std::vector<RouteEvent*> * dropoffs = pRoute->getDropoffEvents();
    
    const double fracSharedDist = (double)1/(double)pRoute->getNumMatchedRiders();
    double totalTripCost = fracSharedDist*sharedDistance; // instantiate trip cost with the link shared by all riders
    
    // step 1: compute pickup distance (but only if the final pickup does not coincide with the current rider)
    double totalPickupDistance = 0;

    if( pickups->back()->getRequest()->getRiderIndex() != pRiderPickup->getRequest()->getRiderIndex() ) {
        bool isPartOfTrip = false;
        for( int ii = 0; ii < pickups->size()-1; ii++ ) {
            RouteEvent * currPickup = pickups->at(ii);
            if( !isPartOfTrip ) { isPartOfTrip = (currPickup->getRequest()->getRiderIndex() == pRiderPickup->getRequest()->getRiderIndex()); }
            if( isPartOfTrip ) {
                RouteEvent * nextPickup = pickups->at(ii+1);
                double currPickupDist = 0.0;  
                if( nextPickup->getRequest()->getRiderIndex() == riderIndexInserted ) {
                    currPickupDist += pickupDistToInsertedReq;
                } else {
                    currPickupDist += Utility::computeGreatCircleDistance(currPickup->getLat(), currPickup->getLng(), nextPickup->getLat(), nextPickup->getLng());
                }
                
                totalPickupDistance += currPickupDist;                
                double multiplier = (double)(2 - ii)/(double)2;
                double adjCost = multiplier*currPickupDist;
                totalTripCost += adjCost;                
            }
        }
    }
    
    
    // step 2: compute drop distance (but only if the initial drop does not coincide with the current rider)
    double totalDropoffDistance = 0.0;
    if( dropoffs->front()->getRequest()->getRiderIndex() != pRiderPickup->getRequest()->getRiderIndex() ) {
        for( int jj = 0; jj < dropoffs->size()-1; jj++ ) {
            RouteEvent * currDrop = dropoffs->at(jj);
            RouteEvent * nextDrop = dropoffs->at(jj+1);
            const double currDropoffDist = Utility::computeGreatCircleDistance(currDrop->getLat(), currDrop->getLng(), nextDrop->getLat(), nextDrop->getLng());
            totalDropoffDistance += currDropoffDist;
            
            double multiplier = (double)(jj+1)/(double)(pRoute->getNumMatchedRiders()-1);
            double adjCost = multiplier*currDropoffDist;
            totalTripCost += adjCost;
            
            if( nextDrop->getRequest()->getRiderIndex() == pRiderDropoff->getRequest()->getRiderIndex() ) {                                                
                break;
            }
        }
    } 
    
    // now sum the three trip components and return trip distance for rider
    const double totalTripDistForRider = totalPickupDistance + sharedDistance + totalDropoffDistance;
    const double totalTripCostForRider = totalTripCost;
                
    std::pair<const double, const double> costDistPair = make_pair(totalTripDistForRider, totalTripCostForRider);    
    return costDistPair;
}
Route::RouteMetrics * MultiplePickupsModel::computeRouteMetrics(Route * pRoute) {
        
    //    compute shared distance
    RouteEvent * lastPickup = pRoute->getPickupEvents()->back();
    RouteEvent * firstDrop  = pRoute->getDropoffEvents()->front();
    double sharedDist = Utility::computeGreatCircleDistance(lastPickup->getLat(), lastPickup->getLng(), firstDrop->getLat(), firstDrop->getLng());
        
    // define total trip distance
    const double totalTripDist = computeTotalTripDistance(pRoute, sharedDist);
    
    // compute average savings
    const RiderMetrics * pMasterMetrics   = pRoute->getMasterMetrics();
    const RiderMetrics * pMinionMetrics   = pRoute->getMinionMetrics();
    const RiderMetrics * pParasiteMetrics = pRoute->getParasiteMetrics();
    
    double totalInconv  = 0.0;
    double totalSavings = 0.0;
    if( pMasterMetrics != NULL ) {
        totalInconv  += pMasterMetrics->inconv;
        totalSavings += pMasterMetrics->savings;
    }
    
    if( pMinionMetrics != NULL ) {
        totalInconv  += pMinionMetrics->inconv;
        totalSavings += pMinionMetrics->savings;
    }
    
    if( pParasiteMetrics != NULL ) {
        totalInconv  += pParasiteMetrics->inconv;
        totalSavings += pParasiteMetrics->savings;
    }
    
    int numRiders = pRoute->getNumMatchedRiders();
    double avgInconv  = totalInconv/(double)numRiders;
    double avgSavings = totalSavings/(double)numRiders;
    
    // now define RouteMetrics object
    Route::RouteMetrics * pRouteMetrics = new Route::RouteMetrics(totalTripDist, sharedDist, avgInconv, avgSavings);    
    return pRouteMetrics;
}
const double MultiplePickupsModel::computeTotalTripDistance(Route * pRoute, const double &sharedDist) {
  
    // compute total trip distance    
    std::vector<RouteEvent*> * pickups  = pRoute->getPickupEvents();
    std::vector<RouteEvent*> * dropoffs = pRoute->getDropoffEvents();
    
    // compute pickup distances
    double totalPickupDist = 0.0;
    for( int ii = 0; ii < pickups->size()-1; ii++ ) {
        RouteEvent * currPickup = pickups->at(ii);
        RouteEvent * nextPickup = pickups->at(ii+1);
        const double currPickupDist = Utility::computeGreatCircleDistance(currPickup->getLat(), currPickup->getLng(), nextPickup->getLat(), nextPickup->getLng());
        totalPickupDist += currPickupDist;
    }    
    
    // compute drop distance
    double totalDropDist = 0.0;
    for( int jj = 0; jj < dropoffs->size()-1; jj++ ) {
        RouteEvent * currDrop = dropoffs->at(jj);
        RouteEvent * nextDrop = dropoffs->at(jj+1);
        const double currDropDist = Utility::computeGreatCircleDistance(currDrop->getLat(), currDrop->getLng(), nextDrop->getLat(), nextDrop->getLng());
        totalDropDist += currDropDist;
    }
    
    // define total trip distance
    const double totalTripDist = totalPickupDist + sharedDist + totalDropDist;
    return totalTripDist;
}
/*
 *   compute the cost of a candidate match
 *     - computed as difference in driver distance from master pickup to final drop
 *     - note: when pickup swaps are to be considered, then we need to also consider distance to master pickup (if pickup has not occurred)
 */
const double MultiplePickupsModel::computeCostOfRoute(Route* pRoute) const {
    int n = pRoute->getPickupEvents()->size();
    assert( (1 <= n) && (n <= 3));
    
    // use master inconvenience
    const double driverTripDist = pRoute->getRouteMetrics()->_totalTripDist;
    const double masterTripDist = pRoute->getMasterMetrics()->pooledDist;
    double addedDistance = driverTripDist - masterTripDist;
    
    if( 0.0 < abs(addedDistance) < 0.0001 ) {
        addedDistance = 0.0;
    }
    
    const double cost = addedDistance;
    
    return cost;
}
const double MultiplePickupsModel::computeCostOfMatch(FeasibleMatch * pMatch) const {
    
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
    
    if( 0.0 < abs(addedDistance) < 0.0001 ) {
        addedDistance = 0.0;
    }
    
    const double cost = addedDistance;
      
    return cost;
}

AssignedTrip * MultiplePickupsModel::convertFeasibleMatchToAssignedTripObject(FeasibleMatch * pMatch) {
    
    AssignedTrip * pTrip = new AssignedTrip(pMatch->pDriver, pMatch->_masterDispatchEvent, pMatch->_masterTripUUID, pMatch->_masterRequestEvent, pMatch->_masterPickupEventFromActuals, pMatch->_masterDropEventFromActuals, pMatch->_masterReqIndex);
    pTrip->setMasterId(pMatch->_masterId);
    pTrip->setMinionId(pMatch->_minionId);
    pTrip->setMasterIndex(pMatch->_masterIndex);
    pTrip->setMinionIndex(pMatch->_minionIndex);
    pTrip->setMinionTripUUID(pMatch->_minionTripUUID);
    
    pTrip->setMatch(pMatch);

    return pTrip;
}
bool MultiplePickupsModel::removeRouteFromAssignedRoutes(Route * pMatchedRoute, std::set<Route*, RouteEndComp> *pOpenRoutes) {
    for( std::set<Route*, RouteEndComp>::iterator routeItr = pOpenRoutes->begin(); routeItr != pOpenRoutes->end(); ) {
        if( (*routeItr)->getRouteIndex() == pMatchedRoute->getRouteIndex() )  {
            pOpenRoutes->erase(routeItr++);
            return true;
        } else {
            routeItr++;
        }
    }
}

Route * MultiplePickupsModel::createNewRouteWithUnmatchedRider(const Driver * pDriver, Request * pMinionRequest, const Event * pActualDispatchEvent) {
    
    const Event * pDispatchEvent = pMinionRequest->getActualDispatchEvent();
    assert( pDispatchEvent != NULL );
    
    Route * pOpenRoute = new Route(_numRoutesCreated, pDriver, pDispatchEvent);
    pOpenRoute->addRequest(pMinionRequest);
    pOpenRoute->addPickupEvent(pMinionRequest,  pMinionRequest->getActualPickupEvent()->timeT, pMinionRequest->getActualPickupEvent()->lat, pMinionRequest->getActualPickupEvent()->lng);
    pOpenRoute->addDropoffEvent(pMinionRequest, pMinionRequest->getActualDropEvent()->timeT, pMinionRequest->getActualDropEvent()->lat, pMinionRequest->getActualDropEvent()->lng);
    
    _numRoutesCreated++;
    
    return pOpenRoute;
}

bool MultiplePickupsModel::checkIfCandRouteMeetsSavings(Route* pCandRoute) {
        
    // check savings for master
    const RiderMetrics * pMasterMetrics = pCandRoute->getMasterMetrics();
    assert( pMasterMetrics != NULL );
    const double savings_master = pMasterMetrics->savings;
    bool isSavings = (savings_master >= this->_minOverlapThreshold);
    if( isSavings == false  )
        return false;
    
    // check savings for minion
    const RiderMetrics * pMinionMetrics = pCandRoute->getMinionMetrics();
    if( pMinionMetrics != NULL ) {
        const double savings_minion = pMinionMetrics->savings;
        if( savings_minion < this->_minOverlapThreshold ) 
            return false;
    }
    
    // check savings for parasite
    const RiderMetrics * pParasiteMetrics = pCandRoute->getParasiteMetrics();
    if( pParasiteMetrics != NULL ) {
        const double savings_parasite = pParasiteMetrics->savings;
        if( savings_parasite < this->_minOverlapThreshold )
            return false;
    }
        
    return true;
}
RouteEvent * MultiplePickupsModel::getDropEventInRouteForRider(const int riderIndex, Route* pCandRoute) {
    
    for( std::vector<RouteEvent*>::iterator dropItr = pCandRoute->getDropoffEvents()->begin(); dropItr != pCandRoute->getDropoffEvents()->end(); ++dropItr ) {
        if( (*dropItr)->getRequest()->getRiderIndex() == riderIndex ) {
            return *dropItr;
        }
    }
    
    return NULL;
}

// --------------------
//    build solution
// --------------------
void MultiplePickupsModel::buildSolution(std::set<AssignedRoute*, AssignedRouteIndexComp> &assignedRoutes) {
    
    // instantiate solution
    pSolution = new MultPickupSoln(MULTIPLE_PICKUPS, _startTime, _endTime, _allRequests.size(), _allDrivers->size(), assignedRoutes, _disqualifiedRequests, _maxAllowablePickups);
    
    // compute solution metrics
    try {
        pSolution->buildSolutionMetrics();    
    } catch( RiderCountException &ex ) {
        std::cout << "\n*** ERROR: RiderCountException thrown ***\n" << std::endl;
        std::cout << ex.what() << std::endl;
        std::cout << "\texiting... " << std::endl;
        exit(1);
    } catch( exception &ex ) {
        std::cout << "\n** exception: " << ex.what() << " **\n" << std::endl;
    }
}

// --------------------
//     I/O goodies
// --------------------
void MultiplePickupsModel::initCandidateMatchFile(std::ofstream &outFile) {    
    outFile << "\n----------------------------------------\n" << std::endl;
    outFile << "       MITM Multiple Match Candidates" << std::endl;
    outFile << "          max allowable pickups: " << Utility::intToStr(this->_maxAllowablePickups) << endl;
    outFile << "\n----------------------------------------\n\n" << std::endl;      
}
void MultiplePickupsModel::printRouteCandidatesForCurrRequest(std::multimap<const double, Route*> * pCandRouteMap, std::ofstream * pOut) {
                
    int costBuff = 20;
    int numBuff = 10;
    int indexBuff = 10; 
    
    int masterIdBuff = 11;
    int extBuff = 13;
    int dropBuff = 12;
    int distBuff = 22;
    int locBuff = 32;
    
    *pOut << left << setw(costBuff) << "cost" << 
            left << setw(indexBuff) << "rtIndex" << 
            left << setw(numBuff) << "NRiders" << 
            left << setw(indexBuff) << "pickup1" << 
            left << setw(indexBuff) << "pickup2" << 
            left << setw(indexBuff) << "pickup3" << 
            left << setw(indexBuff) << "drop1" << 
            left << setw(indexBuff) << "drop2" << 
            left << setw(indexBuff) << "drop3" << endl;    
  
    for( std::multimap<const double, Route*>::iterator rtItr = pCandRouteMap->begin(); rtItr != pCandRouteMap->end(); ++rtItr ) {
        const int rtIndex = rtItr->second->getRouteIndex();
        const std::vector<Request*> * pRequests = rtItr->second->getRequests();
        int numRiders = (int)pRequests->size();
        std::vector<RouteEvent*> * pPickups = rtItr->second->getPickupEvents();
        std::vector<RouteEvent*> * pDrops   = rtItr->second->getDropoffEvents();
        std::string pickup1Str = (numRiders>0) ? Utility::intToStr(pPickups->at(0)->getRequest()->getRiderIndex()) : "-";
        std::string pickup2Str = (numRiders>1) ? Utility::intToStr(pPickups->at(1)->getRequest()->getRiderIndex()) : "-";
        std::string pickup3Str = (numRiders>2) ? Utility::intToStr(pPickups->at(2)->getRequest()->getRiderIndex()) : "-";
        std::string drop1Str = (numRiders>0) ? Utility::intToStr(pDrops->at(0)->getRequest()->getRiderIndex()) : "-";
        std::string drop2Str = (numRiders>1) ? Utility::intToStr(pDrops->at(1)->getRequest()->getRiderIndex()) : "-";
        std::string drop3Str = (numRiders>2) ? Utility::intToStr(pDrops->at(2)->getRequest()->getRiderIndex()) : "-";
    
        *pOut << left << setw(costBuff) << Utility::truncateDouble(rtItr->first,4) << 
                 left << setw(indexBuff) << Utility::intToStr(rtIndex) << 
                 left << setw(indexBuff) << Utility::intToStr(numRiders) << 
                 left << setw(indexBuff) << pickup1Str << 
                 left << setw(indexBuff) << pickup2Str << 
                 left << setw(indexBuff) << pickup3Str << 
                 left << setw(indexBuff) << drop1Str << 
                 left << setw(indexBuff) << drop2Str << 
                 left << setw(indexBuff) << drop3Str << 
            endl;
    }
              
    *pOut << "\n" << std::endl;    
}
