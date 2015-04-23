/* 
 * File:   main.cpp
 * Author: jonpetersen
 *
 * Created on March 18, 2015, 11:04 AM
 */

#include <iostream>
#include <iterator>
#include "DataContainer.hpp"
#include "Output.hpp"
//#include "MitmModel.hpp"
#include "MitmModel_new.hpp"
#include "UFBW_fixed.hpp"

#include <iterator>
#include <deque>

using namespace std;

enum {
        DEFAULTVALUES,
        OPTIN,
        BATCHWINDOW,
        PICKUP,
        SAVINGSRATE
     } Scenario;
    
struct ParameterSet {
    int           _iteration;
    double        _optInRate;
    int           _batchWindowLengthInSec;
    double        _maxPickupDistInKm;
    double        _minDiscount;
    std::string   _paramSetStr;
};

struct DataInputValues {
    
    DataInputValues(const std::string input, const std::string inputBase, const std::string cvsFname, const std::string timeline, const int simLengthMin, const bool popInitCandidates) :
        _inputPath(input), _inputBasePath(inputBase), _cvsFilename(cvsFname), _timelineStr(timeline), _simLengthInMinutes(simLengthMin),  _populateInitOpenTrips(popInitCandidates) {};
    
    const std::string   _inputPath;
    const std::string   _inputBasePath;
    const std::string   _cvsFilename;
    const std::string   _timelineStr;
    const int           _simLengthInMinutes;
    const bool          _populateInitOpenTrips;
};

struct DataOutputValues {
    DataOutputValues(const std::string outputBasePath, const bool printDebugFiles, const bool printToScreen) : 
        _outputBasePath(outputBasePath), _printDebugFiles(printDebugFiles), _printToScreen(printToScreen) {};
    
    const std::string _outputBasePath;
    std::string _outputScenarioPath;
    const bool _printDebugFiles;
    const bool _printToScreen;    
};

struct DefaultValues { 
    DefaultValues(const double optIn, const int batchWindow, const double maxPickupDist, const double minSavings) : 
        _optInRate(optIn), _batchWindowLengthInSec(batchWindow), _maxPickupDistance(maxPickupDist), _minSavings(minSavings) {};
    
    const double _optInRate;
    const int    _batchWindowLengthInSec;
    const double _maxPickupDistance;
    const double _minSavings;
};

struct SolnMaps {
    std::map<double, double> matchRate_MITM;
    std::map<double, double> matchRate_UFBW;
    
    std::map<double, double> inconv_MITM;
    std::map<double, double> inconv_UFBW;
    
    std::map<double, double> numTrips_MITM;
    std::map<double, double> numTrip_UFBW;
};

SolnMaps * runModels_defaultValues             ( bool runMITMModel, bool runUFBW_seqPickups, DataInputValues * pDataInput, DataOutputValues * pDataOutput, DefaultValues * pDefaultValues);
SolnMaps * runModels_optInScenarios            ( bool runMITMModel, bool runUFBW_seqPickups, DataInputValues * pDataInput, DataOutputValues * pDataOutput, DefaultValues * pDefaultValues, std::vector<double> * pOptInValues);
SolnMaps * runModels_batchWindowScenarios      ( bool runMITMModel, bool runUFBW_seqPickups, DataInputValues * pDataInput, DataOutputValues * pDataOutput, DefaultValues * pDefaultValues, std::vector<int> * pBatchWindowValues );
SolnMaps * runModels_maxPickupDistanceScenarios( bool runMITMModel, bool runUFBW_seqPickups, DataInputValues * pDataInput, DataOutputValues * pDataOutput, DefaultValues * pDefaultValues, std::vector<double> * pMaxPickupValues );
SolnMaps * runModels_minSavingsScenarios ( bool runMITMModel, bool runUFBW_seqPickups, DataInputValues * pDataInput, DataOutputValues * pDataOutput, DefaultValues * pDefaultValues, std::vector<double> * pMinSavingsValues );

ParameterSet generateCurrentParameterSet(int,int,double,int,double,double,std::vector<double>*, std::vector<int>*, std::vector<double>*, std::vector<double>*);

std::vector<int>    defineBatchWindowRange();
std::vector<double> defineOptInRange();
std::vector<double> defineMaxPickupDistRange();
std::vector<double> defineMinPoolDiscountRange();

//Output * pOutput;
void printBanner(ParameterSet * pParamSet, int N);
void printSummaryOfDataInput(DataContainer*);
void printDriverInfo(DataContainer*);
void printRiderInfo(const std::set<Rider*, RiderIndexComp>*);
void writeAndPrintInputs(DataContainer*, Output*);

void printSolutionMetricsForCurrExperiment(SolnMaps * pSolnMaps, int currExperiment, const std::string outputBasePath);
void printMatchRateMetrics(ofstream &outFile, std::string inputName, std::map<double,double> * pMatchRateMap_MITM, std::map<double,double> *pMatchRateMap_UFBW);
void printInconvenienceMetrics(ofstream &outFile, std::string inputName, std::map<double,double> * pInconvMap_MITM, std::map<double,double> * pInconvMap_UFBW);
void printNumTripsMetrics(ofstream &outFile, std::string inputName, std::map<double,double> * pNumTripsMap_MITM, std::map<double,double> * pNumTripsMap_UFBW);
/*
 *   MAIN
 */
