#include <stdio.h>
#include "cudacover.h"

#define THREAD_PER_BLOCK 256

__global__
void kernel(int cover_koef, int features, char *lset, int lset_len, char *block, int block_len, int *results, int *results_counter)
{
    int index = blockIdx.x*blockDim.x + threadIdx.x;

    if (index < block_len) {
        bool coverAll = true;

        for (int i = 0; i < lset_len; ++i) {
            int cover = 0;

            for (int j = 0; j < features; ++j) {
                if (lset[features * i + j] && block[features * index + j]) {
                    cover += 1;
                }
            }

            if (cover < cover_koef) {
                coverAll = false;
                break;
            }
        }

        if (coverAll) {
            results[atomicAdd(results_counter, 1)] = index;
        }
    }
}

cudacover_result_t cudacover_init(cudacover_t** ctx,
                                  int lset_len,
                                  int features,
                                  int block_len) {
    cudacover_t* temp = (cudacover_t*)malloc(sizeof(cudacover_t));
    if (temp == NULL) {
        return CUDA_COVER_RESULT_ERR;
    }

    temp->cover_koef = 1;
    temp->lset_len = lset_len;
    temp->features = features;
    temp->block_len = block_len;
    temp->lset = (char*)malloc(lset_len * features * sizeof(char));
    temp->block = (char*)malloc(block_len * features * sizeof(char));
    temp->results = (int*)malloc(block_len * sizeof(int));
    temp->results_counter = 0;

    cudaMalloc(&temp->__lset, lset_len * features * sizeof(char));
    cudaMalloc(&temp->__block, block_len * features * sizeof(char));
    cudaMalloc(&temp->__results, block_len * sizeof(int));
    cudaMalloc(&temp->__results_counter, sizeof(int));
    temp->__lset_uploaded = false;

    *ctx = temp;
    return CUDA_COVER_RESULT_OK;
}

cudacover_result_t cudacover_check(cudacover_t* ctx,
                                   int block_len) {
    if (ctx == NULL) {
        return CUDA_COVER_RESULT_ERR;
    }

    if (!ctx->__lset_uploaded) {
        ctx->__lset_uploaded = true;
        cudaMemcpy(ctx->__lset, ctx->lset, ctx->lset_len * ctx->features * sizeof(char), cudaMemcpyHostToDevice);
    }
    cudaMemcpy(ctx->__block, ctx->block, block_len * ctx->features * sizeof(char), cudaMemcpyHostToDevice);
    cudaMemcpy(ctx->__results_counter, &ctx->results_counter, sizeof(int), cudaMemcpyHostToDevice);

    dim3 blocks = dim3((block_len + THREAD_PER_BLOCK - 1)/THREAD_PER_BLOCK);
    dim3 threads = dim3(THREAD_PER_BLOCK);
    kernel<<<blocks, threads>>>(ctx->cover_koef,
                                ctx->features,
                                ctx->__lset,
                                ctx->lset_len,
                                ctx->__block,
                                block_len,
                                ctx->__results,
                                ctx->__results_counter);

    cudaMemcpy(&ctx->results_counter, ctx->__results_counter, sizeof(int), cudaMemcpyDeviceToHost);
    cudaMemcpy(ctx->results, ctx->__results, ctx->results_counter * sizeof(int), cudaMemcpyDeviceToHost);

    return CUDA_COVER_RESULT_OK;
}

cudacover_result_t cudacover_free(cudacover_t** ctx) {
    cudacover_t* temp = *ctx;
    if (temp == NULL) {
        return CUDA_COVER_RESULT_ERR;
    }

    cudaFree(temp->__lset);
    cudaFree(temp->__block);
    cudaFree(temp->__results_counter);
    cudaFree(temp->__results);

    free(temp->lset);
    free(temp->block);
    free(temp->results);
    free(temp);

    *ctx = NULL;
    return CUDA_COVER_RESULT_OK;
}
