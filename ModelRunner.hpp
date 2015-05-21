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
#include <sys/stat.h>   /* mkdir */

using namespace std;

class Output;

class ModelRunner {
public:
    
    enum Experiment{
        DEFAULTVALUES,
        OPTIN,
        BATCHWINDOW,
        PICKUP,
        SAVINGSRATE
     };
     
    struct DataInputValues {    
        DataInputValues(const std::string input, const std::string inputBase, const std::string cvsFname, const std::string timeline, const int simLengthMin, const bool popInitCandidates) :
            _inputPath(input), _inputBasePath(inputBase), _cvsFilename(cvsFname), _timelineStr(timeline), _simLengthInMinutes(simLengthMin),  _populateInitOpenTrips(popInitCandidates) {};

        const std::string   _inputPath;
        const std::string   _inputBasePath;
        const std::string   _cvsFilename;
        const std::string   _timelineStr;
        const int           _simLengthInMinutes;
        const bool          _populateInitOpenTrips;
    };
    
    struct DefaultModelParameters { 
        DefaultModelParameters(const double optIn, const int batchWindow, const double maxPickupDist, const double minSavings, const double flexDepOptInRate, const int flexDepWindowSec, const int maxAllowablePickups) : 
            _optInRate(optIn), _batchWindowLengthInSec(batchWindow), _maxPickupDistance(maxPickupDist), _minSavings(minSavings), _flexDepOptInRate(flexDepOptInRate), _flexDepWindowInSec(flexDepWindowSec), _maxAllowablePickups(maxAllowablePickups) {};

        const double _optInRate;
        const int    _batchWindowLengthInSec;
        const double _maxPickupDistance;
        const double _minSavings;
        const double _flexDepOptInRate;
        const int    _flexDepWindowInSec;       
        const int    _maxAllowablePickups;
    };    
    
    struct DataOutputValues {
        DataOutputValues(const std::string outputBasePath, const bool printDebugFiles, const bool printIndivSolnMetrics, const bool printToScreen) : 
            _outputBasePath(outputBasePath), _printDebugFiles(printDebugFiles), _printIndivSolnMetrics(printIndivSolnMetrics), _printToScreen(printToScreen) {};
    
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
    
     
    ModelRunner(const ModelRunner::Experiment &experiment, const bool &runMITMModel, const bool &runUFBW_seqPickups, const bool &runFlexDepModel, const bool &runUFBW_perfectInfo, const bool &runMultiplePickupsModel, DataInputValues * pDataInput, DataOutputValues * dataOutput, DefaultModelParameters *  defaultValues, const Geofence * geofence );
    virtual ~ModelRunner();
    
    // construct DataContainer object
    DataContainer * constructDataContainer(double optInRate, int batchWindowLengthInSec);
    
    // controller that invokes all individual models
    std::map<double, SolnMaps*> * runAllModels();
    
    // individual model runs
    std::map<const ModelEnum, SolnMetrics*> * runModelsForCurrExperiment(double optInRate, double batchWindowLengthInSecDouble, double maxPickupDistance, double minPctSavings);
    
    // set input values 
    void setInputValues( std::vector<double> optInVals, std::vector<double> batchWindowVals, std::vector<double> maxPickupVals, std::vector<double> minSavingsVals );
        
    // update solution to be returned
    bool updateModelSolnMaps(std::map<double, SolnMaps*> * pModelSolnMap, const std::map<const ModelEnum, ModelRunner::SolnMetrics*> * pModelCurrSolnMap, double currInputValue);
    
    void printSolutionSummaryForCurrExperiment(const std::map<double, SolnMaps*> * pInputValSolnMap);
    
    void setInclInitPickupDistForSavingsConstr(bool includeInitTripSavingsConstr) { _inclInitPickupDistForSavingsConstr = includeInitTripSavingsConstr; }
    
    IndivSolnMetrics * getIndivSolnMetrics(Solution * pSoln);
    
private:

    const Experiment _experiment;
    const DataInputValues *  pDataInput;
    const DefaultModelParameters * pDefaultValues;
    DataOutputValues * pDataOutput;
    
    const bool _runMitmModel;
    const bool _runUFBW_fixedPickup;
    const bool _runUFBW_pickupSwap;
    const bool _runFlexDeparture;
    const bool _runUFBW_PI; 
    const bool _runMultiplePickupsModel;
    
    const Geofence* pGeofence;
    
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
    bool _inclInitPickupDistForSavingsConstr;
};

#endif	/* MODELRUNNER_HPP */

