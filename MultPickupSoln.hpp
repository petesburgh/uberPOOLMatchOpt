/* 
 * File:   MultPickupSoln.hpp
 * Author: jonpetersen
 *
 * Created on May 18, 2015, 1:36 PM
 */

#ifndef MULTPICKUPSOLN_HPP
#define	MULTPICKUPSOLN_HPP

#include "Driver.hpp"
#include "AssignedRoute.hpp"
#include "Comparators.hpp"
#include "ModelEnum.hpp"
#include <vector>
#include <set>

using namespace std;

class MultPickupSoln {
public:
    MultPickupSoln(int model, const time_t simStart, const time_t simEnd, const int totalReqs, const int totalDrivers, std::set<AssignedRoute*, AssignedRouteIndexComp> &assignedRoutes, std::set<Request*,ReqComp> &disqualifiedReqs);
    virtual ~MultPickupSoln();
    
    void buildSolutionMetrics();
    
private:
    const int _model;
    const time_t _simStartTime;
    const time_t _simEndTime;
    const int _totalRequests;
    const int _totalDrivers;
    
    const std::set<AssignedRoute*, AssignedRouteIndexComp> _allRoutesFromSolution;
    
    std::set<AssignedRoute*, AssignedRouteIndexComp> _matchedRoutes;
    std::set<AssignedRoute*, AssignedRouteIndexComp> _unmatchedRoutes;
    std::set<Request*, ReqComp> _disqualifiedRequests;    
};

#endif	/* MULTPICKUPSOLN_HPP */

