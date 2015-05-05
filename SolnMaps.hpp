/* 
 * File:   SolnMaps.hpp
 * Author: jonpetersen
 *
 * Created on May 5, 2015, 2:30 PM
 */

#ifndef SOLNMAPS_HPP
#define	SOLNMAPS_HPP

struct SolnMaps {
    std::map<ModelEnum, const int> numRequests_inputs;
    std::map<ModelEnum, double> matchRateMap;
    std::map<ModelEnum, double> inconvMap;
    std::map<ModelEnum, double> numTripsMap;
    double matchRate_FD_FDOptIn = -1.0;
    double matchRate_FD_nonFDOptIn = -1.0;
};

#endif	/* SOLNMAPS_HPP */

