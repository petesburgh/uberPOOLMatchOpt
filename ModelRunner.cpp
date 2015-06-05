/* 
 * File:   ModelRunner.cpp
 * Author: jonpetersen
 * 
 * Created on May 1, 2015, 3:54 PM
 */

#include "ModelRunner.hpp"
#include "UFBW_perfectInformation.hpp"

ModelRunner::ModelRunner( const Experiment &experiment, UserConfig * pUserConfig, DataInputValues * dataInput, DataOutputValues * dataOutput, DefaultModelParameters * defaultValues) : 
    _experiment(experiment), 
    _runMitmModel(pUserConfig->getBooleanParams()->_runMITMModel), 
    _runUFBW_fixedPickup(pUserConfig->getBooleanParams()->_runUFBW_seqPickups), 
    _runFlexDeparture(pUserConfig->getBooleanParams()->_runFlexDepModel), 
    _runUFBW_PI(pUserConfig->getBooleanParams()->_runUFBW_perfInfo), 
    _runMultiplePickupsModel(pUserConfig->getBooleanParams()->_runMultiplePickups), 
    pDataInput(dataInput), pDataOutput(dataOutput) , pDefaultValues(defaultValues),
    _inclInitPickupDistForSavingsConstr(pUserConfig->getBooleanParams()->_inclInitPickupInSavingsConstr),
    _useAggTripSavingsForObjAndConstr(pUserConfig->getBooleanParams()->_useAggConstrAndObj) 
{          
    pGeofence = NULL; // instantiate Geofence; this may be later updated if a valid file is provided     
}

ModelRunner::~ModelRunner() {
}

void ModelRunner::extractGeofence(const std::string _geofencePath) {
    
    pGeofence = new Geofence(Geofence::REQ_ONLY);
    
    ifstream inFile;
    inFile.open(_geofencePath);
    std::string line;
    if( inFile.is_open() ) {
        while( getline(inFile,line) ) {

            std::stringstream ss(line);
            std::string token;
            std::pair<double,double> currVertex;
            int column = 0;
            while( ss >> token ) {                
                assert( column <= 1 );
                size_t prec = 10;
                double currValue = std::stod(token,&prec);
                if( column == 0 ) {
                    currVertex.first = currValue;
                } else if( column == 1 ) {
                    currVertex.second = currValue;
                }
                column++;
            }
            pGeofence->addLatLng(currVertex);
        }
        inFile.close();
    }    
}

