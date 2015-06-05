/* 
 * File:   UserConfig.cpp
 * Author: jonpetersen
 * 
 * Created on June 1, 2015, 7:37 PM
 */

#include "UserConfig.hpp"


UserConfig::UserConfig() {
    // populate key values from input file
    input_int_maxAllowablePickups               = "maxAllowablePickups";
    input_int_defaultUpFrontBatchWindowInSec    = "default_upFrontBatchWindowInSec";
    input_int_defaultFlexDepWindowInSec         = "default_flexDepWindowInSec";
    input_int_simLenInMin                       = "simLengthInMin";
    
    input_str_experiment        = "experiment";
    input_str_outputBasePath    = "outputBasePath";
    input_str_inputCSV          = "inputCSVFile";
    input_str_geofenceData      = "geofenceFile";
    input_str_simStartTime      = "simStartTime";
    input_str_geofenceType      = "geofenceType";
    
    input_bool_runMITM                       = "runMITMModel";
    input_bool_runUFBWSeqPickups             = "runUFBW_seqPickups";
    input_bool_runFlexDep                    = "runFlexDepModel";
    input_bool_runPerfInfo                   = "runUFBW_perfectInfo";
    input_bool_multPickups                   = "runMultiplePickups";
    input_bool_inclInitPickupInSavingsConstr = "inclInitPickupInSavingsConstr";
    input_bool_printDebugFiles               = "printDebugFiles";
    input_bool_printToScreen                 = "printToScreen";
    input_bool_printIndivSolnMetrics         = "printIndivSolnMetrics";
    input_bool_populateInitOpenTrips         = "populateInitOpenTrips";  
    input_bool_useAggTripInObjAndConstr      = "useAggTripObjConstr";
    
    input_double_defaultOptInRate        = "default_optInRate";    
    input_double_defaultMaxMatchDistInKm = "default_maxMatchDistInKm";
    input_double_defaultMinPoolDiscount  = "default_minPoolDiscount";
    input_double_flexDepOptInRate        = "flexDepOptInRate";
    
    input_range_optInExp        = "range_optInExp";
    input_range_batchWindowExp  = "range_batchWindowExp";
    input_range_maxPickupExp    = "range_maxPickupExp";
    input_range_minSavingsExp   = "range_minSavingsExp";
    
    // eligible string values for experiment
    eligExperimentInputStrings.insert("DEFAULTVALUES");
    eligExperimentInputStrings.insert("OPTIN");
    eligExperimentInputStrings.insert("BATCHWINDOW");
    eligExperimentInputStrings.insert("PICKUPDIST");
    eligExperimentInputStrings.insert("SAVINGSRATE");
    
    // eligible string values for Geofence restriction type
    eligGeofenceRestrTypeStrings.insert("REQ_ONLY");
    eligGeofenceRestrTypeStrings.insert("ORIG_ONLY");
    eligGeofenceRestrTypeStrings.insert("ENTIRE_TRIP");
    
    
    // instantiate booleans, int, double, string parameters
    pBoolParams   = new UserConfig::BoolParams();
    pIntParams    = new UserConfig::IntParams();
    pDoubleParams = new UserConfig::DoubleParams();
    pStringParams = new UserConfig::StringParams();
    pEnumParams   = new UserConfig::EnumParams();
    pRangeParams  = new UserConfig::RangeParams();
}

UserConfig::~UserConfig() {
}

