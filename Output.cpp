/* 
 * File:   Output.cpp
 * Author: jonpetersen
 * 
 * Created on March 23, 2015, 2:06 PM
 */

#include "Output.hpp"
#include "ModelRunner.hpp"


Output::Output(const std::string outputBasePath, const std::string outputExperimentPath) : 
    _outputBasePath(outputBasePath), _outputExperimentPath(outputExperimentPath) {  
               
    // ensure directory exists
    int status_base = mkdir(_outputBasePath.c_str(), S_IRWXU | S_IRWXG | S_IROTH);
    int status_scen = mkdir(_outputExperimentPath.c_str(), S_IRWXU | S_IRWXG | S_IROTH);
}

Output::~Output() {
}

void Output::writeAndPrintInputs(DataContainer* pDataContainer, const bool printDebugFiles) {
    
    // cout
    if( pDataContainer->printToScreen() ) {
        printSummaryOfDataInput(pDataContainer);
    }
    
    // print debug files as .txt files to output folders
    if( printDebugFiles ) {
        printSummaryInfo();
        printTripSnapshot();
        printDrivers();
        printPoolRiders();
        printRequestsInSim();  
        printInitOpenTrips();
    }      
}

void Output::printSummaryInfo() {
    
    std::cout << "output scenario path: " << std::endl;
    std::cout << "\tpath = " << _outputScenarioPath << std::endl;
    
    std::string filename = "snapshot_summary.txt";
    std::string outPath = _outputScenarioPath + filename;
            
    ofstream outFile;
    outFile.open(outPath);
    
    outFile << "\n----------------------------------------\n" << std::endl;
    outFile << "     summary of input data" << std::endl;
    outFile << "\n----------------------------------------\n\n" << std::endl;
    
    int buffer = 15;
    outFile << "\n\t" << left << setw(buffer) << "inputPath: " << pDataContainer->getInputPath() << std::endl;
    outFile << "\t" << left << setw(buffer) << "filename:  " << pDataContainer->getCsvFilename() << std::endl;
    
    buffer = 35;
    outFile << "\n\t" << left << setw(buffer) << "snapshot time:  " << Utility::convertTimeTToString(pDataContainer->getTimeline()) << std::endl;
    outFile << "\t" << left << setw(buffer) << "sim end time: " << Utility::convertTimeTToString(pDataContainer->getSimEndTime()) << std::endl;
//    outFile << "\t" << left << setw(buffer) << "up front batching window (sec): " << Utility::intToStr(pDataContainer->getUpFrontBatchWindowLenInSec()) << std::endl;

    
    buffer = 35;    
    outFile << "\n\t" << left << setw(buffer) << "total number drivers: " << Utility::intToStr(pDataContainer->getAllDrivers()->size()) << std::endl;
    
    outFile << "\n\t" << left << setw(buffer) << "total uberX riders: " << pDataContainer->getAllUberXRiders()->size() << std::endl;
    double pctPoolUsers = (double)(100*pDataContainer->getAllUberPoolRiders()->size())/(double)(pDataContainer->getAllUberXRiders()->size());
    outFile << "\t" << left << setw(buffer) << "total uberPOOL riders (proxy):  " << Utility::intToStr(pDataContainer->getAllUberPoolRiders()->size()) << "  (" << Utility::doubleToStr(pctPoolUsers) << "%)" << std::endl;    
    
    outFile << "\n\t" << left << setw(buffer) << "total trips in snapshot:   " << Utility::intToStr(pDataContainer->getAllTrips()->size()) << std::endl;
    double pctPoolTrips = (double)(100*pDataContainer->getUberPoolTrips()->size())/(double)(pDataContainer->getAllTrips()->size());
    outFile << "\t" << left << setw(buffer) << "total uberPOOL trips (proxy): " << Utility::intToStr(pDataContainer->getUberPoolTrips()->size()) << "  (" << Utility::doubleToStr(pctPoolTrips) << "%)" << std::endl;
                
    outFile << "\n\n\n\t(end of file)\n\n" << std::endl;
    outFile.close();
}

void Output::printTripSnapshot() {
    std::string filename = "summaryAllTrips.txt";
    std::string outPath = _outputScenarioPath + filename;
    
    ofstream outFile(outPath);
    outFile << "\n----------------------------------------\n" << std::endl;
    outFile << "      summary of trips within snapshot" << std::endl;
    outFile << "\n----------------------------------------\n\n" << std::endl;
    
    outFile << "TIMELINE: " << Utility::convertTimeTToString(pDataContainer->getTimeline()) << "\n\n" << std::endl;
    
    // buffers
    int indexBuffer  = 8;
    int statusBuffer = 16;
    int eventBuffer  = 55;
    
    
    outFile << "\t" <<
            left << setw(indexBuffer) << "index" << 
            left << setw(statusBuffer) << "status" << 
            left << setw(eventBuffer) << "requestEvent" << 
            left << setw(eventBuffer) << "dispatchEvent" << 
            left << setw(eventBuffer) << "pickupEvent" << 
            left << setw(eventBuffer) << "dropoffEvent" << std::endl;
    outFile << "\n" << std::endl;
    
    const std::vector<TripData*>* pTrips = pDataContainer->getAllTrips();
    for( std::vector<TripData*>::const_iterator tripItr = pTrips->begin(); tripItr != pTrips->end(); ++tripItr ) {
        
        outFile << "\t" << 
                left << setw(indexBuffer) << Utility::intToStr((*tripItr)->getIndex()) << 
                left << setw(statusBuffer) << (*tripItr)->getStatusString() <<
                left << setw(eventBuffer) << (*tripItr)->getRequestEvent()->getEventStr() << 
                left << setw(eventBuffer) << (*tripItr)->getDispatchEvent()->getEventStr() << 
                left << setw(eventBuffer) << (*tripItr)->getPickupEvent()->getEventStr() << 
                left << setw(eventBuffer) << (*tripItr)->getDropoffEvent()->getEventStr() << std::endl;
    }
    
    outFile << "\n\n\t(end of file)\n\n\n" << std::endl;
    
    outFile.close();
    
}

void Output::printPoolRiders() {
    
    std::string filename = "poolRiders.txt";
    std::string outPath = _outputScenarioPath + filename;
    
    ofstream outFile(outPath);
    outFile << "\n----------------------------------------\n" << std::endl;
    outFile << "          summary of riders" << std::endl;
    outFile << "\n----------------------------------------\n\n" << std::endl;

    std::set<Rider*>::const_iterator rIt;
    for( rIt = pDataContainer->getAllUberPoolRiders()->begin(); rIt != pDataContainer->getAllUberPoolRiders()->end(); ++rIt ) {
        outFile << "\nrider ID:    " << (*rIt)->getRiderID() << std::endl;
        outFile << "rider index: " << (*rIt)->getIndex() << std::endl;
        std::vector<TripData*>::const_iterator iTrip;
        for( iTrip = (*rIt)->getTrips()->begin(); iTrip != (*rIt)->getTrips()->end(); ++iTrip ) {
            outFile << "\tTRIP: " << (*iTrip)->getIndex() << "  (rider ID " << (*iTrip)->getRiderID() << ")" << std::endl;
        }
    }
    
    outFile << "\n\n\n\t(end of file)\n\n" << std::endl;

    outFile.close();    
}

void Output::printDrivers() {
    
    std::string filename = "drivers.txt";
    std::string outPath = _outputScenarioPath + filename;
    
    ofstream outFile(outPath);
    outFile << "\n----------------------------------------\n" << std::endl;
    outFile << "          summary of drivers" << std::endl;
    outFile << "\n----------------------------------------\n\n" << std::endl;    
    
    // get all drivers
    const std::set<Driver*, DriverIndexComp>* pAllDrivers = pDataContainer->getAllDrivers();
    std::set<Driver*>::const_iterator it;
    for( it = pAllDrivers->begin(); it != pAllDrivers->end(); ++it ) {
        outFile << "\ndriver id: " << (*it)->getId() << std::endl;
        std::vector<TripData*>::const_iterator iTrip;
        for( iTrip = (*it)->getTrips()->begin(); iTrip != (*it)->getTrips()->end(); ++iTrip ) {
            outFile << "\t TRIP: " << (*iTrip)->getIndex() << "  (driver ID " << (*iTrip)->getDriverID() << ")" << std::endl;
        }
    }     
    
    outFile << "\n\n\n\t(end of file)\n\n" << std::endl;
    
    outFile.close();   
}

