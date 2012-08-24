#include <stdlib.h>
#include "UserInterface.hh"
#include "Utility.hh"

int main(int argc,char **argv)
{
  if(!ui.initialize(argc,argv))  
    return false;

  return EXIT_SUCCESS;
}
