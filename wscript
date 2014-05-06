
APPNAME = 'aislinn'
VERSION = '0.0.1'

def options(ctx):
    ctx.load('compiler_cxx')

def configure(ctx):
    ctx.load('compiler_cxx')
    ctx.env.append_value("CXXFLAGS", "-g")
    ctx.check_cxx(header_name="mhash.h",
                  lib="mhash", uselib_store='LLVM')
    ctx.check_cfg(package='',
                  path='llvm-config',
                  # FIXME: Replace 'all' with specific libraries
                  args='--cppflags --ldflags --libs all',
                  uselib_store='LLVM')

def build(ctx):
    ctx.recurse('src')
