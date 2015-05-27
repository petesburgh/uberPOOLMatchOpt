/* 
 * File:   RiderCountException.hpp
 * Author: jonpetersen
 *
 * Created on May 21, 2015, 3:54 PM
 */

#ifndef RIDERCOUNTEXCEPTION_HPP
#define	RIDERCOUNTEXCEPTION_HPP

#include "AssignedRoute.hpp"
#include "Utility.hpp"
#include <iostream>
#include <exception>

using namespace std;

class RiderCountException : public exception {
public:
    RiderCountException(AssignedRoute * rt, int maxAllowed) : pRoute(rt), _maxAllowable(maxAllowed) {};
    ~RiderCountException() throw() {};
    
    virtual const char * what() const throw() {
        std::string errorStr = "";
        errorStr += "route " + Utility::intToStr(pRoute->getIndex()) + " is assigned to " + 
                Utility::intToStr(pRoute->getNumRidersInRoute()) + " riders (max allowed is " + 
                Utility::intToStr(_maxAllowable) + ")";        
        return errorStr.c_str();
    }
    
        
private:
    const AssignedRoute * pRoute;
    const int _maxAllowable;
};


#endif	/* RIDERCOUNTEXCEPTION_HPP */

