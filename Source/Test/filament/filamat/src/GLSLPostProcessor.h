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

#ifndef TNT_GLSLPOSTPROCESSOR_H
#define TNT_GLSLPOSTPROCESSOR_H

#include <filamat/MaterialBuilder.h>    // for MaterialBuilder:: enums

#include <private/filament/Variant.h>

#include "ShaderMinifier.h"

 // Copyright (c) 2016 Google Inc.
 //
 // Licensed under the Apache License, Version 2.0 (the "License");
 // you may not use this file except in compliance with the License.
 // You may obtain a copy of the License at
 //
 //     http://www.apache.org/licenses/LICENSE-2.0
 //
 // Unless required by applicable law or agreed to in writing, software
 // distributed under the License is distributed on an "AS IS" BASIS,
 // WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 // See the License for the specific language governing permissions and
 // limitations under the License.

#ifndef INCLUDE_SPIRV_TOOLS_OPTIMIZER_HPP_
#define INCLUDE_SPIRV_TOOLS_OPTIMIZER_HPP_

#include <memory>
#include <ostream>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

// Copyright (c) 2016 Google Inc.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#ifndef INCLUDE_SPIRV_TOOLS_LIBSPIRV_HPP_
#define INCLUDE_SPIRV_TOOLS_LIBSPIRV_HPP_

#include <functional>
#include <memory>
#include <string>
#include <vector>
// Copyright (c) 2015-2020 The Khronos Group Inc.
// Modifications Copyright (C) 2020 Advanced Micro Devices, Inc. All rights
// reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#ifndef INCLUDE_SPIRV_TOOLS_LIBSPIRV_H_
#define INCLUDE_SPIRV_TOOLS_LIBSPIRV_H_

