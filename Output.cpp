/* 
 * File:   Output.cpp
 * Author: jonpetersen
 * 
 * Created on March 23, 2015, 2:06 PM
 */

#include "Output.hpp"


Output::Output(DataContainer * dataContainer) {    
    pDataContainer = dataContainer; 
    _outputBasePath = dataContainer->getOutputPath();
    
    // ensure directory exists
    int status = mkdir(_outputBasePath.c_str(), S_IRWXU | S_IRWXG | S_IROTH);
}

Output::Output(const Output& orig) {
}

Output::~Output() {
}

void Output::printSummaryInfo() {
    std::string filename = "snapshot_summary.txt";
    std::string outPath = _outputBasePath + filename; 
        
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
    outFile << "\t" << left << setw(buffer) << "up front batching window (sec): " << Utility::intToStr(pDataContainer->getUpFrontBatchWindowLenInSec()) << std::endl;

    
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
    std::string outPath = _outputBasePath + filename;
    
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
    std::string outPath = _outputBasePath + filename;
    
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
    std::string outPath = _outputBasePath + filename;
    
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
    std::string outPath = _outputBasePath + filename;
    
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
    outFile << "\n\t" << left << setw(indexBuffer) << "index" << 
            left << setw(riderBuffer) << "riderID" << 
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
        outFile << "\t" << left << setw(indexBuffer) << Utility::intToStr((*iReq)->getIndex()) << 
                left << setw(riderBuffer) << (*iReq)->getRiderID() << 
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
    std::string outPath = _outputBasePath + filename;
    
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
void Output::printSolution(Solution* pSolution) {
    
    std::string filename = "solution_";
    std::string modelname = "-";
    switch( pSolution->getModel() ) {
        case Solution::MITM : 
            modelname = "MITM";
            break;
        case Solution::UFBW_fixedPickups :
            modelname = "UFBW_fixed";
            break;
        case Solution::UFBW_pickupSwaps : 
            modelname = "UFBW_pickUpSwaps";
            break;
        default :
            modelname += "other";
    } 
    
    filename += modelname;
    filename += ".txt";
    std::string outpath = _outputBasePath + filename;
    
    ofstream outFile(outpath.c_str());
    outFile << "\n----------------------------------------\n" << std::endl;
    outFile << "          solution for " << modelname << std::endl;
    outFile << "\n----------------------------------------\n\n" << std::endl; 

    
    printDataSummary(outFile, pSolution);
    printSolutionSummary(outFile, pSolution);
    if( pSolution->getRequestMetrics()->_numMatchedRequests > 0 ) {
        printMatchTripsSummary(outFile,pSolution);
    }
    if( pSolution->getRequestMetrics()->_numUnmatchedRequests > 0 ) {
        printUnmatchedTripsSummary(outFile,pSolution);
    }
    if( pSolution->getRequestMetrics()->_numDisqualifiedRequests > 0 ) {
        printDisqualifiedRequestsSummary(outFile,pSolution);
    }
    
    outFile.close();
}
void Output::printDataSummary(ofstream& outFile, Solution* pSolution) {
    outFile << "\n\n--- INPUT DATA SUMMARY ---\n" << std::endl;
    
    int headerBuffer = 30;
    outFile << "\t" << left << setw(headerBuffer) << "start time: " << Utility::convertTimeTToString(pSolution->getStartTime()) << std::endl;
    outFile << "\t" << left << setw(headerBuffer) << "end time: " << Utility::convertTimeTToString(pSolution->getEndTime()) << std::endl;
    outFile << "\t" << left << setw(headerBuffer) << "total requests (proxy): " << Utility::intToStr(pSolution->getTotalRequests()) << std::endl;
    outFile << "\t" << left << setw(headerBuffer) << "total drivers: " << Utility::intToStr(pSolution->getTotalDrivers()) << std::endl;
    
    //const int totalRequests = pSolution->g
}
void Output::printSolutionSummary(ofstream &outFile, Solution * pSolution) {
    outFile << "\n\n--- SUMMARY METRICS ---\n" << std::endl;
    
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

    
}

void Output::printMatchTripsSummary(ofstream &outFile, Solution * pSolution) {
    
    outFile << "\n\n--- MATCH SOLUTION SUMMARY ---\n" << std::endl;    
    
    int ixBuff = 9;
    int isExtBuff = 9;
    int fifoBuff = 12;
    int geoBuff = 25;
    int distBuff = 14;
    int inconvBuff = 17;
    
    outFile << "  " << left << setw(ixBuff) << "DRIVER" << 
            left << setw(ixBuff) << "MASTER" << 
            left << setw(ixBuff) << "MINION" << 
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
        
        outFile << "  " << left << setw(ixBuff) << Utility::intToStr(driverIndex) << 
                left << setw(ixBuff) << Utility::intToStr(masterIndex) << 
                left << setw(ixBuff) << Utility::intToStr(minionIndex) << 
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
}

void Output::printUnmatchedTripsSummary(ofstream& outFile, Solution* pSolution) {
    
    outFile << "\n\n\n\n\n\n--- UNMATCHED MASTERS SUMMARY ---\n" << std::endl;    
    
    const std::set<AssignedTrip*, AssignedTripIndexComp> * pUnmatchedTrips = pSolution->getUnmatchedTrips();
    std::set<AssignedTrip*, AssignedTripIndexComp>::const_iterator tripItr;
    
    int ixBuff = 9;
    int timeBuff = 25;
    int geoBuff = 25;
    int distBuff = 15;
    
    outFile << "  " << left << setw(ixBuff) << "DRIVER" << 
            left << setw(ixBuff) << "RIDER" << 
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
        
        outFile << "  " << left << setw(ixBuff) << Utility::intToStr((*tripItr)->getDriver()->getIndex()) << 
                left << setw(ixBuff) << Utility::intToStr((*tripItr)->getMasterIndex()) << 
                left << setw(timeBuff) << Utility::convertTimeTToString((*tripItr)->getMasterRequestEvent()->timeT) << 
                left << setw(geoBuff) << Utility::convertToLatLngStr(pickupLoc, 5) << 
                left << setw(geoBuff) << Utility::convertToLatLngStr(dropLoc, 5) << 
                left << setw(timeBuff) << Utility::convertTimeTToString((*tripItr)->getMasterPickupEventFromActuals()->timeT) << 
                left << setw(timeBuff) << Utility::convertTimeTToString((*tripItr)->getMasterDropEventFromActuals()->timeT) << 
                left << setw(distBuff) << Utility::truncateDouble(distKm, 2) << 
                std::endl;
    }
   
}

