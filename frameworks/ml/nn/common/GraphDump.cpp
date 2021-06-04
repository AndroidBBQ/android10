/*
 * Copyright (C) 2017 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#define LOG_TAG "GraphDump"

#include "GraphDump.h"

#include "HalInterfaces.h"

#include <android-base/logging.h>
#include <set>
#include <iostream>
#include <sstream>

namespace android {
namespace nn {

// class Dumper is a wrapper around an std::ostream (if instantiated
// with a pointer to a stream) or around LOG(INFO) (otherwise).
//
// Send fragments of output to it with operator<<(), as per usual
// stream conventions.  Unlike with LOG(INFO), there is no implicit
// end-of-line.  To end a line, send Dumper::endl.
//
// Example:
//
//   Dumper d(nullptr);  // will go to LOG(INFO)
//   d << "These words are";
//   d << " all" << " on";
//   d << " the same line." << Dumper::endl;
//
namespace {
class Dumper {
public:
    Dumper(std::ostream* outStream) : mStream(outStream) { }

    Dumper(const Dumper&) = delete;
    void operator=(const Dumper&) = delete;

    template <typename T>
    Dumper& operator<<(const T& val) {
        mStringStream << val;
        return *this;
    }

    class EndlType { };

    Dumper& operator<<(EndlType) {
        if (mStream) {
            *mStream << mStringStream.str() << std::endl;
        } else {
            // TODO: There is a limit of how long a single LOG line
            // can be; extra characters are truncated.  (See
            // LOGGER_ENTRY_MAX_PAYLOAD and LOGGER_ENTRY_MAX_LEN.)  We
            // may want to figure out the linebreak rules for the .dot
            // format and try to ensure that we generate correct .dot
            // output whose lines do not exceed some maximum length.
            // The intelligence for breaking the lines might have to
            // live in graphDump() rather than in the Dumper class, so
            // that it can be sensitive to the .dot format.
            LOG(INFO) << mStringStream.str();
        }
        std::ostringstream empty;
        std::swap(mStringStream, empty);
        return *this;
    }

    static const EndlType endl;
private:
    std::ostream* mStream;
    std::ostringstream mStringStream;
};

const Dumper::EndlType Dumper::endl;
}


// Provide short name for OperandType value.
static std::string translate(OperandType type) {
    switch (type) {
        case OperandType::FLOAT32:             return "F32";
        case OperandType::INT32:               return "I32";
        case OperandType::UINT32:              return "U32";
        case OperandType::TENSOR_FLOAT32:      return "TF32";
        case OperandType::TENSOR_INT32:        return "TI32";
        case OperandType::TENSOR_QUANT8_ASYMM: return "TQ8A";
        case OperandType::OEM:                 return "OEM";
        case OperandType::TENSOR_OEM_BYTE:     return "TOEMB";
        default:                               return toString(type);
    }
}

// If the specified Operand of the specified Model has OperandType
// nnType corresponding to C++ type cppType and is of
// OperandLifeTime::CONSTANT_COPY, then write the Operand's value to
// the Dumper.
namespace {
template<OperandType nnType, typename cppType>
void tryValueDump(Dumper& dump, const Model& model, const Operand& opnd) {
    if (opnd.type != nnType ||
        opnd.lifetime != OperandLifeTime::CONSTANT_COPY ||
        opnd.location.length != sizeof(cppType)) {
        return;
    }

    cppType val;
    memcpy(&val, &model.operandValues[opnd.location.offset], sizeof(cppType));
    dump << " = " << val;
}
}

void graphDump(const char* name, const Model& model, std::ostream* outStream) {
    // Operand nodes are named "d" (operanD) followed by operand index.
    // Operation nodes are named "n" (operatioN) followed by operation index.
    // (These names are not the names that are actually displayed -- those
    //  names are given by the "label" attribute.)

    Dumper dump(outStream);

    dump << "// " << name << Dumper::endl;
    dump << "digraph {" << Dumper::endl;

    // model inputs and outputs
    std::set<uint32_t> modelIO;
    for (unsigned i = 0, e = model.inputIndexes.size(); i < e; i++) {
        modelIO.insert(model.inputIndexes[i]);
    }
    for (unsigned i = 0, e = model.outputIndexes.size(); i < e; i++) {
        modelIO.insert(model.outputIndexes[i]);
    }

    // model operands
    for (unsigned i = 0, e = model.operands.size(); i < e; i++) {
        dump << "    d" << i << " [";
        if (modelIO.count(i)) {
            dump << "style=filled fillcolor=black fontcolor=white ";
        }
        dump << "label=\"" << i;
        const Operand& opnd = model.operands[i];
        const char* kind = nullptr;
        switch (opnd.lifetime) {
            case OperandLifeTime::CONSTANT_COPY:
                kind = "COPY";
                break;
            case OperandLifeTime::CONSTANT_REFERENCE:
                kind = "REF";
                break;
            case OperandLifeTime::NO_VALUE:
                kind = "NO";
                break;
            default:
                // nothing interesting
                break;
        }
        if (kind) {
            dump << ": " << kind;
        }
        dump << "\\n" << translate(opnd.type);
        tryValueDump<OperandType::FLOAT32,   float>(dump, model, opnd);
        tryValueDump<OperandType::INT32,       int>(dump, model, opnd);
        tryValueDump<OperandType::UINT32, unsigned>(dump, model, opnd);
        if (opnd.dimensions.size()) {
            dump << "(";
            for (unsigned i = 0, e = opnd.dimensions.size(); i < e; i++) {
                if (i > 0) {
                    dump << "x";
                }
                dump << opnd.dimensions[i];
            }
            dump << ")";
        }
        dump << "\"]" << Dumper::endl;
    }

    // model operations
    for (unsigned i = 0, e = model.operations.size(); i < e; i++) {
        const Operation& operation = model.operations[i];
        dump << "    n" << i << " [shape=box";
        const uint32_t maxArity = std::max(operation.inputs.size(), operation.outputs.size());
        if (maxArity > 1) {
            if (maxArity == operation.inputs.size()) {
                dump << " ordering=in";
            } else {
                dump << " ordering=out";
            }
        }
        dump << " label=\"" << i << ": "
             << toString(operation.type) << "\"]" << Dumper::endl;
        {
            // operation inputs
            for (unsigned in = 0, inE = operation.inputs.size(); in < inE; in++) {
                dump << "    d" << operation.inputs[in] << " -> n" << i;
                if (inE > 1) {
                    dump << " [label=" << in << "]";
                }
                dump << Dumper::endl;
            }
        }

        {
            // operation outputs
            for (unsigned out = 0, outE = operation.outputs.size(); out < outE; out++) {
                dump << "    n" << i << " -> d" << operation.outputs[out];
                if (outE > 1) {
                    dump << " [label=" << out << "]";
                }
                dump << Dumper::endl;
            }
        }
    }
    dump << "}" << Dumper::endl;
}

}  // namespace nn
}  // namespace android
