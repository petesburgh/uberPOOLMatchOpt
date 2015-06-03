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
#include "ModelRunner.hpp"
#include "UserConfig.hpp"
#include "Experiment.hpp"

#include <iterator>
#include <deque>

using namespace std;


struct ParameterSet {
    int           _iteration;
    double        _optInRate;
    int           _batchWindowLengthInSec;
    double        _maxPickupDistInKm;
    double        _minDiscount;
    std::string   _paramSetStr;
};

void printMetricsToScreen(UserConfig * pUserConfig);
std::vector<double> getExperimentInputValues(const Experiment &experiment, std::vector<double>&, std::vector<double>&, std::vector<double>&, std::vector<double>&);

std::vector<double> defineBatchWindowRange();
std::vector<double> defineOptInRange();
std::vector<double> defineMaxPickupDistRange();
std::vector<double> defineMinPoolDiscountRange();

void printBanner(ParameterSet * pParamSet, int N);
void printSummaryOfDataInput(DataContainer*);
void printDriverInfo(DataContainer*);
void printRiderInfo(const std::set<Rider*, RiderIndexComp>*);


// -----------
//    MAIN
// -----------
int main(int argc, char** argv) {
    
    const std::string configFile       = "/Users/jonpetersen/Documents/uberPOOL/testEnv/config/parameters.txt";
    UserConfig * pUserConfig = new UserConfig();
    try {
        pUserConfig->readConfigValues(configFile);
        printMetricsToScreen(pUserConfig);
    } catch( FileNotFoundException &ex ) {
        std::cout << "\n*** FileNotFoundException thrown ***" << std::endl;
        ex.what();
        std::cout << "\t(existing unsuccessfully)" << endl;
        exit(1);
    }
                         
    // specify singleton inputs // TODO: move to text file
    const std::string inputPath        = pUserConfig->getStringParams()->_inputPath; 
    const std::string outputBasePath   = pUserConfig->getStringParams()->_outputBasePath; 
    const std::string geofenceDataPath = pUserConfig->getStringParams()->_geofenceDataPath; 
    
    /*
     *  populate all input scenarios
     *      scen 01: SF, one hour sim from 1600-1700 UTC on 2015-04-13, no geofence
     *      scen 02: SF, one hour sim from 1600-1700 UTC on 2015-04-13, SF whiteout geofencee
     *      scen 03: SF, one week sim from 2015-04-12 0000 - 2015-04-19 0000 (UTC), no geofence
     *      scen 04: SF, one week sim from 2015-04-12 0000 - 2015-04-19 0000 (UTC), SF whiteout geofence
     *      scen 05: SF, four hour sim from 1300-1700 UTC on 2015-04-24, SF whiteout geofence
     *      scen 06: Chengdu, one week sim from 2015-04-29 1600 - 2015-04-05 1600 UTC, no geofence
     *      scen 07: Chengdu, two week data from 2015-04-29 1600 - 2015-04-12 1600 UTC consolidated to be consolidated so that the second week of data is to be moved to first week (subtract 7 days from all times)     
     *      scen 08: SF, one day sim from 2014-04-27 07:00:00 - 2015-04-28 07:00:00 UTC, no geofence (flexible departure analysis)
     *      scen 09: LA, one day sim from 2014-04-27 07:00:00 - 2015-04-28 07:00:00 UTC, no geofence (flexible departure analysis)
     *      scen 10: Austin, one day sim from 2014-04-27 07:00:00 - 2015-04-28 07:00:00 UTC, no geofence (flexible departure analysis)
     *      scen 11: scen 09 with LA geofence
     *      scen 12: LA, 5-hour sim from 1800-2300 local with LA geofence (comparing accounting of savings)
     *      scen 13: NJ, 1-week sim from 0000-0000 local, no geofence (MITM)
     *      scen 14: SD, 1-week sim from 0000-0000 local 04/26 to 05/03, no geofence
     *      scen 15: Chengdu, 1 week sim from 0000-0000 local 05/10/2015 to 05/17/2015, no geo (1 MM weekly trip)
     *      scen 16: SF, NO SNAP, 1 week sim from 1000-1000 local 05/04/2015 to 05/11/2015, with geo
     *      scen 17: SF, 55% SNAP, same as scen 16
     *      scen 18: SF, 100% SNAP, same as scen 16
     *      scen 19: SFO-OAK sim, 1-week from 0000-0000 local 05/20 - 05/27, no geo
     */
    
    const int scenNumber = pUserConfig->getIntParams()->_scenNumber; 
    
    
    GenerateInstanceScenarios * pScenarios = new GenerateInstanceScenarios();
    ProblemInstance * pInstance = pScenarios->generateInstanceScenarios(scenNumber,geofenceDataPath); 
    
    const bool printDebugFiles       = pUserConfig->getBooleanParams()->_printDebugFiles; // true;
    const bool printToScreen         = pUserConfig->getBooleanParams()->_printToScreen; // false;
    const bool printIndivSolnMetrics = pUserConfig->getBooleanParams()->_printIndivSolnMetrics; // true;
    const bool populateInitOpenTrips = pUserConfig->getBooleanParams()->_populateInitOpenTrips; // false;
        
    ModelRunner::DataInputValues  * pDataInput  = new ModelRunner::DataInputValues(inputPath, outputBasePath, pInstance->getInputCsvFilename(), pInstance->getSimStartTimeString(), pInstance->getSimLengthInMin(), populateInitOpenTrips);    
    ModelRunner::DataOutputValues * pDataOutput = new ModelRunner::DataOutputValues(outputBasePath,printDebugFiles,printIndivSolnMetrics,printToScreen);
    
    // specify DEFAULT values
    const double default_optInRate               = pUserConfig->getDoubleParams()->_defaultOptInRate; // 0.40;
    const int    default_upFrontBatchWindowInSec = pUserConfig->getIntParams()->_default_upFrontBatchWindowInSec; // 30;
    const double default_maxMatchDistInKm        = pUserConfig->getDoubleParams()->_default_maxMatchDistInKm; // 3.0;
    const double default_minPoolDiscount         = pUserConfig->getDoubleParams()->_default_minPoolDiscount; // 0.2;
    const double default_flexDepOptInRate        = pUserConfig->getDoubleParams()->_flexDepOptInRate; // 0.25;
    const int    default_flexDepWindowInSec      = pUserConfig->getIntParams()->_flexDepWindowInSec; // 600;
    const int    default_maxAllowablePickups     = pUserConfig->getIntParams()->_maxAllowablePickups; 
        
    ModelRunner::DefaultModelParameters * pDefaultInputs = new ModelRunner::DefaultModelParameters(default_optInRate, default_upFrontBatchWindowInSec, default_maxMatchDistInKm, default_minPoolDiscount, default_flexDepOptInRate, default_flexDepWindowInSec, default_maxAllowablePickups);
                
    const bool inclInitPickupInSavingsConstr = true;
    
    // specify RANGES to iterate experiments                 
    std::vector<double> range_optInRate = pUserConfig->getRangeParams()->optInRange; // defineOptInRange(); // opt-in ranges: 0.20, 0.30, 0.40, 0.50, 0.60, 0.70, 0.80, 0.90, 1.0            
    std::vector<double> range_upFrontBatchWindowInSec = pUserConfig->getRangeParams()->batchWindowRange; // defineBatchWindowRange(); // batch window values: 15, 30, 45, 60, 75, 90, 120, 150, 300, 600            
    std::vector<double> range_maxMatchDistInKm = pUserConfig->getRangeParams()->maxPickupRange; // defineMaxPickupDistRange(); // max pickup range (km): 0.5, 1.0, 1.5, 2.0, 2.5, 3.0, 3.5, 4.0, 4.5, 5.0, 5.5, 6.0, 10.0           
    std::vector<double> range_minPoolDiscount = pUserConfig->getRangeParams()->minSavingsRange; // defineMinPoolDiscountRange();  // max pool discount for master: 0.05, 0.10, 0.15, 0.20, 0.25, 0.30, 0.35, 0.40, 0.45, 0.50
        
    // uberPOOL TEST MODELS RUN
    const bool runMITMModel        = pUserConfig->getBooleanParams()->_runMITMModel;
    const bool runUFBW_seqPickups  = pUserConfig->getBooleanParams()->_runUFBW_seqPickups;
    //const bool runUFBW_flexPickups = false; 
    const bool runFlexDepModel     = pUserConfig->getBooleanParams()->_runFlexDepModel;
    const bool runUFBW_perfectInfo = pUserConfig->getBooleanParams()->_runUFBW_perfInfo; 
    const bool runMultiplePickups  = pUserConfig->getBooleanParams()->_runMultiplePickups;
            
    // create ModelRunner object which controls all optimizations 
    ModelRunner * pModelRunner = new ModelRunner( pUserConfig->getEnumParams()->_experiment, runMITMModel, runUFBW_seqPickups, runFlexDepModel, runUFBW_perfectInfo, runMultiplePickups, pDataInput, pDataOutput, pDefaultInputs, pInstance->getGeofence() );     
    pModelRunner->setInputValues(range_optInRate, range_upFrontBatchWindowInSec, range_maxMatchDistInKm, range_minPoolDiscount);
    pModelRunner->setInclInitPickupDistForSavingsConstr(inclInitPickupInSavingsConstr);
    
    // ---- SOLVE MODELS ----
    const std::map<double, SolnMaps*> * pInputValSolnMap = pModelRunner->runAllModels();

    // ---- PRINT SUMMARY SOLUTION ----
    pModelRunner->printSolutionSummaryForCurrExperiment(pInputValSolnMap);
      
    std::cout << "\n\n\n--- success! ---\n" << std::endl;
  
    return 0;
}

