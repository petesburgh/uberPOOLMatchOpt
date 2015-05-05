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



std::vector<double> getExperimentInputValues(const ModelRunner::Experiment &experiment, std::vector<double>&, std::vector<double>&, std::vector<double>&, std::vector<double>&);

/*SolnMaps * runModels_defaultValues             ( bool runMITMModel, bool runUFBW_seqPickups, bool runFlexDepartureModel, bool runUFBW_perfectInfo, ModelRunner::DataInputValues * pDataInput, ModelRunner::DataOutputValues * pDataOutput, ModelRunner::DefaultModelParameters * pDefaultValues, const std::vector<Geofence*> * pGeofences);
SolnMaps * runModels_optInScenarios            ( bool runMITMModel, bool runUFBW_seqPickups, bool runFlexDepartureModel, bool runUFBW_perfectInfo, ModelRunner::DataInputValues * pDataInput, ModelRunner::DataOutputValues * pDataOutput, ModelRunner::DefaultModelParameters * pDefaultValues, std::vector<double> * pOptInValues, const std::vector<Geofence*> * pGeofences);
SolnMaps * runModels_batchWindowScenarios      ( bool runMITMModel, bool runUFBW_seqPickups, bool runFlexDepartureModel, bool runUFBW_perfectInfo, ModelRunner::DataInputValues * pDataInput, ModelRunner::DataOutputValues * pDataOutput, ModelRunner::DefaultModelParameters * pDefaultValues, std::vector<double> * pBatchWindowValues, const std::vector<Geofence*> * pGeofences );
SolnMaps * runModels_maxPickupDistanceScenarios( bool runMITMModel, bool runUFBW_seqPickups, bool runFlexDepartureModel, bool runUFBW_perfectInfo, ModelRunner::DataInputValues * pDataInput, ModelRunner::DataOutputValues * pDataOutput, ModelRunner::DefaultModelParameters * pDefaultValues, std::vector<double> * pMaxPickupValues, const std::vector<Geofence*> * pGeofences );
SolnMaps * runModels_minSavingsScenarios       ( bool runMITMModel, bool runUFBW_seqPickups, bool runFlexDepartureModel, bool runUFBW_perfectInfo, ModelRunner::DataInputValues * pDataInput, ModelRunner::DataOutputValues * pDataOutput, ModelRunner::DefaultModelParameters * pDefaultValues, std::vector<double> * pMinSavingsValues, const std::vector<Geofence*> * pGeofences );
*/

std::vector<double> defineBatchWindowRange();
std::vector<double> defineOptInRange();
std::vector<double> defineMaxPickupDistRange();
std::vector<double> defineMinPoolDiscountRange();

void printBanner(ParameterSet * pParamSet, int N);
void printSummaryOfDataInput(DataContainer*);
void printDriverInfo(DataContainer*);
void printRiderInfo(const std::set<Rider*, RiderIndexComp>*);
//void writeAndPrintInputs(DataContainer*, Output*);