#ifdef __cplusplus
extern "C" {
#else
#include <stdbool.h>
#endif

#include <stddef.h>
#include <stdint.h>

#if defined(SPIRV_TOOLS_SHAREDLIB)
#if defined(_WIN32)
#if defined(SPIRV_TOOLS_IMPLEMENTATION)
#define SPIRV_TOOLS_EXPORT __declspec(dllexport)
#else
#define SPIRV_TOOLS_EXPORT __declspec(dllimport)
#endif
#else
#if defined(SPIRV_TOOLS_IMPLEMENTATION)
#define SPIRV_TOOLS_EXPORT __attribute__((visibility("default")))
#else
#define SPIRV_TOOLS_EXPORT
#endif
#endif
#else
#define SPIRV_TOOLS_EXPORT
#endif

// Helpers

#define SPV_BIT(shift) (1 << (shift))

#define SPV_FORCE_16_BIT_ENUM(name) SPV_FORCE_16BIT_##name = 0x7fff
#define SPV_FORCE_32_BIT_ENUM(name) SPV_FORCE_32BIT_##name = 0x7fffffff

// Enumerations

typedef enum spv_result_t {
    SPV_SUCCESS = 0,
    SPV_UNSUPPORTED = 1,
    SPV_END_OF_STREAM = 2,
    SPV_WARNING = 3,
    SPV_FAILED_MATCH = 4,
    SPV_REQUESTED_TERMINATION = 5,  // Success, but signals early termination.
    SPV_ERROR_INTERNAL = -1,
    SPV_ERROR_OUT_OF_MEMORY = -2,
    SPV_ERROR_INVALID_POINTER = -3,
    SPV_ERROR_INVALID_BINARY = -4,
    SPV_ERROR_INVALID_TEXT = -5,
    SPV_ERROR_INVALID_TABLE = -6,
    SPV_ERROR_INVALID_VALUE = -7,
    SPV_ERROR_INVALID_DIAGNOSTIC = -8,
    SPV_ERROR_INVALID_LOOKUP = -9,
    SPV_ERROR_INVALID_ID = -10,
    SPV_ERROR_INVALID_CFG = -11,
    SPV_ERROR_INVALID_LAYOUT = -12,
    SPV_ERROR_INVALID_CAPABILITY = -13,
    SPV_ERROR_INVALID_DATA = -14,  // Indicates data rules validation failure.
    SPV_ERROR_MISSING_EXTENSION = -15,
    SPV_ERROR_WRONG_VERSION = -16,  // Indicates wrong SPIR-V version
    SPV_FORCE_32_BIT_ENUM(spv_result_t)
} spv_result_t;

// Severity levels of messages communicated to the consumer.
typedef enum spv_message_level_t {
    SPV_MSG_FATAL,           // Unrecoverable error due to environment.
    // Will exit the program immediately. E.g.,
    // out of memory.
    SPV_MSG_INTERNAL_ERROR,  // Unrecoverable error due to SPIRV-Tools
    // internals.
    // Will exit the program immediately. E.g.,
    // unimplemented feature.
    SPV_MSG_ERROR,           // Normal error due to user input.
    SPV_MSG_WARNING,         // Warning information.
    SPV_MSG_INFO,            // General information.
    SPV_MSG_DEBUG,           // Debug information.
} spv_message_level_t;

typedef enum spv_endianness_t {
    SPV_ENDIANNESS_LITTLE,
    SPV_ENDIANNESS_BIG,
    SPV_FORCE_32_BIT_ENUM(spv_endianness_t)
} spv_endianness_t;

// The kinds of operands that an instruction may have.
//
// Some operand types are "concrete".  The binary parser uses a concrete
// operand type to describe an operand of a parsed instruction.
//
// The assembler uses all operand types.  In addition to determining what
// kind of value an operand may be, non-concrete operand types capture the
// fact that an operand might be optional (may be absent, or present exactly
// once), or might occur zero or more times.
//
// Sometimes we also need to be able to express the fact that an operand
// is a member of an optional tuple of values.  In that case the first member
// would be optional, and the subsequent members would be required.
//
// NOTE: Although we don't promise binary compatibility, as a courtesy, please
// add new enum values at the end.
typedef enum spv_operand_type_t {
    // A sentinel value.
    SPV_OPERAND_TYPE_NONE = 0,

    // Set 1:  Operands that are IDs.
    SPV_OPERAND_TYPE_ID,
    SPV_OPERAND_TYPE_TYPE_ID,
    SPV_OPERAND_TYPE_RESULT_ID,
    SPV_OPERAND_TYPE_MEMORY_SEMANTICS_ID,  // SPIR-V Sec 3.25
    SPV_OPERAND_TYPE_SCOPE_ID,             // SPIR-V Sec 3.27

    // Set 2:  Operands that are literal numbers.
    SPV_OPERAND_TYPE_LITERAL_INTEGER,  // Always unsigned 32-bits.
    // The Instruction argument to OpExtInst. It's an unsigned 32-bit literal
    // number indicating which instruction to use from an extended instruction
    // set.
    SPV_OPERAND_TYPE_EXTENSION_INSTRUCTION_NUMBER,
    // The Opcode argument to OpSpecConstantOp. It determines the operation
    // to be performed on constant operands to compute a specialization constant
    // result.
    SPV_OPERAND_TYPE_SPEC_CONSTANT_OP_NUMBER,
    // A literal number whose format and size are determined by a previous operand
    // in the same instruction.  It's a signed integer, an unsigned integer, or a
    // floating point number.  It also has a specified bit width.  The width
    // may be larger than 32, which would require such a typed literal value to
    // occupy multiple SPIR-V words.
    SPV_OPERAND_TYPE_TYPED_LITERAL_NUMBER,

    // Set 3:  The literal string operand type.
    SPV_OPERAND_TYPE_LITERAL_STRING,

    // Set 4:  Operands that are a single word enumerated value.
    SPV_OPERAND_TYPE_SOURCE_LANGUAGE,               // SPIR-V Sec 3.2
    SPV_OPERAND_TYPE_EXECUTION_MODEL,               // SPIR-V Sec 3.3
    SPV_OPERAND_TYPE_ADDRESSING_MODEL,              // SPIR-V Sec 3.4
    SPV_OPERAND_TYPE_MEMORY_MODEL,                  // SPIR-V Sec 3.5
    SPV_OPERAND_TYPE_EXECUTION_MODE,                // SPIR-V Sec 3.6
    SPV_OPERAND_TYPE_STORAGE_CLASS,                 // SPIR-V Sec 3.7
    SPV_OPERAND_TYPE_DIMENSIONALITY,                // SPIR-V Sec 3.8
    SPV_OPERAND_TYPE_SAMPLER_ADDRESSING_MODE,       // SPIR-V Sec 3.9
    SPV_OPERAND_TYPE_SAMPLER_FILTER_MODE,           // SPIR-V Sec 3.10
    SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT,          // SPIR-V Sec 3.11
    SPV_OPERAND_TYPE_IMAGE_CHANNEL_ORDER,           // SPIR-V Sec 3.12
    SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE,       // SPIR-V Sec 3.13
    SPV_OPERAND_TYPE_FP_ROUNDING_MODE,              // SPIR-V Sec 3.16
    SPV_OPERAND_TYPE_LINKAGE_TYPE,                  // SPIR-V Sec 3.17
    SPV_OPERAND_TYPE_ACCESS_QUALIFIER,              // SPIR-V Sec 3.18
    SPV_OPERAND_TYPE_FUNCTION_PARAMETER_ATTRIBUTE,  // SPIR-V Sec 3.19
    SPV_OPERAND_TYPE_DECORATION,                    // SPIR-V Sec 3.20
    SPV_OPERAND_TYPE_BUILT_IN,                      // SPIR-V Sec 3.21
    SPV_OPERAND_TYPE_GROUP_OPERATION,               // SPIR-V Sec 3.28
    SPV_OPERAND_TYPE_KERNEL_ENQ_FLAGS,              // SPIR-V Sec 3.29
    SPV_OPERAND_TYPE_KERNEL_PROFILING_INFO,         // SPIR-V Sec 3.30
    SPV_OPERAND_TYPE_CAPABILITY,                    // SPIR-V Sec 3.31

    // NOTE: New concrete enum values should be added at the end.

    // Set 5:  Operands that are a single word bitmask.
    // Sometimes a set bit indicates the instruction requires still more operands.
    SPV_OPERAND_TYPE_IMAGE,                  // SPIR-V Sec 3.14
    SPV_OPERAND_TYPE_FP_FAST_MATH_MODE,      // SPIR-V Sec 3.15
    SPV_OPERAND_TYPE_SELECTION_CONTROL,      // SPIR-V Sec 3.22
    SPV_OPERAND_TYPE_LOOP_CONTROL,           // SPIR-V Sec 3.23
    SPV_OPERAND_TYPE_FUNCTION_CONTROL,       // SPIR-V Sec 3.24
    SPV_OPERAND_TYPE_MEMORY_ACCESS,          // SPIR-V Sec 3.26
    SPV_OPERAND_TYPE_FRAGMENT_SHADING_RATE,  // SPIR-V Sec 3.FSR

    // NOTE: New concrete enum values should be added at the end.

    // The "optional" and "variable"  operand types are only used internally by
    // the assembler and the binary parser.
    // There are two categories:
    //    Optional : expands to 0 or 1 operand, like ? in regular expressions.
    //    Variable : expands to 0, 1 or many operands or pairs of operands.
    //               This is similar to * in regular expressions.

    // NOTE: These FIRST_* and LAST_* enum values are DEPRECATED.
    // The concept of "optional" and "variable" operand types are only intended
    // for use as an implementation detail of parsing SPIR-V, either in text or
    // binary form.  Instead of using enum ranges, use characteristic function
    // spvOperandIsConcrete.
    // The use of enum value ranges in a public API makes it difficult to insert
    // new values into a range without also breaking binary compatibility.
    //
    // Macros for defining bounds on optional and variable operand types.
    // Any variable operand type is also optional.
    // TODO(dneto): Remove SPV_OPERAND_TYPE_FIRST_* and SPV_OPERAND_TYPE_LAST_*
#define FIRST_OPTIONAL(ENUM) ENUM, SPV_OPERAND_TYPE_FIRST_OPTIONAL_TYPE = ENUM
#define FIRST_VARIABLE(ENUM) ENUM, SPV_OPERAND_TYPE_FIRST_VARIABLE_TYPE = ENUM
#define LAST_VARIABLE(ENUM)                         \
  ENUM, SPV_OPERAND_TYPE_LAST_VARIABLE_TYPE = ENUM, \
        SPV_OPERAND_TYPE_LAST_OPTIONAL_TYPE = ENUM

  // An optional operand represents zero or one logical operands.
  // In an instruction definition, this may only appear at the end of the
  // operand types.
    FIRST_OPTIONAL(SPV_OPERAND_TYPE_OPTIONAL_ID),
    // An optional image operand type.
    SPV_OPERAND_TYPE_OPTIONAL_IMAGE,
    // An optional memory access type.
    SPV_OPERAND_TYPE_OPTIONAL_MEMORY_ACCESS,
    // An optional literal integer.
    SPV_OPERAND_TYPE_OPTIONAL_LITERAL_INTEGER,
    // An optional literal number, which may be either integer or floating point.
    SPV_OPERAND_TYPE_OPTIONAL_LITERAL_NUMBER,
    // Like SPV_OPERAND_TYPE_TYPED_LITERAL_NUMBER, but optional, and integral.
    SPV_OPERAND_TYPE_OPTIONAL_TYPED_LITERAL_INTEGER,
    // An optional literal string.
    SPV_OPERAND_TYPE_OPTIONAL_LITERAL_STRING,
    // An optional access qualifier
    SPV_OPERAND_TYPE_OPTIONAL_ACCESS_QUALIFIER,
    // An optional context-independent value, or CIV.  CIVs are tokens that we can
    // assemble regardless of where they occur -- literals, IDs, immediate
    // integers, etc.
    SPV_OPERAND_TYPE_OPTIONAL_CIV,

    // A variable operand represents zero or more logical operands.
    // In an instruction definition, this may only appear at the end of the
    // operand types.
    FIRST_VARIABLE(SPV_OPERAND_TYPE_VARIABLE_ID),
    SPV_OPERAND_TYPE_VARIABLE_LITERAL_INTEGER,
    // A sequence of zero or more pairs of (typed literal integer, Id).
    // Expands to zero or more:
    //  (SPV_OPERAND_TYPE_TYPED_LITERAL_INTEGER, SPV_OPERAND_TYPE_ID)
    // where the literal number must always be an integer of some sort.
    SPV_OPERAND_TYPE_VARIABLE_LITERAL_INTEGER_ID,
    // A sequence of zero or more pairs of (Id, Literal integer)
    LAST_VARIABLE(SPV_OPERAND_TYPE_VARIABLE_ID_LITERAL_INTEGER),

    // The following are concrete enum types from the DebugInfo extended
    // instruction set.
    SPV_OPERAND_TYPE_DEBUG_INFO_FLAGS,  // DebugInfo Sec 3.2.  A mask.
    SPV_OPERAND_TYPE_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING,  // DebugInfo Sec 3.3
    SPV_OPERAND_TYPE_DEBUG_COMPOSITE_TYPE,                // DebugInfo Sec 3.4
    SPV_OPERAND_TYPE_DEBUG_TYPE_QUALIFIER,                // DebugInfo Sec 3.5
    SPV_OPERAND_TYPE_DEBUG_OPERATION,                     // DebugInfo Sec 3.6

    // The following are concrete enum types from the OpenCL.DebugInfo.100
    // extended instruction set.
    SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_INFO_FLAGS,  // Sec 3.2. A Mask
    SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING,  // Sec 3.3
    SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_COMPOSITE_TYPE,                // Sec 3.4
    SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_TYPE_QUALIFIER,                // Sec 3.5
    SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_OPERATION,                     // Sec 3.6
    SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_IMPORTED_ENTITY,               // Sec 3.7

    // The following are concrete enum types from SPV_INTEL_float_controls2
    // https://github.com/intel/llvm/blob/39fa9b0cbfbae88327118990a05c5b387b56d2ef/sycl/doc/extensions/SPIRV/SPV_INTEL_float_controls2.asciidoc
    SPV_OPERAND_TYPE_FPDENORM_MODE,     // Sec 3.17 FP Denorm Mode
    SPV_OPERAND_TYPE_FPOPERATION_MODE,  // Sec 3.18 FP Operation Mode
    // A value enum from https://github.com/KhronosGroup/SPIRV-Headers/pull/177
    SPV_OPERAND_TYPE_QUANTIZATION_MODES,
    // A value enum from https://github.com/KhronosGroup/SPIRV-Headers/pull/177
    SPV_OPERAND_TYPE_OVERFLOW_MODES,

    // Concrete operand types for the provisional Vulkan ray tracing feature.
    SPV_OPERAND_TYPE_RAY_FLAGS,               // SPIR-V Sec 3.RF
    SPV_OPERAND_TYPE_RAY_QUERY_INTERSECTION,  // SPIR-V Sec 3.RQIntersection
    SPV_OPERAND_TYPE_RAY_QUERY_COMMITTED_INTERSECTION_TYPE,  // SPIR-V Sec
    // 3.RQCommitted
    SPV_OPERAND_TYPE_RAY_QUERY_CANDIDATE_INTERSECTION_TYPE,  // SPIR-V Sec
    // 3.RQCandidate

// Concrete operand types for integer dot product.
// Packed vector format
SPV_OPERAND_TYPE_PACKED_VECTOR_FORMAT,  // SPIR-V Sec 3.x
// An optional packed vector format
SPV_OPERAND_TYPE_OPTIONAL_PACKED_VECTOR_FORMAT,

// This is a sentinel value, and does not represent an operand type.
// It should come last.
SPV_OPERAND_TYPE_NUM_OPERAND_TYPES,

SPV_FORCE_32_BIT_ENUM(spv_operand_type_t)
} spv_operand_type_t;

// Returns true if the given type is concrete.
bool spvOperandIsConcrete(spv_operand_type_t type);

// Returns true if the given type is concrete and also a mask.
bool spvOperandIsConcreteMask(spv_operand_type_t type);

typedef enum spv_ext_inst_type_t {
    SPV_EXT_INST_TYPE_NONE = 0,
    SPV_EXT_INST_TYPE_GLSL_STD_450,
    SPV_EXT_INST_TYPE_OPENCL_STD,
    SPV_EXT_INST_TYPE_SPV_AMD_SHADER_EXPLICIT_VERTEX_PARAMETER,
    SPV_EXT_INST_TYPE_SPV_AMD_SHADER_TRINARY_MINMAX,
    SPV_EXT_INST_TYPE_SPV_AMD_GCN_SHADER,
    SPV_EXT_INST_TYPE_SPV_AMD_SHADER_BALLOT,
    SPV_EXT_INST_TYPE_DEBUGINFO,
    SPV_EXT_INST_TYPE_OPENCL_DEBUGINFO_100,
    SPV_EXT_INST_TYPE_NONSEMANTIC_CLSPVREFLECTION,
    SPV_EXT_INST_TYPE_NONSEMANTIC_SHADER_DEBUGINFO_100,

    // Multiple distinct extended instruction set types could return this
    // value, if they are prefixed with NonSemantic. and are otherwise
    // unrecognised
    SPV_EXT_INST_TYPE_NONSEMANTIC_UNKNOWN,

    SPV_FORCE_32_BIT_ENUM(spv_ext_inst_type_t)
} spv_ext_inst_type_t;

// This determines at a high level the kind of a binary-encoded literal
// number, but not the bit width.
// In principle, these could probably be folded into new entries in
// spv_operand_type_t.  But then we'd have some special case differences
// between the assembler and disassembler.
typedef enum spv_number_kind_t {
    SPV_NUMBER_NONE = 0,  // The default for value initialization.
    SPV_NUMBER_UNSIGNED_INT,
    SPV_NUMBER_SIGNED_INT,
    SPV_NUMBER_FLOATING,
} spv_number_kind_t;

typedef enum spv_text_to_binary_options_t {
    SPV_TEXT_TO_BINARY_OPTION_NONE = SPV_BIT(0),
    // Numeric IDs in the binary will have the same values as in the source.
    // Non-numeric IDs are allocated by filling in the gaps, starting with 1
    // and going up.
    SPV_TEXT_TO_BINARY_OPTION_PRESERVE_NUMERIC_IDS = SPV_BIT(1),
    SPV_FORCE_32_BIT_ENUM(spv_text_to_binary_options_t)
} spv_text_to_binary_options_t;

typedef enum spv_binary_to_text_options_t {
    SPV_BINARY_TO_TEXT_OPTION_NONE = SPV_BIT(0),
    SPV_BINARY_TO_TEXT_OPTION_PRINT = SPV_BIT(1),
    SPV_BINARY_TO_TEXT_OPTION_COLOR = SPV_BIT(2),
    SPV_BINARY_TO_TEXT_OPTION_INDENT = SPV_BIT(3),
    SPV_BINARY_TO_TEXT_OPTION_SHOW_BYTE_OFFSET = SPV_BIT(4),
    // Do not output the module header as leading comments in the assembly.
    SPV_BINARY_TO_TEXT_OPTION_NO_HEADER = SPV_BIT(5),
    // Use friendly names where possible.  The heuristic may expand over
    // time, but will use common names for scalar types, and debug names from
    // OpName instructions.
    SPV_BINARY_TO_TEXT_OPTION_FRIENDLY_NAMES = SPV_BIT(6),
    // Add some comments to the generated assembly
    SPV_BINARY_TO_TEXT_OPTION_COMMENT = SPV_BIT(7),
    SPV_FORCE_32_BIT_ENUM(spv_binary_to_text_options_t)
} spv_binary_to_text_options_t;

// Constants

// The default id bound is to the minimum value for the id limit
// in the spir-v specification under the section "Universal Limits".
const uint32_t kDefaultMaxIdBound = 0x3FFFFF;

// Structures

// Information about an operand parsed from a binary SPIR-V module.
// Note that the values are not included.  You still need access to the binary
// to extract the values.
typedef struct spv_parsed_operand_t {
    // Location of the operand, in words from the start of the instruction.
    uint16_t offset;
    // Number of words occupied by this operand.
    uint16_t num_words;
    // The "concrete" operand type.  See the definition of spv_operand_type_t
    // for details.
    spv_operand_type_t type;
    // If type is a literal number type, then number_kind says whether it's
    // a signed integer, an unsigned integer, or a floating point number.
    spv_number_kind_t number_kind;
    // The number of bits for a literal number type.
    uint32_t number_bit_width;
} spv_parsed_operand_t;

// An instruction parsed from a binary SPIR-V module.
typedef struct spv_parsed_instruction_t {
    // An array of words for this instruction, in native endianness.
    const uint32_t* words;
    // The number of words in this instruction.
    uint16_t num_words;
    uint16_t opcode;
    // The extended instruction type, if opcode is OpExtInst.  Otherwise
    // this is the "none" value.
    spv_ext_inst_type_t ext_inst_type;
    // The type id, or 0 if this instruction doesn't have one.
    uint32_t type_id;
    // The result id, or 0 if this instruction doesn't have one.
    uint32_t result_id;
    // The array of parsed operands.
    const spv_parsed_operand_t* operands;
    uint16_t num_operands;
} spv_parsed_instruction_t;

typedef struct spv_const_binary_t {
    const uint32_t* code;
    const size_t wordCount;
} spv_const_binary_t;

typedef struct spv_binary_t {
    uint32_t* code;
    size_t wordCount;
} spv_binary_t;

typedef struct spv_text_t {
    const char* str;
    size_t length;
} spv_text_t;

typedef struct spv_position_t {
    size_t line;
    size_t column;
    size_t index;
} spv_position_t;

typedef struct spv_diagnostic_t {
    spv_position_t position;
    char* error;
    bool isTextSource;
} spv_diagnostic_t;

// Opaque struct containing the context used to operate on a SPIR-V module.
// Its object is used by various translation API functions.
typedef struct spv_context_t spv_context_t;

typedef struct spv_validator_options_t spv_validator_options_t;

typedef struct spv_optimizer_options_t spv_optimizer_options_t;

typedef struct spv_reducer_options_t spv_reducer_options_t;

typedef struct spv_fuzzer_options_t spv_fuzzer_options_t;

// Type Definitions

typedef spv_const_binary_t* spv_const_binary;
typedef spv_binary_t* spv_binary;
typedef spv_text_t* spv_text;
typedef spv_position_t* spv_position;
typedef spv_diagnostic_t* spv_diagnostic;
typedef const spv_context_t* spv_const_context;
typedef spv_context_t* spv_context;
typedef spv_validator_options_t* spv_validator_options;
typedef const spv_validator_options_t* spv_const_validator_options;
typedef spv_optimizer_options_t* spv_optimizer_options;
typedef const spv_optimizer_options_t* spv_const_optimizer_options;
typedef spv_reducer_options_t* spv_reducer_options;
typedef const spv_reducer_options_t* spv_const_reducer_options;
typedef spv_fuzzer_options_t* spv_fuzzer_options;
typedef const spv_fuzzer_options_t* spv_const_fuzzer_options;

// Platform API

// Returns the SPIRV-Tools software version as a null-terminated string.
// The contents of the underlying storage is valid for the remainder of
// the process.
SPIRV_TOOLS_EXPORT const char* spvSoftwareVersionString(void);
// Returns a null-terminated string containing the name of the project,
// the software version string, and commit details.
// The contents of the underlying storage is valid for the remainder of
// the process.
SPIRV_TOOLS_EXPORT const char* spvSoftwareVersionDetailsString(void);

// Certain target environments impose additional restrictions on SPIR-V, so it's
// often necessary to specify which one applies.  SPV_ENV_UNIVERSAL_* implies an
// environment-agnostic SPIR-V.
//
// When an API method needs to derive a SPIR-V version from a target environment
// (from the spv_context object), the method will choose the highest version of
// SPIR-V supported by the target environment.  Examples:
//    SPV_ENV_VULKAN_1_0           ->  SPIR-V 1.0
//    SPV_ENV_VULKAN_1_1           ->  SPIR-V 1.3
//    SPV_ENV_VULKAN_1_1_SPIRV_1_4 ->  SPIR-V 1.4
//    SPV_ENV_VULKAN_1_2           ->  SPIR-V 1.5
//    SPV_ENV_VULKAN_1_3           ->  SPIR-V 1.6
// Consult the description of API entry points for specific rules.
typedef enum {
    SPV_ENV_UNIVERSAL_1_0,  // SPIR-V 1.0 latest revision, no other restrictions.
    SPV_ENV_VULKAN_1_0,     // Vulkan 1.0 latest revision.
    SPV_ENV_UNIVERSAL_1_1,  // SPIR-V 1.1 latest revision, no other restrictions.
    SPV_ENV_OPENCL_2_1,     // OpenCL Full Profile 2.1 latest revision.
    SPV_ENV_OPENCL_2_2,     // OpenCL Full Profile 2.2 latest revision.
    SPV_ENV_OPENGL_4_0,     // OpenGL 4.0 plus GL_ARB_gl_spirv, latest revisions.
    SPV_ENV_OPENGL_4_1,     // OpenGL 4.1 plus GL_ARB_gl_spirv, latest revisions.
    SPV_ENV_OPENGL_4_2,     // OpenGL 4.2 plus GL_ARB_gl_spirv, latest revisions.
    SPV_ENV_OPENGL_4_3,     // OpenGL 4.3 plus GL_ARB_gl_spirv, latest revisions.
    // There is no variant for OpenGL 4.4.
    SPV_ENV_OPENGL_4_5,     // OpenGL 4.5 plus GL_ARB_gl_spirv, latest revisions.
    SPV_ENV_UNIVERSAL_1_2,  // SPIR-V 1.2, latest revision, no other restrictions.
    SPV_ENV_OPENCL_1_2,     // OpenCL Full Profile 1.2 plus cl_khr_il_program,
    // latest revision.
    SPV_ENV_OPENCL_EMBEDDED_1_2,  // OpenCL Embedded Profile 1.2 plus
    // cl_khr_il_program, latest revision.
    SPV_ENV_OPENCL_2_0,  // OpenCL Full Profile 2.0 plus cl_khr_il_program,
    // latest revision.
    SPV_ENV_OPENCL_EMBEDDED_2_0,  // OpenCL Embedded Profile 2.0 plus
    // cl_khr_il_program, latest revision.
    SPV_ENV_OPENCL_EMBEDDED_2_1,  // OpenCL Embedded Profile 2.1 latest revision.
    SPV_ENV_OPENCL_EMBEDDED_2_2,  // OpenCL Embedded Profile 2.2 latest revision.
    SPV_ENV_UNIVERSAL_1_3,  // SPIR-V 1.3 latest revision, no other restrictions.
    SPV_ENV_VULKAN_1_1,     // Vulkan 1.1 latest revision.
    SPV_ENV_WEBGPU_0,       // DEPRECATED, may be removed in the future.
    SPV_ENV_UNIVERSAL_1_4,  // SPIR-V 1.4 latest revision, no other restrictions.

    // Vulkan 1.1 with VK_KHR_spirv_1_4, i.e. SPIR-V 1.4 binary.
    SPV_ENV_VULKAN_1_1_SPIRV_1_4,

    SPV_ENV_UNIVERSAL_1_5,  // SPIR-V 1.5 latest revision, no other restrictions.
    SPV_ENV_VULKAN_1_2,     // Vulkan 1.2 latest revision.

    SPV_ENV_UNIVERSAL_1_6,  // SPIR-V 1.6 latest revision, no other restrictions.
    SPV_ENV_VULKAN_1_3,     // Vulkan 1.3 latest revision.

    SPV_ENV_MAX  // Keep this as the last enum value.
} spv_target_env;

// SPIR-V Validator can be parameterized with the following Universal Limits.
typedef enum {
    spv_validator_limit_max_struct_members,
    spv_validator_limit_max_struct_depth,
    spv_validator_limit_max_local_variables,
    spv_validator_limit_max_global_variables,
    spv_validator_limit_max_switch_branches,
    spv_validator_limit_max_function_args,
    spv_validator_limit_max_control_flow_nesting_depth,
    spv_validator_limit_max_access_chain_indexes,
    spv_validator_limit_max_id_bound,
} spv_validator_limit;

// Returns a string describing the given SPIR-V target environment.
SPIRV_TOOLS_EXPORT const char* spvTargetEnvDescription(spv_target_env env);

// Parses s into *env and returns true if successful.  If unparsable, returns
// false and sets *env to SPV_ENV_UNIVERSAL_1_0.
SPIRV_TOOLS_EXPORT bool spvParseTargetEnv(const char* s, spv_target_env* env);

// Determines the target env value with the least features but which enables
// the given Vulkan and SPIR-V versions. If such a target is supported, returns
// true and writes the value to |env|, otherwise returns false.
//
// The Vulkan version is given as an unsigned 32-bit number as specified in
// Vulkan section "29.2.1 Version Numbers": the major version number appears
// in bits 22 to 21, and the minor version is in bits 12 to 21.  The SPIR-V
// version is given in the SPIR-V version header word: major version in bits
// 16 to 23, and minor version in bits 8 to 15.
SPIRV_TOOLS_EXPORT bool spvParseVulkanEnv(uint32_t vulkan_ver,
    uint32_t spirv_ver,
    spv_target_env* env);

// Creates a context object for most of the SPIRV-Tools API.
// Returns null if env is invalid.
//
// See specific API calls for how the target environment is interpreted
// (particularly assembly and validation).
SPIRV_TOOLS_EXPORT spv_context spvContextCreate(spv_target_env env);

// Destroys the given context object.
SPIRV_TOOLS_EXPORT void spvContextDestroy(spv_context context);

// Creates a Validator options object with default options. Returns a valid
// options object. The object remains valid until it is passed into
// spvValidatorOptionsDestroy.
SPIRV_TOOLS_EXPORT spv_validator_options spvValidatorOptionsCreate(void);

// Destroys the given Validator options object.
SPIRV_TOOLS_EXPORT void spvValidatorOptionsDestroy(
    spv_validator_options options);

// Records the maximum Universal Limit that is considered valid in the given
// Validator options object. <options> argument must be a valid options object.
SPIRV_TOOLS_EXPORT void spvValidatorOptionsSetUniversalLimit(
    spv_validator_options options, spv_validator_limit limit_type,
    uint32_t limit);

// Record whether or not the validator should relax the rules on types for
// stores to structs.  When relaxed, it will allow a type mismatch as long as
// the types are structs with the same layout.  Two structs have the same layout
// if
//
// 1) the members of the structs are either the same type or are structs with
// same layout, and
//
// 2) the decorations that affect the memory layout are identical for both
// types.  Other decorations are not relevant.
SPIRV_TOOLS_EXPORT void spvValidatorOptionsSetRelaxStoreStruct(
    spv_validator_options options, bool val);

// Records whether or not the validator should relax the rules on pointer usage
// in logical addressing mode.
//
// When relaxed, it will allow the following usage cases of pointers:
// 1) OpVariable allocating an object whose type is a pointer type
// 2) OpReturnValue returning a pointer value
SPIRV_TOOLS_EXPORT void spvValidatorOptionsSetRelaxLogicalPointer(
    spv_validator_options options, bool val);

// Records whether or not the validator should relax the rules because it is
// expected that the optimizations will make the code legal.
//
// When relaxed, it will allow the following:
// 1) It will allow relaxed logical pointers.  Setting this option will also
//    set that option.
// 2) Pointers that are pass as parameters to function calls do not have to
//    match the storage class of the formal parameter.
// 3) Pointers that are actual parameters on function calls do not have to point
//    to the same type pointed as the formal parameter.  The types just need to
//    logically match.
// 4) GLSLstd450 Interpolate* instructions can have a load of an interpolant
//    for a first argument.
SPIRV_TOOLS_EXPORT void spvValidatorOptionsSetBeforeHlslLegalization(
    spv_validator_options options, bool val);

// Records whether the validator should use "relaxed" block layout rules.
// Relaxed layout rules are described by Vulkan extension
// VK_KHR_relaxed_block_layout, and they affect uniform blocks, storage blocks,
// and push constants.
//
// This is enabled by default when targeting Vulkan 1.1 or later.
// Relaxed layout is more permissive than the default rules in Vulkan 1.0.
SPIRV_TOOLS_EXPORT void spvValidatorOptionsSetRelaxBlockLayout(
    spv_validator_options options, bool val);

// Records whether the validator should use standard block layout rules for
// uniform blocks.
SPIRV_TOOLS_EXPORT void spvValidatorOptionsSetUniformBufferStandardLayout(
    spv_validator_options options, bool val);

// Records whether the validator should use "scalar" block layout rules.
// Scalar layout rules are more permissive than relaxed block layout.
//
// See Vulkan extension VK_EXT_scalar_block_layout.  The scalar alignment is
// defined as follows:
// - scalar alignment of a scalar is the scalar size
// - scalar alignment of a vector is the scalar alignment of its component
// - scalar alignment of a matrix is the scalar alignment of its component
// - scalar alignment of an array is the scalar alignment of its element
// - scalar alignment of a struct is the max scalar alignment among its
//   members
//
// For a struct in Uniform, StorageClass, or PushConstant:
// - a member Offset must be a multiple of the member's scalar alignment
// - ArrayStride or MatrixStride must be a multiple of the array or matrix
//   scalar alignment
SPIRV_TOOLS_EXPORT void spvValidatorOptionsSetScalarBlockLayout(
    spv_validator_options options, bool val);

// Records whether the validator should use "scalar" block layout
// rules (as defined above) for Workgroup blocks.  See Vulkan
// extension VK_KHR_workgroup_memory_explicit_layout.
SPIRV_TOOLS_EXPORT void spvValidatorOptionsSetWorkgroupScalarBlockLayout(
    spv_validator_options options, bool val);

// Records whether or not the validator should skip validating standard
// uniform/storage block layout.
SPIRV_TOOLS_EXPORT void spvValidatorOptionsSetSkipBlockLayout(
    spv_validator_options options, bool val);

// Records whether or not the validator should allow the LocalSizeId
// decoration where the environment otherwise would not allow it.
SPIRV_TOOLS_EXPORT void spvValidatorOptionsSetAllowLocalSizeId(
    spv_validator_options options, bool val);

// Whether friendly names should be used in validation error messages.
SPIRV_TOOLS_EXPORT void spvValidatorOptionsSetFriendlyNames(
    spv_validator_options options, bool val);

// Creates an optimizer options object with default options. Returns a valid
// options object. The object remains valid until it is passed into
// |spvOptimizerOptionsDestroy|.
SPIRV_TOOLS_EXPORT spv_optimizer_options spvOptimizerOptionsCreate(void);

// Destroys the given optimizer options object.
SPIRV_TOOLS_EXPORT void spvOptimizerOptionsDestroy(
    spv_optimizer_options options);

// Records whether or not the optimizer should run the validator before
// optimizing.  If |val| is true, the validator will be run.
SPIRV_TOOLS_EXPORT void spvOptimizerOptionsSetRunValidator(
    spv_optimizer_options options, bool val);

// Records the validator options that should be passed to the validator if it is
// run.
SPIRV_TOOLS_EXPORT void spvOptimizerOptionsSetValidatorOptions(
    spv_optimizer_options options, spv_validator_options val);

// Records the maximum possible value for the id bound.
SPIRV_TOOLS_EXPORT void spvOptimizerOptionsSetMaxIdBound(
    spv_optimizer_options options, uint32_t val);

// Records whether all bindings within the module should be preserved.
SPIRV_TOOLS_EXPORT void spvOptimizerOptionsSetPreserveBindings(
    spv_optimizer_options options, bool val);

// Records whether all specialization constants within the module
// should be preserved.
SPIRV_TOOLS_EXPORT void spvOptimizerOptionsSetPreserveSpecConstants(
    spv_optimizer_options options, bool val);

// Creates a reducer options object with default options. Returns a valid
// options object. The object remains valid until it is passed into
// |spvReducerOptionsDestroy|.
SPIRV_TOOLS_EXPORT spv_reducer_options spvReducerOptionsCreate(void);

// Destroys the given reducer options object.
SPIRV_TOOLS_EXPORT void spvReducerOptionsDestroy(spv_reducer_options options);

// Sets the maximum number of reduction steps that should run before the reducer
// gives up.
SPIRV_TOOLS_EXPORT void spvReducerOptionsSetStepLimit(
    spv_reducer_options options, uint32_t step_limit);

// Sets the fail-on-validation-error option; if true, the reducer will return
// kStateInvalid if a reduction step yields a state that fails SPIR-V
// validation. Otherwise, an invalid state is treated as uninteresting and the
// reduction backtracks and continues.
SPIRV_TOOLS_EXPORT void spvReducerOptionsSetFailOnValidationError(
    spv_reducer_options options, bool fail_on_validation_error);

// Sets the function that the reducer should target.  If set to zero the reducer
// will target all functions as well as parts of the module that lie outside
// functions.  Otherwise the reducer will restrict reduction to the function
// with result id |target_function|, which is required to exist.
SPIRV_TOOLS_EXPORT void spvReducerOptionsSetTargetFunction(
    spv_reducer_options options, uint32_t target_function);

// Creates a fuzzer options object with default options. Returns a valid
// options object. The object remains valid until it is passed into
// |spvFuzzerOptionsDestroy|.
SPIRV_TOOLS_EXPORT spv_fuzzer_options spvFuzzerOptionsCreate(void);

// Destroys the given fuzzer options object.
SPIRV_TOOLS_EXPORT void spvFuzzerOptionsDestroy(spv_fuzzer_options options);

// Enables running the validator after every transformation is applied during
// a replay.
SPIRV_TOOLS_EXPORT void spvFuzzerOptionsEnableReplayValidation(
    spv_fuzzer_options options);

// Sets the seed with which the random number generator used by the fuzzer
// should be initialized.
SPIRV_TOOLS_EXPORT void spvFuzzerOptionsSetRandomSeed(
    spv_fuzzer_options options, uint32_t seed);

// Sets the range of transformations that should be applied during replay: 0
// means all transformations, +N means the first N transformations, -N means all
// except the final N transformations.
SPIRV_TOOLS_EXPORT void spvFuzzerOptionsSetReplayRange(
    spv_fuzzer_options options, int32_t replay_range);

// Sets the maximum number of steps that the shrinker should take before giving
// up.
SPIRV_TOOLS_EXPORT void spvFuzzerOptionsSetShrinkerStepLimit(
    spv_fuzzer_options options, uint32_t shrinker_step_limit);

// Enables running the validator after every pass is applied during a fuzzing
// run.
SPIRV_TOOLS_EXPORT void spvFuzzerOptionsEnableFuzzerPassValidation(
    spv_fuzzer_options options);

// Enables all fuzzer passes during a fuzzing run (instead of a random subset
// of passes).
SPIRV_TOOLS_EXPORT void spvFuzzerOptionsEnableAllPasses(
    spv_fuzzer_options options);

// Encodes the given SPIR-V assembly text to its binary representation. The
// length parameter specifies the number of bytes for text. Encoded binary will
// be stored into *binary. Any error will be written into *diagnostic if
// diagnostic is non-null, otherwise the context's message consumer will be
// used. The generated binary is independent of the context and may outlive it.
// The SPIR-V binary version is set to the highest version of SPIR-V supported
// by the context's target environment.
SPIRV_TOOLS_EXPORT spv_result_t spvTextToBinary(const spv_const_context context,
    const char* text,
    const size_t length,
    spv_binary* binary,
    spv_diagnostic* diagnostic);

// Encodes the given SPIR-V assembly text to its binary representation. Same as
// spvTextToBinary but with options. The options parameter is a bit field of
// spv_text_to_binary_options_t.
SPIRV_TOOLS_EXPORT spv_result_t spvTextToBinaryWithOptions(
    const spv_const_context context, const char* text, const size_t length,
    const uint32_t options, spv_binary* binary, spv_diagnostic* diagnostic);

// Frees an allocated text stream. This is a no-op if the text parameter
// is a null pointer.
SPIRV_TOOLS_EXPORT void spvTextDestroy(spv_text text);

// Decodes the given SPIR-V binary representation to its assembly text. The
// word_count parameter specifies the number of words for binary. The options
// parameter is a bit field of spv_binary_to_text_options_t. Decoded text will
// be stored into *text. Any error will be written into *diagnostic if
// diagnostic is non-null, otherwise the context's message consumer will be
// used.
SPIRV_TOOLS_EXPORT spv_result_t spvBinaryToText(const spv_const_context context,
    const uint32_t* binary,
    const size_t word_count,
    const uint32_t options,
    spv_text* text,
    spv_diagnostic* diagnostic);

// Frees a binary stream from memory. This is a no-op if binary is a null
// pointer.
SPIRV_TOOLS_EXPORT void spvBinaryDestroy(spv_binary binary);

// Validates a SPIR-V binary for correctness. Any errors will be written into
// *diagnostic if diagnostic is non-null, otherwise the context's message
// consumer will be used.
//
// Validate for SPIR-V spec rules for the SPIR-V version named in the
// binary's header (at word offset 1).  Additionally, if the context target
// environment is a client API (such as Vulkan 1.1), then validate for that
// client API version, to the extent that it is verifiable from data in the
// binary itself.
SPIRV_TOOLS_EXPORT spv_result_t spvValidate(const spv_const_context context,
    const spv_const_binary binary,
    spv_diagnostic* diagnostic);

// Validates a SPIR-V binary for correctness. Uses the provided Validator
// options. Any errors will be written into *diagnostic if diagnostic is
// non-null, otherwise the context's message consumer will be used.
//
// Validate for SPIR-V spec rules for the SPIR-V version named in the
// binary's header (at word offset 1).  Additionally, if the context target
// environment is a client API (such as Vulkan 1.1), then validate for that
// client API version, to the extent that it is verifiable from data in the
// binary itself, or in the validator options.
SPIRV_TOOLS_EXPORT spv_result_t spvValidateWithOptions(
    const spv_const_context context, const spv_const_validator_options options,
    const spv_const_binary binary, spv_diagnostic* diagnostic);

// Validates a raw SPIR-V binary for correctness. Any errors will be written
// into *diagnostic if diagnostic is non-null, otherwise the context's message
// consumer will be used.
SPIRV_TOOLS_EXPORT spv_result_t
spvValidateBinary(const spv_const_context context, const uint32_t* words,
    const size_t num_words, spv_diagnostic* diagnostic);

// Creates a diagnostic object. The position parameter specifies the location in
// the text/binary stream. The message parameter, copied into the diagnostic
// object, contains the error message to display.
SPIRV_TOOLS_EXPORT spv_diagnostic
spvDiagnosticCreate(const spv_position position, const char* message);

// Destroys a diagnostic object.  This is a no-op if diagnostic is a null
// pointer.
SPIRV_TOOLS_EXPORT void spvDiagnosticDestroy(spv_diagnostic diagnostic);

// Prints the diagnostic to stderr.
SPIRV_TOOLS_EXPORT spv_result_t
spvDiagnosticPrint(const spv_diagnostic diagnostic);

// Gets the name of an instruction, without the "Op" prefix.
SPIRV_TOOLS_EXPORT const char* spvOpcodeString(const uint32_t opcode);

// The binary parser interface.

// A pointer to a function that accepts a parsed SPIR-V header.
// The integer arguments are the 32-bit words from the header, as specified
// in SPIR-V 1.0 Section 2.3 Table 1.
// The function should return SPV_SUCCESS if parsing should continue.
typedef spv_result_t(*spv_parsed_header_fn_t)(
    void* user_data, spv_endianness_t endian, uint32_t magic, uint32_t version,
    uint32_t generator, uint32_t id_bound, uint32_t reserved);

// A pointer to a function that accepts a parsed SPIR-V instruction.
// The parsed_instruction value is transient: it may be overwritten
// or released immediately after the function has returned.  That also
// applies to the words array member of the parsed instruction.  The
// function should return SPV_SUCCESS if and only if parsing should
// continue.
typedef spv_result_t(*spv_parsed_instruction_fn_t)(
    void* user_data, const spv_parsed_instruction_t* parsed_instruction);

// Parses a SPIR-V binary, specified as counted sequence of 32-bit words.
// Parsing feedback is provided via two callbacks provided as function
// pointers.  Each callback function pointer can be a null pointer, in
// which case it is never called.  Otherwise, in a valid parse the
// parsed-header callback is called once, and then the parsed-instruction
// callback once for each instruction in the stream.  The user_data parameter
// is supplied as context to the callbacks.  Returns SPV_SUCCESS on successful
// parse where the callbacks always return SPV_SUCCESS.  For an invalid parse,
// returns a status code other than SPV_SUCCESS, and if diagnostic is non-null
// also emits a diagnostic. If diagnostic is null the context's message consumer
// will be used to emit any errors. If a callback returns anything other than
// SPV_SUCCESS, then that status code is returned, no further callbacks are
// issued, and no additional diagnostics are emitted.
SPIRV_TOOLS_EXPORT spv_result_t spvBinaryParse(
    const spv_const_context context, void* user_data, const uint32_t* words,
    const size_t num_words, spv_parsed_header_fn_t parse_header,
    spv_parsed_instruction_fn_t parse_instruction, spv_diagnostic* diagnostic);

#ifdef __cplusplus
}
#endif

