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
#include "FlexDepSolution.hpp"
#include "ModelEnum.hpp"
#include "ModelRunner.hpp"
#include "SolnMaps.hpp"

#include <iostream>
#include <fstream>
#include <iomanip>      /* setw */
#include <sys/stat.h>   /* mkdir */

using namespace std;

class Solution;
class ModelRunner;

class Output {
public:
    Output(const std::string outputBasePath, const std::string outputExperimentPath);
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
    void printSolution(Solution * pSolution, const ModelEnum &model);
    //void printDataSummary(Solution * pSolution, std::string &outpath);
    void printSolutionSummary(Solution * pSolution, std::string &outpath, std::string &modelname );
    void printMatchTripsSummary(Solution * pSolution, std::string &outpath);
    void printUnmatchedTripsSummary(Solution * pSolution, std::string &outpath);
    void printDisqualifiedRequestsSummary(Solution * pSolution, std::string &outpath);
    
    const std::string getOutputBasePath() const { return _outputBasePath; }
    const std::string getOutputScenarioPath() const { return _outputScenarioPath; }
    
   /* void printSolutionSummary_FD(FlexDepSolution * pFlexDepSoln, std::string &outpath, std::string &modelname );
    void printMatchTripsSummary_FD(FlexDepSolution * pFDSolution, std::string &outpath);
    void printUnmatchedTripsSummary_FD(FlexDepSolution * pFDSolution, std::string &outpath); */
    
    void writeAndPrintInputs(DataContainer* pDataContainer, const bool printDebugFiles);
    void printSummaryOfDataInput(DataContainer * pDataContainer);
    
    void printSolutionSummaryMetricsForCurrSolutions(const int &experiment, const std::map<double, SolnMaps*> * pModelSolnMap );
    void printInputRequestsMetrics( std::ofstream &outFile, const std::map<double, SolnMaps*> * pModelSolnMap );    
    void printMatchRateMetrics( std::ofstream &outFile, std::string inputName, std::set<double> * pInputRange, const std::map<double, SolnMaps*> * pModelSolnMap );
    void printInconvenienceMetrics( std::ofstream &outFile, std::string inputName, std::set<double> * pInputRange, const std::map<double, SolnMaps*> * pModelSolnMap );
    void printNumTripsMetrics( std::ofstream &outFile, std::string inputName, std::set<double> * pInputRange, const std::map<double, SolnMaps*> * pModelSolnMap );
    
    void setOutputScenarioPath(const std::string outputScenPath) { _outputScenarioPath = outputScenPath; }
    
    
    const std::string getOutputExperimentPath() const { return _outputExperimentPath; }
 
    void setDataContainer(DataContainer * dataContainer) { pDataContainer = dataContainer; }
    
    std::set<double> getKeyValues( const std::map<double, SolnMaps*> * pModelSolnMap );
    
    
private:
    
    const std::string _outputBasePath;
    std::string _outputExperimentPath;
    std::string _outputScenarioPath;
    DataContainer * pDataContainer;

};

#endif	/* OUTPUT_HPP */

