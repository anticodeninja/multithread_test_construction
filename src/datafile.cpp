#include "datafile.hpp"

#include <exception>
#include <sstream>
#include <stdexcept>

feature_t parseNext(std::istream& stream) {
    std::string temp;
    stream >> temp;

    if (temp == "-") {
        return DataFile::DASH;
    }

    return std::stoul(temp);
}

TestSet::TestSet():
    _testSet(nullptr) { }

TestSet::TestSet(TestSet&& source) :
    _testSet(source._testSet),
    _testSetLen(source._testSetLen) {
    source._testSet = nullptr;
}

TestSet::~TestSet() {
    if (_testSet != nullptr) {
        delete [] _testSet;
        _testSet = nullptr;
    }
}

void TestSet::setSet(feature_t* testSet, set_size_t testSetLen) {
    if (_testSet != nullptr) {
        delete [] _testSet;
    }

    _testSet = testSet;
    _testSetLen = testSetLen;
}

DataFile::DataFile():
    _learningSetLen(NOT_INITIALIZED),
    _featuresLen(NOT_INITIALIZED),
    _pfeaturesLen(NOT_INITIALIZED),
    _uimSetLen(NOT_INITIALIZED),
    _recognizeSetLen(NOT_INITIALIZED),
    _rangesCalculated(false),

    _learningSetFeatures(nullptr),
    _learningSetPfeatures(nullptr),
    _rangesMin(nullptr),
    _rangesMax(nullptr),
    _uimSet(nullptr),
    _uimWeights(nullptr),
    _recognizeSetFeatures(nullptr) { }

DataFile::~DataFile() {
    reset();
}

void DataFile::load(std::istream& inputStream) {
    reset();

    for (;;) {
        std::string header;

        if (!(inputStream >> header)) {
            break;
        } else if (header == "info:") {
            break;
        } else if (header == "learning_set:") {
            readLearningSetBlock(inputStream);
        } else if (header == "ranges:") {
            readRangesBlock(inputStream);
        } else if (header == "uim:") {
            readUimBlock(inputStream);
        } else if (header == "uim_weights:") {
            readUimWeightsBlock(inputStream);
        } else if (header == "recognize_set:") {
            readRecognizeSetBlock(inputStream);
        } else if (header == "test_set:") {
            readTestSetBlock(inputStream);
        } else {
            std::stringstream fmt;
            fmt << "Cannot parse input_stream, unknown block " << header;
            throw std::runtime_error(fmt.str());
        }
    }

    calc();
}

void DataFile::save(std::ostream& outputStream) {
    if (_learningSetFeatures != nullptr) {
        writeLearningSetBlock(outputStream);
    }

    if (_rangesMin != nullptr && !_rangesCalculated) {
        writeRangesBlock(outputStream);
    }

    if (_uimSet != nullptr) {
        writeUimBlock(outputStream);
    }

    if (_uimWeights != nullptr) {
        writeUimWeightsBlock(outputStream);
    }

    if (_recognizeSetFeatures != nullptr) {
        writeRecognizeSetBlock(outputStream);
    }

    writeTestSetBlock(outputStream);
}

void DataFile::reset() {
    _learningSetLen = NOT_INITIALIZED;
    _featuresLen = NOT_INITIALIZED;
    _pfeaturesLen = NOT_INITIALIZED;
    _uimSetLen = NOT_INITIALIZED;
    _recognizeSetLen = NOT_INITIALIZED;
    _rangesCalculated = false;

    if (_learningSetFeatures != nullptr) {
        delete [] _learningSetFeatures;
        _learningSetFeatures = nullptr;
    }

    if (_learningSetPfeatures != nullptr) {
        delete [] _learningSetPfeatures;
        _learningSetPfeatures = nullptr;
    }

    if (_rangesMin != nullptr) {
        delete [] _rangesMin;
        _rangesMin = nullptr;
    }

    if (_rangesMax != nullptr) {
        delete [] _rangesMax;
        _rangesMax = nullptr;
    }

    if (_uimSet != nullptr) {
        delete [] _uimSet;
        _uimSet = nullptr;
    }

    if (_uimWeights != nullptr) {
        delete [] _uimWeights;
        _uimWeights = nullptr;
    }

    if (_recognizeSetFeatures != nullptr) {
        delete [] _recognizeSetFeatures;
        _recognizeSetFeatures = nullptr;
    }
}

