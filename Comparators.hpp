/* 
 * File:   Comparators.hpp
 * Author: jonpetersen
 *
 * Created on March 25, 2015, 9:41 AM
 */

#ifndef COMPARATORS_HPP
#define	COMPARATORS_HPP

#include "Request.hpp"
#include "OpenTrip.hpp"
#include "AssignedTrip.hpp"
#include <iostream>
#include <time.h>

using namespace std;

struct ReqComp {
    bool operator()(const Request* req1, const Request* req2) const {
        //return req1->getReqTime() < req2->getReqTime();
        return req1->getReqTime() <= req2->getReqTime();
    }
};

struct EtaComp {
    bool operator()(const OpenTrip* openTrip1, const OpenTrip* openTrip2) const {
        //return openTrip1->getETD() < openTrip2->getETD();
        return openTrip1->getETD() <= openTrip2->getETD();
    }
};

struct AssignedTripIndexComp {
    bool operator()(const AssignedTrip* trip1, const AssignedTrip* trip2) const {
        return trip1->getIndex() < trip2->getIndex();
    }
};

struct RiderIndexComp {
    bool operator()(const Rider * pRider1, const Rider * pRider2) const {
        return pRider1->getIndex() < pRider2->getIndex();
    }
};

struct DriverIndexComp {
    bool operator()(const Driver * pDriver1, const Driver * pDriver2) const {
        return pDriver1->getIndex() < pDriver2->getIndex();
    }
};


#endif	/* COMPARATORS_HPP */

