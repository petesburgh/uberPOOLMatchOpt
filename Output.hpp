/* 
 * File:   Output.hpp
 * Author: jonpetersen
 *
 * Created on March 23, 2015, 2:06 PM
 */

#ifndef OUTPUT_HPP
#define	OUTPUT_HPP

#include "DataContainer.hpp"
#include "Utility.hpp"
#include "Solution.hpp"

#include <iostream>
#include <fstream>
#include <iomanip>      /* setw */
#include <sys/stat.h>   /* mkdir */

using namespace std;

class Output {
public:
    Output(DataContainer * dataContainer, const std::string outputBasePath, const std::string outputScenarioPath);
    virtual ~Output();
    
    // print debug files
    void printSummaryInfo();
    void printTripSnapshot();
    void printPoolRiders();
    void printDrivers();
    void printRequestsInSim();
    void printAllRequestsInSim();
    void printInitOpenTrips();
    
    // print solution
    void printSolution(Solution * pSolution);
    void printDataSummary(ofstream &outFile, Solution * pSolution);
    void printSolutionSummary(ofstream &outfile, Solution * pSolution);
    void printMatchTripsSummary(ofstream &outFile, Solution * pSolution);
    void printUnmatchedTripsSummary(ofstream &outFile, Solution * pSolution);
    void printDisqualifiedRequestsSummary(ofstream &outFile, Solution * pSolution);
    
    const std::string getOutputBasePath() const { return _outputBasePath; }
    const std::string getOutputScenarioPath() const { return _outputScenarioPath; }
    
private:
    
    const std::string _outputBasePath;
    const std::string _outputScenarioPath;
    DataContainer * pDataContainer;

};

#endif	/* OUTPUT_HPP */

