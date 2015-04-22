/* 
 * File:   DataContainer.cpp
 * Author: jonpetersen
 * 
 * Created on March 18, 2015, 11:19 AM
 */

#include "DataContainer.hpp"
#include "TripData.hpp"

// constructors
DataContainer::DataContainer() {
}
DataContainer::DataContainer(const std::string &inputPath, const std::string &csvFilename, const std::string &timelineStr, const int batchWindowInSec, const double pctPoolUsers, const int simLengthInMin, const bool printDebugFiles, const bool printToScreen) {
    _inputPath = inputPath;
    _csvFilename = csvFilename;
    _timelineStr = timelineStr;
    _timeline = Utility::convertDateTimeStringToTimeT(timelineStr);
    _batchWindowInSec = batchWindowInSec;
    _pctPoolUsers = pctPoolUsers;
    _printDebugFiles = printDebugFiles;
    _printToScreen = printToScreen;
    _simEndTime = _timeline + (time_t)(60*simLengthInMin);
    
    // simple data validation
    assert( _batchWindowInSec >= 0 );
    assert( (0 <= _pctPoolUsers) && (_pctPoolUsers <= 1) );
    assert( _timeline <= _simEndTime );
    
}

// destructor
DataContainer::~DataContainer() {
}

// ----------------------
//   extract CSV file
// ----------------------
std::istream& operator>>(std::istream& str,CSVRow& data) {
    data.readNextRow(str);
    return str;
} 
void DataContainer::extractCvsSnapshot() {
    
    const std::string filePath = _inputPath + _csvFilename;    
    std::ifstream       file(filePath);
    CSVRow row;
    
    int rowIndex = 0;
    while( file >> row ) {        
        size_t nCols = row.size();
        assert( nCols == 15 ); // ENSURE proper dimension of CSV 

       // ignore header row
       if( rowIndex > 0 ) {
           // define current trip
           TripData * currTrip = defineCurrentTripInfoFromCsvLine(row);    
           
           // get Driver* object (and define if necessary)
           Driver * pDriver = getDriverFromTrip(currTrip->getDriverID());
           pDriver->addTrip(currTrip); 
                     
           /*
            *  ignore if the trip if:
            *       (a) it is already complete OR
            *       (b) the request is after the sim end time
            *  note: we did not ignore drivers since not needed for driver info
           */
           if( currTrip->getStatus() == TripData::COMPLETE )
               continue; 
           if( currTrip->getStatus() == TripData::FUTURE ) {
               if( currTrip->getRequestEvent()->timeT > _simEndTime ) 
                   continue;
           }
           
                      
           _allTrips.push_back(currTrip); // append to global list of all trips 

           // get Rider* object (and define if necessary)
           Rider * pRider = getRiderFromTrip(currTrip->getRiderID());
           pRider->addTrip(currTrip);                     
           currTrip->setRiderIndex(pRider->getIndex());
           currTrip->setDriverIndex(pDriver->getIndex());

        } else {
            rowIndex++;
        }
           
    }
     
    file.close();    
}

TripData* DataContainer::defineCurrentTripInfoFromCsvLine(CSVRow& row) {
    TripData * currTrip = new TripData();
       
    // build REQUEST event
    const double reqLat      = atof(row[4].c_str());  
    const double reqLng      = atof(row[5].c_str());    
    time_t requestTime  = Utility::convertDateTimeStringToTimeT(row[3]);
    Event request(requestTime,reqLat,reqLng);
    
    // build DISPATCH event
    const double dispatchLat = atof(row[7].c_str());
    const double dispatchLng = atof(row[8].c_str());
    time_t dispatchTime = Utility::convertDateTimeStringToTimeT(row[6]); 
    Event dispatch(dispatchTime,dispatchLat,dispatchLng);
    
    // build PICKUP event
    double pickLat = atof(row[10].c_str());
    double pickLng = atof(row[11].c_str());    
    time_t pickupTime   = Utility::convertDateTimeStringToTimeT(row[9]);
    Event pickup(pickupTime,pickLat,pickLng);

    // build DROPOFF event
    double dropLat = atof(row[13].c_str());
    double dropLng = atof(row[14].c_str());
    time_t dropoffTime  = Utility::convertDateTimeStringToTimeT(row[12]);
    Event drop(dropoffTime,dropLat,dropLng);

    // populate trip data
    currTrip->setIndex(_allTrips.size());       // index
    currTrip->setUniqueTripID(row[0]);          // unique trip ID
    currTrip->setDriverID(row[1].c_str());      // driver ID
    currTrip->setRiderID(row[2].c_str());       // rider ID    
    currTrip->setRequestEvent(request);         // request 
    currTrip->setDispatchEvent(dispatch);       // dispatch
    currTrip->setPickupEvent(pickup);           // pickup 
    currTrip->setDropoffEvent(drop);            // dropoff 
    
    // get status of trip at timeline
    TripData::Status initStatus = currTrip->getTripStatusDuringTime(_timeline);
    currTrip->setTripStatus(initStatus);        // initial status at timeline

    return currTrip;
}

