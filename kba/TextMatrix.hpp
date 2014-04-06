#ifndef TEXTMATRIX_HPP
#define TEXTMATRIX_HPP
#include <Eigen/Dense>
#include <vector>
#include <map>
#include <string>
#include <set>

class TextMatrix {  
private:
  std::vector<std::string> rowId;
  std::vector<std::string> colId;
  std::map<int, std::string> idxRMap; // map from index to row names
  std::map<std::string, int> rowIdxMap; // map from rowname to to index.
  std::map<int, std::string> idxCMap; // map from index to row names
  std::map<std::string, int> colIdxMap; // map from col name to index;

  bool squarMatrix; // true if row index names and col Index names are same.
  Eigen::MatrixXf* _matrix; // Matrix  q
public:
  /**
   */
  void initializeMatrix(std::vector<std::string> rowName, std::vector<std::string> colNames);
  void initializeMatrix(std::set<std::string> rowName, std::set<std::string> colNames, const float initValue);
  void initializeMatrix(std::set<std::string> rowName, std::string colName, const float initValue);
  void setValue(std::string rowName, std::string colName, float value);
  int rIndex(std::string rowName);
  int cIndex(std::string colName);
  size_t getNumRows();
  Eigen::MatrixXf* getMatrix();
  TextMatrix();
  ~TextMatrix();
};
#endif
