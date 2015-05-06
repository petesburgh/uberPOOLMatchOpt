/* 
 * File:   ModelRunner.cpp
 * Author: jonpetersen
 * 
 * Created on May 1, 2015, 3:54 PM
 */

#include "ModelRunner.hpp"
#include "UFBW_perfectInformation.hpp"

ModelRunner::ModelRunner( const ModelRunner::Experiment &experiment, const bool &runMITMModel, const bool &runUFBW_seqPickups, const bool &runFlexDepModel, const bool &runUFBW_perfectInfo, DataInputValues * dataInput, DataOutputValues * dataOutput, DefaultModelParameters * defaultValues, const std::vector<Geofence*> * geofences ) : 
    _experiment(experiment), _runMitmModel(runMITMModel), _runUFBW_fixedPickup(runUFBW_seqPickups), _runUFBW_pickupSwap(false), _runFlexDeparture(runFlexDepModel), _runUFBW_PI(runUFBW_perfectInfo), pDataInput(dataInput), pDataOutput(dataOutput) , pDefaultValues(defaultValues), pGeofences(geofences) {      
}

ModelRunner::~ModelRunner() {
}

DataContainer * ModelRunner::constructDataContainer(double optInRate, int batchWindowLengthInSec) {
    
    DataContainer * pDataContainer = NULL;
    
    /* 
     * STEP 1: read parse CSV input 
    */
    pDataContainer = new DataContainer(pDataInput->_inputPath, pDataInput->_cvsFilename, pDataInput->_timelineStr, optInRate, pDataInput->_simLengthInMinutes, pDataOutput->_printDebugFiles, pDataOutput->_printToScreen, pGeofences);
    pDataContainer->setBatchWindowInSeconds(batchWindowLengthInSec);
    pDataContainer->extractCvsSnapshot();  

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
    _initRequests = pDataContainer->getInitPoolRequestsAtTimeline();
    _allRequestsInSim = pDataContainer->getAllPoolRequestsInSim();
    _initOpenTrips = pDataContainer->getInitOpenTripsAtTimeline();

    return pDataContainer;
}

