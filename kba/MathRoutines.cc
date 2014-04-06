#include "MathRoutines.hpp"

unsigned long math::factorial(unsigned long number) {
  if ( number == 0 ) 
    return 1;
  return(number * factorial(number - 1));
}
  
double math::logFactorial(unsigned long number) {
  if ( number == 0 ) 
    return 1;
  return(log(number) + logFactorial(number - 1));
}
