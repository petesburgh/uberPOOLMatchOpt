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
#include "Solution.hpp"
#include "RiderCountException.hpp"
#include <vector>
#include <set>

using namespace std;

class MultPickupSoln : public Solution {
            
public:
    
    struct RiderCountDistn {
        int numTrips_1Rider;
        int numTrips_2Riders;
        int numTrips_3Riders;
    };
        
    MultPickupSoln(int model, const time_t simStart, const time_t simEnd, const int totalReqs, const int totalDrivers, std::set<AssignedRoute*, AssignedRouteIndexComp> &assignedRoutes, std::set<Request*,ReqComp> &disqualifiedReqs, int maxRiders);
    virtual ~MultPickupSoln();
    
    void buildSolutionMetrics();
    const int getMaxRiders() const {return _maxRidersPerTrip;} 
    
    const RiderCountDistn * getRiderCountDistn() const { return &_riderCountDistn; }
            
    // getters for solution objects
    std::set<AssignedRoute*, AssignedRouteIndexComp> * getAssignedRoutes() { return &_allRoutesFromSolution; }
    std::set<AssignedRoute*, AssignedRouteIndexComp> * getMatchedRoutes() { return &_matchedRoutes; }
    std::set<AssignedRoute*, AssignedRouteIndexComp> * getUnmatchedRoutes() { return &_unmatchedRoutes; }
            
private:

    const int _maxRidersPerTrip;
    std::set<AssignedRoute*, AssignedRouteIndexComp> _allRoutesFromSolution;
    std::set<AssignedRoute*, AssignedRouteIndexComp> _matchedRoutes;
    std::set<AssignedRoute*, AssignedRouteIndexComp> _unmatchedRoutes;
    RiderCountDistn _riderCountDistn;
    
};

#endif	/* MULTPICKUPSOLN_HPP */