#endif  // INCLUDE_SPIRV_TOOLS_LIBSPIRV_H_


namespace spvtools {

    // Message consumer. The C strings for source and message are only alive for the
    // specific invocation.
    using MessageConsumer = std::function<void(
        spv_message_level_t /* level */, const char* /* source */,
        const spv_position_t& /* position */, const char* /* message */
        )>;

    // C++ RAII wrapper around the C context object spv_context.
    class Context {
    public:
        // Constructs a context targeting the given environment |env|.
        //
        // See specific API calls for how the target environment is interpreted
        // (particularly assembly and validation).
        //
        // The constructed instance will have an empty message consumer, which just
        // ignores all messages from the library. Use SetMessageConsumer() to supply
        // one if messages are of concern.
        explicit Context(spv_target_env env);

        // Enables move constructor/assignment operations.
        Context(Context&& other);
        Context& operator=(Context&& other);

        // Disables copy constructor/assignment operations.
        Context(const Context&) = delete;
        Context& operator=(const Context&) = delete;

        // Destructs this instance.
        ~Context();

        // Sets the message consumer to the given |consumer|. The |consumer| will be
        // invoked once for each message communicated from the library.
        void SetMessageConsumer(MessageConsumer consumer);

        // Returns the underlying spv_context.
        spv_context& CContext();
        const spv_context& CContext() const;

