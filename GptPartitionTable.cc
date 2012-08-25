#include <blkid.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <sys/wait.h>
#include <errno.h>
#include <limits.h>
#include <sstream>
#include "Utility.hh"
#include "GptPartition.hh"
#include "GptPartitionTable.hh"

#define MAX_PARTITIONS 128

using std::stringstream;
using std::dec;
using std::hex;
using std::endl;

static string get_gpt_label_uuid(const string &path)
{
  string cmd;
  FILE *in = 0;
  char line[LINE_MAX] = {0};
  char *p = 0;

  cmd = "sgdisk -p " + path + " 2> /dev/null | sed -rn 's|^Disk identifier \\(GUID\\): ([0-9a-zA-Z-]+)$|\\1|p' 2> /dev/null";

  if((in = popen(cmd.c_str(),"r")) == 0)
    return "";

  if(fgets(line,sizeof(line),in) == 0)
  {
    pclose(in);
    return "";
  }

  if(pclose(in) == -1)
    return "";

  p = strchr(line,'\n');

  if(p != 0)
    *p = 0;

  return line;
}

GptPartitionTable::GptPartitionTable()
{
}

GptPartitionTable::~GptPartitionTable()
{
  size_t i = 0;
  GptPartition *part = 0;
  
  while(i < _table.size())
  {
    part = (GptPartition *) _table.at(i);
    delete part;
    ++i;
  }
}

bool GptPartitionTable::read(const string &path)
{
  int fd = -1;
  blkid_probe probe = 0;
  blkid_partlist partlist = 0;
  blkid_parttable parttable = 0;
  string label;
  string uuid;
  vector <Partition *> table;
  int i = 0;
  int j = 0;
  blkid_partition partition = 0;
  GptPartition *part = 0;
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

  if((label = blkid_parttable_get_type(parttable)) != "gpt")
    goto bail;

  if((uuid = get_gpt_label_uuid(path)) == "")
    goto bail;

  j = blkid_partlist_numof_partitions(partlist);

  while(i < j && (partition = blkid_partlist_get_partition(partlist,i)) != 0)
  {
    part = new GptPartition();

    part->setNumber(blkid_partition_get_partno(partition));

    part->setStart(blkid_partition_get_start(partition));

    part->setEnd(blkid_partition_get_start(partition) + blkid_partition_get_size(partition) - 1);

    part->setSectors(blkid_partition_get_size(partition));

    part->setType(blkid_partition_get_type_string(partition));

    part->setName(blkid_partition_get_name(partition));

    part->setUUID(blkid_partition_get_uuid(partition));

    part->setFlags(blkid_partition_get_flags(partition));

    table.push_back(part);

    ++i;
  }

  _label = label;

  _uuid = uuid;

  _table = table;

  rv = true;

bail:

  if(fd != -1)
    close(fd);

  if(probe != 0)
    blkid_free_probe(probe);

  return rv;
}

bool GptPartitionTable::write(const string &path)
{
  stringstream cmd;
  size_t i = 0;
  GptPartition *part = 0;
  pid_t pid = -1;
  int status = 0;

  if(_table.empty())
    return false;
  
  cmd << "sgdisk --clear --disk-guid='" << (_uuid.empty() ? "R" : _uuid.c_str()) << "'";

  while(i < _table.size())
  {
    part = (GptPartition *) _table.at(i);

    cmd << dec;

    cmd << " --new=" << part->getNumber() << ":" << part->getStart() << ":" << part->getEnd();
    
    cmd << " --change-name=" << part->getNumber() << ":'" << part->getName() << "'";
    
    cmd << " --partition-guid=" << part->getNumber() << ":'" << (part->getUUID().empty() ? "R" : part->getUUID().c_str()) << "'";
    
    cmd << " --typecode=" << part->getNumber() << ":'" << part->getType() << "'";   

    cmd << " --attributes=" << part->getNumber() << ":=:0x" << hex << part->getFlags();

    ++i;
  }

  cmd << " " << path;

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

unsigned long long GptPartitionTable::getNumber(unsigned long long n)
{
  GptPartition *part = 0;

  if(n < _table.size())
  {
    part = (GptPartition *) _table.at(n);
    
    return part->getNumber();
  }
  
  return 0;
}

unsigned long long GptPartitionTable::getStart(unsigned long long n)
{
  GptPartition *part = 0;

  if(n < _table.size())
  {
    part = (GptPartition *) _table.at(n);
    
    return part->getStart();
  }
  
  return 0;
}

unsigned long long GptPartitionTable::getEnd(unsigned long long n)
{
  GptPartition *part = 0;

  if(n < _table.size())
  {
    part = (GptPartition *) _table.at(n);
    
    return part->getEnd();
  }
  
  return 0;
}

unsigned long long GptPartitionTable::getSectors(unsigned long long n)
{
  GptPartition *part = 0;

  if(n < _table.size())
  {
    part = (GptPartition *) _table.at(n);
    
    return part->getSectors();
  }
  
  return 0;
}

bool GptPartitionTable::getActive(unsigned long long n)
{
  GptPartition *part = 0;

  if(n < _table.size())
  {
    part = (GptPartition *) _table.at(n);
    
    return part->getActive();
  }
  
  return false;
}

// -%- strip: yes; add-newline: yes; use-tabs: no; indent-width: 2; tab-width: 2; -%-
