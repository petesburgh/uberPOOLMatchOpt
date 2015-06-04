/* 
 * File:   DataContainer.hpp
 * Author: jonpetersen
 *
 * Created on March 18, 2015, 11:19 AM
 */

#ifndef DATACONTAINER_HPP
#define	DATACONTAINER_HPP

#include "CSVRow.hpp"
#include "TripData.hpp"
#include "Driver.hpp"
#include "Rider.hpp"
#include "Request.hpp"
#include "OpenTrip.hpp"
#include "LatLng.hpp"
#include "Comparators.hpp"
#include "Geofence.hpp"
#include "Utility.hpp"

#include "ItemNotFoundException.hpp" /* custom exception */
#include "FileNotFoundException.hpp" /* custom exception */

#include<iostream>
#include<sstream>
#include<vector>
#include<string>
#include<set>
#include<fstream>
#include<iterator>
#include<assert.h>
#include<stdlib.h>  /* atof */
#include<time.h>    /* struct tm */
#include<math.h>    /* round */
#include<algorithm> /* min */

using namespace std;

class DataContainer {
public:
   // DataContainer(const std::string &inputPath, const std::string &filename, const std::string &timelineStr, const double optInRate, const int simLengthInMin, const bool printDebugFiles, const bool printToScreen, const Geofence * geofence);
    DataContainer(const std::string &inputCsvFile, const std::string &timelineStr, const double optInRate, const int simLengthInMin, const bool printDebugFiles, const bool printToScreen, const Geofence * geofence);
    virtual ~DataContainer();
    
    // methods called by main()
    void extractCvsSnapshot();          // extracts DB snapshot
    bool generateUberPoolTripProxy();   // filters uberX trips to proxy uberPOOL trips
    int buildUberPOOLTrips();           // build uberPOOL trips (discarding uberX trips) 
    bool populateRequestsAndTrips();    // translate data into requests and trips
    
    // I/O methods
    TripData* defineCurrentTripInfoFromCsvLine(CSVRow& row);
    
    // getters
    const std::string getCsvInputFile()  const { return _inputCsvFile; }
   // const std::string getInputPath()   const { return _inputPath; }
    //const std::string getCsvFilename() const { return _csvFilename; }
    const std::vector<TripData*>* getAllTrips() const { return &_allTrips; }
    const std::vector<TripData*>* getUberPoolTrips() const { return &_uberPOOLTrips; }
    const std::set<Driver*, DriverIndexComp>* getAllDrivers() const { return &_allDrivers; }
    const std::set<Rider*, RiderIndexComp>* getAllUberXRiders() const { return &_allUberXRiders; }
    const std::set<Rider*, RiderIndexComp>* getAllUberPoolRiders() const { return &_uberPoolRiders; }
    const time_t getTimeline() const { return _timeline; }
    const int getUpFrontBatchWindowLenInSec() const { return _batchWindowInSec; }
    const double getOptInRate() const { return _optInRate; }
    const time_t getSimEndTime() const { return _simEndTime; }
    
    // get objects to instantiate algorithms
    std::set<Request*,  ReqComp> getAllPoolRequestsInSim() { return _allRequestsInSim; } 
    std::set<Request*,  ReqComp> getInitPoolRequestsAtTimeline() { return _initRequests; }     
    std::set<OpenTrip*, EtdComp> getInitOpenTripsAtTimeline() { return _initOpenTrips; }    
    
    // methods to return objects from IDs
    Rider * getRiderFromID(const std::string riderID);
    Driver * getDriverFromID(const std::string driverID);
    
    // get print methods
    const bool printDebugFiles() const { return _printDebugFiles; }
    const bool printToScreen() const { return _printToScreen; }
    
    // check if a given trip if feasible for geofence
    bool isEligForGeofence(TripData * pTrip, const Geofence * pGeofence);
    
    // set instance-specific attributes
   void setBatchWindowInSeconds(int batchWindow) { _batchWindowInSec = batchWindow; }
    
   
private:
    // I/O
   // std::string _inputPath;   // input path for folder containing input data
   // std::string _csvFilename; // name of csv file containing snapshot data
    std::string _inputCsvFile; // 
    std::string _timelineStr; // string of timeline defining snapshot
    int _batchWindowInSec;    // length of up-front batch window
    double _optInRate;        // percent of users requesting uberX that define POOL requests
    time_t _timeline;         // time_t represented converted from inputted string
    time_t _simEndTime;       // end time of simulation (i.e. do not consider trips after this time)
    bool _printDebugFiles;    // bool to determine whether/not to print debug files
    bool _printToScreen;      // cout data
    
    // all geofences
    const Geofence* pGeofence;
        
    // entities across DB snapshot
    std::vector<TripData*> _allTrips;      // ALL trips
    std::vector<TripData*> _uberPOOLTrips; // POOL trips
    
    // initial requests (i.e. TripData that have not been dispatched)
    std::set<Request*,  ReqComp> _allRequestsInSim;  // all requests starting at timeline through sim end
    std::set<Request*,  ReqComp> _initRequests;      // open requests only at timeline
    std::set<Request*,  ReqComp> _futureRequests;    // all future requests after timeline
    std::set<OpenTrip*, EtdComp> _initOpenTrips;     // open trips at timeline
        
    // vector of all drivers
    std::set<Driver*, DriverIndexComp> _allDrivers;
    
    // vector of ALL uberX riders (this is not the final vector used)
    std::set<Rider*, RiderIndexComp> _allUberXRiders;
    
    // vector of uberPOOL riders that are proxy from all requests using given input parameter
    std::set<Rider*, RiderIndexComp> _uberPoolRiders;
    
    // mapping between id and Driver/Rider object ptr
    std::map<const std::string, Driver*> _idDriverMap;
    std::map<const std::string, Rider*> _idRiderMap;
    
    // given id, get Driver object
    Driver * getDriverFromTrip(const std::string id);
    
    // given id, get Rider object
    Rider * getRiderFromTrip(const std::string id);
    
    // check if given rider ID is a POOL rider
    const bool isPOOLRider(std::string riderID) const;

};

#endif	/* DATACONTAINER_HPP */