std::map<double, SolnMaps*> * ModelRunner::runAllModels() {
    std::map<double, SolnMaps*> * pModelSolnMap = new std::map<double, SolnMaps*>();
    
    // run DEFAULT VALUES
    switch( _experiment ) {
        case ModelRunner::DEFAULTVALUES :
        {             
            const std::string outputExpPath = pDataOutput->_outputBasePath + "defaultParams/";
            pOutput = new Output(pDataOutput->_outputBasePath, outputExpPath);
            std::string outputScenarioPath = outputExpPath;
            pOutput->setOutputScenarioPath(outputScenarioPath);
            
            const std::map<const ModelEnum, ModelRunner::SolnMetrics*> * pSolnMetrics_defaultVals = runModelsForCurrExperiment(pDefaultValues->_optInRate, pDefaultValues->_batchWindowLengthInSec, pDefaultValues->_maxPickupDistance, pDefaultValues->_minSavings); // second input is a dummy argument                     
            updateModelSolnMaps(pModelSolnMap, pSolnMetrics_defaultVals, -1);
            break;
        }
        case ModelRunner::OPTIN :
        {
            for( std::vector<double>::iterator optInItr = _range_optInValues.begin(); optInItr != _range_optInValues.end(); ++optInItr ) {
                
                const std::string outputExpPath = pDataOutput->_outputBasePath + "OPTIN/";
                pOutput = new Output(pDataOutput->_outputBasePath, outputExpPath);
                const std::string outputScenarioPath = outputExpPath + "scen-" + Utility::doubleToStr(*optInItr) + "/";
                pOutput->setOutputScenarioPath(outputScenarioPath);
                Utility::createFolder(outputScenarioPath);
            
                std::map<const ModelEnum, ModelRunner::SolnMetrics*> * pSolnMapCurrOptInRate = runModelsForCurrExperiment(*optInItr, pDefaultValues->_batchWindowLengthInSec, pDefaultValues->_maxPickupDistance, pDefaultValues->_minSavings );
                updateModelSolnMaps(pModelSolnMap, pSolnMapCurrOptInRate, *optInItr);
            }
            break;
        }
        case ModelRunner::BATCHWINDOW :
        {
            for( std::vector<double>::iterator batchWindowItr = _range_batchWindowValues.begin(); batchWindowItr != _range_batchWindowValues.end(); ++batchWindowItr ) {
                
                const std::string outputExpPath = pDataOutput->_outputBasePath + "BATCHWINDOW/";
                pOutput = new Output(pDataOutput->_outputBasePath, outputExpPath);
                const std::string outputScenarioPath = outputExpPath + "scen-" + Utility::doubleToStr(*batchWindowItr) + "/";
                pOutput->setOutputScenarioPath(outputScenarioPath);      
                Utility::createFolder(outputScenarioPath);
                
                std::map<const ModelEnum, ModelRunner::SolnMetrics*> * pSolnMapBatchWindow = runModelsForCurrExperiment(pDefaultValues->_optInRate, *batchWindowItr, pDefaultValues->_maxPickupDistance, pDefaultValues->_minSavings );
                updateModelSolnMaps(pModelSolnMap, pSolnMapBatchWindow, *batchWindowItr);
            }
            break;
        }
        case ModelRunner::PICKUP :
        {
            for( std::vector<double>::iterator maxPickupItr = _range_maxPickupDistValues.begin(); maxPickupItr != _range_maxPickupDistValues.end(); ++maxPickupItr ) {
                
                const std::string outputExpPath = pDataOutput->_outputBasePath + "MAXPICKUP/";
                pOutput = new Output(pDataOutput->_outputBasePath, outputExpPath);
                const std::string outputScenarioPath = outputExpPath + "scen-" + Utility::doubleToStr(*maxPickupItr) + "/";
                pOutput->setOutputScenarioPath(outputScenarioPath);     
                Utility::createFolder(outputScenarioPath);
                
                std::map<const ModelEnum, ModelRunner::SolnMetrics*> * pSolnMapMaxPickupDist = runModelsForCurrExperiment( pDefaultValues->_optInRate, pDefaultValues->_batchWindowLengthInSec, *maxPickupItr, pDefaultValues->_minSavings );
                updateModelSolnMaps(pModelSolnMap, pSolnMapMaxPickupDist, *maxPickupItr);
            }
            break;
        }        
        case ModelRunner::SAVINGSRATE :
        {
            for( std::vector<double>::iterator minSavingsItr = _range_minSavingsValues.begin(); minSavingsItr != _range_minSavingsValues.end(); ++minSavingsItr ) {
                
                const std::string outputExpPath = pDataOutput->_outputBasePath + "MINSAVINGS/";
                pOutput = new Output(pDataOutput->_outputBasePath, outputExpPath);
                const std::string outputScenarioPath = outputExpPath + "scen-" + Utility::doubleToStr(*minSavingsItr) + "/";
                pOutput->setOutputScenarioPath(outputScenarioPath);                
                Utility::createFolder(outputScenarioPath);
                
                std::map<const ModelEnum, ModelRunner::SolnMetrics*> * pSolnMapMinSavings = runModelsForCurrExperiment( pDefaultValues->_optInRate, pDefaultValues->_batchWindowLengthInSec, pDefaultValues->_maxPickupDistance, *minSavingsItr );
                updateModelSolnMaps(pModelSolnMap, pSolnMapMinSavings, *minSavingsItr);
            }
            break;
        }
        default :
        {
            std::cout << "\n\n\n*** ERROR: Experiment not defined (expecting one from {DEFAULTVALUES, OPTIN, BATCHWINDOW, PICKUP, SAVINGSRATE} ***\n\n" << std::endl;
            std::cout << "\texiting unsuccessfully...\n" << std::endl;
            exit(1);
        }
    }    
    
    return pModelSolnMap;
}