void Output::printDisqualifiedRequestsSummary(ofstream& outFile, Solution* pSolution) {
    
    outFile << "\n\n\n\n\n\n--- DISQUALIFIED REQUESTS ---\n" << std::endl;
    
    const std::set<Request*, ReqComp> * pDisqualReqs = pSolution->getDisqualifiedRequests();
    std::set<Request*, ReqComp>::const_iterator reqItr;
    
    int ixBuff = 9;
    int timeBuff = 25;
    int locBuff = 25;
    int ynBuff = 11;
    
    outFile << "  " << left << setw(ixBuff) << "REQ_ID" << 
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
        
        
        outFile << "  " << left << setw(ixBuff) << Utility::intToStr((*reqItr)->getIndex()) << 
                left << setw(ixBuff) << Utility::intToStr((*reqItr)->getRiderIndex()) << 
                left << setw(ynBuff) << isInitStr << 
                left << setw(timeBuff) << Utility::convertTimeTToString((*reqItr)->getReqTime()) << 
                left << setw(locBuff) << Utility::convertToLatLngStr(origLoc, 5) << 
                left << setw(locBuff) << Utility::convertToLatLngStr(destLoc, 5) <<
                left << setw(timeBuff) << Utility::convertTimeTToString((*reqItr)->getActTimeOfPickupFromTripActuals()) << 
                left << setw(timeBuff) << Utility::convertTimeTToString((*reqItr)->getActTimeOfDropoffFromTripActuals()) <<                
                std::endl;
                
    }
    
}