void UserConfig::readConfigValues(const std::string inputFile) {    
    constructKeyValueMapFromInputFile(inputFile);
}
void UserConfig::constructKeyValueMapFromInputFile(const std::string inputFile) {
    std::ifstream file;
    file.open(inputFile);
    std::string   line;
    
    // ensure file is open
    if( file.is_open() == false ) {
        throw new FileNotFoundException(inputFile);
    }

    while( std::getline(file, line) )
    {
      std::istringstream is_line(line);
      std::string key;
      if( std::getline(is_line, key, ':') )
      {
        std::string value;
        if( std::getline(is_line, value, '#') )  {
            Utility::removeWhitespaceString(key);   // remove all whitespace contained in key string
            Utility::removeWhitespaceString(value);
            if( strcmp(key.c_str(), input_str_simStartTime.c_str()) == 0 ) { 
                value.insert(10," "); // add one space between date and time
            }

            // populate BOOLEAN values
            if( strcmp( key.c_str(), input_bool_runMITM.c_str()) == 0 ) {
                pBoolParams->_runMITMModel = Utility::stringToBoolean(value);            
            } else if( strcmp( key.c_str(), input_bool_runUFBWSeqPickups.c_str()) == 0 ) {
                pBoolParams->_runUFBW_seqPickups = Utility::stringToBoolean(value);
            } else if( strcmp( key.c_str(), input_bool_runFlexDep.c_str() ) == 0 ) {
                pBoolParams->_runFlexDepModel = Utility::stringToBoolean(value);
            } else if( strcmp( key.c_str(), input_bool_runPerfInfo.c_str()) == 0 ) {
                pBoolParams->_runUFBW_perfInfo = Utility::stringToBoolean(value);
            } else if( strcmp( key.c_str(), input_bool_multPickups.c_str()) == 0 ) {
                pBoolParams->_runMultiplePickups = Utility::stringToBoolean(value);
                std::string runMult = (pBoolParams->_runMultiplePickups) ? "true" : "false";
            } else if( strcmp( key.c_str(),input_bool_printDebugFiles.c_str()) == 0 ) {
                pBoolParams->_printDebugFiles = Utility::stringToBoolean(value);                // print debug files
            } else if( strcmp(key.c_str(), input_bool_printToScreen.c_str()) == 0 ) {
                pBoolParams->_printToScreen = Utility::stringToBoolean(value);                  // print to screen 
            } else if( strcmp(key.c_str(), input_bool_printIndivSolnMetrics.c_str()) == 0 ) {
                pBoolParams->_printIndivSolnMetrics = Utility::stringToBoolean(value);          // print indiv soln metrics
            } else if( strcmp(key.c_str(), input_bool_populateInitOpenTrips.c_str()) == 0 ) {
                pBoolParams->_populateInitOpenTrips = Utility::stringToBoolean(value);          // populate init open trips
            } else if( strcmp(key.c_str(), input_bool_inclInitPickupInSavingsConstr.c_str()) == 0 ) {
                pBoolParams->_inclInitPickupInSavingsConstr = Utility::stringToBoolean(value);  // incl init pickup in savings constr
            } else if( strcmp(key.c_str(), input_bool_useAggTripInObjAndConstr.c_str()) == 0 ) {
                pBoolParams->_useAggConstrAndObj = Utility::stringToBoolean(value);
            }
                        
            // populate INT values
            else if( strcmp(key.c_str(), input_int_maxAllowablePickups.c_str()) == 0 ) {
                pIntParams->_maxAllowablePickups = Utility::stringToInt(value);             //  max pickups permitted
            } else if( strcmp(key.c_str(), input_int_defaultUpFrontBatchWindowInSec.c_str()) == 0 ) {
                pIntParams->_default_upFrontBatchWindowInSec = Utility::stringToInt(value); // default up front batch window in sec
            } else if( strcmp(key.c_str(), input_int_defaultFlexDepWindowInSec.c_str()) == 0 ) {
                pIntParams->_flexDepWindowInSec = Utility::stringToInt(value);      // default flex dep window in sec
            } else if( strcmp(key.c_str(), input_int_simLenInMin.c_str()) == 0 ) {
                pIntParams->_simLengthInMin = Utility::stringToInt(value);
            }  
            
            // populate DOUBLE values
            else if( strcmp(key.c_str(), input_double_defaultOptInRate.c_str()) == 0 ) {
                pDoubleParams->_defaultOptInRate = Utility::stringToDouble(value);         // default optIn rate
            } else if( strcmp(key.c_str(), input_double_defaultMaxMatchDistInKm.c_str()) == 0 ) {
                pDoubleParams->_default_maxMatchDistInKm = Utility::stringToDouble(value); // default max match dist
            } else if( strcmp(key.c_str(), input_double_defaultMinPoolDiscount.c_str()) == 0 ) {
                pDoubleParams->_default_minPoolDiscount = Utility::stringToDouble(value);  // default min savings 
            } else if( strcmp(key.c_str(), input_double_flexDepOptInRate.c_str()) == 0 ) {
                pDoubleParams->_flexDepOptInRate = Utility::stringToDouble(value); // default optIn 
            }
            
            // populate STRING values
            else if( strcmp(key.c_str(), input_str_inputCSV.c_str()) == 0 ) {
                pStringParams->_inputData = value;                                      // input CSV file
            } else if( strcmp(key.c_str(), input_str_geofenceData.c_str()) == 0 ) {
                pStringParams->_geofenceData = value;                                   // input file assoicated with geofence .txt input
            } else if( strcmp(key.c_str(), input_str_outputBasePath.c_str()) == 0 ) {
                pStringParams->_outputBasePath = value;                                 // output base path                                   
            } else if( strcmp(key.c_str(), input_str_simStartTime.c_str()) == 0 ) {
                pStringParams->_simStartTime = value;                                   // sim start time in string format
            }
            
            // populate ENUM values
            else if( strcmp(key.c_str(), input_str_experiment.c_str()) == 0 ) {
                try {
                    Experiment expValue = convertExperimentToExpEnum(value);
                    pEnumParams->_experiment = expValue;
                } catch( const ExperimentUndefinedException &ex ) {
                    std::cerr << "\nExperimentUndefinedException thrown... " << endl;
                    std::cerr << ex.what() << endl;
                    std::cerr << "\t(process terminated)" << endl;
                    exit(1);
                }
            } else if( strcmp(key.c_str(), input_str_geofenceType.c_str()) == 0 ) {
                try {
                    Geofence::Type geofenceType = convertGeofenceTypeToEnum(value);
                    pEnumParams->_geofenceType = geofenceType;
                } catch( const EnumException &ex ) {
                    std::cerr << "\nEnumException thrown... " << std::endl;
                    std::cerr << ex.what() << std::endl;
                    std::cerr << "\t(process terminated)" << std::endl;
                    exit(1);
                }
            }
            
            // if the experiment vectors are read, parse accordingly
            else if( strcmp(key.c_str(), input_range_optInExp.c_str()) == 0 ) {
                std::vector<double> optInRange = parseCsvRange_double(value);
                pRangeParams->optInRange = optInRange;
            } else if( strcmp(key.c_str(), input_range_batchWindowExp.c_str()) == 0 ) {
                std::vector<double> batchWindowRange = parseCsvRange_double(value);
                pRangeParams->batchWindowRange = batchWindowRange;
            } else if( strcmp(key.c_str(), input_range_maxPickupExp.c_str()) == 0 ) {
                std::vector<double> maxPickupRange = parseCsvRange_double(value);
                pRangeParams->maxPickupRange = maxPickupRange;
            } else if( strcmp(key.c_str(), input_range_minSavingsExp.c_str()) == 0 ) {
                std::vector<double> minSavingsRange = parseCsvRange_double(value);
                pRangeParams->minSavingsRange = minSavingsRange;
            }

            _keyValueMap.insert(make_pair(key,value));
        }
      }
    }    
}

