/* 
 * File:   ItemNotFoundException.hpp
 * Author: jonpetersen
 *
 * Created on March 25, 2015, 1:05 PM
 */

#ifndef ITEMNOTFOUNDEXCEPTION_HPP
#define	ITEMNOTFOUNDEXCEPTION_HPP

#include <iostream>
#include <exception>
using namespace std;

class ItemNotFoundException : public exception {
public:
    ItemNotFoundException(const std::string item, const std::string id, const std::string mapname) : _item(item), _stringId(id), _mapName(mapname){};
    ItemNotFoundException(const ItemNotFoundException& orig);
    ~ItemNotFoundException() throw() {};
    
    virtual const char * what() const throw() {
        std::string errorStr = "";
        errorStr += _item + " ID " + _stringId + " not found in map `" + _mapName + "'";
        return errorStr.c_str();        
    }
    
private:
    
    const std::string _item;
    const std::string _stringId;
    const std::string _mapName;

};

#endif	/* ITEMNOTFOUNDEXCEPTION_HPP */

