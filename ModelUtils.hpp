    /* 
 * File:   ModelUtils.hpp
 * Author: jonpetersen
 *
 * Created on April 3, 2015, 7:40 PM
 */

#ifndef MODELUTILS_HPP
#define	MODELUTILS_HPP

#include "AssignedTrip.hpp"
#include "AssignedRoute.hpp"
#include "OpenTrip.hpp"
#include "Route.hpp"
#include "Event.hpp"
#include "Comparators.hpp"
#include "FeasibleMatch.hpp"
#include "Request.hpp"
#include "UFBW_fixed.hpp"
#include "MasterCand.hpp"
#include "MinionCand.hpp"

#include <iostream>
#include <set>
#include <vector>
#include <map>
#include <cmath>
using namespace std;

class ModelUtils {
    
public:
    
    static AssignedTrip * convertOpenTripToAssignedTrip(OpenTrip * pOpenTrip) {
  
        // if a trip has been completed, define the open trip here                
        AssignedTrip * pUnmatchedTrip = new AssignedTrip(pOpenTrip->getDriver(), pOpenTrip->getMasterDispatcEvent(), pOpenTrip->getRiderTripUUID(), pOpenTrip->getMasterRequestEvent(), pOpenTrip->getMasterActualPickupEvent(), pOpenTrip->getMasterActualDropEvent(), pOpenTrip->getOrigRequestIndex());
        
        pUnmatchedTrip->setMasterId(pOpenTrip->getMasterID());
        pUnmatchedTrip->setMasterIndex(pOpenTrip->getMasterIndex());
                
        return pUnmatchedTrip;
    }
    
    /*
     *   the following method accomplishes two things:
     *       1. defines the subset of OpenTrip objects that have completed (w.r.t. ETD) relative to the inputted time
     *       2. removes the expired open trips from the set of inputted OpenTrip objects
     *   this is used in models to track eligible master candidates
     */
    static std::set<AssignedTrip*, AssignedTripIndexComp> getCompletedOpenTrips(const time_t &tm, std::set<OpenTrip*,EtdComp> &openTrips, std::set<AssignedTrip*, AssignedTripIndexComp> &assignedTrips) {

        std::set<AssignedTrip*, AssignedTripIndexComp> unmatchedTrips;
               
        // iterate over open trips and check for completion
        std::set<OpenTrip*, EtdComp>::iterator openTripItr;
        std::set<OpenTrip*, EtdComp>::iterator lastDeletionItr = openTrips.begin();
        for( openTripItr = openTrips.begin(); openTripItr != openTrips.end(); ++openTripItr ) {

            // terminate loop if ETD is after current time epoch (openTrips are sorted by ETD)
            if( (*openTripItr)->getETD() > tm ) {
                lastDeletionItr = openTripItr;
                break;
            }

            // convert expired open trip to AssignedTrip that is unmatched
            AssignedTrip * pAssignedTrip = ModelUtils::convertOpenTripToAssignedTrip(*openTripItr);
            
            pAssignedTrip->setIndex(assignedTrips.size());            
            assignedTrips.insert(pAssignedTrip); 
                                   
            unmatchedTrips.insert(pAssignedTrip);
            lastDeletionItr = openTripItr;
        }
                
        // now delete all expired open trips
        if( lastDeletionItr != openTrips.begin() ) {
            openTrips.erase(openTrips.begin(), lastDeletionItr); 
        }        
        
        return unmatchedTrips;  
    }
    
