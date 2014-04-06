#ifndef QUERY_HPP
#define QUERY_HPP

struct Query {
  std::string qnum; \\ this must be unique for each query
  std::string query; // the actual query, for subqueries this might be empty
  std::string queryType;
  std::string group;
  std::string description; // the actual description of the query
  std::vector<std::string> tokens;
  std::map<std::string, int> termFreq;

};
#endif
