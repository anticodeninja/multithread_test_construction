#! /usr/bin/env python
# encoding: utf-8

import os

from waflib import Options, Scripting, Logs
from waflib.Node import Node
from waflib.Task import Task, RUN_ME
from waflib.Build import BuildContext

from scripts.utils import validate_result
from scripts.analyzer import AnalyzerServer

class RunTestTask(Task):
    color = 'PINK'
    vars = ['INPUT_FILE', 'REFERENCE_FILE']

    def __init__(self, *args, **kwargs):
        super().__init__(*args, **kwargs)
        self.force = args[0]
        self.executable = args[1]

    def runnable_status(self):
        if self.force:
            return RUN_ME
        return super().runnable_status()

    def run(self):
        Logs.pprint('CYAN', "Creating environment...")
        result = self.exec_command('cp %s %s' %
                                   (self.env.INPUT_FILE, self.executable.parent.make_node('input_data.txt').abspath()))
        if result != 0: return result

        Logs.pprint('CYAN', "Working...")
        original_dir = os.getcwd()
        os.chdir(self.executable.parent.abspath())
        result = self.exec_command('%s' % self.executable.abspath())
        os.chdir(original_dir)         
        if result != 0: return result

        if self.env.REFERENCE_FILE:
            Logs.pprint('CYAN', "Reference checking...")
            try:
                validate_result(self.env.REFERENCE_FILE, self.executable.parent.find_node('output_data.txt').abspath())
                Logs.pprint('CYAN', "Everything is ok")
            except Exception as e:
                Logs.pprint('CYAN', "Error: %s" % str(e))
                return 1

        return 0

class RunContext(BuildContext):
    cmd = 'debug'
    fun = 'debug'

    def run(self, force, executable):
        self.add_to_group(RunTestTask(force, self.bldnode.find_node(executable), env=self.env))

class RunTestsContext(BuildContext):
    cmd = 'run_tests'
    fun = 'run_tests'

    def run(self, available_params, configurations):
        test_build_path = self.path.make_node(self.bldnode.name + '_tests')
       
        Options.lockfile = Options.lockfile + '_tests'
        Options.options.out = test_build_path.abspath()
        Options.options.profiling = True
        Options.options.input_file = os.path.relpath(self.env.INPUT_FILE, self.path.abspath())
        Options.options.reference_file = os.path.relpath(self.env.REFERENCE_FILE, self.path.abspath())

        for configuration in configurations:
            for configuration_param in available_params:
                setattr(Options.options, configuration_param, configuration_param in configuration['modules'])

            Logs.pprint('PINK', 'Testing %s build...' % configuration['id'])
      
            Scripting.run_command('configure')
            Scripting.run_command('build')
            Scripting.run_command('debug')

            self.exec_command('cp %s %s' % (
                test_build_path.find_node('current_profile.txt').abspath(),
                self.bldnode.make_node('%s_profile.txt' % configuration['id']).abspath()))
         

        Scripting.run_command('distclean')

class AnalyzeContext(BuildContext):
    cmd = 'analyze'
    fun = 'analyze'

    def run(self, port, result_path):
        AnalyzerServer(port, result_path).run()