void DataFile::calc() {
}

void DataFile::setLearningSetBlock(uint32_t* learningSetFeatures,
                                   uint32_t* learningSetPfeatures,
                                   uint32_t learningSetLen,
                                   uint32_t featuresLen,
                                   uint32_t pfeaturesLen) {

    if (_learningSetLen != DASH && _learningSetLen != learningSetLen) {
        throw std::runtime_error("Invalid data, learningSetLength is not consist in different blocks");
    }
    if (_featuresLen != DASH && _featuresLen != featuresLen) {
        throw std::runtime_error("Invalid data, featuresLength is not consist in different blocks");
    }
    if (_pfeaturesLen != DASH && _pfeaturesLen != pfeaturesLen) {
        throw std::runtime_error("Invalid data, pfeaturesLength is not consist in different blocks");
    }
    if (_learningSetFeatures != nullptr) {
        delete [] _learningSetFeatures;
    }
    if (_learningSetPfeatures != nullptr) {
        delete [] _learningSetPfeatures;
    }

    _learningSetLen = learningSetLen;
    _featuresLen = featuresLen;
    _pfeaturesLen = pfeaturesLen;
    _learningSetFeatures = learningSetFeatures;
    _learningSetPfeatures = learningSetPfeatures;
}

void DataFile::readLearningSetBlock(std::istream& inputStream) {
    set_size_t learningSetLen;
    feature_size_t featuresLen, pfeaturesLen;
    inputStream >> learningSetLen >> featuresLen >> pfeaturesLen;

    feature_t* learningSetFeatures = new feature_t[learningSetLen * featuresLen];
    feature_t* learningSetPfeatures = new feature_t[learningSetLen * pfeaturesLen];

    for (auto i = 0; i < learningSetLen; ++i) {
        for (auto j = 0; j < featuresLen; ++j) {
            learningSetFeatures[featuresLen * i + j] = parseNext(inputStream);
        }
        for (auto j = 0; j < pfeaturesLen; ++j) {
            learningSetPfeatures[pfeaturesLen * i + j] = parseNext(inputStream);
        }
    }

    setLearningSetBlock(learningSetFeatures,
                        learningSetPfeatures,
                        learningSetLen,
                        featuresLen,
                        pfeaturesLen);
}

void DataFile::writeLearningSetBlock(std::ostream& outputStream) {
    outputStream << "learning_set: "
                 << _learningSetLen << " "
                 << _featuresLen << " "
                 << _pfeaturesLen << std::endl;

    for (auto i = 0; i < _learningSetLen; ++i) {
        for (auto j = 0; j < _featuresLen; ++j) {
            if (_learningSetFeatures[_featuresLen * i + j] != DASH) {
                outputStream << _learningSetFeatures[_featuresLen * i + j];
            } else {
                outputStream << "-";
            }
            outputStream << " ";
        }
        outputStream << " ";
        for (auto j = 0; j < _pfeaturesLen; ++j) {
            if (_learningSetFeatures[_pfeaturesLen * i + j] != DASH) {
                outputStream << _learningSetPfeatures[_pfeaturesLen * i + j];
            } else {
                outputStream << "-";
            }
            outputStream << " ";
        }
        outputStream << std::endl;
    }

    outputStream << std::endl;
}


void DataFile::setRangesBlock(feature_t* rangesMin,
                              feature_t* rangesMax,
                              feature_size_t featuresLen) {

    if (_featuresLen != DASH && _featuresLen != featuresLen) {
        throw std::runtime_error("Invalid data, featuresLength is not consist in different blocks");
    }
    if (_rangesMin != nullptr) {
        delete [] _rangesMin;
    }
    if (_rangesMax != nullptr) {
        delete [] _rangesMax;
    }

    _featuresLen = featuresLen;
    _rangesMin = rangesMin;
    _rangesMax = rangesMax;
}

