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
                  action='count',
                  default=False,
                  help='Use time-collector for profiling perfomance')

   ctx.add_option('--mt-d2', '--use-multithread-divide-2',
                  dest="use_multithread_divide_2",
                  action='store_true',
                  default=False,
                  help='Use multithread-divide-2 realization of algorithm')
   ctx.add_option('--mt-d2o', '--use-multithread-divide-2-optimized',
                  dest="use_multithread_divide_2_optimized",
                  action='store_true',
                  default=False,
                  help='Use multithread-divide-2-optimized realization of algorithm')
   ctx.add_option('--mt-mw', '--use-multithread-master-worker',
                  dest="use_multithread_master_worker",
                  action='store_true',
                  default=False,
                  help='Use multithread-master-worker realization of algorithm')
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
   ctx.add_option('--ll', '--use-local-lock',
                  dest="use_local_lock",
                  action='store_true',
                  default=False,
                  help='Use local-lock for row merging')

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
      ctx.env.profiling = ctx.options.profiling
      ctx.env.append_value('DEFINES', 'TIME_PROFILE=%d' % ctx.options.profiling)
      
   if ctx.options.use_multithread_divide_2:
      ctx.env.append_value('DEFINES', 'MULTITHREAD_DIVIDE2')

   if ctx.options.use_multithread_divide_2_optimized:
      ctx.env.append_value('DEFINES', 'MULTITHREAD_DIVIDE2_OPTIMIZED')

   if ctx.options.use_multithread_master_worker:
      ctx.env.append_value('DEFINES', 'MULTITHREAD_MASTERWORKER')

   if any([
         ctx.options.use_multithread_divide_2,
         ctx.options.use_multithread_divide_2_optimized,
         ctx.options.use_multithread_master_worker
   ]):
      ctx.env.append_value('DEFINES', 'MULTITHREAD')
      ctx.env.append_value('CXXFLAGS', '-pthread')
      ctx.env.append_value('LINKFLAGS', '-pthread')
      
   if ctx.options.use_vector_for_work_matrices:
      ctx.env.append_value('DEFINES', 'IRREDUNDANT_VECTOR')
      
   if ctx.options.use_different_work_matrices:
      ctx.env.append_value('DEFINES', 'DIFFERENT_MATRICES')

   if ctx.options.use_local_lock:
      ctx.env.append_value('DEFINES', 'USE_LOCAL_LOCK')

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
   ctx.add_group()
   ctx.run(True, 'multithread')

def perf(ctx):
   build(ctx)
   ctx.add_group()
   ctx.run(100, 'multithread')

def run_tests(ctx):
   ctx.run(
      True,
      available_params = [
         "use_multithread_divide_2",
         "use_multithread_master_worker",
         "use_vector_for_work_matrices",
         "use_different_work_matrices",
         "use_local_lock",
      ],
      configurations = [
         { "id": "1_vanilla", "modules": [] },
         { "id": "2_mt-d2", "modules": [ "use_multithread_divide_2" ] },
         { "id": "3_mt-d2+vm", "modules": [ "use_multithread_divide_2", "use_vector_for_work_matrices" ] },
         { "id": "4_mt-d2+dm", "modules": [ "use_multithread_divide_2", "use_different_work_matrices" ] },
         { "id": "5_mt-d2+vm+dm", "modules": [ "use_multithread_divide_2", "use_different_work_matrices", "use_vector_for_work_matrices" ] },
         { "id": "6_mt-d2+ll", "modules": [ "use_multithread_divide_2", "use_local_lock" ] },
         { "id": "7_mt-mw", "modules": [ "use_multithread_master_worker" ] },
         { "id": "8_mt-mw+ll", "modules": [ "use_multithread_master_worker", "use_local_lock" ] },
         { "id": "7_mt-mw", "modules": [ "use_multithread_master_worker" ] },
         { "id": "9_mt-mw+ll+dm", "modules": [ "use_multithread_master_worker", "use_local_lock", "use_different_work_matrices" ] },
      ]
   )

def analyze(ctx):
   ctx.run(8080, ctx.bldnode.abspath())
