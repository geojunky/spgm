import platform

# Let's define a common build environment first...
common_env = Environment()
# check dependencies
conf = Configure(common_env)

deps = ['pthread', 'gomp']

for i in range(len(deps)):
    if not conf.CheckLib(deps[i]):
        print 'Did not find %s-lib, exiting!'%(deps[i])
        Exit(1)
    #end if
#end for

conf.env.Append(CCFLAGS = '-Wall')
# Add -g for debug builds
debug = ARGUMENTS.get('debug', 0)
if int(debug):
    conf.env.Append(CCFLAGS = '-g')

# Add -O3 for release builds
release = ARGUMENTS.get('release', 1)
if int(release) and not debug:
    conf.env.Append(CCFLAGS = '-O3')

# Check architecture
if(platform.architecture()[0]=='64bit'):
    conf.env.Append(CPPDEFINES={'ARCH64':1})

# Finalize configuring environment
env = conf.Finish()

# version
common_env.Append(CPPDEFINES={'VERSION': 1})

# Our release build is derived from the common build environment...
release_env = common_env.Clone()
release_env.Append(CPPDEFINES=['RELEASE'])
release_env.VariantDir('build/release', 'src')

# We define our debug build environment in a similar fashion...
debug_env = common_env.Clone()
debug_env.Append(CPPDEFINES=['DEBUG'])
debug_env.VariantDir('build/debug', 'src')

for mode, env in dict(release=release_env,
                     debug=debug_env).iteritems():
    env.SConscript('build/%s/SConscript' % mode, {'env': env})

