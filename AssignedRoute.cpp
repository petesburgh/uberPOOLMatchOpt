/* 
 * File:   AssignedRoute.cpp
 * Author: jonpetersen
 * 
 * Created on May 14, 2015, 4:13 PM
 */

#include "AssignedRoute.hpp"

AssignedRoute::AssignedRoute(Route * rt) : pRoute(rt) {
    _index = pRoute->getRouteIndex();
}

AssignedRoute::~AssignedRoute() {
}