    private:
        spv_context context_;
    };

    // A RAII wrapper around a validator options object.
    class ValidatorOptions {
    public:
        ValidatorOptions() : options_(spvValidatorOptionsCreate()) {}
        ~ValidatorOptions() { spvValidatorOptionsDestroy(options_); }
        // Allow implicit conversion to the underlying object.
        operator spv_validator_options() const { return options_; }

        // Sets a limit.
        void SetUniversalLimit(spv_validator_limit limit_type, uint32_t limit) {
            spvValidatorOptionsSetUniversalLimit(options_, limit_type, limit);
        }

        void SetRelaxStructStore(bool val) {
            spvValidatorOptionsSetRelaxStoreStruct(options_, val);
        }

        // Enables VK_KHR_relaxed_block_layout when validating standard
        // uniform/storage buffer/push-constant layout.  If true, disables
        // scalar block layout rules.
        void SetRelaxBlockLayout(bool val) {
            spvValidatorOptionsSetRelaxBlockLayout(options_, val);
        }

        // Enables VK_KHR_uniform_buffer_standard_layout when validating standard
        // uniform layout.  If true, disables scalar block layout rules.
        void SetUniformBufferStandardLayout(bool val) {
            spvValidatorOptionsSetUniformBufferStandardLayout(options_, val);
        }

        // Enables VK_EXT_scalar_block_layout when validating standard
        // uniform/storage buffer/push-constant layout.  If true, disables
        // relaxed block layout rules.
        void SetScalarBlockLayout(bool val) {
            spvValidatorOptionsSetScalarBlockLayout(options_, val);
        }

        // Enables scalar layout when validating Workgroup blocks.  See
        // VK_KHR_workgroup_memory_explicit_layout.
        void SetWorkgroupScalarBlockLayout(bool val) {
            spvValidatorOptionsSetWorkgroupScalarBlockLayout(options_, val);
        }

        // Skips validating standard uniform/storage buffer/push-constant layout.
        void SetSkipBlockLayout(bool val) {
            spvValidatorOptionsSetSkipBlockLayout(options_, val);
        }

        // Enables LocalSizeId decorations where the environment would not otherwise
        // allow them.
        void SetAllowLocalSizeId(bool val) {
            spvValidatorOptionsSetAllowLocalSizeId(options_, val);
        }

        // Records whether or not the validator should relax the rules on pointer
        // usage in logical addressing mode.
        //
        // When relaxed, it will allow the following usage cases of pointers:
        // 1) OpVariable allocating an object whose type is a pointer type
        // 2) OpReturnValue returning a pointer value
        void SetRelaxLogicalPointer(bool val) {
            spvValidatorOptionsSetRelaxLogicalPointer(options_, val);
        }

        // Records whether or not the validator should relax the rules because it is
        // expected that the optimizations will make the code legal.
        //
        // When relaxed, it will allow the following:
        // 1) It will allow relaxed logical pointers.  Setting this option will also
        //    set that option.
        // 2) Pointers that are pass as parameters to function calls do not have to
        //    match the storage class of the formal parameter.
        // 3) Pointers that are actual parameters on function calls do not have to
        //    point to the same type pointed as the formal parameter.  The types just
        //    need to logically match.
        // 4) GLSLstd450 Interpolate* instructions can have a load of an interpolant
        //    for a first argument.
        void SetBeforeHlslLegalization(bool val) {
            spvValidatorOptionsSetBeforeHlslLegalization(options_, val);
        }

        // Whether friendly names should be used in validation error messages.
        void SetFriendlyNames(bool val) {
            spvValidatorOptionsSetFriendlyNames(options_, val);
        }

    private:
        spv_validator_options options_;
    };

    // A C++ wrapper around an optimization options object.
    class OptimizerOptions {
    public:
        OptimizerOptions() : options_(spvOptimizerOptionsCreate()) {}
        ~OptimizerOptions() { spvOptimizerOptionsDestroy(options_); }

        // Allow implicit conversion to the underlying object.
        operator spv_optimizer_options() const { return options_; }

        // Records whether or not the optimizer should run the validator before
        // optimizing.  If |run| is true, the validator will be run.
        void set_run_validator(bool run) {
            spvOptimizerOptionsSetRunValidator(options_, run);
        }

        // Records the validator options that should be passed to the validator if it
        // is run.
        void set_validator_options(const ValidatorOptions& val_options) {
            spvOptimizerOptionsSetValidatorOptions(options_, val_options);
        }

        // Records the maximum possible value for the id bound.
        void set_max_id_bound(uint32_t new_bound) {
            spvOptimizerOptionsSetMaxIdBound(options_, new_bound);
        }

        // Records whether all bindings within the module should be preserved.
        void set_preserve_bindings(bool preserve_bindings) {
            spvOptimizerOptionsSetPreserveBindings(options_, preserve_bindings);
        }

        // Records whether all specialization constants within the module
        // should be preserved.
        void set_preserve_spec_constants(bool preserve_spec_constants) {
            spvOptimizerOptionsSetPreserveSpecConstants(options_,
                preserve_spec_constants);
        }

    private:
        spv_optimizer_options options_;
    };

