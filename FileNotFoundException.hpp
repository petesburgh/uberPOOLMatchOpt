/* 
 * File:   FileNotFoundException.hpp
 * Author: jonpetersen
 *
 * Created on May 28, 2015, 4:48 PM
 */

#ifndef FILENOTFOUNDEXCEPTION_HPP
#define	FILENOTFOUNDEXCEPTION_HPP

#include <iostream>
#include <exception>

using namespace std;

class FileNotFoundException : public exception {
public:
    FileNotFoundException(std::string inputFilePath) : _inputFilePath(inputFilePath) {};
    ~FileNotFoundException() throw() {};
    
    virtual const char * what() const throw() {
        std::string errorStr = "file";
        errorStr += _inputFilePath;
        errorStr += " not found";
        return errorStr.c_str();
    }
    
private:
    const std::string _inputFilePath;
};

#endif	/* FILENOTFOUNDEXCEPTION_HPP */