Driver * DataContainer::getDriverFromTrip(const std::string _id) {
    
    // search for existing entry in mapping between ID and Driver object ptr
    std::map<const std::string, Driver*>::iterator mapItr = _idDriverMap.find(_id);
    if( mapItr != _idDriverMap.end() ) {
        return mapItr->second;
    } 
    
    // if no Driver ptr object exists, create new Driver object here
    else {
        Driver * pDriver = new Driver();
        pDriver->setId(_id); // string identifier as found in DB snapshot
        pDriver->setIndex(_allDrivers.size()); // numeric identifier
        _allDrivers.insert(pDriver); // add to set of all drivers
        _idDriverMap.insert(make_pair(pDriver->getId(), pDriver));
        return pDriver;
    }   
}
Rider * DataContainer::getRiderFromTrip(const std::string id) {
    
    // search for existing entry in mapping between ID and Rider object ptr
    std::map<const std::string, Rider*>::iterator mapItr = _idRiderMap.find(id);
    if( mapItr != _idRiderMap.end() ) {
        return mapItr->second;
    } 
    
    // if no Rider ptr object exists, create new Rider object here
    else {
        Rider * pRider = new Rider();
        pRider->setID(id);
        pRider->setIndex(_allUberXRiders.size());
        _allUberXRiders.insert(pRider);
        _idRiderMap.insert(make_pair(id, pRider));
        return pRider;
    }
}

// -------------------------------------------------------
//   generate uberPOOL requests proxy from uberX requests
// -------------------------------------------------------
bool DataContainer::generateUberPoolTripProxy() {
    
    // if all uberX requests are POOL requests (trivial)
    if( _pctPoolUsers == 1 ) {
        _uberPoolRiders = _allUberXRiders;
        return true;
    } 
    
    // if no uberX requests are POOL requests then the test does not make sense
    else if( _pctPoolUsers == 0 ) {
        std::cerr << "\n\n*** ERROR: no uberPOOL requests are given (0 percent) ***\n\n" << std::endl;
        std::cerr << "\tsystem exiting upon error... " << std::endl;
        return false;
    }
    
    // if the pct is strictly between (0,1)
    else {
        int numPoolUsers = (int)round(_pctPoolUsers*_allUberXRiders.size());
       
        // randomize users
        std::multimap<double, Rider*> randomRiderMap;
        for( std::set<Rider*, RiderIndexComp>::iterator riderIt = _allUberXRiders.begin(); riderIt != _allUberXRiders.end(); ++riderIt ) {
            double randNum = ((double)(std::rand()) / (RAND_MAX));
            randomRiderMap.insert(make_pair(randNum, *riderIt));            
        }
        
                        
        for( std::map<double, Rider*>::iterator randomMapItr = randomRiderMap.begin(); randomMapItr != randomRiderMap.end(); ++randomMapItr ) {
                        
            _uberPoolRiders.insert(randomMapItr->second);
            
            // define all trips taken by riders as POOL trips
            const std::vector<TripData*> * pTrips = randomMapItr->second->getTrips();
            for( std::vector<TripData*>::const_iterator tripItr = pTrips->begin(); tripItr != pTrips->end(); ++tripItr ) {
                (*tripItr)->definePOOLTrip();
            }
            
            if( _uberPoolRiders.size() == numPoolUsers ) {
                return true;
            }            
        } 
               
        
        /*for( std::set<Rider*>::iterator rItr = _allUberXRiders.begin(); rItr != _allUberXRiders.end(); ++rItr ) {
            _uberPoolRiders.insert(*rItr);
            
            // define all trips taken by rider as POOL trips
            const std::vector<TripData*>* pTrips = (*rItr)->getTrips();
            for( std::vector<TripData*>::const_iterator tripItr = pTrips->begin(); tripItr != pTrips->end(); ++tripItr ) {
                (*tripItr)->definePOOLTrip();
            }
            if( _uberPoolRiders.size() == numPoolUsers ) {
                return true;
            }
        }   */    
    }
    
    return false;
}
int DataContainer::buildUberPOOLTrips() {
    int tripsAdded = 0;
    std::vector<TripData*>::iterator iTrip;
    for( iTrip = _allTrips.begin(); iTrip != _allTrips.end(); ++iTrip ) {
        if( (*iTrip)->isPOOL() ) {
            _uberPOOLTrips.push_back(*iTrip);
            tripsAdded++;
        }
    }
    
    return tripsAdded;
}

