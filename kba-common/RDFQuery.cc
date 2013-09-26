#include "RDFQuery.hpp"

std::vector< boost::shared_ptr<unsigned char> > RDFQuery::getSourceNodes(const unsigned char* predicate, const unsigned char* object, bool isObjectLiteral) {
  std::vector< boost::shared_ptr<unsigned char > > sources;
  librdf_node* predicateNode  = librdf_new_node_from_uri_string(RDFQuery::_world, predicate);
  librdf_node* objectNode;

  /**
   std::cout <<"printing model : "<< librdf_model_sync(RDFQuery::_model) << " : " << librdf_model_size(RDFQuery::_model)<<"\n";  
  raptor_iostream* iostr;
  raptor_world *raptor_world_ptr;
  raptor_world_ptr = librdf_world_get_raptor(RDFQuery::_world);
  iostr = raptor_new_iostream_to_file_handle(raptor_world_ptr, stdout);
  librdf_model_write(RDFQuery::_model, iostr);
  raptor_free_iostream(iostr);
  */

  if(isObjectLiteral)
    objectNode = librdf_new_node_from_literal(RDFQuery::_world, object, NULL, 0);
  else 
    objectNode = librdf_new_node_from_uri_string(RDFQuery::_world, object);
  
  if(predicateNode != NULL && objectNode != NULL) {
    librdf_iterator* iterator = librdf_model_get_sources(RDFQuery::_model, predicateNode, objectNode);
    if(iterator != NULL) {  
      while(!librdf_iterator_end(iterator)) {
        librdf_node* node = (librdf_node*) librdf_iterator_get_object(iterator); // returns a shared object pointer and so we do not need to free this.
        if(librdf_node_is_resource(node)) {
          librdf_uri* uri= librdf_node_get_uri(node); // returns a share pointer and so do not worry about freeing it.
          unsigned char* uriString = librdf_uri_to_string(uri);   // the string returned by this is malloced and must be freed and thats why we are using shared_ptr
	  boost::shared_ptr<unsigned char> uriPtr(uriString);
          sources.push_back(uriPtr);
	} 
        librdf_iterator_next(iterator);
      }
      librdf_free_iterator(iterator);
    }

  }

  if(objectNode != NULL)
    librdf_free_node(objectNode);
  if(predicateNode != NULL)
    librdf_free_node(predicateNode);
  return sources;
}



RDFQuery::RDFQuery(librdf_model* model, librdf_world* world) {
  _model = model;
  _world = world;
}
