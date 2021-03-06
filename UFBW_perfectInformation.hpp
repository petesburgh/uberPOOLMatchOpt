/* 
 * File:   UFBW_perfectInformation.hpp
 * Author: jonpetersen
 *
 * Created on April 23, 2015, 8:23 PM
 */

#ifndef UFBW_PERFECTINFORMATION_HPP
#define	UFBW_PERFECTINFORMATION_HPP

#include "Request.hpp"
#include "Driver.hpp"
#include "OpenTrip.hpp"
#include "MasterCand.hpp"
#include "MinionCand.hpp"
#include "MasterMinionMatchCand.hpp"
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

class UFBW_perfectInformation {
public:
    
    friend class ModelUtils;

public:
    UFBW_perfectInformation(const time_t startTime, const time_t endTime, const int lenBatchWindow, const double maxMatchDistKm, const double minOverlapThreshold, std::set<Request*, ReqComp> initRequests, std::set<OpenTrip*, EtdComp> initOpenTrips, const std::set<Driver*, DriverIndexComp> * drivers, const bool inclMinionPickupInSavingsConstr);    
    virtual ~UFBW_perfectInformation();
    
    bool solve(bool printDebugFiles, Output * pOutput, bool populateInitOpenTrips, bool printToScreen);
    std::set<AssignedTrip*, AssignedTripIndexComp> solveMatchingOptimization(std::set<MasterMinionMatchCand*, MasterMinionMatchComp> * pEligMatches, std::set<Request*, ReqComp> * pBatchRequests, bool printToScreen);
        
    std::set<Request*, ReqComp> getRequestsInInterval(std::deque<Request*> &requestsToProcess, const time_t &currBatchStartTime, const time_t &currBatchEndTime);
    std::pair<std::set<MasterCand*, MasterComp>, std::set<MinionCand*, MinionComp> > generateCandidateMastersAndMinions(std::set<OpenTrip*, EtdComp> &initOpenTrips, std::set<Request*, ReqComp> &currBatchRequests);
    std::set<MasterMinionMatchCand*, MasterMinionMatchComp> generateFeasibleMasterMinionMatches(const time_t &reqTimeOfFirstReqInCurrBatch, std::set<MasterCand*, MasterComp> &candMasters, std::set<MinionCand*, MinionComp> &candMinions, std::set<OpenTrip*, EtdComp> * initOpenTrips);
    void assignWeightsForMatchCandidates(std::set<MasterMinionMatchCand*, MasterMinionMatchComp> * pCandidateMatches);
    double computeEdgeWeightOfCurrCandidateMatch(MasterMinionMatchCand * pCurrMatchCand);
    bool checkIfCandidateMatchIsTimeEligible(MasterCand * pMaster, MinionCand *pMinion, std::set<OpenTrip*, EtdComp> * pInitOpenTrips);
    bool checkIfCandMasterIsInitialOpenTrip(MasterCand * pMaster, std::set<OpenTrip*, EtdComp> * pInitOpenTrips);
    
    int convertUnmatchedRequestsToOpenTrips(std::set<AssignedTrip*, AssignedTripIndexComp> * pMatchedTrips, std::set<Request*, ReqComp> * pRequests, std::set<AssignedTrip*, AssignedTripIndexComp> * pAssignedTrips);
    
    std::queue<Request*> cloneRequests(std::set<Request*, ReqComp> requests);
    std::set<OpenTrip*, EtdComp> cloneOpenTrips(std::set<OpenTrip*, EtdComp> openTrips);
    
