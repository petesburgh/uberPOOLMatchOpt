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

    // populate input and output structures belonging to ModelRunner
    ModelRunner::DataInputValues  * pDataInput  = new ModelRunner::DataInputValues(pUserConfig);
    ModelRunner::DataOutputValues * pDataOutput = new ModelRunner::DataOutputValues(pUserConfig);

    // populate default input values
    ModelRunner::DefaultModelParameters * pDefaultInputs = new ModelRunner::DefaultModelParameters(pUserConfig); // default_optInRate, default_upFrontBatchWindowInSec, default_maxMatchDistInKm, default_minPoolDiscount, default_flexDepOptInRate, default_flexDepWindowInSec, default_maxAllowablePickups);
             
    // create ModelRunner object which controls all optimizations 
    ModelRunner * pModelRunner = new ModelRunner( pUserConfig->getEnumParams()->_experiment, pUserConfig, pDataInput, pDataOutput, pDefaultInputs );     
    pModelRunner->extractGeofence(pUserConfig->getStringParams()->_geofenceData);
    pModelRunner->setInputValues(pUserConfig); //range_optInRate, range_upFrontBatchWindowInSec, range_maxMatchDistInKm, range_minPoolDiscount);
    pModelRunner->setInclInitPickupDistForSavingsConstr(pUserConfig->getBooleanParams()->_inclInitPickupInSavingsConstr);
    
    // ---- SOLVE MODELS ----
    const std::map<double, SolnMaps*> * pInputValSolnMap = pModelRunner->runAllModels();

    // ---- PRINT SUMMARY SOLUTION ----
    pModelRunner->printSolutionSummaryForCurrExperiment(pInputValSolnMap);
      
    std::cout << "\n\n\n--- success! ---\n" << std::endl;
  
    return 0;
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
    cout << "\n\n--- summary of input parameters ---\n" << endl;
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
    cout << "  " << left << setw(31) << "run MITM: " << runMITM << endl;
    cout << "  " << left << setw(31) << "run UFBW: " << runUFBW << endl;
    cout << "  " << left << setw(31) << "run PI: " << runPI << endl;
    cout << "  " << left << setw(31) << "run FD: " << runFD << endl;
    cout << "  " << left << setw(31) << "run mult pickups: " << runMultPicks << endl;
    cout << "  " << left << setw(31) << "incl init pickup in savings: " << inclInitPickupSavings << endl;
    cout << "  " << left << setw(31) << "print debug files:  " << printDebug << endl;
    cout << "  " << left << setw(31) << "print to screen: " << printScreen << endl;
    cout << "  " << left << setw(31) << "print indiv metrics: " << printIndiv << endl;
    cout << "  " << left << setw(31) << "populate init open trips: " << populateInit << endl;
        
    cout << "\nINTEGER PARAMETERS" << endl;
    const UserConfig::IntParams * pIntParams = pUserConfig->getIntParams();
    std::string maxPickups = Utility::intToStr(pIntParams->_maxAllowablePickups);
    std::string ufbwSec = Utility::intToStr(pIntParams->_default_upFrontBatchWindowInSec);
    std::string flexDepWindowSec = Utility::intToStr(pIntParams->_flexDepWindowInSec);
    std::string simLenInMin = Utility::intToStr(pIntParams->_simLengthInMin);
    cout << "  " << left << setw(30) << "sim length in min: " << simLenInMin << endl;
    cout << "  " << left << setw(30) << "max pickups: " << maxPickups << endl;
    cout << "  " << left << setw(30) << "up front batch window sec: " << ufbwSec << endl;
    cout << "  " << left << setw(30) << "flex dep window in sec: " << flexDepWindowSec << endl;
    
    
    cout << "\nDOUBLE PARAMETERS" << endl;
    const UserConfig::DoubleParams * pDoubleParams = pUserConfig->getDoubleParams();
    std::string defaultOptIn = Utility::doubleToStr(pDoubleParams->_defaultOptInRate);
    std::string defaultMaxDist = Utility::doubleToStr(pDoubleParams->_default_maxMatchDistInKm);
    std::string defaultMinSavings = Utility::doubleToStr(pDoubleParams->_default_minPoolDiscount);
    std::string flexDepOptIn = Utility::doubleToStr(pDoubleParams->_flexDepOptInRate);
    cout << "  " << left << setw(28) << "default optIn rate: " << defaultOptIn << endl;
    cout << "  " << left << setw(28) << "default max dist (km): " << defaultMaxDist << endl;
    cout << "  " << left << setw(28) << "default min savings (%):" << defaultMinSavings << endl;
    cout << "  " << left << setw(28) << "flex dep optIn rate: " << flexDepOptIn << endl;
    
    cout << "\nSTRING PARAMETERS" << endl;
    const UserConfig::StringParams * pStringParams = pUserConfig->getStringParams();
    cout << "  " << left << setw(20) << "input data: " << pStringParams->_inputData << endl;
    cout << "  " << left << setw(20) << "geofence data: " << pStringParams->_geofenceData << endl;
    cout << "  " << left << setw(20) << "output base path: " << pStringParams->_outputBasePath << endl;
    cout << "  " << left << setw(20) << "sim start time: " << pStringParams->_simStartTime << endl;
    
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
    cout << "  " << left << setw(20) << "experiment: " << experimentStr << endl;
    
    std::string geofenceTypeStr = "";
    const Geofence::Type pGeoType = pUserConfig->getEnumParams()->_geofenceType;
    switch( pGeoType ) {
        case Geofence::REQ_ONLY :
            geofenceTypeStr = "request_only";
            break;
        case Geofence::ORIG_ONLY :
            geofenceTypeStr = "orig_only";
            break;
        case Geofence::ENTIRE_TRIP : 
            geofenceTypeStr = "entire_trip";
            break;
        default : 
            geofenceTypeStr = "OTHER";
    }
    cout << "  " << left << setw(20) << "geofence restr: " << geofenceTypeStr << endl;
    
    cout << "\nRANGE PARAMETERS FOR EXPERIMENTS" << endl;
    std::vector<double> optInRange = pUserConfig->getRangeParams()->optInRange;
    
    cout << "  " << left << setw(18) << "opt-in: ";
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
    cout << "  " << left << setw(18) << "batchWindow:";
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
    cout << "  " << left << setw(18) << "maxPickupDist:";
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
    cout << "  " << left << setw(18) << "minSavings:";
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