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

#ifdef TIME_PROFILE
    TimeCollector::Initialize(static_cast<int>(Timers::TimeCollectorCount));
    TimeCollectorEntry all(static_cast<int>(Timers::All));
#endif

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

#ifdef TIME_PROFILE
    all.Stop();
    std::ofstream timeCollectorOutput("current_profile.txt");

    std::map<int, std::string> names_string = {
            { (int)Timers::All, "All"},
            { (int)Timers::ReadingInput, "ReadingInput"},
            { (int)Timers::PreparingInput, "PreparingInput"},
            { (int)Timers::CalcR2Indexes, "CalcR2Indexes"},
            { (int)Timers::SortMatrix, "SortMatrix"},
            { (int)Timers::CalcR2Matrix, "CalcR2Matrix"},
            { (int)Timers::PlanBuilding, "PlanBuilding"},
            { (int)Timers::QHandling, "QHandling"},
            { (int)Timers::RMerging, "RMerging"},
            { (int)Timers::WritingOutput, "WritingOutput"},
            { (int)Timers::Threading, "Threading"},
            { (int)Timers::CrossThreading, "CrossThreading"},
            { (int)Timers::TimeCollectorCount, "TimeCollectorCount"},
    };

    TimeCollector::PrintInfo(timeCollectorOutput, names_string);
#endif

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