void UserConfig::printKeyValueMap() {
    std::map<std::string,std::string>::iterator mapItr;
    for( mapItr = _keyValueMap.begin(); mapItr != _keyValueMap.end(); ++mapItr ) {
        std::cout << "key = " << mapItr->first << std::endl;
        std::cout << "value = " << mapItr->second << std::endl;
    }
}

Experiment UserConfig::convertExperimentToExpEnum(std::string experimentStr) {

    std::string defaultValuesStr = "DEFAULTVALUES";
    std::string optInStr         = "OPTIN";
    std::string batchWindowStr   = "BATCHWINDOW";
    std::string pickupDistStr    = "PICKUPDIST";
    std::string savingsStr       = "SAVINGSRATE";
    
    if( strcmp(experimentStr.c_str(), defaultValuesStr.c_str()) == 0 ) {
        return DEFAULTVALUES;
    } else if( strcmp(experimentStr.c_str(), optInStr.c_str()) == 0 ) {
        return OPTIN;
    } else if( strcmp(experimentStr.c_str(), batchWindowStr.c_str()) == 0 ) {
        return BATCHWINDOW;        
    } else if( strcmp(experimentStr.c_str(), pickupDistStr.c_str()) == 0 ) {
        return PICKUP;
    } else if( strcmp(experimentStr.c_str(), savingsStr.c_str()) == 0 ) {
        return SAVINGSRATE;
    } else {
        throw new ExperimentUndefinedException(experimentStr, eligExperimentInputStrings);
    }
}
Geofence::Type UserConfig::convertGeofenceTypeToEnum(std::string geofenceRestrTypeStr) {
    std::string reqOnlyStr = "REQ_ONLY";
    std::string origOnlyStr = "ORIG_ONLY";
    std::string entireTripStr = "ENTIRE_TRIP";
    
    if( strcmp( geofenceRestrTypeStr.c_str(), reqOnlyStr.c_str()) == 0 ) {
        return Geofence::REQ_ONLY;
    } else if( strcmp( geofenceRestrTypeStr.c_str(), origOnlyStr.c_str()) == 0 ) {
        return Geofence::ORIG_ONLY;
    } else if( strcmp( geofenceRestrTypeStr.c_str(), entireTripStr.c_str()) == 0 ) {
        return Geofence::ENTIRE_TRIP;
    } else {
        throw new EnumException(geofenceRestrTypeStr, eligGeofenceRestrTypeStrings);
    }
}

std::vector<double> UserConfig::parseCsvRange_double(std::string str) {
    std::vector<double> rangeVec;
    
    std::stringstream ss(str);
    double value;
    while( ss >> value) {
        rangeVec.push_back(value);
        
        if( ss.peek() == ',' )
            ss.ignore();
    }
        
    return rangeVec;
}