    /*
     *   the following method accomplishes two things:
     *       1. defines the subset of OpenTrip objects that have completed (w.r.t. ETD) relative to the inputted time
     *       2. removes the expired open trips from the set of inputted OpenTrip objects
     *   this is used in models to track eligible master candidates
     */
    static int assignCompletedOpenRoutes(const time_t &tm, std::set<Route*, RouteEndComp> &openRoutes, std::set<AssignedRoute*, AssignedRouteIndexComp> &assignedRoutes) {
                
        int numCompletedRoutes = 0;
        
        std::set<Route*, RouteEndComp>::iterator lastDeletionItr = openRoutes.begin();
        for( std::set<Route*, RouteEndComp>::iterator routeItr = openRoutes.begin(); routeItr != openRoutes.end(); ++routeItr ) {
            
            // terminate loop if ETD is after current time epoch (openTrips are sorted by ETD)
            if( (*routeItr)->getRouteEndTime() > tm ) {
                lastDeletionItr = routeItr;
                break;
            }

            AssignedRoute * pAssignedRoute = new AssignedRoute(*routeItr);             
            assignedRoutes.insert(pAssignedRoute); 
            numCompletedRoutes++;
                                               
            lastDeletionItr = routeItr;
        }
        
        // now delete all expired open trips
        if( lastDeletionItr != openRoutes.begin() ) {
            openRoutes.erase(openRoutes.begin(), lastDeletionItr); 
        }            
        
        return numCompletedRoutes;
    }
    
    //static double getPickupDistanceAtTimeOfMinionRequest(Request * pRequest, OpenTrip * pOpenTrip) {
    static double getPickupDistanceAtTimeOfMinionRequest_maxPickupConstr(
            const time_t minionReqTime,         // request time
            const double minionPickupLat,       // request lat
            const double minionPickupLng,       // request lng
            const time_t masterPickupTime,      // previous event time
            const double masterPickupLat,       // previous event start lat
            const double masterPickupLng,       // previous event start lng
            const time_t masterDropoffTime,     // next event time
            const double masterDropLat,         // next event end lat
            const double masterDropLng,         // next event end lng
            const time_t masterDispatchTime,    // dispatch time
            const double masterDispatchLat,     // dispatch lat
            const double masterDispatchLng      // dispatch lng
            ) {
        
        // case 1: pickup has NOT occurred (non-extended)
        if( minionReqTime <= masterPickupTime ) {
            
            // pickup distance from current location to minion origin
            LatLng estMasterLocation = Utility::estLocationByLinearProxy(minionReqTime, masterDispatchTime, masterDispatchLat, masterDispatchLng, masterPickupTime, masterPickupLat, masterPickupLng);
            double pickupDistance_driverToMaster = Utility::computeGreatCircleDistance(estMasterLocation.getLat(), estMasterLocation.getLng(), masterPickupLat, masterPickupLng);
            
            // pickup distance from master to minion
            double pickupDistance_masterToMinion = Utility::computeGreatCircleDistance(masterPickupLat, masterPickupLng, minionPickupLat, minionPickupLng);
            
            double pickupDistance_total = pickupDistance_driverToMaster + pickupDistance_masterToMinion;
            return pickupDistance_total;
        }
        
        // case 2: pickup HAS occurred (extended)
        else {
            // estimate location     
            LatLng estLocAtTimeOfRequest = Utility::estLocationByLinearProxy(minionReqTime, masterPickupTime, masterPickupLat, masterPickupLng, masterDropoffTime, masterDropLat, masterDropLng);
            
            // pickup distance is 
            double pickupDistance = Utility::computeGreatCircleDistance(estLocAtTimeOfRequest.getLat(), estLocAtTimeOfRequest.getLng(), minionPickupLat, minionPickupLng);
            
            return pickupDistance;
        }                
    }
    
