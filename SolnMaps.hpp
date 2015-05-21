/* 
 * File:   SolnMaps.hpp
 * Author: jonpetersen
 *
 * Created on May 5, 2015, 2:30 PM
 */

#ifndef SOLNMAPS_HPP
#define	SOLNMAPS_HPP

#include "ModelEnum.hpp"


struct SolnMaps {
    std::map<ModelEnum, const int> numRequests_inputs;
    std::map<ModelEnum, double> matchRateMap;
    std::map<ModelEnum, double> inconvMap;
    std::map<ModelEnum, double> numTripsMap;
    std::map<ModelEnum, double> avgSavingsAllMatchedRiders;
    std::map<ModelEnum, double> avgSavingsMasters;
    std::map<ModelEnum, double> avgSavingsMinions;
    std::map<ModelEnum, double> avgOverlapDist;
    std::map<ModelEnum, double> avgPctOverlap_Trip;
    std::map<ModelEnum, double> avgPctOverlap_ALL;
    std::map<ModelEnum, double> avgPctOverlap_Masters;
    std::map<ModelEnum, double> avgPctOverlap_Minions;
    std::map<ModelEnum, double> avgWaitTimeOfMatchAllRiders;
    std::map<ModelEnum, double> avgWaitTimeOfMatchMasters;
    std::map<ModelEnum, double> avgWaitTimeOfMatchMinions;
           
    double matchRate_FD_FDOptIn = -1.0;
    double matchRate_FD_nonFDOptIn = -1.0;
    
    // ---------------------------------
    //   individual metrics (by rider)
    // ---------------------------------
    
    // overlap
    std::map<ModelEnum, std::vector<double> > indivMatchOverlapDistances;  
    std::map<ModelEnum, std::vector<double> > indivPctOverlap_Trip;
    std::map<ModelEnum, std::vector<double> > indivPctTripOverlap_ALL;
    std::map<ModelEnum, std::vector<double> > indivPctTripOverlap_Masters;
    std::map<ModelEnum, std::vector<double> > indivPctTripOverlap_Minions;
    
    // inconvenience
    std::map<ModelEnum, std::vector<double> > indivPctInconvenience_ALL;
    std::map<ModelEnum, std::vector<double> > indivPctInconvenience_Masters;
    std::map<ModelEnum, std::vector<double> > indivPctInconvenience_Minions;
    
    // savings
    std::map<ModelEnum, std::vector<double> > indivPctSavings_ALL;
    std::map<ModelEnum, std::vector<double> > indivPctSavings_Masters;
    std::map<ModelEnum, std::vector<double> > indivPctSavings_Minions;
    
    // wait times
    std::map<ModelEnum, std::vector<int> > indivWaitTimeToMatch_all;
    std::map<ModelEnum, std::vector<int> > indivWaitTimeToMatch_Masters;
    std::map<ModelEnum, std::vector<int> > indivWaitTimeToMatch_Minions;
           
    // flexible departure match rates
    std::vector<double> indivFlexDepMatchRate_FDOptIns;
    std::vector<double> indivFlexDepMatchRate_FDNonOptIns;    
};

#endif	/* SOLNMAPS_HPP */

