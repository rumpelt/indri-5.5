#ifndef KLDIVERGENCEMODEL_HPP
#define KLDIVERGENCEMODEL_HPP
#include "Model.hpp"
#include "Distribution.hpp"
#include "QueryThread.hpp"

class KLDivergenceModel : public Model {
private:
public:
  float score(query_t& query, Distribution& psg);
};
#endif
