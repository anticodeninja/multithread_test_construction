#ifndef DATAFILE_H
#define DATAFILE_H

#include <cstdint>
#include <iostream>
#include <limits>
#include <map>

#include "global_settings.h"

class TestSet {

public:
    TestSet();
    virtual ~TestSet();

    TestSet(const TestSet&) = delete;
    TestSet(TestSet&& source);
    TestSet& operator=(const TestSet&) = delete;

    void setSet(feature_t* testSet, set_size_t testSetLen);
    inline feature_t get(set_size_t id) const { return _testSet[id]; }
    inline set_size_t getLen() const { return _testSetLen; }

private:
    set_size_t _testSetLen;
    feature_t* _testSet;

};

class DataFile {

public:
    const int NOT_INITIALIZED = std::numeric_limits<set_size_t>::max();
    const int DASH = std::numeric_limits<feature_t>::max();

public:
    DataFile();
    virtual ~DataFile();

    DataFile(const DataFile&) = delete;
    DataFile& operator=(const DataFile&) = delete;

    void load(std::istream& inputStream);
    void save(std::ostream& outputStream);
    void reset();
    void transfer(const DataFile& source);
    void calc();

    void setLearningSetBlock(feature_t* learningSetFeatures,
                             feature_t* learningSetPfeatures,
                             set_size_t learningSetLen,
                             feature_size_t featuresLen,
                             set_size_t pfeaturesLen);

    void setRangesBlock(feature_t* rangesMin,
                        feature_t* rangesMax,
                        feature_size_t featuresLen);

    void setUimBlock(feature_t* uimSet,
                     set_size_t uimSetLen,
                     feature_size_t featuresLen);

    void setUimWeightsBlock(feature_t* uimWeights,
                            feature_size_t featuresLen);

    void setRecognizeSetBlock(feature_t* recognizeSetFeatures,
                              set_size_t recognizeSetLen,
                              feature_size_t featuresLen);

    void setTestSetBlock(feature_t* testSetFeatures,
                         feature_size_t hKoef,
                         set_size_t testSetLen,
                         feature_size_t featuresLen);

    inline set_size_t getLearningSetLen() const { return _learningSetLen; }
    inline feature_size_t getFeaturesLen() const { return _featuresLen; }
    inline feature_size_t getPfeaturesLen() const { return _pfeaturesLen; }
    inline set_size_t getUimSetLen() const { return _uimSetLen; }
    inline set_size_t getRecognizeSetLen() const { return _recognizeSetLen; }
    inline bool getRangesCalculated() const { return _rangesCalculated; }

    inline feature_t* getLearningSetFeatures() const { return _learningSetFeatures; }
    inline feature_t* getLearningSetPfeatures() const { return _learningSetPfeatures; }
    inline feature_t* getRangesMin() const { return _rangesMin; }
    inline feature_t* getRangesMax() const { return _rangesMax; }
    inline feature_t* getUimSet() const { return _uimSet; }
    inline feature_t* getUimWeights() const { return _uimWeights; }
    inline feature_t* getRecognizeSetFeatures() const { return _recognizeSetFeatures; }

private:
    void readLearningSetBlock(std::istream& inputStream);
    void readRangesBlock(std::istream& inputStream);
    void readRecognizeSetBlock(std::istream& inputStream);
    void readUimBlock(std::istream& inputStream);
    void readUimWeightsBlock(std::istream& inputStream);
    void readTestSetBlock(std::istream& inputStream);

    void writeLearningSetBlock(std::ostream& outputStream);
    void writeRangesBlock(std::ostream& outputStream);
    void writeRecognizeSetBlock(std::ostream& outputStream);
    void writeUimBlock(std::ostream& outputStream);
    void writeUimWeightsBlock(std::ostream& outputStream);
    void writeTestSetBlock(std::ostream& outputStream);

private:
    set_size_t _learningSetLen;
    feature_size_t _featuresLen;
    feature_size_t _pfeaturesLen;
    set_size_t _uimSetLen;
    set_size_t _recognizeSetLen;
    bool _rangesCalculated;

    feature_t* _learningSetFeatures;
    feature_t* _learningSetPfeatures;
    feature_t* _rangesMin;
    feature_t* _rangesMax;
    feature_t* _uimSet;
    feature_t* _uimWeights;
    feature_t* _recognizeSetFeatures;
    std::map<feature_size_t, TestSet> _testSets;
};

#endif // DATAFILE_H
