#include "Passage.hpp"
#include <stdexcept>
#include <iostream>
#include <stdio.h>

Passage::Passage() {}
Passage::Passage(int psgSize, int windowSize): passageSize(psgSize), windowSize(windowSize) {
}

void Passage::setWindowSize(int psgSize, int windowSize) {
  Passage::windowSize = windowSize;
  Passage::passageSize = psgSize;
}

void Passage::setTerms(std::vector<std::string> terms) {
  Passage::terms = terms;
}

void Passage::addTerms(std::vector<std::string>&  terms) {
  for(std::vector<std::string>::iterator termIt = terms.begin(); termIt != terms.end(); termIt++) {
    Passage::terms.push_back(*termIt);
  }
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
void Passage::printPassage() {
  for(std::vector<std::string>::iterator tkIt = (Passage::terms).begin(); tkIt != (Passage::terms).end(); ++tkIt) {
    std::cout <<" " << *tkIt;
  }
}

std::map<std::string, float> Passage::getTfIdf() {
  return Passage::tfIdf;
}

std::string Passage::getPsgId() {
  return Passage::psgId;
}

