#include <stdlib.h>
#include <unistd.h>
#include <locale.h>
#include "UserInterface.hh"
#include "Utility.hh"
#include "PartitionModule.hh"

static Module edge_guard_module;

static Module modules[] =
{
  edge_guard_module,
  partition_module,
  edge_guard_module
};

int main(int argc,char **argv)
{
  Module *module = &modules[1];

  if(geteuid() != 0)
    return EXIT_FAILURE;

  if(setlocale(LC_ALL,"") == 0)
    return EXIT_FAILURE;

  if(!ui.initialize(argc,argv))
    return EXIT_FAILURE;

  while(module->getName() != "")
  {
    int i = module->run();
    
    if(i < 0)
      --module;
    else if(i > 0)
      ++module;
    else if(i == 0)
      break;
  }

  return EXIT_SUCCESS;
}