void DataFile::readRangesBlock(std::istream& inputStream) {
    feature_size_t featuresLen;
    inputStream >> featuresLen;

    feature_t* rangesMin = new feature_t[featuresLen];
    feature_t* rangesMax = new feature_t[featuresLen];

    for (auto i = 0; i < featuresLen; ++i) {
        inputStream >> rangesMin[i];
    }
    for (auto i = 0; i < featuresLen; ++i) {
        inputStream >> rangesMax[i];
    }

    setRangesBlock(rangesMin,
                   rangesMax,
                   featuresLen);
}

void DataFile::writeRangesBlock(std::ostream& outputStream) {
    outputStream << "ranges: "
                 << _featuresLen << std::endl;

    for (auto i = 0; i < _featuresLen; ++i) {
        outputStream << _rangesMin[i] << " ";
    }
    outputStream << std::endl;

    for (auto i = 0; i < _featuresLen; ++i) {
        outputStream << _rangesMax[i] << " ";
    }
    outputStream << std::endl;

    outputStream << std::endl;
}


void DataFile::setUimBlock(feature_t* uimSet,
                           feature_size_t uimSetLen,
                           feature_size_t featuresLen) {

    if (_uimSetLen != DASH && _uimSetLen != uimSetLen) {
        throw std::runtime_error("Invalid data, uimSetLen is not consist in different blocks");
    }
    if (_featuresLen != DASH && _featuresLen != featuresLen) {
        throw std::runtime_error("Invalid data, featuresLength is not consist in different blocks");
    }
    if (_uimSet != nullptr) {
        delete [] _uimSet;
    }

    _uimSetLen = uimSetLen;
    _featuresLen = featuresLen;
    _uimSet = uimSet;
}

void DataFile::readUimBlock(std::istream& inputStream) {
    feature_size_t uimSetLen, featuresLen;
    inputStream >> uimSetLen >> featuresLen;

    feature_t* uimSet = new feature_t[uimSetLen * featuresLen];

    for (auto i = 0; i < uimSetLen; ++i) {
        for (auto j = 0; j < featuresLen; ++j) {
            inputStream >> uimSet[featuresLen * i + j];
        }
    }

    setUimBlock(uimSet,
                uimSetLen,
                featuresLen);
}

void DataFile::writeUimBlock(std::ostream& outputStream) {
    outputStream << "uim: "
                 << _uimSetLen << " "
                 << _featuresLen << std::endl;

    for (auto i = 0; i < _uimSetLen; ++i) {
        for (auto j = 0; j < _featuresLen; ++j) {
            outputStream << _uimSet[_featuresLen * i + j] << " ";
        }
        outputStream << std::endl;
    }

    outputStream << std::endl;
}


void DataFile::setUimWeightsBlock(feature_t* uimWeights,
                                  feature_size_t featuresLen) {

    if (_featuresLen != DASH && _featuresLen != featuresLen) {
        throw std::runtime_error("Invalid data, featuresLength is not consist in different blocks");
    }
    if (_uimWeights != nullptr) {
        delete [] _uimWeights;
    }

    _featuresLen = featuresLen;
    _uimWeights = uimWeights;
}

void DataFile::readUimWeightsBlock(std::istream& inputStream) {
    feature_size_t featuresLen;
    inputStream >> featuresLen;

    feature_t* uimWeights = new feature_t[featuresLen];

    for (auto i = 0; i < featuresLen; ++i) {
        inputStream >> uimWeights[i];
    }

    setUimWeightsBlock(uimWeights,
                   featuresLen);
}

void DataFile::writeUimWeightsBlock(std::ostream& outputStream) {
    outputStream << "uim_weights: "
                 << _featuresLen << std::endl;

    for (auto i = 0; i < _featuresLen; ++i) {
        outputStream << _uimWeights[i] << " ";
    }
    outputStream << std::endl;

    outputStream << std::endl;
}


