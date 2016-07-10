#include <iostream>
#include <fstream>
#include <chrono>
#include <map>

#include "global_settings.h"
#include "timecollector.h"
#include "input_matrix.h"
#include "irredundant_matrix.h"

std::ofstream* debugOutput;
std::mutex* debugLock;

void printBuildFlags(std::ostream& stream);

int main()
{
    debugOutput = new std::ofstream("debug_output.txt");
    debugLock = new std::mutex();
    printBuildFlags(getDebugStream());

    TimeCollector::Initialize();
    TimeCollector::ThreadInitialize();
    TimeCollectorEntry executionTime(Counters::All);

    std::ifstream input_stream("input_data.txt");
    InputMatrix inputMatrix(input_stream);

#ifdef DEBUG_MODE
    inputMatrix.printFeatureMatrix(getDebugStream());
    inputMatrix.printImageMatrix(getDebugStream());
    inputMatrix.printDebugInfo(getDebugStream());
#endif

    IrredundantMatrix irredundantMatrix(inputMatrix.getFeatureWidth());
    inputMatrix.calculate(irredundantMatrix);

    std::ofstream resultOutput("output_data.txt");
    irredundantMatrix.printMatrix(resultOutput);
    resultOutput << std::endl;
    irredundantMatrix.printR(resultOutput);

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
