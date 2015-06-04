/* 
 * File:   EnumException.hpp
 * Author: jonpetersen
 *
 * Created on June 4, 2015, 10:38 AM
 */

#ifndef ENUMEXCEPTION_HPP
#define	ENUMEXCEPTION_HPP

#include <iostream>
#include <exception>
#include <set>

using namespace std;

class EnumException : public exception {
public:
    EnumException(std::string val, std::set<std::string> expectedVals) : _value(val), _expValues(expectedVals) {};
    ~EnumException() throw() {};
    
    virtual const char * what() const throw() {
        std::string errorStr = "value '";
        errorStr += _value;
        errorStr += "' not found... exp values ";
        int counter = 0;
        for( std::set<std::string>::iterator itr = _expValues.begin(); itr != _expValues.end(); ++itr ) {
            if( counter == 0 ) {
                errorStr += " ( ";
            } else {
                errorStr += ", ";
            }
            errorStr += *itr;
            counter++;
        }
        errorStr += " )";

        return errorStr.c_str();        
    }
    
private:
    const std::string _value;
    const std::set<std::string> _expValues;
};

#endif	/* ENUMEXCEPTION_HPP */

