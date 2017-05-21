#include <iostream>
#include <fstream>
#include <chrono>
#include <map>

#include "../argparse-port/argparse.h"

#include "global_settings.h"
#include "timecollector.hpp"
#include "input_matrix.hpp"
#include "irredundant_matrix.hpp"

std::ofstream* debugOutput;
std::mutex* debugLock;

void printBuildFlags(std::ostream& stream);

int main(int argc, char** argv)
{
    parser_t* parser;
    parser_init(&parser);

    parser_string_arg_t* input_arg;
    parser_string_add_arg(parser, &input_arg, "input");
    parser_string_set_help(input_arg, "input file");

    parser_string_arg_t* output_arg;
    parser_string_add_arg(parser, &output_arg, "output");
    parser_string_set_help(output_arg, "output file");

    parser_flag_arg_t* no_transfer;
    parser_flag_add_arg(parser, &no_transfer, "--no-transfer");
    parser_flag_set_help(no_transfer, "no transfer blocks from input file to output");

    if (parser_parse(parser, argc, argv) != PARSER_RESULT_OK) {
        printf("%s", parser_get_last_err(parser));
        parser_free(&parser);
        return 1;
    }

#ifdef DEBUG_MODE
    debugOutput = new std::ofstream("debug_output.txt");
    debugLock = new std::mutex();
    printBuildFlags(getDebugStream());
#endif

    TimeCollector::Initialize();
    TimeCollector::ThreadInitialize();
    TimeCollectorEntry executionTime(Counters::All);

    InputMatrix inputMatrix;
    START_COLLECT_TIME(readingInput, Counters::ReadingInput);
    if (strcmp("-", input_arg->value) != 0) {
        std::ifstream input_stream(input_arg->value);
        inputMatrix.read(input_stream);
    } else {
        inputMatrix.read(std::cin);
    }
    STOP_COLLECT_TIME(readingInput);

#ifdef DEBUG_MODE
    inputMatrix.printFeatureMatrix(getDebugStream());
    inputMatrix.printImageMatrix(getDebugStream());
    inputMatrix.printDebugInfo(getDebugStream());
#endif

    IrredundantMatrix irredundantMatrix(inputMatrix.getFeatureWidth());
    inputMatrix.calculate(irredundantMatrix);

    if (strcmp("-", output_arg->value) != 0) {
        std::ofstream output_stream(output_arg->value);
        irredundantMatrix.write(output_stream);
    } else {
        irredundantMatrix.write(std::cout);
    }

#ifdef DEBUG_MODE
    getDebugStream() << "# Irreduntant Matrix" << std::endl;
    irredundantMatrix.printMatrix(*debugOutput);
    getDebugStream() << "# R Matrix" << std::endl;
    irredundantMatrix.printR(*debugOutput);
#endif

    executionTime.Stop();
    std::ofstream timeCollectorOutput("current_profile.txt");

    TimeCollector::ThreadFinalize();
    TimeCollector::PrintInfo(timeCollectorOutput);

    debugOutput->close();
    delete debugOutput;
    delete debugLock;

    parser_free(&parser);
    return 0;
}

void printBuildFlags(std::ostream& debugOutput) {
    debugOutput << "# BuildFlags" << std::endl;

#ifdef IRREDUNDANT_VECTOR
    debugOutput << "- Irredundant Vector" << std::endl;
#endif

#ifdef TIME_PROFILE
    debugOutput << "- Time Profile" << std::endl;
#endif

#ifdef DIFFERENT_MATRICES
    debugOutput << "- Different Matrices" << std::endl;
#endif

#ifdef DEBUG_MODE
    debugOutput << "- Debug Mode" << std::endl;
#endif

#ifdef MULTITHREAD_DIVIDE2
    debugOutput << "- MultiThread Divide 2 Algo" << std::endl;
#elif MULTITHREAD_MASTERWORKER
    debugOutput << "- MultiThread MasterWorker Algo" << std::endl;
#endif
}

std::ostream& getDebugStream() {
    return *debugOutput;
}

std::mutex& getDebugStreamLock() {
    return *debugLock;
}
