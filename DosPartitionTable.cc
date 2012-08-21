#include <blkid.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/wait.h>
#include <string.h>
#include <errno.h>
#include <sstream>
#include "Utility.hh"
#include "DosPartition.hh"
#include "DosPartitionTable.hh"

#define MAX_PARTITIONS 60

using std::stringstream;
using std::dec;
using std::hex;
using std::endl;

DosPartitionTable::DosPartitionTable()
{
}

DosPartitionTable::~DosPartitionTable()
{
  size_t i = 0;
  DosPartition *part = 0;
  
  while(i < _table.size())
  {
    part = (DosPartition *) _table.at(i);
    delete part;
    ++i;
  }
}

bool DosPartitionTable::read(const string &path)
{
  int fd = -1;
  blkid_probe probe = 0;
  blkid_partlist partlist = 0;
  blkid_parttable parttable = 0;
  string label;
  vector <Partition *> table;
  int i = 0;
  int j = 0;
  blkid_partition partition = 0;
  DosPartition *part = 0;
  bool rv = false;

  if((fd = open(path.c_str(),O_RDONLY)) == -1)
    goto bail;

  if((probe = blkid_new_probe()) == 0)
    goto bail;

  if(blkid_probe_set_device(probe,fd,0,0) == -1)
    goto bail;

  if((partlist = blkid_probe_get_partitions(probe)) == 0)
    goto bail;

  if((parttable = blkid_partlist_get_table(partlist)) == 0)
    goto bail;

  if((label = blkid_parttable_get_type(parttable)) != "dos")
    goto bail;

  j = blkid_partlist_numof_partitions(partlist);

  while(i < j && (partition = blkid_partlist_get_partition(partlist,i)) != 0)
  {
    part = new DosPartition();

    part->setNumber(blkid_partition_get_partno(partition));

    part->setStart(blkid_partition_get_start(partition));

    part->setEnd(blkid_partition_get_start(partition) + blkid_partition_get_size(partition) - 1);

    part->setSectors(blkid_partition_get_size(partition));

    part->setType(blkid_partition_get_type(partition));

    part->setActive((blkid_partition_get_flags(partition) == 0x80) ? true : false);

    table.push_back(part);

    ++i;
  }

  _label = label;

  _table = table;

  rv = true;

bail:

  if(fd != -1)
    close(fd);

  if(probe != 0)
    blkid_free_probe(probe);

  return rv;
}

bool DosPartitionTable::write(const string &path)
{
  stringstream cmd;
  size_t i = 0;
  DosPartition *part = 0;
  pid_t pid = -1;
  int status = 0;

  if(_table.empty())
    return false;

  cmd << "echo -n -e '";  

  while(i < _table.size())
  {
    part = (DosPartition *) _table.at(i);
    cmd << dec;
    cmd << part->getStart();
    cmd << " ";
    cmd << part->getSectors();
    cmd << " ";
    cmd << hex;
    cmd << "0x";
    cmd << ((unsigned int) part->getType());
    cmd << " ";
    cmd << (part->getActive() ? "*" : "-");
    cmd << "\\n";
    ++i;
  }

  cmd << "' | sfdisk --unit S --Linux " << path;

  logfile << "Executing command: " << cmd.str() << endl;

  if((pid == execute(cmd.str())) == -1 || waitpid(pid,&status,0) == -1)
  {
    logfile << __func__ << ": " << strerror(errno) << endl; 
    return false;
  }

  if(!WIFEXITED(status) || WEXITSTATUS(status) != 0)
  {
    logfile << "Command did not exit normally: " << cmd.str() << endl;
    return false;
  }

  logfile << "Finished executing command: " << cmd.str() << endl;

  return true;
}