void Output::printRequestsInSim() {
    
    std::string filename = "poolRequests.txt";
    std::string outPath = _outputScenarioPath + filename;
    
    ofstream outFile(outPath);
    outFile << "\n----------------------------------------\n" << std::endl;
    outFile << "      summary of uberPOOL requests" << std::endl;
    outFile << "\n----------------------------------------\n\n" << std::endl;  
   
    // print sim time
    outFile << left << setw(15) << "timeline:  " << Utility::convertTimeTToString(pDataContainer->getTimeline()) << std::endl;
    outFile << left << setw(15) << "sim end time: " << Utility::convertTimeTToString(pDataContainer->getSimEndTime()) << std::endl;
       
    // print header
    int indexBuffer = 10;
    int riderBuffer = 45;
    int timeBuffer = 30;
    int locBuffer = 25;
    int initBuffer = 12;
    outFile << "\n\t" << left << setw(indexBuffer) << "tripIx" << 
            left << setw(riderBuffer) << "riderID" << 
            left << setw(indexBuffer) << "riderIx" <<
            left << setw(initBuffer) << "initReq?" << 
            left << setw(timeBuffer) << "requestTime" << 
            left << setw(locBuffer) << "pickupLat" << 
            left << setw(locBuffer) << "pickupLng" << 
            left << setw(locBuffer) << "dropLat" << 
            left << setw(locBuffer) << "dropLng" << std::endl;
            
    // iterate over all requests
    std::set<Request*, ReqComp> requests = pDataContainer->getAllPoolRequestsInSim();
    for( std::set<Request*, ReqComp>::iterator iReq = requests.begin(); iReq != requests.end(); ++iReq ) {
        std::string isInit = (*iReq)->isInitRequest() ? "yes" : "no"; 
        outFile << "\t" << left << setw(indexBuffer) << Utility::intToStr((*iReq)->getReqIndex()) << 
                left << setw(riderBuffer) << (*iReq)->getRiderID() << 
                left << setw(indexBuffer) << Utility::intToStr((*iReq)->getRiderIndex()) <<
                left << setw(initBuffer) << isInit <<
                left << setw(timeBuffer) << Utility::convertTimeTToString((*iReq)->getReqTime()) << 
                left << setw(locBuffer) << Utility::doubleToStr((*iReq)->getPickupLat()) << 
                left << setw(locBuffer) << Utility::doubleToStr((*iReq)->getPickupLng()) << 
                left << setw(locBuffer) << Utility::doubleToStr((*iReq)->getDropoffLat()) << 
                left << setw(locBuffer) << Utility::doubleToStr((*iReq)->getDropoffLng()) << std::endl;
    }
    
    outFile << "\n\n\n\t(end of file)\n\n" << std::endl;    
    outFile.close();    
}

void Output::printInitOpenTrips() {
    
    std::string filename = "initOpenTrips.txt";
    std::string outPath = _outputScenarioPath + filename;
    
    ofstream outFile(outPath);
    outFile << "\n----------------------------------------\n" << std::endl;
    outFile << "    summary of init open trips (masters)" << std::endl;
    outFile << "\n----------------------------------------\n\n" << std::endl;   
    
    int ixBuffer = 8;
    int idBuffer = 12;
    int timeBuffer = 28;
    int eventBuffer = 55;
    
    // print headers
    outFile << "\t" << left << setw(ixBuffer) << "index" << 
            left << setw(idBuffer) << "driverID" << 
            left << setw(idBuffer) << "riderID" << 
            left << setw(timeBuffer) << "ETA" << 
            left << setw(timeBuffer) << "ETD" << 
            left << setw(eventBuffer) << "request" << 
            left << setw(eventBuffer) << "dispatched" << std::endl;
    
    // print every init open trip
    std::set<OpenTrip*, EtaComp> initOpenTrips = pDataContainer->getInitOpenTripsAtTimeline();
    std::set<OpenTrip*, EtaComp>::iterator itr;
    int counter = 0;
    for( itr = initOpenTrips.begin(); itr != initOpenTrips.end(); ++itr ) {
        
        std::string riderIntStr = "";
        std::string driverIntStr = "";
       
        try {
            Rider * pRider   = pDataContainer->getRiderFromID((*itr)->getMasterID());
            riderIntStr = Utility::intToStr(pRider->getIndex());
        } catch( ItemNotFoundException &ex ) {
            std::cerr << "\n*** ItemNotFoundException thrown ***" << std::endl;
            std::cerr << ex.what() << std::endl;           
        }
               
        try {
            Driver * pDriver = pDataContainer->getDriverFromID((*itr)->getDriverID());
            driverIntStr = Utility::intToStr(pDriver->getIndex());
        } catch( ItemNotFoundException &ex ) {
            std::cerr << "\n*** ItemNotFoundException thrown ***" << std::endl;
            std::cerr << ex.what() << std::endl;    
        }
        
        outFile << "\t" << left << setw(ixBuffer) << Utility::intToStr(counter) << 
                left << setw(idBuffer) << driverIntStr << 
                left << setw(idBuffer) << riderIntStr << 
                left << setw(timeBuffer) << Utility::convertTimeTToString((*itr)->getETA()) << 
                left << setw(timeBuffer) << Utility::convertTimeTToString((*itr)->getETD()) << 
                left << setw(eventBuffer) << (*itr)->getMasterRequestEvent()->getEventStr() << 
                left << setw(eventBuffer) << (*itr)->getMasterDispatcEvent()->getEventStr() << std::endl;
        
        counter++;
    }
    
//    outFile << "initial requests:  " << Utility::intToStr()
    
    outFile << "\n\n\n\t(end of file)\n\n" << std::endl;
    outFile.close();
}


