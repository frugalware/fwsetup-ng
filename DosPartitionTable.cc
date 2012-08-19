#include <blkid.h>
#include <fcntl.h>
#include <unistd.h>
#include "DosPartition.hh"
#include "DosPartitionTable.hh"

bool DosPartitionTable::read(const string &path)
{
  int fd = -1;
  blkid_probe probe = 0;
  blkid_partlist partlist = 0;
  blkid_parttable parttable = 0;
  string label;
  vector <Partition> table;
  int i = 0;
  int j = 0;
  blkid_partition partition = 0;
  DosPartition part;
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
    part.setNumber(blkid_partition_get_partno(partition));
    
    part.setStart(blkid_partition_get_start(partition));
    
    part.setEnd(blkid_partition_get_start(partition) + blkid_partition_get_size(partition) - 1);
    
    part.setSectors(blkid_partition_get_size(partition));
    
    part.setType(blkid_partition_get_type(partition));
    
    part.setActive((blkid_partition_get_flags(partition) == 0x80) ? true : false);
    
    table[i++] = part;
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
  return true;
}