int main(int argc, char** argv) {
             
    // specify singleton inputs // TODO: move to text file
    const std::string inputPath      = "/Users/jonpetersen/Documents/uberPOOL/testEnv/";
    const std::string outputBasePath = "/Users/jonpetersen/Documents/uberPOOL/testEnv/Output/";
    const std::string csvFilename    = "trips-SF-2015-04-13-1600-1700-uberX.csv";    
    const std::string timelineStr    = "2015-04-13 16:00:00";
    const int simLengthInMin         = 60;
    const bool printDebugFiles       = true;
    const bool printToScreen         = false;
    const bool populateInitOpenTrips = false;
    
    DataInputValues  dataInput(inputPath, outputBasePath, csvFilename, timelineStr, simLengthInMin, populateInitOpenTrips);
    DataOutputValues dataOutput(outputBasePath,printDebugFiles,printToScreen);
        
    // specify DEFAULT values
    const double default_optInRate = 0.40;
    const int    default_upFrontBatchWindowInSec = 30;
    const double default_maxMatchDistInKm = 3.0;
    const double default_minPoolDiscount = 0.2;
    
    
    DefaultValues defaultInputs(default_optInRate,default_upFrontBatchWindowInSec,default_maxMatchDistInKm,default_minPoolDiscount);
                
    // specify RANGES to iterate experiments 
            // opt-in ranges: 0.20, 0.30, 0.40, 0.50, 0.60, 0.70, 0.80, 0.90, 1.0
    std::vector<double> range_optInRate = defineOptInRange();
    
        // batch window values: 15, 30, 45, 60, 75, 90, 120, 150, 300, 600
    std::vector<int>    range_upFrontBatchWindowInSec = defineBatchWindowRange();
    
        // max pickup range (km): 0.5, 1.0, 1.5, 2.0, 2.5, 3.0, 3.5, 4.0, 4.5, 5.0, 5.5, 6.0, 10.0
    std::vector<double> range_maxMatchDistInKm = defineMaxPickupDistRange();
    
        // max pool discount for master: 0.05, 0.10, 0.15, 0.20, 0.25, 0.30, 0.35, 0.40, 0.45, 0.50
    std::vector<double> range_minPoolDiscount = defineMinPoolDiscountRange();
        
    // TYPE OF TESTS TO RUN
    const bool runMITMModel        = true;
    const bool runUFBW_seqPickups  = true;
    const bool runUFBW_flexPickups = false; 
    
    // SPECIFY TYPE OF EXPERIMENT
    int experiment = DEFAULTVALUES;  //DEFAULTVALUES, OPTIN, BATCHWINDOW, PICKUP, SAVINGSRATE
    
    // instantiate SolnMaps to track solution
    SolnMaps * pSolnMaps = NULL;
                
    switch ( experiment ) {
        case DEFAULTVALUES : 
        {
            std::cout << "\n\nSOLVING DEFAULT SCENARIOS...\n\n" << std::endl;
            pSolnMaps = runModels_defaultValues( runMITMModel, runUFBW_seqPickups, &dataInput, &dataOutput, &defaultInputs );
            break;
        }
        case OPTIN :
        {
            std::cout << "\n\nRUNNING OPT-IN SCENARIOS...\n\n" << std::endl;
            pSolnMaps = runModels_optInScenarios( runMITMModel, runUFBW_seqPickups, &dataInput, &dataOutput, &defaultInputs, &range_optInRate );
            break;
        }
        case BATCHWINDOW :
        {
            std::cout << "\n\nRUNNING BATCH WINDOW SCENARIOS...\n\n" << std::endl;
            pSolnMaps = runModels_batchWindowScenarios( runMITMModel, runUFBW_seqPickups, &dataInput, &dataOutput, &defaultInputs, &range_upFrontBatchWindowInSec );
            break;
        }
        case PICKUP :
        {
            std::cout << "\n\nRUNNING MAX PICKUP DISTANCE SCENARIOS...\n\n" << std::endl;
            pSolnMaps = runModels_maxPickupDistanceScenarios( runMITMModel, runUFBW_seqPickups, &dataInput, &dataOutput, &defaultInputs, &range_maxMatchDistInKm );
            break;
        }
        case SAVINGSRATE : 
        {
            std::cout << "\n\nRUNNING MIN SAVINGS SCENARIOS... \n\n" << std::endl;
            pSolnMaps = runModels_minSavingsScenarios( runMITMModel, runUFBW_seqPickups, &dataInput, &dataOutput, &defaultInputs, &range_minPoolDiscount );
            break;
        }
        default :
            std::cout << "OTHER";
    }
    
    if( experiment != DEFAULTVALUES ) {
        printSolutionMetricsForCurrExperiment(pSolnMaps,experiment,outputBasePath);
    }
    
            
    std::cout << "\n\n\n--- success! ---\n" << std::endl;
  
    return 0;
}

// define ranges for experiment
std::vector<int> defineBatchWindowRange() {
    std::vector<int> batchWindowRange;
    
    // all times are in SECONDS
    batchWindowRange.push_back(15);
    batchWindowRange.push_back(30);
    batchWindowRange.push_back(45);
    batchWindowRange.push_back(60);
    batchWindowRange.push_back(75);
    batchWindowRange.push_back(90);
    batchWindowRange.push_back(120);
    batchWindowRange.push_back(150);
    batchWindowRange.push_back(300);
    batchWindowRange.push_back(600);
    
    return batchWindowRange;
}
std::vector<double> defineOptInRange() {
    std::vector<double> optInRange;
    
    optInRange.push_back(0.20);
    optInRange.push_back(0.30);
    optInRange.push_back(0.40);
    optInRange.push_back(0.50);
    optInRange.push_back(0.60);
    optInRange.push_back(0.70);
    optInRange.push_back(0.80);
    optInRange.push_back(0.90);
    optInRange.push_back(0.9999);    
    
    return optInRange;
}
std::vector<double> defineMaxPickupDistRange() {
    std::vector<double> maxPickupRange;
    
    maxPickupRange.push_back(0.50);
    maxPickupRange.push_back(1.0);
    maxPickupRange.push_back(1.5);
    maxPickupRange.push_back(2.0);
    maxPickupRange.push_back(2.5);
    maxPickupRange.push_back(3.0);
    maxPickupRange.push_back(3.5);
    maxPickupRange.push_back(4.0);
    maxPickupRange.push_back(4.5);
    maxPickupRange.push_back(5.0);
    maxPickupRange.push_back(5.5);
    maxPickupRange.push_back(6.0);
    maxPickupRange.push_back(10.0);
    
    return maxPickupRange;    
}
std::vector<double> defineMinPoolDiscountRange() {
    std::vector<double> minPoolDiscMasterRange;
    
    minPoolDiscMasterRange.push_back(0.05);
    minPoolDiscMasterRange.push_back(0.10);
    minPoolDiscMasterRange.push_back(0.15);
    minPoolDiscMasterRange.push_back(0.20);
    minPoolDiscMasterRange.push_back(0.25);
    minPoolDiscMasterRange.push_back(0.30);
    minPoolDiscMasterRange.push_back(0.35);
    minPoolDiscMasterRange.push_back(0.40);
    minPoolDiscMasterRange.push_back(0.45);
    minPoolDiscMasterRange.push_back(0.50);    
    
    return minPoolDiscMasterRange;
}

