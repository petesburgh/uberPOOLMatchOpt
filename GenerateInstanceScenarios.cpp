#include "GenerateInstanceScenarios.hpp"

// populate input scenario maps
ProblemInstance * generateInstanceScenarios(const int scenIndex) {
       
    std::vector<Geofence*> emptyGeofenceVec;
    
    switch ( scenIndex ) {
        case 1 :
        {
            // INSTANCE 01: SF, ONE HOUR SIM FROM 2015-04-13 1600-1700 UTC, NO GEOFENCES
            int cityIndex = SF;
            const std::string simStartTimeStr   = "2015-04-13 16:00:00";
            const int simLengthInMin            = 60;
            const std::string inputCsvFilename  = "trips-SF-2015-04-13-1600-1700-uberX.csv";
            const std::string scenStr           = "SF-20150413-1600-1Hr-noGeo";                        
            ProblemInstance * pInstance = new ProblemInstance(cityIndex, simStartTimeStr, simLengthInMin, inputCsvFilename, emptyGeofenceVec, scenStr);
            return pInstance;                        
        }
        case 2 :
        {
            // INSTANCE 02: SF, ONE HOUR SIM FROM 2015-04-13 1600-1700 UTC WITH TWO GEOFENCES
            int cityIndex                       = SF;
            const std::string simStartTimeStr   = "2015-04-13 16:00:00";
            const int simLengthInMin            = 60;
            const std::string inputCsvFilename  = "trips-SF-2015-04-13-1600-1700-uberX.csv";
            const std::string scenStr           = "SF-20150413-1600-1Hr-2Geofences";                        
            
            // define two geofences: SF city (7x7) and SFO airport
            std::vector<Geofence*> geofences;
            Geofence * pGeofence_SF_city    = new Geofence(Geofence::REQ_ONLY, 37.698491,37.801104,-122.530174,-122.344093); 
            Geofence * pGeofence_SF_airport = new Geofence(Geofence::REQ_ONLY, 37.604440,37.636529,-122.405205,-122.346153);
            geofences.push_back(pGeofence_SF_city);
            geofences.push_back(pGeofence_SF_airport);
            
            ProblemInstance * pInstance = new ProblemInstance(cityIndex, simStartTimeStr, simLengthInMin, inputCsvFilename, geofences, scenStr);
            return pInstance;   
        }
        case 3 :
        {
            // INSTANCE 03: SF, ONE WEEK SIM FROM 2015-04-12 0000 - 2015-04-19 0000 (UTC), no geofences
            int cityIndex                        = SF;
            const std::string simStartTimeStr    = "2015-04-12 07:00:00";
            const int simLengthInMin             = 10080;
            const std::string inputCsvFilename   = "trips-SF-2015-04-12-0000-2015-04-19-0000-uberX.csv";  
            const std::string scenStr            = "SF-20150412-0000-1Week-noGeo";
            
            ProblemInstance * pInstance = new ProblemInstance(cityIndex, simStartTimeStr, simLengthInMin, inputCsvFilename, emptyGeofenceVec, scenStr);
            return pInstance;
        }
        case 4 :
        {
            // INSTANCE 04: SF, ONE WEEK SIM FROM 2015-04-12 0000 - 2015-04-19 0000 (UTC), two geofences
            const int cityIndex                  = SF;
            const std::string simStartTimeStr    = "2015-04-12 07:00:00";
            const int simLengthInMin             = 10080;
            const std::string inputCsvFilename   = "trips-SF-2015-04-12-0000-2015-04-19-0000-uberX.csv";  
            const std::string scenStr            = "SF-20150412-0000-1Week-noGeo";
            
            // define two geofences: SF city (7x7) and SFO airport
            std::vector<Geofence*> geofences;
            Geofence * pGeofence_SF_city    = new Geofence(Geofence::REQ_ONLY, 37.698491,37.801104,-122.530174,-122.344093); 
            Geofence * pGeofence_SF_airport = new Geofence(Geofence::REQ_ONLY, 37.604440,37.636529,-122.405205,-122.346153);
            geofences.push_back(pGeofence_SF_city);
            geofences.push_back(pGeofence_SF_airport);            
            
            ProblemInstance * pInstance = new ProblemInstance(cityIndex, simStartTimeStr, simLengthInMin, inputCsvFilename, geofences, scenStr);
            return pInstance;            
        }
        case 5 :
        {
            // INSTANCE 05: SF, four hour sim from 1300-1700 UTC on 2015-04-24, one SF geofence (incl SFO airport)
            const int cityIndex                  = SF;
            const std::string simStartTimeStr    = "2015-04-24 13:00:00";
            const int simLengthInMin             = 240;
            const std::string inputCsvFilename   = "trips-SF-2015-04-24-1300-1700-uberX.csv";
            const std::string scenStr            = "SF-20150424-0600-4Hr-1Geo";
            
            // define one geofence: encompasses most of SF incl SFO 
            std::vector<Geofence*> geofences;
            Geofence * pGeofence_SF = new Geofence(Geofence::ENTIRE_TRIP, 37.59573590243413, 37.82000000, -122.54596710205078, -122.32349395751953);
            geofences.push_back(pGeofence_SF);
            
            ProblemInstance * pInstance = new ProblemInstance(cityIndex, simStartTimeStr, simLengthInMin, inputCsvFilename, geofences, scenStr);
            return pInstance;
        }
        case 6 :
        {
            // INSTANCE 06: Chengdu, one week sim from 2015-03-29 1600 - 2015-04-05 1600 UTC, no consolidation, no geofences
            const int cityIndex                  = CHENGDU;
            const std::string simStartTimeStr    = "2015-03-29 16:00:00";
            const int simLengthInMin             = 10080;
            const std::string inputCsvFilename   = "trips-Chengdu-20150330-0000-20150406-0000-uberX.csv"; 
            const std::string scenStr            = "Chengdu-20150330-1Week-noConsolidation-noGeo";
            
            ProblemInstance * pInstance = new ProblemInstance(cityIndex, simStartTimeStr, simLengthInMin, inputCsvFilename, emptyGeofenceVec, scenStr);
            return pInstance;
        }
        case 7 :
        {
            // INSTANCE 07: Chengdu, two week data from 2015-03-29 1600 - 2015-04-05 1600 UTC, data to be consolidated, no geofences
            const int cityIndex                  = CHENGDU;
            const std::string simStartTimeStr    = "2015-03-29 16:00:00";
            const int simLengthInMin             = 10080;
            const std::string inputCsvFilename   = "trips-Chengdu-20150330-0000-20150413-0000-uberX.csv"; 
            const std::string scenStr            = "Chengdu-20150330-1Week-noConsolidation-noGeo";
            
            ProblemInstance * pInstance = new ProblemInstance(cityIndex, simStartTimeStr, simLengthInMin, inputCsvFilename, emptyGeofenceVec, scenStr);  
            return pInstance;
        }
        case 8 :
        {
            // INSTANCE 08: SF, one week data from 2015-04-27 0700 - 2015-04-28 0700 UTC, no geofences (FD analysis))
            const int cityIndex                  = SF;
            const std::string simStartTimeStr    = "2015-04-27 07:00:00";
            const int simLengthInMin             = 1440;
            const std::string inputCsvFilename   = "trips-SF-20150427-0700-20150428-0700-uberX.csv"; 
            const std::string scenStr            = "SF-20150427-0000-1Day-noGeo";
            
            ProblemInstance * pInstance = new ProblemInstance(cityIndex, simStartTimeStr, simLengthInMin, inputCsvFilename, emptyGeofenceVec, scenStr);  
            return pInstance;            
        }
        case 9 :
        {
            // INSTANCE 09: LA, one week data from 2015-04-27 0700 - 2015-04-28 0700 UTC, no geofences (FD analysis)
            const int cityIndex                  = LA;
            const std::string simStartTimeStr    = "2015-04-27 07:00:00";
            const int simLengthInMin             = 1440;
            const std::string inputCsvFilename   = "trips-LA-20150427-0700-20150428-0700-uberX.csv"; 
            const std::string scenStr            = "LA-20150427-0000-1Day-noGeo";
            
            ProblemInstance * pInstance = new ProblemInstance(cityIndex, simStartTimeStr, simLengthInMin, inputCsvFilename, emptyGeofenceVec, scenStr);  
            return pInstance;                  
        }
        case 10 :
        {
            // INSTANCE 10: AUS, one week data from 2015-04-27 0700 - 2015-04-28 0700 UTC, no geofences (FD analysis)
            const int cityIndex                  = AUSTIN;
            const std::string simStartTimeStr    = "2015-04-27 07:00:00";
            const int simLengthInMin             = 1440;
            const std::string inputCsvFilename   = "trips-AUS-20150427-0700-20150428-0700-uberX.csv"; 
            const std::string scenStr            = "AUS-20150427-0000-1Day-noGeo";
            
            ProblemInstance * pInstance = new ProblemInstance(cityIndex, simStartTimeStr, simLengthInMin, inputCsvFilename, emptyGeofenceVec, scenStr);  
            return pInstance;             
        }
        case 11 : 
        {
            const int cityIndex                  = LA;
            const std::string simStartTimeStr    = "2015-04-27 07:00:00";
            const int simLengthInMin             = 1440;
            const std::string inputCsvFilename   = "trips-LA-20150427-0700-20150428-0700-uberX.csv"; 
            const std::string scenStr            = "LA-20150427-0000-1Day-noGeo";
            
            std::vector<Geofence*> geofenceVec;
            Geofence * pGeofence = new Geofence(Geofence::ENTIRE_TRIP, 33.9308311, 34.136744, -118.58746, -118.19967);
            
            ProblemInstance * pInstance = new ProblemInstance(cityIndex, simStartTimeStr, simLengthInMin, inputCsvFilename, geofenceVec, scenStr);  
            return pInstance;              
        }
        default : 
        {
            std::cout << "\n\n--- ERROR: SCENARIO NUMBER NOT DEFINED ---\n" << std::endl;
            std::cout << "\tplease either change scenario index number or add new instance in 'GenerateInstanceScenarios.cpp'" << std::endl;
            std::cout << "\n\n-- exiting with error flag --\n\n" << std::endl;
            exit(1);
        }
        
        return NULL;
    }
    
    
    
    
    return NULL;
}