DataContainer * ModelRunner::constructDataContainer(double optInRate, int batchWindowLengthInSec) {
    
    DataContainer * pDataContainer = NULL;
    
    /* 
     * STEP 1: read parse CSV input 
    */
//    pDataContainer = new DataContainer(pDataInput->_inputPath, pDataInput->_cvsFilename, pDataInput->_timelineStr, optInRate, pDataInput->_simLengthInMinutes, pDataOutput->_printDebugFiles, pDataOutput->_printToScreen, pGeofence);
    pDataContainer = new DataContainer(pDataInput->_inputCvsFile, pDataInput->_timelineStr, optInRate, pDataInput->_simLengthInMinutes, pDataOutput->_printDebugFiles, pDataOutput->_printToScreen, pGeofence);
    pDataContainer->setBatchWindowInSeconds(batchWindowLengthInSec);
    try {
        pDataContainer->extractCvsSnapshot();  
    } catch( FileNotFoundException &ex ) {
        std::cerr << "\n*** FileNotFoundException thrown ***" << std::endl;
        std::cerr << ex.what() << std::endl;
        std::cerr << "\t(exiting unsuccessfully) " << std::endl;
        exit(1);
    }

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
        case DEFAULTVALUES :
        {             
            const std::string outputExpPath = pDataOutput->_outputBasePath + "defaultParams/";
            pOutput = new Output(pDataOutput->_outputBasePath, outputExpPath, pDataOutput->_printIndivSolnMetrics);
            std::string outputScenarioPath = outputExpPath;
            pOutput->setOutputScenarioPath(outputScenarioPath);
            
            const std::map<const ModelEnum, ModelRunner::SolnMetrics*> * pSolnMetrics_defaultVals = runModelsForCurrExperiment(pDefaultValues->_optInRate, pDefaultValues->_batchWindowLengthInSec, pDefaultValues->_maxPickupDistance, pDefaultValues->_minSavings, _useAggTripSavingsForObjAndConstr); // second input is a dummy argument                     
            break;
        }
        case OPTIN :
        {
            for( std::vector<double>::iterator optInItr = _range_optInValues.begin(); optInItr != _range_optInValues.end(); ++optInItr ) {
                
                const std::string outputExpPath = pDataOutput->_outputBasePath + "OPTIN/";
                pOutput = new Output(pDataOutput->_outputBasePath, outputExpPath, pDataOutput->_printIndivSolnMetrics);
                const std::string outputScenarioPath = outputExpPath + "scen-" + Utility::doubleToStr(*optInItr) + "/";
                pOutput->setOutputScenarioPath(outputScenarioPath);
                Utility::createFolder(outputScenarioPath);
            
                std::map<const ModelEnum, ModelRunner::SolnMetrics*> * pSolnMapCurrOptInRate = runModelsForCurrExperiment(*optInItr, pDefaultValues->_batchWindowLengthInSec, pDefaultValues->_maxPickupDistance, pDefaultValues->_minSavings, _useAggTripSavingsForObjAndConstr );
                updateModelSolnMaps(pModelSolnMap, pSolnMapCurrOptInRate, *optInItr);
            }
            break;
        }
        case BATCHWINDOW :
        {
            for( std::vector<double>::iterator batchWindowItr = _range_batchWindowValues.begin(); batchWindowItr != _range_batchWindowValues.end(); ++batchWindowItr ) {
                
                const std::string outputExpPath = pDataOutput->_outputBasePath + "BATCHWINDOW/";
                pOutput = new Output(pDataOutput->_outputBasePath, outputExpPath, pDataOutput->_printIndivSolnMetrics);
                const std::string outputScenarioPath = outputExpPath + "scen-" + Utility::doubleToStr(*batchWindowItr) + "/";
                pOutput->setOutputScenarioPath(outputScenarioPath);      
                Utility::createFolder(outputScenarioPath);
                
                std::map<const ModelEnum, ModelRunner::SolnMetrics*> * pSolnMapBatchWindow = runModelsForCurrExperiment(pDefaultValues->_optInRate, *batchWindowItr, pDefaultValues->_maxPickupDistance, pDefaultValues->_minSavings, _useAggTripSavingsForObjAndConstr );
                updateModelSolnMaps(pModelSolnMap, pSolnMapBatchWindow, *batchWindowItr);
            }
            break;
        }
        case PICKUP :
        {
            for( std::vector<double>::iterator maxPickupItr = _range_maxPickupDistValues.begin(); maxPickupItr != _range_maxPickupDistValues.end(); ++maxPickupItr ) {
                
                const std::string outputExpPath = pDataOutput->_outputBasePath + "MAXPICKUP/";
                pOutput = new Output(pDataOutput->_outputBasePath, outputExpPath, pDataOutput->_printIndivSolnMetrics);
                const std::string outputScenarioPath = outputExpPath + "scen-" + Utility::doubleToStr(*maxPickupItr) + "/";
                pOutput->setOutputScenarioPath(outputScenarioPath);     
                Utility::createFolder(outputScenarioPath);
                
                std::map<const ModelEnum, ModelRunner::SolnMetrics*> * pSolnMapMaxPickupDist = runModelsForCurrExperiment( pDefaultValues->_optInRate, pDefaultValues->_batchWindowLengthInSec, *maxPickupItr, pDefaultValues->_minSavings, _useAggTripSavingsForObjAndConstr );
                updateModelSolnMaps(pModelSolnMap, pSolnMapMaxPickupDist, *maxPickupItr);
            }
            break;
        }        
        case SAVINGSRATE :
        {
            for( std::vector<double>::iterator minSavingsItr = _range_minSavingsValues.begin(); minSavingsItr != _range_minSavingsValues.end(); ++minSavingsItr ) {
                
                const std::string outputExpPath = pDataOutput->_outputBasePath + "MINSAVINGS/";
                pOutput = new Output(pDataOutput->_outputBasePath, outputExpPath, pDataOutput->_printIndivSolnMetrics);
                const std::string outputScenarioPath = outputExpPath + "scen-" + Utility::doubleToStr(*minSavingsItr) + "/";
                pOutput->setOutputScenarioPath(outputScenarioPath);                
                Utility::createFolder(outputScenarioPath);
                
                std::map<const ModelEnum, ModelRunner::SolnMetrics*> * pSolnMapMinSavings = runModelsForCurrExperiment( pDefaultValues->_optInRate, pDefaultValues->_batchWindowLengthInSec, pDefaultValues->_maxPickupDistance, *minSavingsItr, _useAggTripSavingsForObjAndConstr );
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

std::map<const ModelEnum, ModelRunner::SolnMetrics*> * ModelRunner::runModelsForCurrExperiment(double optInRate, double batchWindowLengthInSecDouble, double maxPickupDistance, double minPctSavings, const bool useAggTripSavingsForObjAndConstr) {
        
    std::map<const ModelEnum, ModelRunner::SolnMetrics*> * pModelSolnMetricMap = new std::map<const ModelEnum, ModelRunner::SolnMetrics*>();
    
    int batchWindowLengthInSec = (int)batchWindowLengthInSecDouble;
    DataContainer * pDataContainer = constructDataContainer(optInRate, batchWindowLengthInSec);
    
    pOutput->setDataContainer(pDataContainer);
    pOutput->writeAndPrintInputs(pDataContainer, pDataOutput->_printDebugFiles); 
               
    /*
     *    MAN IN THE MUNI (MITM) MODEL 
     */
    if( _runMitmModel ) {
        MitmModel * pMitmModel = new MitmModel(pDataContainer->getTimeline(), pDataContainer->getSimEndTime(), maxPickupDistance, minPctSavings, _allRequestsInSim, _initOpenTrips, pDataContainer->getAllDrivers(), _inclInitPickupDistForSavingsConstr, useAggTripSavingsForObjAndConstr);
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
            pSolnMetrics->avgSavingsAllMatchedRiders = pMitmSolution->getSavingsMetrics()->_avgMatchedRiderSavingsPct;
            pSolnMetrics->avgSavingsMasters = pMitmSolution->getSavingsMetrics()->_avgMasterSavingsPct;
            pSolnMetrics->avgSavingsMinions = pMitmSolution->getSavingsMetrics()->_avgMinionSavingsPct;
            pSolnMetrics->avgWaitTimeMatch_allRiders = pMitmSolution->getMatchMetrics()->_avgWaitTimeOfMatchesForAllMatchedRiders;
            pSolnMetrics->avgWaitTimeMatch_masters   = pMitmSolution->getMatchMetrics()->_avgWaitTimeOfMatchesForMasters;
            pSolnMetrics->avgWaitTimeMatch_minions   = pMitmSolution->getMatchMetrics()->_avgWaitTimeOfMatchesForMinions;
            pSolnMetrics->avgSharedDist = pMitmSolution->getOverlapMetrics()->_avgOverlapDist;
            pSolnMetrics->avgPctSharedDist_Trip = pMitmSolution->getOverlapMetrics()->_avgTripOverlapPct;
            pSolnMetrics->avgPctSharedDist_ALL = pMitmSolution->getOverlapMetrics()->_avgPctOverlapAll;
            pSolnMetrics->avgPctSharedDist_Masters = pMitmSolution->getOverlapMetrics()->_avgPctOverlapMasters;
            pSolnMetrics->avgPctSharedDist_Minions = pMitmSolution->getOverlapMetrics()->_avgPctOverlapMinions;
            
            IndivSolnMetrics * pIndivSolnMetrics = getIndivSolnMetrics(pMitmSolution);
            pSolnMetrics->pIndivMetrics = pIndivSolnMetrics;
                        
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
            pSolnMetrics->avgSavingsAllMatchedRiders = pFixedBatchSolution->getSavingsMetrics()->_avgMatchedRiderSavingsPct;
            pSolnMetrics->avgSavingsMasters = pFixedBatchSolution->getSavingsMetrics()->_avgMasterSavingsPct;
            pSolnMetrics->avgSavingsMinions = pFixedBatchSolution->getSavingsMetrics()->_avgMinionSavingsPct;
            pSolnMetrics->avgWaitTimeMatch_allRiders = pFixedBatchSolution->getMatchMetrics()->_avgWaitTimeOfMatchesForAllMatchedRiders;
            pSolnMetrics->avgWaitTimeMatch_masters   = pFixedBatchSolution->getMatchMetrics()->_avgWaitTimeOfMatchesForMasters;
            pSolnMetrics->avgWaitTimeMatch_minions   = pFixedBatchSolution->getMatchMetrics()->_avgWaitTimeOfMatchesForMinions;
            pSolnMetrics->avgSharedDist = pFixedBatchSolution->getOverlapMetrics()->_avgOverlapDist;
            pSolnMetrics->avgPctSharedDist_ALL = pFixedBatchSolution->getOverlapMetrics()->_avgPctOverlapAll;
            pSolnMetrics->avgPctSharedDist_Trip = pFixedBatchSolution->getOverlapMetrics()->_avgTripOverlapPct;
            pSolnMetrics->avgPctSharedDist_Masters = pFixedBatchSolution->getOverlapMetrics()->_avgPctOverlapMasters;
            pSolnMetrics->avgPctSharedDist_Minions = pFixedBatchSolution->getOverlapMetrics()->_avgPctOverlapMinions;            
            
            IndivSolnMetrics * pIndivSolnMetrics = getIndivSolnMetrics(pFixedBatchSolution);
            pSolnMetrics->pIndivMetrics = pIndivSolnMetrics;            
            
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
            pSolnMetrics->avgSavingsAllMatchedRiders = pBatchSolnWithPerfInfo->getSavingsMetrics()->_avgMatchedRiderSavingsPct;
            pSolnMetrics->avgSavingsMasters = pBatchSolnWithPerfInfo->getSavingsMetrics()->_avgMasterSavingsPct;
            pSolnMetrics->avgSavingsMinions = pBatchSolnWithPerfInfo->getSavingsMetrics()->_avgMinionSavingsPct;
            pSolnMetrics->avgWaitTimeMatch_allRiders = pBatchSolnWithPerfInfo->getMatchMetrics()->_avgWaitTimeOfMatchesForAllMatchedRiders;
            pSolnMetrics->avgWaitTimeMatch_masters   = pBatchSolnWithPerfInfo->getMatchMetrics()->_avgWaitTimeOfMatchesForMasters;
            pSolnMetrics->avgWaitTimeMatch_minions   = pBatchSolnWithPerfInfo->getMatchMetrics()->_avgWaitTimeOfMatchesForMinions;
            pSolnMetrics->avgSharedDist = pBatchSolnWithPerfInfo->getOverlapMetrics()->_avgOverlapDist;
            pSolnMetrics->avgPctSharedDist_ALL = pBatchSolnWithPerfInfo->getOverlapMetrics()->_avgPctOverlapAll;
            pSolnMetrics->avgPctSharedDist_Trip = pBatchSolnWithPerfInfo->getOverlapMetrics()->_avgTripOverlapPct;
            pSolnMetrics->avgPctSharedDist_Masters = pBatchSolnWithPerfInfo->getOverlapMetrics()->_avgPctOverlapMasters;
            pSolnMetrics->avgPctSharedDist_Minions = pBatchSolnWithPerfInfo->getOverlapMetrics()->_avgPctOverlapMinions;  
            
            IndivSolnMetrics * pIndivSolnMetrics = getIndivSolnMetrics(pBatchSolnWithPerfInfo);
            pSolnMetrics->pIndivMetrics = pIndivSolnMetrics;            
            
            pModelSolnMetricMap->insert(make_pair(UFBW_PERFECT_INFO, pSolnMetrics));
        }
    }
    
    /*
     *    FLEXIBLE DEPARTURES
     */
    if( _runFlexDeparture ) { 
        FlexDepartureModel * pFlexDepModel = new FlexDepartureModel(pDataContainer->getTimeline(), pDataContainer->getSimEndTime(), batchWindowLengthInSec, pDefaultValues->_flexDepWindowInSec, maxPickupDistance, minPctSavings, _allRequestsInSim, _initOpenTrips, pDataContainer->getAllDrivers(), pDefaultValues->_flexDepOptInRate, _inclInitPickupDistForSavingsConstr);
        bool modelSolved = pFlexDepModel->solve(pDataOutput->_printDebugFiles, pOutput, pDataInput->_populateInitOpenTrips, pDataOutput->_printToScreen);
        if( modelSolved ) {
            Solution * pFlexDepSoln = pFlexDepModel->getSolution();
            pOutput->printSolution(pFlexDepSoln, FLEX_DEPARTURE);
            
            // update solution maps
            SolnMetrics * pSolnMetrics  = new SolnMetrics();
            pSolnMetrics->numRequests   = pFlexDepSoln->getTotalRequests();
            pSolnMetrics->matchRate     = pFlexDepSoln->getRequestMetrics()->_matchedPercentage;
            pSolnMetrics->inconvenience = pFlexDepSoln->getInconvenienceMetrics()->_avgPctAddedDistsForAll;
            pSolnMetrics->numTrips      = pFlexDepSoln->getTotalNumTripsFromSoln();
            pSolnMetrics->avgSavingsAllMatchedRiders = pFlexDepSoln->getSavingsMetrics()->_avgMatchedRiderSavingsPct;
            pSolnMetrics->avgSavingsMasters = pFlexDepSoln->getSavingsMetrics()->_avgMasterSavingsPct;
            pSolnMetrics->avgSavingsMinions = pFlexDepSoln->getSavingsMetrics()->_avgMinionSavingsPct;
            pSolnMetrics->avgWaitTimeMatch_allRiders = pFlexDepSoln->getMatchMetrics()->_avgWaitTimeOfMatchesForAllMatchedRiders;
            pSolnMetrics->avgWaitTimeMatch_masters   = pFlexDepSoln->getMatchMetrics()->_avgWaitTimeOfMatchesForMasters;
            pSolnMetrics->avgWaitTimeMatch_minions   = pFlexDepSoln->getMatchMetrics()->_avgWaitTimeOfMatchesForMinions;
            pSolnMetrics->avgSharedDist = pFlexDepSoln->getOverlapMetrics()->_avgOverlapDist;
            pSolnMetrics->avgPctSharedDist_Trip = pFlexDepSoln->getOverlapMetrics()->_avgTripOverlapPct;
            pSolnMetrics->avgPctSharedDist_ALL = pFlexDepSoln->getOverlapMetrics()->_avgPctOverlapAll;
            pSolnMetrics->avgPctSharedDist_Masters = pFlexDepSoln->getOverlapMetrics()->_avgPctOverlapMasters;
            pSolnMetrics->avgPctSharedDist_Minions = pFlexDepSoln->getOverlapMetrics()->_avgPctOverlapMinions;  
            
            IndivSolnMetrics * pIndivSolnMetrics = getIndivSolnMetrics(pFlexDepSoln);
            pSolnMetrics->pIndivMetrics = pIndivSolnMetrics; 
            
            pModelSolnMetricMap->insert(make_pair(FLEX_DEPARTURE, pSolnMetrics));            
        }
    }
    
    /*
     *    MULTIPLE PICKUPS
     */
    if( _runMultiplePickupsModel ) {
        MultiplePickupsModel * pMultiplePickupsModel = new MultiplePickupsModel(pDataContainer->getTimeline(), pDataContainer->getSimEndTime(), maxPickupDistance, minPctSavings, _allRequestsInSim, _initOpenTrips, pDataContainer->getAllDrivers(), _inclInitPickupDistForSavingsConstr, pDefaultValues->_maxAllowablePickups);
        bool modelSolved = pMultiplePickupsModel->solve(pDataOutput->_printDebugFiles, pOutput, pDataInput->_populateInitOpenTrips);     
        if( modelSolved ) {          
            
            MultPickupSoln * pSolution = pMultiplePickupsModel->getSolution();
            pOutput->printSolution(pSolution, MULTIPLE_PICKUPS);
            
            // update solution maps
            SolnMetrics * pSolnMetrics  = new SolnMetrics();
            pSolnMetrics->numRequests   = pSolution->getTotalRequests();
            pSolnMetrics->matchRate     = pSolution->getRequestMetrics()->_matchedPercentage;
            pSolnMetrics->inconvenience = -1.0; // TODO: update
            pSolnMetrics->numTrips      = pSolution->getMatchedRoutes()->size() + pSolution->getUnmatchedRoutes()->size();
            pSolnMetrics->avgSavingsAllMatchedRiders = -1.0; //pSolution->getSavingsMetrics()->_avgMatchedRiderSavingsPct;
            pSolnMetrics->avgSavingsMasters = -1.0; //pSolution->getSavingsMetrics()->_avgMasterSavingsPct;
            pSolnMetrics->avgSavingsMinions = -1.0; //pSolution->getSavingsMetrics()->_avgMinionSavingsPct;
            pSolnMetrics->avgWaitTimeMatch_allRiders = 0.0; //pSolution->getMatchMetrics()->_avgWaitTimeOfMatchesForAllMatchedRiders;
            pSolnMetrics->avgWaitTimeMatch_masters   = 0.0; //pSolution->getMatchMetrics()->_avgWaitTimeOfMatchesForMasters;
            pSolnMetrics->avgWaitTimeMatch_minions   = 0.0; //pSolution->getMatchMetrics()->_avgWaitTimeOfMatchesForMinions;
            pSolnMetrics->avgSharedDist = -1.0; // pSolution->getOverlapMetrics()->_avgOverlapDist;
            pSolnMetrics->avgPctSharedDist_Trip = -1.0; // pSolution->getOverlapMetrics()->_avgTripOverlapPct;
            pSolnMetrics->avgPctSharedDist_ALL = -1.0; // pSolution->getOverlapMetrics()->_avgPctOverlapAll;
            pSolnMetrics->avgPctSharedDist_Masters = -1.0; // pSolution->getOverlapMetrics()->_avgPctOverlapMasters;
            pSolnMetrics->avgPctSharedDist_Minions = -1.0; // pSolution->getOverlapMetrics()->_avgPctOverlapMinions;  

            IndivSolnMetrics * pIndivSolnMetrics = getIndivSolnMetrics(pSolution);
            pSolnMetrics->pIndivMetrics = pIndivSolnMetrics; 
            
            pModelSolnMetricMap->insert(make_pair(MULTIPLE_PICKUPS, pSolnMetrics));             
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
            modelAllSolnMapItr->second->numTripsMap.insert(make_pair(it->first, it->second->numTrips));
            modelAllSolnMapItr->second->avgSavingsAllMatchedRiders.insert(make_pair(it->first, it->second->avgSavingsAllMatchedRiders));
            modelAllSolnMapItr->second->avgSavingsMasters.insert(make_pair(it->first, it->second->avgSavingsMasters));
            modelAllSolnMapItr->second->avgSavingsMinions.insert(make_pair(it->first, it->second->avgSavingsMinions));
            modelAllSolnMapItr->second->avgWaitTimeOfMatchAllRiders.insert(make_pair(it->first, it->second->avgWaitTimeMatch_allRiders));
            modelAllSolnMapItr->second->avgWaitTimeOfMatchMasters.insert(make_pair(it->first, it->second->avgWaitTimeMatch_masters));
            modelAllSolnMapItr->second->avgWaitTimeOfMatchMinions.insert(make_pair(it->first, it->second->avgWaitTimeMatch_minions));
            modelAllSolnMapItr->second->avgOverlapDist.insert(make_pair(it->first, it->second->avgSharedDist));
            modelAllSolnMapItr->second->avgPctOverlap_Trip.insert(make_pair(it->first, it->second->avgPctSharedDist_Trip));
            modelAllSolnMapItr->second->avgPctOverlap_ALL.insert(make_pair(it->first, it->second->avgPctSharedDist_ALL));
            modelAllSolnMapItr->second->avgPctOverlap_Masters.insert(make_pair(it->first, it->second->avgPctSharedDist_Masters));
            modelAllSolnMapItr->second->avgPctOverlap_Minions.insert(make_pair(it->first, it->second->avgPctSharedDist_Minions));
           
            // individual metrics
            modelAllSolnMapItr->second->indivPctInconvenience_ALL.insert(make_pair(it->first, it->second->pIndivMetrics->_inconv_ALL));
            modelAllSolnMapItr->second->indivPctInconvenience_Masters.insert(make_pair(it->first, it->second->pIndivMetrics->_inconv_Masters));
            modelAllSolnMapItr->second->indivPctInconvenience_Minions.insert(make_pair(it->first, it->second->pIndivMetrics->_inconv_Minions));
            
            modelAllSolnMapItr->second->indivMatchOverlapDistances.insert(make_pair(it->first, it->second->pIndivMetrics->_overlapDist));
            modelAllSolnMapItr->second->indivPctOverlap_Trip.insert(make_pair(it->first, it->second->pIndivMetrics->_overlapPct_Trip));
            modelAllSolnMapItr->second->indivPctTripOverlap_ALL.insert(make_pair(it->first, it->second->pIndivMetrics->_overlapPct_ALL));
            modelAllSolnMapItr->second->indivPctTripOverlap_Masters.insert(make_pair(it->first, it->second->pIndivMetrics->_overlapPct_Masters));
            modelAllSolnMapItr->second->indivPctTripOverlap_Minions.insert(make_pair(it->first, it->second->pIndivMetrics->_overlapPct_Minions));

            modelAllSolnMapItr->second->indivPctSavings_ALL.insert(make_pair(it->first, it->second->pIndivMetrics->_savings_ALL));
            modelAllSolnMapItr->second->indivPctSavings_Masters.insert(make_pair(it->first, it->second->pIndivMetrics->_savings_Masters));
            modelAllSolnMapItr->second->indivPctSavings_Minions.insert(make_pair(it->first, it->second->pIndivMetrics->_savings_Minions));
            
            modelAllSolnMapItr->second->indivWaitTimeToMatch_all.insert(make_pair(it->first, it->second->pIndivMetrics->_waitTimeToMatch_ALL));
            modelAllSolnMapItr->second->indivWaitTimeToMatch_Masters.insert(make_pair(it->first, it->second->pIndivMetrics->_waitTimeToMatch_Masters));
            modelAllSolnMapItr->second->indivWaitTimeToMatch_Minions.insert(make_pair(it->first, it->second->pIndivMetrics->_waitTimeToMatch_Minions));

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
            
            std::map<ModelEnum, double> initSavingsAllRidersMap;
            initSavingsAllRidersMap.insert(make_pair(it->first, it->second->avgSavingsAllMatchedRiders));
            
            std::map<ModelEnum, double> initSavingsMastersMap;
            initSavingsMastersMap.insert(make_pair(it->first, it->second->avgSavingsMasters));
            
            std::map<ModelEnum, double> initSavingsMinionsMap;
            initSavingsMinionsMap.insert(make_pair(it->first, it->second->avgSavingsMinions));
            
            std::map<ModelEnum, double> initSharedDistMap;
            initSharedDistMap.insert(make_pair(it->first, it->second->avgSharedDist));
            
            std::map<ModelEnum, double> initPctTripOverlapMap;
            initPctTripOverlapMap.insert(make_pair(it->first, it->second->avgPctSharedDist_Trip));
            
            std::map<ModelEnum, double> initPctSharedDistMap_ALL;  
            initPctSharedDistMap_ALL.insert(make_pair(it->first, it->second->avgPctSharedDist_ALL));
            
            std::map<ModelEnum, double> initPctSharedDistMap_Masters;  
            initPctSharedDistMap_Masters.insert(make_pair(it->first, it->second->avgPctSharedDist_Masters));
            
            std::map<ModelEnum, double> initPctSharedDistMap_Minions;  
            initPctSharedDistMap_Minions.insert(make_pair(it->first, it->second->avgPctSharedDist_Minions));
            
            std::map<ModelEnum, double> initWaitTImeMatchAllRidersMap;
            initWaitTImeMatchAllRidersMap.insert(make_pair(it->first, it->second->avgWaitTimeMatch_allRiders));
            
            std::map<ModelEnum, double> initWaitTimeMatchMastersMap;
            initWaitTimeMatchMastersMap.insert(make_pair(it->first, it->second->avgWaitTimeMatch_masters));
            
            std::map<ModelEnum, double> initWaitTimeMatchMinionsMap;
            initWaitTimeMatchMinionsMap.insert(make_pair(it->first, it->second->avgWaitTimeMatch_minions));
                        
            // mapping between models and individual metrics
            std::map<ModelEnum, std::vector<double> > initIndivInconvMap_ALL;
            initIndivInconvMap_ALL.insert(make_pair(it->first, it->second->pIndivMetrics->_inconv_ALL));
            
            std::map<ModelEnum, std::vector<double> > initIndivInconvMap_Masters; 
            initIndivInconvMap_Masters.insert(make_pair(it->first, it->second->pIndivMetrics->_inconv_Masters));
                        
            std::map<ModelEnum, std::vector<double> > initIndivInconvMap_Minions;
            initIndivInconvMap_Minions.insert(make_pair(it->first, it->second->pIndivMetrics->_inconv_Minions));
                        
            std::map<ModelEnum, std::vector<double> > initIndivOverlapDist;
            initIndivOverlapDist.insert(make_pair(it->first, it->second->pIndivMetrics->_overlapDist));
            
            std::map<ModelEnum, std::vector<double> > initIndivPctTripOverlap;
            initIndivPctTripOverlap.insert(make_pair(it->first, it->second->pIndivMetrics->_overlapPct_Trip));
            
            std::map<ModelEnum, std::vector<double> > initIndivPctOverlap_ALL;
            initIndivPctOverlap_ALL.insert(make_pair(it->first, it->second->pIndivMetrics->_overlapPct_ALL));
            
            std::map<ModelEnum, std::vector<double> > initIndivPctOverlap_Masters;
            initIndivPctOverlap_Masters.insert(make_pair(it->first, it->second->pIndivMetrics->_overlapPct_Masters));
            
            std::map<ModelEnum, std::vector<double> > initIndivPctOverlap_Minions;
            initIndivPctOverlap_Minions.insert(make_pair(it->first, it->second->pIndivMetrics->_overlapPct_Minions));
            
            std::map<ModelEnum, std::vector<double> > initIndivPctSavings_ALL;
            initIndivPctSavings_ALL.insert(make_pair(it->first, it->second->pIndivMetrics->_savings_ALL));
            
            std::map<ModelEnum, std::vector<double> > initIndivPctSavings_Masters;
            initIndivPctSavings_Masters.insert(make_pair(it->first, it->second->pIndivMetrics->_savings_Masters));                        
            
            std::map<ModelEnum, std::vector<double> > initIndivPctSavings_Minions;
            initIndivPctSavings_Minions.insert(make_pair(it->first, it->second->pIndivMetrics->_savings_Minions));
            
            std::map<ModelEnum, std::vector<int> > initIndivWaitTimeMatch_all;
            initIndivWaitTimeMatch_all.insert(make_pair(it->first, it->second->pIndivMetrics->_waitTimeToMatch_ALL));
            
            std::map<ModelEnum, std::vector<int> > initIndivWaitTimeMatch_Masters;
            initIndivWaitTimeMatch_Masters.insert(make_pair(it->first, it->second->pIndivMetrics->_waitTimeToMatch_Masters));
            
            std::map<ModelEnum, std::vector<int> > initIndivWaitTimeMatch_Minions;
            initIndivWaitTimeMatch_Minions.insert(make_pair(it->first, it->second->pIndivMetrics->_waitTimeToMatch_Minions));
            
            SolnMaps * pSolnMaps = new SolnMaps();           
            pSolnMaps->matchRateMap = initMatchRateMap;
            pSolnMaps->inconvMap = initInconvMap;
            pSolnMaps->numTripsMap = initNumTripsMap;
            pSolnMaps->numRequests_inputs = initReqMap;
            pSolnMaps->avgSavingsAllMatchedRiders = initSavingsAllRidersMap;
            pSolnMaps->avgSavingsMasters = initSavingsMastersMap;
            pSolnMaps->avgSavingsMinions = initSavingsMinionsMap;
            pSolnMaps->avgWaitTimeOfMatchAllRiders = initWaitTImeMatchAllRidersMap;
            pSolnMaps->avgWaitTimeOfMatchMasters = initWaitTimeMatchMastersMap;
            pSolnMaps->avgWaitTimeOfMatchMinions = initWaitTimeMatchMinionsMap;
            pSolnMaps->avgOverlapDist = initSharedDistMap;
            pSolnMaps->avgPctOverlap_Trip = initPctTripOverlapMap;
            pSolnMaps->avgPctOverlap_ALL = initPctSharedDistMap_ALL;
            pSolnMaps->avgPctOverlap_Masters = initPctSharedDistMap_Masters;
            pSolnMaps->avgPctOverlap_Minions = initPctSharedDistMap_Minions;
            
            // individual metrics
            pSolnMaps->indivPctInconvenience_ALL     = initIndivInconvMap_ALL;
            pSolnMaps->indivPctInconvenience_Masters = initIndivInconvMap_Masters;
            pSolnMaps->indivPctInconvenience_Minions = initIndivInconvMap_Minions;
            pSolnMaps->indivMatchOverlapDistances    = initIndivOverlapDist;
            pSolnMaps->indivPctTripOverlap_ALL       = initIndivPctOverlap_ALL;
            pSolnMaps->indivPctOverlap_Trip          = initIndivPctTripOverlap;
            pSolnMaps->indivPctTripOverlap_Masters   = initIndivPctOverlap_Masters;
            pSolnMaps->indivPctTripOverlap_Minions   = initIndivPctOverlap_Minions;
            pSolnMaps->indivPctSavings_ALL           = initIndivPctSavings_ALL;
            pSolnMaps->indivPctSavings_Masters       = initIndivPctSavings_Masters;
            pSolnMaps->indivPctSavings_Minions       = initIndivPctSavings_Minions;
            pSolnMaps->indivWaitTimeToMatch_all      = initIndivWaitTimeMatch_all;
            pSolnMaps->indivWaitTimeToMatch_Masters  = initIndivWaitTimeMatch_Masters;
            pSolnMaps->indivWaitTimeToMatch_Minions  = initIndivWaitTimeMatch_Minions;
                        
            pModelSolnMap->insert(make_pair(currInputValue, pSolnMaps));
        }
    }    
    
    return true;
}

ModelRunner::IndivSolnMetrics * ModelRunner::getIndivSolnMetrics(Solution * pSoln) {    
    IndivSolnMetrics * pIndivSolnMetrics = new IndivSolnMetrics();
    
    pIndivSolnMetrics->_inconv_ALL         = pSoln->getIndivMatchedRidersMetrics()->_inconv_ALL;
    pIndivSolnMetrics->_inconv_Masters     = pSoln->getIndivMatchedRidersMetrics()->_inconv_Masters;
    pIndivSolnMetrics->_inconv_Minions     = pSoln->getIndivMatchedRidersMetrics()->_inconv_Minions;
    
    pIndivSolnMetrics->_overlapDist        = pSoln->getIndivMatchedRidersMetrics()->_overlapDist;
    pIndivSolnMetrics->_overlapPct_Trip    = pSoln->getIndivMatchedRidersMetrics()->_overlapPct_Trip;
    pIndivSolnMetrics->_overlapPct_ALL     = pSoln->getIndivMatchedRidersMetrics()->_overlapPct_ALL;
    pIndivSolnMetrics->_overlapPct_Masters = pSoln->getIndivMatchedRidersMetrics()->_overlapPct_Masters;
    pIndivSolnMetrics->_overlapPct_Minions = pSoln->getIndivMatchedRidersMetrics()->_overlapPct_Minions;
    
    pIndivSolnMetrics->_savings_ALL        = pSoln->getIndivMatchedRidersMetrics()->_savings_ALL;
    pIndivSolnMetrics->_savings_Masters    = pSoln->getIndivMatchedRidersMetrics()->_savings_Masters;
    pIndivSolnMetrics->_savings_Minions    = pSoln->getIndivMatchedRidersMetrics()->_savings_Minions;
    
    pIndivSolnMetrics->_waitTimeToMatch_ALL = pSoln->getIndivMatchedRidersMetrics()->_waitTimeToMatch_ALL;
    pIndivSolnMetrics->_waitTimeToMatch_Masters = pSoln->getIndivMatchedRidersMetrics()->_waitTimeToMatch_Masters;
    pIndivSolnMetrics->_waitTimeToMatch_Minions = pSoln->getIndivMatchedRidersMetrics()->_waitTimeToMatch_Minions;
    
    return pIndivSolnMetrics;    
}

void ModelRunner::setInputValues(UserConfig* pUserConfig) {
    _range_optInValues = pUserConfig->getRangeParams()->optInRange;
    _range_batchWindowValues = pUserConfig->getRangeParams()->batchWindowRange;
    _range_maxPickupDistValues = pUserConfig->getRangeParams()->maxPickupRange;
    _range_minSavingsValues = pUserConfig->getRangeParams()->minSavingsRange;    
}

void ModelRunner::printSolutionSummaryForCurrExperiment(const std::map<double, SolnMaps*> * pInputValSolnMap) {
    pOutput->printSolutionSummaryMetricsForCurrSolutions(_experiment, pInputValSolnMap);
}