// ---------------------------------
//         SOLVE EXPERIMENT
// ---------------------------------
SolnMaps * runModels_defaultValues(bool runMITMModel, bool runUFBW_seqPickups, DataInputValues * pDataInput, DataOutputValues * pDataOutput, DefaultValues * pDefaultValues) {    
    
    // use all default values
    DataContainer * pDataContainer = new DataContainer(pDataInput->_inputPath, pDataInput->_cvsFilename, pDataInput->_timelineStr, pDefaultValues->_optInRate, pDataInput->_simLengthInMinutes, pDataOutput->_printDebugFiles, pDataOutput->_printToScreen);
    pDataContainer->setBatchWindowInSeconds(pDefaultValues->_batchWindowLengthInSec);
    pDataContainer->extractCvsSnapshot();
    
    // define scenario-dependent output path   
    const std::string outputScenPath = pDataOutput->_outputBasePath;
    pDataOutput->_outputScenarioPath = pDataOutput->_outputBasePath + "defaultParams/";
    
    // instantiate output object
    Output * pOutput = new Output(pDataContainer, pDataOutput->_outputBasePath, pDataOutput->_outputScenarioPath);

    /*
     *  step 2: filter uberX users to proxy for uberPOOL trips
     */
    pDataContainer->generateUberPoolTripProxy();   // get uberPOOL users
    int nPoolTrips = pDataContainer->buildUberPOOLTrips(); // build only uberPOOL trips (subset of all trips which also contain uberX trips)  
    std::cout << Utility::intToStr(nPoolTrips) << " uberPOOL trips created" << std::endl;

    /*
     *   step 3: convert Trip objects into:
     *      (i)  Request objects (i.e. Trips that have not yet been dispatched)
     *      (ii) Dispatch objects (i.e. Driver-Rider pairs that have been dispatched)
     */
    pDataContainer->populateRequestsAndTrips();
    std::set<Request*,  ReqComp> initRequests = pDataContainer->getInitPoolRequestsAtTimeline();
    std::set<Request*,  ReqComp> allRequestsInSim = pDataContainer->getAllPoolRequestsInSim();
    std::set<OpenTrip*, EtaComp> initOpenTrips = pDataContainer->getInitOpenTripsAtTimeline();
    const std::set<Driver*, DriverIndexComp>* pDrivers = pDataContainer->getAllDrivers();  
    
    // print input files    
    writeAndPrintInputs(pDataContainer,pOutput);

    /*
     *   step 4: run all modules requested by user
     */     
    std::map<double,double> matchRateMap_MITM;
    std::map<double,double> matchRateMap_UFBW;
    std::map<double,double> inconvMap_MITM;
    std::map<double,double> inconvMap_UFBW;
    std::map<double,double>    numTripsMap_MITM;
    std::map<double,double>    numTripsMap_UFBW;
    
    if( runMITMModel ) {
        MitmModel_new * pMitmModel = new MitmModel_new(pDataContainer->getTimeline(), pDataContainer->getSimEndTime(), pDefaultValues->_maxPickupDistance, pDefaultValues->_minSavings, allRequestsInSim, initOpenTrips, pDataContainer->getAllDrivers());
        bool modelSolved = pMitmModel->solve(pDataOutput->_printDebugFiles, pOutput, pDataInput->_populateInitOpenTrips);
        if( modelSolved ) {          
            Solution * pMitmSolution = pMitmModel->getSolution();
            pOutput->printSolution(pMitmSolution);
            
            // update solution maps
            matchRateMap_MITM.insert(make_pair(1.0,pMitmSolution->getRequestMetrics()->_matchedPercentage));
            inconvMap_MITM.insert(make_pair(1.0,pMitmSolution->getInconvenienceMetrics()->_avgPctAddedDistsForAll));
            numTripsMap_MITM.insert(make_pair(1.0,pMitmSolution->getTotalNumTripsFromSoln()));
        }
    }
    if( runUFBW_seqPickups ) { 
        UFBW_fixed * pFixedBatchModel = new UFBW_fixed(pDataContainer->getTimeline(), pDataContainer->getSimEndTime(), pDefaultValues->_batchWindowLengthInSec, pDefaultValues->_maxPickupDistance, pDefaultValues->_minSavings, allRequestsInSim, initOpenTrips, pDataContainer->getAllDrivers());
        bool modelSolved = pFixedBatchModel->solve(pDataOutput->_printDebugFiles, pOutput, pDataInput->_populateInitOpenTrips, pDataOutput->_printToScreen);
        if( modelSolved ) {
            Solution * pFixedBatchSolution = pFixedBatchModel->getSolution();
            pOutput->printSolution(pFixedBatchSolution);
            
            // update solution maps
            matchRateMap_UFBW.insert(make_pair(1.0,pFixedBatchSolution->getRequestMetrics()->_matchedPercentage));
            inconvMap_UFBW.insert(make_pair(1.0,pFixedBatchSolution->getInconvenienceMetrics()->_avgPctAddedDistsForAll));
            numTripsMap_UFBW.insert(make_pair(1.0, pFixedBatchSolution->getTotalNumTripsFromSoln()));
        }
    } 
    
    // define, populate, and reutrn SolnMaps object
    SolnMaps * pSolnMaps = new SolnMaps();
    pSolnMaps->matchRate_MITM = matchRateMap_MITM;
    pSolnMaps->matchRate_UFBW = matchRateMap_UFBW;
    pSolnMaps->inconv_MITM = inconvMap_MITM;
    pSolnMaps->inconv_UFBW = inconvMap_UFBW;
    pSolnMaps->numTrips_MITM = numTripsMap_MITM;
    pSolnMaps->numTrip_UFBW = numTripsMap_UFBW;
    
    return pSolnMaps;
}
SolnMaps * runModels_optInScenarios(bool runMITMModel, bool runUFBW_seqPickups, DataInputValues * pDataInput, DataOutputValues * pDataOutput, DefaultValues * pDefaultValues, std::vector<double> * pOptInValues) {
    
    /*
     *   since opt-in rates change, both models need to be run 
     */
    const std::string OptInFolderName = "OptIn";
    int scenarioNum = 1;
    
    std::map<double,double>    matchRateMap_MITM;
    std::map<double,double>    matchRateMap_UFBW;
    std::map<double,double>    inconvMap_MITM;
    std::map<double,double>    inconvMap_UFBW;
    std::map<double,double>    numTripsMap_MITM;
    std::map<double,double>    numTripsMap_UFBW;     
    
    for( std::vector<double>::iterator optInItr = pOptInValues->begin(); optInItr != pOptInValues->end(); ++optInItr ) {
        
        // generate DataContainer object
        const double currOptInRate = *optInItr;
        DataContainer * pDataContainer = new DataContainer(pDataInput->_inputPath, pDataInput->_cvsFilename, pDataInput->_timelineStr, currOptInRate, pDataInput->_simLengthInMinutes, pDataOutput->_printDebugFiles, pDataOutput->_printToScreen);
        pDataContainer->setBatchWindowInSeconds(pDefaultValues->_batchWindowLengthInSec);
        pDataContainer->extractCvsSnapshot();
        
        // define scenario string
        std::string scenarioStr = "iter_" + Utility::intToStr(scenarioNum) + "-" + Utility::doubleToStr(currOptInRate) + "-" + Utility::intToStr(pDefaultValues->_batchWindowLengthInSec) + "-" + Utility::doubleToStr(pDefaultValues->_maxPickupDistance) + "-" + Utility::doubleToStr(pDefaultValues->_minSavings);
    
        // define scenario-dependent output path   
        const std::string outputScenPath = pDataOutput->_outputBasePath;
        const std::string optInOutPath = outputScenPath + "/" + OptInFolderName + "/";
        mkdir(optInOutPath.c_str(), S_IRWXU | S_IRWXG | S_IROTH);
        pDataOutput->_outputScenarioPath = optInOutPath + scenarioStr + "/";
        
        
        // instantiate output object
        Output * pOutput = new Output(pDataContainer, pDataOutput->_outputBasePath, pDataOutput->_outputScenarioPath);
        
        /*
        *  step 2: filter uberX users to proxy for uberPOOL trips
        */
        pDataContainer->generateUberPoolTripProxy();   // get uberPOOL users
        int nPoolTrips = pDataContainer->buildUberPOOLTrips(); // build only uberPOOL trips (subset of all trips which also contain uberX trips)  
        std::cout << Utility::intToStr(nPoolTrips) << " uberPOOL trips created" << std::endl;

        /*
         *   step 3: convert Trip objects into:
         *      (i)  Request objects (i.e. Trips that have not yet been dispatched)
         *      (ii) Dispatch objects (i.e. Driver-Rider pairs that have been dispatched)
         */
        pDataContainer->populateRequestsAndTrips();
        std::set<Request*,  ReqComp> initRequests = pDataContainer->getInitPoolRequestsAtTimeline();
        std::set<Request*,  ReqComp> allRequestsInSim = pDataContainer->getAllPoolRequestsInSim();
        std::set<OpenTrip*, EtaComp> initOpenTrips = pDataContainer->getInitOpenTripsAtTimeline();
        const std::set<Driver*, DriverIndexComp>* pDrivers = pDataContainer->getAllDrivers();  

        // print input files    
        writeAndPrintInputs(pDataContainer,pOutput);
        
       

        /*
         *   step 4: run all modules requested by user
         */     
        if( runMITMModel ) {
            MitmModel_new * pMitmModel = new MitmModel_new(pDataContainer->getTimeline(), pDataContainer->getSimEndTime(), pDefaultValues->_maxPickupDistance, pDefaultValues->_minSavings, allRequestsInSim, initOpenTrips, pDataContainer->getAllDrivers());
            bool modelSolved = pMitmModel->solve(pDataOutput->_printDebugFiles, pOutput, pDataInput->_populateInitOpenTrips);
            if( modelSolved ) {          
                Solution * pMitmSolution = pMitmModel->getSolution();
                pOutput->printSolution(pMitmSolution);
                
                // populate solution maps
                matchRateMap_MITM.insert(make_pair(currOptInRate,pMitmSolution->getRequestMetrics()->_matchedPercentage));
                inconvMap_MITM.insert(make_pair(currOptInRate,pMitmSolution->getInconvenienceMetrics()->_avgPctAddedDistsForAll));
                numTripsMap_MITM.insert(make_pair(currOptInRate,(double)pMitmSolution->getTotalNumTripsFromSoln()));
            }
        }
        if( runUFBW_seqPickups ) { 
            UFBW_fixed * pFixedBatchModel = new UFBW_fixed(pDataContainer->getTimeline(), pDataContainer->getSimEndTime(), pDefaultValues->_batchWindowLengthInSec, pDefaultValues->_maxPickupDistance, pDefaultValues->_minSavings, allRequestsInSim, initOpenTrips, pDataContainer->getAllDrivers());
            bool modelSolved = pFixedBatchModel->solve(pDataOutput->_printDebugFiles, pOutput, pDataInput->_populateInitOpenTrips, pDataOutput->_printToScreen);
            if( modelSolved ) {
                Solution * pFixedBatchSolution = pFixedBatchModel->getSolution();
                pOutput->printSolution(pFixedBatchSolution);
                
                // populate solution maps
                matchRateMap_UFBW.insert(make_pair(currOptInRate,pFixedBatchSolution->getRequestMetrics()->_matchedPercentage));
                inconvMap_UFBW.insert(make_pair(currOptInRate,pFixedBatchSolution->getInconvenienceMetrics()->_avgPctAddedDistsForAll));
                numTripsMap_UFBW.insert(make_pair(currOptInRate,(double)pFixedBatchSolution->getTotalNumTripsFromSoln()));
            }
        } 
        
        scenarioNum++;
    }      
    
    // define, populate, and return SolnMaps object
    SolnMaps * pSolnMaps = new SolnMaps();
    pSolnMaps->matchRate_MITM = matchRateMap_MITM;
    pSolnMaps->inconv_MITM = inconvMap_MITM;
    pSolnMaps->numTrips_MITM = numTripsMap_MITM;
    pSolnMaps->matchRate_UFBW = matchRateMap_UFBW;
    pSolnMaps->inconv_UFBW = inconvMap_UFBW;
    pSolnMaps->numTrip_UFBW = numTripsMap_UFBW;
    
    return pSolnMaps;
}
SolnMaps * runModels_batchWindowScenarios( bool runMITMModel, bool runUFBW_seqPickups, DataInputValues * pDataInput, DataOutputValues * pDataOutput, DefaultValues * pDefaultValues, std::vector<int> * pBatchWindowValues ) {
    
    // instantiate DataContainer object - only the batch window will change
    DataContainer * pDataContainer = new DataContainer(pDataInput->_inputPath, pDataInput->_cvsFilename, pDataInput->_timelineStr, pDefaultValues->_optInRate, pDataInput->_simLengthInMinutes, pDataOutput->_printDebugFiles, pDataOutput->_printToScreen);
    pDataContainer->setBatchWindowInSeconds(pDefaultValues->_batchWindowLengthInSec);
    pDataContainer->extractCvsSnapshot();
    
    // filter uberX users to proxy for uberPOOL trips
    pDataContainer->generateUberPoolTripProxy();   // get uberPOOL users
    int nPoolTrips = pDataContainer->buildUberPOOLTrips(); // build only uberPOOL trips (subset of all trips which also contain uberX trips)  
    std::cout << Utility::intToStr(nPoolTrips) << " uberPOOL trips created" << std::endl;

     //   step 3: convert Trip objects into Request and OpenTrip objects
    pDataContainer->populateRequestsAndTrips();
    std::set<Request*,  ReqComp> initRequests = pDataContainer->getInitPoolRequestsAtTimeline();
    std::set<Request*,  ReqComp> allRequestsInSim = pDataContainer->getAllPoolRequestsInSim();
    std::set<OpenTrip*, EtaComp> initOpenTrips = pDataContainer->getInitOpenTripsAtTimeline();
    const std::set<Driver*, DriverIndexComp>* pDrivers = pDataContainer->getAllDrivers();
        
    
    const std::string batchWindowFolderName = "BatchWindow"; // define common output folder
    const std::string batchWindowOutPath = pDataOutput->_outputBasePath + batchWindowFolderName + "/";
    mkdir( batchWindowOutPath.c_str(), S_IRWXU | S_IRWXG | S_IROTH );
    
    std::map<double,double>    matchRateMap_MITM;
    std::map<double,double>    matchRateMap_UFBW;
    std::map<double,double>    inconvMap_MITM;
    std::map<double,double>    inconvMap_UFBW;
    std::map<double,double>    numTripsMap_MITM;
    std::map<double,double>    numTripsMap_UFBW;  

    double matchRate_MITM = -1.0;    // for solution maps of MITM soln
    double inconv_MITM    = -1.0;    // for solution maps of MITM soln
    double numTrips_MITM  = -1.0;    // for solution maps of MITM soln    
    
    // first, solve MITM - solution will not change as batch window changes
    if( runMITMModel ) {        
        
        // define scenario-dependent output path           
        const std::string outputScenPath = batchWindowOutPath + "MITM/";                
        pDataOutput->_outputScenarioPath = outputScenPath;
        mkdir(pDataOutput->_outputScenarioPath.c_str(), S_IRWXU | S_IRWXG | S_IROTH);
        
        // instantiate output object
        Output * pOutput = new Output(pDataContainer, pDataOutput->_outputBasePath, pDataOutput->_outputScenarioPath);
        
        // print input files    
        writeAndPrintInputs(pDataContainer,pOutput); 
                
        MitmModel_new * pMitmModel = new MitmModel_new(pDataContainer->getTimeline(), pDataContainer->getSimEndTime(), pDefaultValues->_maxPickupDistance, pDefaultValues->_minSavings, allRequestsInSim, initOpenTrips, pDataContainer->getAllDrivers());
        bool modelSolved = pMitmModel->solve(pDataOutput->_printDebugFiles, pOutput, pDataInput->_populateInitOpenTrips);
        if( modelSolved ) {          
            Solution * pMitmSolution = pMitmModel->getSolution();
            pOutput->printSolution(pMitmSolution);
            
            matchRate_MITM = pMitmSolution->getRequestMetrics()->_matchedPercentage;
            inconv_MITM = pMitmSolution->getInconvenienceMetrics()->_avgPctAddedDistsForAll;
            numTrips_MITM = (double)pMitmSolution->getTotalNumTripsFromSoln();            
        }        
    }

    // solve UFBW - iterate over different batch windows
    if( runUFBW_seqPickups ) {
        int scenarioNum = 1;
        for( std::vector<int>::iterator batchWindowItr = pBatchWindowValues->begin(); batchWindowItr != pBatchWindowValues->end(); ++batchWindowItr ) {
            const int currBatchWindowLength = *batchWindowItr;
            
            // set the current batch window value in the DataContainer object
            pDataContainer->setBatchWindowInSeconds(currBatchWindowLength);
            
            // define scenario string
            std::string scenarioStr = "iter_" + Utility::intToStr(scenarioNum) + "-" + Utility::doubleToStr(pDefaultValues->_optInRate) + "-" + Utility::intToStr(currBatchWindowLength) + "-" + Utility::doubleToStr(pDefaultValues->_maxPickupDistance) + "-" + Utility::doubleToStr(pDefaultValues->_minSavings);

            // define scenario-dependent output path   
            const std::string outputPathBatchWindow = batchWindowOutPath + "UFBW/";
            mkdir(outputPathBatchWindow.c_str(), S_IRWXU | S_IRWXG | S_IROTH);
            const std::string outputPathBatchWindowScen = outputPathBatchWindow + scenarioStr + "/";
            pDataOutput->_outputScenarioPath = outputPathBatchWindowScen;
            mkdir(pDataOutput->_outputScenarioPath.c_str(), S_IRWXU | S_IRWXG | S_IROTH);
            
            // instantiate output object
            Output * pOutput = new Output(pDataContainer, pDataOutput->_outputBasePath, pDataOutput->_outputScenarioPath);
            
            
            UFBW_fixed * pFixedBatchModel = new UFBW_fixed(pDataContainer->getTimeline(), pDataContainer->getSimEndTime(), currBatchWindowLength, pDefaultValues->_maxPickupDistance, pDefaultValues->_minSavings, allRequestsInSim, initOpenTrips, pDataContainer->getAllDrivers());
            bool modelSolved = pFixedBatchModel->solve(pDataOutput->_printDebugFiles, pOutput, pDataInput->_populateInitOpenTrips, pDataOutput->_printToScreen);
            if( modelSolved ) {
                Solution * pFixedBatchSolution = pFixedBatchModel->getSolution();
                pOutput->printSolution(pFixedBatchSolution);
                
                // populate solution maps
                matchRateMap_UFBW.insert(make_pair((double)currBatchWindowLength,pFixedBatchSolution->getRequestMetrics()->_matchedPercentage));
                inconvMap_UFBW.insert(make_pair((double)currBatchWindowLength,pFixedBatchSolution->getInconvenienceMetrics()->_avgPctAddedDistsForAll));
                numTripsMap_UFBW.insert(make_pair((double)currBatchWindowLength,(double)pFixedBatchSolution->getTotalNumTripsFromSoln()));
                
                // if MITM was also solved, populate as well just to have the same length of maps
                if( matchRate_MITM != -1.0 ) {
                    matchRateMap_MITM.insert(make_pair((double)currBatchWindowLength,matchRate_MITM));                    
                }
                if( inconv_MITM != -1.0 ) {
                    inconvMap_MITM.insert(make_pair((double)currBatchWindowLength,inconv_MITM));
                }
                if( numTrips_MITM != -1.0 ) {
                    numTripsMap_MITM.insert(make_pair((double)currBatchWindowLength,(double)numTrips_MITM));
                }
                
            }       
                       
            scenarioNum++;
        }
    }
    
    // define, populate, and return SolnMaps object
    SolnMaps * pSolnMaps = new SolnMaps();
    pSolnMaps->matchRate_MITM = matchRateMap_MITM;
    pSolnMaps->inconv_MITM = inconvMap_MITM;
    pSolnMaps->numTrips_MITM = numTripsMap_MITM;
    pSolnMaps->matchRate_UFBW = matchRateMap_UFBW;
    pSolnMaps->inconv_UFBW = inconvMap_UFBW;
    pSolnMaps->numTrip_UFBW = numTripsMap_UFBW;
    
    return pSolnMaps;
}
SolnMaps * runModels_maxPickupDistanceScenarios( bool runMITMModel, bool runUFBW_seqPickups, DataInputValues * pDataInput, DataOutputValues * pDataOutput, DefaultValues * pDefaultValues, std::vector<double> * pMaxPickupValues ) {
    
    // only need to generate input once since only the algos are affected by the max pickup distance
    DataContainer * pDataContainer = new DataContainer(pDataInput->_inputPath, pDataInput->_cvsFilename, pDataInput->_timelineStr, pDefaultValues->_optInRate, pDataInput->_simLengthInMinutes, pDataOutput->_printDebugFiles, pDataOutput->_printToScreen);
    pDataContainer->setBatchWindowInSeconds(pDefaultValues->_batchWindowLengthInSec);
    pDataContainer->extractCvsSnapshot();
    
    // filter uberX users to proxy for uberPOOL trips
    pDataContainer->generateUberPoolTripProxy();   // get uberPOOL users
    int nPoolTrips = pDataContainer->buildUberPOOLTrips(); // build only uberPOOL trips (subset of all trips which also contain uberX trips)  
    std::cout << Utility::intToStr(nPoolTrips) << " uberPOOL trips created" << std::endl;

     //   step 3: convert Trip objects into Request and OpenTrip objects
    pDataContainer->populateRequestsAndTrips();
    std::set<Request*,  ReqComp> initRequests = pDataContainer->getInitPoolRequestsAtTimeline();
    std::set<Request*,  ReqComp> allRequestsInSim = pDataContainer->getAllPoolRequestsInSim();
    std::set<OpenTrip*, EtaComp> initOpenTrips = pDataContainer->getInitOpenTripsAtTimeline();
    const std::set<Driver*, DriverIndexComp>* pDrivers = pDataContainer->getAllDrivers();
        
    
    const std::string maxPickupOutPath = pDataOutput->_outputBasePath + "MaxPickupDist"; // define common output folder
    mkdir( maxPickupOutPath.c_str(), S_IRWXU | S_IRWXG | S_IROTH );   
    
    std::map<double,double> matchRateMap_MITM;
    std::map<double,double> matchRateMap_UFBW;
    std::map<double,double> inconvMap_MITM;
    std::map<double,double> inconvMap_UFBW;
    std::map<double,double>    numTripsMap_MITM;
    std::map<double,double>    numTripsMap_UFBW;    
    
    // iterate over possible values of max pickup distance
    int scenarioNum = 1;
    for( std::vector<double>::iterator maxPickupItr = pMaxPickupValues->begin(); maxPickupItr != pMaxPickupValues->end(); ++maxPickupItr ) {
        const double currMaxPickupDist = *maxPickupItr;
        
        // define scenario string
        std::string scenarioStr = "iter_" + Utility::intToStr(scenarioNum) + "-" + Utility::doubleToStr(pDefaultValues->_optInRate) + "-" + Utility::intToStr(pDefaultValues->_batchWindowLengthInSec) + "-" + Utility::doubleToStr(currMaxPickupDist) + "-" + Utility::doubleToStr(pDefaultValues->_minSavings);
        const std::string outputPathMaxDist = maxPickupOutPath + "/" + scenarioStr + "/";
        pDataOutput->_outputScenarioPath = outputPathMaxDist;
        mkdir(pDataOutput->_outputScenarioPath.c_str(), S_IRWXU | S_IRWXG | S_IROTH); 
        
        
        // instantiate output object
        Output * pOutput = new Output(pDataContainer, pDataOutput->_outputBasePath, pDataOutput->_outputScenarioPath);
        
        // print input files    
        writeAndPrintInputs(pDataContainer,pOutput);

        /*
         *   step 4: run all modules requested by user
         */         
        if( runMITMModel ) {
            MitmModel_new * pMitmModel = new MitmModel_new(pDataContainer->getTimeline(), pDataContainer->getSimEndTime(), currMaxPickupDist, pDefaultValues->_minSavings, allRequestsInSim, initOpenTrips, pDataContainer->getAllDrivers());
            bool modelSolved = pMitmModel->solve(pDataOutput->_printDebugFiles, pOutput, pDataInput->_populateInitOpenTrips);
            if( modelSolved ) {          
                Solution * pMitmSolution = pMitmModel->getSolution();
                pOutput->printSolution(pMitmSolution);
                
                // update solution maps for MITM model
                matchRateMap_MITM.insert(make_pair(currMaxPickupDist,pMitmSolution->getRequestMetrics()->_matchedPercentage));
                inconvMap_MITM.insert(make_pair(currMaxPickupDist,pMitmSolution->getInconvenienceMetrics()->_avgPctAddedDistsForAll));
                numTripsMap_MITM.insert(make_pair(currMaxPickupDist,(double)pMitmSolution->getTotalNumTripsFromSoln()));
            }
        }
        if( runUFBW_seqPickups ) { 
            UFBW_fixed * pFixedBatchModel = new UFBW_fixed(pDataContainer->getTimeline(), pDataContainer->getSimEndTime(), pDefaultValues->_batchWindowLengthInSec, currMaxPickupDist, pDefaultValues->_minSavings, allRequestsInSim, initOpenTrips, pDataContainer->getAllDrivers());
            bool modelSolved = pFixedBatchModel->solve(pDataOutput->_printDebugFiles, pOutput, pDataInput->_populateInitOpenTrips, pDataOutput->_printToScreen);
            if( modelSolved ) {
                Solution * pFixedBatchSolution = pFixedBatchModel->getSolution();
                pOutput->printSolution(pFixedBatchSolution);
                
                // update solution maps for UFBW model
                matchRateMap_UFBW.insert(make_pair(currMaxPickupDist,pFixedBatchSolution->getRequestMetrics()->_matchedPercentage));
                inconvMap_UFBW.insert(make_pair(currMaxPickupDist,pFixedBatchSolution->getInconvenienceMetrics()->_avgPctAddedDistsForAll));
                numTripsMap_UFBW.insert(make_pair(currMaxPickupDist,(double)pFixedBatchSolution->getTotalNumTripsFromSoln()));
            }
        } 
                
        scenarioNum++;
    }
    
    // define, populate, and return SolnMaps object
    SolnMaps * pSolnMaps = new SolnMaps();
    pSolnMaps->matchRate_MITM = matchRateMap_MITM;
    pSolnMaps->inconv_MITM = inconvMap_MITM;
    pSolnMaps->numTrips_MITM = numTripsMap_MITM;
    pSolnMaps->matchRate_UFBW = matchRateMap_UFBW;
    pSolnMaps->inconv_UFBW = inconvMap_UFBW;
    pSolnMaps->numTrip_UFBW = numTripsMap_UFBW;
    
    return pSolnMaps;
}
SolnMaps * runModels_minSavingsScenarios( bool runMITMModel, bool runUFBW_seqPickups, DataInputValues * pDataInput, DataOutputValues * pDataOutput, DefaultValues * pDefaultValues, std::vector<double> * pMinSavingsValues ) {
    
    // only need to generate input once since only the algos are affected by the min savings discount
    DataContainer * pDataContainer = new DataContainer(pDataInput->_inputPath, pDataInput->_cvsFilename, pDataInput->_timelineStr, pDefaultValues->_optInRate, pDataInput->_simLengthInMinutes, pDataOutput->_printDebugFiles, pDataOutput->_printToScreen);
    pDataContainer->setBatchWindowInSeconds(pDefaultValues->_batchWindowLengthInSec);
    pDataContainer->extractCvsSnapshot();
    
    // filter uberX users to proxy for uberPOOL trips
    pDataContainer->generateUberPoolTripProxy();   // get uberPOOL users
    int nPoolTrips = pDataContainer->buildUberPOOLTrips(); // build only uberPOOL trips (subset of all trips which also contain uberX trips)  
    std::cout << Utility::intToStr(nPoolTrips) << " uberPOOL trips created" << std::endl;

     //   step 3: convert Trip objects into Request and OpenTrip objects
    pDataContainer->populateRequestsAndTrips();
    std::set<Request*,  ReqComp> initRequests = pDataContainer->getInitPoolRequestsAtTimeline();
    std::set<Request*,  ReqComp> allRequestsInSim = pDataContainer->getAllPoolRequestsInSim();
    std::set<OpenTrip*, EtaComp> initOpenTrips = pDataContainer->getInitOpenTripsAtTimeline();
    const std::set<Driver*, DriverIndexComp>* pDrivers = pDataContainer->getAllDrivers();
        
    std::map<double,double> matchRateMap_MITM;
    std::map<double,double> matchRateMap_UFBW;
    std::map<double,double> inconvMap_MITM;
    std::map<double,double> inconvMap_UFBW;
    std::map<double,double>    numTripsMap_MITM;
    std::map<double,double>    numTripsMap_UFBW;    
    
    const std::string minSavingsPath = pDataOutput->_outputBasePath + "MinSavings"; // define common output folder
    mkdir( minSavingsPath.c_str(), S_IRWXU | S_IRWXG | S_IROTH );    
    
    // iterate over possible values of max pickup distance
    int scenarioNum = 1;
    for( std::vector<double>::iterator minSavingsItr = pMinSavingsValues->begin(); minSavingsItr != pMinSavingsValues->end(); ++minSavingsItr ) {
        const double currMinSavings = *minSavingsItr;
        
        // define scenario string
        std::string scenarioStr = "iter_" + Utility::intToStr(scenarioNum) + "-" + Utility::doubleToStr(pDefaultValues->_optInRate) + "-" + Utility::intToStr(pDefaultValues->_batchWindowLengthInSec) + "-" + Utility::doubleToStr(pDefaultValues->_maxPickupDistance) + "-" + Utility::doubleToStr(currMinSavings);
        const std::string outputMinSavingsScen = minSavingsPath + "/" + scenarioStr + "/";
        pDataOutput->_outputScenarioPath = outputMinSavingsScen;
        mkdir(pDataOutput->_outputScenarioPath.c_str(), S_IRWXU | S_IRWXG | S_IROTH); 
                
        // instantiate output object
        Output * pOutput = new Output(pDataContainer, pDataOutput->_outputBasePath, pDataOutput->_outputScenarioPath);
        
        // print input files    
        writeAndPrintInputs(pDataContainer,pOutput);

        /*
         *   step 4: run all modules requested by user
         */     
        if( runMITMModel ) {
            MitmModel_new * pMitmModel = new MitmModel_new(pDataContainer->getTimeline(), pDataContainer->getSimEndTime(), pDefaultValues->_maxPickupDistance, currMinSavings, allRequestsInSim, initOpenTrips, pDataContainer->getAllDrivers());
            bool modelSolved = pMitmModel->solve(pDataOutput->_printDebugFiles, pOutput, pDataInput->_populateInitOpenTrips);
            if( modelSolved ) {          
                Solution * pMitmSolution = pMitmModel->getSolution();
                pOutput->printSolution(pMitmSolution);
                
                // populate solution map for MITM solution
                matchRateMap_MITM.insert(make_pair(currMinSavings,pMitmSolution->getRequestMetrics()->_matchedPercentage));
                inconvMap_MITM.insert(make_pair(currMinSavings,pMitmSolution->getInconvenienceMetrics()->_avgPctAddedDistsForAll));
                numTripsMap_MITM.insert(make_pair(currMinSavings,(double)pMitmSolution->getTotalNumTripsFromSoln()));
            }
        }
        if( runUFBW_seqPickups ) { 
            UFBW_fixed * pFixedBatchModel = new UFBW_fixed(pDataContainer->getTimeline(), pDataContainer->getSimEndTime(), pDefaultValues->_batchWindowLengthInSec, pDefaultValues->_maxPickupDistance, currMinSavings, allRequestsInSim, initOpenTrips, pDataContainer->getAllDrivers());
            bool modelSolved = pFixedBatchModel->solve(pDataOutput->_printDebugFiles, pOutput, pDataInput->_populateInitOpenTrips, pDataOutput->_printToScreen);
            if( modelSolved ) {
                Solution * pFixedBatchSolution = pFixedBatchModel->getSolution();
                pOutput->printSolution(pFixedBatchSolution);
                
                // populate solution map for UFBW solution
                matchRateMap_UFBW.insert(make_pair(currMinSavings,pFixedBatchSolution->getRequestMetrics()->_matchedPercentage));
                inconvMap_UFBW.insert(make_pair(currMinSavings,pFixedBatchSolution->getInconvenienceMetrics()->_avgPctAddedDistsForAll));
                numTripsMap_UFBW.insert(make_pair(currMinSavings,(double)pFixedBatchSolution->getTotalNumTripsFromSoln()));
            }
        } 
                
        scenarioNum++;
    }    
    
    // define, populate, and return SolnMaps object
    SolnMaps * pSolnMaps = new SolnMaps();
    pSolnMaps->matchRate_MITM = matchRateMap_MITM;
    pSolnMaps->inconv_MITM = inconvMap_MITM;
    pSolnMaps->numTrips_MITM = numTripsMap_MITM;
    pSolnMaps->matchRate_UFBW = matchRateMap_UFBW;
    pSolnMaps->inconv_UFBW = inconvMap_UFBW;
    pSolnMaps->numTrip_UFBW = numTripsMap_UFBW;    
    
    return pSolnMaps;
}
void printSolutionMetricsForCurrExperiment(SolnMaps * pSolnMaps, int currExperiment, const std::string outputBasePath) {
    
    std::string scenName = "UNKNOWN";
    std::string inputName = "";
    
    switch( currExperiment ) {
        case OPTIN :
        {
            scenName = "SUMMARY-OPT-IN";
            inputName = "optIn(%)";
            break;
        }
        case BATCHWINDOW :
        {
            scenName = "SUMMARY-BATCH-WINDOW";
            inputName = "batchWindow(sec)";
            break;
        }
        case PICKUP :
        {
            scenName = "SUMMARY-MAX-PICKUP-DIST";
            inputName = "maxPickupDist(km)";
            break;
        }
        case SAVINGSRATE : 
        {
            scenName = "SUMMARY-MIN-SAVINGS-RATE";
            inputName = "minSavingsRate(%)";
            break;
        }
    }
    
    std::string outputPath  = outputBasePath + scenName + ".txt";
    
    ofstream outFile;
    outFile.open(outputPath);
    
    outFile << "-------------------------------------------------------------------------" << std::endl;
    outFile << "    summary for experiment:  " << scenName << std::endl;
    outFile << "-------------------------------------------------------------------------\n\n" << std::endl;
    
    printMatchRateMetrics    ( outFile, inputName, &(pSolnMaps->matchRate_MITM), &(pSolnMaps->matchRate_UFBW));
    printInconvenienceMetrics( outFile, inputName, &(pSolnMaps->inconv_MITM),    &(pSolnMaps->inconv_UFBW)   );
    printNumTripsMetrics     ( outFile, inputName, &(pSolnMaps->numTrips_MITM),   &(pSolnMaps->numTrip_UFBW) );
    
    outFile << "\n\n-- end of file --\n" << std::endl;
    
    outFile.close();   
}
void printMatchRateMetrics(ofstream &outFile, std::string inputName, std::map<double,double> * pMatchRateMap_MITM, std::map<double,double> *pMatchRateMap_UFBW) {
   
    outFile << "\nMATCH RATE\n" << std::endl;
    
    std::set<double> inputRange;
    for( std::map<double,double>::iterator it = pMatchRateMap_MITM->begin(); it != pMatchRateMap_MITM->end(); ++it ) {
        inputRange.insert(it->first);
    }
        
    outFile << left << setw(15) << inputName << left << setw(15) << "MITM" << left << setw(15) << "UFBW" << std::endl;

    for( std::set<double>::iterator inputItr = inputRange.begin(); inputItr != inputRange.end(); ++inputItr ) {
        
        std::string mitmMatchRateStr = "-";
        std::map<double,double>::iterator mitmItr = pMatchRateMap_MITM->find(*inputItr);
        if( mitmItr != pMatchRateMap_MITM->end() ) {
            mitmMatchRateStr = Utility::truncateDouble(mitmItr->second,4);
        }
        
        std::string ufbwMatchRateStr = "-";
        std::map<double,double>::iterator ufbwItr = pMatchRateMap_UFBW->find(*inputItr);
        if( ufbwItr != pMatchRateMap_UFBW->end() ) {
            ufbwMatchRateStr = Utility::truncateDouble(ufbwItr->second,4);
        }
        
        outFile << left << setw(15) << Utility::doubleToStr(*inputItr) << left << setw(15) << mitmMatchRateStr << left << setw(15) << ufbwMatchRateStr << std::endl;
    }
        
    outFile << "\n\n" << std::endl;        
}
void printInconvenienceMetrics(ofstream &outFile, std::string inputName, std::map<double,double> * pInconvMap_MITM, std::map<double,double> * pInconvMap_UFBW) {
    
    outFile << "\nAVG MATCHED RIDER INCONVENIENCE\n" << std::endl;
    
    std::set<double> inputRange;
    for( std::map<double,double>::iterator it = pInconvMap_MITM->begin(); it != pInconvMap_MITM->end(); ++it ) {
        inputRange.insert(it->first);
    }    
    
    outFile << left << setw(15) << inputName << left << setw(15) << "MITM" << left << setw(15) << "UFBW" << std::endl;
    for( std::set<double>::iterator inputItr = inputRange.begin(); inputItr != inputRange.end(); ++inputItr ) {
        
        std::string mitmMatchRateStr = "-";
        std::map<double,double>::iterator mitmItr = pInconvMap_MITM->find(*inputItr);
        if( mitmItr != pInconvMap_MITM->end() ) {
            mitmMatchRateStr = Utility::truncateDouble(mitmItr->second,4);
        }
        
        std::string ufbwMatchRateStr = "-";
        std::map<double,double>::iterator ufbwItr = pInconvMap_UFBW->find(*inputItr);
        if( ufbwItr != pInconvMap_UFBW->end() ) {
            ufbwMatchRateStr = Utility::truncateDouble(ufbwItr->second,4);
        }
        
        outFile << left << setw(15) << Utility::doubleToStr(*inputItr) << left << setw(15) << mitmMatchRateStr << left << setw(15) << ufbwMatchRateStr << std::endl;
    }
        
    outFile << "\n\n" << std::endl;     
    
}
void printNumTripsMetrics(ofstream &outFile, std::string inputName, std::map<double,double> * pNumTripsMap_MITM, std::map<double,double> * pNumTripsMap_UFBW) {
    outFile << "\nTOTAL NUM TRIPS\n" << std::endl;
    
    std::set<double> inputRange;
    for( std::map<double,double>::iterator it = pNumTripsMap_MITM->begin(); it != pNumTripsMap_MITM->end(); ++it ) {
        inputRange.insert(it->first);
    }    
    
    outFile << left << setw(15) << inputName << left << setw(15) << "MITM" << left << setw(15) << "UFBW" << std::endl;
    for( std::set<double>::iterator inputItr = inputRange.begin(); inputItr != inputRange.end(); ++inputItr ) {
        
        std::string mitmMatchRateStr = "-";
        std::map<double,double>::iterator mitmItr = pNumTripsMap_MITM->find(*inputItr);
        if( mitmItr != pNumTripsMap_MITM->end() ) {
            mitmMatchRateStr = Utility::intToStr((int)mitmItr->second);
        }
        
        std::string ufbwMatchRateStr = "-";
        std::map<double,double>::iterator ufbwItr = pNumTripsMap_UFBW->find(*inputItr);
        if( ufbwItr != pNumTripsMap_UFBW->end() ) {
            ufbwMatchRateStr = Utility::intToStr((int)ufbwItr->second);
        }
        
        outFile << left << setw(15) << Utility::doubleToStr(*inputItr) << left << setw(15) << mitmMatchRateStr << left << setw(15) << ufbwMatchRateStr << std::endl;
    }
        
    outFile << "\n\n" << std::endl;     
}

