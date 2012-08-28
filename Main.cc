#include <stdlib.h>
#include <unistd.h>
#include <locale.h>
#include <string.h>
#include <errno.h>
#include "UserInterface.hh"
#include "Utility.hh"
#include "PartitionModule.hh"

using std::endl;

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
  {
    logfile << argv[0] << " must be run as root." << endl;
    return EXIT_FAILURE;
  }

  if(setlocale(LC_ALL,"") == 0)
  {
    logfile << "setlocale: " << strerror(errno) << endl;
    return EXIT_FAILURE;
  }

  if(!ui.initialize(argc,argv))
  {
    logfile << "User Interface failed to initialize." << endl;
    return EXIT_FAILURE;
  }

  while(module->getName() != "")
  {
    logfile << "Starting module '" << module->getName() << "'." << endl;
    
    int i = module->run();
    
    if(i < 0)
    {
      logfile << "Going back a module." << endl;
      --module;
    }
    else if(i > 0)
    {
      logfile << "Going forward a module." << endl;
      ++module;
    }
    else if(i == 0)
    {
      logfile << "Module '" << module->getName() << "' reported an error." << endl;
      break;
    }
  }

  return EXIT_SUCCESS;
}
