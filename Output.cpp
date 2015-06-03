/* 
 * File:   Output.cpp
 * Author: jonpetersen
 * 
 * Created on March 23, 2015, 2:06 PM
 */

#include "Output.hpp"
#include "ModelRunner.hpp"
#include "UserConfig.hpp"


Output::Output(const std::string outputBasePath, const std::string outputExperimentPath, const bool printIndivMetrics) : 
    _outputBasePath(outputBasePath), _outputExperimentPath(outputExperimentPath), _printIndivMetrics(printIndivMetrics) {  
    
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
    
    
    outFile <<
            left << setw(indexBuffer) << "index" << 
            left << setw(statusBuffer) << "status" << 
            left << setw(eventBuffer) << "requestEvent" << 
            left << setw(eventBuffer) << "dispatchEvent" << 
            left << setw(eventBuffer) << "pickupEvent" << 
            left << setw(eventBuffer) << "dropoffEvent" << std::endl;
    outFile << "\n" << std::endl;
    
    const std::vector<TripData*>* pTrips = pDataContainer->getAllTrips();
    for( std::vector<TripData*>::const_iterator tripItr = pTrips->begin(); tripItr != pTrips->end(); ++tripItr ) {
        
        outFile << 
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
    outFile << "\n" << left << setw(indexBuffer) << "tripIx" << 
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
        outFile << left << setw(indexBuffer) << Utility::intToStr((*iReq)->getReqIndex()) << 
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
    std::set<OpenTrip*, EtdComp> initOpenTrips = pDataContainer->getInitOpenTripsAtTimeline();
    std::set<OpenTrip*, EtdComp>::iterator itr;
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
        case UFBW_PERFECT_INFO :
            modelname = "UFBW_perfectInformation";
            break;
        case FLEX_DEPARTURE :
            modelname = "FlexDepartures";
            break;
        case MULTIPLE_PICKUPS :
            modelname = "MultPickups";
            break;
        default :
            modelname += "other";
    } 
    
    filename += modelname;
    std::string outpath = _outputScenarioPath + filename;
    
    MultPickupSoln * pMultPickupSoln = dynamic_cast<MultPickupSoln*>(pSolution);
    bool isMultPickupSoln = (pMultPickupSoln != NULL);
    
    printSolutionSummary(pSolution, outpath, modelname);
    if( pSolution->getRequestMetrics()->_numMatchedRequests > 0 ) {
        // if 3-rider solution, call its own print method         
        if( isMultPickupSoln ) {
            printMultPickupMatchTripSummary(pMultPickupSoln, outpath);
        } else {
            printMatchTripSummary(pSolution, outpath);
        }
    }
    
    if( pSolution->getRequestMetrics()->_numUnmatchedRequests > 0 ) {
        if( isMultPickupSoln ) {
            printMultPickupUnmatchedTripSummary(pMultPickupSoln, outpath);
        } else {
            printUnmatchedTripsSummary(pSolution, outpath);
        }        
    }
    if( pSolution->getRequestMetrics()->_numDisqualifiedRequests > 0 ) {
        printDisqualifiedRequestsSummary(pSolution, outpath);
    }
    
    if( _printIndivMetrics ) {
        printIndivSolnMetrics(pSolution, outpath);
    }    
}
void Output::printSolutionSummary(Solution* pSolution, std::string& outpath, std::string& modelname) {
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
    
    // check if the solution corresponds to a flex departure model
    FlexDepSolution * pFlexDepSoln = dynamic_cast<FlexDepSolution*>(pSolution);
    const bool isFlexDepSoln = (pFlexDepSoln != NULL);
    const FlexDepSolution::RequestMetrics_FD * pReqSum_FD = (pFlexDepSoln != NULL) ? pFlexDepSoln->getFlexDepReqMetrics() : NULL;
    const FlexDepSolution::RequestMetrics_nonFD * pReqSum_nonFD = (pFlexDepSoln != NULL) ? pFlexDepSoln->getNonFlexDepReqMetrics() : NULL;
      
    
    // print request summary
    const Solution::RequestMetrics * pRequestSummary = pSolution->getRequestMetrics();
    int totalRequests       = pRequestSummary->_totalRequests;
    int matchedRequests     = pRequestSummary->_numMatchedRequests;
    int unmatchedRequests   = pRequestSummary->_numUnmatchedRequests;
    int disqualRequests     = pRequestSummary->_numDisqualifiedRequests;
    double matchRate        = pRequestSummary->_matchedPercentage;
    
    outFile << "\n\n\tREQUEST SUMMARY\n" << std::endl;
    outFile << "\t\t" << left << setw(metricBuffer) << "total requests: " << Utility::intToStr(totalRequests) << std::endl;
    if( isFlexDepSoln ) {
        outFile << "\t\t" << left << setw(metricBuffer) << "  FD reqs: " << "  " << Utility::intToStr(pReqSum_FD->_totalRequests) << std::endl;
        outFile << "\t\t" << left << setw(metricBuffer) << "  non-FD reqs: "  << "  " << Utility::intToStr(pReqSum_nonFD->_totalRequests) << "\n" << std::endl;        
    }
    
    
    if( matchedRequests > 0 ) {
        outFile << "\t\t" << left << setw(metricBuffer) << "matched requests:"  << Utility::intToStr(matchedRequests) << std::endl;
        if( isFlexDepSoln ) {
            outFile << "\t\t" << left << setw(metricBuffer) << "  matched FD reqs: " << "  " << Utility::intToStr(pReqSum_FD->_matchedRequests) << std::endl;
            outFile << "\t\t" << left << setw(metricBuffer) << "  matched non-FD reqs: " << "  " << Utility::intToStr(pReqSum_nonFD->_matchedRequests) << "\n" << std::endl;            
        }
    }
    if( unmatchedRequests > 0 ) {
        outFile << "\t\t" << left << setw(metricBuffer) << "unmatched requests: " << Utility::intToStr(unmatchedRequests) << std::endl;
        if( isFlexDepSoln ) {
            outFile << "\t\t" << left << setw(metricBuffer) << "  unmatched FD reqs: " << "  " << Utility::intToStr(pReqSum_FD->_unmatchedRequests) << std::endl;
            outFile << "\t\t" << left << setw(metricBuffer) << "  unmatched non-FD reqs: " << "  " << Utility::intToStr(pReqSum_nonFD->_unmatchedRequests) << "\n" << std::endl;            
        }
    }
    if( disqualRequests > 0 ) {
        outFile << "\t\t" << left << setw(metricBuffer) << "disqualified requests: " << Utility::intToStr(disqualRequests) << std::endl;
        if( isFlexDepSoln ) {
            outFile << "  " << std::endl;
        }
    }
    
    outFile << "\t\t" << left << setw(metricBuffer) << "match rate: " << Utility::truncateDouble(matchRate,2) << "%" << std::endl;   
    if( isFlexDepSoln ) {
        outFile << "\t\t" << left << setw(metricBuffer) << "  FD req match rate: " << "  " << Utility::truncateDouble(pReqSum_FD->_matchPercentage,2) << "%" << std::endl;
        outFile << "\t\t" << left << setw(metricBuffer) << "  non-FD req match rate: " << "  " << Utility::truncateDouble(pReqSum_nonFD->_matchPercentage,2) << "% \n" << std::endl;
    }
    
    outFile << "\t\t" << left << setw(metricBuffer) << "total trips: " << Utility::intToStr(pSolution->getTotalNumTripsFromSoln()) << std::endl;
    
    
    // print match quality summary
    if( matchedRequests > 0 ) {
        const Solution::MatchMetrics * pMetrics = pSolution->getMatchMetrics();
        
        const int totalMatchedTrips = pMetrics->_numMatches;
        const int numFIFOTrips = pMetrics->_numFIFOMatches;
        const double pctFIFO = pMetrics->_pctFIFOMatches;
        const int numFILOTrips = pMetrics->_numFILOMatches;
        const double pctFILO = pMetrics->_pctFILOMatches;
        const int numExtendedMatches = pMetrics->_numExtendedMatches;
        const double pctExtended = pMetrics->_pctExtendedMatches;
        const double pctFifoExt = pMetrics->_pctFIFOExtendedMatches;
        const double pctFifoNonext = pMetrics->_pctFIFONonExtendedMatches;
        const double pctFiloExt = pMetrics->_pctFILOExtendedMatches;
        const double pctFiloNonext = pMetrics->_pctFILONonExtendedMatches;
        const double meanWaitSecAll = pMetrics->_avgWaitTimeOfMatchesForAllMatchedRiders;
        const double meanWaitSecMasters = pMetrics->_avgWaitTimeOfMatchesForMasters;
        const double meanWaitSecMinions = pMetrics->_avgWaitTimeOfMatchesForMinions;
        
        
        outFile << "\n\n\tMATCH METRICS" << std::endl;
        outFile << "\t\t" << left << setw(metricBuffer) << "total matches: " << Utility::intToStr(totalMatchedTrips) << std::endl;
        outFile << "\t\t" << left << setw(metricBuffer) << "number FIFO matches: " << left << setw(7) << Utility::intToStr(numFIFOTrips) << "  (" << Utility::truncateDouble(pctFIFO,2) << "%)" << std::endl;
        outFile << "\t\t" << left << setw(metricBuffer) << "number FILO matches: " << left << setw(7) << Utility::intToStr(numFILOTrips) << "  (" << Utility::truncateDouble(pctFILO,2) << "%)" << std::endl;
        outFile << "\n\t\t" << left << setw(metricBuffer) << "number extended matches: " << left << setw(7) << Utility::intToStr(numExtendedMatches) << "  (" << Utility::truncateDouble(pctExtended,2) << "%)" << std::endl;
        outFile << "\n\t\t" << left << setw(metricBuffer) << "share of FIFO extended matches: " << right << setw(7) << Utility::truncateDouble(pctFifoExt,2) << "%" << std::endl;
        outFile << "\t\t" << left << setw(metricBuffer) << "share of FIFO nonextended matches: " << right << setw(7) << Utility::truncateDouble(pctFifoNonext,2) << "%" << std::endl;
        outFile << "\t\t" << left << setw(metricBuffer) << "share of FILO extended matches" << right << setw(7) << Utility::truncateDouble(pctFiloExt,2) << "%" << std::endl;
        outFile << "\t\t" << left << setw(metricBuffer) << "share of FILO nonextended matches: " << right << setw(7) << Utility::truncateDouble(pctFiloNonext,2) << "%" << std::endl; 
        outFile << "\n\t\t" << left << setw(metricBuffer) << "mean wait time of all matches: " << left << setw(7) << Utility::truncateDouble(meanWaitSecAll,2) << std::endl;
        outFile << "\t\t" << left << setw(metricBuffer) << "mean wait time of matches (masters): " << left << setw(7) << Utility::truncateDouble(meanWaitSecMasters,2) << std::endl;
        outFile << "\t\t" << left << setw(metricBuffer) << "mean wait time of matches (minions): " << left << setw(7) << Utility::truncateDouble(meanWaitSecMinions,2) << std::endl;
    }
    
    // print inconvenience metrics
    if( matchedRequests > 0 ) {
        
        const Solution::MatchInconvenienceMetrics * pInconv = pSolution->getInconvenienceMetrics();
        const Solution::SavingsMetrics * pSavings = pSolution->getSavingsMetrics();
        const Solution::MatchOverlapMetrics * pOverlap = pSolution->getOverlapMetrics();
             
        outFile << "\n\n\tINCONVENIENCE METRICS" << std::endl;
        outFile << "\t\t" << left << setw(metricBuffer) << "avg inconvenience ALL riders: " << right << setw(7) << Utility::truncateDouble(pInconv->_avgPctAddedDistsForAll,2) << "%" << std::endl;
        outFile << "\t\t" << left << setw(metricBuffer) << "avg inconvenience MASTERS: "    << right << setw(7) << Utility::truncateDouble(pInconv->_avgPctAddedDistsForMasters,2) << "%" << std::endl;
        outFile << "\t\t" << left << setw(metricBuffer) << "avg inconvenience MINIONS: "    << right << setw(7) << Utility::truncateDouble(pInconv->_avgPctAddedDistsForMinions,2) << "%" << std::endl;
        
        outFile << "\n\n\tMATCH OVERLAP METRICS" << std::endl;
        outFile << "\t\t" << left << setw(metricBuffer) << "avg overlap dist (km): "      << left << setw(7) << Utility::truncateDouble(pOverlap->_avgOverlapDist, 2) << std::endl;
        outFile << "\t\t" << left << setw(metricBuffer) << "avg pct trip overlap: "       << Utility::truncateDouble(pOverlap->_avgTripOverlapPct,2) << "%" << std::endl;
        outFile << "\t\t" << left << setw(metricBuffer) << "avg pct overlap all riders: " <<  Utility::truncateDouble(pOverlap->_avgPctOverlapAll, 2) << "%" << std::endl;
        outFile << "\t\t" << left << setw(metricBuffer) << "avg pct overlap of MASTERS: " << right << setw(7) << Utility::truncateDouble(pOverlap->_avgPctOverlapMasters, 2) << "%" << std::endl;
        outFile << "\t\t" << left << setw(metricBuffer) << "avg pct overlap of MINIONS: " << right << setw(7) << Utility::truncateDouble(pOverlap->_avgPctOverlapMinions, 2) << "%" << std::endl;
        
        outFile << "\n\n\tSAVINGS METRICS" << std::endl;
        outFile << "\t\t" << left << setw(metricBuffer) << "avg savings of ALL riders: " << right << setw(7) << Utility::truncateDouble(pSavings->_avgMatchedRiderSavingsPct,2) << "%" << std::endl;
        outFile << "\t\t" << left << setw(metricBuffer) << "avg savings of MASTERS:    " << right << setw(7) << Utility::truncateDouble(pSavings->_avgMasterSavingsPct,2) << "%" << std::endl;
        outFile << "\t\t" << left << setw(metricBuffer) << "avg savings of MINIONS:    " << right << setw(7) << Utility::truncateDouble(pSavings->_avgMinionSavingsPct,2) << "%" << std::endl;
    }    
    
    outFile.close();       
}
void Output::printMatchTripSummary(Solution * pSolution, std::string &outpath) {
    
   // outFile << "\n\n--- MATCH SOLUTION SUMMARY ---\n" << std::endl;    
    
    std::string outFilePath = outpath + "_MATCHED_TRIPS.txt";
    ofstream outFile(outFilePath.c_str());
    
    // check if solution corresponds to FD model
    FlexDepSolution * pFDSoln = dynamic_cast<FlexDepSolution*>(pSolution);
    const bool isFDSoln = (pFDSoln != NULL);
   
    int ixBuff = 13;
    int isExtBuff = 9;
    int fifoBuff = 12;
    int geoBuff = 25;
    int distBuff = 14;
    int inconvBuff = 17;
    int uuidBuff = 45;
    int timeBuff = 25;
    int isFdBuff = 9; 
            
    outFile << left << setw(ixBuff) << "DRIVER" << 
        left << setw(ixBuff) << "MASTER_IX" << 
        left << setw(ixBuff) << "MINION_IX" << 
        left << setw(uuidBuff) << "MASTER_UUID" << 
        left << setw(uuidBuff) << "MINION_UUID" << 
        left << setw(timeBuff) << "MASTER_REQ_TIME" << 
        left << setw(timeBuff) << "MINION_REQ_TIME";        
        if( isFDSoln ) {
            outFile << left << setw(isFdBuff) << "MAST_FD" << 
                       left << setw(isFdBuff) << "MIN_FD";
        }        
    outFile << left << setw(isExtBuff) << "EXT?" << 
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
        const std::string masterOrigStr = Utility::convertToLatLngStr(masterOrig, 6);
        const std::string masterDestStr = Utility::convertToLatLngStr(masterDest, 6);
                
        // get minion orig & dest
        LatLng minionOrig = (*tripItr)->getMatchDetails()->_minionOrig;
        LatLng minionDest = (*tripItr)->getMatchDetails()->_minionDest;        
        const std::string minionOrigStr = Utility::convertToLatLngStr(minionOrig, 6);
        const std::string minionDestStr = Utility::convertToLatLngStr(minionDest, 6);
        
        const int driverIndex = (*tripItr)->getDriver()->getIndex();
        int masterIndex  = (*tripItr)->getMatchDetails()->_masterIndex;
        int minionIndex  = (*tripItr)->getMatchDetails()->_minionIndex;        
        
        std::string addlDistMasterStr = Utility::truncateDouble((*tripItr)->getMatchDetails()->_pctAddlDistMaster, 2) + "%";
        std::string addlDistMinionStr = Utility::truncateDouble((*tripItr)->getMatchDetails()->_pctAddlDistMinion, 2) + "%";
        
        // driver, master locations at time of minion request
        std::string driverLocAtMinReq = Utility::convertToLatLngStr((*tripItr)->getMatchDetails()->_masterDriverLocAtTimeOfMinionReq, 5);
        std::string masterLocAtMinReq = Utility::convertToLatLngStr((*tripItr)->getMatchDetails()->_masterLocAtTimeOfMinionReq, 5);
        
        // check if whether/not rider is flex dep opt-in
        outFile << left << setw(ixBuff) << Utility::intToStr(driverIndex) << 
                left << setw(ixBuff) << Utility::intToStr(masterIndex) << 
                left << setw(ixBuff) << Utility::intToStr(minionIndex) << 
                left << setw(uuidBuff) << (*tripItr)->getMasterTripUUID() << 
                left << setw(uuidBuff) << (*tripItr)->getMinionTripUUID() <<
                left << setw(timeBuff) << Utility::convertTimeTToString((*tripItr)->getMatchDetails()->_masterRequest) << 
                left << setw(timeBuff) << Utility::convertTimeTToString((*tripItr)->getMatchDetails()->_minionRequest);
        if( isFDSoln ) {
            const std::set<const int> matchedFdReqIndices = pFDSoln->getMatchedFDReqIndices();
            std::set<const int>::const_iterator masterFdItr = matchedFdReqIndices.find((*tripItr)->getMatchDetails()->_masterReqIndex);
            std::set<const int>::const_iterator minionFdItr = matchedFdReqIndices.find((*tripItr)->getMatchDetails()->_minionReqIndex);            
            std::string isMasterFDStr = (masterFdItr != matchedFdReqIndices.end()) ? "yes" : "no";
            std::string isMinionFDStr = (minionFdItr != matchedFdReqIndices.end()) ? "yes" : "no";
            outFile << left << setw(isFdBuff) << isMasterFDStr << 
                    left << setw(isFdBuff) << isMinionFDStr;
        }
        outFile << left << setw(isExtBuff) << isExtStr << 
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
// print matches from multiple pickup model
void Output::printMultPickupMatchTripSummary(MultPickupSoln* pMultPickupSoln, std::string& outpath) {
    std::string outFilePath = outpath + "_MATCHED_TRIPS.txt";
    ofstream outFile(outFilePath.c_str());
    
    int ctBuff = 7;
    int ixBuff = 13;
    int isExtBuff = 9;
    int fifoBuff = 12;
    int geoBuff = 25;
    int distBuff = 14;
    int inconvBuff = 17;
    int uuidBuff = 45;
    int timeBuff = 25;
    int isFdBuff = 9; 
    
    outFile <<
            left << setw(ctBuff) << "rtIx" <<
            left << setw(ixBuff) << "pickup_1" <<
            left << setw(uuidBuff) << "pick_1_uuid" << 
            left << setw(timeBuff) << "reqTime_1" << 
            left << setw(timeBuff) << "pickupTime_1" << 
            left << setw(geoBuff) << "pickupLoc_1" << 
            left << setw(ixBuff) << "pickup_2" << 
            left << setw(uuidBuff) << "pick_2_uuid" << 
            left << setw(timeBuff) << "reqTime_2" << 
            left << setw(timeBuff) << "pickupTime_2" <<
            left << setw(geoBuff) << "pickupLoc_2" << 
            left << setw(ixBuff) << "rider_3" << 
            left << setw(uuidBuff) << "pick_3_uuid" << 
            left << setw(timeBuff) << "reqTime_3" << 
            left << setw(timeBuff) << "pickupTime_3" <<
            left << setw(geoBuff) << "pickupLoc_3" << 
            left << setw(ixBuff) << "drop_1" << 
            left << setw(uuidBuff) << "drop_1_uuid" <<
            left << setw(timeBuff) << "dropTime_1" << 
            left << setw(geoBuff) << "dropLoc_1" << 
            left << setw(ixBuff) << "drop_2" << 
            left << setw(uuidBuff) << "drop_2_uuid" << 
            left << setw(timeBuff) << "dropTime_2" << 
            left << setw(geoBuff) << "dropLoc_2" << 
            left << setw(ixBuff) << "drop_3" << 
            left << setw(uuidBuff) << "drop_3_uuid" <<
            left << setw(timeBuff) << " dropTime_3" << 
            left << setw(ixBuff) << "dropLoc_3" << 
    endl;
    
    std::set<AssignedRoute*, AssignedRouteIndexComp> * pMatchedRoutes = pMultPickupSoln->getMatchedRoutes();
    std::set<AssignedRoute*, AssignedRouteIndexComp>::iterator routeItr;
    for( routeItr = pMatchedRoutes->begin(); routeItr != pMatchedRoutes->end(); ++routeItr ) {
        Route * pRoute = (*routeItr)->getRoute();
        const std::vector<RouteEvent*> * pPickupEvents = pRoute->getPickupEvents();
        RouteEvent * pickup1 = (pPickupEvents->size() >= 1) ? pPickupEvents->at(0) : NULL;
        RouteEvent * pickup2 = (pPickupEvents->size() >= 2) ? pPickupEvents->at(1) : NULL;
        RouteEvent * pickup3 = (pPickupEvents->size() >= 3) ? pPickupEvents->at(2) : NULL;
        
        // pickup 1 info
        std::string p1_rider_str = (pickup1 == NULL) ? "-" : Utility::intToStr(pickup1->getRequest()->getRiderIndex());
        std::string p1_rider_uuid = (pickup1 == NULL) ? "-" : pickup1->getRequest()->getRiderTripUUID();
        std::string p1_req_str = (pickup1 == NULL) ? "-" : Utility::convertTimeTToString(pickup1->getRequest()->getReqTime());
        std::string p1_pick_str = (pickup1 == NULL) ? "-" : Utility::convertTimeTToString(pickup1->getEventTime());
        std::string p1_loc_str = (pickup1 == NULL) ? "-" : Utility::convertToLatLngStr(*(pickup1->getLocation()),6);
        
        // pickup 2 info
        std::string p2_rider_str = (pickup2 == NULL) ? "-" : Utility::intToStr(pickup2->getRequest()->getRiderIndex());
        std::string p2_rider_uuid = (pickup2 == NULL) ? "-" : pickup2->getRequest()->getRiderTripUUID();
        std::string p2_req_str = (pickup2 == NULL) ? "-" : Utility::convertTimeTToString(pickup2->getRequest()->getReqTime());
        std::string p2_pick_str = (pickup2 == NULL) ? "-" : Utility::convertTimeTToString(pickup2->getEventTime());
        std::string p2_loc_str = (pickup2 == NULL) ? "-" : Utility::convertToLatLngStr(*(pickup2->getLocation()),6);
        
        // pickup 3 info
        std::string p3_rider_str = (pickup3 == NULL) ? "-" : Utility::intToStr(pickup3->getRequest()->getRiderIndex());
        std::string p3_rider_uuid = (pickup3 == NULL) ? "-" : pickup3->getRequest()->getRiderTripUUID();
        std::string p3_req_str = (pickup3 == NULL) ? "-" : Utility::convertTimeTToString(pickup3->getRequest()->getReqTime());
        std::string p3_pick_str = (pickup3 == NULL) ? "-" : Utility::convertTimeTToString(pickup3->getEventTime());
        std::string p3_loc_str = (pickup3 == NULL) ? "-" : Utility::convertToLatLngStr(*(pickup3->getLocation()),6);
        
        // drop 1 info
        const std::vector<RouteEvent*> * pDropEvents = (*routeItr)->getRoute()->getDropoffEvents();
        RouteEvent * drop1 = (pDropEvents->size() >= 1) ? pDropEvents->at(0) : NULL;
        RouteEvent * drop2 = (pDropEvents->size() >= 2) ? pDropEvents->at(1) : NULL;
        RouteEvent * drop3 = (pDropEvents->size() >= 3) ? pDropEvents->at(2) : NULL;
        
        // drop 1 info
        std::string d1_rider_str = (drop1 == NULL) ? "-" : Utility::intToStr(drop1->getRequest()->getRiderIndex());
        std::string d1_rider_uuid = (drop1 == NULL) ? "-" : drop1->getRequest()->getRiderTripUUID();
        std::string d1_drop_str = (drop1 == NULL) ? "-" : Utility::convertTimeTToString(drop1->getEventTime());
        std::string d1_loc_str = (drop1 == NULL) ? "-" : Utility::convertToLatLngStr(*(drop1->getLocation()),6);
        
        // drop 2 info
        std::string d2_rider_str = (drop2 == NULL) ? "-" : Utility::intToStr(drop2->getRequest()->getRiderIndex());
        std::string d2_rider_uuid = (drop2 == NULL) ? "-" : drop2->getRequest()->getRiderTripUUID();
        std::string d2_drop_str = (drop2 == NULL) ? "-" : Utility::convertTimeTToString(drop2->getEventTime());
        std::string d2_loc_str = (drop2 == NULL) ? "-" : Utility::convertToLatLngStr(*(drop2->getLocation()),6);
        
        // drop 3 info
        std::string d3_rider_str = (drop3 == NULL) ? "-" : Utility::intToStr(drop3->getRequest()->getRiderIndex());
        std::string d3_rider_uuid = (drop3 == NULL) ? "-" : drop3->getRequest()->getRiderTripUUID();
        std::string d3_drop_str = (drop3 == NULL) ? "-" : Utility::convertTimeTToString(drop3->getEventTime());
        std::string d3_loc_str = (drop3 == NULL) ? "-" : Utility::convertToLatLngStr(*(drop3->getLocation()),6);
        
        outFile <<
                left << setw(ctBuff) << Utility::intToStr((*routeItr)->getIndex()) << 
                left << setw(ixBuff) << p1_rider_str <<
                left << setw(uuidBuff) << p1_rider_uuid <<
                left << setw(timeBuff) << p1_req_str << 
                left << setw(timeBuff) << p1_pick_str << 
                left << setw(geoBuff) << p1_loc_str << 
                left << setw(ixBuff) << p2_rider_str << 
                left << setw(uuidBuff) << p2_rider_uuid <<
                left << setw(timeBuff) << p2_req_str << 
                left << setw(timeBuff) << p2_pick_str << 
                left << setw(geoBuff) << p2_loc_str << 
                left << setw(ixBuff) << p3_rider_str << 
                left << setw(uuidBuff) << p3_rider_uuid <<
                left << setw(timeBuff) << p3_req_str << 
                left << setw(timeBuff) << p3_pick_str << 
                left << setw(geoBuff) << p3_loc_str << 
                left << setw(ixBuff) << d1_rider_str << 
                left << setw(uuidBuff) << d1_rider_uuid <<
                left << setw(timeBuff) << d1_drop_str << 
                left << setw(geoBuff) << d1_loc_str << 
                left << setw(ixBuff) << d2_rider_str << 
                left << setw(uuidBuff) << d2_rider_uuid << 
                left << setw(timeBuff) << d2_drop_str << 
                left << setw(geoBuff) << d2_loc_str << 
                left << setw(ixBuff) << d3_rider_str << 
                left << setw(uuidBuff) << d3_rider_uuid <<
                left << setw(timeBuff) << d3_drop_str << 
                left << setw(geoBuff) << d3_loc_str << 
        std::endl;                
    }  
    
    outFile.close();
}
void Output::printUnmatchedTripsSummary(Solution* pSolution, std::string& outpath) {
  
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
    int isFdBuff = 9;   // for flexible departures
    
    // check if solution corresponds to flexible departures
    FlexDepSolution * pFDSoln = dynamic_cast<FlexDepSolution*>(pSolution);
    const bool isFDSoln = (pFDSoln != NULL);
    
    outFile << left << setw(ixBuff) << "DRIVER" << 
            left << setw(ixBuff) << "RIDER_IX" << 
            left << setw(uuidBuff) << "RIDER_UUID";
    if( isFDSoln ) {
        outFile << left << setw(isFdBuff) << "IS_FD?";     
    }
    outFile << left << setw(timeBuff) << "REQUEST_TIME" << 
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
                left << setw(uuidBuff) << (*tripItr)->getMasterTripUUID();
        if( isFDSoln ) {
            const std::set<const int> * pFDReqIndices = pFDSoln->getFlexDepReqIndices();
            std::set<const int>::const_iterator fdItr = pFDReqIndices->find((*tripItr)->getMasterReqIndex());
            std::string isFdStr = (fdItr != pFDReqIndices->end()) ? "yes" : "no";        
            outFile << left << setw(isFdBuff) << isFdStr;
        }
        outFile << left << setw(timeBuff) << Utility::convertTimeTToString((*tripItr)->getMasterRequestEvent()->timeT) << 
                left << setw(geoBuff) << Utility::convertToLatLngStr(pickupLoc, 6) << 
                left << setw(geoBuff) << Utility::convertToLatLngStr(dropLoc, 6) << 
                left << setw(timeBuff) << Utility::convertTimeTToString((*tripItr)->getMasterPickupEventFromActuals()->timeT) << 
                left << setw(timeBuff) << Utility::convertTimeTToString((*tripItr)->getMasterDropEventFromActuals()->timeT) << 
                left << setw(distBuff) << Utility::truncateDouble(distKm, 2) << 
                std::endl;        
    }
    
    outFile.close();    
}
void Output::printMultPickupUnmatchedTripSummary(MultPickupSoln* pSolution, std::string& outpath) {
    std::string outFilePath = outpath + "_UNMATCHED_TRIPS.txt";
    ofstream outFile(outFilePath.c_str());
       
    const std::set<AssignedRoute*, AssignedRouteIndexComp> * pUnmatchedTrips = pSolution->getUnmatchedRoutes();
    std::set<AssignedRoute*, AssignedRouteIndexComp>::const_iterator routeItr;        
    
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
    
    for( routeItr = pUnmatchedTrips->begin(); routeItr != pUnmatchedTrips->end(); ++routeItr ) {
        assert( (*routeItr)->getNumRidersInRoute() == 1 );
        Request * pRequest = (*routeItr)->getRoute()->getRequests()->front();
        LatLng pickupLoc(pRequest->getActualPickupEvent()->lat, pRequest->getActualPickupEvent()->lng);
        LatLng dropLoc(pRequest->getActualDropEvent()->lat, pRequest->getActualDropEvent()->lng);
        
        const double tripDist = Utility::computeGreatCircleDistance(pickupLoc._lat,pickupLoc._lng,dropLoc._lat,dropLoc._lng);
        
        outFile << left << setw(ixBuff) << Utility::intToStr(pRequest->getActualDriver()->getIndex()) << 
                left << setw(ixBuff) << Utility::intToStr(pRequest->getRiderIndex()) << 
                left << setw(uuidBuff) << pRequest->getRiderTripUUID() << 
                left << setw(timeBuff) << Utility::convertTimeTToString(pRequest->getReqTime()) << 
                left << setw(geoBuff) << Utility::convertToLatLngStr(pickupLoc,6) << 
                left << setw(geoBuff) << Utility::convertToLatLngStr(dropLoc,6) << 
                left << setw(timeBuff) << Utility::convertTimeTToString(pRequest->getActualPickupEvent()->timeT) << 
                left << setw(timeBuff) << Utility::convertTimeTToString(pRequest->getActualDropEvent()->timeT) << 
                left << setw(distBuff) << Utility::truncateDouble(tripDist,2) << 
        endl;
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
void Output::printIndivSolnMetrics(Solution * pSolution, std::string &outpath) {
    std::string outFilePath = outpath + "_indivMetrics.txt";
    ofstream outFile(outFilePath.c_str());
    
    outFile << "\n----------------------------------------\n" << std::endl;
    outFile << "       individual solution metrics " << std::endl;
    outFile << "\n----------------------------------------\n\n" << std::endl; 
    
    printIndivdualRiderInconvenienceMetrics(pSolution, outFile);
    printIndividualTripOverlapMetrics(pSolution, outFile);
    printIndividualSavingsMetrics(pSolution, outFile);
    printIndividualWaitTimeofMatchMetrics(pSolution, outFile);
    
    outFile << "\n\t(end of file)\n" << std::endl;
    outFile.close();
        
}
void Output::printIndivdualRiderInconvenienceMetrics(Solution * pSolution, ofstream &outFile) {
    outFile << "\n\n\n-- rider inconvenience --\n" << std::endl;
    
    std::vector<double> pctInconv_all     = pSolution->getIndivMatchedRidersMetrics()->_inconv_ALL;
    std::vector<double> pctInconv_masters = pSolution->getIndivMatchedRidersMetrics()->_inconv_Masters;
    std::vector<double> pctInconv_minions = pSolution->getIndivMatchedRidersMetrics()->_inconv_Minions;
    
    std::string csv_all = Utility::convertToCsvString("CSV_PCTINCONV_ALL", &pctInconv_all);
    std::string csv_masters = Utility::convertToCsvString("CSV_PCTINCONV_MASTERS", &pctInconv_masters);
    std::string csv_minions = Utility::convertToCsvString("CSV_PCTINCONV_MINIONS", &pctInconv_minions);
    
    outFile << "\n" << csv_all << std::endl;
    outFile << "\n" << csv_masters << std::endl;
    outFile << "\n" << csv_minions << std::endl;
}
void Output::printIndividualTripOverlapMetrics(Solution * pSolution, ofstream &outFile) {
    outFile << "\n\n\n-- trip overlap --\n" << std::endl;
    
    std::vector<double> overlapDist_all    = pSolution->getIndivMatchedRidersMetrics()->_overlapDist;
    std::vector<double> pctOverlap_trip    = pSolution->getIndivMatchedRidersMetrics()->_overlapPct_Trip;
    std::vector<double> pctOverlap_all     = pSolution->getIndivMatchedRidersMetrics()->_overlapPct_ALL;
    std::vector<double> pctOverlap_masters = pSolution->getIndivMatchedRidersMetrics()->_overlapPct_Masters;
    std::vector<double> pctOverlap_minions = pSolution->getIndivMatchedRidersMetrics()->_overlapPct_Minions;
    
    std::string csv_distances = Utility::convertToCsvString("CSV_OVERLAP_DIST_KM_ALL", &overlapDist_all);
    std::string csv_trip      = Utility::convertToCsvString("CSV_PCTOVERLAP_TRIP", &pctOverlap_trip);
    std::string csv_all       = Utility::convertToCsvString("CSV_PCTOVERLAP_ALL", &pctOverlap_all);
    std::string csv_masters   = Utility::convertToCsvString("CSV_PCTOVERLAP_MASTERS", &pctOverlap_masters);
    std::string csv_minions   = Utility::convertToCsvString("CSV_PCTOVERLAP_MINIONS", &pctOverlap_minions);
    
    outFile << "\n" << csv_distances << std::endl;
    outFile << "\n" << csv_trip << std::endl;
    outFile << "\n" << csv_all << std::endl;
    outFile << "\n" << csv_masters << std::endl;
    outFile << "\n" << csv_minions << std::endl;    
}
void Output::printIndividualSavingsMetrics(Solution * pSolution, ofstream &outFile) {
    outFile << "\n\n\n-- rider savings -- \n" << std::endl;
    
    std::vector<double> pctSavings_all     = pSolution->getIndivMatchedRidersMetrics()->_savings_ALL;
    std::vector<double> pctSavings_masters = pSolution->getIndivMatchedRidersMetrics()->_savings_Masters;
    std::vector<double> pctSavings_minions = pSolution->getIndivMatchedRidersMetrics()->_savings_Minions;
    
    std::string csv_all     = Utility::convertToCsvString("CSV_PCTSAVINGS_ALL", &pctSavings_all);
    std::string csv_masters = Utility::convertToCsvString("CSV_PCTSAVINGS_MASTERS", &pctSavings_masters);
    std::string csv_minions = Utility::convertToCsvString("CSV_PCTSAVINGS_MINIONS", &pctSavings_minions);
    
    outFile << "\n" << csv_all << std::endl;
    outFile << "\n" << csv_masters << std::endl;
    outFile << "\n" << csv_minions << std::endl;
    
}
void Output::printIndividualWaitTimeofMatchMetrics(Solution * pSolution, ofstream &outFile) {
    outFile << "\n\n\n-- wait time to match --\n" << std::endl;
    
    std::vector<int> waitTimeMatch_all     = pSolution->getIndivMatchedRidersMetrics()->_waitTimeToMatch_ALL;
    std::vector<int> waitTimeMatch_masters = pSolution->getIndivMatchedRidersMetrics()->_waitTimeToMatch_Masters;
    std::vector<int> waitTimeMatch_minions = pSolution->getIndivMatchedRidersMetrics()->_waitTimeToMatch_Minions;
    
    std::string csv_all     = Utility::convertToCsvString("CSV_WAITTIMETOMATCH_SEC_ALL", &waitTimeMatch_all);
    std::string csv_masters = Utility::convertToCsvString("CSV_WAITTIMETOMATCH_SEC_MASTERS", &waitTimeMatch_masters);
    std::string csv_minions = Utility::convertToCsvString("CSV_WAITTIMETOMATCH_SEC_MINIONS", &waitTimeMatch_minions);
    
    outFile << "\n" << csv_all << std::endl;
    outFile << "\n" << csv_masters << std::endl;
    outFile << "\n" << csv_minions << std::endl;
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
        case DEFAULTVALUES : 
            inputValueStr = "default";
            break;
        case OPTIN :
            inputValueStr = "opt-in";
            break;
        case BATCHWINDOW : 
            inputValueStr = "batchLength";
            break;
        case PICKUP :
            inputValueStr = "maxPickupDist";
            break;
        case SAVINGSRATE :
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
    printOverlapMetrics      ( outFile, inputValueStr, &inputRange, pModelSolnMap );
    printNumTripsMetrics     ( outFile, inputValueStr, &inputRange, pModelSolnMap ); 
    printRiderSavingsMetrics ( outFile, inputValueStr, &inputRange, pModelSolnMap );
    printMatchWaitTimeMetrics( outFile, inputValueStr, &inputRange, pModelSolnMap );
    
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
        
    std::string csv_mitm_str       = "CSV_MITM";
    std::string csv_ufbw_str       = "CSV_UFBW";
    std::string csv_flexDep_str    = "CSV_FD";
    std::string csv_ufbwPI_str     = "CSV_UFBW_PI";
    std::string csv_multPickUp_str = "CSV_MULT_PICKUPS"; 
    
    std::string csv_FD_FDReqsStr = "";
    std::string csv_FD_nonFDReqsStr = "";
    
    std::string mitmMatchRateStr         = "-";
    std::string ufbwMatchRateStr         = "-";
    std::string flexDepMatchRateStr      = "-";
    std::string ufbwPerfInfoMatchRateStr = "-";
    std::string multPickupMatchRateStr   = "-";
        
    // print models     
    outFile << left << setw(15) << inputName << left << setw(15) << "MITM" << left << setw(15) << "UFBW" << left << setw(15) << "FD" << left << setw(15) << "UFBW-PI" << left << setw(15) << "MULT_PICKUPS" << std::endl;

  
    for( std::set<double>::iterator inputItr = pInputRange->begin(); inputItr != pInputRange->end(); ++inputItr ) {
        
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
                case MULTIPLE_PICKUPS :
                {
                    multPickupMatchRateStr = Utility::truncateDouble(mapItr->second,4);
                    csv_multPickUp_str += "," + multPickupMatchRateStr;
                    break;
                }
            }                        
        }
           
        outFile << left << setw(15) << Utility::doubleToStr(*inputItr) << 
                   left << setw(15) << mitmMatchRateStr << 
                   left << setw(15) << ufbwMatchRateStr << 
                   left << setw(15) << flexDepMatchRateStr << 
                   left << setw(15) << ufbwPerfInfoMatchRateStr << 
                   left << setw(15) << multPickupMatchRateStr <<
                std::endl;
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
    if( csv_multPickUp_str != "CSV_MULTPICKUPS" ) {
        outFile << csv_multPickUp_str << std::endl;
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
        
    std::string csv_mitm_str        = "CSV_MITM";
    std::string csv_ufbw_str        = "CSV_UFBW";
    std::string csv_flexDep_str     = "CSV_FD";
    std::string csv_ufbwPI_str      = "CSV_UFBW_PI";
    std::string csv_multPickups_str = "CSV_MULT_PICKUPS";
        
    std::string mitmMatchRateStr         = "-";
    std::string ufbwMatchRateStr         = "-";
    std::string flexDepMatchRateStr      = "-";
    std::string ufbwPerfInfoMatchRateStr = "-";
    std::string multPickupsInconvStr     = "-";
        
    // print models     
    outFile << left << setw(15) << inputName << left << setw(15) << "MITM" << left << setw(15) << "UFBW" << left << setw(15) << "FD" << left << setw(15) << "UFBW-PI" << left << setw(15) << "MULT_PICKUPS" << std::endl;

  
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
                case MULTIPLE_PICKUPS : 
                {
                    multPickupsInconvStr = Utility::truncateDouble(mapItr->second,4);
                    csv_multPickups_str += "," + multPickupsInconvStr;
                    break;
                }
            }                        
        }
           
        outFile << 
                left << setw(15) << Utility::doubleToStr(*inputItr) << 
                left << setw(15) << mitmMatchRateStr << 
                left << setw(15) << ufbwMatchRateStr << 
                left << setw(15) << flexDepMatchRateStr << 
                left << setw(15) << ufbwPerfInfoMatchRateStr << 
                left << setw(15) << multPickupsInconvStr << 
            std::endl;
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
    if( csv_multPickups_str != "CSV_MULT_PICKUPS" ) {
        outFile << csv_multPickups_str << std::endl;
    }
            
    outFile << "\n\n" << std::endl;  
        
}
void Output::printNumTripsMetrics(std::ofstream& outFile, std::string inputName, std::set<double>* pInputRange, const std::map<double,SolnMaps*>* pModelSolnMap) {
    outFile << "\nNUMBER DRIVER TRIPS\n" << std::endl;
        
    std::string csv_mitm_str        = "CSV_MITM";
    std::string csv_ufbw_str        = "CSV_UFBW";
    std::string csv_flexDep_str     = "CSV_FD";
    std::string csv_ufbwPI_str      = "CSV_UFBW_PI";
    std::string csv_multPickups_str = "CSV_MULT_PICKUPS";
        
    std::string mitmMatchRateStr         = "-";
    std::string ufbwMatchRateStr         = "-";
    std::string flexDepMatchRateStr      = "-";
    std::string ufbwPerfInfoMatchRateStr = "-";
    std::string multPickupsNumTripsStr   = "-";
        
    // print models     
    outFile << 
            left << setw(15) << inputName << 
            left << setw(15) << "MITM" << 
            left << setw(15) << "UFBW" << 
            left << setw(15) << "FD" << 
            left << setw(15) << "UFBW-PI" << 
            left << setw(15) << "MULT_PICKUPS" << 
    std::endl;
    

    for( std::set<double>::iterator inputItr = pInputRange->begin(); inputItr != pInputRange->end(); ++inputItr ) {
                
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
                case MULTIPLE_PICKUPS :
                {
                    multPickupsNumTripsStr =  Utility::intToStr((int) mapItr->second);
                    csv_multPickups_str += "," + multPickupsNumTripsStr;
                    break;
                }                
            }                        
        }
           
        outFile << 
                left << setw(15) << Utility::doubleToStr(*inputItr) << 
                left << setw(15) << mitmMatchRateStr << 
                left << setw(15) << ufbwMatchRateStr << 
                left << setw(15) << flexDepMatchRateStr << 
                left << setw(15) << ufbwPerfInfoMatchRateStr << 
                left << setw(15) << multPickupsNumTripsStr << 
            std::endl;
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
    if( csv_multPickups_str != "CSV_MULT_PICKUPS" ) {
        outFile << csv_multPickups_str << std::endl;
    }
            
    outFile << "\n\n" << std::endl;  
           
}
void Output::printRiderSavingsMetrics(std::ofstream& outFile, std::string inputName, std::set<double>* pInputRange, const std::map<double,SolnMaps*>* pModelSolnMap) {
   outFile << "\nRIDER SAVINGS METRICS\n" << std::endl;    
        
    std::string csv_mitm_str        = "CSV_MITM";
    std::string csv_ufbw_str        = "CSV_UFBW";
    std::string csv_FD_str          = "CSV_FD";
    std::string csv_ufbwPI_str      = "CSV_UFBW_PI";
    std::string csv_multPickups_str = "CSV_MULT_PICKUPS";
    
    // mitm CSV strings
    std::string csv_mitm_savings_masters_str = "CSV_MITM_Avg_Savings_Masters";
    std::string csv_mitm_savings_minions_str = "CSV_MITM_Avg_Savings_Minions";
    
    // UFBW fixed strings
    std::string csv_ufbw_savings_masters_str = "CSV_UFBW_Avg_Savings_Masters";
    std::string csv_ufbw_savings_minions_str = "CSV_UFBW_Avg_Savings_Minions";
    
    // PI fixed strings
    std::string csv_ufbwPI_savings_masters_str = "CSV_UFBW_PI_Avg_Savings_Masters";
    std::string csv_ufbwPI_savings_minions_str = "CSV_UFBW_PI_Avg_Savings_Minions";   
    
    // FD fixed strings
    std::string csv_FD_savings_masters_str = "CSV_FD_Avg_Savings_Masters";
    std::string csv_FD_savings_minions_str = "CSV_FD_Avg_Savings_Minions"; 

    // multiple pickups strings
    std::string csv_MP_savings_masters_str = "CSV_MP_Avg_Savings_Masters";
    std::string csv_MP_savings_minions_str = "CSV_MP_Avg_Savings_Minions";
        
    std::string mitmSavingsStr        = "-";
    std::string ufbwSavingsStr        = "-";
    std::string ufbwPISavingsStr      = "-";
    std::string flexDepSavingsStr     = "-";
    std::string multPickupsSavingsStr = "-";
    
    bool MITM_solved = false;
    bool UFBW_fixed_solved = false;
    bool UFBW_PI_solved = false;
    bool FD_solved = false;
    bool MultPickups_solved = false;
        
    // print models     
    outFile << 
            left << setw(15) << inputName << 
            left << setw(15) << "MITM" << 
            left << setw(15) << "UFBW" << 
            left << setw(15) << "FD" << 
            left << setw(15) << "UFBW-PI" << 
            left << setw(15) << "MULT_PICKUPS" << 
    std::endl;

  
    for( std::set<double>::iterator inputItr = pInputRange->begin(); inputItr != pInputRange->end(); ++inputItr ) {
        
        
        std::map<double, SolnMaps*>::const_iterator modelSolnMapItr = pModelSolnMap->find(*inputItr);
        
        std::map<ModelEnum, double> avgSavingsMap_ALL     = modelSolnMapItr->second->avgSavingsAllMatchedRiders;
        std::map<ModelEnum, double> avgSavingsMap_Masters = modelSolnMapItr->second->avgSavingsMasters;
        std::map<ModelEnum, double> avgSavingsMap_Minions = modelSolnMapItr->second->avgSavingsMinions;
         
        for( std::map<ModelEnum, double>::iterator mapItr = avgSavingsMap_ALL.begin(); mapItr != avgSavingsMap_ALL.end(); ++mapItr ) {
            switch( mapItr->first ) {
                case MITM_SEQ_PICKUPS :
                {           
                    MITM_solved = true;
                    
                    // add pct overall for all riders
                    mitmSavingsStr = Utility::truncateDouble(mapItr->second, 4);
                    csv_mitm_str += "," + mitmSavingsStr;
                                       
                    // add savings for masters
                    std::map<ModelEnum, double>::iterator mastersItr = avgSavingsMap_Masters.find(mapItr->first);
                    csv_mitm_savings_masters_str += "," + Utility::truncateDouble(mastersItr->second, 4);
                    
                    // add savings for minions
                    std::map<ModelEnum, double>::iterator minionsItr = avgSavingsMap_Minions.find(mapItr->first);
                    csv_mitm_savings_minions_str += "," + Utility::truncateDouble(minionsItr->second, 4);
                                        
                    break;
                }
   
                case UFBW_FIXED_PICKUPS :
                {
                    UFBW_fixed_solved = true;
                    
                    // add pct overall for all riders
                    ufbwSavingsStr = Utility::truncateDouble(mapItr->second, 4);
                    csv_ufbw_str += "," + ufbwSavingsStr;
                    
                    // add savings for masters
                    std::map<ModelEnum, double>::iterator mastersItr = avgSavingsMap_Masters.find(mapItr->first);
                    csv_ufbw_savings_masters_str += "," + Utility::truncateDouble(mastersItr->second, 4);
                    
                    // add savings for minions
                    std::map<ModelEnum, double>::iterator minionsItr = avgSavingsMap_Minions.find(mapItr->first);
                    csv_ufbw_savings_minions_str += "," + Utility::truncateDouble(minionsItr->second, 4);
                                           
                    break;
                }
                case UFBW_PERFECT_INFO :
                {
                    UFBW_PI_solved = true; 
                                        
                    // add pct overall for all riders
                    ufbwPISavingsStr = Utility::truncateDouble(mapItr->second, 4);
                    csv_ufbwPI_str += "," + ufbwPISavingsStr;
                    
                    // add savings for masters
                    std::map<ModelEnum, double>::iterator mastersItr = avgSavingsMap_Masters.find(mapItr->first);
                    csv_ufbwPI_savings_masters_str += "," + Utility::truncateDouble(mastersItr->second, 4);
                    
                    // add savings for minions
                    std::map<ModelEnum, double>::iterator minionsItr = avgSavingsMap_Minions.find(mapItr->first);
                    csv_ufbwPI_savings_minions_str += "," + Utility::truncateDouble(minionsItr->second, 4);
                                     
                    break;                                                 
                }
                case FLEX_DEPARTURE :
                {
                    FD_solved = true;
                                          
                    // add pct overall for all riders
                    flexDepSavingsStr = Utility::truncateDouble(mapItr->second, 4);
                    csv_FD_str += "," + ufbwPISavingsStr;
                    
                    // add savings for masters
                    std::map<ModelEnum, double>::iterator mastersItr = avgSavingsMap_Masters.find(mapItr->first);
                    csv_FD_savings_masters_str += "," + Utility::truncateDouble(mastersItr->second, 4);
                    
                    // add savings for minions
                    std::map<ModelEnum, double>::iterator minionsItr = avgSavingsMap_Minions.find(mapItr->first);
                    csv_FD_savings_minions_str += "," + Utility::truncateDouble(minionsItr->second, 4);
                                       
                    break;
                }
                case MULTIPLE_PICKUPS :
                {
                    MultPickups_solved = true;
                    
                    // add pct overall for all riders
                    multPickupsSavingsStr = Utility::truncateDouble(mapItr->second, 4);
                    csv_multPickups_str += "," + multPickupsSavingsStr;
                    
                    // add savings for masters
                    std::map<ModelEnum, double>::iterator mastersItr = avgSavingsMap_Masters.find(mapItr->first);
                    csv_MP_savings_masters_str += "," + Utility::truncateDouble(mastersItr->second, 4);
                    
                    // add savings for minions
                    std::map<ModelEnum, double>::iterator minionsItr = avgSavingsMap_Minions.find(mapItr->first);
                    csv_MP_savings_minions_str += "," + Utility::truncateDouble(minionsItr->second, 4);
                         
                    break;
                }
            }                        
        }
           
        outFile << 
                left << setw(15) << Utility::doubleToStr(*inputItr) << 
                left << setw(15) << mitmSavingsStr << 
                left << setw(15) << ufbwSavingsStr << 
                left << setw(15) << flexDepSavingsStr << 
                left << setw(15) << ufbwPISavingsStr << 
                left << setw(15) << multPickupsSavingsStr << 
        std::endl;
    }
    
    outFile << "\n" << std::endl;
    if( MITM_solved ) {
        outFile << csv_mitm_str  << std::endl;
        outFile << csv_mitm_savings_masters_str << std::endl;
        outFile << csv_mitm_savings_minions_str << "\n" << std::endl;
    }
    if( UFBW_fixed_solved ) {
        outFile << csv_ufbw_str  << std::endl;
        outFile << csv_ufbw_savings_masters_str << std::endl;
        outFile << csv_ufbw_savings_minions_str << "\n" << std::endl;     
    }
    if( FD_solved ) {
        outFile << csv_FD_str  << std::endl;
        outFile << csv_FD_savings_masters_str << std::endl;
        outFile << csv_FD_savings_minions_str << "\n" << std::endl;       
    }
    if( UFBW_PI_solved ) {
        outFile << csv_ufbwPI_str  << std::endl;
        outFile << csv_ufbwPI_savings_masters_str << std::endl;
        outFile << csv_ufbwPI_savings_minions_str << "\n" << std::endl;       
    }
    if( MultPickups_solved ) {
        outFile << csv_multPickups_str << std::endl;
        outFile << csv_MP_savings_masters_str << std::endl;
        outFile << csv_MP_savings_minions_str << "\n" << std::endl;
    }
            
    outFile << "\n\n" << std::endl;      
}
void Output::printOverlapMetrics(std::ofstream& outFile, std::string inputName, std::set<double>* pInputRange, const std::map<double,SolnMaps*>* pModelSolnMap) {
    outFile << "\nTRIP OVERLAP METRICS (avg % overlap)\n" << std::endl;
        
    std::string csv_mitm_str        = "CSV_MITM_Avg_Pct_ALL";
    std::string csv_ufbw_str        = "CSV_UFBW_Avg_Pct_ALL";
    std::string csv_flexDep_str     = "CSV_FD_Avg_Pct_ALL";
    std::string csv_ufbwPI_str      = "CSV_UFBW_PI_Avg_Pct_ALL";
    std::string csv_multPickups_str = "CSV_MP_Avg_Pct_ALL"; 
    
    // mitm CSV strings
    std::string csv_mitm_dist_str        = "CSV_MITM_Avg_Dist";
    std::string csv_mitm_pct_trip_str    = "CSV_MITM_Avg_Pct_Trip";
    std::string csv_mitm_pct_masters_str = "CSV_MITM_Avg_Pct_Masters";
    std::string csv_mitm_pct_minions_str = "CSV_MITM_Avg_Pct_Minions";
    
    // UFBW fixed strings
    std::string csv_ufbw_dist_str        = "CSV_UFBW_Avg_Dist";
    std::string csv_ufbw_pct_trip_str    = "CSV_UFBW_Avt_Pct_Trip";
    std::string csv_ufbw_pct_masters_str = "CSV_UFBW_Avg_Pct_Masters";
    std::string csv_ufbw_pct_minions_str = "CSV_UFBW_Avg_Pct_Minions";
    
    // PI fixed strings
    std::string csv_ufbwPI_dist_str        = "CSV_UFBW_PI_Avg_Dist";
    std::string csv_ufbwPI_pct_trip_str    = "CSV_UFBW_PI_Avt_Pct_Trip";
    std::string csv_ufbwPI_pct_masters_str = "CSV_UFBW_PI_Avg_Pct_Masters";
    std::string csv_ufbwPI_pct_minions_str = "CSV_UFBW_PI_Avg_Pct_Minions";    
    
    // FD fixed strings
    std::string csv_flexDep_dist_str        = "CSV_FD_Avg_Dist";
    std::string csv_flexDep_pct_trip_str    = "CSV_FD_Avg_Pct_Trip";
    std::string csv_flexDep_pct_masters_str = "CSV_FD_Avg_Pct_Masters";
    std::string csv_flexDep_pct_minions_str = "CSV_FD_Avg_Pct_Minions"; 
    
    // MULTIPLE PIKCUPS fixed strings
    std::string csv_multPickups_dist_str        = "CSV_MP_Avg_Dist";
    std::string csv_multPickups_pct_trip_str    = "CSV_MP_Avg_Pct_Trip";
    std::string csv_multPickups_pct_masters_str = "CSV_MP_Avg_Pct_Masters";
    std::string csv_multPickups_pct_minions_str = "CSV_MP_Avg_Pct_Minions";     
        
    std::string mitmTripOverlapStr         = "-";
    std::string ufbwTripOverlapStr         = "-";
    std::string flexDepTripOverlapStr      = "-";
    std::string ufbwPerfInfoTripOverlapStr = "-";
    std::string multPickupsTripOverlapStr  = "-";
    
    bool MITM_solved       = false;
    bool UFBW_fixed_solved = false;
    bool UFBW_PI_solved    = false;
    bool FD_solved         = false;
    bool MP_solved         = false;
        
    // print models     
    outFile << 
            left << setw(15) << inputName << 
            left << setw(15) << "MITM" << 
            left << setw(15) << "UFBW" << 
            left << setw(15) << "FD" << 
            left << setw(15) << "UFBW-PI" << 
            left << setw(15) << "MULT-PICKUPS" << 
    std::endl;
  
    for( std::set<double>::iterator inputItr = pInputRange->begin(); inputItr != pInputRange->end(); ++inputItr ) {
        
        std::map<double, SolnMaps*>::const_iterator modelSolnMapItr = pModelSolnMap->find(*inputItr);
                
        std::map<ModelEnum, double> avgDistOverlapMap = modelSolnMapItr->second->avgOverlapDist;
        std::map<ModelEnum, double> avgPctOverlap_Trip = modelSolnMapItr->second->avgPctOverlap_Trip;
        std::map<ModelEnum, double> avgPctOverlapMap  = modelSolnMapItr->second->avgPctOverlap_ALL;
        std::map<ModelEnum, double> avgPctOverlapMastersMap = modelSolnMapItr->second->avgPctOverlap_Masters;
        std::map<ModelEnum, double> avgPctOverlapMinionsMap = modelSolnMapItr->second->avgPctOverlap_Minions;
        

        for( std::map<ModelEnum, double>::iterator mapItr = avgPctOverlap_Trip.begin(); mapItr != avgPctOverlap_Trip.end(); ++mapItr ) {
            switch( mapItr->first ) {
                case MITM_SEQ_PICKUPS :
                {           
                    MITM_solved = true;         
                    
                    // add pct overall for all riders
                    std::map<ModelEnum,double>::iterator avgPctItr = avgPctOverlapMap.find(mapItr->first);
                    csv_mitm_str += "," + Utility::truncateDouble(mapItr->second, 4);
                    
                    // add overlap dist
                    std::map<ModelEnum, double>::iterator avgDistItr = avgDistOverlapMap.find(mapItr->first);
                    csv_mitm_dist_str += "," + Utility::truncateDouble(avgDistItr->second, 4);
                    
                    // add overlap pct for trip                    
                   // std::map<ModelEnum, double>::iterator tripOverlapItr = avgPctOverlap_Trip.find(mapItr->first);
                    mitmTripOverlapStr = Utility::truncateDouble(mapItr->second, 4);
                    csv_mitm_pct_trip_str += "," + mitmTripOverlapStr;
                    
                    // add overlap pct for masters
                    std::map<ModelEnum, double>::iterator masterOverlapItr = avgPctOverlapMastersMap.find(mapItr->first);
                    csv_mitm_pct_masters_str += "," + Utility::truncateDouble(masterOverlapItr->second, 4);
                    
                    // add overlap pct for minions
                    std::map<ModelEnum, double>::iterator minionOverlapItr = avgPctOverlapMinionsMap.find(mapItr->first);
                    csv_mitm_pct_minions_str += "," + Utility::truncateDouble(minionOverlapItr->second,4);
                                        
                    break;
                }
               /* case ModelEnum::MITM_PICKUP_SWAPS :
                {
                    
                }*/
                case UFBW_FIXED_PICKUPS :
                {
                    UFBW_fixed_solved = true;
                    
                    // add pct overall for all riders                    
                    csv_ufbw_str += "," + Utility::truncateDouble(mapItr->second, 4);
                    
                    // add overlap dist
                    std::map<ModelEnum, double>::iterator avgDistItr = avgDistOverlapMap.find(mapItr->first);
                    csv_ufbw_dist_str += "," + Utility::truncateDouble(avgDistItr->second, 4);
                    
                    // add overlap pct for trip                    
                    std::map<ModelEnum, double>::iterator tripOverlapItr = avgPctOverlap_Trip.find(mapItr->first);
                    ufbwTripOverlapStr = Utility::truncateDouble(tripOverlapItr->second, 4);
                    csv_ufbw_pct_trip_str += "," + ufbwTripOverlapStr;                    
                    
                    // add overlap pct for masters
                    std::map<ModelEnum, double>::iterator masterOverlapItr = avgPctOverlapMastersMap.find(mapItr->first);
                    csv_ufbw_pct_masters_str += "," + Utility::truncateDouble(masterOverlapItr->second, 4);
                    
                    // add overlap pct for minions
                    std::map<ModelEnum, double>::iterator minionOverlapItr = avgPctOverlapMinionsMap.find(mapItr->first);
                    csv_ufbw_pct_minions_str += "," + Utility::truncateDouble(minionOverlapItr->second,4);
                                                 
                    break;
                }
                /*case ModelEnum::UFBW_PICKUP_SWAPS :
                {
                    
                }*/
                case UFBW_PERFECT_INFO :
                {
                    UFBW_PI_solved = true; 
                    
                    // add pct overall for all riders                    
                    //ufbwPerfInfoMatchRateStr = Utility::truncateDouble(mapItr->second, 4); 
                    csv_ufbwPI_str += "," + Utility::truncateDouble(mapItr->second, 4);
                                        
                    // add overlap dist
                    std::map<ModelEnum, double>::iterator avgDistItr = avgDistOverlapMap.find(mapItr->first);
                    csv_ufbwPI_dist_str += "," + Utility::truncateDouble(avgDistItr->second, 4);
                    
                    // add overlap pct for trip
                    std::map<ModelEnum, double>::iterator tripOverlapItr = avgPctOverlap_Trip.find(mapItr->first);
                    ufbwPerfInfoTripOverlapStr = Utility::truncateDouble(tripOverlapItr->second,4);
                    csv_ufbwPI_pct_trip_str += "," + ufbwPerfInfoTripOverlapStr;
                    
                    // add overlap pct for masters
                    std::map<ModelEnum, double>::iterator masterOverlapItr = avgPctOverlapMastersMap.find(mapItr->first);
                    csv_ufbwPI_pct_masters_str += "," + Utility::truncateDouble(masterOverlapItr->second, 4);
                    
                    // add overlap pct for minions
                    std::map<ModelEnum, double>::iterator minionOverlapItr = avgPctOverlapMinionsMap.find(mapItr->first);
                    csv_ufbwPI_pct_minions_str += "," + Utility::truncateDouble(minionOverlapItr->second,4);
                    
                    break;                                                 
                }
                case FLEX_DEPARTURE :
                {
                    FD_solved = true;
                    
                    // add pct overall for all riders                    
                    csv_flexDep_str += "," + Utility::truncateDouble(mapItr->second, 4);
                    
                    // add overlap dist
                    std::map<ModelEnum, double>::iterator avgDistItr = avgDistOverlapMap.find(mapItr->first);
                    csv_flexDep_dist_str += "," + Utility::truncateDouble(avgDistItr->second, 4);
                    
                    // add overlap pct for trip
                    std::map<ModelEnum, double>::iterator tripOverlapItr = avgPctOverlap_Trip.find(mapItr->first);
                    flexDepTripOverlapStr = Utility::truncateDouble(tripOverlapItr->second, 4); 
                    csv_flexDep_pct_trip_str += "," + flexDepTripOverlapStr; 
                    
                    // add overlap pct for masters
                    std::map<ModelEnum, double>::iterator masterOverlapItr = avgPctOverlapMastersMap.find(mapItr->first);
                    csv_flexDep_pct_masters_str += "," + Utility::truncateDouble(masterOverlapItr->second, 4);
                    
                    // add overlap pct for minions
                    std::map<ModelEnum, double>::iterator minionOverlapItr = avgPctOverlapMinionsMap.find(mapItr->first);
                    csv_flexDep_pct_minions_str += "," + Utility::truncateDouble(minionOverlapItr->second,4);                    
                                        
                    break;
                }
                case MULTIPLE_PICKUPS : 
                {
                    MP_solved = true;
                    
                    // add pct overall for all riders                    
                    csv_multPickups_str += "," + Utility::truncateDouble(mapItr->second, 4);
                    
                    // add overlap dist
                    std::map<ModelEnum, double>::iterator avgDistItr = avgDistOverlapMap.find(mapItr->first);
                    csv_multPickups_dist_str += "," + Utility::truncateDouble(avgDistItr->second, 4);
                    
                    // add overlap pct for trip
                    std::map<ModelEnum, double>::iterator tripOverlapItr = avgPctOverlap_Trip.find(mapItr->first);
                    multPickupsTripOverlapStr = Utility::truncateDouble(tripOverlapItr->second, 4); 
                    csv_multPickups_pct_trip_str += "," + flexDepTripOverlapStr; 
                    
                    // add overlap pct for masters
                    std::map<ModelEnum, double>::iterator masterOverlapItr = avgPctOverlapMastersMap.find(mapItr->first);
                    csv_multPickups_pct_masters_str += "," + Utility::truncateDouble(masterOverlapItr->second, 4);
                    
                    // add overlap pct for minions
                    std::map<ModelEnum, double>::iterator minionOverlapItr = avgPctOverlapMinionsMap.find(mapItr->first);
                    csv_multPickups_pct_minions_str += "," + Utility::truncateDouble(minionOverlapItr->second,4);                                          
                }
            }                        
        }
           
        outFile << 
                left << setw(15) << Utility::doubleToStr(*inputItr) << 
                left << setw(15) << mitmTripOverlapStr << 
                left << setw(15) << ufbwTripOverlapStr << 
                left << setw(15) << flexDepTripOverlapStr << 
                left << setw(15) << ufbwPerfInfoTripOverlapStr << 
                left << setw(15) << multPickupsTripOverlapStr << 
        std::endl;
    }
    
    outFile << "\n" << std::endl;
    if( MITM_solved ) {
        outFile << csv_mitm_dist_str << std::endl;
        outFile << csv_mitm_pct_trip_str << std::endl;
        outFile << csv_mitm_str  << std::endl;                
        outFile << csv_mitm_pct_masters_str << std::endl;
        outFile << csv_mitm_pct_minions_str << "\n" << std::endl;
    }
    if( UFBW_fixed_solved ) {
        outFile << csv_ufbw_dist_str << std::endl;
        outFile << csv_ufbw_pct_trip_str << std::endl;
        outFile << csv_ufbw_str << std::endl;                
        outFile << csv_ufbw_pct_masters_str << std::endl;
        outFile << csv_ufbw_pct_minions_str << "\n" << std::endl;        
    }
    if( FD_solved ) {
        outFile << csv_flexDep_dist_str << std::endl;
        outFile << csv_flexDep_pct_trip_str << std::endl;
        outFile << csv_flexDep_str << std::endl;        
        outFile << csv_flexDep_pct_masters_str << std::endl;
        outFile << csv_flexDep_pct_minions_str << "\n" << std::endl;        
    }
    if( UFBW_PI_solved ) {
        outFile << csv_ufbwPI_dist_str << std::endl;
        outFile << csv_ufbwPI_pct_trip_str << std::endl;
        outFile << csv_ufbwPI_str << std::endl;        
        outFile << csv_ufbwPI_pct_masters_str << std::endl;
        outFile << csv_ufbwPI_pct_minions_str << "\n" << std::endl;        
    }
    if( MP_solved ) {
        outFile << csv_multPickups_dist_str << std::endl;
        outFile << csv_multPickups_pct_trip_str << std::endl;
        outFile << csv_multPickups_str << std::endl;        
        outFile << csv_multPickups_pct_masters_str << std::endl;
        outFile << csv_multPickups_pct_minions_str << "\n" << std::endl;           
    }
            
    outFile << "\n\n" << std::endl;     
}
void Output::printMatchWaitTimeMetrics(std::ofstream& outFile, std::string inputName, std::set<double>* pInputRange, const std::map<double,SolnMaps*>* pModelSolnMap) {
    outFile << "\nWAIT TIME TO MATCH\n" << std::endl;
        
    std::string csv_mitm_str    = "CSV_MITM";
    std::string csv_ufbw_str    = "CSV_UFBW_ALL";
    std::string csv_flexDep_str = "CSV_FD_ALL";
    std::string csv_ufbwPI_str  = "CSV_UFBW_PI_ALL";
    std::string csv_multPickups_str = "CSV_MULT_PICKUPS_ALL";
    
    std::string csv_ufbw_masters_str = "CSV_UFBW_MASTERS";
    std::string csv_ufbw_minions_str = "CSV_UFBW_MINIONS";
    std::string csv_fd_masters_str   = "CSV_FD_MASTERS";
    std::string csv_fd_minions_str   = "CSV_FD_MINIONS";
    std::string csv_ufbwPI_masters_str = "CSV_UFBW_PI_MASTERS";
    std::string csv_ufbwPI_minions_str = "CSV_UFBW_PI_MINIONS";
    std::string csv_multPickups_masters_str = "CSV_MULT_PICKUPS_MASTERS";
    std::string csv_multPickups_minions_str = "CSV_MULT_PICKUPS_MINIONS";
        
    std::string mitmMatchRateStr         = "-";
    std::string ufbwMatchRateStr         = "-";
    std::string flexDepMatchRateStr      = "-";
    std::string ufbwPerfInfoMatchRateStr = "-";
    std::string multPickupsWaitTimeStr   = "-";
    
    bool MITM_solved = false;
    bool UFBW_fixed_solved = false;
    bool UBW_PI_solved = false;
    bool FD_solved = false;
    bool MP_solved;
    
        
    // print models     
    outFile << 
            left << setw(15) << inputName << 
            left << setw(15) << "MITM" << 
            left << setw(15) << "UFBW" << 
            left << setw(15) << "FD" << 
            left << setw(15) << "UFBW-PI" << 
            left << setw(15) << "MULT-PICKUPS" << 
    std::endl;

  
    for( std::set<double>::iterator inputItr = pInputRange->begin(); inputItr != pInputRange->end(); ++inputItr ) {
                
        std::map<double, SolnMaps*>::const_iterator modelSolnMapItr = pModelSolnMap->find(*inputItr);
        
        std::map<ModelEnum, double> avgWaitTimeMatchMap_all     = modelSolnMapItr->second->avgWaitTimeOfMatchAllRiders;
        std::map<ModelEnum, double> avgWaitTimeMatchMap_masters = modelSolnMapItr->second->avgWaitTimeOfMatchMasters;
        std::map<ModelEnum, double> avgWaitTimeMatchMap_minions = modelSolnMapItr->second->avgWaitTimeOfMatchMinions;
         
        for( std::map<ModelEnum, double>::iterator mapItr = avgWaitTimeMatchMap_all.begin(); mapItr != avgWaitTimeMatchMap_all.end(); ++mapItr ) {
            switch( mapItr->first ) {
                case MITM_SEQ_PICKUPS :
                {                    
                    MITM_solved = true;
                    mitmMatchRateStr = Utility::truncateDouble(mapItr->second, 4);
                    csv_mitm_str += "," + mitmMatchRateStr;
                    break;
                }
               /* case ModelEnum::MITM_PICKUP_SWAPS :
                {
                    
                }*/
                case UFBW_FIXED_PICKUPS :
                {
                    UFBW_fixed_solved = true; 
                    
                    // wait time of all future matched riders
                    ufbwMatchRateStr = Utility::truncateDouble(mapItr->second, 4);
                    csv_ufbw_str += "," + ufbwMatchRateStr;
                    
                    // wait time of matched masters (excl. open trips)
                    std::map<ModelEnum, double>::iterator masterWaitItr = avgWaitTimeMatchMap_masters.find(mapItr->first);
                    csv_ufbw_masters_str += "," + Utility::truncateDouble(masterWaitItr->second, 4);
                    
                    // wait time of matched minions (excl. open trips)
                    std::map<ModelEnum, double>::iterator minionWaitItr = avgWaitTimeMatchMap_minions.find(mapItr->first);
                    csv_ufbw_minions_str += "," + Utility::truncateDouble(minionWaitItr->second, 4);
                    
                    break;
                }
                /*case ModelEnum::UFBW_PICKUP_SWAPS :
                {
                    
                }*/
                case UFBW_PERFECT_INFO :
                {
                    UBW_PI_solved = true;
                    
                    // wait time of all future matched riders
                    ufbwPerfInfoMatchRateStr = Utility::truncateDouble(mapItr->second, 4); 
                    csv_ufbwPI_str += "," + ufbwPerfInfoMatchRateStr;  
                    
                    // wait time of matched masters (excl. open trips)
                    std::map<ModelEnum, double>::iterator masterWaitItr = avgWaitTimeMatchMap_masters.find(mapItr->first);
                    csv_ufbwPI_masters_str += "," + Utility::truncateDouble(masterWaitItr->second, 4);
                    
                    // wait time of matched minions (excl. open trips)
                    std::map<ModelEnum, double>::iterator minionWaitItr = avgWaitTimeMatchMap_minions.find(mapItr->first);
                    csv_ufbwPI_minions_str += "," + Utility::truncateDouble(minionWaitItr->second, 4); 
                    
                    break;
                }
                case FLEX_DEPARTURE :
                {
                    FD_solved = true;
                    
                    // wait time of all future matched riders
                    flexDepMatchRateStr = Utility::truncateDouble(mapItr->second, 4);
                    csv_flexDep_str += "," + flexDepMatchRateStr; 
                    
                    // wait time of matched masters (excl. open trips)
                    std::map<ModelEnum, double>::iterator masterWaitItr = avgWaitTimeMatchMap_masters.find(mapItr->first);
                    csv_fd_masters_str += "," + Utility::truncateDouble(masterWaitItr->second, 4);
                    
                    // wait time of matched minions (excl. open trips)
                    std::map<ModelEnum, double>::iterator minionWaitItr = avgWaitTimeMatchMap_minions.find(mapItr->first);
                    csv_fd_minions_str += "," + Utility::truncateDouble(minionWaitItr->second, 4);                     
                    
                    break;
                }
                case MULTIPLE_PICKUPS : 
                {
                    MP_solved = true;
                    
                    // wait time of all future matched riders
                    multPickupsWaitTimeStr = Utility::truncateDouble(mapItr->second, 4);
                    csv_multPickups_str += "," + multPickupsWaitTimeStr; 
                    
                    // wait time of matched masters (excl. open trips)
                    std::map<ModelEnum, double>::iterator masterWaitItr = avgWaitTimeMatchMap_masters.find(mapItr->first);
                    csv_multPickups_masters_str += "," + Utility::truncateDouble(masterWaitItr->second, 4);
                    
                    // wait time of matched minions (excl. open trips)
                    std::map<ModelEnum, double>::iterator minionWaitItr = avgWaitTimeMatchMap_minions.find(mapItr->first);
                    csv_multPickups_minions_str += "," + Utility::truncateDouble(minionWaitItr->second, 4);                          
                    
                    break;
                }
            }                        
        }
           
        outFile << 
                left << setw(15) << Utility::doubleToStr(*inputItr) << 
                left << setw(15) << mitmMatchRateStr << 
                left << setw(15) << ufbwMatchRateStr << 
                left << setw(15) << flexDepMatchRateStr << 
                left << setw(15) << ufbwPerfInfoMatchRateStr << 
                left << setw(15) << multPickupsWaitTimeStr << 
        std::endl;
    }
    
    outFile << "\n" << std::endl;
    if( MITM_solved ) {
        outFile << csv_mitm_str  << std::endl;
    }
    if( UFBW_fixed_solved ) {
        outFile << csv_ufbw_str << std::endl;
        outFile << csv_ufbw_masters_str << std::endl;
        outFile << csv_ufbw_minions_str << std::endl;
    }
    if( FD_solved ) {
        outFile << csv_flexDep_str    << std::endl;
        outFile << csv_fd_masters_str << std::endl;
        outFile << csv_fd_minions_str << std::endl;
    }
    if( UBW_PI_solved ) {
        outFile << csv_ufbwPI_str << std::endl;
        outFile << csv_ufbwPI_masters_str << std::endl;
        outFile << csv_ufbwPI_minions_str << std::endl;
    }
    if( MP_solved ) {
        outFile << csv_multPickups_str << std::endl;
        outFile << csv_multPickups_masters_str << std::endl;
        outFile << csv_multPickups_minions_str << std::endl; 
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

void Output::printInputFileSummary(UserConfig* pUserConfig) {
    cout << "hello world from printInputFIleSummary" << endl;
    exit(0);
}
