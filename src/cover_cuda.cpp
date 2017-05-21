#include <argparse.h>

#include "global_settings.h"
#include "cover_cuda.h"
#include "cover_generator.hpp"
#include "resultset.hpp"
#include "timecollector.hpp"

const set_size_t DEFAULT_WORK_BLOCK = 1024;
const set_size_t DEFAULT_THREADS = 128;

parser_int_arg_t* work_block_arg;
parser_int_arg_t* thread_block_arg;

void initArgParser(parser_t* parser)
{
    parser_int_add_arg(parser, &work_block_arg, "--work-block");
    parser_int_set_alt(work_block_arg, "-b");
    parser_int_set_help(work_block_arg, "amount of rows in working block");
    parser_int_set_default(work_block_arg, DEFAULT_WORK_BLOCK);

    parser_int_add_arg(parser, &thread_block_arg, "--threads-block");
    parser_int_set_alt(thread_block_arg, "-t");
    parser_int_set_help(thread_block_arg, "amount of threads in block");
    parser_int_set_default(thread_block_arg, DEFAULT_THREADS);
}

void findCovering(feature_t* uim,
                  set_size_t uimSetLen,
                  feature_size_t featuresLen,
                  ResultSet& resultSet,
                  int limit,
                  int cover) {

    CoverGenerator generator(featuresLen);

    set_size_t workBlock = parser_int_get_value(work_block_arg);
    set_size_t threadBlock = parser_int_get_value(thread_block_arg);
    uint_fast32_t _count;

    cudacover_t* ctx;
    cudacover_init(&ctx, uimSetLen, featuresLen, workBlock);
    for (auto i=0; i<uimSetLen; ++i) {
        for (auto j=0; j<featuresLen; ++j) {
            ctx->lset[i * featuresLen + j] = uim[i * featuresLen + j];
        }
    }
    ctx->cover_koef = cover;

    for(;;) {
        if (resultSet.isFull()) {
            return;
        }

        if ((_count = generator.next((unsigned char*)ctx->block, workBlock)) == 0) {
            return;
        }

        cudacover_check(ctx, _count, threadBlock);

        for (int i = 0; i < ctx->results_counter; ++i) {
            Result result = Result((unsigned char*)&ctx->block[ctx->results[i] * featuresLen], featuresLen);
            resultSet.append(std::move(result));
        }
    }

    cudacover_free(&ctx);
}