// ----------------------------------
//          PRINT SOLUTION 
// ----------------------------------
void Output::printSolution(Solution* pSolution, const ModelEnum &model) {
    
    std::string filename = "solution_";
    std::string modelname = "-";
    switch( model ) {
        case MITM_SEQ_PICKUPS :
            modelname = "MITM_seq";
            break;
        case UFBW_FIXED_PICKUPS :
            modelname = "UFBW_fixed";
            break;
        case UFBW_PICKUP_SWAPS : 
            modelname = "UFBW_pickUpSwaps";
            break;
        case UFBW_PERFECT_INFO :
            modelname = "UFBW_perfectInformation";
            break;
        case FLEX_DEPARTURE :
            modelname = "FlexDepartures";
            break;
        default :
            modelname += "other";
    } 
    
    filename += modelname; // + ".txt";
    //filename += ".txt";
    std::string outpath = _outputScenarioPath + filename;
    
    /*ofstream outFile(outpath.c_str());
    outFile << "\n----------------------------------------\n" << std::endl;
    outFile << "          solution for " << modelname << std::endl;
    outFile << "\n----------------------------------------\n\n" << std::endl;  */

    // if FD solution
    if( pSolution->getModel() == FLEX_DEPARTURE ) {
        
       /* FlexDepSolution * pFlexDepSoln = dynamic_cast<FlexDepSolution*>(pSolution);
        if( pFlexDepSoln != NULL ) {
          //  printDataSummary(pSolution, outpath);
            printSolutionSummary_FD(pFlexDepSoln, outpath, modelname);
            if( pSolution->getRequestMetrics()->_numMatchedRequests > 0 ) {
                printMatchTripsSummary_FD(pFlexDepSoln, outpath);
            }
            if( pSolution->getRequestMetrics()->_numUnmatchedRequests > 0 ) {
                printUnmatchedTripsSummary_FD(pFlexDepSoln, outpath);
            }
            if( pSolution->getRequestMetrics()->_numDisqualifiedRequests > 0 ) {
                printDisqualifiedRequestsSummary(pFlexDepSoln, outpath);
            }            
        } else {
            std::cout << "\n** ERROR: Solution cannot be cast as FlexDepSoluton **\n" << std::endl;
        }   */              
    }
    
    // all other solution
    else {    
        //printDataSummary(outFile, pSolution);
        printSolutionSummary(pSolution, outpath, modelname);
        if( pSolution->getRequestMetrics()->_numMatchedRequests > 0 ) {
            printMatchTripsSummary(pSolution, outpath);
        }
        if( pSolution->getRequestMetrics()->_numUnmatchedRequests > 0 ) {
            printUnmatchedTripsSummary(pSolution, outpath);
        }
        if( pSolution->getRequestMetrics()->_numDisqualifiedRequests > 0 ) {
            printDisqualifiedRequestsSummary(pSolution, outpath);
        }
    }
        
    
  //  outFile.close();
}
/*void Output::printDataSummary(ofstream& outFile, Solution* pSolution) {
    outFile << "\n\n--- INPUT DATA SUMMARY ---\n" << std::endl;
    
    int headerBuffer = 30;
    outFile << "\t" << left << setw(headerBuffer) << "start time: " << Utility::convertTimeTToString(pSolution->getStartTime()) << std::endl;
    outFile << "\t" << left << setw(headerBuffer) << "end time: " << Utility::convertTimeTToString(pSolution->getEndTime()) << std::endl;
    outFile << "\t" << left << setw(headerBuffer) << "total requests (proxy): " << Utility::intToStr(pSolution->getTotalRequests()) << std::endl;
    outFile << "\t" << left << setw(headerBuffer) << "total drivers: " << Utility::intToStr(pSolution->getTotalDrivers()) << std::endl;
    
    //const int totalRequests = pSolution->g
}*/
void Output::printSolutionSummary(Solution * pSolution, std::string &outpath, std::string &modelname ) {
  //  outFile << "\n\n--- SUMMARY METRICS ---\n" << std::endl;
    
    std::string outFilePath = outpath + "_SUMMARY.txt";
    ofstream outFile(outFilePath.c_str());
    
    outFile << "\n----------------------------------------\n" << std::endl;
    outFile << "          solution for " << modelname << std::endl;
    outFile << "\n----------------------------------------\n\n" << std::endl; 
    
    // print DATA SUMMARY
    outFile << "\n\n--- INPUT DATA SUMMARY ---\n" << std::endl;
    int headerBuffer = 30;
    outFile << "\t" << left << setw(headerBuffer) << "start time: " << Utility::convertTimeTToString(pSolution->getStartTime()) << std::endl;
    outFile << "\t" << left << setw(headerBuffer) << "end time: " << Utility::convertTimeTToString(pSolution->getEndTime()) << std::endl;
    outFile << "\t" << left << setw(headerBuffer) << "total requests (proxy): " << Utility::intToStr(pSolution->getTotalRequests()) << std::endl;
    outFile << "\t" << left << setw(headerBuffer) << "total drivers: " << Utility::intToStr(pSolution->getTotalDrivers()) << "\n\n" << std::endl;    
    
    int metricBuffer = 39;
    
    // print request summary
    const Solution::RequestMetrics * pRequestSummary = pSolution->getRequestMetrics();
    int totalRequests       = pRequestSummary->_totalRequests;
    int matchedRequests     = pRequestSummary->_numMatchedRequests;
    int unmatchedRequests   = pRequestSummary->_numUnmatchedRequests;
    int disqualRequests     = pRequestSummary->_numDisqualifiedRequests;
    double matchRate        = pRequestSummary->_matchedPercentage;
    
    outFile << "\n\n\tREQUEST SUMMARY" << std::endl;
    outFile << "\t\t" << left << setw(metricBuffer) << "total requests: " << Utility::intToStr(totalRequests) << std::endl;
    if( matchedRequests > 0 ) {
        outFile << "\t\t" << left << setw(metricBuffer) << "matched requests:"  << Utility::intToStr(matchedRequests) << std::endl;
    }
    if( unmatchedRequests > 0 ) {
        outFile << "\t\t" << left << setw(metricBuffer) << "unmatched requests: " << Utility::intToStr(unmatchedRequests) << std::endl;
    }
    if( disqualRequests > 0 ) {
        outFile << "\t\t" << left << setw(metricBuffer) << "disqualified requests: " << Utility::intToStr(disqualRequests) << std::endl;
    }
    outFile << "\t\t" << left << setw(metricBuffer) << "match rate: " << Utility::truncateDouble(matchRate,2) << "%" << std::endl;   
    outFile << "\n\t\t" << left << setw(metricBuffer) << "total trips: " << Utility::intToStr(pSolution->getTotalNumTripsFromSoln()) << std::endl;
    
    
    // print match quality summary
    if( matchedRequests > 0 ) {
        const Solution::MatchMetrics * pMetrics = pSolution->getMatchMetrics();
        
        int totalMatchedTrips = pMetrics->_numMatches;
        int numFIFOTrips = pMetrics->_numFIFOMatches;
        double pctFIFO = pMetrics->_pctFIFOMatches;
        int numFILOTrips = pMetrics->_numFILOMatches;
        double pctFILO = pMetrics->_pctFILOMatches;
        int numExtendedMatches = pMetrics->_numExtendedMatches;
        double pctExtended = pMetrics->_pctExtendedMatches;
        double pctFifoExt = pMetrics->_pctFIFOExtendedMatches;
        double pctFifoNonext = pMetrics->_pctFIFONonExtendedMatches;
        double pctFiloExt = pMetrics->_pctFILOExtendedMatches;
        double pctFiloNonext = pMetrics->_pctFILONonExtendedMatches;
        
        
        outFile << "\n\n\tMATCH METRICS" << std::endl;
        outFile << "\t\t" << left << setw(metricBuffer) << "total matches: " << Utility::intToStr(totalMatchedTrips) << std::endl;
        outFile << "\t\t" << left << setw(metricBuffer) << "number FIFO matches: " << left << setw(7) << Utility::intToStr(numFIFOTrips) << "  (" << Utility::truncateDouble(pctFIFO,2) << "%)" << std::endl;
        outFile << "\t\t" << left << setw(metricBuffer) << "number FILO matches: " << left << setw(7) << Utility::intToStr(numFILOTrips) << "  (" << Utility::truncateDouble(pctFILO,2) << "%)" << std::endl;
        outFile << "\n\t\t" << left << setw(metricBuffer) << "number extended matches: " << left << setw(7) << Utility::intToStr(numExtendedMatches) << "  (" << Utility::truncateDouble(pctExtended,2) << "%)" << std::endl;
        outFile << "\n\t\t" << left << setw(metricBuffer) << "share of FIFO extended matches: " << right << setw(7) << Utility::truncateDouble(pctFifoExt,2) << "%" << std::endl;
        outFile << "\t\t" << left << setw(metricBuffer) << "share of FIFO nonextended matches: " << right << setw(7) << Utility::truncateDouble(pctFifoNonext,2) << "%" << std::endl;
        outFile << "\t\t" << left << setw(metricBuffer) << "share of FILO extended matches" << right << setw(7) << Utility::truncateDouble(pctFiloExt,2) << "%" << std::endl;
        outFile << "\t\t" << left << setw(metricBuffer) << "share of FILO nonextended matches: " << right << setw(7) << Utility::truncateDouble(pctFiloNonext,2) << "%" << std::endl;        
    }
    
    // print inconvenience metrics
    if( matchedRequests > 0 ) {
        
        const Solution::MatchInconvenienceMetrics * pInconv = pSolution->getInconvenienceMetrics();
             
        outFile << "\n\n\tINCONVENIENCE METRICS" << std::endl;
        outFile << "\t\t" << left << setw(metricBuffer) << "avg inconvenience ALL riders: " << right << setw(7) << Utility::truncateDouble(pInconv->_avgPctAddedDistsForAll,2) << "%" << std::endl;
        outFile << "\t\t" << left << setw(metricBuffer) << "avg inconvenience MASTERS: "    << right << setw(7) << Utility::truncateDouble(pInconv->_avgPctAddedDistsForMasters,2) << "%" << std::endl;
        outFile << "\t\t" << left << setw(metricBuffer) << "avg inconvenience MINIONS: "    << right << setw(7) << Utility::truncateDouble(pInconv->_avgPctAddedDistsForMinions,2) << "%" << std::endl;
    }    
    
    outFile.close();
    
}
void Output::printMatchTripsSummary(Solution * pSolution, std::string &outpath ) {
    
   // outFile << "\n\n--- MATCH SOLUTION SUMMARY ---\n" << std::endl;    
    
    std::string outFilePath = outpath + "_MATCHED_TRIPS.txt";
    ofstream outFile(outFilePath.c_str());
    
    int ixBuff = 13;
    int isExtBuff = 9;
    int fifoBuff = 12;
    int geoBuff = 25;
    int distBuff = 14;
    int inconvBuff = 17;
    int uuidBuff = 45;
    int timeBuff = 25;
        
    outFile << left << setw(ixBuff) << "DRIVER" << 
            left << setw(ixBuff) << "MASTER_IX" << 
            left << setw(ixBuff) << "MINION_IX" << 
            left << setw(uuidBuff) << "MASTER_UUID" << 
            left << setw(uuidBuff) << "MINION_UUID" << 
            left << setw(timeBuff) << "MASTER_REQ_TIME" << 
            left << setw(timeBuff) << "MINION_REQ_TIME" << 
            left << setw(isExtBuff) << "EXT?" << 
            left << setw(fifoBuff) << "FIFO/FILO" << 
            left << setw(distBuff) << "DIST_TO_MIN" <<
            left << setw(distBuff) << "DIST_SHARED" << 
            left << setw(distBuff) << "DIST_DROP" <<
            left << setw(inconvBuff) << "DIFF_MAST_DIST" << 
            left << setw(inconvBuff) << "DIFF_MIN_DIST" << 
            left << setw(geoBuff) << "MASTER_ORIG" << 
            left << setw(geoBuff) << "MASTER_DEST" << 
            left << setw(geoBuff) << "MINION_ORIG" << 
            left << setw(geoBuff) << "MINION_DEST" << 
            left << setw(geoBuff) << "DRIVER_AT_MIN_REQ" << 
            left << setw(geoBuff) << "MASTER_AT_MIN_REQ" << 
            std::endl;
        
    const std::set<AssignedTrip*, AssignedTripIndexComp> * pMatchedTrips = pSolution->getMatchedTrips();
    std::set<AssignedTrip*, AssignedTripIndexComp>::const_iterator tripItr;
    for( tripItr = pMatchedTrips->begin(); tripItr != pMatchedTrips->end(); ++tripItr ) {
        
        std::string isExtStr = ((*tripItr)->getMatchDetails()->_masterPickedUpAtTimeOfMatch) ? "yes" : "no";
        std::string fifoFiloStr = ((*tripItr)->getMatchDetails()->_fixedDropoff) ? "FIFO" : "FILO";
        
        // get master orig & dest
        LatLng masterOrig = (*tripItr)->getMatchDetails()->_masterOrig;
        LatLng masterDest = (*tripItr)->getMatchDetails()->_masterDest;
        const std::string masterOrigStr = Utility::convertToLatLngStr(masterOrig, 5);
        const std::string masterDestStr = Utility::convertToLatLngStr(masterDest, 5);
                
        // get minion orig & dest
        LatLng minionOrig = (*tripItr)->getMatchDetails()->_minionOrig;
        LatLng minionDest = (*tripItr)->getMatchDetails()->_minionDest;        
        const std::string minionOrigStr = Utility::convertToLatLngStr(minionOrig, 5);
        const std::string minionDestStr = Utility::convertToLatLngStr(minionDest, 5);
        
        const int driverIndex = (*tripItr)->getDriver()->getIndex();
        int masterIndex  = (*tripItr)->getMatchDetails()->_masterIndex;
        int minionIndex  = (*tripItr)->getMatchDetails()->_minionIndex;        
        
        std::string addlDistMasterStr = Utility::truncateDouble((*tripItr)->getMatchDetails()->_pctAddlDistMaster, 2) + "%";
        std::string addlDistMinionStr = Utility::truncateDouble((*tripItr)->getMatchDetails()->_pctAddlDistMinion, 2) + "%";
        
        // driver, master locations at time of minion request
        std::string driverLocAtMinReq = Utility::convertToLatLngStr((*tripItr)->getMatchDetails()->_masterDriverLocAtTimeOfMinionReq, 5);
        std::string masterLocAtMinReq = Utility::convertToLatLngStr((*tripItr)->getMatchDetails()->_masterLocAtTimeOfMinionReq, 5);
        
        outFile << left << setw(ixBuff) << Utility::intToStr(driverIndex) << 
                left << setw(ixBuff) << Utility::intToStr(masterIndex) << 
                left << setw(ixBuff) << Utility::intToStr(minionIndex) << 
                left << setw(uuidBuff) << (*tripItr)->getMasterTripUUID() << 
                left << setw(uuidBuff) << (*tripItr)->getMinionTripUUID() <<
                left << setw(timeBuff) << Utility::convertTimeTToString((*tripItr)->getMatchDetails()->_masterRequest) << 
                left << setw(timeBuff) << Utility::convertTimeTToString((*tripItr)->getMatchDetails()->_minionRequest) <<
                left << setw(isExtBuff) << isExtStr << 
                left << setw(fifoBuff) << fifoFiloStr << 
                left << setw(distBuff) << Utility::truncateDouble((*tripItr)->getMatchDetails()->_distToMinionPickup,4) <<
                left << setw(distBuff) << Utility::truncateDouble((*tripItr)->getMatchDetails()->_sharedDistance,4) << 
                left << setw(distBuff) << Utility::truncateDouble((*tripItr)->getMatchDetails()->_distFromFirstToSecondDrop,4) << 
                left << setw(inconvBuff) << addlDistMasterStr << 
                left << setw(inconvBuff) << addlDistMinionStr <<
                left << setw(geoBuff) << masterOrigStr << 
                left << setw(geoBuff) << masterDestStr << 
                left << setw(geoBuff) << minionOrigStr << 
                left << setw(geoBuff) << minionDestStr <<
                left << setw(geoBuff) << driverLocAtMinReq << 
                left << setw(geoBuff) << masterLocAtMinReq <<
                std::endl;
        
    }  
    
    outFile.close();
}
void Output::printUnmatchedTripsSummary(Solution* pSolution, std::string &outpath ) {
    
   // outFile << "\n\n\n\n\n\n--- UNMATCHED MASTERS SUMMARY ---\n" << std::endl;    
    
    std::string outFilePath = outpath + "_UNMATCHED_TRIPS.txt";
    ofstream outFile(outFilePath.c_str());
    
    const std::set<AssignedTrip*, AssignedTripIndexComp> * pUnmatchedTrips = pSolution->getUnmatchedTrips();
    std::set<AssignedTrip*, AssignedTripIndexComp>::const_iterator tripItr;
    
    int ixBuff = 13;
    int timeBuff = 25;
    int geoBuff = 25;
    int distBuff = 15;
    int uuidBuff = 45;
    
    outFile << left << setw(ixBuff) << "DRIVER" << 
            left << setw(ixBuff) << "RIDER_IX" << 
            left << setw(uuidBuff) << "RIDER_UUID" << 
            left << setw(timeBuff) << "REQUEST_TIME" << 
            left << setw(geoBuff) << "PICKUP_LOC" << 
            left << setw(geoBuff) << "DROP_LOC" << 
            left << setw(timeBuff) << "PICKUP_TIME" << 
            left << setw(timeBuff) << "DROP_TIME" << 
            left << setw(distBuff) << "TRIP_DIST_KM" << 
            std::endl;
    
    for( tripItr = pUnmatchedTrips->begin(); tripItr != pUnmatchedTrips->end(); ++tripItr ) {               
        LatLng pickupLoc((*tripItr)->getMasterPickupEventFromActuals()->lat, (*tripItr)->getMasterPickupEventFromActuals()->lng);       
        LatLng dropLoc((*tripItr)->getMasterDropEventFromActuals()->lat, (*tripItr)->getMasterDropEventFromActuals()->lng);
        const double distKm = Utility::computeGreatCircleDistance(pickupLoc.getLat(), pickupLoc.getLng(), dropLoc.getLat(), dropLoc.getLng());        
                       
        outFile << left << setw(ixBuff) << Utility::intToStr((*tripItr)->getDriver()->getIndex()) << 
                left << setw(ixBuff) << Utility::intToStr((*tripItr)->getMasterIndex()) << 
                left << setw(uuidBuff) << (*tripItr)->getMasterTripUUID() <<
                left << setw(timeBuff) << Utility::convertTimeTToString((*tripItr)->getMasterRequestEvent()->timeT) << 
                left << setw(geoBuff) << Utility::convertToLatLngStr(pickupLoc, 5) << 
                left << setw(geoBuff) << Utility::convertToLatLngStr(dropLoc, 5) << 
                left << setw(timeBuff) << Utility::convertTimeTToString((*tripItr)->getMasterPickupEventFromActuals()->timeT) << 
                left << setw(timeBuff) << Utility::convertTimeTToString((*tripItr)->getMasterDropEventFromActuals()->timeT) << 
                left << setw(distBuff) << Utility::truncateDouble(distKm, 2) << 
                std::endl;        
    }
    
    outFile.close();
   
}
void Output::printDisqualifiedRequestsSummary(Solution* pSolution, std::string &outpath) {
    
    std::string outFilePath = outpath + "_DISQUALED_TRIPS.txt";
    ofstream outFile(outFilePath.c_str());
    
    //outFile << "\n\n\n\n\n\n--- DISQUALIFIED REQUESTS ---\n" << std::endl;
    
    const std::set<Request*, ReqComp> * pDisqualReqs = pSolution->getDisqualifiedRequests();
    std::set<Request*, ReqComp>::const_iterator reqItr;
    
    int ixBuff = 9;
    int timeBuff = 25;
    int locBuff = 25;
    int ynBuff = 11;
    
    outFile << left << setw(ixBuff) << "REQ_ID" << 
            left << setw(ixBuff) << "RIDER" << 
            left << setw(ynBuff) << "INIT_REQ?" << 
            left << setw(timeBuff) << "REQ_TIME" << 
            left << setw(locBuff) << "REQ_ORIGIN" << 
            left << setw(locBuff) << "REQ_DEST" << 
            left << setw(timeBuff) << "PICKUP_TIME_ACTUALS" << 
            left << setw(timeBuff) << "DROPOFF_TIME_ACTUALS" << 
            std::endl;
    
    
    for( reqItr = pDisqualReqs->begin(); reqItr != pDisqualReqs->end(); ++reqItr ) {
        
        LatLng origLoc((*reqItr)->getPickupLat(),(*reqItr)->getPickupLng());
        LatLng destLoc((*reqItr)->getDropoffLat(), (*reqItr)->getDropoffLng());
        std::string isInitStr = (*reqItr)->isInitRequest() ? "yes" : "no";
                
        outFile << left << setw(ixBuff) << Utility::intToStr((*reqItr)->getReqIndex()) << 
                left << setw(ixBuff) << Utility::intToStr((*reqItr)->getRiderIndex()) << 
                left << setw(ynBuff) << isInitStr << 
                left << setw(timeBuff) << Utility::convertTimeTToString((*reqItr)->getReqTime()) << 
                left << setw(locBuff) << Utility::convertToLatLngStr(origLoc, 5) << 
                left << setw(locBuff) << Utility::convertToLatLngStr(destLoc, 5) <<
                left << setw(timeBuff) << Utility::convertTimeTToString((*reqItr)->getActTimeOfPickupFromTripActuals()) << 
                left << setw(timeBuff) << Utility::convertTimeTToString((*reqItr)->getActTimeOfDropoffFromTripActuals()) <<                
                std::endl;
                
    }
    
    outFile.close();
    
}


