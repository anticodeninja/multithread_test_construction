#! /usr/bin/env python
# encoding: utf-8

from scripts.whelper import RunTestTask

APPNAME = 'multithread-test'
VERSION = '0.9'
DEFAULT_CONFIGURATIONS = [
   'uim_st', 'uim_st_dm', 'uim_st_vm',
   'uim_mt-d2', 'uim_mt-d2_dm_ll',
   'uim_mt-d2o', 'uim_mt-d2o_dm_ll',
   'uim_mt-mw', 'uim_mt-mw_dm_ll',
   'cover_st_df', 'cover_mt_df',
   'cover_st_bf', 'cover_mt_bf',
   'cover_cudabf',
   'cover_legtup'
]

top = '.'
out = 'build_directory'

def options(ctx):
   ctx.load('compiler_cxx')

   ctx.add_option('-a', '--aggregate',
                  action='store_true',
                  default=False,
                  help='Run perf tests and aggregate result')
   ctx.add_option('-d', '--debug',
                  action='store_true',
                  default=False,
                  help='Enable debugging')
   ctx.add_option('--debug-output',
                  action='store_true',
                  default=False,
                  help='Use additional messages to finding errors')
   ctx.add_option('-p', '--profiling',
                  action='count',
                  default=False,
                  help='Use time-collector for profiling perfomance')
   ctx.add_option('--gnu-profiling',
                  default=False,
                  action='store_true',
                  help='Use gperf for profiling perfomance')
   ctx.add_option('-c', '--configuration',
                  action='append',
                  help='Use specific configurations for build:\n'+
                       'uim_(st|mt-d2|mt-d2o|mt-mw)_(?dm)_(?vm)_(?ll)\n'+
                       'cover_(df|bf)_(?mt)_(?cuda)')

   ctx.add_option('-i', '--input-file',
                  dest="input_file",
                  action='store',
                  help='Enable debug target with specified input file')
   ctx.add_option('-r', '--reference-file',
                  dest="reference_file",
                  action='store',
                  help='Enable debug output checking with specified reference file')

def configure(ctx):
   ctx.load('compiler_cxx')

   ctx.env.configurations = ctx.options.configuration or DEFAULT_CONFIGURATIONS

   try:
      ctx.load('cuda', tooldir='./scripts/')
   except:
      ctx.env.configurations = [x for x in ctx.env.configurations if 'cuda' not in x]

   ctx.env.append_value('CXXFLAGS', '-std=c++11')

   if ctx.options.debug:
      ctx.env.append_value('CXXFLAGS', '-g')
   else:
      ctx.env.append_value('CXXFLAGS', '-O2')

   if ctx.options.debug_output:
      ctx.env.append_value('DEFINES', 'DEBUG_MODE')

   if ctx.options.gnu_profiling:
      ctx.env.append_value('CXXFLAGS', '-pg')
      ctx.env.append_value('LINKFLAGS', '-pg')

   ctx.env.append_value('INCLUDES', ["argparse-port"])

   if ctx.options.profiling:
      ctx.env.profiling = ctx.options.profiling
      ctx.env.append_value('DEFINES', 'TIME_PROFILE=%d' % ctx.options.profiling)

   if ctx.options.input_file:
      input_file = ctx.path.find_resource(ctx.options.input_file)
      if not input_file:
         ctx.fatal('Input file %s for debugging is not found' % ctx.options.input_file)
      ctx.env.input_file = input_file.abspath()

   if ctx.options.reference_file:
      reference_file = ctx.path.find_resource(ctx.options.reference_file)
      if not reference_file:
         ctx.fatal('Reference file %s for output checking is not found' % ctx.options.reference_file)
      ctx.env.reference_file = reference_file.abspath()

   print('configuring the project in ' + ctx.path.abspath())

def build(ctx):
   for configuration in ctx.env.configurations:
      chunks = configuration.split('_')

      multithreaded = False
      files = []
      defines = []
      libs = []
      lflags = []

      if 'uim' in chunks:
         files.append('uim_program.cpp')
         files.append('input_matrix.cpp')
         files.append('irredundant_matrix.cpp')
         files.append('row.cpp')
         files.append('timecollector.cpp')
         files.append('workrow.cpp')

         if 'mt-d2' in chunks:
            files.append('divide2_plan.cpp')
            defines.append('MULTITHREAD_DIVIDE2')
            multithreaded = True
         elif 'mt-d2o' in chunks:
            files.append('divide2_plan.cpp')
            defines.append('MULTITHREAD_DIVIDE2_OPTIMIZED')
            multithreaded = True
         elif 'mt-mw' in chunks:
            files.append('manyworkers_plan.cpp')
            defines.append('MULTITHREAD_MASTERWORKER')
            multithreaded = True

         if 'dm' in chunks:
            defines.append('DIFFERENT_MATRICES')

         if 'vm' in chunks:
            defines.append('IRREDUNDANT_VECTOR')

         if 'll' in chunks:
            defines.append('USE_LOCAL_LOCK')

      elif 'cover' in chunks:
         files.append('cover_program.cpp')
         files.append('datafile.cpp')
         files.append('resultset.cpp')
         files.append('timecollector.cpp')

         if 'df' in chunks:
            files.append('cover_depthfirst.cpp')
         elif 'bf' in chunks:
            files.append('cover_breadthfirst.cpp')
         elif 'cudabf' in chunks:
            libs.append('CUDA')
            libs.append('CUDART')
            files.append('cover_cuda.cpp')
            files.append('cover_cuda.cu')
         elif 'legtup' in chunks:
            files.append('legacy/cover_tuptests.cpp')
            files.append('legacy/matrix.cpp')

         if 'mt' in chunks:
            multithreaded = True

      if multithreaded:
         defines.append('MULTITHREAD')
         lflags.append('-pthread')

      src_dir = ctx.srcnode.find_dir('src')
      files = [src_dir.find_node(x) for x in files]
      files.append("argparse-port/argparse.c")

      ctx.program(
         features=['cxx', 'cxxprogram'],
         source=files,
         target=configuration,
         defines=defines,
         use=libs,
         linkflags=lflags,
      )

def debug(ctx):
   if not ctx.env.input_file:
      ctx.fatal('Input file should be configured for debug command')

   build(ctx)
   ctx.add_group()
   ctx.run(True, ctx.env.configurations[0])

def perf(ctx):
   build(ctx)
   ctx.add_group()
   ctx.run(100, ctx.env.configurations[0])

def run_tests(ctx):
   ctx.run(ctx.options.aggregate)

def analyze(ctx):
   ctx.run(8080, ctx.bldnode.abspath())