    static double getPickupDistance_maxPickupConstr_route(Route * pRoute, Request * pRequest, int pickupOrder) {
        
        // ensure there are two existing pickups
        assert( pRoute->getPickupEvents()->size() == 2 );
        
        const time_t reqTime = pRequest->getReqTime();
                
        const Event * pDispatch = pRoute->getDispatchEvent();
        RouteEvent * pFirstSchedPick  = pRoute->getPickupEvents()->front();
        RouteEvent * pSecondSchedPick = pRoute->getPickupEvents()->back();
        
        // case 1: the request is to be the first pickup in the new route
        if ( pickupOrder == 1 ) {
            // 1.A the request occurs BEFORE the first sched pickup
            if( reqTime < pFirstSchedPick->getEventTime() ) {
                LatLng currLoc = Utility::estLocationByLinearProxy(reqTime, pDispatch->timeT, pDispatch->lat, pDispatch->lng, pFirstSchedPick->getEventTime(), pFirstSchedPick->getLat(), pFirstSchedPick->getLng());
                double distToCurrReq = Utility::computeGreatCircleDistance(currLoc.getLat(), currLoc.getLng(), pRequest->getActualPickupEvent()->lat, pRequest->getActualPickupEvent()->lng);
                return distToCurrReq;
            }
            // 1.B the request already occurred and request is in between first and second scheduled pickup
            else if( (pFirstSchedPick->getEventTime() <= reqTime) && (reqTime <= pSecondSchedPick->getEventTime()) ) {
                LatLng currLoc = Utility::estLocationByLinearProxy(reqTime, pFirstSchedPick->getEventTime(), pFirstSchedPick->getLat(), pFirstSchedPick->getLng(), pSecondSchedPick->getEventTime(), pSecondSchedPick->getLat(), pSecondSchedPick->getLng());
                double distToCurrReq = Utility::computeGreatCircleDistance(currLoc.getLat(), currLoc.getLng(), pRequest->getActualPickupEvent()->lat, pRequest->getActualPickupEvent()->lng);
                return distToCurrReq;
            }
        }
        
        // case 2: the request is not to be the first pickup in the new route
        else {
            // get prior pickup event
            RouteEvent * pPriorPickup = pRoute->getPickupEvents()->at(pickupOrder-2); // e.g. if the 2nd pickup, then the prior is the 1st pickup which is index 0
            
            // 2.A: the request occurs BEFORE the pickup of the prior event
            if( reqTime < pPriorPickup->getEventTime() ) {
                double distToCurrReq = Utility::computeGreatCircleDistance(pPriorPickup->getLat(), pPriorPickup->getLng(), pRequest->getActualPickupEvent()->lat, pRequest->getActualPickupEvent()->lng);
                return distToCurrReq;
            }
            
            // 2.B: the requests occurs AFTER pickup of the prior event
            else {
                RouteEvent * pNextEvent = (pickupOrder == 2) ? pRoute->getPickupEvents()->back() : pRoute->getDropoffEvents()->front();
                LatLng currLoc = Utility::estLocationByLinearProxy(reqTime, pPriorPickup->getEventTime(), pPriorPickup->getLat(), pPriorPickup->getLng(), pNextEvent->getEventTime(), pNextEvent->getLat(), pNextEvent->getLng());
                double distToCurrReq = Utility::computeGreatCircleDistance(currLoc.getLat(), currLoc.getLng(), pRequest->getActualPickupEvent()->lat, pRequest->getActualPickupEvent()->lng);
                return distToCurrReq;
            }
            
        }
        
        cout << "** unhandled case when checking for pickup distance to minion req **\n" << endl;
        exit(1);
        return 0.0;
    }
    