ParameterSet generateCurrentParameterSet(int runNum, int ScenType, double default_optIn, int default_batchWindowInSec, double default_maxDistInKm, double default_minDiscountSavings, std::vector<double>* pOptInParamVec, std::vector<int>* pBatchLengthVec, std::vector<double>* pMaxDistVec, std::vector<double>* pMinDiscVec) {
    ParameterSet currParamSet;
    
    currParamSet._iteration = runNum;
    currParamSet._optInRate = default_optIn;
    currParamSet._batchWindowLengthInSec = default_batchWindowInSec;
    currParamSet._maxPickupDistInKm = default_maxDistInKm;
    currParamSet._minDiscount = default_minDiscountSavings;
    
    // now override the changed parameter
    switch( ScenType ) {
        case DEFAULTVALUES :
            //
            break;
        case OPTIN :        
            currParamSet._optInRate = pOptInParamVec->at(runNum-1);
            break;
        case BATCHWINDOW :
            currParamSet._batchWindowLengthInSec = pBatchLengthVec->at(runNum-1);
            break;
        case PICKUP :
            currParamSet._maxPickupDistInKm = pMaxDistVec->at(runNum-1);
            break;
        case SAVINGSRATE :
            currParamSet._minDiscount = pMinDiscVec->at(runNum-1);
            break;
        default: 
            break;
            // nothing to do
    }
    
    std::string scenarioStr = "iter_" + Utility::intToStr(runNum) + "-" + Utility::doubleToStr(currParamSet._optInRate) + "-" + Utility::intToStr(currParamSet._batchWindowLengthInSec) + "-" + Utility::doubleToStr(currParamSet._maxPickupDistInKm) + "-" + Utility::doubleToStr(currParamSet._minDiscount);
    currParamSet._paramSetStr = scenarioStr;
    
    
    return currParamSet;
}
void printBanner(ParameterSet * pParamSet, int N) {
    std::cout << "\n---------------------------------------\n" << std::endl;
    std::cout<<  "       uberPOOL test environment" << std::endl;
    std::cout << "   iter " << Utility::intToStr(pParamSet->_iteration) << " of " << Utility::intToStr(N) << std::endl;
    std::cout << "\n---------------------------------------\n\n" << std::endl;
    
}
void writeAndPrintInputs(DataContainer* pDataContainer, Output * pOutput) {
    
    std::string printScreenStr = pDataContainer->printToScreen() ? "true" : "false";
    std::string printDebugStr  = pDataContainer->printDebugFiles() ? "true" : "false";
    std::cout << "\tprint to screen? " << printScreenStr << std::endl;
    std::cout << "\tprint debug?     " << printDebugStr << std::endl;
    
    // print debug files
    if( pDataContainer->printToScreen() ) {
        printSummaryOfDataInput(pDataContainer);
    }
    if( pDataContainer->printDebugFiles() ) {
        std::cout << "\tprint summary info... " << std::endl;
        pOutput->printSummaryInfo();
        std::cout << "\t\tdone." << std::endl;
        pOutput->printTripSnapshot();
        pOutput->printDrivers();
        pOutput->printPoolRiders();
        pOutput->printRequestsInSim();  
        pOutput->printInitOpenTrips();
    }      
}