    // A C++ wrapper around a reducer options object.
    class ReducerOptions {
    public:
        ReducerOptions() : options_(spvReducerOptionsCreate()) {}
        ~ReducerOptions() { spvReducerOptionsDestroy(options_); }

        // Allow implicit conversion to the underlying object.
        operator spv_reducer_options() const {  // NOLINT(google-explicit-constructor)
            return options_;
        }

        // See spvReducerOptionsSetStepLimit.
        void set_step_limit(uint32_t step_limit) {
            spvReducerOptionsSetStepLimit(options_, step_limit);
        }

        // See spvReducerOptionsSetFailOnValidationError.
        void set_fail_on_validation_error(bool fail_on_validation_error) {
            spvReducerOptionsSetFailOnValidationError(options_,
                fail_on_validation_error);
        }

        // See spvReducerOptionsSetTargetFunction.
        void set_target_function(uint32_t target_function) {
            spvReducerOptionsSetTargetFunction(options_, target_function);
        }

    private:
        spv_reducer_options options_;
    };

    // A C++ wrapper around a fuzzer options object.
    class FuzzerOptions {
    public:
        FuzzerOptions() : options_(spvFuzzerOptionsCreate()) {}
        ~FuzzerOptions() { spvFuzzerOptionsDestroy(options_); }

        // Allow implicit conversion to the underlying object.
        operator spv_fuzzer_options() const {  // NOLINT(google-explicit-constructor)
            return options_;
        }

        // See spvFuzzerOptionsEnableReplayValidation.
        void enable_replay_validation() {
            spvFuzzerOptionsEnableReplayValidation(options_);
        }

        // See spvFuzzerOptionsSetRandomSeed.
        void set_random_seed(uint32_t seed) {
            spvFuzzerOptionsSetRandomSeed(options_, seed);
        }

        // See spvFuzzerOptionsSetReplayRange.
        void set_replay_range(int32_t replay_range) {
            spvFuzzerOptionsSetReplayRange(options_, replay_range);
        }

        // See spvFuzzerOptionsSetShrinkerStepLimit.
        void set_shrinker_step_limit(uint32_t shrinker_step_limit) {
            spvFuzzerOptionsSetShrinkerStepLimit(options_, shrinker_step_limit);
        }

        // See spvFuzzerOptionsEnableFuzzerPassValidation.
        void enable_fuzzer_pass_validation() {
            spvFuzzerOptionsEnableFuzzerPassValidation(options_);
        }

        // See spvFuzzerOptionsEnableAllPasses.
        void enable_all_passes() { spvFuzzerOptionsEnableAllPasses(options_); }

    private:
        spv_fuzzer_options options_;
    };

    // C++ interface for SPIRV-Tools functionalities. It wraps the context
    // (including target environment and the corresponding SPIR-V grammar) and
    // provides methods for assembling, disassembling, and validating.
    //
    // Instances of this class provide basic thread-safety guarantee.
    class SpirvTools {
    public:
        enum {
            // Default assembling option used by assemble():
            kDefaultAssembleOption = SPV_TEXT_TO_BINARY_OPTION_NONE,

            // Default disassembling option used by Disassemble():
            // * Avoid prefix comments from decoding the SPIR-V module header, and
            // * Use friendly names for variables.
            kDefaultDisassembleOption = SPV_BINARY_TO_TEXT_OPTION_NO_HEADER |
            SPV_BINARY_TO_TEXT_OPTION_FRIENDLY_NAMES
        };

        // Constructs an instance targeting the given environment |env|.
        //
        // The constructed instance will have an empty message consumer, which just
        // ignores all messages from the library. Use SetMessageConsumer() to supply
        // one if messages are of concern.
        explicit SpirvTools(spv_target_env env);

        // Disables copy/move constructor/assignment operations.
        SpirvTools(const SpirvTools&) = delete;
        SpirvTools(SpirvTools&&) = delete;
        SpirvTools& operator=(const SpirvTools&) = delete;
        SpirvTools& operator=(SpirvTools&&) = delete;

        // Destructs this instance.
        ~SpirvTools();

        // Sets the message consumer to the given |consumer|. The |consumer| will be
        // invoked once for each message communicated from the library.
        void SetMessageConsumer(MessageConsumer consumer);

        // Assembles the given assembly |text| and writes the result to |binary|.
        // Returns true on successful assembling. |binary| will be kept untouched if
        // assembling is unsuccessful.
        // The SPIR-V binary version is set to the highest version of SPIR-V supported
        // by the target environment with which this SpirvTools object was created.
        bool Assemble(const std::string& text, std::vector<uint32_t>* binary,
            uint32_t options = kDefaultAssembleOption) const;
        // |text_size| specifies the number of bytes in |text|. A terminating null
        // character is not required to present in |text| as long as |text| is valid.
        // The SPIR-V binary version is set to the highest version of SPIR-V supported
        // by the target environment with which this SpirvTools object was created.
        bool Assemble(const char* text, size_t text_size,
            std::vector<uint32_t>* binary,
            uint32_t options = kDefaultAssembleOption) const;

        // Disassembles the given SPIR-V |binary| with the given |options| and writes
        // the assembly to |text|. Returns true on successful disassembling. |text|
        // will be kept untouched if diassembling is unsuccessful.
        bool Disassemble(const std::vector<uint32_t>& binary, std::string* text,
            uint32_t options = kDefaultDisassembleOption) const;
        // |binary_size| specifies the number of words in |binary|.
        bool Disassemble(const uint32_t* binary, size_t binary_size,
            std::string* text,
            uint32_t options = kDefaultDisassembleOption) const;

        // Validates the given SPIR-V |binary|. Returns true if no issues are found.
        // Otherwise, returns false and communicates issues via the message consumer
        // registered.
        // Validates for SPIR-V spec rules for the SPIR-V version named in the
        // binary's header (at word offset 1).  Additionally, if the target
        // environment is a client API (such as Vulkan 1.1), then validate for that
        // client API version, to the extent that it is verifiable from data in the
        // binary itself.
        bool Validate(const std::vector<uint32_t>& binary) const;
        // Like the previous overload, but provides the binary as a pointer and size:
        // |binary_size| specifies the number of words in |binary|.
        // Validates for SPIR-V spec rules for the SPIR-V version named in the
        // binary's header (at word offset 1).  Additionally, if the target
        // environment is a client API (such as Vulkan 1.1), then validate for that
        // client API version, to the extent that it is verifiable from data in the
        // binary itself.
        bool Validate(const uint32_t* binary, size_t binary_size) const;
        // Like the previous overload, but takes an options object.
        // Validates for SPIR-V spec rules for the SPIR-V version named in the
        // binary's header (at word offset 1).  Additionally, if the target
        // environment is a client API (such as Vulkan 1.1), then validate for that
        // client API version, to the extent that it is verifiable from data in the
        // binary itself, or in the validator options.
        bool Validate(const uint32_t* binary, size_t binary_size,
            spv_validator_options options) const;

        // Was this object successfully constructed.
        bool IsValid() const;

    private:
        struct Impl;  // Opaque struct for holding the data fields used by this class.
        std::unique_ptr<Impl> impl_;  // Unique pointer to implementation data.
    };

}  // namespace spvtools

#endif  // INCLUDE_SPIRV_TOOLS_LIBSPIRV_HPP_


namespace spvtools {

    namespace opt {
        class Pass;
        struct DescriptorSetAndBinding;
    }  // namespace opt

    // C++ interface for SPIR-V optimization functionalities. It wraps the context
    // (including target environment and the corresponding SPIR-V grammar) and
    // provides methods for registering optimization passes and optimizing.
    //
    // Instances of this class provides basic thread-safety guarantee.
    class Optimizer {
    public:
        // The token for an optimization pass. It is returned via one of the
        // Create*Pass() standalone functions at the end of this header file and
        // consumed by the RegisterPass() method. Tokens are one-time objects that
        // only support move; copying is not allowed.
        struct PassToken {
            struct Impl;  // Opaque struct for holding internal data.

            PassToken(std::unique_ptr<Impl>);

            // Tokens for built-in passes should be created using Create*Pass functions
            // below; for out-of-tree passes, use this constructor instead.
            // Note that this API isn't guaranteed to be stable and may change without
            // preserving source or binary compatibility in the future.
            PassToken(std::unique_ptr<opt::Pass>&& pass);

            // Tokens can only be moved. Copying is disabled.
            PassToken(const PassToken&) = delete;
            PassToken(PassToken&&);
            PassToken& operator=(const PassToken&) = delete;
            PassToken& operator=(PassToken&&);

            ~PassToken();

            std::unique_ptr<Impl> impl_;  // Unique pointer to internal data.
        };

        // Constructs an instance with the given target |env|, which is used to decode
        // the binaries to be optimized later.
        //
        // The instance will have an empty message consumer, which ignores all
        // messages from the library. Use SetMessageConsumer() to supply a consumer
        // if messages are of concern.
        explicit Optimizer(spv_target_env env);

        // Disables copy/move constructor/assignment operations.
        Optimizer(const Optimizer&) = delete;
        Optimizer(Optimizer&&) = delete;
        Optimizer& operator=(const Optimizer&) = delete;
        Optimizer& operator=(Optimizer&&) = delete;

        // Destructs this instance.
        ~Optimizer();

        // Sets the message consumer to the given |consumer|. The |consumer| will be
        // invoked once for each message communicated from the library.
        void SetMessageConsumer(MessageConsumer consumer);

        // Returns a reference to the registered message consumer.
        const MessageConsumer& consumer() const;

        // Registers the given |pass| to this optimizer. Passes will be run in the
        // exact order of registration. The token passed in will be consumed by this
        // method.
        Optimizer& RegisterPass(PassToken&& pass);

        // Registers passes that attempt to improve performance of generated code.
        // This sequence of passes is subject to constant review and will change
        // from time to time.
        Optimizer& RegisterPerformancePasses();

        // Registers passes that attempt to improve the size of generated code.
        // This sequence of passes is subject to constant review and will change
        // from time to time.
        Optimizer& RegisterSizePasses();

        // Registers passes that attempt to legalize the generated code.
        //
        // Note: this recipe is specially designed for legalizing SPIR-V. It should be
        // used by compilers after translating HLSL source code literally. It should
        // *not* be used by general workloads for performance or size improvement.
        //
        // This sequence of passes is subject to constant review and will change
        // from time to time.
        Optimizer& RegisterLegalizationPasses();

        // Register passes specified in the list of |flags|.  Each flag must be a
        // string of a form accepted by Optimizer::FlagHasValidForm().
        //
        // If the list of flags contains an invalid entry, it returns false and an
        // error message is emitted to the MessageConsumer object (use
        // Optimizer::SetMessageConsumer to define a message consumer, if needed).
        //
        // If all the passes are registered successfully, it returns true.
        bool RegisterPassesFromFlags(const std::vector<std::string>& flags);

        // Registers the optimization pass associated with |flag|.  This only accepts
        // |flag| values of the form "--pass_name[=pass_args]".  If no such pass
        // exists, it returns false.  Otherwise, the pass is registered and it returns
        // true.
        //
        // The following flags have special meaning:
        //
        // -O: Registers all performance optimization passes
        //     (Optimizer::RegisterPerformancePasses)
        //
        // -Os: Registers all size optimization passes
        //      (Optimizer::RegisterSizePasses).
        //
        // --legalize-hlsl: Registers all passes that legalize SPIR-V generated by an
        //                  HLSL front-end.
        bool RegisterPassFromFlag(const std::string& flag);

        // Validates that |flag| has a valid format.  Strings accepted:
        //
        // --pass_name[=pass_args]
        // -O
        // -Os
        //
        // If |flag| takes one of the forms above, it returns true.  Otherwise, it
        // returns false.
        bool FlagHasValidForm(const std::string& flag) const;

        // Allows changing, after creation time, the target environment to be
        // optimized for and validated.  Should be called before calling Run().
        void SetTargetEnv(const spv_target_env env);

        // Optimizes the given SPIR-V module |original_binary| and writes the
        // optimized binary into |optimized_binary|. The optimized binary uses
        // the same SPIR-V version as the original binary.
        //
        // Returns true on successful optimization, whether or not the module is
        // modified. Returns false if |original_binary| fails to validate or if errors
        // occur when processing |original_binary| using any of the registered passes.
        // In that case, no further passes are executed and the contents in
        // |optimized_binary| may be invalid.
        //
        // By default, the binary is validated before any transforms are performed,
        // and optionally after each transform.  Validation uses SPIR-V spec rules
        // for the SPIR-V version named in the binary's header (at word offset 1).
        // Additionally, if the target environment is a client API (such as
        // Vulkan 1.1), then validate for that client API version, to the extent
        // that it is verifiable from data in the binary itself.
        //
        // It's allowed to alias |original_binary| to the start of |optimized_binary|.
        bool Run(const uint32_t* original_binary, size_t original_binary_size,
            std::vector<uint32_t>* optimized_binary) const;

        // DEPRECATED: Same as above, except passes |options| to the validator when
        // trying to validate the binary.  If |skip_validation| is true, then the
        // caller is guaranteeing that |original_binary| is valid, and the validator
        // will not be run.  The |max_id_bound| is the limit on the max id in the
        // module.
        bool Run(const uint32_t* original_binary, const size_t original_binary_size,
            std::vector<uint32_t>* optimized_binary,
            const ValidatorOptions& options, bool skip_validation) const;

