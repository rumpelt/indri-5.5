#include "TextMatrix.hpp"
#include "assert.h"
#include <stdexcept>
#include <iostream>

TextMatrix::TextMatrix() : _matrix(0) {}
TextMatrix::~TextMatrix() { if(TextMatrix::_matrix != 0) delete(TextMatrix::_matrix); }
 
void TextMatrix::initializeMatrix(std::vector<std::string>& rowName, std::vector<std::string>& colName) {
  std::set<std::string> unique;
  for(std::vector<std::string>::iterator rowIt = rowName.begin(); rowIt != rowName.end(); ++rowIt) {
    std::string rname = *rowIt;
    if(unique.find(rname) != unique.end()) {
      assert(false);
    }
    TextMatrix::rowId.push_back(rname);
    unique.insert(rname);
  }
  unique.clear();
  for(std::vector<std::string>::iterator colIt = colName.begin(); colIt != colName.end(); ++colIt) {
    std::string cname = *colIt;
    if(unique.find(cname) != unique.end()) {
      assert(false);
    }
    TextMatrix::colId.push_back(cname);
    unique.insert(cname);
  }
  TextMatrix::_matrix = new Eigen::MatrixXf(rowName.size(),colName.size());    
}


void TextMatrix::initializeMatrix(std::set<std::string>& rowName, std::set<std::string>& colName, const float& initValue) {
   
  int  rindex=0;
  for(std::set<std::string>::iterator rowIt = rowName.begin(); rowIt != rowName.end(); ++rowIt, rindex++) {
    std::string rname = *rowIt;
    TextMatrix::rowIdxMap[rname] = rindex;
  }

  int cindex=0;
  for(std::set<std::string>::iterator colIt = colName.begin(); colIt != colName.end(); ++colIt, cindex++) {
    std::string cname = *colIt;
    TextMatrix::colIdxMap[cname] = cindex;
  }
  TextMatrix::_matrix = new Eigen::MatrixXf(rowIdxMap.size(),colIdxMap.size());
  TextMatrix::_matrix->fill(initValue);
}

Eigen::MatrixXf* TextMatrix::getMatrix() {
  return TextMatrix::_matrix;
}
/**
 * THIS IWILL THROW ERROR if there are no rwoName
 */
void TextMatrix::setValue(std::string& rowName, std::string& colName, float value) {
  int rIdx = TextMatrix::rowIdxMap.at(rowName);
  int cIdx = TextMatrix::colIdxMap.at(colName);
  (*(TextMatrix::_matrix))(rIdx, cIdx) = value;
}


void TextMatrix::initializeMatrix(std::set<std::string>& rowName, std::string& colName, const float& initValue) {
   
  int  rindex=0;
  for(std::set<std::string>::iterator rowIt = rowName.begin(); rowIt != rowName.end(); ++rowIt, rindex++) {
    std::string rname = *rowIt;
    TextMatrix::rowIdxMap[rname] = rindex;
  }
  TextMatrix::colIdxMap[colName] = 0;
  TextMatrix::_matrix = new Eigen::MatrixXf(rowIdxMap.size(),1);
  TextMatrix::_matrix->fill(initValue);
}


size_t TextMatrix::getNumRows() {
  return TextMatrix::rowIdxMap.size();
}


int TextMatrix::rIndex(std::string& term) {
  int idx = -1;
  try {
    idx = TextMatrix::rowIdxMap.at(term);
  } catch(std::out_of_range& oor) {
    assert(false);
  }
  return idx;
}

int TextMatrix::cIndex(std::string& term) {
  int idx = -1;
  try {
    idx = TextMatrix::colIdxMap.at(term);
  } catch(std::out_of_range& oor) {
    assert(false);
  }
  return idx;
}
