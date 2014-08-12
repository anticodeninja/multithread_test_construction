#ifndef IRREDUNTANT_MATRIX_FACTORY_H
#define IRREDUNTANT_MATRIX_FACTORY_H

#include <memory>

#include "irredundant_matrix_queue.h"
#include "irredundant_matrix_array.h"

class IrreduntantMatrixFactory
{
public:
    enum IrreduntantMatrixType
    {
        Queue,
        Array
    };

    static void SetIrreduntantMatrixType() {
        _irredundantMatrixType =
    }

    static std::unique_ptr<IrredundantMatrixBase> CreateIrreduntantMatrix() {

    }

private:
    static IrreduntantMatrixType _irredundantMatrixType;
};

#endif // IRREDUNTANT_MATRIX_FACTORY_H
