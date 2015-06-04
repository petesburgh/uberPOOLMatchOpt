#include <iomanip>

#include "GenerateInstanceScenarios.hpp"

// populate input scenario maps
ProblemInstance * GenerateInstanceScenarios::generateInstanceScenarios(const int scenIndex, const std::string &geofenceFolderPath) {
       
    switch ( scenIndex ) {
        case 1 :
        {
            // INSTANCE 01: SF, ONE HOUR SIM FROM 2015-04-13 1600-1700 UTC, NO GEOFENCES
            int cityIndex = SF;
            const std::string simStartTimeStr   = "2015-04-13 16:00:00";
            const int simLengthInMin            = 60;
            const std::string inputCsvFilename  = "trips-SF-2015-04-13-1600-1700-uberX.csv";
            const std::string scenStr           = "SF-20150413-1600-1Hr-noGeo";                        
            ProblemInstance * pInstance = new ProblemInstance(cityIndex, simStartTimeStr, simLengthInMin, inputCsvFilename, NULL, scenStr);
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

            const std::string geofenceInputPath = geofenceFolderPath + "SF-whiteout.txt"; 
            const Geofence * pGeofence = extractGeofence(Geofence::REQ_ONLY, geofenceInputPath);
            
            ProblemInstance * pInstance = new ProblemInstance(cityIndex, simStartTimeStr, simLengthInMin, inputCsvFilename, pGeofence, scenStr);
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
            
            ProblemInstance * pInstance = new ProblemInstance(cityIndex, simStartTimeStr, simLengthInMin, inputCsvFilename, NULL, scenStr);
            return pInstance;
        }
        case 4 :
        {
            // INSTANCE 04: SF, ONE WEEK SIM FROM 2015-04-12 0000 - 2015-04-19 0000 (UTC), SF whiteout geofence
            const int cityIndex                  = SF;
            const std::string simStartTimeStr    = "2015-04-12 07:00:00";
            const int simLengthInMin             = 10080;
            const std::string inputCsvFilename   = "trips-SF-2015-04-12-0000-2015-04-19-0000-uberX.csv";  
            const std::string scenStr            = "SF-20150412-0000-1Week-noGeo";
            
            // extract SF whiteout geofence (geofence id 14617)
            const std::string geofenceInputPath = geofenceFolderPath + "SF-whiteout.txt"; 
            const Geofence * pGeofence = extractGeofence(Geofence::REQ_ONLY, geofenceInputPath);
            
            ProblemInstance * pInstance = new ProblemInstance(cityIndex, simStartTimeStr, simLengthInMin, inputCsvFilename, pGeofence, scenStr);
            return pInstance;            
        }
        case 5 :
        {
            // INSTANCE 05: SF, four hour sim from 1300-1700 UTC on 2015-04-24, SF whiteout geofence
            const int cityIndex                  = SF;
            const std::string simStartTimeStr    = "2015-04-24 13:00:00";
            const int simLengthInMin             = 240;
            const std::string inputCsvFilename   = "trips-SF-2015-04-24-1300-1700-uberX.csv";
            const std::string scenStr            = "SF-20150424-0600-4Hr-1Geo";
            
            // extract SF whiteout geofence (geofence id 14617)
            const std::string geofenceInputPath = geofenceFolderPath + "SF-whiteout.txt"; 
            const Geofence * pGeofence = extractGeofence(Geofence::ENTIRE_TRIP, geofenceInputPath);
            
            ProblemInstance * pInstance = new ProblemInstance(cityIndex, simStartTimeStr, simLengthInMin, inputCsvFilename, pGeofence, scenStr);
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
            
            ProblemInstance * pInstance = new ProblemInstance(cityIndex, simStartTimeStr, simLengthInMin, inputCsvFilename, NULL, scenStr);
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
            
            ProblemInstance * pInstance = new ProblemInstance(cityIndex, simStartTimeStr, simLengthInMin, inputCsvFilename, NULL, scenStr);  
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
            
            ProblemInstance * pInstance = new ProblemInstance(cityIndex, simStartTimeStr, simLengthInMin, inputCsvFilename, NULL, scenStr);  
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
            
            ProblemInstance * pInstance = new ProblemInstance(cityIndex, simStartTimeStr, simLengthInMin, inputCsvFilename, NULL, scenStr);  
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
            
            ProblemInstance * pInstance = new ProblemInstance(cityIndex, simStartTimeStr, simLengthInMin, inputCsvFilename, NULL, scenStr);  
            return pInstance;             
        }
        case 11 : 
        {
            const int cityIndex                  = LA;
            const std::string simStartTimeStr    = "2015-04-27 07:00:00";
            const int simLengthInMin             = 1440;
            const std::string inputCsvFilename   = "trips-LA-20150427-0700-20150428-0700-uberX.csv"; 
            const std::string scenStr            = "LA-20150427-0000-1Day-geofence";

            // extract LA geofence id 23664
            const std::string geofenceInputPath = geofenceFolderPath + "LA-geo.txt";            
            const Geofence * pGeofence = extractGeofence(Geofence::REQ_ONLY, geofenceInputPath);
            
            ProblemInstance * pInstance = new ProblemInstance(cityIndex, simStartTimeStr, simLengthInMin, inputCsvFilename, pGeofence, scenStr);  
            return pInstance;              
        }
        case 12 : 
        {
            const int cityIndex                  = LA;
            const std::string simStartTimeStr    = "2015-04-25 01:00:00";
            const int simLengthInMin             = 300;
            const std::string inputCsvFilename   = "trips-LA-20150425-0100-20150425-0600-uberX.csv"; 
            const std::string scenStr            = "LA-20150424-1800-5Hr-geofence";
            
            // extract LA geofence id 23664
            const std::string geofenceInputPath = geofenceFolderPath + "LA-geo.txt";            
            const Geofence * pGeofence = extractGeofence(Geofence::REQ_ONLY, geofenceInputPath);            
            
            ProblemInstance * pInstance = new ProblemInstance(cityIndex, simStartTimeStr, simLengthInMin, inputCsvFilename, pGeofence, scenStr);  
            return pInstance;              
        }   
        case 13 :
        {
            const int cityIndex                  = NJ;
            const std::string simStartTimeStr    = "2015-04-26 00:00:00";
            const int simLengthInMin             = 1440;
            const std::string inputCsvFilename   = "trips-NJ-20150426-0000-20150503-0000-uberX.csv"; 
            const std::string scenStr            = "NJ-20150426-0000-1Wk-noGeo";
 
            ProblemInstance * pInstance = new ProblemInstance(cityIndex, simStartTimeStr, simLengthInMin, inputCsvFilename, NULL, scenStr);  
            return pInstance;                
        }
        case 14 :
        {
            const int cityIndex                  = SD;
            const std::string simStartTimeStr    = "2015-04-26 00:00:00";
            const int simLengthInMin             = 10080;
            const std::string inputCsvFilename   = "trips-SanDiego-2015-04-26-0700-2015-05-03-0700-uberX.csv";
            const std::string scenStr            = "SD-20150426-0000-1Wk-noGeo";
            
            ProblemInstance * pInstance = new ProblemInstance(cityIndex, simStartTimeStr, simLengthInMin, inputCsvFilename, NULL, scenStr);
            return pInstance;
        }
        case 15 :
        {
            const int cityIndex                 = CHENGDU;
            const std::string simStartTimeStr   = "2015-05-10 00:00:00";
            const int simLengthInMin            = 10080;
            const std::string inputCsvFilename  = "trips-Chengdu-20150509-1600-20150516-1600-uberX.csv";
            const std::string scenStr           = "Chengdu-20150410-0000-1Wk-noGeo";
            
            ProblemInstance * pInstance = new ProblemInstance(cityIndex, simStartTimeStr, simLengthInMin, inputCsvFilename, NULL, scenStr);
            return pInstance;
        }
        case 16 :
        {
            const int cityIndex                 = SF;
            const std::string simStartTimeStr   = "2015-05-04 10:00:00";
            const int simLengthInMin            = 10080;
            const std::string inputCsvFilename  = "SF_no_snapping.csv";
            const std::string scenStr           = "SF-noSnap";
            
            const std::string geofenceInputPath = geofenceFolderPath + "SF-whiteout.txt"; 
            const Geofence * pGeofence = extractGeofence(Geofence::REQ_ONLY, geofenceInputPath);
            
            ProblemInstance * pInstance = new ProblemInstance(cityIndex, simStartTimeStr, simLengthInMin, inputCsvFilename, pGeofence, scenStr);
            return pInstance;
        }
        case 17 : 
        {
            const int cityIndex                 = SF;
            const std::string simStartTimeStr   = "2015-05-04 10:00:00";
            const int simLengthInMin            = 10080;
            const std::string inputCsvFilename  = "SF_55p_snapped.csv";
            const std::string scenStr           = "SF-noSnap";
            
            const std::string geofenceInputPath = geofenceFolderPath + "SF-whiteout.txt"; 
            const Geofence * pGeofence = extractGeofence(Geofence::REQ_ONLY, geofenceInputPath);
            
            ProblemInstance * pInstance = new ProblemInstance(cityIndex, simStartTimeStr, simLengthInMin, inputCsvFilename, pGeofence, scenStr);
            return pInstance;            
        }
        case 18 : 
        {
            const int cityIndex                 = SF;
            const std::string simStartTimeStr   = "2015-05-04 10:00:00";
            const int simLengthInMin            = 10080;
            const std::string inputCsvFilename  = "SF_100p_snapped.csv";
            const std::string scenStr           = "SF-noSnap";
            
            const std::string geofenceInputPath = geofenceFolderPath + "SF-whiteout.txt"; 
            const Geofence * pGeofence = extractGeofence(Geofence::REQ_ONLY, geofenceInputPath);
            
            ProblemInstance * pInstance = new ProblemInstance(cityIndex, simStartTimeStr, simLengthInMin, inputCsvFilename, pGeofence, scenStr);
            return pInstance;            
        }
        case 19 :
        {
            const int cityIndex                 = SF;
            const std::string simStartTimeStr   = "2015-05-20 00:00:00";
            const int simLengthInMin            = 10080;
            const std::string inputCsvFilename  = "SFO_OAK-20150520-0000-20150527-0000.csv";
            const std::string scenStr           = "SFO_OAK_snap";
            
            ProblemInstance * pInstance = new ProblemInstance(cityIndex, simStartTimeStr, simLengthInMin, inputCsvFilename, NULL, scenStr);
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

const Geofence * GenerateInstanceScenarios::extractGeofence(const Geofence::Type type, const std::string &geofenceDataFile) {
    
    Geofence * pGeofence = new Geofence(type);
    
    ifstream inFile;
    inFile.open(geofenceDataFile);
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
    
    return pGeofence;
}