// ------------------------------------------------------------------------
//   translate TripData into Request, OpenTrip, and AssignedTrip objects
// ------------------------------------------------------------------------
bool DataContainer::populateRequestsAndTrips() {
    
    // loop over all trip data
    std::vector<TripData*>::const_iterator iTrip;
    for( iTrip = _uberPOOLTrips.begin(); iTrip != _uberPOOLTrips.end(); ++iTrip ) {
        
        // define lat and lng associated with drop request (note: initially we are using actual drop as proxy for drop request)
        LatLng dropLocation((*iTrip)->getDropoffEvent()->lat, (*iTrip)->getDropoffEvent()->lng);  
                         
        TripData::Status tripStatus = (*iTrip)->getStatus();        
        switch( tripStatus ) {
                                    
            // case 1: Pre-Dispatch (i.e. request has been made but not matched)
            case TripData::PREDISPATCH :
            {                                          
                Rider * pRider = this->getRiderFromID((*iTrip)->getRiderID());
                Driver * pDriver = this->getDriverFromID((*iTrip)->getDriverID());
                Request * pRequest = new Request((*iTrip)->getRiderID(), (*iTrip)->getRiderIndex(), (*iTrip)->getUniqueTripID(), pDriver, (*iTrip)->getRequestEvent(), dropLocation, (*iTrip)->getPickupEvent()->timeT, (*iTrip)->getDropoffEvent()->timeT, (*iTrip)->getRequestEvent(), (*iTrip)->getDispatchEvent(), (*iTrip)->getPickupEvent(), (*iTrip)->getDropoffEvent());
                pRequest->setInitRequest(true);
                pRequest->setIndex((int)_allRequestsInSim.size());
                
                // populate ALL requests within simulation window
                _allRequestsInSim.insert(pRequest); 
                
                // populate INIT requests
                _initRequests.insert(pRequest);
                
                break;
            }
            
            // case 2: trip is in future
            case TripData::FUTURE : 
            {
                Rider * pRider = this->getRiderFromID((*iTrip)->getRiderID());
                Driver * pDriver = this->getDriverFromID((*iTrip)->getDriverID());
                Request * pRequest = new Request((*iTrip)->getRiderID(), (*iTrip)->getRiderIndex(), (*iTrip)->getUniqueTripID(), pDriver, (*iTrip)->getRequestEvent(), dropLocation, (*iTrip)->getPickupEvent()->timeT, (*iTrip)->getDropoffEvent()->timeT, (*iTrip)->getRequestEvent(),(*iTrip)->getDispatchEvent(), (*iTrip)->getPickupEvent(),(*iTrip)->getDropoffEvent());
                pRequest->setInitRequest(false);
                pRequest->setIndex((int)_allRequestsInSim.size());
                
                // populate ALL requests within simulation window
                _allRequestsInSim.insert(pRequest);
                
                break;
            }
            
            // case 3: trip is deadhead (i.e. driver assignment made and is en route to pickup initial passenger)
            case TripData::DEADHEAD :
            {
                int driverIndex = -1;
                int riderIndex = -1;
                Driver * pDriver = NULL;
                try {
                    pDriver = getDriverFromID((*iTrip)->getDriverID());
                } catch( ItemNotFoundException &ex ) {
                    std::cerr << "*** ItemNotFoundException thrown ***" << std::endl;
                    std::cerr << ex.what() << std::endl;
                }
                try {
                    Rider * pRider = getRiderFromID((*iTrip)->getRiderID());
                    riderIndex = pRider->getIndex();                   
                } catch ( ItemNotFoundException &ex ) {
                    std::cerr << "*** ItemNotFoundException thrown ***" << std::endl;
                    std::cerr << ex.what() << std::endl;
                }
                                
                OpenTrip * pOpenTrip = new OpenTrip((*iTrip)->getDriverID(), pDriver, driverIndex, (*iTrip)->getRiderID(), riderIndex, (*iTrip)->getUniqueTripID(), (*iTrip)->getRequestEvent(), (*iTrip)->getDispatchEvent(), (*iTrip)->getPickupEvent()->lat, (*iTrip)->getPickupEvent()->lng, (*iTrip)->getDropoffEvent()->lat, (*iTrip)->getDropoffEvent()->lng, (*iTrip)->getPickupEvent()->timeT, (*iTrip)->getDropoffEvent()->timeT, (*iTrip)->getPickupEvent(), (*iTrip)->getDropoffEvent());
                pOpenTrip->setIsRiderInitPresent(false);    // rider NOT initially present              
                _initOpenTrips.insert(pOpenTrip);
                
                break;
            }
            
            // case 4: trip is en route (i.e. driver assignment made and is between pickup and dropoff)
            case TripData::ENROUTE :
            {
                int driverIndex = -1;
                int riderIndex = -1;
                Driver * pDriver = NULL;
                try {
                    pDriver = getDriverFromID((*iTrip)->getDriverID());
                } catch( ItemNotFoundException &ex ) {
                    std::cerr << "*** ItemNotFoundException thrown ***" << std::endl;
                    std::cerr << ex.what() << std::endl;
                }
                try {
                    Rider * pRider = getRiderFromID((*iTrip)->getRiderID());
                    riderIndex = pRider->getIndex();
                } catch ( ItemNotFoundException &ex ) {
                    std::cerr << "*** ItemNotFoundException thrown ***" << std::endl;
                    std::cerr << ex.what() << std::endl;
                }        
                OpenTrip * pOpenTrip = new OpenTrip((*iTrip)->getDriverID(), pDriver, driverIndex, (*iTrip)->getRiderID(), riderIndex, (*iTrip)->getUniqueTripID(), (*iTrip)->getRequestEvent(), (*iTrip)->getDispatchEvent(), (*iTrip)->getPickupEvent()->lat, (*iTrip)->getPickupEvent()->lng, (*iTrip)->getDropoffEvent()->lat, (*iTrip)->getDropoffEvent()->lng, (*iTrip)->getPickupEvent()->timeT, (*iTrip)->getDropoffEvent()->timeT, (*iTrip)->getPickupEvent(),(*iTrip)->getDropoffEvent());
                pOpenTrip->setIsRiderInitPresent(true);     // rider is initially present
                _initOpenTrips.insert(pOpenTrip);             
                
                break;
            }
            
            // case 5: other cases
            default :
                ; // nothing to do
            
        }
    }
    return true;
}

Rider * DataContainer::getRiderFromID(const std::string riderID) {
    std::map<const std::string, Rider*>::iterator riderItr = _idRiderMap.find(riderID);
    if( riderItr != _idRiderMap.end() ) {
        return riderItr->second;
    } else {
        throw new ItemNotFoundException("rider",riderID,"_idRiderMap");
    }
}

Driver * DataContainer::getDriverFromID(const std::string driverID) {
    std::map<const std::string, Driver*>::iterator driverItr = _idDriverMap.find(driverID);
    if( driverItr != _idDriverMap.end() ) {
        return driverItr->second;
    } else {
        throw new ItemNotFoundException("driver",driverID,"_idDriverMap");
    }
}