std::map<const ModelEnum, ModelRunner::SolnMetrics*> * ModelRunner::runModelsForCurrExperiment(double optInRate, double batchWindowLengthInSecDouble, double maxPickupDistance, double minPctSavings) {
        
    std::map<const ModelEnum, ModelRunner::SolnMetrics*> * pModelSolnMetricMap = new std::map<const ModelEnum, ModelRunner::SolnMetrics*>();
    
    int batchWindowLengthInSec = (int)batchWindowLengthInSecDouble;
    DataContainer * pDataContainer = constructDataContainer(optInRate, batchWindowLengthInSec);
    
    pOutput->setDataContainer(pDataContainer);
    pOutput->writeAndPrintInputs(pDataContainer, pDataOutput->_printDebugFiles); 
       
    
    /*
     *    MAN IN THE MUNI (MITM) MODEL 
     */
    if( _runMitmModel ) {
        MitmModel * pMitmModel = new MitmModel(pDataContainer->getTimeline(), pDataContainer->getSimEndTime(), maxPickupDistance, minPctSavings, _allRequestsInSim, _initOpenTrips, pDataContainer->getAllDrivers(), _inclInitPickupDistForSavingsConstr);
        bool modelSolved = pMitmModel->solve(pDataOutput->_printDebugFiles, pOutput, pDataInput->_populateInitOpenTrips);     
        if( modelSolved ) {          
            Solution * pMitmSolution = pMitmModel->getSolution();            
            pOutput->printSolution(pMitmSolution, MITM_SEQ_PICKUPS);
                       
            // populate solution for current param setting (inner map entry)
            ModelRunner::SolnMetrics * pSolnMetrics = new SolnMetrics(); 
            pSolnMetrics->numRequests = pMitmSolution->getTotalRequests();
            pSolnMetrics->matchRate = pMitmSolution->getRequestMetrics()->_matchedPercentage;
            pSolnMetrics->inconvenience = pMitmSolution->getInconvenienceMetrics()->_avgPctAddedDistsForAll;
            pSolnMetrics->numTrips = pMitmSolution->getTotalNumTripsFromSoln();

            pModelSolnMetricMap->insert(make_pair(MITM_SEQ_PICKUPS, pSolnMetrics));
        }     
    }
    
    /*
     *    UP FRONT BATCHING WINDOW WITH FIXED PICKUPS
     */
    if( _runUFBW_fixedPickup ) {
        UFBW_fixed * pFixedBatchModel = new UFBW_fixed(pDataContainer->getTimeline(), pDataContainer->getSimEndTime(), batchWindowLengthInSec, maxPickupDistance, minPctSavings, _allRequestsInSim, _initOpenTrips, pDataContainer->getAllDrivers(), _inclInitPickupDistForSavingsConstr);
        bool modelSolved = pFixedBatchModel->solve(pDataOutput->_printDebugFiles, pOutput, pDataInput->_populateInitOpenTrips, pDataOutput->_printToScreen);
        if( modelSolved ) {
            Solution * pFixedBatchSolution = pFixedBatchModel->getSolution();
            pOutput->printSolution(pFixedBatchSolution, UFBW_FIXED_PICKUPS);
            
            // update solution maps
            SolnMetrics * pSolnMetrics = new SolnMetrics();
            pSolnMetrics->numRequests = pFixedBatchSolution->getTotalRequests();
            pSolnMetrics->matchRate = pFixedBatchSolution->getRequestMetrics()->_matchedPercentage;
            pSolnMetrics->inconvenience = pFixedBatchSolution->getInconvenienceMetrics()->_avgPctAddedDistsForAll;
            pSolnMetrics->numTrips = pFixedBatchSolution->getTotalNumTripsFromSoln();
            
            pModelSolnMetricMap->insert(make_pair(UFBW_FIXED_PICKUPS, pSolnMetrics));
        }   
    }
    
    /*
     *     UP FRONT BATCHING WINDOW WITH PERFECT INFORMATION
     */
    if( _runUFBW_PI ) {
        UFBW_perfectInformation * pUFBW_PI_Model = new UFBW_perfectInformation(pDataContainer->getTimeline(), pDataContainer->getSimEndTime(), batchWindowLengthInSec, maxPickupDistance, minPctSavings, _allRequestsInSim, _initOpenTrips, pDataContainer->getAllDrivers(), _inclInitPickupDistForSavingsConstr);
        bool modelSolved = pUFBW_PI_Model->solve(pDataOutput->_printDebugFiles, pOutput, pDataInput->_populateInitOpenTrips, pDataOutput->_printToScreen);
        if( modelSolved ) {
            Solution * pBatchSolnWithPerfInfo = pUFBW_PI_Model->getSolution();
            pOutput->printSolution(pBatchSolnWithPerfInfo, UFBW_PERFECT_INFO);
            
            // update solution maps
            SolnMetrics * pSolnMetrics  = new SolnMetrics();
            pSolnMetrics->numRequests   = pBatchSolnWithPerfInfo->getTotalRequests();
            pSolnMetrics->matchRate     = pBatchSolnWithPerfInfo->getRequestMetrics()->_matchedPercentage;
            pSolnMetrics->inconvenience = pBatchSolnWithPerfInfo->getInconvenienceMetrics()->_avgPctAddedDistsForAll;
            pSolnMetrics->numTrips      = pBatchSolnWithPerfInfo->getTotalNumTripsFromSoln();
            
            pModelSolnMetricMap->insert(make_pair(UFBW_PERFECT_INFO, pSolnMetrics));
        }
    }
    
        
    return pModelSolnMetricMap;
}

