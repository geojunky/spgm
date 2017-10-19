#!python
#!/usr/bin/env python
import os, os.path, sys, fnmatch

# ------------------------------------------------------------------
# ------------- BUILD ENVIRONMENT ----------------------------------
# ------------------------------------------------------------------
common_env = Environment(ENV = os.environ, tools = ['default'])

# check dependencies
conf = Configure(common_env)

# ------------------------------------------------------------------
# ------------- BUILD SETTINGS -------------------------------------
# ------------------------------------------------------------------
debug = int(ARGUMENTS.get('debug', 0))
test  = int(ARGUMENTS.get('test', 0))
release = 0
if (debug):
# Debug build
# Add -g for debug builds
    conf.env.Append(CFLAGS = '-g')
    conf.env.Append(CPPFLAGS = '-g')
else:
# Add -O3 for release builds
    conf.env.Append(CFLAGS = '-O3')
    conf.env.Append(CPPFLAGS = '-O3')
    release = 1
#end if

# Finalize configuring environment
env = conf.Finish()

# ------------------------------------------------------------------
# ------------- BUILD TARGETS --------------------------------------
# ------------------------------------------------------------------
mode = ''
if(debug):  mode = 'debug'
else:       mode = 'release'

#
# Paths
#
dirs = ['src']

for sd in dirs:
    buildDir = os.path.join('build/%s'%(mode),sd)
    consFile = os.path.join(buildDir,'SConscript')
    common_env.VariantDir(buildDir, sd)

    common_env.SConscript(consFile, {'env': common_env})
#end for

# ------------------------------------------------------------------
# ------------- POST-BUILD ACTIONS ---------------------------------
# ------------------------------------------------------------------
# Add an action to run tests
def runTests(target=None, source=None, env=None):
    targetdir = target[0].dir
    if(test):
        print '\n[ *** Running Test-suite *** ]\n'
        cmd = os.path.join('./build/%s'%(mode), 'src/tests/testsuite')
        os.system(cmd)
    #end if
#end func

runTestsCommand = Command( 'runTests', [], runTests)
Depends(runTestsCommand, DEFAULT_TARGETS)
Default(runTestsCommand)
