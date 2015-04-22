/* 
 * File:   UFBW_fixed.hpp
 * Author: jonpetersen
 *
 * Created on April 3, 2015, 8:33 AM
 */

#ifndef UFBW_FIXED_HPP
#define	UFBW_FIXED_HPP

#include "Request.hpp"
#include "Driver.hpp"
#include "OpenTrip.hpp"
#include "Comparators.hpp"
#include "FeasibleMatch.hpp"
#include "Output.hpp"
#include "Solution.hpp"

#include "base/commandlineflags.h"
#include "base/logging.h"
#include "linear_solver/linear_solver.h"

#include <time.h>
#include <vector>
#include <set>
#include <map>
#include <queue>
#include <deque>
#include <iostream>

using namespace std;
using namespace operations_research;

class UFBW_fixed {
    
    friend class ModelUtils;

    struct MasterCand {
        
        MasterCand(const Event * reqEvent, const time_t reqTime, const LatLng reqOrig, const LatLng reqDest, const time_t eta, const time_t etd, const Driver * driver,
                   const Event * dispatchEvent, const Event * pickupEvent, const Event * drop, const int riderIx, const std::string riderID, const std::string riderTripUUID) :
            pReqEvent(reqEvent), _reqTime(reqTime), _reqOrig(reqOrig), _reqDest(reqDest), _ETA(eta), _ETD(etd), 
            pDriver(driver), pDispatchEvent(dispatchEvent), pPickupEvent(pickupEvent), pDropEvent(drop), _riderIndex(riderIx), _riderID(riderID), _riderTripUUID(riderTripUUID) {};
        
        const Event * pReqEvent;
        const time_t _reqTime;
        const LatLng _reqOrig;
        const LatLng _reqDest;                
        const time_t _ETA;
        const time_t _ETD;     
        const Driver * pDriver;
        const Event * pDispatchEvent;
        const Event * pPickupEvent;
        const Event * pDropEvent;
        const int _riderIndex;
        const std::string _riderID;
        const std::string _riderTripUUID;
        
        bool operator= (const MasterCand& other) {
            return (this->_riderIndex == other._riderIndex);
        }
    };
    
    struct MinionCand {
        MinionCand(const time_t reqTime, const LatLng reqOrig, const LatLng reqDest, const int riderIx, const std::string riderID, const std::string riderTripUUID) :
            _reqTime(reqTime), _reqOrig(reqOrig), _reqDest(reqDest), _riderIndex(riderIx), _riderID(riderID), _riderTripUUID(riderTripUUID) {};
            
        const time_t _reqTime;
        const LatLng _reqOrig;
        const LatLng _reqDest;
        const int _riderIndex;
        const std::string _riderID;
        const std::string _riderTripUUID;
        
        bool operator= (const MinionCand& other) {
            return (this->_riderIndex == other._riderIndex);
        }
        
    };
    
    struct MasterComp {
        bool operator()(MasterCand * pMaster1, MasterCand * pMaster2) const {
            return ( pMaster1->_ETD <= pMaster2->_ETD );
        }
    };
    
    struct MinionComp {
        bool operator()(MinionCand * pMinion1, MinionCand * pMinion2) const {
            return ( pMinion1->_reqTime <= pMinion2->_reqTime );
        }
    };
    
    struct MasterMinionMatchCand {
        enum DropType {
            FIFO,
            FILO
        };
        
        MasterMinionMatchCand( int ix, MasterCand * master, MinionCand * minion, double distKm, DropType dropType, const double avgSavings, bool isExt, FeasibleMatch * feasMatchPtr) : 
            _matchIndex(ix), pMaster(master), pMinion(minion), _pickupDistKm(distKm), _dropType(dropType), _avgSavings(avgSavings), _isExtended(isExt), pFeasMatch(feasMatchPtr)  {}; 
        const int _matchIndex;
        const MasterCand * pMaster;
        const MinionCand * pMinion;        
        const double _pickupDistKm;
        const DropType _dropType;
        const double _avgSavings;
        const bool _isExtended;
        FeasibleMatch * pFeasMatch;
        
        double _matchWeight;
        
        bool operator= (const MasterMinionMatchCand& other) {
            return ( this->_matchIndex < other._matchIndex );
        }
        
    };  
    