void DataFile::setRecognizeSetBlock(feature_t* recognizeSetFeatures,
                                    feature_size_t recognizeSetLen,
                                    feature_size_t featuresLen) {

    if (_recognizeSetLen != DASH && _recognizeSetLen != recognizeSetLen) {
        throw std::runtime_error("Invalid data, recognizeSetLength is not consist in different blocks");
    }
    if (_featuresLen != DASH && _featuresLen != featuresLen) {
        throw std::runtime_error("Invalid data, featuresLength is not consist in different blocks");
    }
    if (_recognizeSetFeatures != nullptr) {
        delete [] _recognizeSetFeatures;
    }

    _recognizeSetLen = recognizeSetLen;
    _featuresLen = featuresLen;
    _recognizeSetFeatures = recognizeSetFeatures;
}

void DataFile::readRecognizeSetBlock(std::istream& inputStream) {
    feature_size_t recognizeSetLen, featuresLen;
    inputStream >> recognizeSetLen >> featuresLen;

    feature_t* recognizeSetFeatures = new feature_t[recognizeSetLen * featuresLen];

    for (auto i = 0; i < recognizeSetLen; ++i) {
        for (auto j = 0; j < featuresLen; ++j) {
            recognizeSetFeatures[featuresLen * i + j] = parseNext(inputStream);
        }
    }

    setRecognizeSetBlock(recognizeSetFeatures,
                         recognizeSetLen,
                         featuresLen);
}

void DataFile::writeRecognizeSetBlock(std::ostream& outputStream) {
    outputStream << "recognize_set: "
                 << _recognizeSetLen << " "
                 << _featuresLen << std::endl;

    for (auto i = 0; i < _recognizeSetLen; ++i) {
        for (auto j = 0; j < _featuresLen; ++j) {
            if (_recognizeSetFeatures[_featuresLen * i + j] != DASH) {
                outputStream << _recognizeSetFeatures[_featuresLen * i + j];
            } else {
                outputStream << "-";
            }
            outputStream << " ";
        }
        outputStream << std::endl;
    }

    outputStream << std::endl;
}


void DataFile::setTestSetBlock(feature_t* testSetFeatures,
                             feature_size_t hKoef,
                             feature_size_t testSetLen,
                             feature_size_t featuresLen) {

    if (_featuresLen != DASH && _featuresLen != featuresLen) {
        throw std::runtime_error("Invalid data, featuresLength is not consist in different blocks");
    }

    _featuresLen = featuresLen;

    auto keyPair = _testSets.find(hKoef);
    if (keyPair == _testSets.end()) {
        _testSets.emplace(hKoef, TestSet());
        keyPair = _testSets.find(hKoef);
    }

    keyPair->second.setSet(testSetFeatures, testSetLen);
}

void DataFile::readTestSetBlock(std::istream& inputStream) {
    feature_size_t hKoef, testSetLen, featuresLen;
    inputStream >> hKoef >> testSetLen >> featuresLen;

    feature_t* testSetFeatures = new feature_t[testSetLen * featuresLen];

    for (auto i = 0; i < testSetLen; ++i) {
        for (auto j = 0; j < featuresLen; ++j) {
            inputStream >> testSetFeatures[featuresLen * i + j];
        }
    }

    setTestSetBlock(testSetFeatures,
                    hKoef,
                    testSetLen,
                    featuresLen);
}

void DataFile::writeTestSetBlock(std::ostream& outputStream) {
    for (auto const & kv : _testSets) {
        outputStream << "test_set: "
                     << kv.first << " "
                     << kv.second.getLen() << " "
                     << _featuresLen << std::endl;

        for (auto i = 0; i < kv.second.getLen(); ++i) {
            for (auto j = 0; j < _featuresLen; ++j) {
                outputStream << kv.second.get(_featuresLen * i + j) << " ";
            }
            outputStream << std::endl;
        }

        outputStream << std::endl;
    }
}

