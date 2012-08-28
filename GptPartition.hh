#pragma once

#include "Partition.hh"

class GptPartition : public Partition
{

public:
  GptPartition() { _gpt_type = "0FC63DAF-8483-4772-8E79-3D69D8477DE4", _gpt_uuid = "R", _gpt_flags = 0; }
  virtual ~GptPartition() {}
  void setType(const string &type) { _gpt_type = type; }
  void setName(const string &name) { _gpt_name = name; }
  void setUUID(const string &uuid) { _gpt_uuid = uuid; }
  void setFlags(unsigned long long flags) { _gpt_flags = flags; }
  virtual void setActive(bool active) { if(active) _gpt_flags |= 4; else if(_gpt_flags & 4) _gpt_flags ^= 4; }
  virtual void setPurpose(string purpose)
  {
    if(purpose == "bios")
      _gpt_type = "21686148-6449-6E6F-744E-656564454649";
    else if(purpose == "efi")
      _gpt_type = "C12A7328-F81F-11D2-BA4B-00A0C93EC93B";
    else if(purpose == "data")
      _gpt_type = "0FC63DAF-8483-4772-8E79-3D69D8477DE4";
    else if(purpose == "swap")
      _gpt_type = "0657FD6D-A4AB-43C4-84E5-0933C84B4F4F";
    else if(purpose == "raid")
      _gpt_type = "A19D880F-05FC-4D3B-A006-743F0F84911E";
    else if(purpose == "lvm")
      _gpt_type = "E6D6D379-F507-44C2-A23C-238F2A3DF928";
  }
  string getType() { return _gpt_type; }
  string getName() { return _gpt_name; }
  string getUUID() { return _gpt_uuid; }
  unsigned long long getFlags() { return _gpt_flags; }
  virtual bool getActive() { return (_gpt_flags & 4) ? true : false; }
  virtual string getPurpose()
  {
    if(_gpt_type == "21686148-6449-6E6F-744E-656564454649")
      return "bios";
    else if(_gpt_type == "C12A7328-F81F-11D2-BA4B-00A0C93EC93B")
      return "efi";
    else if(_gpt_type == "0FC63DAF-8483-4772-8E79-3D69D8477DE4")
      return "data";
    else if(_gpt_type == "0657FD6D-A4AB-43C4-84E5-0933C84B4F4F")
      return "swap";
    else if(_gpt_type == "A19D880F-05FC-4D3B-A006-743F0F84911E")
      return "raid";
    else if(_gpt_type == "E6D6D379-F507-44C2-A23C-238F2A3DF928")
      return "lvm";
    else
      return "unknown";
  }

protected:
  string _gpt_type;
  string _gpt_name;
  string _gpt_uuid;
  unsigned long long _gpt_flags;

};