// define ranges for experiment
std::vector<double> defineBatchWindowRange() {
    std::vector<double> batchWindowRange;
    
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
    
    optInRange.push_back(0.10); 
    optInRange.push_back(0.20);
    optInRange.push_back(0.30);
    optInRange.push_back(0.40);
    optInRange.push_back(0.50);
    optInRange.push_back(0.60);
    optInRange.push_back(0.70);
    optInRange.push_back(0.80);
    optInRange.push_back(0.90);
    optInRange.push_back(1.0);
         
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

std::vector<double> getExperimentInputValues(const Experiment &experiment, std::vector<double> &optInValues, std::vector<double> &batchWindowValues, std::vector<double> &maxPickupDistValues, std::vector<double> &minSavingsValues) {
    switch( experiment ) {
        case OPTIN :
            return optInValues;
        case BATCHWINDOW :
            return batchWindowValues;
        case PICKUP :
            return maxPickupDistValues;
        case SAVINGSRATE :
            return minSavingsValues;
        case DEFAULTVALUES :
        {
            std::vector<double> emptyVec;
            return emptyVec;
        }
        default :
        {
            std::cout << "\n\n** ERROR: Experiment type not supported **\n\n" << std::endl;
            exit(0);
            std::vector<double> emptyVec;
            return emptyVec;
        }
    }
}

void printMetricsToScreen(UserConfig * pUserConfig) {
    cout << "\n\n--- summary of metrics ---\n" << endl;
    cout << "BOOLEAN PARAMETERS" << endl;
    const UserConfig::BoolParams * pBoolParams = pUserConfig->getBooleanParams();
    std::string runMITM = (pBoolParams->_runMITMModel) ? "true" : "false";
    std::string runUFBW = (pBoolParams->_runUFBW_seqPickups) ? "true" : "false";
    std::string runPI = (pBoolParams->_runUFBW_perfInfo) ? "true" : "false";
    std::string runFD = (pBoolParams->_runFlexDepModel) ? "true" : "false";
    std::string runMultPicks = (pBoolParams->_runMultiplePickups) ? "true" : "false";
    std::string printDebug = (pBoolParams->_printDebugFiles) ? "true" : "false";
    std::string printScreen = (pBoolParams->_printToScreen) ? "true" : "false";
    std::string printIndiv = (pBoolParams->_printIndivSolnMetrics) ? "true" : "false";
    std::string populateInit = (pBoolParams->_populateInitOpenTrips) ? "true" : "false";   
    std::string inclInitPickupSavings = (pBoolParams->_inclInitPickupInSavingsConstr) ? "true" : "false";
    cout << "  " << left << setw(35) << "run MITM: " << runMITM << endl;
    cout << "  " << left << setw(35) << "run UFBW: " << runUFBW << endl;
    cout << "  " << left << setw(35) << "run PI: " << runPI << endl;
    cout << "  " << left << setw(35) << "run FD: " << runFD << endl;
    cout << "  " << left << setw(35) << "run mult pickups: " << runMultPicks << endl;
    cout << "  " << left << setw(35) << "incl init pickup in savings: " << inclInitPickupSavings << endl;
    cout << "  " << left << setw(35) << "print debug files:  " << printDebug << endl;
    cout << "  " << left << setw(35) << "print to screen: " << printScreen << endl;
    cout << "  " << left << setw(35) << "print indiv metrics: " << printIndiv << endl;
    cout << "  " << left << setw(35) << "populate init open trips: " << populateInit << endl;
        
    cout << "\nINTEGER PARAMETERS" << endl;
    const UserConfig::IntParams * pIntParams = pUserConfig->getIntParams();
    std::string scen = Utility::intToStr(pIntParams->_scenNumber);
    std::string maxPickups = Utility::intToStr(pIntParams->_maxAllowablePickups);
    std::string ufbwSec = Utility::intToStr(pIntParams->_default_upFrontBatchWindowInSec);
    std::string flexDepWindowSec = Utility::intToStr(pIntParams->_flexDepWindowInSec);
    cout << "  " << left << setw(35) << "scenario number: " << scen << endl;
    cout << "  " << left << setw(35) << "max pickups: " << maxPickups << endl;
    cout << "  " << left << setw(35) << "up front batch window sec: " << ufbwSec << endl;
    cout << "  " << left << setw(35) << "flex dep window in sec: " << flexDepWindowSec << endl;
    
    
    cout << "\nDOUBLE PARAMETERS" << endl;
    const UserConfig::DoubleParams * pDoubleParams = pUserConfig->getDoubleParams();
    std::string defaultOptIn = Utility::doubleToStr(pDoubleParams->_defaultOptInRate);
    std::string defaultMaxDist = Utility::doubleToStr(pDoubleParams->_default_maxMatchDistInKm);
    std::string defaultMinSavings = Utility::doubleToStr(pDoubleParams->_default_minPoolDiscount);
    std::string flexDepOptIn = Utility::doubleToStr(pDoubleParams->_flexDepOptInRate);
    cout << "  " << left << setw(35) << "default optIn rate: " << defaultOptIn << endl;
    cout << "  " << left << setw(35) << "default max dist (km): " << defaultMaxDist << endl;
    cout << "  " << left << setw(35) << "default min savings (%):" << defaultMinSavings << endl;
    cout << "  " << left << setw(35) << "flex dep optIn rate: " << flexDepOptIn << endl;
    
    cout << "\nSTRING PARAMETERS" << endl;
    const UserConfig::StringParams * pStringParams = pUserConfig->getStringParams();
    cout << "  " << left << setw(35) << "input path:  " << pStringParams->_inputPath << endl;
    cout << "  " << left << setw(35) << "output base path: " << pStringParams->_outputBasePath << endl;
    cout << "  " << left << setw(35) << "geofence: " << pStringParams->_geofenceDataPath << endl;
    
    cout << "\nENUM PARAMETERS" << endl;
    const UserConfig::EnumParams * pEnumParams = pUserConfig->getEnumParams();
    std::string experimentStr;
    switch( pEnumParams->_experiment ) {
        case DEFAULTVALUES :
            experimentStr = "default_values";
            break;
        case OPTIN :
            experimentStr = "opt_in";
            break;
        case BATCHWINDOW :
            experimentStr = "batch_window";
            break;
        case PICKUP :
            experimentStr = "max_pickup_dist";
            break;
        case SAVINGSRATE :
            experimentStr = "min_savings";
            break;
        default :
            experimentStr = "OTHER";
    }
    cout << "  " << left << setw(35) << "experiment: " << experimentStr << endl;
    
    cout << "\nRANGE PARAMETERS FOR EXPERIMENTS" << endl;
    std::vector<double> optInRange = pUserConfig->getRangeParams()->optInRange;
    
    
    
    cout << "  " << left << setw(15) << "opt-in: ";
    int optInCounter = 0;
    for( std::vector<double>::iterator itr = optInRange.begin(); itr != optInRange.end(); ++itr ) {
        if( optInCounter == 0 ) {
            cout << "( ";
        } else {
            cout << ",";
        }
        cout << Utility::doubleToStr(*itr) << " ";
        optInCounter++;
    }
    cout << ")" << endl;
    
    std::vector<double> batchWindowRange = pUserConfig->getRangeParams()->batchWindowRange;
    cout << "  " << left << setw(15) << "batchWindow:";
    int batchCounter = 0;
    for( std::vector<double>::iterator itr = batchWindowRange.begin(); itr != batchWindowRange.end(); ++itr ) {
        if( batchCounter == 0 ) {
            cout << "( ";
        } else {
            cout << ",";
        }
        cout << Utility::doubleToStr(*itr) << " ";
        batchCounter++;
    }
    cout << ")" << endl;
    
    std::vector<double> maxPickupRange = pUserConfig->getRangeParams()->maxPickupRange;
    cout << "  " << left << setw(15) << "maxPickupDist:";
    int pickupCounter = 0;
    for( std::vector<double>::iterator itr = maxPickupRange.begin(); itr != maxPickupRange.end(); ++itr ) {
        if( pickupCounter == 0 ) {
            cout << "( ";
        } else {
            cout << ",";
        }
        cout << Utility::doubleToStr(*itr) << " ";
        pickupCounter++;
    }
    cout << ")" << endl;
    
    std::vector<double> minSavingsRange = pUserConfig->getRangeParams()->minSavingsRange;
    cout << "  " << left << setw(15) << "minSavings:";
    int savingsCounter = 0;
    for( std::vector<double>::iterator itr = minSavingsRange.begin(); itr != minSavingsRange.end(); ++itr ) {
        if( savingsCounter == 0 ) {
            cout << "( ";
        } else {
            cout << ",";
        }
        cout << Utility::doubleToStr(*itr) << " ";
        savingsCounter++;
    }
    cout << ")" << endl;
}