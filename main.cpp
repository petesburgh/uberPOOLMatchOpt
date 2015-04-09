/* 
 * File:   main.cpp
 * Author: jonpetersen
 *
 * Created on March 18, 2015, 11:04 AM
 */

#include <cstdlib>
#include <iostream>
#include <iterator>
#include "DataContainer.hpp"
#include "Output.hpp"
#include "MitmModel.hpp"
#include "UFBW_fixed.hpp"

#include <iterator>
#include <deque>

using namespace std;

Output * pOutput;
void printBanner();
void printSummaryOfDataInput(DataContainer*);
void printDriverInfo(DataContainer*);
void printRiderInfo(const std::set<Rider*, RiderIndexComp>*);
void writeAndPrintInputs(DataContainer*);

/*
 *   MAIN
 */
int main(int argc, char** argv) {
    
        
    // specify inputs // TODO: move to text file
    const std::string inputPath = "/Users/jonpetersen/Documents/uberPOOL/testEnv/";
    const std::string outPath = "/Users/jonpetersen/Documents/uberPOOL/testEnv/Output/";
    const std::string csvFilename = "trips-SF-2015-03-01-1700-2000-uberX.csv";    
    const std::string timelineStr = "2015-03-01 18:30:00";
    const int upFrontBatchWindowInSec = 30;
    const double pctPoolUsers = 0.40;
    const int maxMatchDistInKm = 3;
    const int simLengthInMin = 60;
    const int maxWaitTimeBatchModelInSec = 30;
    const double minPoolMatchPctForMaster = 0.2;
    const bool printDebugFiles = true;
    const bool printToScreen = true;
    
    // TYPE OF TESTS TO RUN
    const bool runMITMModel        = true;
    const bool runUFBW_seqPickups  = false;
    const bool runUFBW_flexPickups = false; 
    
    printBanner();
    
    /*
        step 1: extract data from DB and populate:
           - set of all trips
           - set of all drivers
           - set of all riders  
    */
    DataContainer * pDataContainer = new DataContainer(inputPath,outPath,csvFilename,timelineStr,upFrontBatchWindowInSec,pctPoolUsers,simLengthInMin,printDebugFiles,printToScreen);
    pDataContainer->extractCvsSnapshot();
    
    // instantiate output object
    pOutput = new Output(pDataContainer);
    
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
        
    std::cout << "AFTER STEP 3 there are a total of " << allRequestsInSim.size() << " requests" << std::endl;
    
    // print input files
    writeAndPrintInputs(pDataContainer);
    
    /*
     *   step 4: run all modules requested by user
     */     
    if( runMITMModel ) {
        MitmModel * pMitmModel = new MitmModel(pDataContainer->getTimeline(), pDataContainer->getSimEndTime(), maxMatchDistInKm, minPoolMatchPctForMaster, allRequestsInSim, initOpenTrips, pDataContainer->getAllDrivers());
        bool modelSolved = pMitmModel->solve(printDebugFiles, pOutput);
        if( modelSolved ) {          
            Solution * pMitmSolution = pMitmModel->getSolution();
            pOutput->printSolution(pMitmSolution);
        }
    }
    if( runUFBW_seqPickups ) {                     
        UFBW_fixed * pFixedBatchModel = new UFBW_fixed(pDataContainer->getTimeline(), pDataContainer->getSimEndTime(), upFrontBatchWindowInSec, maxWaitTimeBatchModelInSec, maxMatchDistInKm, minPoolMatchPctForMaster, allRequestsInSim, initOpenTrips, pDataContainer->getAllDrivers());
        bool modelSolved = pFixedBatchModel->solve(printDebugFiles, pOutput);
        if( modelSolved ) {
            Solution * pFixedBatchSolution = pFixedBatchModel->getSolution();
            pOutput->printSolution(pFixedBatchSolution);
        }
    }
    
    
    std::cout << "\n\n\n--- success! ---\n" << std::endl;
  
 
    return 0;
}

void printBanner() {
    std::cout << "\n---------------------------------------\n" << std::endl;
    std::cout<< "       uberPOOL test environment" << std::endl;
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
