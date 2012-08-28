#include "Utility.hh"
#include "UserInterface.hh"
#include "Device.hh"
#include "PartitionModule.hh"

#ifdef NEWT
#include <newt.h>
#include <stdlib.h>
#include <sstream>

using std::stringstream;

class Item
{

public:
  Item() { device = 0, partition = 0, space = false; }
  ~Item() { }
  static bool getItems(vector <Device *> &devices,vector <Item *> &items,newtComponent &listbox);
  string text;
  Device *device;
  Partition *partition;
  bool space;

};

static string formatDeviceText(Device *device)
{
  stringstream buf;
  
  buf << device->getPath();
  buf << ": ";
  buf << device->getLabelType();
  buf << " label (";
  buf << device->getSizeString();
  buf << ")";
  
  return buf.str();
}

// FIXME: this function makes assumptions about text alignment padding.
static string formatPartitionText(Device *device,Partition *part)
{
  stringstream buf;
  
  buf << "  ";
  buf << "Partition";
  buf << " ";
  buf.width(3);
  buf << part->getNumber();
  buf.width(0);
  buf << ": ";
  buf.width(8);
  buf << part->getPurpose();
  buf.width(0);
  buf << " type (";
  buf << device->sectorsToSizeString(part->getSectors());
  buf << ")";
  
  return buf.str();
}

static string formatSpaceText(Device *device)
{
  stringstream buf;
  
  buf << "  Free space (";
  buf << device->getUnusedSizeString();
  buf << ")";
  
  return buf.str();
}

bool Item::getItems(vector <Device *> &devices,vector <Item *> &items,newtComponent &listbox)
{
  devices = Device::probeAll();
  
  if(devices.empty())
    return false;

  for( size_t i = 0 ; i < devices.size() ; ++i )
  {
    Device *device = devices.at(i);
    
    if(device->isDisk())
    {
      Item *item = new Item();
      
      item->text = formatDeviceText(device);

      item->device = device;
      
      items.push_back(item);
      
      if(device->getLabelType() != "unknown")
      {
        for( size_t j = 0 ; j < device->getTableSize() ; ++j )
        {
          Partition *part = device->getPartition(j);
        
          item = new Item();
          
          item->text = formatPartitionText(device,part);
          
          item->partition = part;
          
          items.push_back(item);
        }

        if(device->getUnusedSizeString() != "0.0BiB")
        {        
          item = new Item();
        
          item->text = formatSpaceText(device);

          item->space = true;
        
          items.push_back(item);
        }
      }
      
      item = new Item();
      
      items.push_back(item);
    }
  }

  if(items.empty())
    return false;

  return true;
}
#endif

PartitionModule::PartitionModule()
{
}

PartitionModule::~PartitionModule()
{
}

int PartitionModule::run()
{
  const char *title = "Partition Setup";

#ifdef NEWT
  vector <Item *> items;
  vector <Device *> devices;
  newtComponent i;
  
  if(!Item::getItems(devices,items,i))
    return 0;

  for( size_t n = 0 ; n < items.size() ; ++n )
  {
    Item *item = items[n];
    
    printf("%s\n",item->text.c_str());
  }
  
#endif

  return 1;
}

string PartitionModule::getName()
{
  return __FILE__;
}

PartitionModule partition_module;
