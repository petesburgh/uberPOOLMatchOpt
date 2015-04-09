/* 
 * File:   CSVRow.cpp
 * Author: jonpetersen
 * 
 * Created on March 18, 2015, 2:00 PM
 */

#include "CSVRow.hpp"

CSVRow::CSVRow() {
}

CSVRow::CSVRow(const CSVRow& orig) {
}

CSVRow::~CSVRow() {
}


void CSVRow::readNextRow(std::istream& str) {
    std::string         line;
    std::getline(str,line,'\n');
   
    std::stringstream   lineStream(line);
    std::string         cell;

    m_data.clear();
    while(std::getline(lineStream,cell,','))
    {
        m_data.push_back(cell);
    }
}