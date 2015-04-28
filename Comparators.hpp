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
#include "MasterCand.hpp"
#include "MinionCand.hpp"
#include "MasterMinionMatchCand.hpp"
#include <iostream>
#include <time.h>

using namespace std;

struct ReqComp {
    bool operator()(const Request* req1, const Request* req2) const {
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

struct MasterMinionMatchComp {
    bool operator()(MasterMinionMatchCand * pMatch1, MasterMinionMatchCand * pMatch2) const {
        return (pMatch1->_matchIndex < pMatch2->_matchIndex);
    }
};  


#endif	/* COMPARATORS_HPP */

