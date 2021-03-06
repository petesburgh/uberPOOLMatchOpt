/* 
 * File:   DataContainer.cpp
 * Author: jonpetersen
 * 
 * Created on March 18, 2015, 11:19 AM
 */

#include "DataContainer.hpp"
#include "TripData.hpp"

DataContainer::DataContainer(const std::string& inputCsvFile, const std::string& timelineStr, const double optInRate, const int simLengthInMin, const bool printDebugFiles, const bool printToScreen, const Geofence* geofence) : 
        _optInRate(optInRate), pGeofence(geofence) {
    _inputCsvFile = inputCsvFile;
    _timelineStr = timelineStr; 
    _timeline = Utility::convertDateTimeStringToTimeT(timelineStr);
    _printDebugFiles = printDebugFiles;
    _printToScreen = printToScreen;
    _simEndTime = _timeline + (time_t)(60*simLengthInMin);    
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
  
    const std::string filePath = _inputCsvFile;
    std::ifstream       file(filePath);
    
    // ensure file is opened
    if( file.is_open() == false ) {
        throw new FileNotFoundException(filePath);
    }
    
    CSVRow row;
    int rowIndex = 0;
    int beginTripAfterEndTrip = 0;
    while( file >> row ) {        
       size_t nCols = row.size();
       assert( nCols == 15 ); // ENSURE proper dimension of CSV 

       // ignore header row
       if( rowIndex > 0 ) {

           // define current trip
           TripData * currTrip = defineCurrentTripInfoFromCsvLine(row); 
           
           // data validation: ensure the trip begins before it ends (e.g. going under a tunnel may reset the time(s))
           if( currTrip->getPickupEvent()->timeT >= currTrip->getDropoffEvent()->timeT ) {
               beginTripAfterEndTrip++;
               continue;
           }
                      
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
           
           // check if the trip satisfies the geofence           
           if( pGeofence != NULL ) {
               const bool isGeofenceEligible = isEligForGeofence(currTrip, pGeofence);
               if( !isGeofenceEligible)                 
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
    
    if( beginTripAfterEndTrip > 0 ) {
        std::cout << "\t" << Utility::intToStr(beginTripAfterEndTrip) << " trips discarded due to negative trip duratio (begintrip >= endtrip)" << std::endl;
    }
    
    file.close();    
}

bool DataContainer::isEligForGeofence(TripData* pTrip, const Geofence* pGeofence) {
    
    assert(pGeofence != NULL);
    
    switch( pGeofence->getGeofenceType() ) {
        case Geofence::REQ_ONLY :
        {
            bool isEligForGeofence = Utility::isPointInPolygon(pGeofence->getLatitudes(), pGeofence->getLongitudes(), make_pair(pTrip->getRequestEvent()->lat, pTrip->getRequestEvent()->lng));            
            return isEligForGeofence;
        }
        case Geofence::ORIG_ONLY :
        {
            bool isEligForGeofence = Utility::isPointInPolygon(pGeofence->getLatitudes(), pGeofence->getLongitudes(), make_pair(pTrip->getPickupEvent()->lat, pTrip->getPickupEvent()->lng));
            return isEligForGeofence;
        }
        case Geofence::ENTIRE_TRIP :
        {
            bool isPickupInGeofence = Utility::isPointInPolygon(pGeofence->getLatitudes(), pGeofence->getLongitudes(), make_pair(pTrip->getPickupEvent()->lat,  pTrip->getPickupEvent()->lng));
            bool isDropInGeofence   = Utility::isPointInPolygon(pGeofence->getLatitudes(), pGeofence->getLongitudes(), make_pair(pTrip->getDropoffEvent()->lat, pTrip->getDropoffEvent()->lng));
            return (isPickupInGeofence && isDropInGeofence);
        }
        default :
        {
            std::cout << "\n\n*** ERROR: Geofence Type not defined ***\n" << std::endl;
            std::cout << "\texiting... " << std::endl; 
            exit(0);
            return false;
        }
    }
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
        pDriver->setCapacity(4); // for now assume capacity is 4
        
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
    
    // ensure optInRate is in [0,1]
    assert( (0.0 <= _optInRate) && (_optInRate <= 1.0) );

    // if no uberX requests are POOL requests then the test does not make sense
    if( _optInRate == 0 ) {
        std::cerr << "\n\n*** ERROR: no uberPOOL requests are given (0 percent) ***\n\n" << std::endl;
        std::cerr << "\tsystem exiting upon error... " << std::endl;
        exit(1);
        return false;
    }
    
    // if the pct is strictly between (0,1]
    else {
        int numPoolUsers = (int)round(_optInRate*_allUberXRiders.size());
       
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
                                
                OpenTrip * pOpenTrip = new OpenTrip((*iTrip)->getDriverID(), pDriver, driverIndex, (*iTrip)->getRiderID(), riderIndex, (*iTrip)->getUniqueTripID(), 
                                                    (*iTrip)->getRequestEvent(), (*iTrip)->getDispatchEvent(), (*iTrip)->getPickupEvent()->lat, (*iTrip)->getPickupEvent()->lng, 
                                                    (*iTrip)->getDropoffEvent()->lat, (*iTrip)->getDropoffEvent()->lng, (*iTrip)->getPickupEvent()->timeT, 
                                                    (*iTrip)->getDropoffEvent()->timeT, (*iTrip)->getPickupEvent(), (*iTrip)->getDropoffEvent(), -1); 
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
                OpenTrip * pOpenTrip = new OpenTrip((*iTrip)->getDriverID(), pDriver, driverIndex, (*iTrip)->getRiderID(), riderIndex, (*iTrip)->getUniqueTripID(), (*iTrip)->getRequestEvent(), (*iTrip)->getDispatchEvent(), (*iTrip)->getPickupEvent()->lat, (*iTrip)->getPickupEvent()->lng, (*iTrip)->getDropoffEvent()->lat, (*iTrip)->getDropoffEvent()->lng, (*iTrip)->getPickupEvent()->timeT, (*iTrip)->getDropoffEvent()->timeT, (*iTrip)->getPickupEvent(),(*iTrip)->getDropoffEvent(), -1);
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