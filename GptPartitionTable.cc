#include <blkid.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <limits.h>
#include "GptPartition.hh"
#include "GptPartitionTable.hh"

#define MAX_PARTITIONS 128

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

bool GptPartitionTable::read(const string &path)
{
  int fd = -1;
  blkid_probe probe = 0;
  blkid_partlist partlist = 0;
  blkid_parttable parttable = 0;
  string label;
  string uuid;
  vector <Partition> table;
  int i = 0;
  int j = 0;
  blkid_partition partition = 0;
  GptPartition part;
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
    part.setNumber(blkid_partition_get_partno(partition));

    part.setStart(blkid_partition_get_start(partition));

    part.setEnd(blkid_partition_get_start(partition) + blkid_partition_get_size(partition) - 1);

    part.setSectors(blkid_partition_get_size(partition));

    part.setType(blkid_partition_get_type_string(partition));

    part.setName(blkid_partition_get_name(partition));

    part.setUUID(blkid_partition_get_uuid(partition));

    part.setFlags(blkid_partition_get_flags(partition));

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
  return true;
}

// -%- strip: yes; add-newline: yes; use-tabs: no; indent-width: 2; tab-width: 2; -%-