/*void Output::printSolutionSummary_FD(FlexDepSolution * pFlexDepSoln, std::string &outpath, std::string &modelname ) {
   // outFile << "\n\n--- SUMMARY METRICS ---\n" << std::endl;
    std::string outFilePath = outpath + "_SUMMARY.txt";
    ofstream outFile(outFilePath.c_str());    
    
    outFile << "\n----------------------------------------\n" << std::endl;
    outFile << "          solution for " << modelname << std::endl;
    outFile << "\n----------------------------------------\n\n" << std::endl; 
        
    // print DATA SUMMARY
    // << "\n\n--- INPUT DATA SUMMARY ---\n" << std::endl;
    int headerBuffer = 30;
    outFile << "\t" << left << setw(headerBuffer) << "start time: " << Utility::convertTimeTToString(pFlexDepSoln->getStartTime()) << std::endl;
    outFile << "\t" << left << setw(headerBuffer) << "end time: " << Utility::convertTimeTToString(pFlexDepSoln->getEndTime()) << std::endl;
    outFile << "\t" << left << setw(headerBuffer) << "total requests (proxy): " << Utility::intToStr(pFlexDepSoln->getTotalRequests()) << std::endl;
    outFile << "\t" << left << setw(headerBuffer) << "total drivers: " << Utility::intToStr(pFlexDepSoln->getTotalDrivers()) << "\n\n" << std::endl;        
    
    
    int metricBuffer = 39;
    
    // print request summary
    const Solution::RequestMetrics * pRequestSummary = pFlexDepSoln->getRequestMetrics();
    const FlexDepSolution::RequestMetrics_FD    * pReqSum_FD    = pFlexDepSoln->getFlexDepReqMetrics();
    const FlexDepSolution::RequestMetrics_nonFD * pReqSum_nonFD = pFlexDepSoln->getNonFlexDepReqMetrics();
    
    int totalRequests       = pRequestSummary->_totalRequests;
    int totalReqs_FD        = pReqSum_FD->_totalRequests;
    int toalReqs_nonFD      = pReqSum_nonFD->_totalRequests;
    int matchedRequests     = pRequestSummary->_numMatchedRequests;
    int matchedReqs_FD      = pReqSum_FD->_matchedRequests;
    int matchedReqs_nonFD   = pReqSum_nonFD->_matchedRequests;
    int unmatchedRequests   = pRequestSummary->_numUnmatchedRequests;
    int unmatchedReqs_FD    = pReqSum_FD->_unmatchedRequests;
    int unmatchedReqs_nonFD = pReqSum_nonFD->_unmatchedRequests;
    int disqualRequests     = pRequestSummary->_numDisqualifiedRequests;
    double matchRate        = pRequestSummary->_matchedPercentage;
    double matchRate_FD     = pReqSum_FD->_matchPercentage;
    double matchRate_nonFD  = pReqSum_nonFD->_matchPercentage; 
     
    outFile << "\n\n\tREQUEST SUMMARY" << std::endl;
    outFile << "\t\t" << left << setw(metricBuffer) << "total requests: " << Utility::intToStr(totalRequests) << std::endl;
    outFile << "\t\t" << left << setw(metricBuffer) << "  flex dep reqs: " << "  " << Utility::intToStr(totalReqs_FD) << std::endl;
    outFile << "\t\t" << left << setw(metricBuffer) << "  non-FD reqs: "  << "  " << Utility::intToStr(toalReqs_nonFD) << std::endl;
    if( matchedRequests > 0 ) {
        outFile << "" << std::endl;
        outFile << "\t\t" << left << setw(metricBuffer) << "matched requests:"  << Utility::intToStr(matchedRequests) << std::endl;
        outFile << "\t\t" << left << setw(metricBuffer) << "  matched FD reqs: " << "  " << Utility::intToStr(matchedReqs_FD) << std::endl;
        outFile << "\t\t" << left << setw(metricBuffer) << "  matched non-FD reqs: " << "  " << Utility::intToStr(matchedReqs_nonFD) << std::endl;
    }
    if( unmatchedRequests > 0 ) {
        outFile << "" << std::endl;
        outFile << "\t\t" << left << setw(metricBuffer) << "unmatched requests: " << Utility::intToStr(unmatchedRequests) << std::endl;
        outFile << "\t\t" << left << setw(metricBuffer) << "  unmatched FD reqs: " << "  " << Utility::intToStr(unmatchedReqs_FD) << std::endl;
        outFile << "\t\t" << left << setw(metricBuffer) << "  unmatched non-FD reqs: " << "  " << Utility::intToStr(unmatchedReqs_nonFD) << std::endl;
    }
    if( disqualRequests > 0 ) {
        outFile << "\t\t" << left << setw(metricBuffer) << "disqualified requests: " << Utility::intToStr(disqualRequests) << std::endl;
    }
    outFile << "" << std::endl;
    outFile << "\t\t" << left << setw(metricBuffer) << "match rate: " << "  " << Utility::truncateDouble(matchRate,2) << "%" << std::endl;  
    outFile << "\t\t" << left << setw(metricBuffer) << "  FD req match rate: " << "  " << Utility::truncateDouble(matchRate_FD,2) << "%" << std::endl;
    outFile << "\t\t" << left << setw(metricBuffer) << "  non-FD req match rate: " << "  " << Utility::truncateDouble(matchRate_nonFD,2) << "%" << std::endl;
    
    outFile << "\n\t\t" << left << setw(metricBuffer) << "total trips: " << Utility::intToStr(pFlexDepSoln->getTotalNumTripsFromSoln()) << std::endl;
    
    
    // print match quality summary
    if( matchedRequests > 0 ) {
        const Solution::MatchMetrics * pMetrics = pFlexDepSoln->getMatchMetrics();
        
        int totalMatchedTrips = pMetrics->_numMatches;
        int numFIFOTrips = pMetrics->_numFIFOMatches;
        double pctFIFO = pMetrics->_pctFIFOMatches;
        int numFILOTrips = pMetrics->_numFILOMatches;
        double pctFILO = pMetrics->_pctFILOMatches;
        int numExtendedMatches = pMetrics->_numExtendedMatches;
        double pctExtended = pMetrics->_pctExtendedMatches;
        double pctFifoExt = pMetrics->_pctFIFOExtendedMatches;
        double pctFifoNonext = pMetrics->_pctFIFONonExtendedMatches;
        double pctFiloExt = pMetrics->_pctFILOExtendedMatches;
        double pctFiloNonext = pMetrics->_pctFILONonExtendedMatches;
        
        
        outFile << "\n\n\tMATCH METRICS" << std::endl;
        outFile << "\t\t" << left << setw(metricBuffer) << "total matches: " << Utility::intToStr(totalMatchedTrips) << std::endl;
        outFile << "\t\t" << left << setw(metricBuffer) << "number FIFO matches: " << left << setw(7) << Utility::intToStr(numFIFOTrips) << "  (" << Utility::truncateDouble(pctFIFO,2) << "%)" << std::endl;
        outFile << "\t\t" << left << setw(metricBuffer) << "number FILO matches: " << left << setw(7) << Utility::intToStr(numFILOTrips) << "  (" << Utility::truncateDouble(pctFILO,2) << "%)" << std::endl;
        outFile << "\n\t\t" << left << setw(metricBuffer) << "number extended matches: " << left << setw(7) << Utility::intToStr(numExtendedMatches) << "  (" << Utility::truncateDouble(pctExtended,2) << "%)" << std::endl;
        outFile << "\n\t\t" << left << setw(metricBuffer) << "share of FIFO extended matches: " << right << setw(7) << Utility::truncateDouble(pctFifoExt,2) << "%" << std::endl;
        outFile << "\t\t" << left << setw(metricBuffer) << "share of FIFO nonextended matches: " << right << setw(7) << Utility::truncateDouble(pctFifoNonext,2) << "%" << std::endl;
        outFile << "\t\t" << left << setw(metricBuffer) << "share of FILO extended matches" << right << setw(7) << Utility::truncateDouble(pctFiloExt,2) << "%" << std::endl;
        outFile << "\t\t" << left << setw(metricBuffer) << "share of FILO nonextended matches: " << right << setw(7) << Utility::truncateDouble(pctFiloNonext,2) << "%" << std::endl;        
    }
    
    // print inconvenience metrics
    if( matchedRequests > 0 ) {
        
        const Solution::MatchInconvenienceMetrics * pInconv = pFlexDepSoln->getInconvenienceMetrics();
             
        outFile << "\n\n\tINCONVENIENCE METRICS" << std::endl;
        outFile << "\t\t" << left << setw(metricBuffer) << "avg inconvenience ALL riders: " << right << setw(7) << Utility::truncateDouble(pInconv->_avgPctAddedDistsForAll,2) << "%" << std::endl;
        outFile << "\t\t" << left << setw(metricBuffer) << "avg inconvenience MASTERS: "    << right << setw(7) << Utility::truncateDouble(pInconv->_avgPctAddedDistsForMasters,2) << "%" << std::endl;
        outFile << "\t\t" << left << setw(metricBuffer) << "avg inconvenience MINIONS: "    << right << setw(7) << Utility::truncateDouble(pInconv->_avgPctAddedDistsForMinions,2) << "%" << std::endl;
    }   

    outFile.close();    
}
void Output::printMatchTripsSummary_FD(FlexDepSolution * pFDSolution, std::string &outpath) {
   // outFile << "\n\n--- MATCH SOLUTION SUMMARY ---\n" << std::endl;    
    
    std::string outFilePath = outpath + "_MATCHED_TRIPS.txt";
    ofstream outFile(outFilePath.c_str());
    
    int ixBuff = 13;
    int isFdBuff = 9;
    int isExtBuff = 9;
    int fifoBuff = 12;
    int geoBuff = 25;
    int distBuff = 14;
    int inconvBuff = 17;
    int uuidBuff = 45;
    int timeBuff = 25;
        
    outFile << left << setw(ixBuff) << "DRIVER" << 
            left << setw(ixBuff) << "MASTER_IX" << 
            left << setw(ixBuff) << "MINION_IX" << 
            left << setw(uuidBuff) << "MASTER_UUID" << 
            left << setw(uuidBuff) << "MINION_UUID" << 
            left << setw(timeBuff) << "MASTER_REQ_TIME" << 
            left << setw(timeBuff) << "MINION_REQ_TIME" << 
            left << setw(isFdBuff) << "mast_FD" << 
            left << setw(isFdBuff) << "min_FD?" <<
            left << setw(isExtBuff) << "EXT?" << 
            left << setw(fifoBuff) << "FIFO/FILO" << 
            left << setw(distBuff) << "DIST_TO_MIN" <<
            left << setw(distBuff) << "DIST_SHARED" << 
            left << setw(distBuff) << "DIST_DROP" <<
            left << setw(inconvBuff) << "DIFF_MAST_DIST" << 
            left << setw(inconvBuff) << "DIFF_MIN_DIST" << 
            left << setw(geoBuff) << "MASTER_ORIG" << 
            left << setw(geoBuff) << "MASTER_DEST" << 
            left << setw(geoBuff) << "MINION_ORIG" << 
            left << setw(geoBuff) << "MINION_DEST" << 
            std::endl;
    
    const std::set<const int> matchedFdReqIndices = pFDSolution->getMatchedFDReqIndices();
    const std::set<AssignedTrip*, AssignedTripIndexComp> * pMatchedTrips = pFDSolution->getMatchedTrips();
    std::set<AssignedTrip*, AssignedTripIndexComp>::const_iterator tripItr;
    for( tripItr = pMatchedTrips->begin(); tripItr != pMatchedTrips->end(); ++tripItr ) {
        
        std::set<const int>::const_iterator masterFdItr = matchedFdReqIndices.find((*tripItr)->getMatchDetails()->_masterReqIndex);
        std::set<const int>::const_iterator minionFdItr = matchedFdReqIndices.find((*tripItr)->getMatchDetails()->_minionReqIndex);
        
        std::string isExtStr = ((*tripItr)->getMatchDetails()->_masterPickedUpAtTimeOfMatch) ? "yes" : "no";
        std::string fifoFiloStr = ((*tripItr)->getMatchDetails()->_fixedDropoff) ? "FIFO" : "FILO";
        std::string isMasterFDStr = (masterFdItr != matchedFdReqIndices.end()) ? "yes" : "no";
        std::string isMinionFDStr = (minionFdItr != matchedFdReqIndices.end()) ? "yes" : "no";
        
        // get master orig & dest
        LatLng masterOrig = (*tripItr)->getMatchDetails()->_masterOrig;
        LatLng masterDest = (*tripItr)->getMatchDetails()->_masterDest;
        const std::string masterOrigStr = Utility::convertToLatLngStr(masterOrig, 5);
        const std::string masterDestStr = Utility::convertToLatLngStr(masterDest, 5);
                
        // get minion orig & dest
        LatLng minionOrig = (*tripItr)->getMatchDetails()->_minionOrig;
        LatLng minionDest = (*tripItr)->getMatchDetails()->_minionDest;        
        const std::string minionOrigStr = Utility::convertToLatLngStr(minionOrig, 5);
        const std::string minionDestStr = Utility::convertToLatLngStr(minionDest, 5);
        
        const int driverIndex = (*tripItr)->getDriver()->getIndex();
        int masterIndex  = (*tripItr)->getMatchDetails()->_masterIndex;
        int minionIndex  = (*tripItr)->getMatchDetails()->_minionIndex;
        
        std::string addlDistMasterStr = Utility::truncateDouble((*tripItr)->getMatchDetails()->_pctAddlDistMaster, 2) + "%";
        std::string addlDistMinionStr = Utility::truncateDouble((*tripItr)->getMatchDetails()->_pctAddlDistMinion, 2) + "%";
        
        outFile << left << setw(ixBuff) << Utility::intToStr(driverIndex) << 
                left << setw(ixBuff) << Utility::intToStr(masterIndex) << 
                left << setw(ixBuff) << Utility::intToStr(minionIndex) << 
                left << setw(uuidBuff) << (*tripItr)->getMasterTripUUID() << 
                left << setw(uuidBuff) << (*tripItr)->getMinionTripUUID() <<
                left << setw(timeBuff) << Utility::convertTimeTToString((*tripItr)->getMatchDetails()->_masterRequest) << 
                left << setw(timeBuff) << Utility::convertTimeTToString((*tripItr)->getMatchDetails()->_minionRequest) <<
                left << setw(isFdBuff) << isMasterFDStr << 
                left << setw(isFdBuff) << isMinionFDStr << 
                left << setw(isExtBuff) << isExtStr << 
                left << setw(fifoBuff) << fifoFiloStr << 
                left << setw(distBuff) << Utility::truncateDouble((*tripItr)->getMatchDetails()->_distToMinionPickup,4) <<
                left << setw(distBuff) << Utility::truncateDouble((*tripItr)->getMatchDetails()->_sharedDistance,4) << 
                left << setw(distBuff) << Utility::truncateDouble((*tripItr)->getMatchDetails()->_distFromFirstToSecondDrop,4) << 
                left << setw(inconvBuff) << addlDistMasterStr << 
                left << setw(inconvBuff) << addlDistMinionStr <<
                left << setw(geoBuff) << masterOrigStr << 
                left << setw(geoBuff) << masterDestStr << 
                left << setw(geoBuff) << minionOrigStr << 
                left << setw(geoBuff) << minionDestStr <<
                std::endl;
        
    }   
    
    outFile.close();
}
void Output::printUnmatchedTripsSummary_FD(FlexDepSolution * pFDSolution, std::string &outpath) {
   // outFile << "\n\n\n\n\n\n--- UNMATCHED MASTERS SUMMARY ---\n" << std::endl;    
    
    std::string outFilePath = outpath + "_UNMATCHED_TRIPS.txt";
    ofstream outFile(outFilePath.c_str());    
    
    const std::set<AssignedTrip*, AssignedTripIndexComp> * pUnmatchedTrips = pFDSolution->getUnmatchedTrips();
    std::set<AssignedTrip*, AssignedTripIndexComp>::const_iterator tripItr;
    
    int ixBuff = 13;
    int isFdBuff = 9;
    int timeBuff = 25;
    int geoBuff = 25;
    int distBuff = 15;
    int uuidBuff = 45;
    
    outFile << left << setw(ixBuff) << "DRIVER" << 
            left << setw(ixBuff) << "RIDER_IX" << 
            left << setw(uuidBuff) << "RIDER_UUID" << 
            left << setw(isFdBuff) << "IS_FD?" << 
            left << setw(timeBuff) << "REQUEST_TIME" << 
            left << setw(geoBuff) << "PICKUP_LOC" << 
            left << setw(geoBuff) << "DROP_LOC" << 
            left << setw(timeBuff) << "PICKUP_TIME" << 
            left << setw(timeBuff) << "DROP_TIME" << 
            left << setw(distBuff) << "TRIP_DIST_KM" << 
            std::endl;
    
    const std::set<const int> * pFDReqIndices = pFDSolution->getFlexDepReqIndices();
    
    for( tripItr = pUnmatchedTrips->begin(); tripItr != pUnmatchedTrips->end(); ++tripItr ) {               
        LatLng pickupLoc((*tripItr)->getMasterPickupEventFromActuals()->lat, (*tripItr)->getMasterPickupEventFromActuals()->lng);       
        LatLng dropLoc((*tripItr)->getMasterDropEventFromActuals()->lat, (*tripItr)->getMasterDropEventFromActuals()->lng);
        const double distKm = Utility::computeGreatCircleDistance(pickupLoc.getLat(), pickupLoc.getLng(), dropLoc.getLat(), dropLoc.getLng());   
        
        std::set<const int>::const_iterator fdItr = pFDReqIndices->find((*tripItr)->getMasterReqIndex());
        std::string isFdStr = (fdItr != pFDReqIndices->end()) ? "yes" : "no";
                       
        outFile << left << setw(ixBuff) << Utility::intToStr((*tripItr)->getDriver()->getIndex()) << 
                left << setw(ixBuff) << Utility::intToStr((*tripItr)->getMasterIndex()) << 
                left << setw(uuidBuff) << (*tripItr)->getMasterTripUUID() <<
                left << setw(isFdBuff) << isFdStr << 
                left << setw(timeBuff) << Utility::convertTimeTToString((*tripItr)->getMasterRequestEvent()->timeT) << 
                left << setw(geoBuff) << Utility::convertToLatLngStr(pickupLoc, 5) << 
                left << setw(geoBuff) << Utility::convertToLatLngStr(dropLoc, 5) << 
                left << setw(timeBuff) << Utility::convertTimeTToString((*tripItr)->getMasterPickupEventFromActuals()->timeT) << 
                left << setw(timeBuff) << Utility::convertTimeTToString((*tripItr)->getMasterDropEventFromActuals()->timeT) << 
                left << setw(distBuff) << Utility::truncateDouble(distKm, 2) << 
                std::endl;        
    }
    
    
    outFile.close();   
}*/