    struct MasterMinionMatchComp {
        bool operator()(MasterMinionMatchCand * pMatch1, MasterMinionMatchCand * pMatch2) const {
            return (pMatch1->_matchIndex < pMatch2->_matchIndex);
        }
    };  
    
    
 
public:
    UFBW_fixed(const time_t startTime, const time_t endTime, const int lenBatchWindow, const double maxMatchDistKm, const double minOverlapThreshold, std::set<Request*, ReqComp> initRequests, std::set<OpenTrip*, EtaComp> initOpenTrips, const std::set<Driver*, DriverIndexComp> * drivers);    
    virtual ~UFBW_fixed();
    
    bool solve(bool printDebugFiles, Output * pOutput, bool populateInitOpenTrips, bool printToScreen);
    std::set<AssignedTrip*, AssignedTripIndexComp> solveMatchingOptimization(std::set<UFBW_fixed::MasterMinionMatchCand*, UFBW_fixed::MasterMinionMatchComp> * pEligMatches, std::set<Request*, ReqComp> * pBatchRequests, bool printToScreen);
        
    std::set<OpenTrip*, EtaComp> convertExpiredUnmatchedReqsToOpenTrips(std::set<Request*, ReqComp> * pUnmatchedRequests, const time_t currTime);
    std::set<Request*, ReqComp> getRequestsInInterval(std::deque<Request*> &requestsToProcess, const time_t &currBatchStartTime, const time_t &currBatchEndTime);
    AssignedTrip * convertWaitingRequestToAssignedTrip(Request * pWaitingRequest);
    std::pair<std::set<MasterCand*, MasterComp>, std::set<MinionCand*, MinionComp> > generateCandidateMastersAndMinions(std::set<OpenTrip*, EtaComp> &openTrips, std::set<Request*, ReqComp> &currBatchRequests);
    std::set<UFBW_fixed::MasterMinionMatchCand*, UFBW_fixed::MasterMinionMatchComp> generateFeasibleMasterMinionMatches(std::set<UFBW_fixed::MasterCand*, UFBW_fixed::MasterComp> &candMasters, std::set<UFBW_fixed::MinionCand*, UFBW_fixed::MinionComp> &candMinions);
    void assignWeightsForMatchCandidates(std::set<MasterMinionMatchCand*, MasterMinionMatchComp> * pCandidateMatches);
    double computeEdgeWeightOfCurrCandidateMatch(MasterMinionMatchCand * pCurrMatchCand);
    
    std::queue<Request*> cloneRequests(std::set<Request*, ReqComp> requests);
    std::set<OpenTrip*, EtaComp> cloneOpenTrips(std::set<OpenTrip*, EtaComp> openTrips);
    