void printSummaryOfDataInput(DataContainer * pDataContainer) {
    
    std::cout << "\nSUMMARY OF INPUT DATA: " << std::endl;
    std::cout << "\n\tinputPath: " << pDataContainer->getInputPath() << std::endl;
    std::cout << "\tfilename:  " << pDataContainer->getCsvFilename() << std::endl;
    
    std::cout << "\n\tsnapshot time:  " << Utility::convertTimeTToString(pDataContainer->getTimeline()) << std::endl;
    std::cout << "\tup front batching window:   " << Utility::intToStr(pDataContainer->getUpFrontBatchWindowLenInSec()) << " seconds" << std::endl;
    
    std::cout << "\n\ttotal drivers:  " << pDataContainer->getAllDrivers()->size() << std::endl;
    
    std::cout << "\n\ttotal uberX riders: " << pDataContainer->getAllUberXRiders()->size() << std::endl;
    double pctPoolRiders = (double)(100*pDataContainer->getAllUberPoolRiders()->size())/(double)(pDataContainer->getAllUberXRiders()->size());
    std::cout << "\ttotal uberPOOL riders (proxy):  " << Utility::intToStr(pDataContainer->getAllUberPoolRiders()->size()) << "  (" << Utility::doubleToStr(pctPoolRiders) << "%)" << std::endl;
    
    std::cout << "\n\ttotal trips in snapshot:   " << pDataContainer->getAllTrips()->size() << std::endl;
    double pctPoolTrips = (double)(100*pDataContainer->getUberPoolTrips()->size())/(double)(pDataContainer->getAllTrips()->size());
    std::cout << "\ttotal uberPOOL trips (proxy):  " << Utility::intToStr(pDataContainer->getUberPoolTrips()->size()) << "  (" << Utility::doubleToStr(pctPoolTrips) << "%)" << std::endl;
        
}
void printDriverInfo(DataContainer* pDataContainer) {
    // get all drivers
    const std::set<Driver*, DriverIndexComp>* pAllDrivers = pDataContainer->getAllDrivers();
    std::set<Driver*, DriverIndexComp>::const_iterator it;
    for( it = pAllDrivers->begin(); it != pAllDrivers->end(); ++it ) {
        std::cout << "\ndriver id: " << (*it)->getId() << std::endl;
        std::vector<TripData*>::const_iterator iTrip;
        for( iTrip = (*it)->getTrips()->begin(); iTrip != (*it)->getTrips()->end(); ++iTrip ) {
            std::cout << "\t TRIP: " << (*iTrip)->getIndex() << "  (driver ID " << (*iTrip)->getDriverID() << ")" << std::endl;
        }
    }    
}
void printRiderInfo(const std::set<Rider*, RiderIndexComp>* pRidersSet) {    
    std::set<Rider*, RiderIndexComp>::const_iterator rIt;
    for( rIt = pRidersSet->begin(); rIt != pRidersSet->end(); ++rIt ) {
        std::cout << "rider ID:  " << (*rIt)->getRiderID() << std::endl;
        std::vector<TripData*>::const_iterator iTrip;
        for( iTrip = (*rIt)->getTrips()->begin(); iTrip != (*rIt)->getTrips()->end(); ++iTrip ) {
            std::cout << "\tTRIP: " << (*iTrip)->getIndex() << "  (rider ID " << (*iTrip)->getRiderID() << ")" << std::endl;
        }
    }
}