#include <stdlib.h>
#include <unistd.h>
#include <locale.h>
#include "UserInterface.hh"
#include "Utility.hh"

int main(int argc,char **argv)
{
  if(geteuid() != 0)
    return EXIT_FAILURE;

  if(setlocale(LC_ALL,"") == 0)
    return EXIT_FAILURE;

  if(!ui.initialize(argc,argv))
    return EXIT_FAILURE;

  return EXIT_SUCCESS;
}