    // --------------
    //   opt model 
    // --------------
    std::map<MPVariable*,MasterMinionMatchCand*> buildModelVariables(MPSolver * pSolver, std::set<UFBW_fixed::MasterMinionMatchCand*, UFBW_fixed::MasterMinionMatchComp> * pEligMatches, std::map<const int, MPConstraint*> * pLeftNodeConstrMap, std::map<const int, MPConstraint*> * pRightNodeConstrMap, std::map<const int, MPConstraint*> * pAggregationConstrMap);        
    void printCurrentBatchMatchCandidates(std::ofstream &outFile, Request * pRequest, std::set<UFBW_fixed::MasterMinionMatchCand*, UFBW_fixed::MasterMinionMatchComp> * pEligMatches);
    void buildConstraints(MPSolver * pSolver, std::map<MPVariable*, UFBW_fixed::MasterMinionMatchCand*> * pMatchVarMap, std::set<UFBW_fixed::MasterMinionMatchCand*, UFBW_fixed::MasterMinionMatchComp>* pEligMatches, std::set<Request*, ReqComp> * pBatchRequests);    
    void buildDegreeConstraints(MPSolver * pSolver, std::map<MPVariable*,UFBW_fixed::MasterMinionMatchCand*> * pMatchVarMap, std::set<UFBW_fixed::MasterMinionMatchCand*, UFBW_fixed::MasterMinionMatchComp>* pEligMatches);
    void buildBatchRequestPairConstraints(MPSolver * pSolver, std::map<MPVariable*, UFBW_fixed::MasterMinionMatchCand*> * pVarMatchMap, std::set<UFBW_fixed::MasterMinionMatchCand*, UFBW_fixed::MasterMinionMatchComp>* pEligMatches, std::set<Request*, ReqComp> * pBatchRequests);
    void instantiateConstraints(MPSolver * pSolver, std::set<UFBW_fixed::MasterMinionMatchCand*, UFBW_fixed::MasterMinionMatchComp> * pEligMatches, std::set<Request*, ReqComp> * pBatchRequests, std::map<const int, MPConstraint*> * pRiderIxLeftDegreeConstrMap, std::map<const int, MPConstraint*> * pRiderIxRightDegreeConstrMap, std::map<const int, MPConstraint*> * pRiderIxAggregationConstrMap);
    void instantiateDegreeConstraints(MPSolver * pSolver, std::set<UFBW_fixed::MasterMinionMatchCand*, UFBW_fixed::MasterMinionMatchComp> * pEligMatches, std::map<const int, MPConstraint*> * pRiderIxLeftDegreeConstrMap, std::map<const int, MPConstraint*> * pRiderIxRighttDegreeConstrMap);
    void instantiateAggregationConstraints(MPSolver * pSolver, std::set<UFBW_fixed::MasterMinionMatchCand*,UFBW_fixed::MasterMinionMatchComp>* pEligMatches, std::set<Request*, ReqComp> * pBatchRequests, std::map<const int, MPConstraint*> * pRiderIxAggregationConstrMap);
    std::vector<MPVariable*> getIncidentEdgeVariablesForMaster(const UFBW_fixed::MasterCand * pMaster, std::map<UFBW_fixed::MasterMinionMatchCand*, MPVariable*> * pMatchVarMap);
    MPConstraint * getLeftNodeConstraint(MPSolver * pSolver, const UFBW_fixed::MasterCand * pMaster);
    MPConstraint * getRightNodeConstraint(MPSolver * pSolver, const UFBW_fixed::MinionCand * pMinion);
    std::vector<UFBW_fixed::MasterMinionMatchCand*> * getOptimalMatchings(MPSolver * pSolver, std::map<MPVariable*,MasterMinionMatchCand*> * pEdgeVariables, bool printToScreen);
    std::set<AssignedTrip*, AssignedTripIndexComp> buildAssignedTripsFromMatchingSolution(std::vector<UFBW_fixed::MasterMinionMatchCand*> * pOptMatchings);
    std::multimap<const int, time_t> getAllIndicesAssociatedWithMatchedRiders(std::set<AssignedTrip*, AssignedTripIndexComp> * pAssignedTrips);
    int removeMatchedOpenTrips_deprecated(std::multimap<const int, time_t> * pMatchedRiderReqTimeMap, std::set<OpenTrip*, EtaComp> * pOpenTrips);
    int removeMatchedOpenTrips(std::multimap<const int, time_t> * pMatchedRiderReqTimeMap, std::set<OpenTrip*, EtaComp> * pOpenTrips);    
    int removeMatchedFutureRequests(std::multimap<const int, time_t>* pMatchedRiderReqTimeMap, std::deque<Request*> * pFutureRequests, Request * pCurrRequestInQueue);
    std::set<int> getRidersThanMayBeMastersOrMinions(std::set<UFBW_fixed::MasterMinionMatchCand*, UFBW_fixed::MasterMinionMatchComp> * pEligMatches, std::set<Request*, ReqComp> * pBatchRequests);
    Request* getMinionRequest(const int riderIndex, std::set<Request*, ReqComp> * pRequests);
    MPConstraint * getRiderCopyAggregationConstraint(MPSolver * pSolver, const int masterIndex);
    
    
    void initEligMatchFile(std::ofstream &outFile);
    
    Solution * getSolution() { return pSolution; }
    
private:
    
    const time_t _startTime;
    const time_t _endTime;
    const int _lenBatchWindowInSec;
    const double _maxMatchDistInKm;
    const double _minOverlapThreshold;
    const std::set<Request*, ReqComp> _allRequests;
    const std::set<OpenTrip*, EtaComp> _initOpenTrips;
    const std::set<Driver*, DriverIndexComp> * _allDrivers;
    
    int _batchCounter;  // tracks the iterator of batch optimization
    
    // all assigned trips (matched & unmatched)
    std::set<AssignedTrip*, AssignedTripIndexComp> _assignedTrips;
   
    // disqualiafied requests (empty for now... needed for Solution constructor)
    std::set<Request*, ReqComp> _disqualifiedRequests;
    
    Solution * pSolution;
};

#endif	/* UFBW_FIXED_HPP */