    // --------------
    //   opt model 
    // --------------
    std::map<MPVariable*,MasterMinionMatchCand*> buildModelVariables(MPSolver * pSolver, std::set<MasterMinionMatchCand*, MasterMinionMatchComp> * pEligMatches, std::map<const int, MPConstraint*> * pLeftNodeConstrMap, std::map<const int, MPConstraint*> * pRightNodeConstrMap, std::map<const int, MPConstraint*> * pAggregationConstrMap);        
    void printCurrentBatchMatchCandidates(std::ofstream &outFile, Request * pRequest, std::set<MasterMinionMatchCand*, MasterMinionMatchComp> * pEligMatches);
    void buildConstraints(MPSolver * pSolver, std::map<MPVariable*, MasterMinionMatchCand*> * pMatchVarMap, std::set<MasterMinionMatchCand*, MasterMinionMatchComp>* pEligMatches, std::set<Request*, ReqComp> * pBatchRequests);    
    void buildDegreeConstraints(MPSolver * pSolver, std::map<MPVariable*,MasterMinionMatchCand*> * pMatchVarMap, std::set<MasterMinionMatchCand*, MasterMinionMatchComp>* pEligMatches);
    void buildBatchRequestPairConstraints(MPSolver * pSolver, std::map<MPVariable*, MasterMinionMatchCand*> * pVarMatchMap, std::set<MasterMinionMatchCand*, MasterMinionMatchComp>* pEligMatches, std::set<Request*, ReqComp> * pBatchRequests);
    void instantiateConstraints(MPSolver * pSolver, std::set<MasterMinionMatchCand*, MasterMinionMatchComp> * pEligMatches, std::set<Request*, ReqComp> * pBatchRequests, std::map<const int, MPConstraint*> * pRiderIxLeftDegreeConstrMap, std::map<const int, MPConstraint*> * pRiderIxRightDegreeConstrMap, std::map<const int, MPConstraint*> * pRiderIxAggregationConstrMap);
    void instantiateDegreeConstraints(MPSolver * pSolver, std::set<MasterMinionMatchCand*, MasterMinionMatchComp> * pEligMatches, std::map<const int, MPConstraint*> * pRiderIxLeftDegreeConstrMap, std::map<const int, MPConstraint*> * pRiderIxRighttDegreeConstrMap);
    void instantiateAggregationConstraints(MPSolver * pSolver, std::set<MasterMinionMatchCand*,MasterMinionMatchComp>* pEligMatches, std::set<Request*, ReqComp> * pBatchRequests, std::map<const int, MPConstraint*> * pRiderIxAggregationConstrMap);
    std::vector<MPVariable*> getIncidentEdgeVariablesForMaster(const MasterCand * pMaster, std::map<MasterMinionMatchCand*, MPVariable*> * pMatchVarMap);
    MPConstraint * getLeftNodeConstraint(MPSolver * pSolver, const MasterCand * pMaster);
    MPConstraint * getRightNodeConstraint(MPSolver * pSolver, const MinionCand * pMinion);
    std::vector<MasterMinionMatchCand*> * getOptimalMatchings(MPSolver * pSolver, std::map<MPVariable*,MasterMinionMatchCand*> * pEdgeVariables, bool printToScreen);
    std::set<AssignedTrip*, AssignedTripIndexComp> buildAssignedTripsFromMatchingSolution(std::vector<MasterMinionMatchCand*> * pOptMatchings);
    std::multimap<const int, time_t> getAllIndicesAssociatedWithMatchedRiders(std::set<AssignedTrip*, AssignedTripIndexComp> * pAssignedTrips);
    int removeMatchedOpenTrips_deprecated(std::multimap<const int, time_t> * pMatchedRiderReqTimeMap, std::set<OpenTrip*, EtdComp> * pOpenTrips);
    int removeMatchedOpenTrips(std::multimap<const int, time_t> * pMatchedRiderReqTimeMap, std::set<OpenTrip*, EtdComp> * pOpenTrips);    
    int removeMatchedFutureRequests(std::multimap<const int, time_t>* pMatchedRiderReqTimeMap, std::deque<Request*> * pFutureRequests, Request * pCurrRequestInQueue);
    std::set<int> getRidersThanMayBeMastersOrMinions(std::set<MasterMinionMatchCand*, MasterMinionMatchComp> * pEligMatches, std::set<Request*, ReqComp> * pBatchRequests);
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
    const std::set<OpenTrip*, EtdComp> _initOpenTrips;
    const std::set<Driver*, DriverIndexComp> * _allDrivers;
    
    int _batchCounter;  // tracks the iterator of batch optimization
    
    // all assigned trips (matched & unmatched)
    std::set<AssignedTrip*, AssignedTripIndexComp> _assignedTrips;
   
    // disqualified requests (empty for now... needed for Solution constructor)
    std::set<Request*, ReqComp> _disqualifiedRequests;
    
    Solution * pSolution;
    
    const bool _inclDistMinionPickupExtMatches;
};

#endif	/* UFBW_PERFECTINFORMATION_HPP */

