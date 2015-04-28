/* 
 * File:   MinionCand.hpp
 * Author: jonpetersen
 *
 * Created on April 23, 2015, 7:54 PM
 */

#ifndef MINIONCAND_HPP
#define	MINIONCAND_HPP

#include "LatLng.hpp"
#include <iostream>
#include <time.h>

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

#endif	/* MINIONCAND_HPP */

