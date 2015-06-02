/* 
 * File:   RiderMetrics.hpp
 * Author: jonpetersen
 *
 * Created on May 27, 2015, 11:57 AM
 */

#ifndef RIDERMETRICS_HPP
#define	RIDERMETRICS_HPP

#include "Request.hpp"
#include <iostream>

using namespace std;

struct RiderMetrics {
    RiderMetrics(const Request * req, double xDist, double pDist, double cost, double svg, double incnv) :
    pRequest(req), uberXDist(xDist), pooledDist(pDist), pooledCost(cost), savings(svg), inconv(incnv){};
    
    const Request * pRequest;
    const double uberXDist;
    const double pooledDist;
    const double pooledCost;
    const double savings;
    const double inconv;
};



#endif	/* RIDERMETRICS_HPP */

