/* 
 * File:   AssignedRoute.hpp
 * Author: jonpetersen
 *
 * Created on May 14, 2015, 4:13 PM
 */

#ifndef ASSIGNEDROUTE_HPP
#define	ASSIGNEDROUTE_HPP

#include "Route.hpp"

#include <iostream>
#include <time.h>
#include <vector>

using namespace std;

class AssignedRoute {
public:
    AssignedRoute(Route * rt);
    virtual ~AssignedRoute();
    
    // getters
    const int getIndex() const { return _index; }
    const int getNumRidersInRoute() const { return pRoute->getNumMatchedRiders(); }
    
private:

    const Route * pRoute;    
    int _index;
};

#endif	/* ASSIGNEDROUTE_HPP */

