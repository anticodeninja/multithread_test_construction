#! /usr/bin/env python
# encoding: utf-8

from scripts.whelper import RunTestTask

APPNAME = 'multithread-test'
VERSION = '0.9'

top = '.'
out = 'build_directory'
  
def options(ctx):
   ctx.load('compiler_cxx')
   
   ctx.add_option('-d', '--debug',
                  action='store_true',
                  default=False,
                  help='Use additional messages to finding errors')
   ctx.add_option('-p', '--profiling',
                  action='store_true',
                  default=False,
                  help='Use time-collector for profiling perfomance')

   ctx.add_option('--mt', '--use-multithread',
                  dest="use_multithread",
                  action='store_true',
                  default=False,
                  help='Use multithread realization of algorithm')
   ctx.add_option('--vm', '--use-vector-for-work-matrices',
                  dest="use_vector_for_work_matrices",
                  action='store_true',
                  default=False,
                  help='Use vector for keeping work matrices in memory')
   ctx.add_option('--dm', '--use-different-work-matrices',
                  dest="use_different_work_matrices",
                  action='store_true',
                  default=False,
                  help='Use different work matrices for merging')

   ctx.add_option('--input-file',
                  dest="input_file",
                  action='store',
                  help='Enable debug target with specified input file')
   ctx.add_option('--reference-file',
                  dest="reference_file",
                  action='store',
                  help='Enable debug output checking with specified reference file')

def configure(ctx):
   ctx.load('compiler_cxx')
   ctx.env.append_value('CXXFLAGS', '-std=c++11')

   if ctx.options.debug:
      ctx.env.append_value('DEFINES', 'DEBUG_MODE')
      ctx.env.append_value('CXXFLAGS', '-g')
   else:
      ctx.env.append_value('CXXFLAGS', '-O2')
      
   if ctx.options.profiling:
      ctx.env.PROFILING = True
      ctx.env.append_value('DEFINES', 'TIME_PROFILE')
      
   if ctx.options.use_multithread:
      ctx.env.append_value('DEFINES', 'MULTITHREAD')
      ctx.env.append_value('CXXFLAGS', '-pthread')
      ctx.env.append_value('LINKFLAGS', '-pthread')
      
   if ctx.options.use_vector_for_work_matrices:
      ctx.env.append_value('DEFINES', 'IRREDUNDANT_VECTOR')
      
   if ctx.options.use_different_work_matrices:
      ctx.env.append_value('DEFINES', 'DIFFERENT_MATRICES')

   if ctx.options.input_file:
      input_file = ctx.path.find_resource(ctx.options.input_file)
      if not input_file:
         ctx.fatal('Input file %s for debugging is not found' % ctx.options.input_file)
      ctx.env.INPUT_FILE = input_file.abspath()

   if ctx.options.reference_file:
      reference_file = ctx.path.find_resource(ctx.options.reference_file)
      if not reference_file:
         ctx.fatal('Reference file %s for output checking is not found' % ctx.options.reference_file)
      ctx.env.REFERENCE_FILE = reference_file.abspath()

   print('configuring the project in ' + ctx.path.abspath())

def build(ctx):
   ctx(
      features=['cxx', 'cxxprogram'],
      source=ctx.path.ant_glob('src/*.cpp'),
      target='multithread',
      use='mylib')

def debug(ctx):
   build(ctx)
   ctx.run(True, 'multithread')

def run_tests(ctx):
   ctx.run(
      available_params = [ "use_multithread", "use_vector_for_work_matrices", "use_different_work_matrices" ],
      configurations = [
         { "id": "vanilla", "modules": [] },
         { "id": "mt", "modules": [ "use_multithread" ] },
         { "id": "mt+vm", "modules": [ "use_multithread", "use_vector_for_work_matrices" ] },
         { "id": "mt+dm", "modules": [ "use_multithread", "use_different_work_matrices" ] },
         { "id": "mt+vm+dm", "modules": [ "use_multithread", "use_different_work_matrices", "use_vector_for_work_matrices" ] },
      ]
   )

def analyze(ctx):
   ctx.run(8080, ctx.bldnode.abspath())