        // Same as above, except it takes an options object.  See the documentation
        // for |OptimizerOptions| to see which options can be set.
        //
        // By default, the binary is validated before any transforms are performed,
        // and optionally after each transform.  Validation uses SPIR-V spec rules
        // for the SPIR-V version named in the binary's header (at word offset 1).
        // Additionally, if the target environment is a client API (such as
        // Vulkan 1.1), then validate for that client API version, to the extent
        // that it is verifiable from data in the binary itself, or from the
        // validator options set on the optimizer options.
        bool Run(const uint32_t* original_binary, const size_t original_binary_size,
            std::vector<uint32_t>* optimized_binary,
            const spv_optimizer_options opt_options) const;

        // Returns a vector of strings with all the pass names added to this
        // optimizer's pass manager. These strings are valid until the associated
        // pass manager is destroyed.
        std::vector<const char*> GetPassNames() const;

        // Sets the option to print the disassembly before each pass and after the
        // last pass.  If |out| is null, then no output is generated.  Otherwise,
        // output is sent to the |out| output stream.
        Optimizer& SetPrintAll(std::ostream* out);

        // Sets the option to print the resource utilization of each pass. If |out|
        // is null, then no output is generated. Otherwise, output is sent to the
        // |out| output stream.
        Optimizer& SetTimeReport(std::ostream* out);

        // Sets the option to validate the module after each pass.
        Optimizer& SetValidateAfterAll(bool validate);

    private:
        struct Impl;                  // Opaque struct for holding internal data.
        std::unique_ptr<Impl> impl_;  // Unique pointer to internal data.
    };

    // Creates a null pass.
    // A null pass does nothing to the SPIR-V module to be optimized.
    Optimizer::PassToken CreateNullPass();

    // Creates a strip-debug-info pass.
    // A strip-debug-info pass removes all debug instructions (as documented in
    // Section 3.42.2 of the SPIR-V spec) of the SPIR-V module to be optimized.
    Optimizer::PassToken CreateStripDebugInfoPass();

    // [Deprecated] This will create a strip-nonsemantic-info pass.  See below.
    Optimizer::PassToken CreateStripReflectInfoPass();

    // Creates a strip-nonsemantic-info pass.
    // A strip-nonsemantic-info pass removes all reflections and explicitly
    // non-semantic instructions.
    Optimizer::PassToken CreateStripNonSemanticInfoPass();

    // Creates an eliminate-dead-functions pass.
    // An eliminate-dead-functions pass will remove all functions that are not in
    // the call trees rooted at entry points and exported functions.  These
    // functions are not needed because they will never be called.
    Optimizer::PassToken CreateEliminateDeadFunctionsPass();

    // Creates an eliminate-dead-members pass.
    // An eliminate-dead-members pass will remove all unused members of structures.
    // This will not affect the data layout of the remaining members.
    Optimizer::PassToken CreateEliminateDeadMembersPass();

    // Creates a set-spec-constant-default-value pass from a mapping from spec-ids
    // to the default values in the form of string.
    // A set-spec-constant-default-value pass sets the default values for the
    // spec constants that have SpecId decorations (i.e., those defined by
    // OpSpecConstant{|True|False} instructions).
    Optimizer::PassToken CreateSetSpecConstantDefaultValuePass(
        const std::unordered_map<uint32_t, std::string>& id_value_map);

    // Creates a set-spec-constant-default-value pass from a mapping from spec-ids
    // to the default values in the form of bit pattern.
    // A set-spec-constant-default-value pass sets the default values for the
    // spec constants that have SpecId decorations (i.e., those defined by
    // OpSpecConstant{|True|False} instructions).
    Optimizer::PassToken CreateSetSpecConstantDefaultValuePass(
        const std::unordered_map<uint32_t, std::vector<uint32_t>>& id_value_map);

    // Creates a flatten-decoration pass.
    // A flatten-decoration pass replaces grouped decorations with equivalent
    // ungrouped decorations.  That is, it replaces each OpDecorationGroup
    // instruction and associated OpGroupDecorate and OpGroupMemberDecorate
    // instructions with equivalent OpDecorate and OpMemberDecorate instructions.
    // The pass does not attempt to preserve debug information for instructions
    // it removes.
    Optimizer::PassToken CreateFlattenDecorationPass();

    // Creates a freeze-spec-constant-value pass.
    // A freeze-spec-constant pass specializes the value of spec constants to
    // their default values. This pass only processes the spec constants that have
    // SpecId decorations (defined by OpSpecConstant, OpSpecConstantTrue, or
    // OpSpecConstantFalse instructions) and replaces them with their normal
    // counterparts (OpConstant, OpConstantTrue, or OpConstantFalse). The
    // corresponding SpecId annotation instructions will also be removed. This
    // pass does not fold the newly added normal constants and does not process
    // other spec constants defined by OpSpecConstantComposite or
    // OpSpecConstantOp.
    Optimizer::PassToken CreateFreezeSpecConstantValuePass();

    // Creates a fold-spec-constant-op-and-composite pass.
    // A fold-spec-constant-op-and-composite pass folds spec constants defined by
    // OpSpecConstantOp or OpSpecConstantComposite instruction, to normal Constants
    // defined by OpConstantTrue, OpConstantFalse, OpConstant, OpConstantNull, or
    // OpConstantComposite instructions. Note that spec constants defined with
    // OpSpecConstant, OpSpecConstantTrue, or OpSpecConstantFalse instructions are
    // not handled, as these instructions indicate their value are not determined
    // and can be changed in future. A spec constant is foldable if all of its
    // value(s) can be determined from the module. E.g., an integer spec constant
    // defined with OpSpecConstantOp instruction can be folded if its value won't
    // change later. This pass will replace the original OpSpecConstantOp
    // instruction with an OpConstant instruction. When folding composite spec
    // constants, new instructions may be inserted to define the components of the
    // composite constant first, then the original spec constants will be replaced
    // by OpConstantComposite instructions.
    //
    // There are some operations not supported yet:
    //   OpSConvert, OpFConvert, OpQuantizeToF16 and
    //   all the operations under Kernel capability.
    // TODO(qining): Add support for the operations listed above.
    Optimizer::PassToken CreateFoldSpecConstantOpAndCompositePass();

    // Creates a unify-constant pass.
    // A unify-constant pass de-duplicates the constants. Constants with the exact
    // same value and identical form will be unified and only one constant will
    // be kept for each unique pair of type and value.
    // There are several cases not handled by this pass:
    //  1) Constants defined by OpConstantNull instructions (null constants) and
    //  constants defined by OpConstantFalse, OpConstant or OpConstantComposite
    //  with value 0 (zero-valued normal constants) are not considered equivalent.
    //  So null constants won't be used to replace zero-valued normal constants,
    //  vice versa.
    //  2) Whenever there are decorations to the constant's result id id, the
    //  constant won't be handled, which means, it won't be used to replace any
    //  other constants, neither can other constants replace it.
    //  3) NaN in float point format with different bit patterns are not unified.
    Optimizer::PassToken CreateUnifyConstantPass();

    // Creates a eliminate-dead-constant pass.
    // A eliminate-dead-constant pass removes dead constants, including normal
    // constants defined by OpConstant, OpConstantComposite, OpConstantTrue, or
    // OpConstantFalse and spec constants defined by OpSpecConstant,
    // OpSpecConstantComposite, OpSpecConstantTrue, OpSpecConstantFalse or
    // OpSpecConstantOp.
    Optimizer::PassToken CreateEliminateDeadConstantPass();

    // Creates a strength-reduction pass.
    // A strength-reduction pass will look for opportunities to replace an
    // instruction with an equivalent and less expensive one.  For example,
    // multiplying by a power of 2 can be replaced by a bit shift.
    Optimizer::PassToken CreateStrengthReductionPass();

    // Creates a block merge pass.
    // This pass searches for blocks with a single Branch to a block with no
    // other predecessors and merges the blocks into a single block. Continue
    // blocks and Merge blocks are not candidates for the second block.
    //
    // The pass is most useful after Dead Branch Elimination, which can leave
    // such sequences of blocks. Merging them makes subsequent passes more
    // effective, such as single block local store-load elimination.
    //
    // While this pass reduces the number of occurrences of this sequence, at
    // this time it does not guarantee all such sequences are eliminated.
    //
    // Presence of phi instructions can inhibit this optimization. Handling
    // these is left for future improvements.
    Optimizer::PassToken CreateBlockMergePass();

    // Creates an exhaustive inline pass.
    // An exhaustive inline pass attempts to exhaustively inline all function
    // calls in all functions in an entry point call tree. The intent is to enable,
    // albeit through brute force, analysis and optimization across function
    // calls by subsequent optimization passes. As the inlining is exhaustive,
    // there is no attempt to optimize for size or runtime performance. Functions
    // that are not in the call tree of an entry point are not changed.
    Optimizer::PassToken CreateInlineExhaustivePass();

    // Creates an opaque inline pass.
    // An opaque inline pass inlines all function calls in all functions in all
    // entry point call trees where the called function contains an opaque type
    // in either its parameter types or return type. An opaque type is currently
    // defined as Image, Sampler or SampledImage. The intent is to enable, albeit
    // through brute force, analysis and optimization across these function calls
    // by subsequent passes in order to remove the storing of opaque types which is
    // not legal in Vulkan. Functions that are not in the call tree of an entry
    // point are not changed.
    Optimizer::PassToken CreateInlineOpaquePass();

    // Creates a single-block local variable load/store elimination pass.
    // For every entry point function, do single block memory optimization of
    // function variables referenced only with non-access-chain loads and stores.
    // For each targeted variable load, if previous store to that variable in the
    // block, replace the load's result id with the value id of the store.
    // If previous load within the block, replace the current load's result id
    // with the previous load's result id. In either case, delete the current
    // load. Finally, check if any remaining stores are useless, and delete store
    // and variable if possible.
    //
    // The presence of access chain references and function calls can inhibit
    // the above optimization.
    //
    // Only modules with relaxed logical addressing (see opt/instruction.h) are
    // currently processed.
    //
    // This pass is most effective if preceded by Inlining and
    // LocalAccessChainConvert. This pass will reduce the work needed to be done
    // by LocalSingleStoreElim and LocalMultiStoreElim.
    //
    // Only functions in the call tree of an entry point are processed.
    Optimizer::PassToken CreateLocalSingleBlockLoadStoreElimPass();

    // Create dead branch elimination pass.
    // For each entry point function, this pass will look for SelectionMerge
    // BranchConditionals with constant condition and convert to a Branch to
    // the indicated label. It will delete resulting dead blocks.
    //
    // For all phi functions in merge block, replace all uses with the id
    // corresponding to the living predecessor.
    //
    // Note that some branches and blocks may be left to avoid creating invalid
    // control flow. Improving this is left to future work.
    //
    // This pass is most effective when preceded by passes which eliminate
    // local loads and stores, effectively propagating constant values where
    // possible.
    Optimizer::PassToken CreateDeadBranchElimPass();

    // Creates an SSA local variable load/store elimination pass.
    // For every entry point function, eliminate all loads and stores of function
    // scope variables only referenced with non-access-chain loads and stores.
    // Eliminate the variables as well.
    //
    // The presence of access chain references and function calls can inhibit
    // the above optimization.
    //
    // Only shader modules with relaxed logical addressing (see opt/instruction.h)
    // are currently processed. Currently modules with any extensions enabled are
    // not processed. This is left for future work.
    //
    // This pass is most effective if preceded by Inlining and
    // LocalAccessChainConvert. LocalSingleStoreElim and LocalSingleBlockElim
    // will reduce the work that this pass has to do.
    Optimizer::PassToken CreateLocalMultiStoreElimPass();

    // Creates a local access chain conversion pass.
    // A local access chain conversion pass identifies all function scope
    // variables which are accessed only with loads, stores and access chains
    // with constant indices. It then converts all loads and stores of such
    // variables into equivalent sequences of loads, stores, extracts and inserts.
    //
    // This pass only processes entry point functions. It currently only converts
    // non-nested, non-ptr access chains. It does not process modules with
    // non-32-bit integer types present. Optional memory access options on loads
    // and stores are ignored as we are only processing function scope variables.
    //
    // This pass unifies access to these variables to a single mode and simplifies
    // subsequent analysis and elimination of these variables along with their
    // loads and stores allowing values to propagate to their points of use where
    // possible.
    Optimizer::PassToken CreateLocalAccessChainConvertPass();

    // Creates a local single store elimination pass.
    // For each entry point function, this pass eliminates loads and stores for
    // function scope variable that are stored to only once, where possible. Only
    // whole variable loads and stores are eliminated; access-chain references are
    // not optimized. Replace all loads of such variables with the value that is
    // stored and eliminate any resulting dead code.
    //
    // Currently, the presence of access chains and function calls can inhibit this
    // pass, however the Inlining and LocalAccessChainConvert passes can make it
    // more effective. In additional, many non-load/store memory operations are
    // not supported and will prohibit optimization of a function. Support of
    // these operations are future work.
    //
    // Only shader modules with relaxed logical addressing (see opt/instruction.h)
    // are currently processed.
    //
    // This pass will reduce the work needed to be done by LocalSingleBlockElim
    // and LocalMultiStoreElim and can improve the effectiveness of other passes
    // such as DeadBranchElimination which depend on values for their analysis.
    Optimizer::PassToken CreateLocalSingleStoreElimPass();

    // Creates an insert/extract elimination pass.
    // This pass processes each entry point function in the module, searching for
    // extracts on a sequence of inserts. It further searches the sequence for an
    // insert with indices identical to the extract. If such an insert can be
    // found before hitting a conflicting insert, the extract's result id is
    // replaced with the id of the values from the insert.
    //
    // Besides removing extracts this pass enables subsequent dead code elimination
    // passes to delete the inserts. This pass performs best after access chains are
    // converted to inserts and extracts and local loads and stores are eliminated.
    Optimizer::PassToken CreateInsertExtractElimPass();

