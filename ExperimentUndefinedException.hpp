/* 
 * File:   ExperimentUndefinedException.hpp
 * Author: jonpetersen
 *
 * Created on June 2, 2015, 1:25 PM
 */

#ifndef EXPERIMENTUNDEFINEDEXCEPTION_HPP
#define	EXPERIMENTUNDEFINEDEXCEPTION_HPP

#include <iostream>
#include <exception>
#include <set>

using namespace std;

class ExperimentUndefinedException : public exception {
    public:
        ExperimentUndefinedException(std::string value, std::set<std::string> vals) : _stringValue(value), _eligValues(vals) {};
        ~ExperimentUndefinedException() throw() {};
        
        virtual const char * what() const throw() {
            std::string errorStr = "experiment '";
            errorStr += _stringValue;
            errorStr += "' not found  (should be {";            
            int counter = 0;
            for( std::set<std::string>::iterator strItr = _eligValues.begin(); strItr != _eligValues.end(); ++strItr ) {
                if( counter > 0 ) {
                    errorStr += "," + *strItr;
                } else {
                    errorStr += *strItr;
                }
            }
            errorStr += "}";
            
            return errorStr.c_str();
        }
        
    private:            
        const std::string _stringValue;
        const std::set<std::string> _eligValues;
};

#endif	/* EXPERIMENTUNDEFINEDEXCEPTION_HPP */

