/* 
 * File:   Utility.hpp
 * Author: jonpetersen
 *
 * Created on March 18, 2015, 3:30 PM
 */

#ifndef UTILITY_HPP
#define	UTILITY_HPP

#include "TripData.hpp"
#include "LatLng.hpp"
#include "TimeAdjacencyException.hpp" /* ensures current time is located in given event adjacency */

#include <iostream>
#include <iomanip>
#include <sstream>
#include <time.h>
#include <math.h>
#include <sys/stat.h>   /* mkdir */

class Event;

using namespace std;

class Utility {
public:
    static std::string intToStr(int x) {   
        std::stringstream ss;
        ss << x;
        return ss.str(); 
    }
    static std::string doubleToStr(double x) {   
        std::stringstream ss;
        ss << std::setprecision(15) << x;
        return ss.str(); 
    }
    /*
     * given 'dateTimeStr' of the form '%m/%d/%y', convert to time_t object where
     * time_t object represents seconds since midnight on Jan 1, 1970 (UNIX time)
     */
    static time_t convertDateTimeStringToTimeT(std::string dateTimeStr) {
        const char * initTimeString = dateTimeStr.c_str();
        struct tm structTm;
        strptime(initTimeString, "%Y-%m-%d %H:%M:%S", &structTm);       
        return timegm(&structTm);
    }
    
    // convert time_t object to reader-friendly string for printing
    static std::string convertTimeTToString(time_t timeT) {
        struct tm * structTime;
        structTime = gmtime(&timeT);
        std::string timeStr = "";
        timeStr += Utility::intToStr(structTime->tm_year + 1900);
        timeStr += "-";
        timeStr += Utility::convertDateIntToTwoDigitString(structTime->tm_mon+1);
        timeStr += "-";
        timeStr += Utility::convertDateIntToTwoDigitString(structTime->tm_mday);
        timeStr += "-";
        timeStr += Utility::convertDateIntToTwoDigitString(structTime->tm_hour);
        timeStr += ":";
        timeStr += Utility::convertDateIntToTwoDigitString(structTime->tm_min);
        timeStr += ":";
        timeStr += Utility::convertDateIntToTwoDigitString(structTime->tm_sec);
        return timeStr;   
    }
    
    static std::string convertDateIntToTwoDigitString(int tm) {
        std::string tmStr = Utility::intToStr(tm);
        if( tmStr.length() == 1 ) {
            tmStr.insert(0,"0");
        }
        return tmStr;
    }
    
    static LatLng estLocationByLinearProxy(const time_t &currTime, const time_t &earlierTime, const double &earlierLat, const double &earlierLng, const time_t &laterTime, const double &laterLat, const double &laterLng) {
        double estLat = -9999.0;
        double estLng = -9999.0;

        // step 1: ensure the currTime is between the times associated with each adjacent event
        if( (currTime < earlierTime) || (currTime > laterTime) ) {
            throw new TimeAdjacencyException(currTime, earlierTime, laterTime);
        } 
                        
        double p = (double)(laterTime - currTime)/(double)(laterTime - earlierTime);
        estLat = p*earlierLat + (1-p)*laterLat;
        estLng = p*earlierLng + (1-p)*laterLng;
        LatLng estLoc(estLat,estLng);
        return estLoc;
    }
    
    static double computeGreatCircleDistance(const double lat1, const double lng1, const double lat2, const double lng2) {
        // Sample format for latitide and longitudes
        // double lat1=45.54243333333333,lat2=45.53722222,long1=-122.96045277777778,long2=-122.9630556;
        // Below is the main code
      
        double PI = 4.0*atan(1.0);
        
        //main code inside the class
        double dlat1=lat1*(PI/180);

        double dlong1=lng1*(PI/180);
        double dlat2=lat2*(PI/180);
        double dlong2=lng2*(PI/180);

        double dLong=dlong1-dlong2;
        double dLat=dlat1-dlat2;

        double aHarv= pow(sin(dLat/2.0),2.0)+cos(dlat1)*cos(dlat2)*pow(sin(dLong/2),2);
        double cHarv=2*atan2(sqrt(aHarv),sqrt(1.0-aHarv));
        //earth's radius from wikipedia varies between 6,356.750 km — 6,378.135 km (˜3,949.901 — 3,963.189 miles)
        //The IUGG value for the equatorial radius of the Earth is 6378.137 km (3963.19 mile)
        const double earth=6378.135;//I am doing miles, just change this to radius in kilometers to get distances in km
        double distance=earth*cHarv;  
        
        return distance;
    }
    
    static double computeMean(std::vector<double> &doubleVec) {
        if( doubleVec.empty() ) 
            return -9999.99;
        
        double sum = 0.0;
        for( std::vector<double>::iterator itr = doubleVec.begin(); itr != doubleVec.end(); ++itr ) {
            sum += *itr;
        }
        
        const double avg = sum/(double)doubleVec.size();
        return avg;
    }
    
    static std::string truncateDouble(double val, int trunc) {
        
        std::stringstream ss;
        ss << std::setprecision(trunc) << std::fixed << val;
        return ss.str();            
    }
    
    static std::string convertToLatLngStr(LatLng &loc, int prec) {
        std::string locStr = "";
        locStr += Utility::truncateDouble(loc.getLat(), prec);
        locStr += ",";
        locStr += Utility::truncateDouble(loc.getLng(), prec);
        return locStr;
    }
    
    static std::string convertToLatLngStr(const LatLng &loc, int prec) {
        std::string locStr = "";
        locStr += Utility::truncateDouble(loc.getLat(), prec);
        locStr += ",";
        locStr += Utility::truncateDouble(loc.getLng(), prec);
        return locStr;
    }    
    
    static int createFolder(const std::string outPath) {
        int status_scen = mkdir(outPath.c_str(), S_IRWXU | S_IRWXG | S_IROTH);
        return status_scen;
    }
    
private:
};

#endif	/* UTILITY_HPP */