//void printSolutionMetricsForCurrExperiment(SolnMaps * pSolnMaps, std::vector<double> inputRange, int currExperiment, const std::string outputBasePath);
void printSolutionMetricsForCurrExperiment2(const std::map<double, SolnMaps*> * pModelSolnMap, int currExperiment, const std::string outputBasePath);
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
        
    ModelRunner::DataInputValues * pDataInput = new ModelRunner::DataInputValues(inputPath, outputBasePath, pInstance->getInputCsvFilename(), pInstance->getSimStartTimeString(), pInstance->getSimLengthInMin(), populateInitOpenTrips);    
    ModelRunner::DataOutputValues * pDataOutput = new ModelRunner::DataOutputValues(outputBasePath,printDebugFiles,printToScreen);
        
    // specify DEFAULT values
    const double default_optInRate               = 0.40;
    const int    default_upFrontBatchWindowInSec = 30;
    const double default_maxMatchDistInKm        = 3.0;
    const double default_minPoolDiscount         = 0.2;
    const double default_flexDepOptInRate        = 0.25;
    const int    default_flexDepWindowInSec      = 600;
        
    ModelRunner::DefaultModelParameters * pDefaultInputs = new ModelRunner::DefaultModelParameters(default_optInRate,default_upFrontBatchWindowInSec,default_maxMatchDistInKm,default_minPoolDiscount,default_flexDepOptInRate,default_flexDepWindowInSec);
                
    // specify RANGES to iterate experiments                 
    std::vector<double> range_optInRate = defineOptInRange(); // opt-in ranges: 0.20, 0.30, 0.40, 0.50, 0.60, 0.70, 0.80, 0.90, 1.0            
    std::vector<double> range_upFrontBatchWindowInSec = defineBatchWindowRange(); // batch window values: 15, 30, 45, 60, 75, 90, 120, 150, 300, 600            
    std::vector<double> range_maxMatchDistInKm = defineMaxPickupDistRange(); // max pickup range (km): 0.5, 1.0, 1.5, 2.0, 2.5, 3.0, 3.5, 4.0, 4.5, 5.0, 5.5, 6.0, 10.0           
    std::vector<double> range_minPoolDiscount = defineMinPoolDiscountRange();  // max pool discount for master: 0.05, 0.10, 0.15, 0.20, 0.25, 0.30, 0.35, 0.40, 0.45, 0.50
        
    // TYPE OF TESTS TO RUN
    const bool runMITMModel        = true;
    const bool runUFBW_seqPickups  = true;
    //const bool runUFBW_flexPickups = false; 
    const bool runFlexDepModel     = false;
    const bool runUFBW_perfectInfo = false; 
        
    // SPECIFY TYPE OF EXPERIMENT
    const ModelRunner::Experiment experiment = ModelRunner::PICKUP;  //DEFAULTVALUES, OPTIN, BATCHWINDOW, PICKUP, SAVINGSRATE
    
    // create ModelRunner object which controls all optimizations 
    ModelRunner * pModelRunner = new ModelRunner( experiment, runMITMModel, runUFBW_seqPickups, runFlexDepModel, runUFBW_perfectInfo, pDataInput, pDataOutput, pDefaultInputs, pInstance->getGeofences() );     
    pModelRunner->setInputValues(range_optInRate, range_upFrontBatchWindowInSec, range_maxMatchDistInKm, range_minPoolDiscount);
              
    
    const std::map<double, SolnMaps*> * pInputValSolnMap = pModelRunner->runAllModels();
    
    std::cout << "\n\nhere is the map... " << std::endl;
    for( std::map<double, SolnMaps*>::const_iterator it = pInputValSolnMap->begin(); it != pInputValSolnMap->end(); ++it ) {
        std::cout << "INPUT VALUE: " << Utility::doubleToStr(it->first) << std::endl;
        std::map<ModelEnum, double> matchRateMap = it->second->matchRateMap;
        for( std::map<ModelEnum, double>::iterator it2 = matchRateMap.begin(); it2 != matchRateMap.end(); ++it2 ) {            
            std::string modelStr = "-";
            switch( it2->first ) {
                case MITM_SEQ_PICKUPS :
                {
                    modelStr = "MITM";
                    break;
                }
                case UFBW_FIXED_PICKUPS :
                {
                    modelStr = "UFBW";
                    break;
                }
                default :
                    modelStr = "other";
            }
            std::cout << "\t" << modelStr << ": " << Utility::truncateDouble(it2->second,4) << std::endl;
        }                
    }
    
    
            
    //PRINT SUMMARY SOLUTION
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
   /* maxPickupRange.push_back(2.0);
    maxPickupRange.push_back(2.5);
    maxPickupRange.push_back(3.0);
    maxPickupRange.push_back(3.5);
    maxPickupRange.push_back(4.0);
    maxPickupRange.push_back(4.5);
    maxPickupRange.push_back(5.0);
    maxPickupRange.push_back(5.5);
    maxPickupRange.push_back(6.0);
    maxPickupRange.push_back(10.0);*/
    
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


void printSolutionMetricsForCurrExperiment2(const std::map<double, SolnMaps*> * pModelSolnMap, int currExperiment, const std::string outputBasePath) {
    
    std::string scenName = "UNKNOWN";
    std::string inputName = "";
    
    switch( currExperiment ) {
        case ModelRunner::OPTIN :
        {
            scenName   = "SUMMARY-OPT-IN";
            inputName  = "optIn(%)";
            break;
        }
        case ModelRunner::BATCHWINDOW :
        {
            scenName   = "SUMMARY-BATCH-WINDOW";
            inputName  = "batchWindow(sec)";
            break;
        }
        case ModelRunner::PICKUP :
        {
            scenName   = "SUMMARY-MAX-PICKUP-DIST";
            inputName  = "maxPickupDist(km)";
            break;
        }
        case ModelRunner::SAVINGSRATE : 
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

    
    
   /* printInputRequestsMetrics( outFile, inputName, &(pSolnMaps->input_numRequests) );
    printMatchRateMetrics    ( outFile, inputName, &inputRange, &(pSolnMaps->matchRate_MITM), &(pSolnMaps->matchRate_UFBW), &(pSolnMaps->matchRage_FD), &(pSolnMaps->matchRate_UFBW_PI), &(pSolnMaps->matchRate_FD_FDReqs), &(pSolnMaps->matchRate_FD_nonFDReqs) );
    printInconvenienceMetrics( outFile, inputName, &inputRange, &(pSolnMaps->inconv_MITM),    &(pSolnMaps->inconv_UFBW), &(pSolnMaps->inconv_FD), &(pSolnMaps->inconv_UFBW_PI) );
    printNumTripsMetrics     ( outFile, inputName, &inputRange, &(pSolnMaps->numTrips_MITM),   &(pSolnMaps->numTrip_UFBW), &(pSolnMaps->numTrips_FD), &(pSolnMaps->numTrips_UFBW_PI) ); */
    
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

/*void printSummaryOfDataInput(DataContainer * pDataContainer) {
    
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
}*/
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

std::vector<double> getExperimentInputValues(const ModelRunner::Experiment &experiment, std::vector<double> &optInValues, std::vector<double> &batchWindowValues, std::vector<double> &maxPickupDistValues, std::vector<double> &minSavingsValues) {
    switch( experiment ) {
        case ModelRunner::OPTIN :
            return optInValues;
        case ModelRunner::BATCHWINDOW :
            return batchWindowValues;
        case ModelRunner::PICKUP :
            return maxPickupDistValues;
        case ModelRunner::SAVINGSRATE :
            return minSavingsValues;
        case ModelRunner::DEFAULTVALUES :
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