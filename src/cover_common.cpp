#include "cover_common.hpp"

#include <algorithm>
#include <stdexcept>
#include <vector>
#include <tuple>

void CoverCommon::calcPriorities(feature_t* uim,
                                 set_size_t uimSetLen,
                                 feature_size_t featuresLen,
                                 feature_size_t needCover,
                                 feature_size_t* currentCover,
                                 feature_size_t* currentColumns,
                                 bool& outUseAll,
                                 feature_size_t* outPriorities,
                                 feature_size_t& outPrioritiesLen) {

    DEBUG_INFO("calPriorities, input");
    debugPrintUim(uim, uimSetLen, featuresLen, currentColumns, currentCover);

    // Precalculate some common variables
    bool useMarkedColumns = false;
    bool markedColumns[featuresLen];
    set_size_t weights[featuresLen];
    std::fill(&markedColumns[0], &markedColumns[featuresLen], false);
    std::fill(&weights[0], &weights[featuresLen], false);

    for (auto i=0; i<uimSetLen; ++i) {
        feature_size_t count = 0;
        for (auto j=0; j<featuresLen; ++j) {
            count += uim[i * featuresLen + j];
            weights[j] += uim[i * featuresLen + j];
        }

        count += currentCover[i];

        if (count < needCover) {
            throw std::runtime_error("The covering cannot be found for the input data");
        } else if (count == needCover) {
            useMarkedColumns = true;
            for (auto j=0; j<featuresLen; ++j) {
                if (uim[i * featuresLen + j]) {
                    markedColumns[j] = true;
                }
            }
        }
    }

    outUseAll = useMarkedColumns;
    if (useMarkedColumns) {
        outPrioritiesLen = 0;
        for (auto j=0; j<featuresLen; ++j) {
            if (markedColumns[j]) {
                outPriorities[outPrioritiesLen++] = j;
            }
        }
    } else {
        std::tuple<set_size_t, feature_size_t> weights_sorted[featuresLen];
        for (auto j=0; j<featuresLen; ++j) {
            weights_sorted[j] = std::make_tuple(weights[j], j);
        }
        std::sort(&weights_sorted[0], &weights_sorted[featuresLen]);
        std::reverse(&weights_sorted[0], &weights_sorted[featuresLen]);

        outPrioritiesLen = 0;
        for (auto j=0; j<featuresLen; ++j) {
            if (std::get<0>(weights_sorted[j]) == 0) {
                break;
            }
            outPriorities[outPrioritiesLen++] = std::get<1>(weights_sorted[j]);
        }
    }

    DEBUG_BLOCK
        (
         getDebugStream() << "Calc priorities, useAll: " << outUseAll << ", c: ";
         for (auto j=0; j<outPrioritiesLen; ++j) {
             getDebugStream() << currentColumns[outPriorities[j]] << " ";
         }
         getDebugStream() << std::endl << std::endl;
         );
}

void CoverCommon::reduceUim(feature_t* uim,
                            set_size_t uimSetLen,
                            feature_size_t featuresLen,
                            feature_size_t* currentColumns,
                            feature_size_t* currentCover,
                            feature_t* newUim,
                            set_size_t& newUimSetLen,
                            feature_size_t& newFeaturesLen,
                            feature_size_t* newCurrentColumns,
                            feature_size_t* newCurrentCover,
                            feature_size_t needCover,
                            feature_size_t* columns,
                            feature_size_t columnsLen) {

    DEBUG_INFO("reduceUim, input");
    debugPrintUim(uim, uimSetLen, featuresLen, currentColumns, currentCover);

    newFeaturesLen = featuresLen - columnsLen;

    feature_size_t cs = 0;
    feature_size_t cj = 0;
    for (auto j=0; j<featuresLen; ++j) {
        if (cs < columnsLen && columns[cs] == j) {
            cs += 1;
            continue;
        }

        newCurrentColumns[cj++] = currentColumns[j];
    }

    newUimSetLen = 0;
    for (auto i=0; i<uimSetLen; ++i) {
        feature_size_t count = 0;
        for (auto j = 0; j < columnsLen; ++j) {
            count += uim[i * featuresLen + columns[j]];
        }
        count += currentCover[i];

        if (count < needCover) {
            newCurrentCover[newUimSetLen] = count;

            feature_size_t cs = 0;
            feature_size_t cj = 0;
            for (auto j=0; j<featuresLen; ++j) {
                if (cs < columnsLen && columns[cs] == j) {
                    cs += 1;
                    continue;
                }

                newUim[newFeaturesLen * newUimSetLen + (cj++)] = uim[i * featuresLen + j];
            }

            newUimSetLen += 1;
        }
    }

    DEBUG_INFO("reduceUim, output");
    debugPrintUim(newUim, newUimSetLen, newFeaturesLen, newCurrentColumns, newCurrentCover);
}

void CoverCommon::reorderUim(feature_t* uim,
                             set_size_t uimSetLen,
                             feature_size_t featuresLen,
                             feature_size_t* currentColumns,
                             feature_t* newUim,
                             set_size_t& newUimSetLen,
                             feature_size_t& newFeaturesLen,
                             feature_size_t* newCurrentColumns,
                             feature_size_t* order,
                             feature_size_t orderLen) {

    DEBUG_INFO("reorderUim, input");
    debugPrintUim(uim, uimSetLen, featuresLen, currentColumns, nullptr);

    newFeaturesLen = orderLen;
    feature_size_t tempCurrentColumns[orderLen];
    for (auto j=0; j<orderLen; ++j) {
        tempCurrentColumns[j] = currentColumns[order[j]];
    }
    for (auto j=0; j<orderLen; ++j) {
        newCurrentColumns[j] = tempCurrentColumns[j];
    }

    newUimSetLen = uimSetLen;
    feature_t tempUimRow[orderLen];
    for (auto i=0; i<uimSetLen; ++i) {
        for (auto j=0; j<orderLen; ++j) {
            tempUimRow[j] = uim[i * featuresLen + order[j]];
        }
        for (auto j=0; j<orderLen; ++j) {
            uim[i * orderLen + j] = tempUimRow[j];
        }
    }

    DEBUG_INFO("reorderUim, output");
    debugPrintUim(newUim, newUimSetLen, newFeaturesLen, newCurrentColumns, nullptr);
}

#ifdef DEBUG_MODE
void CoverCommon::debugPrintUim(feature_t* uim,
                                set_size_t uimSetLen,
                                feature_size_t featuresLen,
                                feature_size_t* currentColumns,
                                feature_size_t* currentCover)
{
    DEBUG_BLOCK
        (
         for (auto j=0; j<featuresLen; ++j) {
             getDebugStream() << std::setw(3)
                              << (currentColumns != nullptr ? currentColumns[j] : j);
         }
         getDebugStream() << std::endl;

         for (auto i=0; i<uimSetLen; ++i) {
             for (auto j=0; j<featuresLen; ++j) {
                 getDebugStream() << std::setw(3) << (uim[i* featuresLen + j] ? 1 : 0);
             }
             if (currentCover != nullptr) {
                 getDebugStream() << "  " << currentCover[i];
             }

             getDebugStream() << std::endl;
         }
         );
}
#endif // DEBUG_MODE
