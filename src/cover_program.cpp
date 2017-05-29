#include <argparse.h>
#include <fstream>

#include "global_settings.h"
#include "timecollector.hpp"
#include "datafile.hpp"
#include "resultset.hpp"

INIT_DEBUG_OUTPUT();

void initArgParser(parser_t* parser);

void findCovering(feature_t* uim,
                  set_size_t uimSetLen,
                  feature_size_t featuresLen,
                  ResultSet& resultSet,
                  set_size_t limit,
                  feature_size_t covering);

int main(int argc, char** argv)
{
    parser_t* parser;
    parser_init(&parser);

    parser_string_arg_t* input_arg;
    parser_string_add_arg(parser, &input_arg, "input");
    parser_string_set_help(input_arg, "input file");

    parser_string_arg_t* output_arg;
    parser_string_add_arg(parser, &output_arg, "output");
    parser_string_set_help(output_arg, "output file");

    parser_int_arg_t* covering_arg;
    parser_int_add_arg(parser, &covering_arg, "--covering");
    parser_int_set_alt(covering_arg, "-c");
    parser_int_set_help(covering_arg, "amount of column which should be covered");
    parser_int_set_default(covering_arg, 1);

    parser_int_arg_t* result_limit_arg;
    parser_int_add_arg(parser, &result_limit_arg, "--result-limit");
    parser_int_set_alt(result_limit_arg, "-l");
    parser_int_set_help(result_limit_arg, "maximal amount of result");
    parser_int_set_default(result_limit_arg, 10);

    parser_flag_arg_t* no_transfer;
    parser_flag_add_arg(parser, &no_transfer, "--no-transfer");
    parser_flag_set_help(no_transfer, "no transfer blocks from input file to output");

    initArgParser(parser);

    if (parser_parse(parser, argc, argv) != PARSER_RESULT_OK) {
        printf("%s", parser_get_last_err(parser));
        parser_free(&parser);
        return 1;
    }

    TimeCollector::Initialize();
    TimeCollector::ThreadInitialize();
    TimeCollectorEntry executionTime(Counters::All);

    START_COLLECT_TIME(readingInput, Counters::ReadingInput);
    DataFile dataFile;
    if (strcmp("-", parser_string_get_value(input_arg)) != 0) {
        std::ifstream input_stream(parser_string_get_value(input_arg));
        dataFile.load(input_stream);
    } else {
        dataFile.load(std::cin);
    }

    auto uimSetLen = dataFile.getUimSetLen();
    auto featuresLen = dataFile.getFeaturesLen();
    auto uim = dataFile.getUimSet();
    STOP_COLLECT_TIME(readingInput);

    ResultSet resultSet(parser_int_get_value(result_limit_arg), featuresLen);

    findCovering(uim,
                 uimSetLen,
                 featuresLen,
                 resultSet,
                 parser_int_get_value(result_limit_arg),
                 parser_int_get_value(covering_arg));

    if (parser_flag_is_filled(no_transfer)) {
        dataFile.reset();
    }

    auto tests = new feature_t[resultSet.getSize() * featuresLen];
    for(auto i=0; i<resultSet.getSize(); ++i) {
        for(auto j=0; j<featuresLen; ++j) {
            tests[i * featuresLen + j] = resultSet.get(i)[j];
        }
    }
    dataFile.setTestSetBlock(tests,
                             parser_int_get_value(covering_arg),
                             resultSet.getSize(),
                             featuresLen);

    if (strcmp("-", output_arg->value) != 0) {
        std::ofstream output_stream(output_arg->value);
        dataFile.save(output_stream);
    } else {
        dataFile.save(std::cout);
    }

    executionTime.Stop();

    TimeCollector::ThreadFinalize();
    std::ofstream timeCollectorOutput("current_profile.txt");
    TimeCollector::PrintInfo(timeCollectorOutput);

    parser_free(&parser);
    return 0;
}

