#include <iostream>
#include <fstream>
#include <chrono>

#include "global_settings.h"
#include "timecollector.h"
#include "input_matrix.h"

#include "irredundant_matrix_array.h"
#include "irredundant_matrix_queue.h"

using namespace std;

int main(int argc, char** argv)
{
    #ifdef TIME_PROFILE
    TimeCollector::Initialize(TimeCollectorCount);
    TimeCollectorEntry all(All, true);
    #endif

    #ifdef DEBUG_MODE
    cout << "Program start" << endl;
    #endif

    ifstream input_stream("../input_data.txt");
    InputMatrix inputMatrix(input_stream);

    #ifdef DEBUG_MODE
    cout << "Feature Matrix:" << endl;
    inputMatrix.printFeatureMatrix(cout);

    cout << "Image Matrix:" << endl;
    inputMatrix.printImageMatrix(cout);

    inputMatrix.printDebugInfo(cout);
    #endif

//    IrredundantMatrixQueue irredundantMatrix;
    IrredundantMatrixArray irredundantMatrix;
    inputMatrix.calculateSingleThread(irredundantMatrix, false);
//    inputMatrix.calculateMultiThreadWithOptimalPlanBuilding(irredundantMatrix, false);

    cout << "Coverage Matrix:" << endl;
    irredundantMatrix.printMatrix(cout);

    #ifdef TIME_PROFILE
    all.Stop();
    cout << "All: " << chrono::duration_cast<chrono::nanoseconds>(TimeCollector::GetTimeValue(All)).count() << "ns\n";
    cout << "ReadingInput: " << chrono::duration_cast<chrono::nanoseconds>(TimeCollector::GetTimeValue(ReadingInput)).count() << "ns\n";
    cout << "PreparingInput: " << chrono::duration_cast<chrono::nanoseconds>(TimeCollector::GetTimeValue(PreparingInput)).count() << "ns\n";
    cout << "PlanBuilding: " << chrono::duration_cast<chrono::nanoseconds>(TimeCollector::GetTimeValue(PlanBuilding)).count() << "ns\n";
    cout << "QHandling: " << chrono::duration_cast<chrono::nanoseconds>(TimeCollector::GetTimeValue(QHandling)).count() << "ns\n";
    cout << "RMerging: " << chrono::duration_cast<chrono::nanoseconds>(TimeCollector::GetTimeValue(RMerging)).count() << "ns\n";
    cout << "Threading: " << chrono::duration_cast<chrono::nanoseconds>(TimeCollector::GetTimeValue(Threading)).count() << "ns\n";
    cout << "CrossThreading: " << chrono::duration_cast<chrono::nanoseconds>(TimeCollector::GetTimeValue(CrossThreading)).count() << "ns\n";
    cout << "WritingOutput: " << chrono::duration_cast<chrono::nanoseconds>(TimeCollector::GetTimeValue(WritingOutput)).count() << "ns\n";
    #endif

    return 0;
}

