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
#include "MitmModel.hpp"
#include "UFBW_fixed.hpp"
#include "FlexDepartureModel.hpp"
#include "UFBW_perfectInformation.hpp"
#include "GenerateInstanceScenarios.hpp"
#include "ProblemInstance.hpp"
#include "Geofence.hpp"
#include "FlexDepSolution.hpp"

#include <iterator>
#include <deque>

using namespace std;

enum {
        DEFAULTVALUES,
        OPTIN,
        BATCHWINDOW,
        PICKUP,
        SAVINGSRATE
     } Experiment;
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
    DefaultValues(const double optIn, const int batchWindow, const double maxPickupDist, const double minSavings, const double flexDepOptInRate, const int flexDepWindowSec) : 
        _optInRate(optIn), _batchWindowLengthInSec(batchWindow), _maxPickupDistance(maxPickupDist), _minSavings(minSavings), _flexDepOptInRate(flexDepOptInRate), _flexDepWindowInSec(flexDepWindowSec) {};
    
    const double _optInRate;
    const int    _batchWindowLengthInSec;
    const double _maxPickupDistance;
    const double _minSavings;
    const double _flexDepOptInRate;
    const int    _flexDepWindowInSec;
};
struct SolnMaps {
    // INPUT data
    std::map<double, const int>  input_numRequests;
    
    // OUTPUT data    
    std::map<double, double> matchRate_MITM;
    std::map<double, double> matchRate_UFBW;
    std::map<double, double> matchRage_FD;
    std::map<double, double> matchRate_UFBW_PI;
    
    std::map<double, double> inconv_MITM;
    std::map<double, double> inconv_UFBW;
    std::map<double, double> inconv_FD;
    std::map<double, double> inconv_UFBW_PI;
    
    std::map<double, double> numTrips_MITM;
    std::map<double, double> numTrip_UFBW;
    std::map<double, double> numTrips_FD;
    std::map<double, double> numTrips_UFBW_PI;
    
    // soln maps specifically for flex departures
    std::map<double, double> matchRate_FD_FDReqs;
    std::map<double, double> matchRate_FD_nonFDReqs;
    
};

SolnMaps * runModels_defaultValues             ( bool runMITMModel, bool runUFBW_seqPickups, bool runFlexDepartureModel, bool runUFBW_perfectInfo, DataInputValues * pDataInput, DataOutputValues * pDataOutput, DefaultValues * pDefaultValues, const std::vector<Geofence*> * pGeofences);
SolnMaps * runModels_optInScenarios            ( bool runMITMModel, bool runUFBW_seqPickups, bool runFlexDepartureModel, bool runUFBW_perfectInfo, DataInputValues * pDataInput, DataOutputValues * pDataOutput, DefaultValues * pDefaultValues, std::vector<double> * pOptInValues, const std::vector<Geofence*> * pGeofences);
SolnMaps * runModels_batchWindowScenarios      ( bool runMITMModel, bool runUFBW_seqPickups, bool runFlexDepartureModel, bool runUFBW_perfectInfo, DataInputValues * pDataInput, DataOutputValues * pDataOutput, DefaultValues * pDefaultValues, std::vector<int> * pBatchWindowValues, const std::vector<Geofence*> * pGeofences );
SolnMaps * runModels_maxPickupDistanceScenarios( bool runMITMModel, bool runUFBW_seqPickups, bool runFlexDepartureModel, bool runUFBW_perfectInfo, DataInputValues * pDataInput, DataOutputValues * pDataOutput, DefaultValues * pDefaultValues, std::vector<double> * pMaxPickupValues, const std::vector<Geofence*> * pGeofences );
SolnMaps * runModels_minSavingsScenarios       ( bool runMITMModel, bool runUFBW_seqPickups, bool runFlexDepartureModel, bool runUFBW_perfectInfo, DataInputValues * pDataInput, DataOutputValues * pDataOutput, DefaultValues * pDefaultValues, std::vector<double> * pMinSavingsValues, const std::vector<Geofence*> * pGeofences );

std::vector<int>    defineBatchWindowRange();
std::vector<double> defineOptInRange();
std::vector<double> defineMaxPickupDistRange();
std::vector<double> defineMinPoolDiscountRange();

void printBanner(ParameterSet * pParamSet, int N);
void printSummaryOfDataInput(DataContainer*);
void printDriverInfo(DataContainer*);
void printRiderInfo(const std::set<Rider*, RiderIndexComp>*);
void writeAndPrintInputs(DataContainer*, Output*);

void printSolutionMetricsForCurrExperiment(SolnMaps * pSolnMaps, std::vector<double> inputRange, int currExperiment, const std::string outputBasePath);
void printInputRequestsMetrics( ofstream &outFile, std::string inputName, std::map<double, const int> * pNumReqMap);
void printMatchRateMetrics(ofstream &outFile, std::string inputName, std::vector<double> * pInputRange, std::map<double,double> * pMatchRateMap_MITM, std::map<double,double> *pMatchRateMap_UFBW, std::map<double,double> * pMatchRateMap_FD, std::map<double,double> * pMatchRateMap_UFBW_PI, std::map<double,double> * pMatchRateMap_FD_FDReqs, std::map<double,double> * pMatchRateMap_FD_nonFDReqs);
void printInconvenienceMetrics(ofstream &outFile, std::string inputName, std::vector<double> * pInputRange, std::map<double,double> * pInconvMap_MITM, std::map<double,double> * pInconvMap_UFBW, std::map<double,double> * pInconvMap_FD, std::map<double,double> * pInconvMap_UFBW_PI);
void printNumTripsMetrics(ofstream &outFile, std::string inputName, std::vector<double> * pInputRange, std::map<double,double> * pNumTripsMap_MITM, std::map<double,double> * pNumTripsMap_UFBW, std::map<double,double> * pNumTripsMap_FD, std::map<double,double> * pNumTripsMap_UFBW_PI);

