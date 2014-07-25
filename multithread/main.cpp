#include <iostream>
#include <fstream>
#include <chrono>

#include "timecollector.h"
#include "input_matrix.h"

using namespace std;

int main()
{
    TimeCollector::Initialize(TimeCollectorCount);
    TimeCollectorEntry all(All, true);

    ifstream input_stream("../input_data.txt");
    InputMatrix inputMatrix(input_stream);

    cout << "Feature Matrix:" << endl;
    inputMatrix.printFeatureMatrix(cout);

    cout << "Image Matrix:" << endl;
    inputMatrix.printImageMatrix(cout);

    IrredundantMatrix irredundantMatrix;
    inputMatrix.calculateCoverageMatrix(irredundantMatrix);

    cout << "Coverage Matrix:" << endl;
    irredundantMatrix.printMatrix(cout);

    all.Stop();
    cout << "All: " << chrono::duration_cast<chrono::nanoseconds>(TimeCollector::GetTimeValue(All)).count() << "ns\n";
    cout << "ReadingInput: " << chrono::duration_cast<chrono::nanoseconds>(TimeCollector::GetTimeValue(ReadingInput)).count() << "ns\n";
    cout << "PreparingInput: " << chrono::duration_cast<chrono::nanoseconds>(TimeCollector::GetTimeValue(PreparingInput)).count() << "ns\n";
    cout << "PlanBuilding: " << chrono::duration_cast<chrono::nanoseconds>(TimeCollector::GetTimeValue(PlanBuilding)).count() << "ns\n";
    cout << "QHandling: " << chrono::duration_cast<chrono::nanoseconds>(TimeCollector::GetTimeValue(QHandling)).count() << "ns\n";
    cout << "RMerging: " << chrono::duration_cast<chrono::nanoseconds>(TimeCollector::GetTimeValue(RMerging)).count() << "ns\n";
    cout << "WritingOutput: " << chrono::duration_cast<chrono::nanoseconds>(TimeCollector::GetTimeValue(WritingOutput)).count() << "ns\n";

    return 0;
}