    // Creates a dead insert elimination pass.
    // This pass processes each entry point function in the module, searching for
    // unreferenced inserts into composite types. These are most often unused
    // stores to vector components. They are unused because they are never
    // referenced, or because there is another insert to the same component between
    // the insert and the reference. After removing the inserts, dead code
    // elimination is attempted on the inserted values.
    //
    // This pass performs best after access chains are converted to inserts and
    // extracts and local loads and stores are eliminated. While executing this
    // pass can be advantageous on its own, it is also advantageous to execute
    // this pass after CreateInsertExtractPass() as it will remove any unused
    // inserts created by that pass.
    Optimizer::PassToken CreateDeadInsertElimPass();

    // Create aggressive dead code elimination pass
    // This pass eliminates unused code from the module. In addition,
    // it detects and eliminates code which may have spurious uses but which do
    // not contribute to the output of the function. The most common cause of
    // such code sequences is summations in loops whose result is no longer used
    // due to dead code elimination. This optimization has additional compile
    // time cost over standard dead code elimination.
    //
    // This pass only processes entry point functions. It also only processes
    // shaders with relaxed logical addressing (see opt/instruction.h). It
    // currently will not process functions with function calls. Unreachable
    // functions are deleted.
    //
    // This pass will be made more effective by first running passes that remove
    // dead control flow and inlines function calls.
    //
    // This pass can be especially useful after running Local Access Chain
    // Conversion, which tends to cause cycles of dead code to be left after
    // Store/Load elimination passes are completed. These cycles cannot be
    // eliminated with standard dead code elimination.
    //
    // If |preserve_interface| is true, all non-io variables in the entry point
    // interface are considered live and are not eliminated. This mode is needed
    // by GPU-Assisted validation instrumentation, where a change in the interface
    // is not allowed.
    //
    // If |remove_outputs| is true, allow outputs to be removed from the interface.
    // This is only safe if the caller knows that there is no corresponding input
    // variable in the following shader. It is false by default.
    Optimizer::PassToken CreateAggressiveDCEPass(bool preserve_interface = false,
        bool remove_outputs = false);

    // Creates a remove-unused-interface-variables pass.
    // Removes variables referenced on the |OpEntryPoint| instruction that are not
    // referenced in the entry point function or any function in its call tree. Note
    // that this could cause the shader interface to no longer match other shader
    // stages.
    Optimizer::PassToken CreateRemoveUnusedInterfaceVariablesPass();

    // Creates an empty pass.
    // This is deprecated and will be removed.
    // TODO(jaebaek): remove this pass after handling glslang's broken unit tests.
    //                https://github.com/KhronosGroup/glslang/pull/2440
    Optimizer::PassToken CreatePropagateLineInfoPass();

    // Creates an empty pass.
    // This is deprecated and will be removed.
    // TODO(jaebaek): remove this pass after handling glslang's broken unit tests.
    //                https://github.com/KhronosGroup/glslang/pull/2440
    Optimizer::PassToken CreateRedundantLineInfoElimPass();

    // Creates a compact ids pass.
    // The pass remaps result ids to a compact and gapless range starting from %1.
    Optimizer::PassToken CreateCompactIdsPass();

    // Creates a remove duplicate pass.
    // This pass removes various duplicates:
    // * duplicate capabilities;
    // * duplicate extended instruction imports;
    // * duplicate types;
    // * duplicate decorations.
    Optimizer::PassToken CreateRemoveDuplicatesPass();

    // Creates a CFG cleanup pass.
    // This pass removes cruft from the control flow graph of functions that are
    // reachable from entry points and exported functions. It currently includes the
    // following functionality:
    //
    // - Removal of unreachable basic blocks.
    Optimizer::PassToken CreateCFGCleanupPass();

    // Create dead variable elimination pass.
    // This pass will delete module scope variables, along with their decorations,
    // that are not referenced.
    Optimizer::PassToken CreateDeadVariableEliminationPass();

    // create merge return pass.
    // changes functions that have multiple return statements so they have a single
    // return statement.
    //
    // for structured control flow it is assumed that the only unreachable blocks in
    // the function are trivial merge and continue blocks.
    //
    // a trivial merge block contains the label and an opunreachable instructions,
    // nothing else.  a trivial continue block contain a label and an opbranch to
    // the header, nothing else.
    //
    // these conditions are guaranteed to be met after running dead-branch
    // elimination.
    Optimizer::PassToken CreateMergeReturnPass();

    // Create value numbering pass.
    // This pass will look for instructions in the same basic block that compute the
    // same value, and remove the redundant ones.
    Optimizer::PassToken CreateLocalRedundancyEliminationPass();

    // Create LICM pass.
    // This pass will look for invariant instructions inside loops and hoist them to
    // the loops preheader.
    Optimizer::PassToken CreateLoopInvariantCodeMotionPass();

    // Creates a loop fission pass.
    // This pass will split all top level loops whose register pressure exceedes the
    // given |threshold|.
    Optimizer::PassToken CreateLoopFissionPass(size_t threshold);

    // Creates a loop fusion pass.
    // This pass will look for adjacent loops that are compatible and legal to be
    // fused. The fuse all such loops as long as the register usage for the fused
    // loop stays under the threshold defined by |max_registers_per_loop|.
    Optimizer::PassToken CreateLoopFusionPass(size_t max_registers_per_loop);

    // Creates a loop peeling pass.
    // This pass will look for conditions inside a loop that are true or false only
    // for the N first or last iteration. For loop with such condition, those N
    // iterations of the loop will be executed outside of the main loop.
    // To limit code size explosion, the loop peeling can only happen if the code
    // size growth for each loop is under |code_growth_threshold|.
    Optimizer::PassToken CreateLoopPeelingPass();

    // Creates a loop unswitch pass.
    // This pass will look for loop independent branch conditions and move the
    // condition out of the loop and version the loop based on the taken branch.
    // Works best after LICM and local multi store elimination pass.
    Optimizer::PassToken CreateLoopUnswitchPass();

    // Create global value numbering pass.
    // This pass will look for instructions where the same value is computed on all
    // paths leading to the instruction.  Those instructions are deleted.
    Optimizer::PassToken CreateRedundancyEliminationPass();

    // Create scalar replacement pass.
    // This pass replaces composite function scope variables with variables for each
    // element if those elements are accessed individually.  The parameter is a
    // limit on the number of members in the composite variable that the pass will
    // consider replacing.
    Optimizer::PassToken CreateScalarReplacementPass(uint32_t size_limit = 100);

    // Create a private to local pass.
    // This pass looks for variables declared in the private storage class that are
    // used in only one function.  Those variables are moved to the function storage
    // class in the function that they are used.
    Optimizer::PassToken CreatePrivateToLocalPass();

    // Creates a conditional constant propagation (CCP) pass.
    // This pass implements the SSA-CCP algorithm in
    //
    //      Constant propagation with conditional branches,
    //      Wegman and Zadeck, ACM TOPLAS 13(2):181-210.
    //
    // Constant values in expressions and conditional jumps are folded and
    // simplified. This may reduce code size by removing never executed jump targets
    // and computations with constant operands.
    Optimizer::PassToken CreateCCPPass();

    // Creates a workaround driver bugs pass.  This pass attempts to work around
    // a known driver bug (issue #1209) by identifying the bad code sequences and
    // rewriting them.
    //
    // Current workaround: Avoid OpUnreachable instructions in loops.
    Optimizer::PassToken CreateWorkaround1209Pass();

    // Creates a pass that converts if-then-else like assignments into OpSelect.
    Optimizer::PassToken CreateIfConversionPass();

    // Creates a pass that will replace instructions that are not valid for the
    // current shader stage by constants.  Has no effect on non-shader modules.
    Optimizer::PassToken CreateReplaceInvalidOpcodePass();

    // Creates a pass that simplifies instructions using the instruction folder.
    Optimizer::PassToken CreateSimplificationPass();

    // Create loop unroller pass.
    // Creates a pass to unroll loops which have the "Unroll" loop control
    // mask set. The loops must meet a specific criteria in order to be unrolled
    // safely this criteria is checked before doing the unroll by the
    // LoopUtils::CanPerformUnroll method. Any loop that does not meet the criteria
    // won't be unrolled. See CanPerformUnroll LoopUtils.h for more information.
    Optimizer::PassToken CreateLoopUnrollPass(bool fully_unroll, int factor = 0);

    // Create the SSA rewrite pass.
    // This pass converts load/store operations on function local variables into
    // operations on SSA IDs.  This allows SSA optimizers to act on these variables.
    // Only variables that are local to the function and of supported types are
    // processed (see IsSSATargetVar for details).
    Optimizer::PassToken CreateSSARewritePass();

    // Create pass to convert relaxed precision instructions to half precision.
    // This pass converts as many relaxed float32 arithmetic operations to half as
    // possible. It converts any float32 operands to half if needed. It converts
    // any resulting half precision values back to float32 as needed. No variables
    // are changed. No image operations are changed.
    //
    // Best if run after function scope store/load and composite operation
    // eliminations are run. Also best if followed by instruction simplification,
    // redundancy elimination and DCE.
    Optimizer::PassToken CreateConvertRelaxedToHalfPass();

    // Create relax float ops pass.
    // This pass decorates all float32 result instructions with RelaxedPrecision
    // if not already so decorated.
    Optimizer::PassToken CreateRelaxFloatOpsPass();

    // Create copy propagate arrays pass.
    // This pass looks to copy propagate memory references for arrays.  It looks
    // for specific code patterns to recognize array copies.
    Optimizer::PassToken CreateCopyPropagateArraysPass();

    // Create a vector dce pass.
    // This pass looks for components of vectors that are unused, and removes them
    // from the vector.  Note this would still leave around lots of dead code that
    // a pass of ADCE will be able to remove.
    Optimizer::PassToken CreateVectorDCEPass();

    // Create a pass to reduce the size of loads.
    // This pass looks for loads of structures where only a few of its members are
    // used.  It replaces the loads feeding an OpExtract with an OpAccessChain and
    // a load of the specific elements.  The parameter is a threshold to determine
    // whether we have to replace the load or not.  If the ratio of the used
    // components of the load is less than the threshold, we replace the load.
    Optimizer::PassToken CreateReduceLoadSizePass(
        double load_replacement_threshold = 0.9);

    // Create a pass to combine chained access chains.
    // This pass looks for access chains fed by other access chains and combines
    // them into a single instruction where possible.
    Optimizer::PassToken CreateCombineAccessChainsPass();

    // Create a pass to instrument bindless descriptor checking
    // This pass instruments all bindless references to check that descriptor
    // array indices are inbounds, and if the descriptor indexing extension is
    // enabled, that the descriptor has been initialized. If the reference is
    // invalid, a record is written to the debug output buffer (if space allows)
    // and a null value is returned. This pass is designed to support bindless
    // validation in the Vulkan validation layers.
    //
    // TODO(greg-lunarg): Add support for buffer references. Currently only does
    // checking for image references.
    //
    // Dead code elimination should be run after this pass as the original,
    // potentially invalid code is not removed and could cause undefined behavior,
    // including crashes. It may also be beneficial to run Simplification
    // (ie Constant Propagation), DeadBranchElim and BlockMerge after this pass to
    // optimize instrument code involving the testing of compile-time constants.
    // It is also generally recommended that this pass (and all
    // instrumentation passes) be run after any legalization and optimization
    // passes. This will give better analysis for the instrumentation and avoid
    // potentially de-optimizing the instrument code, for example, inlining
    // the debug record output function throughout the module.
    //
    // The instrumentation will read and write buffers in debug
    // descriptor set |desc_set|. It will write |shader_id| in each output record
    // to identify the shader module which generated the record.
    // |desc_length_enable| controls instrumentation of runtime descriptor array
    // references, |desc_init_enable| controls instrumentation of descriptor
    // initialization checking, and |buff_oob_enable| controls instrumentation
    // of storage and uniform buffer bounds checking, all of which require input
    // buffer support. |texbuff_oob_enable| controls instrumentation of texel
    // buffers, which does not require input buffer support.
    Optimizer::PassToken CreateInstBindlessCheckPass(
        uint32_t desc_set, uint32_t shader_id, bool desc_length_enable = false,
        bool desc_init_enable = false, bool buff_oob_enable = false,
        bool texbuff_oob_enable = false);

    // Create a pass to instrument physical buffer address checking
    // This pass instruments all physical buffer address references to check that
    // all referenced bytes fall in a valid buffer. If the reference is
    // invalid, a record is written to the debug output buffer (if space allows)
    // and a null value is returned. This pass is designed to support buffer
    // address validation in the Vulkan validation layers.
    //
    // Dead code elimination should be run after this pass as the original,
    // potentially invalid code is not removed and could cause undefined behavior,
    // including crashes. Instruction simplification would likely also be
    // beneficial. It is also generally recommended that this pass (and all
    // instrumentation passes) be run after any legalization and optimization
    // passes. This will give better analysis for the instrumentation and avoid
    // potentially de-optimizing the instrument code, for example, inlining
    // the debug record output function throughout the module.
    //
    // The instrumentation will read and write buffers in debug
    // descriptor set |desc_set|. It will write |shader_id| in each output record
    // to identify the shader module which generated the record.
    Optimizer::PassToken CreateInstBuffAddrCheckPass(uint32_t desc_set,
        uint32_t shader_id);