// -----------
//    MAIN
// -----------
int main(int argc, char** argv) {
             
    // specify singleton inputs // TODO: move to text file
    const std::string inputPath      = "/Users/jonpetersen/Documents/uberPOOL/testEnv/TripDataInput/";
    const std::string outputBasePath = "/Users/jonpetersen/Documents/uberPOOL/testEnv/Output/";
    
    /*
     *  populate all input scenarios
     *      scen 01: SF, one hour sim from 1600-1700 UTC on 2015-04-13, no geofences
     *      scen 02: SF, one hour sim from 1600-1700 UTC on 2015-04-13, two geofences (7x7 and SFO)
     *      scen 03: SF, one week sim from 2015-04-12 0000 - 2015-04-19 0000 (UTC), no geofences
     *      scen 04: SF, one week sim from 2015-04-12 0000 - 2015-04-19 0000 (UTC), two geofences (7x7 and SFO)
     *      scen 05: SF, four hour sim from 1300-1700 UTC on 2015-04-24, one SF geofence (incl SFO airport)
     *      scen 06: Chengdu, one week sim from 2015-04-29 1600 - 2015-04-05 1600 UTC, no geofences
     *      scen 07: Chengdu, two week data from 2015-04-29 1600 - 2015-04-12 1600 UTC consolidated to be consolidated so that the second week of data is to be moved to first week (subtract 7 days from all times)     
     *      scen 08: SF, one day sim from 2014-04-27 07:00:00 - 2015-04-28 07:00:00 UTC, no geo (flexible departure analysis)
     *      scen 09: LA, one day sim from 2014-04-27 07:00:00 - 2015-04-28 07:00:00 UTC, no geo (flexible departure analysis)
     *      scen 10: Austin, one day sim from 2014-04-27 07:00:00 - 2015-04-28 07:00:00 UTC (flexible departure analysis)
     *      scen 11: scen 09 with LA geofence
     */
    
    const int scenNumber = 11;
    ProblemInstance * pInstance = generateInstanceScenarios(scenNumber);
    
    const bool printDebugFiles       = false;
    const bool printToScreen         = false;
    const bool populateInitOpenTrips = false;
        
    //DataInputValues  dataInput(inputPath, outputBasePath, csvFilename, timelineStr, simLengthInMin, populateInitOpenTrips);
    DataInputValues  dataInput(inputPath, outputBasePath, pInstance->getInputCsvFilename(), pInstance->getSimStartTimeString(), pInstance->getSimLengthInMin(), populateInitOpenTrips);
    DataOutputValues dataOutput(outputBasePath,printDebugFiles,printToScreen);
        
    // specify DEFAULT values
    const double default_optInRate               = 0.40;
    const int    default_upFrontBatchWindowInSec = 30;
    const double default_maxMatchDistInKm        = 3.0;
    const double default_minPoolDiscount         = 0.2;
    const double default_flexDepOptInRate        = 0.25;
    const int    default_flexDepWindowInSec      = 600;
        
    DefaultValues defaultInputs(default_optInRate,default_upFrontBatchWindowInSec,default_maxMatchDistInKm,default_minPoolDiscount,default_flexDepOptInRate,default_flexDepWindowInSec);
                
    // specify RANGES to iterate experiments                 
    std::vector<double> range_optInRate = defineOptInRange(); // opt-in ranges: 0.20, 0.30, 0.40, 0.50, 0.60, 0.70, 0.80, 0.90, 1.0            
    std::vector<int>    range_upFrontBatchWindowInSec = defineBatchWindowRange(); // batch window values: 15, 30, 45, 60, 75, 90, 120, 150, 300, 600            
    std::vector<double> range_maxMatchDistInKm = defineMaxPickupDistRange(); // max pickup range (km): 0.5, 1.0, 1.5, 2.0, 2.5, 3.0, 3.5, 4.0, 4.5, 5.0, 5.5, 6.0, 10.0           
    std::vector<double> range_minPoolDiscount = defineMinPoolDiscountRange();  // max pool discount for master: 0.05, 0.10, 0.15, 0.20, 0.25, 0.30, 0.35, 0.40, 0.45, 0.50
        
    // TYPE OF TESTS TO RUN
    const bool runMITMModel        = true;
    const bool runUFBW_seqPickups  = false;
    //const bool runUFBW_flexPickups = false; 
    const bool runFlexDepModel     = false;
    const bool runUFBW_perfectInfo = false; 
    
    // SPECIFY TYPE OF EXPERIMENT
    int experiment = SAVINGSRATE;  //DEFAULTVALUES, OPTIN, BATCHWINDOW, PICKUP, SAVINGSRATE
    
    // instantiate SolnMaps to track solution
    SolnMaps * pSolnMaps = NULL;
    
    std::vector<double> inputRange;
                
    switch ( experiment ) {
        case DEFAULTVALUES : 
        {
            std::cout << "\n\nSOLVING DEFAULT SCENARIOS...\n\n" << std::endl;
            pSolnMaps = runModels_defaultValues( runMITMModel, runUFBW_seqPickups, runFlexDepModel, runUFBW_perfectInfo, &dataInput, &dataOutput, &defaultInputs, pInstance->getGeofences() );
            break;
        }
        case OPTIN :
        {
            std::cout << "\n\nRUNNING OPT-IN SCENARIOS...\n\n" << std::endl;
            pSolnMaps = runModels_optInScenarios( runMITMModel, runUFBW_seqPickups, runFlexDepModel, runUFBW_perfectInfo, &dataInput, &dataOutput, &defaultInputs, &range_optInRate, pInstance->getGeofences()  );
            inputRange = range_optInRate;
            break;
        }
        case BATCHWINDOW :
        {
            std::cout << "\n\nRUNNING BATCH WINDOW SCENARIOS...\n\n" << std::endl;
            pSolnMaps  = runModels_batchWindowScenarios( runMITMModel, runUFBW_seqPickups, runFlexDepModel, runUFBW_perfectInfo, &dataInput, &dataOutput, &defaultInputs, &range_upFrontBatchWindowInSec, pInstance->getGeofences()  );
            for( std::vector<int>::iterator windowItr = range_upFrontBatchWindowInSec.begin(); windowItr != range_upFrontBatchWindowInSec.end(); ++windowItr ) {
                inputRange.push_back(*windowItr); 
            }
            break;
        }
        case PICKUP :
        {
            std::cout << "\n\nRUNNING MAX PICKUP DISTANCE SCENARIOS...\n\n" << std::endl;
            pSolnMaps  = runModels_maxPickupDistanceScenarios( runMITMModel, runUFBW_seqPickups, runFlexDepModel, runUFBW_perfectInfo, &dataInput, &dataOutput, &defaultInputs, &range_maxMatchDistInKm, pInstance->getGeofences()  );
            inputRange = range_maxMatchDistInKm;
            break;
        }
        case SAVINGSRATE : 
        {
            std::cout << "\n\nRUNNING MIN SAVINGS SCENARIOS... \n\n" << std::endl;
            pSolnMaps  = runModels_minSavingsScenarios( runMITMModel, runUFBW_seqPickups, runFlexDepModel, runUFBW_perfectInfo, &dataInput, &dataOutput, &defaultInputs, &range_minPoolDiscount, pInstance->getGeofences()  );
            inputRange = range_minPoolDiscount;
            break;
        }
        default :
            std::cout << "OTHER";
    }
    
    if( experiment != DEFAULTVALUES ) {
        printSolutionMetricsForCurrExperiment(pSolnMaps,inputRange,experiment,outputBasePath);
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
    
    //optInRange.push_back(0.05); //
    optInRange.push_back(0.10); //
    //optInRange.push_back(0.15); // 
    optInRange.push_back(0.20);
    //optInRange.push_back(0.25); //
    optInRange.push_back(0.30);
    //optInRange.push_back(0.35); //
    optInRange.push_back(0.40);
    //optInRange.push_back(0.45); //
    optInRange.push_back(0.50);
    //optInRange.push_back(0.55); //
    optInRange.push_back(0.60);
    //optInRange.push_back(0.65); //
    optInRange.push_back(0.70);
    //optInRange.push_back(0.75); //
    optInRange.push_back(0.80);
    //optInRange.push_back(0.85); //
    optInRange.push_back(0.90);
    //optInRange.push_back(0.95); //
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
SolnMaps * runModels_defaultValues(bool runMITMModel, bool runUFBW_seqPickups, bool runFlexDepartureModel, bool runUFBW_perfectInfo, DataInputValues * pDataInput, DataOutputValues * pDataOutput, DefaultValues * pDefaultValues, const std::vector<Geofence*> * pGeofences) {    
    
    // use all default values
    DataContainer * pDataContainer = new DataContainer(pDataInput->_inputPath, pDataInput->_cvsFilename, pDataInput->_timelineStr, pDefaultValues->_optInRate, pDataInput->_simLengthInMinutes, pDataOutput->_printDebugFiles, pDataOutput->_printToScreen, pGeofences);
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
    std::map<double,const int>   numRequests_inputs;
    std::map<double,double> matchRateMap_MITM;
    std::map<double,double> matchRateMap_UFBW;
    std::map<double,double> matchRateMap_FD;
    std::map<double,double> matchRateMap_UFBW_PI;    
    std::map<double,double> inconvMap_MITM;
    std::map<double,double> inconvMap_UFBW;
    std::map<double,double> inconvMap_FD;
    std::map<double,double> inconvMap_UFBW_PI;
    std::map<double,double> matchRateMap_PI;
    std::map<double,double> numTripsMap_MITM;
    std::map<double,double> numTripsMap_UFBW;
    std::map<double,double> numTripsMap_FD;
    std::map<double,double> numTripsMap_UFBW_PI;
    std::map<double,double> matchRateMap_FD_FDReqs;
    std::map<double,double> matchRateMap_FD_nonFDReqs;
    
    if( runMITMModel ) {
        MitmModel * pMitmModel = new MitmModel(pDataContainer->getTimeline(), pDataContainer->getSimEndTime(), pDefaultValues->_maxPickupDistance, pDefaultValues->_minSavings, allRequestsInSim, initOpenTrips, pDataContainer->getAllDrivers());
        bool modelSolved = pMitmModel->solve(pDataOutput->_printDebugFiles, pOutput, pDataInput->_populateInitOpenTrips);
        if( modelSolved ) {          
            Solution * pMitmSolution = pMitmModel->getSolution();
            pOutput->printSolution(pMitmSolution);
            
            // update solution maps
            numRequests_inputs.insert(make_pair(1.0,pMitmSolution->getTotalRequests()));
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
            if( numRequests_inputs.empty() ) {
                numRequests_inputs.insert(make_pair(1.0, pFixedBatchSolution->getTotalRequests()));
            }
            matchRateMap_UFBW.insert(make_pair(1.0,pFixedBatchSolution->getRequestMetrics()->_matchedPercentage));
            inconvMap_UFBW.insert(make_pair(1.0,pFixedBatchSolution->getInconvenienceMetrics()->_avgPctAddedDistsForAll));
            numTripsMap_UFBW.insert(make_pair(1.0, pFixedBatchSolution->getTotalNumTripsFromSoln()));
        }
    }
    if( runFlexDepartureModel ) {
        FlexDepartureModel * pFlexDepModel = new FlexDepartureModel(pDataContainer->getTimeline(), pDataContainer->getSimEndTime(), pDefaultValues->_batchWindowLengthInSec, pDefaultValues->_flexDepWindowInSec, pDefaultValues->_maxPickupDistance, pDefaultValues->_minSavings, allRequestsInSim, initOpenTrips, pDataContainer->getAllDrivers(), pDefaultValues->_flexDepOptInRate);
        bool modelSolved = pFlexDepModel->solve(pDataOutput->_printDebugFiles, pOutput, pDataInput->_populateInitOpenTrips, pDataOutput->_printToScreen);
        if( modelSolved ) {
            FlexDepSolution * pFlexDepSolution = pFlexDepModel->getSolution();            
            pOutput->printSolution(pFlexDepSolution);
            
            if( numRequests_inputs.empty() ) {
                numRequests_inputs.insert(make_pair(1.0, pFlexDepSolution->getTotalRequests()));
            }
            matchRateMap_FD.insert(make_pair(1.0, pFlexDepSolution->getRequestMetrics()->_matchedPercentage));
            inconvMap_FD.insert(make_pair(1.0, pFlexDepSolution->getInconvenienceMetrics()->_avgPctAddedDistsForAll));
            numTripsMap_FD.insert(make_pair(1.0, pFlexDepSolution->getTotalNumTripsFromSoln()));
            
            matchRateMap_FD_FDReqs.insert(make_pair(1.0,pFlexDepSolution->getFlexDepReqMetrics()->_matchPercentage));
            matchRateMap_FD_nonFDReqs.insert(make_pair(1.0,pFlexDepSolution->getNonFlexDepReqMetrics()->_matchPercentage));
            //matchRate_FD_nonFDReqs;
        }
        
    }
    if( runUFBW_perfectInfo ) {              
        UFBW_perfectInformation * pPerfectInfoBatchModel = new UFBW_perfectInformation(pDataContainer->getTimeline(), pDataContainer->getSimEndTime(), pDefaultValues->_batchWindowLengthInSec, pDefaultValues->_maxPickupDistance, pDefaultValues->_minSavings, allRequestsInSim, initOpenTrips, pDataContainer->getAllDrivers());
        bool modelSolved = pPerfectInfoBatchModel->solve(pDataOutput->_printDebugFiles, pOutput, pDataInput->_populateInitOpenTrips, pDataOutput->_printToScreen);
        if( modelSolved ) {
            std::cout << "\n\nmodel solved successfully... " << std::endl;
            std::cout << "extracting solution... " << std::endl;
            Solution * pPerfectInfoBatchSolution = pPerfectInfoBatchModel->getSolution();
            std::cout << "\tdone." << std::endl;
            std::cout << "printing solution... " << std::endl;
            pOutput->printSolution(pPerfectInfoBatchSolution);
            std::cout << "\tdone. " << std::endl;
            
            // update solution maps
            if( numRequests_inputs.empty() ) {
                numRequests_inputs.insert(make_pair(1.0,pPerfectInfoBatchSolution->getTotalRequests()));
            }
            matchRateMap_UFBW_PI.insert(make_pair(1.0,pPerfectInfoBatchSolution->getRequestMetrics()->_matchedPercentage));
            inconvMap_UFBW_PI.insert(make_pair(1.0,pPerfectInfoBatchSolution->getInconvenienceMetrics()->_avgPctAddedDistsForAll));
            numTripsMap_UFBW_PI.insert(make_pair(1.0,pPerfectInfoBatchSolution->getTotalNumTripsFromSoln()));
        }
    }
    
    // define, populate, and reutrn SolnMaps object
    SolnMaps * pSolnMaps = new SolnMaps();
    
    pSolnMaps->input_numRequests = numRequests_inputs;
    pSolnMaps->matchRate_MITM    = matchRateMap_MITM;
    pSolnMaps->matchRate_UFBW    = matchRateMap_UFBW;
    pSolnMaps->matchRage_FD      = matchRateMap_FD;    
    pSolnMaps->matchRate_UFBW_PI = matchRateMap_UFBW_PI;
    
    pSolnMaps->inconv_MITM       = inconvMap_MITM;
    pSolnMaps->inconv_UFBW       = inconvMap_UFBW;
    pSolnMaps->inconv_FD         = inconvMap_FD;
    pSolnMaps->inconv_UFBW_PI    = inconvMap_UFBW_PI;
    
    pSolnMaps->numTrips_MITM     = numTripsMap_MITM;
    pSolnMaps->numTrip_UFBW      = numTripsMap_UFBW;
    pSolnMaps->numTrips_FD       = numTripsMap_FD;
    pSolnMaps->numTrips_UFBW_PI  = numTripsMap_UFBW_PI;
    
    if( matchRateMap_FD_FDReqs.size() > 0 ) {
        pSolnMaps->matchRate_FD_FDReqs = matchRateMap_FD_FDReqs;
        pSolnMaps->matchRate_FD_nonFDReqs = matchRateMap_FD_nonFDReqs;
    }
    
    return pSolnMaps;
}
SolnMaps * runModels_optInScenarios(bool runMITMModel, bool runUFBW_seqPickups, bool runFlexDepartureModel, bool runUFBW_perfectInfo, DataInputValues * pDataInput, DataOutputValues * pDataOutput, DefaultValues * pDefaultValues, std::vector<double> * pOptInValues, const std::vector<Geofence*> * pGeofences) {
    
    /*
     *   since opt-in rates change, both models need to be run 
     */
    const std::string OptInFolderName = "OptIn";
    int scenarioNum = 1;
    
    std::map<double,const int> numRequests_inputs;
    
    std::map<double,double>    matchRateMap_MITM;
    std::map<double,double>    matchRateMap_UFBW;
    std::map<double,double>    matchRateMap_FD;
    std::map<double,double>    matchRateMap_UFBW_PI;
    
    std::map<double,double>    inconvMap_MITM;
    std::map<double,double>    inconvMap_UFBW;
    std::map<double,double>    inconvMap_FD;
    std::map<double,double>    inconvMap_UFBW_PI;
    
    std::map<double,double>    numTripsMap_MITM;
    std::map<double,double>    numTripsMap_UFBW; 
    std::map<double,double>    numTripsMap_FD;
    std::map<double,double>    numTripsMap_UFBW_PI;
    
    std::map<double,double>    matchRate_FD_FDReqs;
    std::map<double,double>    matchRate_FD_nonFDReqs;
    
    for( std::vector<double>::iterator optInItr = pOptInValues->begin(); optInItr != pOptInValues->end(); ++optInItr ) {
        
        // generate DataContainer object
        const double currOptInRate = *optInItr;
        DataContainer * pDataContainer = new DataContainer(pDataInput->_inputPath, pDataInput->_cvsFilename, pDataInput->_timelineStr, currOptInRate, pDataInput->_simLengthInMinutes, pDataOutput->_printDebugFiles, pDataOutput->_printToScreen, pGeofences);
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
            MitmModel * pMitmModel = new MitmModel(pDataContainer->getTimeline(), pDataContainer->getSimEndTime(), pDefaultValues->_maxPickupDistance, pDefaultValues->_minSavings, allRequestsInSim, initOpenTrips, pDataContainer->getAllDrivers());
            bool modelSolved = pMitmModel->solve(pDataOutput->_printDebugFiles, pOutput, pDataInput->_populateInitOpenTrips);
            if( modelSolved ) {          
                Solution * pMitmSolution = pMitmModel->getSolution();
                pOutput->printSolution(pMitmSolution);
                
                // populate solution maps
                numRequests_inputs.insert(make_pair(currOptInRate,pMitmSolution->getTotalRequests()));
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
                if( numRequests_inputs.empty() ) {
                    numRequests_inputs.insert(make_pair(currOptInRate,pFixedBatchSolution->getTotalRequests()));
                }
                matchRateMap_UFBW.insert(make_pair(currOptInRate,pFixedBatchSolution->getRequestMetrics()->_matchedPercentage));
                inconvMap_UFBW.insert(make_pair(currOptInRate,pFixedBatchSolution->getInconvenienceMetrics()->_avgPctAddedDistsForAll));
                numTripsMap_UFBW.insert(make_pair(currOptInRate,(double)pFixedBatchSolution->getTotalNumTripsFromSoln()));
            }
        } 
        if( runFlexDepartureModel ) {
            FlexDepartureModel * pFlexDepModel = new FlexDepartureModel(pDataContainer->getTimeline(), pDataContainer->getSimEndTime(), pDefaultValues->_batchWindowLengthInSec, pDefaultValues->_flexDepWindowInSec, pDefaultValues->_maxPickupDistance, pDefaultValues->_minSavings, allRequestsInSim, initOpenTrips, pDataContainer->getAllDrivers(), pDefaultValues->_flexDepOptInRate);
            bool modelSolved = pFlexDepModel->solve(pDataOutput->_printDebugFiles, pOutput, pDataInput->_populateInitOpenTrips, pDataOutput->_printToScreen);
            if( modelSolved ) {
                continue;
                FlexDepSolution * pFlexDepSolution = pFlexDepModel->getSolution();
                pOutput->printSolution(pFlexDepSolution);
                
                // populate solution maps
                if( numRequests_inputs.empty() ) {
                    numRequests_inputs.insert(make_pair(currOptInRate,pFlexDepSolution->getTotalRequests()));                    
                }
                matchRateMap_FD.insert(make_pair(currOptInRate,pFlexDepSolution->getRequestMetrics()->_matchedPercentage));
                inconvMap_FD.insert(make_pair(currOptInRate,pFlexDepSolution->getInconvenienceMetrics()->_avgPctAddedDistsForAll));
                numTripsMap_FD.insert(make_pair(currOptInRate,pFlexDepSolution->getTotalNumTripsFromSoln()));                
                
                matchRate_FD_FDReqs.insert(make_pair(currOptInRate, pFlexDepSolution->getFlexDepReqMetrics()->_matchPercentage));
                matchRate_FD_nonFDReqs.insert(make_pair(currOptInRate, pFlexDepSolution->getNonFlexDepReqMetrics()->_matchPercentage));
            }
        }
        if( runUFBW_perfectInfo ) {              
            UFBW_perfectInformation * pPerfectInfoBatchModel = new UFBW_perfectInformation(pDataContainer->getTimeline(), pDataContainer->getSimEndTime(), pDefaultValues->_batchWindowLengthInSec, pDefaultValues->_maxPickupDistance, pDefaultValues->_minSavings, allRequestsInSim, initOpenTrips, pDataContainer->getAllDrivers());
            bool modelSolved = pPerfectInfoBatchModel->solve(pDataOutput->_printDebugFiles, pOutput, pDataInput->_populateInitOpenTrips, pDataOutput->_printToScreen);
            if( modelSolved ) {
                std::cout << "\n\nmodel solved successfully... " << std::endl;
                std::cout << "extracting solution... " << std::endl;
                Solution * pPerfectInfoBatchSolution = pPerfectInfoBatchModel->getSolution();
                std::cout << "\tdone." << std::endl;
                std::cout << "printing solution... " << std::endl;
                pOutput->printSolution(pPerfectInfoBatchSolution);
                std::cout << "\tdone. " << std::endl;

                // update solution maps
                if( numRequests_inputs.empty() ) {
                    numRequests_inputs.insert(make_pair(currOptInRate,pPerfectInfoBatchSolution->getTotalRequests()));
                }
                matchRateMap_UFBW_PI.insert(make_pair(currOptInRate,pPerfectInfoBatchSolution->getRequestMetrics()->_matchedPercentage));
                inconvMap_UFBW_PI.insert(make_pair(currOptInRate,pPerfectInfoBatchSolution->getInconvenienceMetrics()->_avgPctAddedDistsForAll));
                numTripsMap_UFBW_PI.insert(make_pair(currOptInRate,pPerfectInfoBatchSolution->getTotalNumTripsFromSoln()));
            }
        }
        
        scenarioNum++;
    }      
        
    // define, populate, and return SolnMaps object
    SolnMaps * pSolnMaps = new SolnMaps();
    pSolnMaps->input_numRequests = numRequests_inputs;
    pSolnMaps->matchRate_MITM = matchRateMap_MITM;
    pSolnMaps->inconv_MITM = inconvMap_MITM;
    pSolnMaps->numTrips_MITM = numTripsMap_MITM;
    pSolnMaps->matchRate_UFBW = matchRateMap_UFBW;
    pSolnMaps->inconv_UFBW = inconvMap_UFBW;
    pSolnMaps->numTrip_UFBW = numTripsMap_UFBW;
    pSolnMaps->matchRage_FD = matchRateMap_FD;
    pSolnMaps->inconv_FD = inconvMap_FD;
    pSolnMaps->numTrips_FD = numTripsMap_FD;
    pSolnMaps->matchRate_UFBW_PI = matchRateMap_UFBW_PI;
    pSolnMaps->inconv_UFBW_PI = inconvMap_UFBW_PI;
    pSolnMaps->numTrips_UFBW_PI = numTripsMap_UFBW_PI;
    
    if( matchRate_FD_FDReqs.size() > 0 ) {
        pSolnMaps->matchRate_FD_FDReqs = matchRate_FD_FDReqs;
        pSolnMaps->matchRate_FD_nonFDReqs = matchRate_FD_nonFDReqs;
    }
    
    return pSolnMaps;
}
SolnMaps * runModels_batchWindowScenarios( bool runMITMModel, bool runUFBW_seqPickups, bool runFlexDepartureModel, bool runUFBW_perfectInfo, DataInputValues * pDataInput, DataOutputValues * pDataOutput, DefaultValues * pDefaultValues, std::vector<int> * pBatchWindowValues, const std::vector<Geofence*> * pGeofences ) {
    
    // instantiate DataContainer object - only the batch window will change
    DataContainer * pDataContainer = new DataContainer(pDataInput->_inputPath, pDataInput->_cvsFilename, pDataInput->_timelineStr, pDefaultValues->_optInRate, pDataInput->_simLengthInMinutes, pDataOutput->_printDebugFiles, pDataOutput->_printToScreen, pGeofences);
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
    
    std::map<double,const int> numRequests_inputs;
    std::map<double,double>    matchRateMap_MITM;
    std::map<double,double>    matchRateMap_UFBW;
    std::map<double,double>    matchRateMap_FD;
    std::map<double,double>    matchRateMap_UFBW_PI;
    std::map<double,double>    inconvMap_MITM;
    std::map<double,double>    inconvMap_UFBW;
    std::map<double,double>    inconvMap_FD;
    std::map<double,double>    inconvMap_UFBW_PI;
    std::map<double,double>    numTripsMap_MITM;
    std::map<double,double>    numTripsMap_UFBW; 
    std::map<double,double>    numTripsMap_FD;
    std::map<double,double>    numTripsMap_UFBW_PI;
    std::map<double,double>    matchRate_FD_FDReqs;
    std::map<double,double>    matchRate_FD_nonFDReqs;

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
                
        MitmModel * pMitmModel = new MitmModel(pDataContainer->getTimeline(), pDataContainer->getSimEndTime(), pDefaultValues->_maxPickupDistance, pDefaultValues->_minSavings, allRequestsInSim, initOpenTrips, pDataContainer->getAllDrivers());
        bool modelSolved = pMitmModel->solve(pDataOutput->_printDebugFiles, pOutput, pDataInput->_populateInitOpenTrips);
        if( modelSolved ) {          
            Solution * pMitmSolution = pMitmModel->getSolution();
            pOutput->printSolution(pMitmSolution);
            
            matchRate_MITM = pMitmSolution->getRequestMetrics()->_matchedPercentage;
            inconv_MITM = pMitmSolution->getInconvenienceMetrics()->_avgPctAddedDistsForAll;
            numTrips_MITM = (double)pMitmSolution->getTotalNumTripsFromSoln();            
        }    
        
    }

    int scenarioNum = 1;
    for( std::vector<int>::iterator batchWindowItr = pBatchWindowValues->begin(); batchWindowItr != pBatchWindowValues->end(); ++batchWindowItr ) {
        const int currBatchWindowLength = *batchWindowItr;    
        
        // set the current batch window value in the DataContainer object
        pDataContainer->setBatchWindowInSeconds(currBatchWindowLength);
    
        // define scenario string
        std::string scenarioStr = "iter_" + Utility::intToStr(scenarioNum) + "-" + Utility::doubleToStr(pDefaultValues->_optInRate) + "-" + Utility::intToStr(currBatchWindowLength) + "-" + Utility::doubleToStr(pDefaultValues->_maxPickupDistance) + "-" + Utility::doubleToStr(pDefaultValues->_minSavings);

        // define scenario-dependent output path   
        const std::string outputScenPath = pDataOutput->_outputBasePath;
        const std::string batchWindowOutPath = outputScenPath + "/" + batchWindowFolderName + "/";
        mkdir(batchWindowOutPath.c_str(), S_IRWXU | S_IRWXG | S_IROTH);
        pDataOutput->_outputScenarioPath = batchWindowOutPath + scenarioStr + "/";
        
        
        // instantiate output object
        Output * pOutput = new Output(pDataContainer, pDataOutput->_outputBasePath, pDataOutput->_outputScenarioPath);    
            
        // filter uberX users to proxy for uberPOOL trips
        pDataContainer->generateUberPoolTripProxy();   // get uberPOOL users
        int nPoolTrips = pDataContainer->buildUberPOOLTrips(); // build only uberPOOL trips (subset of all trips which also contain uberX trips)  
        std::cout << Utility::intToStr(nPoolTrips) << " uberPOOL trips created" << std::endl;

        
        // convert Trip objects into Request and Dispatch objects         
        pDataContainer->populateRequestsAndTrips();
        std::set<Request*,  ReqComp> initRequests = pDataContainer->getInitPoolRequestsAtTimeline();
        std::set<Request*,  ReqComp> allRequestsInSim = pDataContainer->getAllPoolRequestsInSim();
        std::set<OpenTrip*, EtaComp> initOpenTrips = pDataContainer->getInitOpenTripsAtTimeline();
        const std::set<Driver*, DriverIndexComp>* pDrivers = pDataContainer->getAllDrivers();     
    
                
        // solve UFBW - iterate over different batch windows
        if( runUFBW_seqPickups ) {
    
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
                    numRequests_inputs.insert(make_pair((double)currBatchWindowLength,pFixedBatchSolution->getTotalRequests()));
                }
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
        }
        
        
        if( runFlexDepartureModel ) {
            FlexDepartureModel * pFlexDepModel = new FlexDepartureModel(pDataContainer->getTimeline(), pDataContainer->getSimEndTime(), currBatchWindowLength, pDefaultValues->_flexDepWindowInSec, pDefaultValues->_maxPickupDistance, pDefaultValues->_minSavings, allRequestsInSim, initOpenTrips, pDataContainer->getAllDrivers(), pDefaultValues->_flexDepOptInRate);
            bool modelSolved = pFlexDepModel->solve(pDataOutput->_printDebugFiles, pOutput, pDataInput->_populateInitOpenTrips, pDataOutput->_printToScreen);
            if( modelSolved ) {
                FlexDepSolution * pFlexDepSolution = pFlexDepModel->getSolution();
                pOutput->printSolution(pFlexDepSolution);
                
                // populate solution maps
                if( numRequests_inputs.empty() ) {
                    numRequests_inputs.insert(make_pair((double)currBatchWindowLength,pFlexDepSolution->getTotalRequests()));                    
                }
                matchRateMap_FD.insert(make_pair((double)currBatchWindowLength,pFlexDepSolution->getRequestMetrics()->_matchedPercentage));
                inconvMap_FD.insert(make_pair((double)currBatchWindowLength,pFlexDepSolution->getInconvenienceMetrics()->_avgPctAddedDistsForAll));
                numTripsMap_FD.insert(make_pair((double)currBatchWindowLength,pFlexDepSolution->getTotalNumTripsFromSoln()));                
                
                matchRate_FD_FDReqs.insert(make_pair((double)currBatchWindowLength, pFlexDepSolution->getFlexDepReqMetrics()->_matchPercentage));
                matchRate_FD_nonFDReqs.insert(make_pair((double)currBatchWindowLength, pFlexDepSolution->getNonFlexDepReqMetrics()->_matchPercentage));
                
                // if MITM was also solved, populate as well just to have the same length of maps
                if( matchRate_MITM != -1.0 ) {
                    numRequests_inputs.insert(make_pair((double)currBatchWindowLength,pFlexDepSolution->getTotalRequests()));
                }
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
        }
       
    
        if( runUFBW_perfectInfo ) { 

            UFBW_perfectInformation * pPerfectInfoBatchModel = new UFBW_perfectInformation(pDataContainer->getTimeline(), pDataContainer->getSimEndTime(), pDefaultValues->_batchWindowLengthInSec, pDefaultValues->_maxPickupDistance, pDefaultValues->_minSavings, allRequestsInSim, initOpenTrips, pDataContainer->getAllDrivers());
            bool modelSolved = pPerfectInfoBatchModel->solve(pDataOutput->_printDebugFiles, pOutput, pDataInput->_populateInitOpenTrips, pDataOutput->_printToScreen);
            if( modelSolved ) {
                std::cout << "\n\nmodel solved successfully... " << std::endl;
                std::cout << "extracting solution... " << std::endl;
                Solution * pPerfectInfoBatchSolution = pPerfectInfoBatchModel->getSolution();
                std::cout << "\tdone." << std::endl;
                std::cout << "printing solution... " << std::endl;
                pOutput->printSolution(pPerfectInfoBatchSolution);
                std::cout << "\tdone. " << std::endl;

                // update solution maps
                if( numRequests_inputs.empty() ) {
                    numRequests_inputs.insert(make_pair((double)currBatchWindowLength,pPerfectInfoBatchSolution->getTotalRequests()));
                }
                matchRateMap_UFBW_PI.insert(make_pair(1.0,pPerfectInfoBatchSolution->getRequestMetrics()->_matchedPercentage));
                inconvMap_UFBW_PI.insert(make_pair(1.0,pPerfectInfoBatchSolution->getInconvenienceMetrics()->_avgPctAddedDistsForAll));
                numTripsMap_UFBW_PI.insert(make_pair(1.0,(double)pPerfectInfoBatchSolution->getTotalNumTripsFromSoln()));
                
                // if MITM was also solved, populate as well just to have the same length of maps
                if( matchRate_MITM != -1.0 ) {
                    numRequests_inputs.insert(make_pair((double)currBatchWindowLength,pPerfectInfoBatchSolution->getTotalRequests()));
                }
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
        }
        scenarioNum++;
    }
    
    // define, populate, and return SolnMaps object
    SolnMaps * pSolnMaps = new SolnMaps();
    pSolnMaps->input_numRequests = numRequests_inputs;
    pSolnMaps->matchRate_MITM = matchRateMap_MITM;
    pSolnMaps->inconv_MITM = inconvMap_MITM;
    pSolnMaps->numTrips_MITM = numTripsMap_MITM;
    pSolnMaps->matchRate_UFBW = matchRateMap_UFBW;
    pSolnMaps->inconv_UFBW = inconvMap_UFBW;
    pSolnMaps->numTrip_UFBW = numTripsMap_UFBW;
    pSolnMaps->matchRate_UFBW_PI = matchRateMap_UFBW_PI;
    pSolnMaps->inconv_UFBW_PI = inconvMap_UFBW_PI;
    pSolnMaps->numTrips_UFBW_PI = numTripsMap_UFBW_PI;
    pSolnMaps->matchRage_FD = matchRateMap_FD;
    pSolnMaps->inconv_FD = inconvMap_FD;
    pSolnMaps->numTrips_FD = numTripsMap_FD;
    
    if( matchRate_FD_FDReqs.size() > 0 ) {
        pSolnMaps->matchRate_FD_FDReqs = matchRate_FD_FDReqs;
        pSolnMaps->matchRate_FD_nonFDReqs = matchRate_FD_nonFDReqs;
    }
    
    return pSolnMaps;
}
SolnMaps * runModels_maxPickupDistanceScenarios( bool runMITMModel, bool runUFBW_seqPickups, bool runFlexDepartureModel, bool runUFBW_perfectInfo, DataInputValues * pDataInput, DataOutputValues * pDataOutput, DefaultValues * pDefaultValues, std::vector<double> * pMaxPickupValues, const std::vector<Geofence*> * pGeofences ) {
    
    // only need to generate input once since only the algos are affected by the max pickup distance
    DataContainer * pDataContainer = new DataContainer(pDataInput->_inputPath, pDataInput->_cvsFilename, pDataInput->_timelineStr, pDefaultValues->_optInRate, pDataInput->_simLengthInMinutes, pDataOutput->_printDebugFiles, pDataOutput->_printToScreen, pGeofences);
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
    
    std::map<double, const int> numRequests_inputs;
    std::map<double,double> matchRateMap_MITM;
    std::map<double,double> matchRateMap_UFBW;
    std::map<double,double> matchRateMap_FD;
    std::map<double,double> matchRateMap_UFBW_PI;
    std::map<double,double> inconvMap_MITM;
    std::map<double,double> inconvMap_UFBW;
    std::map<double,double> inconvMap_FD;
    std::map<double,double> inconvMap_UFBW_PI;
    std::map<double,double> numTripsMap_MITM;
    std::map<double,double> numTripsMap_UFBW;    
    std::map<double,double> numTripsMap_FD;
    std::map<double,double> numTripsMap_UFBW_PI;
    
    std::map<double,double> matchRate_FD_FDReqs;
    std::map<double,double> matchRate_FD_nonFDReqs;
    
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
            MitmModel * pMitmModel = new MitmModel(pDataContainer->getTimeline(), pDataContainer->getSimEndTime(), currMaxPickupDist, pDefaultValues->_minSavings, allRequestsInSim, initOpenTrips, pDataContainer->getAllDrivers());
            bool modelSolved = pMitmModel->solve(pDataOutput->_printDebugFiles, pOutput, pDataInput->_populateInitOpenTrips);
            if( modelSolved ) {          
                Solution * pMitmSolution = pMitmModel->getSolution();
                pOutput->printSolution(pMitmSolution);
                
                // update solution maps for MITM model
                numRequests_inputs.insert(make_pair(currMaxPickupDist, pMitmSolution->getTotalRequests()));
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
                if( numRequests_inputs.empty() ) {
                    numRequests_inputs.insert(make_pair(currMaxPickupDist, pFixedBatchSolution->getTotalRequests()));
                }
                matchRateMap_UFBW.insert(make_pair(currMaxPickupDist,pFixedBatchSolution->getRequestMetrics()->_matchedPercentage));
                inconvMap_UFBW.insert(make_pair(currMaxPickupDist,pFixedBatchSolution->getInconvenienceMetrics()->_avgPctAddedDistsForAll));
                numTripsMap_UFBW.insert(make_pair(currMaxPickupDist,(double)pFixedBatchSolution->getTotalNumTripsFromSoln()));
            }
        } 
        if( runFlexDepartureModel ) {
            FlexDepartureModel * pFlexDepModel = new FlexDepartureModel(pDataContainer->getTimeline(), pDataContainer->getSimEndTime(), pDefaultValues->_batchWindowLengthInSec, pDefaultValues->_flexDepWindowInSec, currMaxPickupDist, pDefaultValues->_minSavings, allRequestsInSim, initOpenTrips, pDataContainer->getAllDrivers(), pDefaultValues->_flexDepOptInRate);
            bool modelSolved = pFlexDepModel->solve(pDataOutput->_printDebugFiles, pOutput, pDataInput->_populateInitOpenTrips, pDataOutput->_printToScreen);
            if( modelSolved ) {
                FlexDepSolution * pFlexDepSolution = pFlexDepModel->getSolution();
                pOutput->printSolution(pFlexDepSolution);
                
                // populate solution maps
                if( numRequests_inputs.empty() ) {
                    numRequests_inputs.insert(make_pair(currMaxPickupDist,pFlexDepSolution->getTotalRequests()));                    
                }
                matchRateMap_FD.insert(make_pair(currMaxPickupDist,pFlexDepSolution->getRequestMetrics()->_matchedPercentage));
                inconvMap_FD.insert(make_pair(currMaxPickupDist,pFlexDepSolution->getInconvenienceMetrics()->_avgPctAddedDistsForAll));
                numTripsMap_FD.insert(make_pair(currMaxPickupDist,pFlexDepSolution->getTotalNumTripsFromSoln()));                
                
                matchRate_FD_FDReqs.insert(make_pair(currMaxPickupDist, pFlexDepSolution->getFlexDepReqMetrics()->_matchPercentage));
                matchRate_FD_nonFDReqs.insert(make_pair(currMaxPickupDist, pFlexDepSolution->getNonFlexDepReqMetrics()->_matchPercentage));
            }            
        }
        if( runUFBW_perfectInfo ) {
            UFBW_perfectInformation * pPerfectInfoBatchModel = new UFBW_perfectInformation(pDataContainer->getTimeline(), pDataContainer->getSimEndTime(), pDefaultValues->_batchWindowLengthInSec, pDefaultValues->_maxPickupDistance, pDefaultValues->_minSavings, allRequestsInSim, initOpenTrips, pDataContainer->getAllDrivers());
            bool modelSolved = pPerfectInfoBatchModel->solve(pDataOutput->_printDebugFiles, pOutput, pDataInput->_populateInitOpenTrips, pDataOutput->_printToScreen);
            if( modelSolved ) {
                std::cout << "\n\nmodel solved successfully... " << std::endl;
                std::cout << "extracting solution... " << std::endl;
                Solution * pPerfectInfoBatchSolution = pPerfectInfoBatchModel->getSolution();
                std::cout << "\tdone." << std::endl;
                std::cout << "printing solution... " << std::endl;
                pOutput->printSolution(pPerfectInfoBatchSolution);
                std::cout << "\tdone. " << std::endl;

                // update solution maps
                if( numRequests_inputs.empty() ) {
                    numRequests_inputs.insert(make_pair(currMaxPickupDist,pPerfectInfoBatchSolution->getTotalRequests()));
                }
                matchRateMap_UFBW_PI.insert(make_pair(currMaxPickupDist,pPerfectInfoBatchSolution->getRequestMetrics()->_matchedPercentage));
                inconvMap_UFBW_PI.insert(make_pair(currMaxPickupDist,pPerfectInfoBatchSolution->getInconvenienceMetrics()->_avgPctAddedDistsForAll));
                numTripsMap_UFBW_PI.insert(make_pair(currMaxPickupDist,(double)pPerfectInfoBatchSolution->getTotalNumTripsFromSoln()));
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
    pSolnMaps->matchRate_UFBW_PI = matchRateMap_UFBW_PI;
    pSolnMaps->inconv_UFBW_PI = inconvMap_UFBW_PI;
    pSolnMaps->numTrips_UFBW_PI = numTripsMap_UFBW_PI;
    pSolnMaps->matchRage_FD = matchRateMap_FD;
    pSolnMaps->inconv_FD = inconvMap_FD;
    pSolnMaps->numTrips_FD = numTripsMap_FD;
    
    if( matchRate_FD_FDReqs.size() > 0 ) {
        pSolnMaps->matchRate_FD_FDReqs = matchRate_FD_FDReqs;
        pSolnMaps->matchRate_FD_nonFDReqs = matchRate_FD_nonFDReqs;
    }
    
    return pSolnMaps;
}
SolnMaps * runModels_minSavingsScenarios( bool runMITMModel, bool runUFBW_seqPickups, bool runFlexDepartureModel, bool runUFBW_perfectInfo, DataInputValues * pDataInput, DataOutputValues * pDataOutput, DefaultValues * pDefaultValues, std::vector<double> * pMinSavingsValues, const std::vector<Geofence*> * pGeofences ) {
    
    // only need to generate input once since only the algos are affected by the min savings discount
    DataContainer * pDataContainer = new DataContainer(pDataInput->_inputPath, pDataInput->_cvsFilename, pDataInput->_timelineStr, pDefaultValues->_optInRate, pDataInput->_simLengthInMinutes, pDataOutput->_printDebugFiles, pDataOutput->_printToScreen, pGeofences);
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
        
    std::map<double,const int> numRequests_inputs;
    std::map<double,double> matchRateMap_MITM;
    std::map<double,double> matchRateMap_UFBW;
    std::map<double,double> matchRateMap_FD;
    std::map<double,double> matchRateMap_UFBW_PI;
    std::map<double,double> inconvMap_MITM;
    std::map<double,double> inconvMap_UFBW;
    std::map<double,double> inconvMap_FD;
    std::map<double,double> inconvMap_UFBW_PI;
    std::map<double,double> numTripsMap_MITM;
    std::map<double,double> numTripsMap_UFBW;    
    std::map<double,double> numTripsMap_FD;
    std::map<double,double> numTripsMap_UFBW_PI;
    
    std::map<double,double> matchRate_FD_FDReqs;
    std::map<double,double> matchRate_FD_nonFDReqs;
    
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
            MitmModel * pMitmModel = new MitmModel(pDataContainer->getTimeline(), pDataContainer->getSimEndTime(), pDefaultValues->_maxPickupDistance, currMinSavings, allRequestsInSim, initOpenTrips, pDataContainer->getAllDrivers());
            bool modelSolved = pMitmModel->solve(pDataOutput->_printDebugFiles, pOutput, pDataInput->_populateInitOpenTrips);
            if( modelSolved ) {          
                Solution * pMitmSolution = pMitmModel->getSolution();
                pOutput->printSolution(pMitmSolution);
                
                // populate solution map for MITM solution
                numRequests_inputs.insert(make_pair(currMinSavings, pMitmSolution->getTotalRequests()));
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
                if( numRequests_inputs.empty() ) {
                    numRequests_inputs.insert(make_pair(currMinSavings, pFixedBatchSolution->getTotalRequests()));
                }
                matchRateMap_UFBW.insert(make_pair(currMinSavings,pFixedBatchSolution->getRequestMetrics()->_matchedPercentage));
                inconvMap_UFBW.insert(make_pair(currMinSavings,pFixedBatchSolution->getInconvenienceMetrics()->_avgPctAddedDistsForAll));
                numTripsMap_UFBW.insert(make_pair(currMinSavings,(double)pFixedBatchSolution->getTotalNumTripsFromSoln()));
            }
        } 
        if( runFlexDepartureModel ) {
            FlexDepartureModel * pFlexDepModel = new FlexDepartureModel(pDataContainer->getTimeline(), pDataContainer->getSimEndTime(), pDefaultValues->_batchWindowLengthInSec, pDefaultValues->_flexDepWindowInSec, pDefaultValues->_maxPickupDistance, currMinSavings, allRequestsInSim, initOpenTrips, pDataContainer->getAllDrivers(), pDefaultValues->_flexDepOptInRate);
            bool modelSolved = pFlexDepModel->solve(pDataOutput->_printDebugFiles, pOutput, pDataInput->_populateInitOpenTrips, pDataOutput->_printToScreen);
            if( modelSolved ) {
                FlexDepSolution * pFlexDepSolution = pFlexDepModel->getSolution();
                pOutput->printSolution(pFlexDepSolution);
                
                // populate solution maps
                if( numRequests_inputs.empty() ) {
                    numRequests_inputs.insert(make_pair(currMinSavings,pFlexDepSolution->getTotalRequests()));                    
                }
                matchRateMap_FD.insert(make_pair(currMinSavings,pFlexDepSolution->getRequestMetrics()->_matchedPercentage));
                inconvMap_FD.insert(make_pair(currMinSavings,pFlexDepSolution->getInconvenienceMetrics()->_avgPctAddedDistsForAll));
                numTripsMap_FD.insert(make_pair(currMinSavings,pFlexDepSolution->getTotalNumTripsFromSoln()));                
                
                matchRate_FD_FDReqs.insert(make_pair(currMinSavings, pFlexDepSolution->getFlexDepReqMetrics()->_matchPercentage));
                matchRate_FD_nonFDReqs.insert(make_pair(currMinSavings, pFlexDepSolution->getNonFlexDepReqMetrics()->_matchPercentage));
            }              
        }
        if( runUFBW_perfectInfo ) {
            UFBW_perfectInformation * pPerfectInfoBatchModel = new UFBW_perfectInformation(pDataContainer->getTimeline(), pDataContainer->getSimEndTime(), pDefaultValues->_batchWindowLengthInSec, pDefaultValues->_maxPickupDistance, pDefaultValues->_minSavings, allRequestsInSim, initOpenTrips, pDataContainer->getAllDrivers());
            bool modelSolved = pPerfectInfoBatchModel->solve(pDataOutput->_printDebugFiles, pOutput, pDataInput->_populateInitOpenTrips, pDataOutput->_printToScreen);
            if( modelSolved ) {
                std::cout << "\n\nmodel solved successfully... " << std::endl;
                std::cout << "extracting solution... " << std::endl;
                Solution * pPerfectInfoBatchSolution = pPerfectInfoBatchModel->getSolution();
                std::cout << "\tdone." << std::endl;
                std::cout << "printing solution... " << std::endl;
                pOutput->printSolution(pPerfectInfoBatchSolution);
                std::cout << "\tdone. " << std::endl;

                // update solution maps
                if( numRequests_inputs.empty() ) {
                    numRequests_inputs.insert(make_pair(currMinSavings,pPerfectInfoBatchSolution->getTotalRequests()));
                }
                
                matchRateMap_UFBW_PI.insert(make_pair(currMinSavings,pPerfectInfoBatchSolution->getRequestMetrics()->_matchedPercentage));
                inconvMap_UFBW_PI.insert(make_pair(currMinSavings,pPerfectInfoBatchSolution->getInconvenienceMetrics()->_avgPctAddedDistsForAll));
                numTripsMap_UFBW_PI.insert(make_pair(currMinSavings,(double)pPerfectInfoBatchSolution->getTotalNumTripsFromSoln()));
            }              
        }
                
        scenarioNum++;
    }    
    
    // define, populate, and return SolnMaps object
    SolnMaps * pSolnMaps = new SolnMaps();
    pSolnMaps->input_numRequests = numRequests_inputs;
    pSolnMaps->matchRate_MITM = matchRateMap_MITM;
    pSolnMaps->inconv_MITM = inconvMap_MITM;
    pSolnMaps->numTrips_MITM = numTripsMap_MITM;
    pSolnMaps->matchRate_UFBW = matchRateMap_UFBW;
    pSolnMaps->inconv_UFBW = inconvMap_UFBW;
    pSolnMaps->numTrip_UFBW = numTripsMap_UFBW;  
    pSolnMaps->matchRate_UFBW_PI = matchRateMap_UFBW_PI;
    pSolnMaps->inconv_UFBW_PI = inconvMap_UFBW_PI;
    pSolnMaps->numTrips_UFBW_PI = numTripsMap_UFBW_PI;
    pSolnMaps->matchRage_FD = matchRateMap_FD;
    pSolnMaps->inconv_FD = inconvMap_FD;
    pSolnMaps->numTrips_FD = numTripsMap_FD;
        
    if( matchRate_FD_FDReqs.size() > 0 ) {  
        pSolnMaps->matchRate_FD_FDReqs = matchRate_FD_FDReqs;
        pSolnMaps->matchRate_FD_nonFDReqs = matchRate_FD_nonFDReqs;
    }
    
    return pSolnMaps;
}

