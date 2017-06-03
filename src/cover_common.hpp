#pragma once

#include "global_settings.h"

class CoverCommon {

public:
    static void binarize(feature_t* uim,
                         set_size_t uimSetLen,
                         feature_size_t featuresLen,
                         test_feature_t* newUim);

    static void calcPriorities(test_feature_t* uim,
                               set_size_t uimSetLen,
                               feature_size_t featuresLen,
                               feature_size_t needCover,
                               feature_size_t* currentCover,
                               feature_size_t* currentColumns,
                               bool& outUseAll,
                               feature_size_t* outPriorities,
                               feature_size_t& outPrioritiesLen);

    static void reduceUim(test_feature_t* uim,
                          set_size_t uimSetLen,
                          feature_size_t featuresLen,
                          feature_size_t* currentColumns,
                          feature_size_t* currentCover,
                          test_feature_t* newUim,
                          set_size_t& newUimSetLen,
                          feature_size_t& newFeaturesLen,
                          feature_size_t* newCurrentColumns,
                          feature_size_t* newCurrentCover,
                          feature_size_t needCover,
                          feature_size_t* columns,
                          feature_size_t columnsLen);

    static void reorderUim(test_feature_t* uim,
                           set_size_t uimSetLen,
                           feature_size_t featuresLen,
                           feature_size_t* currentColumns,
                           test_feature_t* newUim,
                           set_size_t& newUimSetLen,
                           feature_size_t& newFeatureLen,
                           feature_size_t* newCurrentColumns,
                           feature_size_t* order,
                           feature_size_t orderLen);

    static void debugPrintUim(test_feature_t* uim,
                              set_size_t uimSetLen,
                              feature_size_t featuresLen,
                              feature_size_t* currentColumns,
                              feature_size_t* currentCover)
        IFN_DEBUG_OUTPUT({});
};
