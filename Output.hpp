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
#include "MultPickupSoln.hpp"
#include "ModelEnum.hpp"
#include "ModelRunner.hpp"
#include "SolnMaps.hpp"
#include "UserConfig.hpp"

#include <iostream>
#include <fstream>
#include <iomanip>      /* setw */
#include <sys/stat.h>   /* mkdir */

using namespace std;

class Solution;
class ModelRunner;

class Output {
public:
    Output(const std::string outputBasePath, const std::string outputExperimentPath, const bool printIndivMetrics);
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
    void printSolutionSummary( Solution * pSolution, std::string &outpath, std::string &modelname );
    void printMatchTripSummary(Solution * pSolution, std::string &outpath);  
    void printMultPickupMatchTripSummary(MultPickupSoln * pMultPickupSoln, std::string &outpath);
    void printUnmatchedTripsSummary(Solution * pSolution, std::string &outpath);
    void printMultPickupUnmatchedTripSummary(MultPickupSoln * pSolution, std::string &outpath);
    void printDisqualifiedRequestsSummary(Solution * pSolution, std::string &outpath);
    void printIndivSolnMetrics(Solution * pSolution, std::string &outpath);
    
    void printIndivdualRiderInconvenienceMetrics(Solution * pSolution, ofstream &outFile);
    void printIndividualTripOverlapMetrics(Solution * pSolution, ofstream &outFile);
    void printIndividualSavingsMetrics(Solution * pSolution, ofstream &outFile);
    void printIndividualWaitTimeofMatchMetrics(Solution * pSolution, ofstream &outFile);    
    
    const std::string getOutputBasePath() const { return _outputBasePath; }
    const std::string getOutputScenarioPath() const { return _outputScenarioPath; }
    
    void writeAndPrintInputs(DataContainer* pDataContainer, const bool printDebugFiles);
    void printSummaryOfDataInput(DataContainer * pDataContainer);
    
    void printSolutionSummaryMetricsForCurrSolutions(const int &experiment, const std::map<double, SolnMaps*> * pModelSolnMap );
    void printInputRequestsMetrics( std::ofstream &outFile, const std::map<double, SolnMaps*> * pModelSolnMap );    
    void printMatchRateMetrics( std::ofstream &outFile, std::string inputName, std::set<double> * pInputRange, const std::map<double, SolnMaps*> * pModelSolnMap );
    void printInconvenienceMetrics( std::ofstream &outFile, std::string inputName, std::set<double> * pInputRange, const std::map<double, SolnMaps*> * pModelSolnMap );
    void printOverlapMetrics( std::ofstream &outFile, std::string inputName, std::set<double> * pInputRange, const std::map<double, SolnMaps*> * pModelSolnMap );
    void printNumTripsMetrics( std::ofstream &outFile, std::string inputName, std::set<double> * pInputRange, const std::map<double, SolnMaps*> * pModelSolnMap );
    void printRiderSavingsMetrics( std::ofstream &outFile, std::string inputName, std::set<double> * pInputRange, const std::map<double, SolnMaps*> * pModelSolnMap );
    void printMatchWaitTimeMetrics( std::ofstream &outFile, std::string inputName, std::set<double> * pInputRange, const std::map<double, SolnMaps*> * pModelSolnMap );
    
    void setOutputScenarioPath(const std::string outputScenPath) { _outputScenarioPath = outputScenPath; }
        
    const std::string getOutputExperimentPath() const { return _outputExperimentPath; } 
    void setDataContainer(DataContainer * dataContainer) { pDataContainer = dataContainer; }    
    std::set<double> getKeyValues( const std::map<double, SolnMaps*> * pModelSolnMap );
    
    void printInputFileSummary(UserConfig * pUserConfig);

    
private:
    const std::string _outputBasePath;
    const bool _printIndivMetrics;
    std::string _outputExperimentPath;
    std::string _outputScenarioPath;
    DataContainer * pDataContainer;

};

#endif	/* OUTPUT_HPP */

