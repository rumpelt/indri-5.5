#include "Passage.hpp"
#include <stdexcept>
#include <iostream>
#include <stdio.h>
void Passage::setTerms(std::vector<std::string> terms) {
  Passage::terms = terms;
}

void Passage::setPsgId(int psgId) {
  char buffer[10];
  sprintf(buffer,"%d",psgId);
  Passage::psgId = buffer;
}

void Passage::setDocId(unsigned long docId) {
  Passage::docId = docId;
}

void Passage::crtTermFreq() {
  Passage::termFreq.clear();
  for(std::vector<std::string>::iterator wordIt = Passage::terms.begin(); wordIt != Passage::terms.end(); ++wordIt) {
    Passage::termFreq[*wordIt]++;
  }
}

int Passage::freq(std::string term) {
  try {
    return Passage::termFreq.at(term);
  }
  catch (const std::out_of_range& oor) {
  }
  return 0;
}

int Passage::getPsgSz() {
  return Passage::terms.size();
}

void Passage::setScore(float score) {
  Passage::score = score;
}

float Passage::getScore() {
  return Passage::score;
}

unsigned long Passage::getDocId() {
  return Passage::docId;
}

void Passage::setTrecId(std::string trecId) {
  Passage::trecId = trecId;
}
std::string Passage::getTrecId() {
  return Passage::trecId;
}

std::vector<std::string> Passage::getTerms() {
  return Passage::terms;
}

void Passage::pushTerm(std::string term) {
  Passage::terms.push_back(term);
}

std::map<std::string, int> Passage::getTermFreq() {
  return Passage::termFreq;
}

std::map<std::string, float> Passage::getTfIdf() {
  return Passage::tfIdf;
}

std::string Passage::getPsgId() {
  return Passage::psgId;
}

void Passage::printPassage() {
  std::cout << std::endl;
  for(std::vector<std::string>::iterator vecIt = Passage::terms.begin(); vecIt != Passage::terms.end(); ++vecIt) {
    std::cout << *vecIt << " "; 
  }  
  std::cout <<std::endl;
}
