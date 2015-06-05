/* 
 * File:   ModelRunner.hpp
 * Author: jonpetersen
 *
 * Created on May 1, 2015, 3:54 PM
 */

#ifndef MODELRUNNER_HPP
#define	MODELRUNNER_HPP

#include <iostream>
#include "DataContainer.hpp"
#include "Output.hpp"
#include "MitmModel.hpp"
#include "ModelEnum.hpp"
#include "FlexDepartureModel.hpp"
#include "SolnMaps.hpp"
#include "MultiplePickupsModel.hpp"
#include "Experiment.hpp"
#include "FileNotFoundException.hpp"
#include "UserConfig.hpp"
#include <sys/stat.h>   /* mkdir */

using namespace std;

class Output;

class ModelRunner {
public:
             
    struct DataInputValues {
        DataInputValues(const UserConfig * pUserConfig) :
            _inputCvsFile(pUserConfig->getStringParams()->_inputData), _timelineStr(pUserConfig->getStringParams()->_simStartTime), _simLengthInMinutes(pUserConfig->getIntParams()->_simLengthInMin), _populateInitOpenTrips(pUserConfig->getBooleanParams()->_populateInitOpenTrips) {};
        
        const std::string   _inputCvsFile;
        const std::string   _timelineStr;
        const int           _simLengthInMinutes;
        const bool          _populateInitOpenTrips;        
    };
    
    struct DefaultModelParameters { 
        DefaultModelParameters(const UserConfig * pUserConfig) : 
            _optInRate(pUserConfig->getDoubleParams()->_defaultOptInRate), 
            _batchWindowLengthInSec(pUserConfig->getIntParams()->_default_upFrontBatchWindowInSec), 
            _maxPickupDistance(pUserConfig->getDoubleParams()->_default_maxMatchDistInKm), 
            _minSavings(pUserConfig->getDoubleParams()->_default_minPoolDiscount), 
            _flexDepOptInRate(pUserConfig->getDoubleParams()->_flexDepOptInRate), 
            _flexDepWindowInSec(pUserConfig->getIntParams()->_flexDepWindowInSec), 
            _maxAllowablePickups(pUserConfig->getIntParams()->_maxAllowablePickups) {};
       
        const double _optInRate;
        const int    _batchWindowLengthInSec;
        const double _maxPickupDistance;
        const double _minSavings;
        const double _flexDepOptInRate;
        const int    _flexDepWindowInSec;       
        const int    _maxAllowablePickups;
    };    
    
    struct DataOutputValues {
        DataOutputValues(const UserConfig * pUserConfig) : 
            _outputBasePath(pUserConfig->getStringParams()->_outputBasePath), 
            _printDebugFiles(pUserConfig->getBooleanParams()->_printDebugFiles), 
            _printIndivSolnMetrics(pUserConfig->getBooleanParams()->_printIndivSolnMetrics), 
            _printToScreen(pUserConfig->getBooleanParams()->_printToScreen) {};
    
        const std::string _outputBasePath;
        std::string _outputScenarioPath;
        const bool _printDebugFiles;
        const bool _printIndivSolnMetrics;
        const bool _printToScreen;    
    };
    
    struct IndivSolnMetrics {
        std::vector<double> _inconv_ALL;
        std::vector<double> _inconv_Masters;
        std::vector<double> _inconv_Minions;
        
        std::vector<double> _overlapDist;
        std::vector<double> _overlapPct_Trip;
        std::vector<double> _overlapPct_ALL;
        std::vector<double> _overlapPct_Masters;
        std::vector<double> _overlapPct_Minions;
        
        std::vector<double> _savings_ALL;
        std::vector<double> _savings_Masters;
        std::vector<double> _savings_Minions;
        
        std::vector<int>    _waitTimeToMatch_ALL;
        std::vector<int>    _waitTimeToMatch_Masters;
        std::vector<int>    _waitTimeToMatch_Minions;       
    };    
    
    struct SolnMetrics {        
        double currInputValue;
        int numRequests;
        double matchRate;
        double inconvenience;
        int numTrips;
        double avgSavingsAllMatchedRiders;
        double avgSavingsMasters;
        double avgSavingsMinions;
        double matchRate_FD_FDOptIns;
        double matchRate_FD_nonFDOptIns;
        double avgWaitTimeMatch_allRiders;
        double avgWaitTimeMatch_masters;
        double avgWaitTimeMatch_minions;
        double avgSharedDist;
        double avgPctSharedDist_Trip;
        double avgPctSharedDist_ALL;
        double avgPctSharedDist_Masters;
        double avgPctSharedDist_Minions;
        
        ModelRunner::IndivSolnMetrics * pIndivMetrics;
    };
    
     
    ModelRunner(const Experiment &experiment, UserConfig * pUserConfig, DataInputValues * pDataInput, DataOutputValues * dataOutput, DefaultModelParameters *  defaultValues );
    virtual ~ModelRunner();
    
    // construct DataContainer object
    DataContainer * constructDataContainer(double optInRate, int batchWindowLengthInSec);
    
    // controller that invokes all individual models
    std::map<double, SolnMaps*> * runAllModels();
    
    // individual model runs
    std::map<const ModelEnum, SolnMetrics*> * runModelsForCurrExperiment(double optInRate, double batchWindowLengthInSecDouble, double maxPickupDistance, double minPctSavings, const bool useAggTripSavingsForObjAndConstr);
    
    // set input values 
    void setInputValues( UserConfig * pUserConfig );  
    
    // update solution to be returned
    bool updateModelSolnMaps(std::map<double, SolnMaps*> * pModelSolnMap, const std::map<const ModelEnum, ModelRunner::SolnMetrics*> * pModelCurrSolnMap, double currInputValue);
    
    void printSolutionSummaryForCurrExperiment(const std::map<double, SolnMaps*> * pInputValSolnMap);
    
    //void setInclInitPickupDistForSavingsConstr(bool includeInitTripSavingsConstr) { _inclInitPickupDistForSavingsConstr = includeInitTripSavingsConstr; }
    
    IndivSolnMetrics * getIndivSolnMetrics(Solution * pSoln);
    
    void extractGeofence(const std::string _geofencePath);
    
private:

    const Experiment _experiment;
    const DataInputValues *  pDataInput;
    const DefaultModelParameters * pDefaultValues;
    DataOutputValues * pDataOutput;
    
    const bool _runMitmModel;
    const bool _runUFBW_fixedPickup;
  //  const bool _runUFBW_pickupSwap;
    const bool _runFlexDeparture;
    const bool _runUFBW_PI; 
    const bool _runMultiplePickupsModel;
    
    Geofence * pGeofence;
    //const Geofence* pGeofence;
    
    Output * pOutput;
    
    // input ranges
    std::vector<double> _range_optInValues;
    std::vector<double> _range_batchWindowValues;
    std::vector<double> _range_maxPickupDistValues;
    std::vector<double> _range_minSavingsValues;
    
    // extracted from DataContainer
    std::set<Request*,  ReqComp> _initRequests;
    std::set<Request*,  ReqComp> _allRequestsInSim;
    std::set<OpenTrip*, EtdComp> _initOpenTrips;
    
    // switches
    const bool _inclInitPickupDistForSavingsConstr;
    const bool _useAggTripSavingsForObjAndConstr;
};

#endif	/* MODELRUNNER_HPP */

