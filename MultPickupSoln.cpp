/* 
 * File:   MultPickupSoln.cpp
 * Author: jonpetersen
 * 
 * Created on May 18, 2015, 1:36 PM
 */

#include "MultPickupSoln.hpp"

MultPickupSoln::MultPickupSoln(int model, const time_t simStart, const time_t simEnd, const int totalReqs, const int totalDrivers, std::set<AssignedRoute*, AssignedRouteIndexComp> &assignedRoutes, std::set<Request*,ReqComp> &disqualifiedReqs) :
    _model(model), _simStartTime(simStart), _simEndTime(simEnd), _totalRequests(totalReqs), _totalDrivers(totalDrivers), _allRoutesFromSolution(assignedRoutes), _disqualifiedRequests(disqualifiedReqs) {};

MultPickupSoln::~MultPickupSoln() {
}

void MultPickupSoln::buildSolutionMetrics() {
    cout << "\n\nbuildSolutionMetrics()\n\n" << endl;
}