void Output::printSummaryOfDataInput(DataContainer * pDataContainer) {
    
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

// -------------------------------------------
//    PRINT SUMMARY SOLUTION FOR EXPERIMENT
// -------------------------------------------
void Output::printSolutionSummaryMetricsForCurrSolutions(const int &experiment, const std::map<double, SolnMaps*> * pModelSolnMap) {
  
    std::string outputPath  = this->_outputExperimentPath + "SUMMARY.txt";
    
    ofstream outFile;
    outFile.open(outputPath);
    
    std::set<double> inputRange = getKeyValues(pModelSolnMap);
    std::string inputValueStr = "";
    switch( experiment ) {
        case ModelRunner::DEFAULTVALUES : 
            inputValueStr = "default";
            break;
        case ModelRunner::OPTIN :
            inputValueStr = "opt-in";
            break;
        case ModelRunner::BATCHWINDOW : 
            inputValueStr = "batchLength";
            break;
        case ModelRunner::PICKUP :
            inputValueStr = "maxPickupDist";
            break;
        case ModelRunner::SAVINGSRATE :
            inputValueStr = "minSavingsPct";
            break;
        default :
            inputValueStr = "unknown";
    }
    
    
    outFile << "--------------------------------" << std::endl;
    outFile << "    summary for experiment" << std::endl;
    outFile << "--------------------------------\n\n" << std::endl;
    
    
    printInputRequestsMetrics( outFile, pModelSolnMap );    
    printMatchRateMetrics    ( outFile, inputValueStr, &inputRange, pModelSolnMap );
    printInconvenienceMetrics( outFile, inputValueStr, &inputRange, pModelSolnMap );
    printNumTripsMetrics     ( outFile, inputValueStr, &inputRange, pModelSolnMap ); 
    
    outFile << "\n\n-- end of file --\n" << std::endl;
}
void Output::printInputRequestsMetrics( std::ofstream &outFile, const std::map<double, SolnMaps*> * pModelSolnMap ) {
    std::string csvString = "CSV_numRequests";
    
    for( std::map<double, SolnMaps*>::const_iterator itr = pModelSolnMap->begin(); itr != pModelSolnMap->end(); ++itr ) {
        const int currNumReqs = itr->second->numRequests_inputs.begin()->second;
        csvString += "," + Utility::intToStr(currNumReqs);
    }
    
    outFile << "INSTANCE SIZES\n" << std::endl;
    outFile << csvString << std::endl;    
}


void Output::printMatchRateMetrics(std::ofstream& outFile, std::string inputName, std::set<double> * pInputRange, const std::map<double,SolnMaps*>* pModelSolnMap) {
    outFile << "\nMATCH RATE\n" << std::endl;
        
    std::string csv_mitm_str    = "CSV_MITM";
    std::string csv_ufbw_str    = "CSV_UFBW";
    std::string csv_flexDep_str = "CSV_FD";
    std::string csv_ufbwPI_str  = "CSV_UFBW_PI";
    
    std::string csv_FD_FDReqsStr = "";
    std::string csv_FD_nonFDReqsStr = "";
    
    std::string mitmMatchRateStr         = "-";
    std::string ufbwMatchRateStr         = "-";
    std::string flexDepMatchRateStr      = "-";
    std::string ufbwPerfInfoMatchRateStr = "-";
        
    // print models     
    outFile << left << setw(15) << inputName << left << setw(15) << "MITM" << left << setw(15) << "UFBW" << left << setw(15) << "FD" << left << setw(15) << "UFBW-PI" << std::endl;

  
    for( std::set<double>::iterator inputItr = pInputRange->begin(); inputItr != pInputRange->end(); ++inputItr ) {
        
        std::cout << "inputItr = " << *inputItr << std::endl;
        
        std::map<double, SolnMaps*>::const_iterator modelSolnMapItr = pModelSolnMap->find(*inputItr);
        
        std::map<ModelEnum, double> modelMatchRateMap = modelSolnMapItr->second->matchRateMap;
        double matchRate_FD_optIns = modelSolnMapItr->second->matchRate_FD_FDOptIn;
        double matchRate_FD_nonOptIns = modelSolnMapItr->second->matchRate_FD_nonFDOptIn;
        
        for( std::map<ModelEnum, double>::iterator mapItr = modelMatchRateMap.begin(); mapItr != modelMatchRateMap.end(); ++mapItr ) {
            switch( mapItr->first ) {
                case MITM_SEQ_PICKUPS :
                {
                    mitmMatchRateStr = Utility::truncateDouble(mapItr->second,4);
                    csv_mitm_str += "," + mitmMatchRateStr;
                    break;
                }
               /* case ModelEnum::MITM_PICKUP_SWAPS :
                {
                    
                }*/
                case UFBW_FIXED_PICKUPS :
                {
                    ufbwMatchRateStr = Utility::truncateDouble(mapItr->second,4);
                    csv_ufbw_str += "," + ufbwMatchRateStr;
                    break;
                }
                /*case ModelEnum::UFBW_PICKUP_SWAPS :
                {
                    
                }*/
                case UFBW_PERFECT_INFO :
                {
                    ufbwPerfInfoMatchRateStr = Utility::truncateDouble(mapItr->second,4); 
                    csv_ufbwPI_str += "," + ufbwPerfInfoMatchRateStr;    
                    break;
                }
                case FLEX_DEPARTURE :
                {
                    flexDepMatchRateStr = Utility::truncateDouble(mapItr->second,4);
                    csv_flexDep_str += "," + flexDepMatchRateStr;    
                    if( matchRate_FD_optIns > -1.0 ) {
                        csv_FD_FDReqsStr += Utility::truncateDouble(matchRate_FD_optIns,4);
                    }
                    if( matchRate_FD_nonOptIns > -1.0 ) {
                        csv_FD_nonFDReqsStr += Utility::truncateDouble(matchRate_FD_nonOptIns,4);
                    }
                    break;
                }
            }                        
        }
           
        outFile << left << setw(15) << Utility::doubleToStr(*inputItr) << left << setw(15) << mitmMatchRateStr << left << setw(15) << ufbwMatchRateStr << left << setw(15) << flexDepMatchRateStr << left << setw(15) << ufbwPerfInfoMatchRateStr << std::endl;
    }
    
    outFile << "\n" << std::endl;
    if( csv_mitm_str != "CSV_MITM" ) {
        outFile << csv_mitm_str << std::endl;
    }
    if( csv_ufbw_str != "CSV_UFBW" ) {
        outFile << csv_ufbw_str << std::endl;
    }
    if( csv_flexDep_str != "CSV_FD" ) {
        outFile << csv_flexDep_str << std::endl;
    }
    if( csv_ufbwPI_str != "CSV_UFBW_PI" ) {
        outFile << csv_ufbwPI_str << std::endl;
    }
            
    outFile << "\n\n" << std::endl;  
    
    if( csv_FD_FDReqsStr != "" ) {
        outFile << csv_FD_FDReqsStr << std::endl;
        outFile << csv_FD_nonFDReqsStr << std::endl;
    }
        
    outFile << "\n\n" << std::endl;  
    
}
void Output::printInconvenienceMetrics( std::ofstream &outFile, std::string inputName, std::set<double> * pInputRange, const std::map<double, SolnMaps*> * pModelSolnMap ) {
    outFile << "\nINCONVENIENCE RATE\n" << std::endl;
        
    std::string csv_mitm_str    = "CSV_MITM";
    std::string csv_ufbw_str    = "CSV_UFBW";
    std::string csv_flexDep_str = "CSV_FD";
    std::string csv_ufbwPI_str  = "CSV_UFBW_PI";
        
    std::string mitmMatchRateStr         = "-";
    std::string ufbwMatchRateStr         = "-";
    std::string flexDepMatchRateStr      = "-";
    std::string ufbwPerfInfoMatchRateStr = "-";
        
    // print models     
    outFile << left << setw(15) << inputName << left << setw(15) << "MITM" << left << setw(15) << "UFBW" << left << setw(15) << "FD" << left << setw(15) << "UFBW-PI" << std::endl;

  
    for( std::set<double>::iterator inputItr = pInputRange->begin(); inputItr != pInputRange->end(); ++inputItr ) {
        
        std::map<double, SolnMaps*>::const_iterator modelSolnMapItr = pModelSolnMap->find(*inputItr);
        
        std::map<ModelEnum, double> modelMatchRateMap = modelSolnMapItr->second->inconvMap;
         
        for( std::map<ModelEnum, double>::iterator mapItr = modelMatchRateMap.begin(); mapItr != modelMatchRateMap.end(); ++mapItr ) {
            switch( mapItr->first ) {
                case MITM_SEQ_PICKUPS :
                {
                    mitmMatchRateStr = Utility::truncateDouble(mapItr->second,4);
                    csv_mitm_str += "," + mitmMatchRateStr;
                    break;
                }
               /* case ModelEnum::MITM_PICKUP_SWAPS :
                {
                    
                }*/
                case UFBW_FIXED_PICKUPS :
                {
                    ufbwMatchRateStr = Utility::truncateDouble(mapItr->second,4);
                    csv_ufbw_str += "," + ufbwMatchRateStr;
                    break;
                }
                /*case ModelEnum::UFBW_PICKUP_SWAPS :
                {
                    
                }*/
                case UFBW_PERFECT_INFO :
                {
                    ufbwPerfInfoMatchRateStr = Utility::truncateDouble(mapItr->second,4); 
                    csv_ufbwPI_str += "," + ufbwPerfInfoMatchRateStr;   
                    break;
                }
                case FLEX_DEPARTURE :
                {
                    flexDepMatchRateStr = Utility::truncateDouble(mapItr->second,4);
                    csv_flexDep_str += "," + flexDepMatchRateStr;  
                    break;
                }
            }                        
        }
           
        outFile << left << setw(15) << Utility::doubleToStr(*inputItr) << left << setw(15) << mitmMatchRateStr << left << setw(15) << ufbwMatchRateStr << left << setw(15) << flexDepMatchRateStr << left << setw(15) << ufbwPerfInfoMatchRateStr << std::endl;
    }
    
    outFile << "\n" << std::endl;
    if( csv_mitm_str != "CSV_MITM" ) {
        outFile << csv_mitm_str  << std::endl;
    }
    if( csv_ufbw_str != "CSV_UFBW" ) {
        outFile << csv_ufbw_str << std::endl;
    }
    if( csv_flexDep_str != "CSV_FD" ) {
        outFile << csv_flexDep_str << std::endl;
    }
    if( csv_ufbwPI_str != "CSV_UFBW_PI" ) {
        outFile << csv_ufbwPI_str << std::endl;
    }
            
    outFile << "\n\n" << std::endl;  
        
}
void Output::printNumTripsMetrics(std::ofstream& outFile, std::string inputName, std::set<double>* pInputRange, const std::map<double,SolnMaps*>* pModelSolnMap) {
    outFile << "\nNUM TRIPS\n" << std::endl;
        
    std::string csv_mitm_str    = "CSV_MITM";
    std::string csv_ufbw_str    = "CSV_UFBW";
    std::string csv_flexDep_str = "CSV_FD";
    std::string csv_ufbwPI_str  = "CSV_UFBW_PI";
        
    std::string mitmMatchRateStr         = "-";
    std::string ufbwMatchRateStr         = "-";
    std::string flexDepMatchRateStr      = "-";
    std::string ufbwPerfInfoMatchRateStr = "-";
        
    // print models     
    outFile << left << setw(15) << inputName << left << setw(15) << "MITM" << left << setw(15) << "UFBW" << left << setw(15) << "FD" << left << setw(15) << "UFBW-PI" << std::endl;

  
    for( std::set<double>::iterator inputItr = pInputRange->begin(); inputItr != pInputRange->end(); ++inputItr ) {
        
        std::cout << "inputItr = " << *inputItr << std::endl;
        
        std::map<double, SolnMaps*>::const_iterator modelSolnMapItr = pModelSolnMap->find(*inputItr);
        
        std::map<ModelEnum, double> modelMatchRateMap = modelSolnMapItr->second->numTripsMap;
         
        for( std::map<ModelEnum, double>::iterator mapItr = modelMatchRateMap.begin(); mapItr != modelMatchRateMap.end(); ++mapItr ) {
            switch( mapItr->first ) {
                case MITM_SEQ_PICKUPS :
                {                    
                    mitmMatchRateStr = Utility::intToStr((int) mapItr->second);
                    csv_mitm_str += "," + mitmMatchRateStr;
                    break;
                }
               /* case ModelEnum::MITM_PICKUP_SWAPS :
                {
                    
                }*/
                case UFBW_FIXED_PICKUPS :
                {
                    ufbwMatchRateStr = Utility::intToStr((int) mapItr->second);
                    csv_ufbw_str += "," + ufbwMatchRateStr;
                    break;
                }
                /*case ModelEnum::UFBW_PICKUP_SWAPS :
                {
                    
                }*/
                case UFBW_PERFECT_INFO :
                {
                    ufbwPerfInfoMatchRateStr = Utility::intToStr((int) mapItr->second); 
                    csv_ufbwPI_str += "," + ufbwPerfInfoMatchRateStr;  
                    break;
                }
                case FLEX_DEPARTURE :
                {
                    flexDepMatchRateStr = Utility::intToStr((int) mapItr->second);
                    csv_flexDep_str += "," + flexDepMatchRateStr;  
                    break;
                }
            }                        
        }
           
        outFile << left << setw(15) << Utility::doubleToStr(*inputItr) << left << setw(15) << mitmMatchRateStr << left << setw(15) << ufbwMatchRateStr << left << setw(15) << flexDepMatchRateStr << left << setw(15) << ufbwPerfInfoMatchRateStr << std::endl;
    }
    
    outFile << "\n" << std::endl;
    if( csv_mitm_str != "CSV_MITM" ) {
        outFile << csv_mitm_str  << std::endl;
    }
    if( csv_ufbw_str != "CSV_UFBW" ) {
        outFile << csv_ufbw_str << std::endl;
    }
    if( csv_flexDep_str != "CSV_FD" ) {
        outFile << csv_flexDep_str << std::endl;
    }
    if( csv_ufbwPI_str != "CSV_UFBW_PI" ) {
        outFile << csv_ufbwPI_str << std::endl;
    }
            
    outFile << "\n\n" << std::endl;  
           
}

std::set<double> Output::getKeyValues( const std::map<double, SolnMaps*> * pModelSolnMap ) {
    std::set<double> keyValues;
    
    for( std::map<double, SolnMaps*>::const_iterator mapItr = pModelSolnMap->begin(); mapItr != pModelSolnMap->end(); ++mapItr ) {
        keyValues.insert(mapItr->first);
    }
    
    return keyValues;
}