    // Create a pass to instrument OpDebugPrintf instructions.
    // This pass replaces all OpDebugPrintf instructions with instructions to write
    // a record containing the string id and the all specified values into a special
    // printf output buffer (if space allows). This pass is designed to support
    // the printf validation in the Vulkan validation layers.
    //
    // The instrumentation will write buffers in debug descriptor set |desc_set|.
    // It will write |shader_id| in each output record to identify the shader
    // module which generated the record.
    Optimizer::PassToken CreateInstDebugPrintfPass(uint32_t desc_set,
        uint32_t shader_id);

    // Create a pass to upgrade to the VulkanKHR memory model.
    // This pass upgrades the Logical GLSL450 memory model to Logical VulkanKHR.
    // Additionally, it modifies memory, image, atomic and barrier operations to
    // conform to that model's requirements.
    Optimizer::PassToken CreateUpgradeMemoryModelPass();

    // Create a pass to do code sinking.  Code sinking is a transformation
    // where an instruction is moved into a more deeply nested construct.
    Optimizer::PassToken CreateCodeSinkingPass();

    // Create a pass to fix incorrect storage classes.  In order to make code
    // generation simpler, DXC may generate code where the storage classes do not
    // match up correctly.  This pass will fix the errors that it can.
    Optimizer::PassToken CreateFixStorageClassPass();

    // Creates a graphics robust access pass.
    //
    // This pass injects code to clamp indexed accesses to buffers and internal
    // arrays, providing guarantees satisfying Vulkan's robustBufferAccess rules.
    //
    // TODO(dneto): Clamps coordinates and sample index for pointer calculations
    // into storage images (OpImageTexelPointer).  For an cube array image, it
    // assumes the maximum layer count times 6 is at most 0xffffffff.
    //
    // NOTE: This pass will fail with a message if:
    // - The module is not a Shader module.
    // - The module declares VariablePointers, VariablePointersStorageBuffer, or
    //   RuntimeDescriptorArrayEXT capabilities.
    // - The module uses an addressing model other than Logical
    // - Access chain indices are wider than 64 bits.
    // - Access chain index for a struct is not an OpConstant integer or is out
    //   of range. (The module is already invalid if that is the case.)
    // - TODO(dneto): The OpImageTexelPointer coordinate component is not 32-bits
    // wide.
    //
    // NOTE: Access chain indices are always treated as signed integers.  So
    //   if an array has a fixed size of more than 2^31 elements, then elements
    //   from 2^31 and above are never accessible with a 32-bit index,
    //   signed or unsigned.  For this case, this pass will clamp the index
    //   between 0 and at 2^31-1, inclusive.
    //   Similarly, if an array has more then 2^15 element and is accessed with
    //   a 16-bit index, then elements from 2^15 and above are not accessible.
    //   In this case, the pass will clamp the index between 0 and 2^15-1
    //   inclusive.
    Optimizer::PassToken CreateGraphicsRobustAccessPass();

    // Create a pass to spread Volatile semantics to variables with SMIDNV,
    // WarpIDNV, SubgroupSize, SubgroupLocalInvocationId, SubgroupEqMask,
    // SubgroupGeMask, SubgroupGtMask, SubgroupLeMask, or SubgroupLtMask BuiltIn
    // decorations or OpLoad for them when the shader model is the ray generation,
    // closest hit, miss, intersection, or callable. This pass can be used for
    // VUID-StandaloneSpirv-VulkanMemoryModel-04678 and
    // VUID-StandaloneSpirv-VulkanMemoryModel-04679 (See "Standalone SPIR-V
    // Validation" section of Vulkan spec "Appendix A: Vulkan Environment for
    // SPIR-V"). When the SPIR-V version is 1.6 or above, the pass also spreads
    // the Volatile semantics to a variable with HelperInvocation BuiltIn decoration
    // in the fragement shader.
    Optimizer::PassToken CreateSpreadVolatileSemanticsPass();

    // Create a pass to replace a descriptor access using variable index.
    // This pass replaces every access using a variable index to array variable
    // |desc| that has a DescriptorSet and Binding decorations with a constant
    // element of the array. In order to replace the access using a variable index
    // with the constant element, it uses a switch statement.
    Optimizer::PassToken CreateReplaceDescArrayAccessUsingVarIndexPass();

    // Create descriptor scalar replacement pass.
    // This pass replaces every array variable |desc| that has a DescriptorSet and
    // Binding decorations with a new variable for each element of the array.
    // Suppose |desc| was bound at binding |b|.  Then the variable corresponding to
    // |desc[i]| will have binding |b+i|.  The descriptor set will be the same.  It
    // is assumed that no other variable already has a binding that will used by one
    // of the new variables.  If not, the pass will generate invalid Spir-V.  All
    // accesses to |desc| must be OpAccessChain instructions with a literal index
    // for the first index.
    Optimizer::PassToken CreateDescriptorScalarReplacementPass();

    // Create a pass to replace each OpKill instruction with a function call to a
    // function that has a single OpKill.  Also replace each OpTerminateInvocation
    // instruction  with a function call to a function that has a single
    // OpTerminateInvocation.  This allows more code to be inlined.
    Optimizer::PassToken CreateWrapOpKillPass();

    // Replaces the extensions VK_AMD_shader_ballot,VK_AMD_gcn_shader, and
    // VK_AMD_shader_trinary_minmax with equivalent code using core instructions and
    // capabilities.
    Optimizer::PassToken CreateAmdExtToKhrPass();

    // Replaces the internal version of GLSLstd450 InterpolateAt* extended
    // instructions with the externally valid version. The internal version allows
    // an OpLoad of the interpolant for the first argument. This pass removes the
    // OpLoad and replaces it with its pointer. glslang and possibly other
    // frontends will create the internal version for HLSL. This pass will be part
    // of HLSL legalization and should be called after interpolants have been
    // propagated into their final positions.
    Optimizer::PassToken CreateInterpolateFixupPass();

    // Removes unused components from composite input variables. Current
    // implementation just removes trailing unused components from input arrays
    // and structs. The pass performs best after maximizing dead code removal.
    // A subsequent dead code elimination pass would be beneficial in removing
    // newly unused component types.
    //
    // WARNING: This pass can only be safely applied standalone to vertex shaders
    // as it can otherwise cause interface incompatibilities with the preceding
    // shader in the pipeline. If applied to non-vertex shaders, the user should
    // follow by applying EliminateDeadOutputStores and
    // EliminateDeadOutputComponents to the preceding shader.
    Optimizer::PassToken CreateEliminateDeadInputComponentsPass();

    // Removes unused components from composite output variables. Current
    // implementation just removes trailing unused components from output arrays
    // and structs. The pass performs best after eliminating dead output stores.
    // A subsequent dead code elimination pass would be beneficial in removing
    // newly unused component types. Currently only supports vertex and fragment
    // shaders.
    //
    // WARNING: This pass cannot be safely applied standalone as it can cause
    // interface incompatibility with the following shader in the pipeline. The
    // user should first apply EliminateDeadInputComponents to the following
    // shader, then apply EliminateDeadOutputStores to this shader.
    Optimizer::PassToken CreateEliminateDeadOutputComponentsPass();

    // Removes unused components from composite input variables. This safe
    // version will not cause interface incompatibilities since it only changes
    // vertex shaders. The current implementation just removes trailing unused
    // components from input structs and input arrays. The pass performs best
    // after maximizing dead code removal. A subsequent dead code elimination
    // pass would be beneficial in removing newly unused component types.
    Optimizer::PassToken CreateEliminateDeadInputComponentsSafePass();

    // Analyzes shader and populates |live_locs| and |live_builtins|. Best results
    // will be obtained if shader has all dead code eliminated first. |live_locs|
    // and |live_builtins| are subsequently used when calling
    // CreateEliminateDeadOutputStoresPass on the preceding shader. Currently only
    // supports tesc, tese, geom, and frag shaders.
    Optimizer::PassToken CreateAnalyzeLiveInputPass(
        std::unordered_set<uint32_t>* live_locs,
        std::unordered_set<uint32_t>* live_builtins);

    // Removes stores to output locations not listed in |live_locs| or
    // |live_builtins|. Best results are obtained if constant propagation is
    // performed first. A subsequent call to ADCE will eliminate any dead code
    // created by the removal of the stores. A subsequent call to
    // CreateEliminateDeadOutputComponentsPass will eliminate any dead output
    // components created by the elimination of the stores. Currently only supports
    // vert, tesc, tese, and geom shaders.
    Optimizer::PassToken CreateEliminateDeadOutputStoresPass(
        std::unordered_set<uint32_t>* live_locs,
        std::unordered_set<uint32_t>* live_builtins);

    // Creates a convert-to-sampled-image pass to convert images and/or
    // samplers with given pairs of descriptor set and binding to sampled image.
    // If a pair of an image and a sampler have the same pair of descriptor set and
    // binding that is one of the given pairs, they will be converted to a sampled
    // image. In addition, if only an image has the descriptor set and binding that
    // is one of the given pairs, it will be converted to a sampled image as well.
    Optimizer::PassToken CreateConvertToSampledImagePass(
        const std::vector<opt::DescriptorSetAndBinding>&
        descriptor_set_binding_pairs);

    // Create an interface-variable-scalar-replacement pass that replaces array or
    // matrix interface variables with a series of scalar or vector interface
    // variables. For example, it replaces `float3 foo[2]` with `float3 foo0, foo1`.
    Optimizer::PassToken CreateInterfaceVariableScalarReplacementPass();

    // Creates a remove-dont-inline pass to remove the |DontInline| function control
    // from every function in the module.  This is useful if you want the inliner to
    // inline these functions some reason.
    Optimizer::PassToken CreateRemoveDontInlinePass();
    // Create a fix-func-call-param pass to fix non memory argument for the function
    // call, as spirv-validation requires function parameters to be an memory
    // object, currently the pass would remove accesschain pointer argument passed
    // to the function
    Optimizer::PassToken CreateFixFuncCallArgumentsPass();
}  // namespace spvtools

#endif  // INCLUDE_SPIRV_TOOLS_OPTIMIZER_HPP_

#include <ShaderLang.h>

#include <backend/DriverEnums.h>

#include <utils/FixedCapacityVector.h>

#include <memory>
#include <string>
#include <vector>

namespace filament {
class SamplerInterfaceBlock;
};

namespace filamat {

using SpirvBlob = std::vector<uint32_t>;
using BindingPointAndSib = std::pair<uint8_t, const filament::SamplerInterfaceBlock*>;
using SibVector = utils::FixedCapacityVector<BindingPointAndSib>;

class GLSLPostProcessor {
public:
    enum Flags : uint32_t {
        PRINT_SHADERS = 1 << 0,
        GENERATE_DEBUG_INFO = 1 << 1,
    };

    GLSLPostProcessor(MaterialBuilder::Optimization optimization, uint32_t flags);

    ~GLSLPostProcessor();

    struct Config {
        filament::Variant variant;
        MaterialBuilder::TargetApi targetApi;
        MaterialBuilder::TargetLanguage targetLanguage;
        filament::backend::ShaderStage shaderType;
        filament::backend::ShaderModel shaderModel;
        filament::backend::FeatureLevel featureLevel;
        filament::MaterialDomain domain;
        const filamat::MaterialInfo* materialInfo;
        bool hasFramebufferFetch;
        bool usesClipDistance;
        struct {
            std::vector<std::pair<uint32_t, uint32_t>> subpassInputToColorLocation;
        } glsl;
    };

    bool process(const std::string& inputShader, Config const& config,
            std::string* outputGlsl,
            SpirvBlob* outputSpirv,
            std::string* outputMsl);

    // public so backend_test can also use it
    static void spirvToMsl(const SpirvBlob* spirv, std::string* outMsl,
            filament::backend::ShaderModel shaderModel, bool useFramebufferFetch,
            const SibVector& sibs, const ShaderMinifier* minifier);

private:
    struct InternalConfig {
        std::string* glslOutput = nullptr;
        SpirvBlob* spirvOutput = nullptr;
        std::string* mslOutput = nullptr;
        EShLanguage shLang = EShLangFragment;
        // use 100 for ES environment, 110 for desktop
         int langVersion = 0;
        ShaderMinifier minifier;
    };

    bool fullOptimization(const glslang::TShader& tShader,
            GLSLPostProcessor::Config const& config, InternalConfig& internalConfig) const;

    void preprocessOptimization(glslang::TShader& tShader,
            GLSLPostProcessor::Config const& config, InternalConfig& internalConfig) const;

    /**
     * Retrieve an optimizer instance tuned for the given optimization level and shader configuration.
     */
    using OptimizerPtr = std::shared_ptr<spvtools::Optimizer>;
    static OptimizerPtr createOptimizer(
            MaterialBuilder::Optimization optimization,
            Config const& config);

    static void registerSizePasses(spvtools::Optimizer& optimizer, Config const& config);
    static void registerPerformancePasses(spvtools::Optimizer& optimizer, Config const& config);

    void optimizeSpirv(OptimizerPtr optimizer, SpirvBlob& spirv) const;

    void fixupClipDistance(SpirvBlob& spirv, GLSLPostProcessor::Config const& config) const;

    const MaterialBuilder::Optimization mOptimization;
    const bool mPrintShaders;
    const bool mGenerateDebugInfo;
};

} // namespace filamat

#endif //TNT_GLSLPOSTPROCESSOR_H
