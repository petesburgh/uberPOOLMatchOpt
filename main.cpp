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
    int            _batchWindowLengthInSec;
    double        _maxPickupDistInKm;
    double        _minDiscountForMaster;
    std::string   _paramSetStr;
};
ParameterSet generateCurrentParameterSet(int,int,double,int,double,double,std::vector<double>*, std::vector<int>*, std::vector<double>*, std::vector<double>*);

std::vector<int>    defineBatchWindowRange();
std::vector<double> defineOptInRange();
std::vector<double> defineMaxPickupDistRange();
std::vector<double> defineMinPoolDiscountForMasterRange();

Output * pOutput;
void printBanner(ParameterSet * pParamSet, int N);
void printSummaryOfDataInput(DataContainer*);
void printDriverInfo(DataContainer*);
void printRiderInfo(const std::set<Rider*, RiderIndexComp>*);
void writeAndPrintInputs(DataContainer*);

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
        
    // specify DEFAULT values
    const int    default_upFrontBatchWindowInSec = 30;
    const double default_optInRate = 0.40;
    const double default_maxMatchDistInKm = 3.0;
    const double default_minPoolDiscountForMaster = 0.2;
                
    // specify RANGES to iterate experiments 
            // opt-in ranges: 0.20, 0.30, 0.40, 0.50, 0.60, 0.70, 0.80, 0.90, 1.0
    std::vector<double> range_optInRate = defineOptInRange();
    
        // batch window values: 15, 30, 45, 60, 75, 90, 120, 150, 300, 600
    std::vector<int>    range_upFrontBatchWindowInSec = defineBatchWindowRange();
    
        // max pickup range (km): 0.5, 1.0, 1.5, 2.0, 2.5, 3.0, 3.5, 4.0, 4.5, 5.0, 5.5, 6.0, 10.0
    std::vector<double> range_maxMatchDistInKm = defineMaxPickupDistRange();
    
        // max pool discount for master: 0.05, 0.10, 0.15, 0.20, 0.25, 0.30, 0.35, 0.40, 0.45, 0.50
    std::vector<double> range_minPoolDiscountForMaster = defineMinPoolDiscountForMasterRange();
        
    // TYPE OF TESTS TO RUN
    const bool runMITMModel        = true;
    const bool runUFBW_seqPickups  = true;
    const bool runUFBW_flexPickups = false; 
    
    // SPECIFY TYPE OF EXPERIMENT
    int experiment = BATCHWINDOW;
    
    int numRuns = -1;
    switch( experiment ) {
        case OPTIN :
            numRuns = (int)range_optInRate.size();
            break;
        case BATCHWINDOW :
            numRuns = (int)range_upFrontBatchWindowInSec.size();
            break;
        case PICKUP :
            numRuns = (int)range_maxMatchDistInKm.size();
            break;
        case SAVINGSRATE :
            numRuns = (int)range_minPoolDiscountForMaster.size();
            break;
        case DEFAULTVALUES :
            numRuns = 1;
            break;
        default :
            std::cerr << "** ERROR: Unhandled Experiment Type **" << std::endl;
    }
     
        
    // ----------------------------------------------
    // 
    //          uberPOOL MATCHING SIMULATOR
    // 
    // ----------------------------------------------            
    ParameterSet currParamSet;
    for( int ii = 1; ii <= numRuns; ii++ ) {
        
        currParamSet = generateCurrentParameterSet(
                ii,
                experiment,
                default_optInRate,
                default_upFrontBatchWindowInSec,
                default_maxMatchDistInKm,
                default_minPoolDiscountForMaster,
                &range_optInRate,
                &range_upFrontBatchWindowInSec,
                &range_maxMatchDistInKm, 
                &range_minPoolDiscountForMaster
        );
        
        // create scenario output path
        const std::string outputScenarioPath = outputBasePath + currParamSet._paramSetStr + "/";
                              
        printBanner(&currParamSet, numRuns);

        /*
            step 1: extract data from DB and populate:
               - set of all trips
               - set of all drivers
               - set of all riders  
        */
        //DataContainer * pDataContainer = new DataContainer(inputPath,outPath,csvFilename,timelineStr,upFrontBatchWindowInSec,pctPoolUsers,simLengthInMin,printDebugFiles,printToScreen);
        DataContainer * pDataContainer = new DataContainer(inputPath,csvFilename,timelineStr,currParamSet._batchWindowLengthInSec,currParamSet._optInRate,simLengthInMin,printDebugFiles,printToScreen);
        pDataContainer->extractCvsSnapshot();

        // instantiate output object
        pOutput = new Output(pDataContainer, outputBasePath, outputScenarioPath);

        /*
         *  step 2: filter uberX users to proxy for uberPOOL trips
         */
        pDataContainer->generateUberPoolTripProxy();   // get uberPOOL users
        int nPoolTrips = pDataContainer->buildUberPOOLTrips(); // build only uberPOOL trips (subset of all trips which also contain uberX trips)    
          
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

        // TODO: DELETE
        std::cout << "requests:        " << allRequestsInSim.size() << std::endl;
        std::cout << "init open trips: " << initOpenTrips.size() << std::endl;
        int expReqs = allRequestsInSim.size() + initOpenTrips.size();
        std::cout << "EXP NO. REQs:    " << expReqs << std::endl;

        std::cout << "AFTER STEP 3 there are a total of " << allRequestsInSim.size() << " requests" << std::endl;

        // print input files
        writeAndPrintInputs(pDataContainer);

        /*
         *   step 4: run all modules requested by user
         */     
        if( runMITMModel ) {
            //MitmModel_new * pMitmModel = new MitmModel_new(pDataContainer->getTimeline(), pDataContainer->getSimEndTime(), maxMatchDistInKm, minPoolMatchPctForMaster, allRequestsInSim, initOpenTrips, pDataContainer->getAllDrivers());
            MitmModel_new * pMitmModel = new MitmModel_new(pDataContainer->getTimeline(), pDataContainer->getSimEndTime(), currParamSet._maxPickupDistInKm, currParamSet._minDiscountForMaster, allRequestsInSim, initOpenTrips, pDataContainer->getAllDrivers());
            bool modelSolved = pMitmModel->solve(printDebugFiles, pOutput, populateInitOpenTrips);
            if( modelSolved ) {          
                Solution * pMitmSolution = pMitmModel->getSolution();
                pOutput->printSolution(pMitmSolution);
            }
        }
        if( runUFBW_seqPickups ) {                     
            //UFBW_fixed * pFixedBatchModel = new UFBW_fixed(pDataContainer->getTimeline(), pDataContainer->getSimEndTime(), upFrontBatchWindowInSec, maxMatchDistInKm, minPoolMatchPctForMaster, allRequestsInSim, initOpenTrips, pDataContainer->getAllDrivers());
            UFBW_fixed * pFixedBatchModel = new UFBW_fixed(pDataContainer->getTimeline(), pDataContainer->getSimEndTime(), currParamSet._batchWindowLengthInSec, currParamSet._maxPickupDistInKm, currParamSet._minDiscountForMaster, allRequestsInSim, initOpenTrips, pDataContainer->getAllDrivers());
            bool modelSolved = pFixedBatchModel->solve(printDebugFiles, pOutput, populateInitOpenTrips, printToScreen);
            if( modelSolved ) {
                std::cout << "\nUFBW solved successfully" << std::endl;
                std::cout << "\tgetting solution... " << std::endl;
                Solution * pFixedBatchSolution = pFixedBatchModel->getSolution();
                std::cout << "\t\tdone." << std::endl;
                std::cout << "\tprinting... " << std::endl;
                pOutput->printSolution(pFixedBatchSolution);
                std::cout << "\t\tdone." << std::endl; 
            }
        }
        
        std::cout << "\n\n--- done with iteration " << ii << " --" << std::endl;
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
std::vector<double> defineMinPoolDiscountForMasterRange() {
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
ParameterSet generateCurrentParameterSet(int runNum, int ScenType, double default_optIn, int default_batchWindowInSec, double default_maxDistInKm, double default_minDiscountSavingsMaster, std::vector<double>* pOptInParamVec, std::vector<int>* pBatchLengthVec, std::vector<double>* pMaxDistVec, std::vector<double>* pMinDiscMasterVec) {
    ParameterSet currParamSet;
    
    currParamSet._iteration = runNum;
    currParamSet._optInRate = default_optIn;
    currParamSet._batchWindowLengthInSec = default_batchWindowInSec;
    currParamSet._maxPickupDistInKm = default_maxDistInKm;
    currParamSet._minDiscountForMaster = default_minDiscountSavingsMaster;
    
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
            currParamSet._minDiscountForMaster = pMinDiscMasterVec->at(runNum-1);
            break;
        default: 
            break;
            // nothing to do
    }
    
    std::string scenarioStr = "iter_" + Utility::intToStr(runNum) + "-" + Utility::doubleToStr(currParamSet._optInRate) + "-" + Utility::intToStr(currParamSet._batchWindowLengthInSec) + "-" + Utility::doubleToStr(currParamSet._maxPickupDistInKm) + "-" + Utility::doubleToStr(currParamSet._minDiscountForMaster);
    currParamSet._paramSetStr = scenarioStr;
    
    
    return currParamSet;
}
void printBanner(ParameterSet * pParamSet, int N) {
    std::cout << "\n---------------------------------------\n" << std::endl;
    std::cout<<  "       uberPOOL test environment" << std::endl;
    std::cout << "   iter " << Utility::intToStr(pParamSet->_iteration) << " of " << Utility::intToStr(N) << std::endl;
    std::cout << "\n---------------------------------------\n\n" << std::endl;
    
}
void writeAndPrintInputs(DataContainer* pDataContainer) {
    // print debug files
    if( pDataContainer->printToScreen() ) {
        printSummaryOfDataInput(pDataContainer);
        //printDriverInfo(pDataContainer);                        // TODO: delete
        //printRiderInfo(pDataContainer->getAllUberXRiders());    // TODO: delete
        //printRiderInfo(pDataContainer->getAllUberPoolRiders()); // TODO: delete
    }
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
