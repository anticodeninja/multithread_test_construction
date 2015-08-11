#include <iostream>
#include <fstream>
#include <chrono>
#include <map>

#include "global_settings.h"
#include "timecollector.h"
#include "input_matrix.h"
#include "irredundant_matrix.h"

std::ofstream* debugOutput;

void printBuildFlags(std::ostream& stream);

int main()
{
    debugOutput = new std::ofstream("debug_output.txt");
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

    IrredundantMatrix irredundantMatrix;

    #ifdef MULTITHREAD
    inputMatrix.calculateMultiThreadWithOptimalPlanBuilding(irredundantMatrix);
    #else
    inputMatrix.calculateSingleThread(irredundantMatrix);
    #endif

    std::ofstream resultOutput("output_data.txt");
    irredundantMatrix.printMatrix(resultOutput);

#ifdef DEBUG_MODE
    getDebugStream() << "# Irreduntant Matrix" << std::endl;
    irredundantMatrix.printMatrix(*debugOutput);
#endif

    #ifdef TIME_PROFILE
    all.Stop();
    std::ofstream timeCollectorOutput("time_collector.txt");

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

    #ifdef MULTITHREAD
    debugOutput << "- MultiThread" << std::endl;
    #endif
}

std::ostream& getDebugStream() {
    return *debugOutput;
}
