#include <string>
#include <vector>
#include <boost/shared_ptr.hpp>
#ifdef _cplusplus
extern "C" { 
#endif
  #include <redland.h>
#ifdef _cplusplus
}
#endif

class RDFQuery {
private:
  librdf_model* _model;
  librdf_world* _world;
public:
  void setModel(librdf_model* model);
  void setWorld(librdf_model* model);

  std::vector< boost::shared_ptr<unsigned char> > getSourceNodes(const unsigned char* predicate, const unsigned char* object, bool isObjectLiteral=true);
 
  RDFQuery(librdf_model* model, librdf_world* world);
};
