/* 
 * File:   UserConfig.hpp
 * Author: jonpetersen
 *
 * Created on June 1, 2015, 7:37 PM
 */

#ifndef USERCONFIG_HPP
#define	USERCONFIG_HPP

#include "Utility.hpp" /* converting string to int, double, bool */
#include "FileNotFoundException.hpp" /* custom exception */
#include "Experiment.hpp" /* enum for experiment */
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <map>
#include <set>
using namespace std;

class UserConfig {
public:

    struct BoolParams {
        bool _runMITMModel;
        bool _runUFBW_seqPickups;
        bool _runFlexDepModel;
        bool _runUFBW_perfInfo;
        bool _runMultiplePickups;
        bool _inclInitPickupInSavingsConstr;
        bool _printDebugFiles;
        bool _printToScreen;
        bool _printIndivSolnMetrics;
        bool _populateInitOpenTrips;        
    };
    
    struct IntParams {
        int _scenNumber; 
        int _maxAllowablePickups;
        int _default_upFrontBatchWindowInSec;
        int _flexDepWindowInSec;        
    };
    
    struct DoubleParams {
        double _defaultOptInRate;
        double _default_maxMatchDistInKm;
        double _default_minPoolDiscount;
        double _flexDepOptInRate;
    };
    
    struct StringParams {
        std::string _experiment;
        std::string _inputPath;
        std::string _outputBasePath;
        std::string _geofenceDataPath;        
    };
    
    struct EnumParams {
        Experiment _experiment;
    };
    
    struct RangeParams {
        std::vector<double> optInRange;
        std::vector<double> batchWindowRange;
        std::vector<double> maxPickupRange;
        std::vector<double> minSavingsRange;
    };
    
    UserConfig();
    virtual ~UserConfig();
    
    void readConfigValues(const std::string inputFile);
    void constructKeyValueMapFromInputFile(const std::string inputFile);
    void defineConfigValuesFromKeyValueMap();
    
    // debugging
    void printKeyValueMap();
    void printMetricsToScreen();
    
    // convert string to Enum values
    Experiment convertExperimentToExpEnum(std::string experimentStr);
    
    // setters
    void setBoolParams(BoolParams * boolParams) { pBoolParams = boolParams; }
    void setIntParams(IntParams * intParams) { pIntParams = intParams; }
    void setDoubleParams(DoubleParams * doubleParams) { pDoubleParams = doubleParams; }
    void setStringParams(StringParams * strParams) { pStringParams = strParams; }
    
    // getters each parameter group
    const UserConfig::IntParams * getIntParams() const { return pIntParams; }
    const UserConfig::DoubleParams * getDoubleParams() const { return pDoubleParams; }
    const UserConfig::BoolParams * getBooleanParams() const { return pBoolParams; }
    const UserConfig::StringParams * getStringParams() const { return pStringParams; }
    const UserConfig::EnumParams * getEnumParams() const { return pEnumParams; }
    const UserConfig::RangeParams * getRangeParams() const { return pRangeParams; }
    
    std::vector<double> parseCsvRange_double(std::string str);
    
private:
    std::map<std::string,std::string> _keyValueMap;
        
    UserConfig::BoolParams   * pBoolParams;
    UserConfig::IntParams    * pIntParams;
    UserConfig::DoubleParams * pDoubleParams;
    UserConfig::StringParams * pStringParams;
    UserConfig::EnumParams   * pEnumParams;
    UserConfig::RangeParams  * pRangeParams;
    
    // key values from input
    std::string input_int_scenNumber;    
    std::string input_int_maxAllowablePickups;
    std::string input_int_defaultUpFrontBatchWindowInSec;
    std::string input_int_defaultFlexDepWindowInSec; 
    
    std::string input_str_experiment;
    std::string input_str_inputPath;
    std::string input_str_outputBasePath;
    std::string input_str_geofenceDataPath;
    
    std::string input_bool_runMITM;
    std::string input_bool_runUFBWSeqPickups;
    std::string input_bool_runFlexDep;
    std::string input_bool_runPerfInfo;
    std::string input_bool_multPickups;
    std::string input_bool_inclInitPickupInSavingsConstr;
    std::string input_bool_printDebugFiles;
    std::string input_bool_printToScreen;
    std::string input_bool_printIndivSolnMetrics;
    std::string input_bool_populateInitOpenTrips;   
    
    std::string input_double_defaultOptInRate;
    std::string input_double_defaultMaxMatchDistInKm;
    std::string input_double_defaultMinPoolDiscount;
    std::string input_double_flexDepOptInRate;
    
    std::string input_range_optInExp;
    std::string input_range_batchWindowExp;
    std::string input_range_maxPickupExp;
    std::string input_range_minSavingsExp;
    
    std::set<std::string> eligExperimentInputStrings;
    
};

#endif	/* USERCONFIG_HPP */