void printSolutionMetricsForCurrExperiment(SolnMaps * pSolnMaps, std::vector<double> inputRange, int currExperiment, const std::string outputBasePath) {
    
    std::string scenName = "UNKNOWN";
    std::string inputName = "";
    
    switch( currExperiment ) {
        case OPTIN :
        {
            scenName   = "SUMMARY-OPT-IN";
            inputName  = "optIn(%)";
            break;
        }
        case BATCHWINDOW :
        {
            scenName   = "SUMMARY-BATCH-WINDOW";
            inputName  = "batchWindow(sec)";
            break;
        }
        case PICKUP :
        {
            scenName   = "SUMMARY-MAX-PICKUP-DIST";
            inputName  = "maxPickupDist(km)";
            break;
        }
        case SAVINGSRATE : 
        {
            scenName   = "SUMMARY-MIN-SAVINGS-RATE";
            inputName  = "minSavingsRate(%)";
            break;
        }
    }
    
    std::string outputPath  = outputBasePath + scenName + ".txt";
    
    ofstream outFile;
    outFile.open(outputPath);
    
    outFile << "-------------------------------------------------------------------------" << std::endl;
    outFile << "    summary for experiment:  " << scenName << std::endl;
    outFile << "-------------------------------------------------------------------------\n\n" << std::endl;

    printInputRequestsMetrics( outFile, inputName, &(pSolnMaps->input_numRequests) );
    printMatchRateMetrics    ( outFile, inputName, &inputRange, &(pSolnMaps->matchRate_MITM), &(pSolnMaps->matchRate_UFBW), &(pSolnMaps->matchRage_FD), &(pSolnMaps->matchRate_UFBW_PI), &(pSolnMaps->matchRate_FD_FDReqs), &(pSolnMaps->matchRate_FD_nonFDReqs) );
    printInconvenienceMetrics( outFile, inputName, &inputRange, &(pSolnMaps->inconv_MITM),    &(pSolnMaps->inconv_UFBW), &(pSolnMaps->inconv_FD), &(pSolnMaps->inconv_UFBW_PI) );
    printNumTripsMetrics     ( outFile, inputName, &inputRange, &(pSolnMaps->numTrips_MITM),   &(pSolnMaps->numTrip_UFBW), &(pSolnMaps->numTrips_FD), &(pSolnMaps->numTrips_UFBW_PI) );
    
    outFile << "\n\n-- end of file --\n" << std::endl;
    
    outFile.close();   
}
void printInputRequestsMetrics( ofstream &outFile, std::string inputName, std::map<double, const int> * pNumReqMap) {
    std::string csvString = "CSV_numRequests";
    
    for( std::map<double, const int>::iterator mapItr = pNumReqMap->begin(); mapItr != pNumReqMap->end(); ++mapItr ) {
        csvString += "," + Utility::intToStr(mapItr->second);
    }
    
    outFile << "INSTANCE SIZES\n" << std::endl;
    outFile << csvString << std::endl;
    
}
void printMatchRateMetrics(ofstream &outFile, std::string inputName, std::vector<double> * pInputRange, std::map<double,double> * pMatchRateMap_MITM, std::map<double,double> * pMatchRateMap_UFBW, std::map<double,double> * pMatchRateMap_FD, std::map<double,double> * pMatchRateMap_UFBW_PI, std::map<double,double> * pMatchRateMap_FD_FDReqs, std::map<double,double> * pMatchRateMap_FD_nonFDReqs) {
   
    outFile << "\nMATCH RATE\n" << std::endl;
        
    std::string csv_mitm_str    = "CSV_MITM";
    std::string csv_ufbw_str    = "CSV_UFBW";
    std::string csv_flexDep_str = "CSV_FD";
    std::string csv_ufbwPI_str  = "CSV_UFBW_PI";
    
    std::string csv_FD_FDReqsStr = "";
    std::string csv_FD_nonFDReqsStr = "";
        
    outFile << left << setw(15) << inputName << left << setw(15) << "MITM" << left << setw(15) << "UFBW" << left << setw(15) << "FD" << left << setw(15) << "UFBW-PI" << std::endl;

  
    for( std::vector<double>::iterator inputItr = pInputRange->begin(); inputItr != pInputRange->end(); ++inputItr ) {
        
        std::cout << "inputItr = " << *inputItr << std::endl;
        
        std::string mitmMatchRateStr = "-";
        std::map<double,double>::iterator mitmItr = pMatchRateMap_MITM->find(*inputItr);
        if( mitmItr != pMatchRateMap_MITM->end() ) {
            mitmMatchRateStr = Utility::truncateDouble(mitmItr->second,4);
            csv_mitm_str += "," + mitmMatchRateStr;
        }
        
        std::string ufbwMatchRateStr = "-";
        std::map<double,double>::iterator ufbwItr = pMatchRateMap_UFBW->find(*inputItr);
        if( ufbwItr != pMatchRateMap_UFBW->end() ) {
            ufbwMatchRateStr = Utility::truncateDouble(ufbwItr->second,4);
            csv_ufbw_str += "," + ufbwMatchRateStr;
        }
        
        std::string flexDepMatchRateStr = "-";
        std::map<double,double>::iterator flexDepItr = pMatchRateMap_FD->find(*inputItr);
        if( flexDepItr != pMatchRateMap_FD->end() ) {
            flexDepMatchRateStr = Utility::truncateDouble(flexDepItr->second,4);
            csv_flexDep_str += "," + flexDepMatchRateStr;
        }
        
        std::string ufbwPerfInfoMatchRateStr = "-";
        std::map<double,double>::iterator ufbwPerfInfoItr = pMatchRateMap_UFBW_PI->find(*inputItr);
        if( ufbwPerfInfoItr != pMatchRateMap_UFBW_PI->end() ) {
            std::cout << "\tfound! match rate from PI is " << ufbwPerfInfoItr->second << std::endl;
            ufbwPerfInfoMatchRateStr = Utility::truncateDouble(ufbwPerfInfoItr->second,4);
            csv_ufbwPI_str += "," + ufbwPerfInfoMatchRateStr;
        } else {
            std::cout << "\tcould not find match rate associated with opt in rate " << (*inputItr) << std::endl;
        }
        

        if( pMatchRateMap_FD_FDReqs->size() > 0 ) {
                                    
            // add match rate of FD requests
            csv_FD_FDReqsStr = "CSV_FD_FDReqs";
            for( std::map<double,double>::iterator itr = pMatchRateMap_FD_FDReqs->begin(); itr != pMatchRateMap_FD_FDReqs->end(); ++itr ) {
                std::string currMatchStr = Utility::truncateDouble(itr->second,4);
                csv_FD_FDReqsStr += "," + currMatchStr;
            }                        
            
            // add match rate of non-FD requests
            csv_FD_nonFDReqsStr = "CSV_FD_nonFDReqs";
            for( std::map<double,double>::iterator itr = pMatchRateMap_FD_nonFDReqs->begin(); itr != pMatchRateMap_FD_nonFDReqs->end(); ++itr ) {
                std::string currMatchStr = Utility::truncateDouble(itr->second,4);
                csv_FD_nonFDReqsStr += "," + currMatchStr;
            }            
        }
        
        outFile << left << setw(15) << Utility::doubleToStr(*inputItr) << left << setw(15) << mitmMatchRateStr << left << setw(15) << ufbwMatchRateStr << left << setw(15) << flexDepMatchRateStr << left << setw(15) << ufbwPerfInfoMatchRateStr << std::endl;
    }
    
    outFile << "\n" << csv_mitm_str  << std::endl;
    outFile << csv_ufbw_str << std::endl;
    outFile << csv_flexDep_str << std::endl;
    outFile << csv_ufbwPI_str << std::endl;
    
    if( csv_FD_FDReqsStr != "" ) {
        outFile << csv_FD_FDReqsStr << std::endl;
        outFile << csv_FD_nonFDReqsStr << std::endl;
    }
        
    outFile << "\n\n" << std::endl;        
}
void printInconvenienceMetrics(ofstream &outFile, std::string inputName, std::vector<double> * pInputRange, std::map<double,double> * pInconvMap_MITM, std::map<double,double> * pInconvMap_UFBW, std::map<double,double> * pInconvMap_FD, std::map<double,double> * pInconvMap_UFBW_PI) {
    
    outFile << "\nAVG MATCHED RIDER INCONVENIENCE\n" << std::endl;
    
    std::string csv_mitm_str    = "CSV_MITM";
    std::string csv_ufbw_str    = "CSV_UFBW";
    std::string csv_flexDep_str = "CSV_FD";
    std::string csv_ufbwPI_str  = "CSV_UFBW_PI";
      
    outFile << left << setw(15) << inputName << left << setw(15) << "MITM" << left << setw(15) << "UFBW" << left << setw(15) << "FD" << left << setw(15) << "UFBW-PI" << std::endl;
    
    for( std::vector<double>::iterator inputItr = pInputRange->begin(); inputItr != pInputRange->end(); ++inputItr ) {
        
        std::string mitmInconvStr = "-";
        std::map<double,double>::iterator mitmItr = pInconvMap_MITM->find(*inputItr);
        if( mitmItr != pInconvMap_MITM->end() ) {
            mitmInconvStr = Utility::truncateDouble(mitmItr->second,4);
            csv_mitm_str += "," + mitmInconvStr;
        }
        
        std::string ufbwInconvStr = "-";
        std::map<double,double>::iterator ufbwItr = pInconvMap_UFBW->find(*inputItr);
        if( ufbwItr != pInconvMap_UFBW->end() ) {
           ufbwInconvStr = Utility::truncateDouble(ufbwItr->second,4);
           csv_ufbw_str += "," + ufbwInconvStr;
        }
        
        std::string flexDepStr = "-";
        std::map<double,double>::iterator flexDepItr = pInconvMap_FD->find(*inputItr);
        if( flexDepItr != pInconvMap_FD->end() ) {
            flexDepStr = Utility::truncateDouble(flexDepItr->second,4);
            csv_flexDep_str += "," + flexDepStr;
        }
        
        std::string ufbwPerfInfoInconvStr = "-";
        std::map<double,double>::iterator ufbwPerfInfoItr = pInconvMap_UFBW_PI->find(*inputItr);
        if( ufbwPerfInfoItr != pInconvMap_UFBW_PI->end() ) {
            ufbwPerfInfoInconvStr = Utility::truncateDouble(ufbwPerfInfoItr->second,4);
            csv_ufbwPI_str += "," + ufbwPerfInfoInconvStr;
        }
        
        outFile << left << setw(15) << Utility::doubleToStr(*inputItr) << left << setw(15) << mitmInconvStr << left << setw(15) << ufbwInconvStr << left << setw(15) << flexDepStr << left << setw(15) << ufbwPerfInfoInconvStr << std::endl;
    }
       
    outFile << "\n" << csv_mitm_str  << std::endl;
    outFile << csv_ufbw_str << std::endl;
    outFile << csv_flexDep_str << std::endl;
    outFile << csv_ufbwPI_str << std::endl;
    
    outFile << "\n\n" << std::endl;     
    
}
void printNumTripsMetrics(ofstream &outFile, std::string inputName, std::vector<double> * pInputRange, std::map<double,double> * pNumTripsMap_MITM, std::map<double,double> * pNumTripsMap_UFBW, std::map<double,double> * pNumTripsMap_FD, std::map<double,double> * pNumTripsMap_UFBW_PI) {
    outFile << "\nTOTAL NUM TRIPS\n" << std::endl;
    
    std::string csv_mitm_str    = "CSV_MITM";
    std::string csv_ufbw_str    = "CSV_UFBW";
    std::string csv_flexDep_str = "CSV_FD";
    std::string csv_ufbwPI_str  = "CSV_UFBW_PI";   
    
    outFile << left << setw(15) << inputName << left << setw(15) << "MITM" << left << setw(15) << "UFBW" << left << setw(15) << "FD" << left << setw(15) << "UFBW-PI" << std::endl;
    
    for( std::vector<double>::iterator inputItr = pInputRange->begin(); inputItr != pInputRange->end(); ++inputItr ) {
        
        std::string mitmNumTripsStr = "-";
        std::map<double,double>::iterator mitmItr = pNumTripsMap_MITM->find(*inputItr);
        if( mitmItr != pNumTripsMap_MITM->end() ) {
            mitmNumTripsStr = Utility::intToStr((int)mitmItr->second);
            csv_mitm_str += "," + mitmNumTripsStr;
        }
        
        std::string ufbwNumTripsStr = "-";
        std::map<double,double>::iterator ufbwItr = pNumTripsMap_UFBW->find(*inputItr);
        if( ufbwItr != pNumTripsMap_UFBW->end() ) {
            ufbwNumTripsStr = Utility::intToStr((int)ufbwItr->second);
            csv_ufbw_str += "," + ufbwNumTripsStr;
        }
        
        std::string flexDepStr = "-";
        std::map<double,double>::iterator flexDepItr = pNumTripsMap_FD->find(*inputItr);
        if( flexDepItr != pNumTripsMap_FD->end() ) {
            flexDepStr = Utility::intToStr((int)flexDepItr->second);
            csv_flexDep_str += "," + flexDepStr;
        }
        
        std::string ufbwPerfInfoNumTripsStr = "-";
        std::map<double,double>::iterator ufbwPerfInfoItr = pNumTripsMap_UFBW_PI->find(*inputItr);
        if( ufbwPerfInfoItr != pNumTripsMap_UFBW_PI->end() ) {
            ufbwPerfInfoNumTripsStr = Utility::intToStr((int)ufbwPerfInfoItr->second);
            csv_ufbwPI_str += "," + ufbwPerfInfoNumTripsStr;
        }
        
        outFile << left << setw(15) << Utility::doubleToStr(*inputItr) << left << setw(15) << mitmNumTripsStr << left << setw(15) << ufbwNumTripsStr << left << setw(15) << flexDepStr << left << setw(15) << ufbwPerfInfoNumTripsStr << std::endl;
    }
        
    
    outFile << "\n" << csv_mitm_str  << std::endl;
    outFile << csv_ufbw_str << std::endl;
    outFile << csv_flexDep_str << std::endl;
    outFile << csv_ufbwPI_str << std::endl;
    
    outFile << "\n\n" << std::endl;     
}

void printBanner(ParameterSet * pParamSet, int N) {
    std::cout << "\n---------------------------------------\n" << std::endl;
    std::cout<<  "       uberPOOL test environment" << std::endl;
    std::cout << "   iter " << Utility::intToStr(pParamSet->_iteration) << " of " << Utility::intToStr(N) << std::endl;
    std::cout << "\n---------------------------------------\n\n" << std::endl;
    
}
void writeAndPrintInputs(DataContainer* pDataContainer, Output * pOutput) {
    
    // cout
    if( pDataContainer->printToScreen() ) {
        printSummaryOfDataInput(pDataContainer);
    }
    
    // print debug files as .txt files to output folders
    if( pDataContainer->printDebugFiles() ) {
        pOutput->printSummaryInfo();
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