/* 
 * File:   MasterMinionMatchCand.hpp
 * Author: jonpetersen
 *
 * Created on April 23, 2015, 8:07 PM
 */

#ifndef MASTERMINIONMATCHCAND_HPP
#define	MASTERMINIONMATCHCAND_HPP

#include "MasterCand.hpp"
#include "MinionCand.hpp"
#include "FeasibleMatch.hpp"
#include <iostream>

struct MasterMinionMatchCand {
    enum DropType {
        FIFO,
        FILO
    };

    MasterMinionMatchCand( int ix, MasterCand * master, MinionCand * minion, double distKm, DropType dropType, const double avgSavings, bool isExt, FeasibleMatch * feasMatchPtr) : 
        _matchIndex(ix), pMaster(master), pMinion(minion), _pickupDistKm(distKm), _dropType(dropType), _avgSavings(avgSavings), _isExtended(isExt), pFeasMatch(feasMatchPtr) {}; 
    const int _matchIndex;
    const MasterCand * pMaster;
    const MinionCand * pMinion;        
    const double _pickupDistKm;
    const DropType _dropType;
    const double _avgSavings;
    const bool _isExtended;
    FeasibleMatch * pFeasMatch;
    
    double _matchWeight;

    bool operator= (const MasterMinionMatchCand& other) {
        return ( this->_matchIndex < other._matchIndex );
    }

};


#endif	/* MASTERMINIONMATCHCAND_HPP */

