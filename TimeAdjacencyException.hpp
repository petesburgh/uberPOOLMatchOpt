/* 
 * File:   TimeAdjacencyException.hpp
 * Author: jonpetersen
 *
 * Created on March 26, 2015, 9:11 AM
 */

#ifndef TIMEADJACENCYEXCEPTION_HPP
#define	TIMEADJACENCYEXCEPTION_HPP

#include <iostream>
#include <exception>
#include <time.h>

using namespace std;

class TimeAdjacencyException : public exception {
    public:
        //
        TimeAdjacencyException(time_t currentTime, time_t earlierTime, time_t laterTime) : _currentTime(currentTime), _earlierTime(earlierTime), _laterTime(laterTime) {};
        TimeAdjacencyException(const TimeAdjacencyException& orig);
        ~TimeAdjacencyException() throw() {};
        
        virtual const char * what() const throw() {
            std::string errStr = "current time not contained in interval defined by adjacent events";
            return errStr.c_str();
        }
    private:
        
        const time_t _currentTime;
        const time_t _earlierTime;
        const time_t _laterTime;
        
};


#endif	/* TIMEADJACENCYEXCEPTION_HPP */

