#include <iostream>
#include <fstream>
#include <chrono>

#include "global_settings.h"
#include "timecollector.h"
#include "input_matrix.h"

#include "irredundant_matrix_array.h"
#include "irredundant_matrix_queue.h"

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

    IrredundantMatrixArray irredundantMatrix;
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
    TimeCollector::PrintInfo(timeCollectorOutput);
    #endif

    return 0;
}

