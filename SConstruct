import os

env = Environment()

def CheckPkg(context,pkg):
  context.Message('Checking for %s... ' % pkg)
  rv = context.TryAction('pkg-config --exists \'%s\'' % pkg)[0]
  context.Result(rv)
  return rv

conf = Configure(env,{ "CheckPkg" : CheckPkg })

if not conf.CheckPkg('uuid'):
  print 'libuuid was not found.'
  Exit(1)

if not conf.CheckPkg('blkid'):
  print 'libblkid was not found.'
  Exit(1)

if not conf.CheckPkg('libnewt'):
  print 'libnewt was not found.'
  Exit(1)

conf.Finish()

try:
  cxxflags = os.environ['CXXFLAGS']
except KeyError:
  cxxflags = '-O2'
  pass

env.Append(CXXFLAGS = cxxflags)

env.Append(CXXFLAGS = '-Wall -Wextra -Wno-unused-parameter -ggdb3 -DNEWT')

env.ParseConfig('pkg-config --cflags --libs uuid blkid libnewt')

env.Program('fwsetup',[
  'Main.cc',
  'GptPartitionTable.cc',
  'DosPartitionTable.cc',
  'Device.cc',
  'UserInterface.cc',
  'Utility.cc'
])
