/* 
 * File:   ModelUtils.hpp
 * Author: jonpetersen
 *
 * Created on April 3, 2015, 7:40 PM
 */

#ifndef MODELUTILS_HPP
#define	MODELUTILS_HPP

#include "AssignedTrip.hpp"
#include "OpenTrip.hpp"
#include "Event.hpp"
#include "Comparators.hpp"
#include "FeasibleMatch.hpp"
#include "Request.hpp"
#include "UFBW_fixed.hpp"

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
        Event masterReq(pOpenTrip->getMasterRequestEvent()->timeT, pOpenTrip->getMasterRequestEvent()->lat, pOpenTrip->getMasterRequestEvent()->lng);
        Event masterDispatch(pOpenTrip->getMasterDispatcEvent()->timeT, pOpenTrip->getMasterDispatcEvent()->lat, pOpenTrip->getMasterDispatcEvent()->lng);

        AssignedTrip * pUnmatchedTrip = new AssignedTrip(pOpenTrip->getDriver());
        
        pUnmatchedTrip->setMasterId(pOpenTrip->getMasterID());
        pUnmatchedTrip->setMasterIndex(pOpenTrip->getMasterIndex());
        pUnmatchedTrip->setMasterRequest(masterReq);
        pUnmatchedTrip->setMasterDispatch(masterDispatch);        

        Event actPickUp(pOpenTrip->getETA(), pOpenTrip->getActPickupLat(), pOpenTrip->getActPickupLng());
        pUnmatchedTrip->setMasterPickupFromActuals(actPickUp);
        Event actDrop(pOpenTrip->getETD(), pOpenTrip->getDropRequestLat(), pOpenTrip->getDropRequestLng());
        pUnmatchedTrip->setMasterDropFromActuals(actDrop);
        
        return pUnmatchedTrip;
    }
    
    /*
     *   the following method accomplishes two things:
     *       1. defines the subset of OpenTrip objects that have completed (w.r.t. ETD) relative to the inputted time
     *       2. removes the expired open trips from the set of inputted OpenTrip objects
     *   this is used in models to track eligible master candidates
     */
    static std::set<AssignedTrip*, AssignedTripIndexComp> getCompletedOpenTrips(const time_t &tm, std::set<OpenTrip*,EtaComp> &openTrips, std::set<AssignedTrip*, AssignedTripIndexComp> &assignedTrips) {

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
            
            pAssignedTrip->setIndex(assignedTrips.size());
            Event actualPickUp((*openTripItr)->getETA(), (*openTripItr)->getActPickupLat(), (*openTripItr)->getActPickupLng());
            pAssignedTrip->setMasterPickupFromActuals(actualPickUp);
            Event actualDropOff((*openTripItr)->getETD(), (*openTripItr)->getDropRequestLat(), (*openTripItr)->getDropRequestLng());
            pAssignedTrip->setMasterDropFromActuals(actualDropOff);
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
     *   given time, estimate location of candidate master (represented by OpenTrip object)
     */
    static LatLng estimateLocationOfMasterAtTime(const time_t &currTime, const Event * pMasterDispatch, const Event * pMasterPickup, const Event * pMasterDrop) {
        
    double estLat = 0.0;
    double estLng = 0.0;

    // case 1: the master has not yet been picked up
    if( currTime <= pMasterPickup->timeT ) {
        
        // if the current time coincides with the dispatch time then the location will be the dispatch
        if( currTime == pMasterDispatch->timeT ) {            
            LatLng locAtTimeOfDispatch = LatLng(pMasterDispatch->lat, pMasterDispatch->lng);
            return locAtTimeOfDispatch;
        }
        
        // if the current time coincides with the pickup time, then the location will be the pickup
        if( currTime == pMasterPickup->timeT ) {
            LatLng locAtTimeOfPickup = LatLng(pMasterPickup->lat, pMasterPickup->lng);
            return locAtTimeOfPickup;
        }
                
        try {    
            LatLng estLocation = Utility::estLocationByLinearProxy(currTime, pMasterDispatch->timeT, pMasterDispatch->lat, pMasterDispatch->lng, pMasterPickup->timeT, pMasterPickup->lat, pMasterPickup->lng);              
            return estLocation;
        } catch( TimeAdjacencyException &ex ) {  
            std::cerr << "\n\n*** TimeAdjacencyException caught ***\n" << std::endl;
            std::cerr << ex.what() << std::endl;
            std::cerr << "\tcurrent time:       " << Utility::convertTimeTToString(currTime) << std::endl;
            std::cerr << "\tearlier event time: " << Utility::convertTimeTToString(pMasterDispatch->timeT) << std::endl;
            std::cerr << "\tlater event time:   " << Utility::convertTimeTToString(pMasterPickup->timeT) << std::endl;
            std::cerr << "\n(exiting)" << std::endl;
            exit(1);
        }         
    }
    
    
    // case 2: the master has already been picked up
    else {
        // extract adjacent events
        const time_t masterPickupTime = pMasterPickup->timeT;
        const double masterPickupLat  = pMasterPickup->lat;
        const double masterPickupLng  = pMasterPickup->lng;
        const time_t masterDropTime   = pMasterDrop->timeT;
        const double masterDropLat    = pMasterDrop->lat;
        const double masterDropLng    = pMasterDrop->lng;
 
        try {            
            LatLng estLocation = Utility::estLocationByLinearProxy(currTime, masterPickupTime, masterPickupLat, masterPickupLng, masterDropTime, masterDropLat, masterDropLng);            
            return estLocation;
        } catch( TimeAdjacencyException &ex ) {
            std::cerr << "\n\n*** TimeAdjacencyException caught ***\n" << std::endl;
            std::cerr << ex.what() << std::endl;
            std::cerr << "\tcurrent time: " << Utility::convertTimeTToString(currTime) << std::endl;
            std::cerr << "\tearlier event time: " << Utility::convertTimeTToString(masterPickupTime) << std::endl;
            std::cerr << "\tlater event time:   " << Utility::convertTimeTToString(masterDropTime) << std::endl;
            std::cerr << "\n(exiting)" << std::endl;
            exit(1);
        }
    }
     
    LatLng estLatLng(estLat,estLng);
    return estLatLng;  
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
    static FeasibleMatch * checkIfOverlapIsFeasWithforFIFOMatch(const double _minOverlapThreshold, const std::string minionId, const double distToMinion, const double masterUberXDist, const double minionUberXDist, UFBW_fixed::MinionCand * pMinionCand, UFBW_fixed::MasterCand * pMasterCand) {
    
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
            const double uberXDistMinion = Utility::computeGreatCircleDistance(minionPickupLat, minionPickupLng, pMinionCand->_reqDest.getLat(), pMinionCand->_reqDest.getLng()); 
            const bool   isExtendedMatch = (pMasterCand->_ETA <= pMinionCand->_reqTime);
            const double addlDistMaster = (double)totalDistMaster - (double)masterUberXDist;
            const double addlDistMinion = (double)totalDistMinion - (double)uberXDistMinion;
            const double pctAddlDistMaster = (double)100*(double)addlDistMaster/(double)masterUberXDist;
            const double pctAddlDistMinion = (double)100*(double)addlDistMinion/(double)uberXDistMinion;
            
            
            const double masterSavings = abs(distance_pool_master - distance_uberX_master);
            const double minionSavings = abs(distance_pool_minion - distance_uberX_minion);
            const double avgSavings = (masterSavings + minionSavings)/(double)2;

            
            // get dispatch time (-1 if dispatch has not yet occurred)
            time_t masterDispatchTime = (pMasterCand->pDispatchEvent == NULL) ? -1 : pMasterCand->pDispatchEvent->timeT;

            FeasibleMatch * pFeasMatch = new FeasibleMatch(pMasterCand->pDriver, pMasterCand->_riderID, pMasterCand->_riderIndex, minionId, pMinionCand->_riderIndex, true, isExtendedMatch, 
                        distToMinion, sharedDistance, dropDist, totalDistMaster, totalDistMinion, masterUberXDist, uberXDistMinion,
                        pMasterCand->_reqTime, masterDispatchTime, pMasterCand->_ETA, pMasterCand->_ETD, pMasterCand->_reqOrig, pMasterCand->_reqDest, pctAddlDistMaster,
                        pMinionCand->_reqTime, -1, -1, -1, pMinionCand->_reqOrig, pMinionCand->_reqDest, pctAddlDistMinion,
                        masterSavings, minionSavings, avgSavings);    
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
    static FeasibleMatch * checkIfOverlapIsFeasWithforFILOMatch(const double _minOverlapThreshold, const double distToMinion, const double masterUberXDist, const double minionUberXDist, UFBW_fixed::MinionCand * pMinionCand, UFBW_fixed::MasterCand * pMasterCand) {
     
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
            
            const double masterSavings = abs(distance_pool_master - distance_uberX_master);
            const double minionSavings = abs(distance_pool_minion - distance_uberX_minion);
            const double avgSavings = (masterSavings + minionSavings)/(double)2;

            // get dispatch time (-1 if dispatch has not yet occurred)
            time_t masterDispatchTime = (pMasterCand->pDispatchEvent == NULL) ? -1 : pMasterCand->pDispatchEvent->timeT;

            FeasibleMatch * pFeasMatch = new FeasibleMatch(pMasterCand->pDriver, pMasterCand->_riderID, pMasterCand->_riderIndex, pMinionCand->_riderID, pMinionCand->_riderIndex, false, isExtendedMatch,
                    distToMinion, sharedDistance, distToMasterDrop, totalDistMaster, sharedDistance, masterUberXDist, minionUberXDist,
                    pMasterCand->_reqTime, masterDispatchTime, pMasterCand->_ETA, -1, pMasterCand->_reqOrig, pMasterCand->_reqDest, pctAddlDistMaster,
                    pMinionCand->_reqTime, pMinionCand->_reqTime, -1, -1, minionOrig, minionDest, pctAddlDistMinion,
                    masterSavings, minionSavings, avgSavings);    
            return pFeasMatch;
        } else {
            return NULL;
        }
    }
};

#endif	/* MODELUTILS_HPP */

