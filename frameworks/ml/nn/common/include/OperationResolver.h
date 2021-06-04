/*
 * Copyright (C) 2018 The Android Open Source Project
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

#ifndef ANDROID_ML_NN_COMMON_OPERATION_RESOLVER_H
#define ANDROID_ML_NN_COMMON_OPERATION_RESOLVER_H

#include "HalInterfaces.h"
#include "OperationsUtils.h"

namespace android {
namespace nn {

// Encapsulates an operation implementation.
struct OperationRegistration {
    OperationType type;
    const char* name;

    // Validates operand types, shapes, and any values known during graph creation.
    std::function<bool(const IOperationValidationContext*)> validate;

    // prepare is called when the inputs this operation depends on have been
    // computed. Typically, prepare does any remaining validation and sets
    // output shapes via context->setOutputShape(...).
    std::function<bool(IOperationExecutionContext*)> prepare;

    // Executes the operation, reading from context->getInputBuffer(...)
    // and writing to context->getOutputBuffer(...).
    std::function<bool(IOperationExecutionContext*)> execute;

    struct Flag {
        // Whether the operation allows at least one operand to be omitted.
        bool allowOmittedOperand = false;
        // Whether the operation allows at least one input operand to be a zero-sized tensor.
        bool allowZeroSizedInput = false;
    } flags;

    OperationRegistration(OperationType type, const char* name,
                          std::function<bool(const IOperationValidationContext*)> validate,
                          std::function<bool(IOperationExecutionContext*)> prepare,
                          std::function<bool(IOperationExecutionContext*)> execute, Flag flags)
        : type(type),
          name(name),
          validate(validate),
          prepare(prepare),
          execute(execute),
          flags(flags) {}
};

// A registry of operation implementations.
class IOperationResolver {
   public:
    virtual const OperationRegistration* findOperation(OperationType operationType) const = 0;
    virtual ~IOperationResolver() {}
};

// A registry of builtin operation implementations.
//
// Note that some operations bypass BuiltinOperationResolver (b/124041202).
//
// Usage:
//   const OperationRegistration* operationRegistration =
//           BuiltinOperationResolver::get()->findOperation(operationType);
//   NN_RET_CHECK(operationRegistration != nullptr);
//   NN_RET_CHECK(operationRegistration->validate != nullptr);
//   NN_RET_CHECK(operationRegistration->validate(&context));
//
class BuiltinOperationResolver : public IOperationResolver {
    DISALLOW_COPY_AND_ASSIGN(BuiltinOperationResolver);

   public:
    static const BuiltinOperationResolver* get() {
        static BuiltinOperationResolver instance;
        return &instance;
    }

    const OperationRegistration* findOperation(OperationType operationType) const override;

   private:
    BuiltinOperationResolver();

    void registerOperation(const OperationRegistration* operationRegistration);

    const OperationRegistration* mRegistrations[kNumberOfOperationTypes] = {};
};

// NN_REGISTER_OPERATION creates OperationRegistration for consumption by
// OperationResolver.
//
// Usage:
// (check OperationRegistration::Flag for available fields and default values.)
//
// - With default flags.
//   NN_REGISTER_OPERATION(FOO_OP, foo_op::kOperationName, foo_op::validate,
//                         foo_op::prepare, foo_op::execute);
//
// - With a customized flag.
//   NN_REGISTER_OPERATION(FOO_OP, foo_op::kOperationName, foo_op::validate,
//                         foo_op::prepare, foo_op::execute, .allowZeroSizedInput = true);
//
// - With multiple customized flags.
//   NN_REGISTER_OPERATION(FOO_OP, foo_op::kOperationName, foo_op::validate,
//                         foo_op::prepare, foo_op::execute, .allowOmittedOperand = true,
//                         .allowZeroSizedInput = true);
//
#ifdef NN_INCLUDE_CPU_IMPLEMENTATION
#define NN_REGISTER_OPERATION(identifier, operationName, validate, prepare, execute, ...)     \
    const OperationRegistration* register_##identifier() {                                    \
        static OperationRegistration registration(OperationType::identifier, operationName,   \
                                                  validate, prepare, execute, {__VA_ARGS__}); \
        return &registration;                                                                 \
    }
#else
// This version ignores CPU execution logic (prepare and execute).
// The compiler is supposed to omit that code so that only validation logic
// makes it into libneuralnetworks_utils.
#define NN_REGISTER_OPERATION(identifier, operationName, validate, unused_prepare, unused_execute, \
                              ...)                                                                 \
    const OperationRegistration* register_##identifier() {                                         \
        static OperationRegistration registration(OperationType::identifier, operationName,        \
                                                  validate, nullptr, nullptr, {__VA_ARGS__});      \
        return &registration;                                                                      \
    }
#endif

}  // namespace nn
}  // namespace android

#endif  // ANDROID_ML_NN_COMMON_OPERATION_RESOLVER_H
