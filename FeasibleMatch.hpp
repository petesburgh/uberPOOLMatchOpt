/* 
 * File:   FeasibleMatch.hpp
 * Author: jonpetersen
 *
 * Created on March 26, 2015, 8:25 PM
 */

#ifndef FEASIBLEMATCH_HPP
#define	FEASIBLEMATCH_HPP

#include "LatLng.hpp"
#include <iostream>
#include <time.h>

using namespace std;

struct FeasibleMatch {
    FeasibleMatch(const Driver * driver, std::string masterId, const int masterIndex, std::string minionId, const int minionIndex, bool fixedDrop, const bool masterPickedUp,
                  double dMinPick, double sharedDist, double distToDrop, double distMast, double distMin, double xMaster, double xMinion,
                  time_t masterReq, time_t masterDispatch, time_t masterPickup, time_t masterDrop, LatLng masterOrig,  LatLng masterDest, const double pctAddlDistMaster,
                  time_t minionReq, time_t minionDispatch, time_t minionPickup, time_t minionDrop, LatLng minionOrig, LatLng minionDest, const double pctAddlDistMinion,
                  double masterSavings, double minionSavings, double avgSavings) : 
        pDriver(driver), _masterId(masterId), _masterIndex(masterIndex), _minionId(minionId), _minionIndex(minionIndex), _fixedDropoff(fixedDrop), _masterPickedUpAtTimeOfMatch(masterPickedUp),
        _distToMinionPickup(dMinPick), _sharedDistance(sharedDist), _distFromFirstToSecondDrop(distToDrop), _totalDistanceForMaster(distMast), _totalDistanceForMinion(distMin), _uberXDistanceForMaster(xMaster), _uberXDistanceForMinion(xMinion),
        _masterRequest(masterReq), _masterDispatch(masterDispatch), _masterPickup(masterPickup), _masterDrop(masterDrop), _masterOrig(masterOrig), _masterDest(masterDest), _pctAddlDistMaster(pctAddlDistMaster),
        _minionRequest(minionReq), _minionDispatch(minionDispatch), _minionPickup(minionPickup), _minionDrop(minionDrop), _minionOrig(minionOrig), _minionDest(minionDest), _pctAddlDistMinion(pctAddlDistMinion),
        _masterSavings(masterSavings), _minionSavings(minionSavings), _avgSavings(avgSavings) {};
    
    // trip basics
    const Driver * pDriver;
    const std::string _masterId;
    const std::string _minionId;
    const bool _fixedDropoff;
    const bool _masterPickedUpAtTimeOfMatch;
    const double _masterSavings;
    const double _minionSavings;
    const double _avgSavings;
    
    // trip metrics
    const double _distToMinionPickup;
    const double _sharedDistance;
    const double _distFromFirstToSecondDrop;
    const double _totalDistanceForMaster;
    const double _totalDistanceForMinion;
    const double _uberXDistanceForMaster;
    const double _uberXDistanceForMinion;
    
    // master info
    const int _masterIndex;
    const time_t _masterRequest;
    const time_t _masterDispatch;
    const time_t _masterPickup;
    const time_t _masterDrop;
    LatLng _masterOrig;
    LatLng _masterDest;
    const double _pctAddlDistMaster;
    
    // minion info  
    const int _minionIndex;
    const time_t _minionRequest;
    const time_t _minionDispatch;
    const time_t _minionPickup;
    const time_t _minionDrop;
    LatLng _minionOrig;
    LatLng _minionDest;
    const double _pctAddlDistMinion;
    
};

#endif	/* FEASIBLEMATCH_HPP */

