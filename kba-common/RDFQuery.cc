#include "RDFQuery.hpp"

std::vector< boost::shared_ptr<unsigned char> > RDFQuery::getTargetNodes(const unsigned char* subject, const unsigned char* predicate) {
  std::vector< boost::shared_ptr<unsigned char > > targets;
  librdf_node* subjectNode  = librdf_new_node_from_uri_string(RDFQuery::_world, subject);
  librdf_node* predicateNode  = librdf_new_node_from_uri_string(RDFQuery::_world, predicate);
  
  if(predicateNode != NULL && subjectNode != NULL) {
    librdf_iterator* iterator = librdf_model_get_targets(RDFQuery::_model, subjectNode, predicateNode);
    if(iterator != NULL) {  
      while(!librdf_iterator_end(iterator)) {
        librdf_node* node = (librdf_node*) librdf_iterator_get_object(iterator); // returns a shared object pointer and so we do not need to free this.
        if(librdf_node_is_resource(node)) {
          librdf_uri* uri= librdf_node_get_uri(node); // returns a share pointer and so do not worry about freeing it.
          unsigned char* uriString = librdf_uri_to_string(uri);   // the string returned by this is malloced and must be freed and thats why we are using shared_ptr
	  boost::shared_ptr<unsigned char> uriPtr(uriString);
          targets.push_back(uriPtr);
	}
        else if(librdf_node_is_literal(node)) {
          size_t lengthOfLiteral;
          unsigned char* literalVal = librdf_node_get_literal_value_as_counted_string(node, &lengthOfLiteral); // this returns a pointer which must be copied for use.
          unsigned char* destination = (unsigned char*) malloc(lengthOfLiteral+1); // plus 1 for the null character
          memcpy(destination, (const unsigned char* )literalVal, lengthOfLiteral); 
          *(destination+lengthOfLiteral) = '\0'; 
          boost::shared_ptr<unsigned char> uriPtr(destination);
          targets.push_back(uriPtr);
	} 
        librdf_iterator_next(iterator);
      }
      librdf_free_iterator(iterator);
    }

  }

  if(subjectNode != NULL)
    librdf_free_node(subjectNode);
  if(predicateNode != NULL)
    librdf_free_node(predicateNode);
  return targets;
}

/**
 * This function was tested
 */

std::vector< boost::shared_ptr<unsigned char> > RDFQuery::getSourceNodes(const unsigned char* predicate, const unsigned char* object, bool isObjectLiteral) {
  std::vector< boost::shared_ptr<unsigned char > > sources;
  librdf_node* predicateNode  = librdf_new_node_from_uri_string(RDFQuery::_world, predicate);
  librdf_node* objectNode;

  
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



RDFQuery::RDFQuery(librdf_model* model, librdf_world* world): _model(model), _world(world) {
  
}
