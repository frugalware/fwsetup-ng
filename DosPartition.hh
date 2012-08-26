#pragma once

#include "Partition.hh"

class DosPartition : public Partition
{

public:
  DosPartition() { _dos_type = 0, _dos_active = false; }
  virtual ~DosPartition() { }
  void setType(unsigned char type) { _dos_type = type; }
  virtual void setActive(bool active) { _dos_active = active; }
  virtual void setPurpose(string purpose)
  {
    if(purpose == "extended")
      _dos_type = 0x05;
    else if(purpose == "efi")
      _dos_type = 0xEF;
    else if(purpose == "data")
      _dos_type = 0x83;
    else if(purpose == "swap")
      _dos_type = 0x82;
    else if(purpose == "raid")
      _dos_type = 0xFD;
    else if(purpose == "lvm")
      _dos_type = 0x8E;
    else if(purpose == "empty")
      _dos_type = 0x00;
  }
  unsigned char getType() { return _dos_type; }
  virtual bool getActive() { return _dos_active; }
  virtual string getPurpose()
  {
    switch(_dos_type)
    {
      case 0x05:
        return "extended";
      case 0xEF:
        return "efi";
      case 0x83:
        return "data";
      case 0x82:
        return "swap";
      case 0xFD:
        return "raid";
      case 0x8E:
        return "lvm";
      case 0x00:
        return "empty";
      default:
        return "unknown";
    }
  }

protected:
  unsigned char _dos_type;
  bool _dos_active;

};
