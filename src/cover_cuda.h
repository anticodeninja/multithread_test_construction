#ifndef CUDACOVER_H
#define CUDACOVER_H

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

    typedef struct {
        int cover_koef;
        int lset_len;
        int features;
        int block_len;

        char* lset;
        char* block;

        int results_counter;
        int* results;

        bool __lset_uploaded;
        char* __lset;
        char* __block;
        int* __results_counter;
        int* __results;
    } cudacover_t;

    typedef enum {
        CUDA_COVER_RESULT_OK,
        CUDA_COVER_RESULT_ERR
    } cudacover_result_t;

    cudacover_result_t cudacover_init(cudacover_t** ctx,
                                      int lset_len,
                                      int features,
                                      int block_len);

    cudacover_result_t cudacover_check(cudacover_t* ctx,
                                       int block_len,
                                       int thread_block);

    cudacover_result_t cudacover_free(cudacover_t** ctx);

#ifdef __cplusplus
}
#endif

#endif // CUDACOVER_H
