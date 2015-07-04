#include <iostream>
#include <fstream>
#include <chrono>
#include <map>

#include "global_settings.h"
#include "timecollector.h"
#include "input_matrix.h"
#include "irredundant_matrix.h"

using namespace std;

int main()
{
    #ifdef TIME_PROFILE
    TimeCollector::Initialize(static_cast<int>(Timers::TimeCollectorCount));
    TimeCollectorEntry all(static_cast<int>(Timers::All));
    #endif

    ifstream input_stream("input_data.txt");
    InputMatrix inputMatrix(input_stream);

    #ifdef DEBUG_MODE
    ofstream debugOutput("debug_output.txt");
    inputMatrix.printFeatureMatrix(debugOutput);
    inputMatrix.printImageMatrix(debugOutput);
    inputMatrix.printDebugInfo(debugOutput);
    #endif

    IrredundantMatrix irredundantMatrix;

    #ifdef MULTITHREAD
    inputMatrix.calculateMultiThreadWithOptimalPlanBuilding(irredundantMatrix);
    #else
    inputMatrix.calculateSingleThread(irredundantMatrix);
    #endif

    ofstream resultOutput("output_data.txt");
    irredundantMatrix.printMatrix(resultOutput);

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

    return 0;
}

