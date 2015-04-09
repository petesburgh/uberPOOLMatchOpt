/* 
 * File:   CSVRow.hpp
 * Author: jonpetersen
 *
 * Created on March 18, 2015, 2:00 PM
 */

#ifndef CSVROW_HPP
#define	CSVROW_HPP

#include<iostream>
#include<vector>
#include<sstream>
#include<string>
#include<fstream>

using namespace std;

class CSVRow {
public:
    CSVRow();
    CSVRow(const CSVRow& orig);
    virtual ~CSVRow();
    
    std::string const& operator[](std::size_t index) const {
        return m_data[index];
    }
    std::size_t size() const {
        return m_data.size();
    }
    void readNextRow(std::istream& str);
    
    const std::vector<std::string>* getData() const { return &m_data; }
    
private:
    
    std::vector<std::string> m_data;

};

#endif	/* CSVROW_HPP */