    /*
     * 
     *    given master, minion pair determine if the match is feasible where the MASTER is dropped first
     *      MASTER constraint: s1 + 0.5h <= (1-t)x 
     *      MINION constraint: 0.5h + s2 <= (1-t)y where
     *          s1 = distance of single leg to pick up minion 
     *          s2 = distance of single leg to drop off minion
     *          h  = distance of shared leg 
     *          x  = distance of uberX trip for master 
     *          y  = distance of uberX trip for minion
     *          t  = threshold [0,1] savings required for legality
     */
    static FeasibleMatch * checkIfOverlapIsFeasWithforFIFOMatch(const double _minOverlapThreshold, const std::string minionId, const double distToMinion, const double masterUberXDist, const double minionUberXDist, MinionCand * pMinionCand, MasterCand * pMasterCand, int waitTimeInSecOfMaster, int waitTimeInSecOfMinion) {
    
        // step 1: compute h: distance of shared leg with MASTER being dropped first (Haversine)
        const double minionPickupLat  = pMinionCand->_reqOrig.getLat(); //pMinionReq->getPickupLat();
        const double minionPickupLng  = pMinionCand->_reqOrig.getLng(); //pMinionReq->getPickupLng();
        const double masterDropLat    = pMasterCand->_reqDest.getLat();
        const double masterDropLng    = pMasterCand->_reqDest.getLng();
        const double sharedDistance   = Utility::computeGreatCircleDistance(minionPickupLat, minionPickupLng, masterDropLat, masterDropLng);
        const double dropDistance     = Utility::computeGreatCircleDistance(masterDropLat, masterDropLng, pMinionCand->_reqDest.getLat(), pMinionCand->_reqDest.getLng());

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
            const double dropDist = Utility::computeGreatCircleDistance(masterDropLat, masterDropLng, pMinionCand->_reqDest.getLat(), pMinionCand->_reqDest.getLng());
            const double totalDistMaster = distToMinion + sharedDistance;
            const double totalDistMinion = sharedDistance + dropDist;
            const bool   isExtendedMatch = (pMasterCand->_ETA <= pMinionCand->_reqTime);
            const double addlDistMaster = totalDistMaster - masterUberXDist;
            const double addlDistMinion = totalDistMinion - minionUberXDist;
            const double pctAddlDistMaster = (double)100*(double)addlDistMaster/(double)masterUberXDist;
            const double pctAddlDistMinion = (double)100*(double)addlDistMinion/(double)minionUberXDist;
            
            const double totalTripDistance =  distToMinion + sharedDistance + dropDistance; // note: distToMinion includes initial distance traveled by master at time of match to minion
                        
            const double masterDistCostSavings = masterUberXDist - distance_pool_master;
            const double minionDistCostSavings = minionUberXDist - distance_pool_minion;
            const double masterDistCostPctSavings = (double)100*(masterDistCostSavings/masterUberXDist);
            const double minionDistCostPctSavings = (double)100*(minionDistCostSavings/minionUberXDist);
            const double avgDistCostPctSavings = (masterDistCostPctSavings + minionDistCostPctSavings)/(double)2;
            
            // get dispatch time (-1 if dispatch has not yet occurred)
            time_t masterDispatchTime = (pMasterCand->pDispatchEvent == NULL) ? -1 : pMasterCand->pDispatchEvent->timeT;
            
            // get locations of driver and master at time of minion request
            LatLng masterDriverLocAtTimeOfMinionReq = ModelUtils::computeMasterDriverLocAtTimeOfMinionReq(pMasterCand->pReqEvent, pMasterCand->pDispatchEvent, pMasterCand->pPickupEvent, pMasterCand->pDropEvent, pMinionCand->_reqTime);
            LatLng masterLocAtTimeOfMinionReq = ModelUtils::computeMasterLocAtTimeOfMinionReq(pMasterCand->pPickupEvent, pMasterCand->pDropEvent, pMinionCand->_reqTime);
            FeasibleMatch * pFeasMatch = new FeasibleMatch(pMasterCand->pDriver, pMasterCand->_riderID, pMasterCand->_riderIndex, pMasterCand->_riderTripUUID, minionId, 
                        pMinionCand->_riderIndex, pMinionCand->_riderTripUUID, true, pMasterCand->pReqEvent, pMasterCand->pDispatchEvent, isExtendedMatch, 
                        distToMinion, sharedDistance, dropDist, totalTripDistance, totalDistMaster, totalDistMinion, masterUberXDist, minionUberXDist,
                        pMasterCand->_reqTime, masterDispatchTime, pMasterCand->_ETA, pMasterCand->_ETD, pMasterCand->_ETD, pMasterCand->pPickupEvent, pMasterCand->pDropEvent, pMasterCand->_reqOrig, pMasterCand->_reqDest, pctAddlDistMaster,
                        pMinionCand->_reqTime, -1, -1, -1, pMinionCand->_reqOrig, pMinionCand->_reqDest, pctAddlDistMinion,
                        masterDistCostPctSavings, minionDistCostPctSavings, avgDistCostPctSavings, pMasterCand->_requestIndex, pMinionCand->pRequest->getReqIndex(), masterDriverLocAtTimeOfMinionReq, masterLocAtTimeOfMinionReq,
                        waitTimeInSecOfMaster, waitTimeInSecOfMinion);   
            return pFeasMatch;
        } else {
            return NULL;
        }
    }
    
     /*
      *   given master, minion pair determine if the match is feasible where the MINION is dropped first
     *      MASTER constraint: (s1 + 0.5h + s2) <= (1-t)x 
     *      MINION constraint: 0.5h <= (1-t)y where
     *          s1 = distance of single leg to pick up minion 
     *          h  = distance of shared leg 
     *          s2 = distance of single leg from minion drop to master drop
     *          x  = distance of uberX trip for master 
     *          y  = distance of uberX trip for minion
     *          t  = threshold [0,1] discount required for legality
     */  
    static FeasibleMatch * checkIfOverlapIsFeasWithforFILOMatch(const double _minOverlapThreshold, const double distToMinion, const double masterUberXDist, const double minionUberXDist, MinionCand * pMinionCand, MasterCand * pMasterCand, int waitTimeInSecOfMaster, int waitTimeInSecOfMinion) {
     
        // step 1: compute h: distance of shared leg with MINION being dropped first (Haversine) 
        const double minionPickupLat  = pMinionCand->_reqOrig.getLat();
        const double minionPickupLng  = pMinionCand->_reqOrig.getLng();
        const double minionDropoffLat = pMinionCand->_reqDest.getLat();
        const double minionDropoffLng = pMinionCand->_reqDest.getLng();
        const double sharedDistance   = Utility::computeGreatCircleDistance(minionPickupLat, minionPickupLng, minionDropoffLat, minionDropoffLng);

        // step 2: compute s2: distance from minion drop to master drop
        const double masterDropLat    = pMasterCand->_reqDest.getLat(); // pMasterCand->getDropRequestLat();
        const double masterDropLng    = pMasterCand->_reqDest.getLng(); // _pMasterCand->getDropRequestLng();
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
            LatLng minionOrig = LatLng(minionPickupLat, minionPickupLng);
            LatLng minionDest = LatLng(minionDropoffLat, minionDropoffLng);
            const double totalDistMaster = distToMinion + sharedDistance + distToMasterDrop;
            const bool isExtendedMatch = (pMasterCand->_ETA <= pMinionCand->_reqTime);
            const double minionUberXDist = Utility::computeGreatCircleDistance(minionPickupLat, minionPickupLng, minionDropoffLat, minionDropoffLng);
            const double addlDistMaster = (double)totalDistMaster - (double)masterUberXDist;

            const double pctAddlDistMaster = (double)100*(double)addlDistMaster/(double)masterUberXDist;
            const double pctAddlDistMinion = 0.0;
            
            const double masterDistCostSavings = masterUberXDist - distance_pool_master;
            const double minionDistCostSavings = minionUberXDist - distance_pool_minion;
            const double masterDistCostPctSavings = (double)100*(masterDistCostSavings/masterUberXDist);
            const double minionDistCostPctSavings = (double)100*(minionDistCostSavings/minionUberXDist);
            const double avgDistCostPctSavings = (masterDistCostPctSavings + minionDistCostPctSavings)/(double)2;
            
            const double totalTripDistance =  distToMinion + sharedDistance + distToMasterDrop;            

            // get dispatch time (-1 if dispatch has not yet occurred)
            time_t masterDispatchTime = (pMasterCand->pDispatchEvent == NULL) ? -1 : pMasterCand->pDispatchEvent->timeT;
            
            // get master location at time of minion request
            LatLng masterDriverLocAtTimeOfMinionReq = ModelUtils::computeMasterDriverLocAtTimeOfMinionReq(pMasterCand->pReqEvent, pMasterCand->pDispatchEvent, pMasterCand->pPickupEvent, pMasterCand->pDropEvent, pMinionCand->_reqTime);
            LatLng masterLocAtTimeOfMinionReq = ModelUtils::computeMasterLocAtTimeOfMinionReq(pMasterCand->pPickupEvent, pMasterCand->pDropEvent, pMinionCand->_reqTime);

            FeasibleMatch * pFeasMatch = new FeasibleMatch(pMasterCand->pDriver, pMasterCand->_riderID, pMasterCand->_riderIndex, pMasterCand->_riderTripUUID, 
                    pMinionCand->_riderID, pMinionCand->_riderIndex, pMinionCand->_riderTripUUID, false, pMasterCand->pReqEvent, pMasterCand->pDispatchEvent, isExtendedMatch,
                    distToMinion, sharedDistance, distToMasterDrop, totalTripDistance, totalDistMaster, sharedDistance, masterUberXDist, minionUberXDist,
                    pMasterCand->_reqTime, masterDispatchTime, pMasterCand->_ETA, -1, pMasterCand->_ETD, pMasterCand->pPickupEvent, pMasterCand->pDropEvent, pMasterCand->_reqOrig, pMasterCand->_reqDest, pctAddlDistMaster,
                    pMinionCand->_reqTime, pMinionCand->_reqTime, -1, -1, minionOrig, minionDest, pctAddlDistMinion,
                    masterDistCostPctSavings, minionDistCostPctSavings, avgDistCostPctSavings, pMasterCand->_requestIndex, pMinionCand->pRequest->getReqIndex(), masterDriverLocAtTimeOfMinionReq, masterLocAtTimeOfMinionReq,
                    waitTimeInSecOfMaster, waitTimeInSecOfMinion);    
            return pFeasMatch;
        } else {
            return NULL;
        }
    }
    
    
    // compute pick up distance
    static double computePickupDistance_savingsConstr(
            const time_t masterETA, const double masterOrigLat, const double masterOrigLng, 
            const time_t masterETD, const double masterDestLat, const double masterDestLng, 
            const time_t minionReqTime, const double minionOrigLat, const double minionOrigLng, 
            int inclInitDistExtendedMatches
    ) {                
        double pickupDist = 0.0;
        
        bool isExtended = (masterETA <= minionReqTime);
        
        // case 1: non-extended match
        if( !isExtended ) {
            pickupDist = Utility::computeGreatCircleDistance(masterOrigLat, masterOrigLng, minionOrigLat, minionOrigLng);
        }
        
        // case 2: extended match
        else {
            // get the master location at time of minion request
            LatLng estLocAtMinReq = Utility::estLocationByLinearProxy(minionReqTime, masterETA, masterOrigLat, masterOrigLng, masterETD, masterDestLat, masterDestLng);
            
            // compute dist between master origin and curr location
            const double distFromOrigToCurrLoc = (inclInitDistExtendedMatches) ? Utility::computeGreatCircleDistance(masterOrigLat, masterOrigLng, estLocAtMinReq.getLat(), estLocAtMinReq.getLng()) : 0.0;
            
            // compute dist between curr location and minion origin
            const double distFromCurrLocToMinion = Utility::computeGreatCircleDistance(estLocAtMinReq.getLat(), estLocAtMinReq.getLng(), minionOrigLat, minionOrigLng);
            
            pickupDist = distFromOrigToCurrLoc + distFromCurrLocToMinion;               
        }
        
        return pickupDist;
    }
    
    static LatLng computeMasterDriverLocAtTimeOfMinionReq(const Event * pMasterReqEvent, const Event * pMasterDispatchEvent, const Event * pMasterPickupEvent, const Event * pMasterDropEvent, const time_t minionReqTime) {
        
        // case 1: master dispatch event is NULL (possible in batching framework)
        if( pMasterDispatchEvent == NULL ) {
            LatLng initMasterLoc(pMasterPickupEvent->lat,pMasterPickupEvent->lng);
            return initMasterLoc;
        }
        
        // case 2: master dispatch event is not NULL (always the case in MITM and possibly true with UFBW)
        else {        
            // case 2.A: minion request is between master dispatch and master pickup
            if( (pMasterDispatchEvent->timeT <= minionReqTime) && (minionReqTime <= pMasterPickupEvent->timeT) ) {
                LatLng estLocation = Utility::estLocationByLinearProxy(minionReqTime, pMasterDispatchEvent->timeT, pMasterDispatchEvent->lat, pMasterDispatchEvent->lng, pMasterPickupEvent->timeT, pMasterPickupEvent->lat, pMasterPickupEvent->lng);
                return estLocation;
            }

            // case 2.B: minion request is between master pickup and master dropoff
            if( (pMasterPickupEvent->timeT <= minionReqTime) && (minionReqTime <= pMasterDropEvent->timeT) ) {
                LatLng estLocation = Utility::estLocationByLinearProxy(minionReqTime, pMasterPickupEvent->timeT, pMasterPickupEvent->lat, pMasterPickupEvent->lng, pMasterDropEvent->timeT, pMasterDropEvent->lat, pMasterDropEvent->lng);
                return estLocation;
            }
            
            // case 2.C: minion request is between master request and master dispatch
            if( (pMasterReqEvent->timeT <= minionReqTime) && (minionReqTime <= pMasterDispatchEvent->timeT) ) {
                LatLng initMasterLoc(pMasterPickupEvent->lat,pMasterPickupEvent->lng);
                return initMasterLoc;
            }
            
            std::cout << "\n\n** ERROR: estimated driver location ill-defined (should be between master dispatch and master drop events) **\n\n" << std::endl;
            std::cout << "\texiting... " << std::endl;
            std::cout << "\tminion req time: " << Utility::convertTimeTToString(minionReqTime) << std::endl;            
            std::cout << "\tmaster dispatch time: " << Utility::convertTimeTToString(pMasterDispatchEvent->timeT) << std::endl;
            std::cout << "\tmaster pickup time: " << Utility::convertTimeTToString(pMasterPickupEvent->timeT) << std::endl;
            std::cout << "\tmaster drop time: " << Utility::convertTimeTToString(pMasterDropEvent->timeT) << std::endl;
            exit(-1);
            return LatLng(-1,-1);
            
        }
    }
    
    static LatLng computeMasterLocAtTimeOfMinionReq(const Event * pMasterPickupEvent, const Event * pMasterDropEvent, const time_t minionReqTime) {
        
        // case 1: minion request is BEFORE master pickup
        if( minionReqTime <= pMasterPickupEvent->timeT ) {
            LatLng masterLocation(pMasterPickupEvent->lat, pMasterPickupEvent->lng);
            return masterLocation;
        }
        
        // case 2: minion request is BETWEEN master pickup and drop
        if( (pMasterPickupEvent->timeT <= minionReqTime) && (minionReqTime <= pMasterDropEvent->timeT) ) {
            LatLng estLocation = Utility::estLocationByLinearProxy(minionReqTime, pMasterPickupEvent->timeT, pMasterPickupEvent->lat, pMasterPickupEvent->lng, pMasterDropEvent->timeT, pMasterDropEvent->lat, pMasterDropEvent->lng);
            return estLocation;
        }
        
        std::cout << "\n\n** ERROR: estimated master location ill-defined (should be between master dispatch and master drop events) **\n\n" << std::endl;
        std::cout << "\texiting... " << std::endl;
        exit(-1);
    }
    
};

#endif	/* MODELUTILS_HPP */

