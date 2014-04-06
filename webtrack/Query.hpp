#ifndef QUERY_HPP
#define QUERY_HPP

struct Query {
  std::string qnum;
  std::string query; // the actual query, for subqueries this might be empty
  std::string queryType;
  std::string description; // the actual description of the query
  std::vector<std::string> tokens;
  std::map<std::string, int> termFreq;
  std::vector<Query*> subquery;
  //  ~Query() {for_each(subquery.begin(), subquery.end(), delete)}
};
#endif