bool ModelRunner::updateModelSolnMaps(std::map<double, SolnMaps*> * pModelSolnMap, const std::map<const ModelEnum, ModelRunner::SolnMetrics*> * pModelCurrSolnMap, double currInputValue) {
    
    // loop through all entries for the CURRENT SOLUTION and update the maps from ALL soltuions
    for( std::map<const ModelEnum, ModelRunner::SolnMetrics*>::const_iterator it = pModelCurrSolnMap->begin(); it != pModelCurrSolnMap->end(); ++it ) {        
      
        // check if the key exists
        std::map<double, SolnMaps*>::const_iterator modelAllSolnMapItr = pModelSolnMap->find(currInputValue);
        if( modelAllSolnMapItr != pModelSolnMap->end() ) {
            modelAllSolnMapItr->second->numRequests_inputs.insert(make_pair(it->first, it->second->numRequests));
            modelAllSolnMapItr->second->matchRateMap.insert(make_pair(it->first, it->second->matchRate));
            modelAllSolnMapItr->second->inconvMap.insert(make_pair(it->first, it->second->inconvenience));
            modelAllSolnMapItr->second->numTripsMap.insert(make_pair(it->first, it->second->numTrips));
        } else {
            std::vector<double> inputVals;
            inputVals.push_back(currInputValue);
            
            std::map<ModelEnum, const int> initReqMap;
            initReqMap.insert(make_pair(it->first, it->second->numRequests));
            
            std::map<ModelEnum, double> initMatchRateMap;
            initMatchRateMap.insert(make_pair(it->first, it->second->matchRate));
            
            std::map<ModelEnum, double> initInconvMap;
            initInconvMap.insert(make_pair(it->first, it->second->inconvenience));
            
            std::map<ModelEnum, double> initNumTripsMap;
            initNumTripsMap.insert(make_pair(it->first, it->second->numTrips));
            
            SolnMaps * pSolnMaps = new SolnMaps();           
            pSolnMaps->matchRateMap = initMatchRateMap;
            pSolnMaps->inconvMap = initInconvMap;
            pSolnMaps->numTripsMap = initNumTripsMap;
            pSolnMaps->numRequests_inputs = initReqMap;

            pModelSolnMap->insert(make_pair(currInputValue, pSolnMaps));
        }
    }    
    
    return true;
}

void ModelRunner::setInputValues( std::vector<double> optInVals, std::vector<double> batchWindowVals, std::vector<double> maxPickupVals, std::vector<double> minSavingsVals ) {
    _range_optInValues = optInVals;
    _range_batchWindowValues = batchWindowVals;
    _range_maxPickupDistValues = maxPickupVals;
    _range_minSavingsValues = minSavingsVals;
}

void ModelRunner::printSolutionSummaryForCurrExperiment(const std::map<double, SolnMaps*> * pInputValSolnMap) {
    pOutput->printSolutionSummaryMetricsForCurrSolutions(_experiment, pInputValSolnMap);
}