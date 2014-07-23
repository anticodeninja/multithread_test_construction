#include <iostream>
#include <fstream>
#include <chrono>

#include "input_matrix.h"

using namespace std;

int main()
{
    auto start = chrono::high_resolution_clock::now();
    ifstream input_stream("../input_data.txt");
    InputMatrix inputMatrix(input_stream);
    cout << chrono::duration_cast<chrono::nanoseconds>(chrono::high_resolution_clock::now()-start).count() << "ns\n";

    cout << "Feature Matrix:" << endl;
    inputMatrix.printFeatureMatrix(cout);
    cout << chrono::duration_cast<chrono::nanoseconds>(chrono::high_resolution_clock::now()-start).count() << "ns\n";

    cout << "Image Matrix:" << endl;
    inputMatrix.printImageMatrix(cout);
    cout << chrono::duration_cast<chrono::nanoseconds>(chrono::high_resolution_clock::now()-start).count() << "ns\n";

    return 0;
    IrredundantMatrix irredundantMatrix;
    inputMatrix.calculateCoverageMatrix(irredundantMatrix);
    cout << chrono::duration_cast<chrono::nanoseconds>(chrono::high_resolution_clock::now()-start).count() << "ns\n";

    cout << "Coverage Matrix:" << endl;
    irredundantMatrix.printMatrix(cout);
    cout << chrono::duration_cast<chrono::nanoseconds>(chrono::high_resolution_clock::now()-start).count() << "ns\n";

    return 0;
}

