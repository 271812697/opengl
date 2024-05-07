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

#include "GLSLPostProcessor.h"
 //
 // Copyright (C) 2014 LunarG, Inc.
 // Copyright (C) 2015-2018 Google, Inc.
 //
 // All rights reserved.
 //
 // Redistribution and use in source and binary forms, with or without
 // modification, are permitted provided that the following conditions
 // are met:
 //
 //    Redistributions of source code must retain the above copyright
 //    notice, this list of conditions and the following disclaimer.
 //
 //    Redistributions in binary form must reproduce the above
 //    copyright notice, this list of conditions and the following
 //    disclaimer in the documentation and/or other materials provided
 //    with the distribution.
 //
 //    Neither the name of 3Dlabs Inc. Ltd. nor the names of its
 //    contributors may be used to endorse or promote products derived
 //    from this software without specific prior written permission.
 //
 // THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 // "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 // LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 // FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 // COPYRIGHT HOLDERS OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 // INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 // BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 // LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 // CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 // LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 // ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 // POSSIBILITY OF SUCH DAMAGE.

#pragma once

#if defined(_MSC_VER) && _MSC_VER >= 1900
#pragma warning(disable : 4464) // relative include path contains '..'
#endif

//
// Copyright (C) 2014-2016 LunarG, Inc.
// Copyright (C) 2018 Google, Inc.
//
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions
// are met:
//
//    Redistributions of source code must retain the above copyright
//    notice, this list of conditions and the following disclaimer.
//
//    Redistributions in binary form must reproduce the above
//    copyright notice, this list of conditions and the following
//    disclaimer in the documentation and/or other materials provided
//    with the distribution.
//
//    Neither the name of 3Dlabs Inc. Ltd. nor the names of its
//    contributors may be used to endorse or promote products derived
//    from this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
// FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
// COPYRIGHT HOLDERS OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
// INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
// BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
// LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
// CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
// LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
// ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
// POSSIBILITY OF SUCH DAMAGE.

//
// Call into SPIRV-Tools to disassemble, validate, and optimize.
//

#pragma once
#ifndef GLSLANG_SPV_TOOLS_H
#define GLSLANG_SPV_TOOLS_H

#if ENABLE_OPT
#include <vector>
#include <ostream>
#include "spirv-tools/libspirv.h"
#endif

#include "glslang/MachineIndependent/localintermediate.h"
#ifndef GLSLANG_SPIRV_LOGGER_H
#define GLSLANG_SPIRV_LOGGER_H

#include <string>
#include <vector>

namespace spv {

    // A class for holding all SPIR-V build status messages, including
    // missing/TBD functionalities, warnings, and errors.
    class SpvBuildLogger {
    public:
        SpvBuildLogger() {}

        // Registers a TBD functionality.
        void tbdFunctionality(const std::string& f);
        // Registers a missing functionality.
        void missingFunctionality(const std::string& f);

        // Logs a warning.
        void warning(const std::string& w) { warnings.push_back(w); }
        // Logs an error.
        void error(const std::string& e) { errors.push_back(e); }

        // Returns all messages accumulated in the order of:
        // TBD functionalities, missing functionalities, warnings, errors.
        std::string getAllMessages() const;

    private:
        SpvBuildLogger(const SpvBuildLogger&);

        std::vector<std::string> tbdFeatures;
        std::vector<std::string> missingFeatures;
        std::vector<std::string> warnings;
        std::vector<std::string> errors;
    };

} // end spv namespace

#endif // GLSLANG_SPIRV_LOGGER_H

namespace glslang {

    struct SpvOptions {
        bool generateDebugInfo{ false };
        bool stripDebugInfo{ false };
        bool disableOptimizer{ true };
        bool optimizeSize{ false };
        bool disassemble{ false };
        bool validate{ false };
        bool emitNonSemanticShaderDebugInfo{ false };
        bool emitNonSemanticShaderDebugSource{ false };
        bool compileOnly{ false };
    };

#if ENABLE_OPT

    // Translate glslang's view of target versioning to what SPIRV-Tools uses.
    spv_target_env MapToSpirvToolsEnv(const SpvVersion& spvVersion, spv::SpvBuildLogger* logger);

    // Use the SPIRV-Tools disassembler to print SPIR-V using a SPV_ENV_UNIVERSAL_1_3 environment.
    void SpirvToolsDisassemble(std::ostream& out, const std::vector<unsigned int>& spirv);

    // Use the SPIRV-Tools disassembler to print SPIR-V with a provided SPIR-V environment.
    void SpirvToolsDisassemble(std::ostream& out, const std::vector<unsigned int>& spirv,
        spv_target_env requested_context);

    // Apply the SPIRV-Tools validator to generated SPIR-V.
    void SpirvToolsValidate(const glslang::TIntermediate& intermediate, std::vector<unsigned int>& spirv,
        spv::SpvBuildLogger*, bool prelegalization);

    // Apply the SPIRV-Tools optimizer to generated SPIR-V.  HLSL SPIR-V is legalized in the process.
    void SpirvToolsTransform(const glslang::TIntermediate& intermediate, std::vector<unsigned int>& spirv,
        spv::SpvBuildLogger*, const SpvOptions*);

    // Apply the SPIRV-Tools EliminateDeadInputComponents pass to generated SPIR-V. Put result in |spirv|.
    void SpirvToolsEliminateDeadInputComponents(spv_target_env target_env, std::vector<unsigned int>& spirv,
        spv::SpvBuildLogger*);

    // Apply the SPIRV-Tools AnalyzeDeadOutputStores pass to generated SPIR-V. Put result in |live_locs|.
    // Return true if the result is valid.
    bool SpirvToolsAnalyzeDeadOutputStores(spv_target_env target_env, std::vector<unsigned int>& spirv,
        std::unordered_set<uint32_t>* live_locs,
        std::unordered_set<uint32_t>* live_builtins, spv::SpvBuildLogger*);

    // Apply the SPIRV-Tools EliminateDeadOutputStores and AggressiveDeadCodeElimination passes to generated SPIR-V using
    // |live_locs|. Put result in |spirv|.
    void SpirvToolsEliminateDeadOutputStores(spv_target_env target_env, std::vector<unsigned int>& spirv,
        std::unordered_set<uint32_t>* live_locs,
        std::unordered_set<uint32_t>* live_builtins, spv::SpvBuildLogger*);

    // Apply the SPIRV-Tools optimizer to strip debug info from SPIR-V.  This is implicitly done by
    // SpirvToolsTransform if spvOptions->stripDebugInfo is set, but can be called separately if
    // optimization is disabled.
    void SpirvToolsStripDebugInfo(const glslang::TIntermediate& intermediate,
        std::vector<unsigned int>& spirv, spv::SpvBuildLogger*);

#endif

} // end namespace glslang

#endif // GLSLANG_SPV_TOOLS_H

#include "glslang/Include/intermediate.h"

#include <string>
#include <vector>
#ifndef GLSLANG_SPIRV_LOGGER_H
#define GLSLANG_SPIRV_LOGGER_H

#include <string>
#include <vector>

namespace spv {

    // A class for holding all SPIR-V build status messages, including
    // missing/TBD functionalities, warnings, and errors.
    class SpvBuildLogger {
    public:
        SpvBuildLogger() {}

        // Registers a TBD functionality.
        void tbdFunctionality(const std::string& f);
        // Registers a missing functionality.
        void missingFunctionality(const std::string& f);

        // Logs a warning.
        void warning(const std::string& w) { warnings.push_back(w); }
        // Logs an error.
        void error(const std::string& e) { errors.push_back(e); }

        // Returns all messages accumulated in the order of:
        // TBD functionalities, missing functionalities, warnings, errors.
        std::string getAllMessages() const;

    private:
        SpvBuildLogger(const SpvBuildLogger&);

        std::vector<std::string> tbdFeatures;
        std::vector<std::string> missingFeatures;
        std::vector<std::string> warnings;
        std::vector<std::string> errors;
    };

} // end spv namespace

#endif // GLSLANG_SPIRV_LOGGER_H

namespace glslang {

    void GetSpirvVersion(std::string&);
    int GetSpirvGeneratorVersion();
    void GlslangToSpv(const glslang::TIntermediate& intermediate, std::vector<unsigned int>& spirv,
        SpvOptions* options = nullptr);
    void GlslangToSpv(const glslang::TIntermediate& intermediate, std::vector<unsigned int>& spirv,
        spv::SpvBuildLogger* logger, SpvOptions* options = nullptr);
    bool OutputSpvBin(const std::vector<unsigned int>& spirv, const char* baseName);
    bool OutputSpvHex(const std::vector<unsigned int>& spirv, const char* baseName, const char* varName);

}

#ifndef SPIRVREMAPPER_H
#define SPIRVREMAPPER_H

#include <string>
#include <vector>
#include <cstdlib>
#include <exception>

namespace spv {

    class spirvbin_base_t
    {
    public:
        enum Options {
            NONE = 0,
            STRIP = (1 << 0),
            MAP_TYPES = (1 << 1),
            MAP_NAMES = (1 << 2),
            MAP_FUNCS = (1 << 3),
            DCE_FUNCS = (1 << 4),
            DCE_VARS = (1 << 5),
            DCE_TYPES = (1 << 6),
            OPT_LOADSTORE = (1 << 7),
            OPT_FWD_LS = (1 << 8), // EXPERIMENTAL: PRODUCES INVALID SCHEMA-0 SPIRV
            MAP_ALL = (MAP_TYPES | MAP_NAMES | MAP_FUNCS),
            DCE_ALL = (DCE_FUNCS | DCE_VARS | DCE_TYPES),
            OPT_ALL = (OPT_LOADSTORE),

            ALL_BUT_STRIP = (MAP_ALL | DCE_ALL | OPT_ALL),
            DO_EVERYTHING = (STRIP | ALL_BUT_STRIP)
        };
    };

} // namespace SPV

#include <functional>
#include <cstdint>
#include <unordered_map>
#include <unordered_set>
#include <map>
#include <set>
#include <cassert>

#ifndef spirv_HPP
#define spirv_HPP

namespace spv {

    typedef unsigned int Id;

#define SPV_VERSION 0x10600
#define SPV_REVISION 1

    static const unsigned int MagicNumber = 0x07230203;
    static const unsigned int Version = 0x00010600;
    static const unsigned int Revision = 1;
    static const unsigned int OpCodeMask = 0xffff;
    static const unsigned int WordCountShift = 16;

    enum SourceLanguage {
        SourceLanguageUnknown = 0,
        SourceLanguageESSL = 1,
        SourceLanguageGLSL = 2,
        SourceLanguageOpenCL_C = 3,
        SourceLanguageOpenCL_CPP = 4,
        SourceLanguageHLSL = 5,
        SourceLanguageCPP_for_OpenCL = 6,
        SourceLanguageSYCL = 7,
        SourceLanguageMax = 0x7fffffff,
    };

    enum ExecutionModel {
        ExecutionModelVertex = 0,
        ExecutionModelTessellationControl = 1,
        ExecutionModelTessellationEvaluation = 2,
        ExecutionModelGeometry = 3,
        ExecutionModelFragment = 4,
        ExecutionModelGLCompute = 5,
        ExecutionModelKernel = 6,
        ExecutionModelTaskNV = 5267,
        ExecutionModelMeshNV = 5268,
        ExecutionModelRayGenerationKHR = 5313,
        ExecutionModelRayGenerationNV = 5313,
        ExecutionModelIntersectionKHR = 5314,
        ExecutionModelIntersectionNV = 5314,
        ExecutionModelAnyHitKHR = 5315,
        ExecutionModelAnyHitNV = 5315,
        ExecutionModelClosestHitKHR = 5316,
        ExecutionModelClosestHitNV = 5316,
        ExecutionModelMissKHR = 5317,
        ExecutionModelMissNV = 5317,
        ExecutionModelCallableKHR = 5318,
        ExecutionModelCallableNV = 5318,
        ExecutionModelTaskEXT = 5364,
        ExecutionModelMeshEXT = 5365,
        ExecutionModelMax = 0x7fffffff,
    };

    enum AddressingModel {
        AddressingModelLogical = 0,
        AddressingModelPhysical32 = 1,
        AddressingModelPhysical64 = 2,
        AddressingModelPhysicalStorageBuffer64 = 5348,
        AddressingModelPhysicalStorageBuffer64EXT = 5348,
        AddressingModelMax = 0x7fffffff,
    };

    enum MemoryModel {
        MemoryModelSimple = 0,
        MemoryModelGLSL450 = 1,
        MemoryModelOpenCL = 2,
        MemoryModelVulkan = 3,
        MemoryModelVulkanKHR = 3,
        MemoryModelMax = 0x7fffffff,
    };

    enum ExecutionMode {
        ExecutionModeInvocations = 0,
        ExecutionModeSpacingEqual = 1,
        ExecutionModeSpacingFractionalEven = 2,
        ExecutionModeSpacingFractionalOdd = 3,
        ExecutionModeVertexOrderCw = 4,
        ExecutionModeVertexOrderCcw = 5,
        ExecutionModePixelCenterInteger = 6,
        ExecutionModeOriginUpperLeft = 7,
        ExecutionModeOriginLowerLeft = 8,
        ExecutionModeEarlyFragmentTests = 9,
        ExecutionModePointMode = 10,
        ExecutionModeXfb = 11,
        ExecutionModeDepthReplacing = 12,
        ExecutionModeDepthGreater = 14,
        ExecutionModeDepthLess = 15,
        ExecutionModeDepthUnchanged = 16,
        ExecutionModeLocalSize = 17,
        ExecutionModeLocalSizeHint = 18,
        ExecutionModeInputPoints = 19,
        ExecutionModeInputLines = 20,
        ExecutionModeInputLinesAdjacency = 21,
        ExecutionModeTriangles = 22,
        ExecutionModeInputTrianglesAdjacency = 23,
        ExecutionModeQuads = 24,
        ExecutionModeIsolines = 25,
        ExecutionModeOutputVertices = 26,
        ExecutionModeOutputPoints = 27,
        ExecutionModeOutputLineStrip = 28,
        ExecutionModeOutputTriangleStrip = 29,
        ExecutionModeVecTypeHint = 30,
        ExecutionModeContractionOff = 31,
        ExecutionModeInitializer = 33,
        ExecutionModeFinalizer = 34,
        ExecutionModeSubgroupSize = 35,
        ExecutionModeSubgroupsPerWorkgroup = 36,
        ExecutionModeSubgroupsPerWorkgroupId = 37,
        ExecutionModeLocalSizeId = 38,
        ExecutionModeLocalSizeHintId = 39,
        ExecutionModeNonCoherentColorAttachmentReadEXT = 4169,
        ExecutionModeNonCoherentDepthAttachmentReadEXT = 4170,
        ExecutionModeNonCoherentStencilAttachmentReadEXT = 4171,
        ExecutionModeSubgroupUniformControlFlowKHR = 4421,
        ExecutionModePostDepthCoverage = 4446,
        ExecutionModeDenormPreserve = 4459,
        ExecutionModeDenormFlushToZero = 4460,
        ExecutionModeSignedZeroInfNanPreserve = 4461,
        ExecutionModeRoundingModeRTE = 4462,
        ExecutionModeRoundingModeRTZ = 4463,
        ExecutionModeEarlyAndLateFragmentTestsAMD = 5017,
        ExecutionModeStencilRefReplacingEXT = 5027,
        ExecutionModeStencilRefUnchangedFrontAMD = 5079,
        ExecutionModeStencilRefGreaterFrontAMD = 5080,
        ExecutionModeStencilRefLessFrontAMD = 5081,
        ExecutionModeStencilRefUnchangedBackAMD = 5082,
        ExecutionModeStencilRefGreaterBackAMD = 5083,
        ExecutionModeStencilRefLessBackAMD = 5084,
        ExecutionModeOutputLinesEXT = 5269,
        ExecutionModeOutputLinesNV = 5269,
        ExecutionModeOutputPrimitivesEXT = 5270,
        ExecutionModeOutputPrimitivesNV = 5270,
        ExecutionModeDerivativeGroupQuadsNV = 5289,
        ExecutionModeDerivativeGroupLinearNV = 5290,
        ExecutionModeOutputTrianglesEXT = 5298,
        ExecutionModeOutputTrianglesNV = 5298,
        ExecutionModePixelInterlockOrderedEXT = 5366,
        ExecutionModePixelInterlockUnorderedEXT = 5367,
        ExecutionModeSampleInterlockOrderedEXT = 5368,
        ExecutionModeSampleInterlockUnorderedEXT = 5369,
        ExecutionModeShadingRateInterlockOrderedEXT = 5370,
        ExecutionModeShadingRateInterlockUnorderedEXT = 5371,
        ExecutionModeSharedLocalMemorySizeINTEL = 5618,
        ExecutionModeRoundingModeRTPINTEL = 5620,
        ExecutionModeRoundingModeRTNINTEL = 5621,
        ExecutionModeFloatingPointModeALTINTEL = 5622,
        ExecutionModeFloatingPointModeIEEEINTEL = 5623,
        ExecutionModeMaxWorkgroupSizeINTEL = 5893,
        ExecutionModeMaxWorkDimINTEL = 5894,
        ExecutionModeNoGlobalOffsetINTEL = 5895,
        ExecutionModeNumSIMDWorkitemsINTEL = 5896,
        ExecutionModeSchedulerTargetFmaxMhzINTEL = 5903,
        ExecutionModeStreamingInterfaceINTEL = 6154,
        ExecutionModeNamedBarrierCountINTEL = 6417,
        ExecutionModeMax = 0x7fffffff,
    };

    enum StorageClass {
        StorageClassUniformConstant = 0,
        StorageClassInput = 1,
        StorageClassUniform = 2,
        StorageClassOutput = 3,
        StorageClassWorkgroup = 4,
        StorageClassCrossWorkgroup = 5,
        StorageClassPrivate = 6,
        StorageClassFunction = 7,
        StorageClassGeneric = 8,
        StorageClassPushConstant = 9,
        StorageClassAtomicCounter = 10,
        StorageClassImage = 11,
        StorageClassStorageBuffer = 12,
        StorageClassTileImageEXT = 4172,
        StorageClassCallableDataKHR = 5328,
        StorageClassCallableDataNV = 5328,
        StorageClassIncomingCallableDataKHR = 5329,
        StorageClassIncomingCallableDataNV = 5329,
        StorageClassRayPayloadKHR = 5338,
        StorageClassRayPayloadNV = 5338,
        StorageClassHitAttributeKHR = 5339,
        StorageClassHitAttributeNV = 5339,
        StorageClassIncomingRayPayloadKHR = 5342,
        StorageClassIncomingRayPayloadNV = 5342,
        StorageClassShaderRecordBufferKHR = 5343,
        StorageClassShaderRecordBufferNV = 5343,
        StorageClassPhysicalStorageBuffer = 5349,
        StorageClassPhysicalStorageBufferEXT = 5349,
        StorageClassHitObjectAttributeNV = 5385,
        StorageClassTaskPayloadWorkgroupEXT = 5402,
        StorageClassCodeSectionINTEL = 5605,
        StorageClassDeviceOnlyINTEL = 5936,
        StorageClassHostOnlyINTEL = 5937,
        StorageClassMax = 0x7fffffff,
    };

    enum Dim {
        Dim1D = 0,
        Dim2D = 1,
        Dim3D = 2,
        DimCube = 3,
        DimRect = 4,
        DimBuffer = 5,
        DimSubpassData = 6,
        DimTileImageDataEXT = 4173,
        DimMax = 0x7fffffff,
    };

    enum SamplerAddressingMode {
        SamplerAddressingModeNone = 0,
        SamplerAddressingModeClampToEdge = 1,
        SamplerAddressingModeClamp = 2,
        SamplerAddressingModeRepeat = 3,
        SamplerAddressingModeRepeatMirrored = 4,
        SamplerAddressingModeMax = 0x7fffffff,
    };

    enum SamplerFilterMode {
        SamplerFilterModeNearest = 0,
        SamplerFilterModeLinear = 1,
        SamplerFilterModeMax = 0x7fffffff,
    };

    enum ImageFormat {
        ImageFormatUnknown = 0,
        ImageFormatRgba32f = 1,
        ImageFormatRgba16f = 2,
        ImageFormatR32f = 3,
        ImageFormatRgba8 = 4,
        ImageFormatRgba8Snorm = 5,
        ImageFormatRg32f = 6,
        ImageFormatRg16f = 7,
        ImageFormatR11fG11fB10f = 8,
        ImageFormatR16f = 9,
        ImageFormatRgba16 = 10,
        ImageFormatRgb10A2 = 11,
        ImageFormatRg16 = 12,
        ImageFormatRg8 = 13,
        ImageFormatR16 = 14,
        ImageFormatR8 = 15,
        ImageFormatRgba16Snorm = 16,
        ImageFormatRg16Snorm = 17,
        ImageFormatRg8Snorm = 18,
        ImageFormatR16Snorm = 19,
        ImageFormatR8Snorm = 20,
        ImageFormatRgba32i = 21,
        ImageFormatRgba16i = 22,
        ImageFormatRgba8i = 23,
        ImageFormatR32i = 24,
        ImageFormatRg32i = 25,
        ImageFormatRg16i = 26,
        ImageFormatRg8i = 27,
        ImageFormatR16i = 28,
        ImageFormatR8i = 29,
        ImageFormatRgba32ui = 30,
        ImageFormatRgba16ui = 31,
        ImageFormatRgba8ui = 32,
        ImageFormatR32ui = 33,
        ImageFormatRgb10a2ui = 34,
        ImageFormatRg32ui = 35,
        ImageFormatRg16ui = 36,
        ImageFormatRg8ui = 37,
        ImageFormatR16ui = 38,
        ImageFormatR8ui = 39,
        ImageFormatR64ui = 40,
        ImageFormatR64i = 41,
        ImageFormatMax = 0x7fffffff,
    };

    enum ImageChannelOrder {
        ImageChannelOrderR = 0,
        ImageChannelOrderA = 1,
        ImageChannelOrderRG = 2,
        ImageChannelOrderRA = 3,
        ImageChannelOrderRGB = 4,
        ImageChannelOrderRGBA = 5,
        ImageChannelOrderBGRA = 6,
        ImageChannelOrderARGB = 7,
        ImageChannelOrderIntensity = 8,
        ImageChannelOrderLuminance = 9,
        ImageChannelOrderRx = 10,
        ImageChannelOrderRGx = 11,
        ImageChannelOrderRGBx = 12,
        ImageChannelOrderDepth = 13,
        ImageChannelOrderDepthStencil = 14,
        ImageChannelOrdersRGB = 15,
        ImageChannelOrdersRGBx = 16,
        ImageChannelOrdersRGBA = 17,
        ImageChannelOrdersBGRA = 18,
        ImageChannelOrderABGR = 19,
        ImageChannelOrderMax = 0x7fffffff,
    };

    enum ImageChannelDataType {
        ImageChannelDataTypeSnormInt8 = 0,
        ImageChannelDataTypeSnormInt16 = 1,
        ImageChannelDataTypeUnormInt8 = 2,
        ImageChannelDataTypeUnormInt16 = 3,
        ImageChannelDataTypeUnormShort565 = 4,
        ImageChannelDataTypeUnormShort555 = 5,
        ImageChannelDataTypeUnormInt101010 = 6,
        ImageChannelDataTypeSignedInt8 = 7,
        ImageChannelDataTypeSignedInt16 = 8,
        ImageChannelDataTypeSignedInt32 = 9,
        ImageChannelDataTypeUnsignedInt8 = 10,
        ImageChannelDataTypeUnsignedInt16 = 11,
        ImageChannelDataTypeUnsignedInt32 = 12,
        ImageChannelDataTypeHalfFloat = 13,
        ImageChannelDataTypeFloat = 14,
        ImageChannelDataTypeUnormInt24 = 15,
        ImageChannelDataTypeUnormInt101010_2 = 16,
        ImageChannelDataTypeMax = 0x7fffffff,
    };

    enum ImageOperandsShift {
        ImageOperandsBiasShift = 0,
        ImageOperandsLodShift = 1,
        ImageOperandsGradShift = 2,
        ImageOperandsConstOffsetShift = 3,
        ImageOperandsOffsetShift = 4,
        ImageOperandsConstOffsetsShift = 5,
        ImageOperandsSampleShift = 6,
        ImageOperandsMinLodShift = 7,
        ImageOperandsMakeTexelAvailableShift = 8,
        ImageOperandsMakeTexelAvailableKHRShift = 8,
        ImageOperandsMakeTexelVisibleShift = 9,
        ImageOperandsMakeTexelVisibleKHRShift = 9,
        ImageOperandsNonPrivateTexelShift = 10,
        ImageOperandsNonPrivateTexelKHRShift = 10,
        ImageOperandsVolatileTexelShift = 11,
        ImageOperandsVolatileTexelKHRShift = 11,
        ImageOperandsSignExtendShift = 12,
        ImageOperandsZeroExtendShift = 13,
        ImageOperandsNontemporalShift = 14,
        ImageOperandsOffsetsShift = 16,
        ImageOperandsMax = 0x7fffffff,
    };

    enum ImageOperandsMask {
        ImageOperandsMaskNone = 0,
        ImageOperandsBiasMask = 0x00000001,
        ImageOperandsLodMask = 0x00000002,
        ImageOperandsGradMask = 0x00000004,
        ImageOperandsConstOffsetMask = 0x00000008,
        ImageOperandsOffsetMask = 0x00000010,
        ImageOperandsConstOffsetsMask = 0x00000020,
        ImageOperandsSampleMask = 0x00000040,
        ImageOperandsMinLodMask = 0x00000080,
        ImageOperandsMakeTexelAvailableMask = 0x00000100,
        ImageOperandsMakeTexelAvailableKHRMask = 0x00000100,
        ImageOperandsMakeTexelVisibleMask = 0x00000200,
        ImageOperandsMakeTexelVisibleKHRMask = 0x00000200,
        ImageOperandsNonPrivateTexelMask = 0x00000400,
        ImageOperandsNonPrivateTexelKHRMask = 0x00000400,
        ImageOperandsVolatileTexelMask = 0x00000800,
        ImageOperandsVolatileTexelKHRMask = 0x00000800,
        ImageOperandsSignExtendMask = 0x00001000,
        ImageOperandsZeroExtendMask = 0x00002000,
        ImageOperandsNontemporalMask = 0x00004000,
        ImageOperandsOffsetsMask = 0x00010000,
    };

    enum FPFastMathModeShift {
        FPFastMathModeNotNaNShift = 0,
        FPFastMathModeNotInfShift = 1,
        FPFastMathModeNSZShift = 2,
        FPFastMathModeAllowRecipShift = 3,
        FPFastMathModeFastShift = 4,
        FPFastMathModeAllowContractFastINTELShift = 16,
        FPFastMathModeAllowReassocINTELShift = 17,
        FPFastMathModeMax = 0x7fffffff,
    };

    enum FPFastMathModeMask {
        FPFastMathModeMaskNone = 0,
        FPFastMathModeNotNaNMask = 0x00000001,
        FPFastMathModeNotInfMask = 0x00000002,
        FPFastMathModeNSZMask = 0x00000004,
        FPFastMathModeAllowRecipMask = 0x00000008,
        FPFastMathModeFastMask = 0x00000010,
        FPFastMathModeAllowContractFastINTELMask = 0x00010000,
        FPFastMathModeAllowReassocINTELMask = 0x00020000,
    };

    enum FPRoundingMode {
        FPRoundingModeRTE = 0,
        FPRoundingModeRTZ = 1,
        FPRoundingModeRTP = 2,
        FPRoundingModeRTN = 3,
        FPRoundingModeMax = 0x7fffffff,
    };

    enum LinkageType {
        LinkageTypeExport = 0,
        LinkageTypeImport = 1,
        LinkageTypeLinkOnceODR = 2,
        LinkageTypeMax = 0x7fffffff,
    };

    enum AccessQualifier {
        AccessQualifierReadOnly = 0,
        AccessQualifierWriteOnly = 1,
        AccessQualifierReadWrite = 2,
        AccessQualifierMax = 0x7fffffff,
    };

    enum FunctionParameterAttribute {
        FunctionParameterAttributeZext = 0,
        FunctionParameterAttributeSext = 1,
        FunctionParameterAttributeByVal = 2,
        FunctionParameterAttributeSret = 3,
        FunctionParameterAttributeNoAlias = 4,
        FunctionParameterAttributeNoCapture = 5,
        FunctionParameterAttributeNoWrite = 6,
        FunctionParameterAttributeNoReadWrite = 7,
        FunctionParameterAttributeRuntimeAlignedINTEL = 5940,
        FunctionParameterAttributeMax = 0x7fffffff,
    };

    enum Decoration {
        DecorationRelaxedPrecision = 0,
        DecorationSpecId = 1,
        DecorationBlock = 2,
        DecorationBufferBlock = 3,
        DecorationRowMajor = 4,
        DecorationColMajor = 5,
        DecorationArrayStride = 6,
        DecorationMatrixStride = 7,
        DecorationGLSLShared = 8,
        DecorationGLSLPacked = 9,
        DecorationCPacked = 10,
        DecorationBuiltIn = 11,
        DecorationNoPerspective = 13,
        DecorationFlat = 14,
        DecorationPatch = 15,
        DecorationCentroid = 16,
        DecorationSample = 17,
        DecorationInvariant = 18,
        DecorationRestrict = 19,
        DecorationAliased = 20,
        DecorationVolatile = 21,
        DecorationConstant = 22,
        DecorationCoherent = 23,
        DecorationNonWritable = 24,
        DecorationNonReadable = 25,
        DecorationUniform = 26,
        DecorationUniformId = 27,
        DecorationSaturatedConversion = 28,
        DecorationStream = 29,
        DecorationLocation = 30,
        DecorationComponent = 31,
        DecorationIndex = 32,
        DecorationBinding = 33,
        DecorationDescriptorSet = 34,
        DecorationOffset = 35,
        DecorationXfbBuffer = 36,
        DecorationXfbStride = 37,
        DecorationFuncParamAttr = 38,
        DecorationFPRoundingMode = 39,
        DecorationFPFastMathMode = 40,
        DecorationLinkageAttributes = 41,
        DecorationNoContraction = 42,
        DecorationInputAttachmentIndex = 43,
        DecorationAlignment = 44,
        DecorationMaxByteOffset = 45,
        DecorationAlignmentId = 46,
        DecorationMaxByteOffsetId = 47,
        DecorationNoSignedWrap = 4469,
        DecorationNoUnsignedWrap = 4470,
        DecorationWeightTextureQCOM = 4487,
        DecorationBlockMatchTextureQCOM = 4488,
        DecorationExplicitInterpAMD = 4999,
        DecorationOverrideCoverageNV = 5248,
        DecorationPassthroughNV = 5250,
        DecorationViewportRelativeNV = 5252,
        DecorationSecondaryViewportRelativeNV = 5256,
        DecorationPerPrimitiveEXT = 5271,
        DecorationPerPrimitiveNV = 5271,
        DecorationPerViewNV = 5272,
        DecorationPerTaskNV = 5273,
        DecorationPerVertexKHR = 5285,
        DecorationPerVertexNV = 5285,
        DecorationNonUniform = 5300,
        DecorationNonUniformEXT = 5300,
        DecorationRestrictPointer = 5355,
        DecorationRestrictPointerEXT = 5355,
        DecorationAliasedPointer = 5356,
        DecorationAliasedPointerEXT = 5356,
        DecorationHitObjectShaderRecordBufferNV = 5386,
        DecorationBindlessSamplerNV = 5398,
        DecorationBindlessImageNV = 5399,
        DecorationBoundSamplerNV = 5400,
        DecorationBoundImageNV = 5401,
        DecorationSIMTCallINTEL = 5599,
        DecorationReferencedIndirectlyINTEL = 5602,
        DecorationClobberINTEL = 5607,
        DecorationSideEffectsINTEL = 5608,
        DecorationVectorComputeVariableINTEL = 5624,
        DecorationFuncParamIOKindINTEL = 5625,
        DecorationVectorComputeFunctionINTEL = 5626,
        DecorationStackCallINTEL = 5627,
        DecorationGlobalVariableOffsetINTEL = 5628,
        DecorationCounterBuffer = 5634,
        DecorationHlslCounterBufferGOOGLE = 5634,
        DecorationHlslSemanticGOOGLE = 5635,
        DecorationUserSemantic = 5635,
        DecorationUserTypeGOOGLE = 5636,
        DecorationFunctionRoundingModeINTEL = 5822,
        DecorationFunctionDenormModeINTEL = 5823,
        DecorationRegisterINTEL = 5825,
        DecorationMemoryINTEL = 5826,
        DecorationNumbanksINTEL = 5827,
        DecorationBankwidthINTEL = 5828,
        DecorationMaxPrivateCopiesINTEL = 5829,
        DecorationSinglepumpINTEL = 5830,
        DecorationDoublepumpINTEL = 5831,
        DecorationMaxReplicatesINTEL = 5832,
        DecorationSimpleDualPortINTEL = 5833,
        DecorationMergeINTEL = 5834,
        DecorationBankBitsINTEL = 5835,
        DecorationForcePow2DepthINTEL = 5836,
        DecorationBurstCoalesceINTEL = 5899,
        DecorationCacheSizeINTEL = 5900,
        DecorationDontStaticallyCoalesceINTEL = 5901,
        DecorationPrefetchINTEL = 5902,
        DecorationStallEnableINTEL = 5905,
        DecorationFuseLoopsInFunctionINTEL = 5907,
        DecorationMathOpDSPModeINTEL = 5909,
        DecorationAliasScopeINTEL = 5914,
        DecorationNoAliasINTEL = 5915,
        DecorationInitiationIntervalINTEL = 5917,
        DecorationMaxConcurrencyINTEL = 5918,
        DecorationPipelineEnableINTEL = 5919,
        DecorationBufferLocationINTEL = 5921,
        DecorationIOPipeStorageINTEL = 5944,
        DecorationFunctionFloatingPointModeINTEL = 6080,
        DecorationSingleElementVectorINTEL = 6085,
        DecorationVectorComputeCallableFunctionINTEL = 6087,
        DecorationMediaBlockIOINTEL = 6140,
        DecorationConduitKernelArgumentINTEL = 6175,
        DecorationRegisterMapKernelArgumentINTEL = 6176,
        DecorationMMHostInterfaceAddressWidthINTEL = 6177,
        DecorationMMHostInterfaceDataWidthINTEL = 6178,
        DecorationMMHostInterfaceLatencyINTEL = 6179,
        DecorationMMHostInterfaceReadWriteModeINTEL = 6180,
        DecorationMMHostInterfaceMaxBurstINTEL = 6181,
        DecorationMMHostInterfaceWaitRequestINTEL = 6182,
        DecorationStableKernelArgumentINTEL = 6183,
        DecorationMax = 0x7fffffff,
    };

    enum BuiltIn {
        BuiltInPosition = 0,
        BuiltInPointSize = 1,
        BuiltInClipDistance = 3,
        BuiltInCullDistance = 4,
        BuiltInVertexId = 5,
        BuiltInInstanceId = 6,
        BuiltInPrimitiveId = 7,
        BuiltInInvocationId = 8,
        BuiltInLayer = 9,
        BuiltInViewportIndex = 10,
        BuiltInTessLevelOuter = 11,
        BuiltInTessLevelInner = 12,
        BuiltInTessCoord = 13,
        BuiltInPatchVertices = 14,
        BuiltInFragCoord = 15,
        BuiltInPointCoord = 16,
        BuiltInFrontFacing = 17,
        BuiltInSampleId = 18,
        BuiltInSamplePosition = 19,
        BuiltInSampleMask = 20,
        BuiltInFragDepth = 22,
        BuiltInHelperInvocation = 23,
        BuiltInNumWorkgroups = 24,
        BuiltInWorkgroupSize = 25,
        BuiltInWorkgroupId = 26,
        BuiltInLocalInvocationId = 27,
        BuiltInGlobalInvocationId = 28,
        BuiltInLocalInvocationIndex = 29,
        BuiltInWorkDim = 30,
        BuiltInGlobalSize = 31,
        BuiltInEnqueuedWorkgroupSize = 32,
        BuiltInGlobalOffset = 33,
        BuiltInGlobalLinearId = 34,
        BuiltInSubgroupSize = 36,
        BuiltInSubgroupMaxSize = 37,
        BuiltInNumSubgroups = 38,
        BuiltInNumEnqueuedSubgroups = 39,
        BuiltInSubgroupId = 40,
        BuiltInSubgroupLocalInvocationId = 41,
        BuiltInVertexIndex = 42,
        BuiltInInstanceIndex = 43,
        BuiltInCoreIDARM = 4160,
        BuiltInCoreCountARM = 4161,
        BuiltInCoreMaxIDARM = 4162,
        BuiltInWarpIDARM = 4163,
        BuiltInWarpMaxIDARM = 4164,
        BuiltInSubgroupEqMask = 4416,
        BuiltInSubgroupEqMaskKHR = 4416,
        BuiltInSubgroupGeMask = 4417,
        BuiltInSubgroupGeMaskKHR = 4417,
        BuiltInSubgroupGtMask = 4418,
        BuiltInSubgroupGtMaskKHR = 4418,
        BuiltInSubgroupLeMask = 4419,
        BuiltInSubgroupLeMaskKHR = 4419,
        BuiltInSubgroupLtMask = 4420,
        BuiltInSubgroupLtMaskKHR = 4420,
        BuiltInBaseVertex = 4424,
        BuiltInBaseInstance = 4425,
        BuiltInDrawIndex = 4426,
        BuiltInPrimitiveShadingRateKHR = 4432,
        BuiltInDeviceIndex = 4438,
        BuiltInViewIndex = 4440,
        BuiltInShadingRateKHR = 4444,
        BuiltInBaryCoordNoPerspAMD = 4992,
        BuiltInBaryCoordNoPerspCentroidAMD = 4993,
        BuiltInBaryCoordNoPerspSampleAMD = 4994,
        BuiltInBaryCoordSmoothAMD = 4995,
        BuiltInBaryCoordSmoothCentroidAMD = 4996,
        BuiltInBaryCoordSmoothSampleAMD = 4997,
        BuiltInBaryCoordPullModelAMD = 4998,
        BuiltInFragStencilRefEXT = 5014,
        BuiltInViewportMaskNV = 5253,
        BuiltInSecondaryPositionNV = 5257,
        BuiltInSecondaryViewportMaskNV = 5258,
        BuiltInPositionPerViewNV = 5261,
        BuiltInViewportMaskPerViewNV = 5262,
        BuiltInFullyCoveredEXT = 5264,
        BuiltInTaskCountNV = 5274,
        BuiltInPrimitiveCountNV = 5275,
        BuiltInPrimitiveIndicesNV = 5276,
        BuiltInClipDistancePerViewNV = 5277,
        BuiltInCullDistancePerViewNV = 5278,
        BuiltInLayerPerViewNV = 5279,
        BuiltInMeshViewCountNV = 5280,
        BuiltInMeshViewIndicesNV = 5281,
        BuiltInBaryCoordKHR = 5286,
        BuiltInBaryCoordNV = 5286,
        BuiltInBaryCoordNoPerspKHR = 5287,
        BuiltInBaryCoordNoPerspNV = 5287,
        BuiltInFragSizeEXT = 5292,
        BuiltInFragmentSizeNV = 5292,
        BuiltInFragInvocationCountEXT = 5293,
        BuiltInInvocationsPerPixelNV = 5293,
        BuiltInPrimitivePointIndicesEXT = 5294,
        BuiltInPrimitiveLineIndicesEXT = 5295,
        BuiltInPrimitiveTriangleIndicesEXT = 5296,
        BuiltInCullPrimitiveEXT = 5299,
        BuiltInLaunchIdKHR = 5319,
        BuiltInLaunchIdNV = 5319,
        BuiltInLaunchSizeKHR = 5320,
        BuiltInLaunchSizeNV = 5320,
        BuiltInWorldRayOriginKHR = 5321,
        BuiltInWorldRayOriginNV = 5321,
        BuiltInWorldRayDirectionKHR = 5322,
        BuiltInWorldRayDirectionNV = 5322,
        BuiltInObjectRayOriginKHR = 5323,
        BuiltInObjectRayOriginNV = 5323,
        BuiltInObjectRayDirectionKHR = 5324,
        BuiltInObjectRayDirectionNV = 5324,
        BuiltInRayTminKHR = 5325,
        BuiltInRayTminNV = 5325,
        BuiltInRayTmaxKHR = 5326,
        BuiltInRayTmaxNV = 5326,
        BuiltInInstanceCustomIndexKHR = 5327,
        BuiltInInstanceCustomIndexNV = 5327,
        BuiltInObjectToWorldKHR = 5330,
        BuiltInObjectToWorldNV = 5330,
        BuiltInWorldToObjectKHR = 5331,
        BuiltInWorldToObjectNV = 5331,
        BuiltInHitTNV = 5332,
        BuiltInHitKindKHR = 5333,
        BuiltInHitKindNV = 5333,
        BuiltInCurrentRayTimeNV = 5334,
        BuiltInHitTriangleVertexPositionsKHR = 5335,
        BuiltInHitMicroTriangleVertexPositionsNV = 5337,
        BuiltInHitMicroTriangleVertexBarycentricsNV = 5344,
        BuiltInHitKindFrontFacingMicroTriangleNV = 5405,
        BuiltInHitKindBackFacingMicroTriangleNV = 5406,
        BuiltInIncomingRayFlagsKHR = 5351,
        BuiltInIncomingRayFlagsNV = 5351,
        BuiltInRayGeometryIndexKHR = 5352,
        BuiltInWarpsPerSMNV = 5374,
        BuiltInSMCountNV = 5375,
        BuiltInWarpIDNV = 5376,
        BuiltInSMIDNV = 5377,
        BuiltInCullMaskKHR = 6021,
        BuiltInMax = 0x7fffffff,
    };

    enum SelectionControlShift {
        SelectionControlFlattenShift = 0,
        SelectionControlDontFlattenShift = 1,
        SelectionControlMax = 0x7fffffff,
    };

    enum SelectionControlMask {
        SelectionControlMaskNone = 0,
        SelectionControlFlattenMask = 0x00000001,
        SelectionControlDontFlattenMask = 0x00000002,
    };

    enum LoopControlShift {
        LoopControlUnrollShift = 0,
        LoopControlDontUnrollShift = 1,
        LoopControlDependencyInfiniteShift = 2,
        LoopControlDependencyLengthShift = 3,
        LoopControlMinIterationsShift = 4,
        LoopControlMaxIterationsShift = 5,
        LoopControlIterationMultipleShift = 6,
        LoopControlPeelCountShift = 7,
        LoopControlPartialCountShift = 8,
        LoopControlInitiationIntervalINTELShift = 16,
        LoopControlMaxConcurrencyINTELShift = 17,
        LoopControlDependencyArrayINTELShift = 18,
        LoopControlPipelineEnableINTELShift = 19,
        LoopControlLoopCoalesceINTELShift = 20,
        LoopControlMaxInterleavingINTELShift = 21,
        LoopControlSpeculatedIterationsINTELShift = 22,
        LoopControlNoFusionINTELShift = 23,
        LoopControlLoopCountINTELShift = 24,
        LoopControlMaxReinvocationDelayINTELShift = 25,
        LoopControlMax = 0x7fffffff,
    };

    enum LoopControlMask {
        LoopControlMaskNone = 0,
        LoopControlUnrollMask = 0x00000001,
        LoopControlDontUnrollMask = 0x00000002,
        LoopControlDependencyInfiniteMask = 0x00000004,
        LoopControlDependencyLengthMask = 0x00000008,
        LoopControlMinIterationsMask = 0x00000010,
        LoopControlMaxIterationsMask = 0x00000020,
        LoopControlIterationMultipleMask = 0x00000040,
        LoopControlPeelCountMask = 0x00000080,
        LoopControlPartialCountMask = 0x00000100,
        LoopControlInitiationIntervalINTELMask = 0x00010000,
        LoopControlMaxConcurrencyINTELMask = 0x00020000,
        LoopControlDependencyArrayINTELMask = 0x00040000,
        LoopControlPipelineEnableINTELMask = 0x00080000,
        LoopControlLoopCoalesceINTELMask = 0x00100000,
        LoopControlMaxInterleavingINTELMask = 0x00200000,
        LoopControlSpeculatedIterationsINTELMask = 0x00400000,
        LoopControlNoFusionINTELMask = 0x00800000,
        LoopControlLoopCountINTELMask = 0x01000000,
        LoopControlMaxReinvocationDelayINTELMask = 0x02000000,
    };

    enum FunctionControlShift {
        FunctionControlInlineShift = 0,
        FunctionControlDontInlineShift = 1,
        FunctionControlPureShift = 2,
        FunctionControlConstShift = 3,
        FunctionControlOptNoneINTELShift = 16,
        FunctionControlMax = 0x7fffffff,
    };

    enum FunctionControlMask {
        FunctionControlMaskNone = 0,
        FunctionControlInlineMask = 0x00000001,
        FunctionControlDontInlineMask = 0x00000002,
        FunctionControlPureMask = 0x00000004,
        FunctionControlConstMask = 0x00000008,
        FunctionControlOptNoneINTELMask = 0x00010000,
    };

    enum MemorySemanticsShift {
        MemorySemanticsAcquireShift = 1,
        MemorySemanticsReleaseShift = 2,
        MemorySemanticsAcquireReleaseShift = 3,
        MemorySemanticsSequentiallyConsistentShift = 4,
        MemorySemanticsUniformMemoryShift = 6,
        MemorySemanticsSubgroupMemoryShift = 7,
        MemorySemanticsWorkgroupMemoryShift = 8,
        MemorySemanticsCrossWorkgroupMemoryShift = 9,
        MemorySemanticsAtomicCounterMemoryShift = 10,
        MemorySemanticsImageMemoryShift = 11,
        MemorySemanticsOutputMemoryShift = 12,
        MemorySemanticsOutputMemoryKHRShift = 12,
        MemorySemanticsMakeAvailableShift = 13,
        MemorySemanticsMakeAvailableKHRShift = 13,
        MemorySemanticsMakeVisibleShift = 14,
        MemorySemanticsMakeVisibleKHRShift = 14,
        MemorySemanticsVolatileShift = 15,
        MemorySemanticsMax = 0x7fffffff,
    };

    enum MemorySemanticsMask {
        MemorySemanticsMaskNone = 0,
        MemorySemanticsAcquireMask = 0x00000002,
        MemorySemanticsReleaseMask = 0x00000004,
        MemorySemanticsAcquireReleaseMask = 0x00000008,
        MemorySemanticsSequentiallyConsistentMask = 0x00000010,
        MemorySemanticsUniformMemoryMask = 0x00000040,
        MemorySemanticsSubgroupMemoryMask = 0x00000080,
        MemorySemanticsWorkgroupMemoryMask = 0x00000100,
        MemorySemanticsCrossWorkgroupMemoryMask = 0x00000200,
        MemorySemanticsAtomicCounterMemoryMask = 0x00000400,
        MemorySemanticsImageMemoryMask = 0x00000800,
        MemorySemanticsOutputMemoryMask = 0x00001000,
        MemorySemanticsOutputMemoryKHRMask = 0x00001000,
        MemorySemanticsMakeAvailableMask = 0x00002000,
        MemorySemanticsMakeAvailableKHRMask = 0x00002000,
        MemorySemanticsMakeVisibleMask = 0x00004000,
        MemorySemanticsMakeVisibleKHRMask = 0x00004000,
        MemorySemanticsVolatileMask = 0x00008000,
    };

    enum MemoryAccessShift {
        MemoryAccessVolatileShift = 0,
        MemoryAccessAlignedShift = 1,
        MemoryAccessNontemporalShift = 2,
        MemoryAccessMakePointerAvailableShift = 3,
        MemoryAccessMakePointerAvailableKHRShift = 3,
        MemoryAccessMakePointerVisibleShift = 4,
        MemoryAccessMakePointerVisibleKHRShift = 4,
        MemoryAccessNonPrivatePointerShift = 5,
        MemoryAccessNonPrivatePointerKHRShift = 5,
        MemoryAccessAliasScopeINTELMaskShift = 16,
        MemoryAccessNoAliasINTELMaskShift = 17,
        MemoryAccessMax = 0x7fffffff,
    };

    enum MemoryAccessMask {
        MemoryAccessMaskNone = 0,
        MemoryAccessVolatileMask = 0x00000001,
        MemoryAccessAlignedMask = 0x00000002,
        MemoryAccessNontemporalMask = 0x00000004,
        MemoryAccessMakePointerAvailableMask = 0x00000008,
        MemoryAccessMakePointerAvailableKHRMask = 0x00000008,
        MemoryAccessMakePointerVisibleMask = 0x00000010,
        MemoryAccessMakePointerVisibleKHRMask = 0x00000010,
        MemoryAccessNonPrivatePointerMask = 0x00000020,
        MemoryAccessNonPrivatePointerKHRMask = 0x00000020,
        MemoryAccessAliasScopeINTELMaskMask = 0x00010000,
        MemoryAccessNoAliasINTELMaskMask = 0x00020000,
    };

    enum Scope {
        ScopeCrossDevice = 0,
        ScopeDevice = 1,
        ScopeWorkgroup = 2,
        ScopeSubgroup = 3,
        ScopeInvocation = 4,
        ScopeQueueFamily = 5,
        ScopeQueueFamilyKHR = 5,
        ScopeShaderCallKHR = 6,
        ScopeMax = 0x7fffffff,
    };

    enum GroupOperation {
        GroupOperationReduce = 0,
        GroupOperationInclusiveScan = 1,
        GroupOperationExclusiveScan = 2,
        GroupOperationClusteredReduce = 3,
        GroupOperationPartitionedReduceNV = 6,
        GroupOperationPartitionedInclusiveScanNV = 7,
        GroupOperationPartitionedExclusiveScanNV = 8,
        GroupOperationMax = 0x7fffffff,
    };

    enum KernelEnqueueFlags {
        KernelEnqueueFlagsNoWait = 0,
        KernelEnqueueFlagsWaitKernel = 1,
        KernelEnqueueFlagsWaitWorkGroup = 2,
        KernelEnqueueFlagsMax = 0x7fffffff,
    };

    enum KernelProfilingInfoShift {
        KernelProfilingInfoCmdExecTimeShift = 0,
        KernelProfilingInfoMax = 0x7fffffff,
    };

    enum KernelProfilingInfoMask {
        KernelProfilingInfoMaskNone = 0,
        KernelProfilingInfoCmdExecTimeMask = 0x00000001,
    };

    enum Capability {
        CapabilityMatrix = 0,
        CapabilityShader = 1,
        CapabilityGeometry = 2,
        CapabilityTessellation = 3,
        CapabilityAddresses = 4,
        CapabilityLinkage = 5,
        CapabilityKernel = 6,
        CapabilityVector16 = 7,
        CapabilityFloat16Buffer = 8,
        CapabilityFloat16 = 9,
        CapabilityFloat64 = 10,
        CapabilityInt64 = 11,
        CapabilityInt64Atomics = 12,
        CapabilityImageBasic = 13,
        CapabilityImageReadWrite = 14,
        CapabilityImageMipmap = 15,
        CapabilityPipes = 17,
        CapabilityGroups = 18,
        CapabilityDeviceEnqueue = 19,
        CapabilityLiteralSampler = 20,
        CapabilityAtomicStorage = 21,
        CapabilityInt16 = 22,
        CapabilityTessellationPointSize = 23,
        CapabilityGeometryPointSize = 24,
        CapabilityImageGatherExtended = 25,
        CapabilityStorageImageMultisample = 27,
        CapabilityUniformBufferArrayDynamicIndexing = 28,
        CapabilitySampledImageArrayDynamicIndexing = 29,
        CapabilityStorageBufferArrayDynamicIndexing = 30,
        CapabilityStorageImageArrayDynamicIndexing = 31,
        CapabilityClipDistance = 32,
        CapabilityCullDistance = 33,
        CapabilityImageCubeArray = 34,
        CapabilitySampleRateShading = 35,
        CapabilityImageRect = 36,
        CapabilitySampledRect = 37,
        CapabilityGenericPointer = 38,
        CapabilityInt8 = 39,
        CapabilityInputAttachment = 40,
        CapabilitySparseResidency = 41,
        CapabilityMinLod = 42,
        CapabilitySampled1D = 43,
        CapabilityImage1D = 44,
        CapabilitySampledCubeArray = 45,
        CapabilitySampledBuffer = 46,
        CapabilityImageBuffer = 47,
        CapabilityImageMSArray = 48,
        CapabilityStorageImageExtendedFormats = 49,
        CapabilityImageQuery = 50,
        CapabilityDerivativeControl = 51,
        CapabilityInterpolationFunction = 52,
        CapabilityTransformFeedback = 53,
        CapabilityGeometryStreams = 54,
        CapabilityStorageImageReadWithoutFormat = 55,
        CapabilityStorageImageWriteWithoutFormat = 56,
        CapabilityMultiViewport = 57,
        CapabilitySubgroupDispatch = 58,
        CapabilityNamedBarrier = 59,
        CapabilityPipeStorage = 60,
        CapabilityGroupNonUniform = 61,
        CapabilityGroupNonUniformVote = 62,
        CapabilityGroupNonUniformArithmetic = 63,
        CapabilityGroupNonUniformBallot = 64,
        CapabilityGroupNonUniformShuffle = 65,
        CapabilityGroupNonUniformShuffleRelative = 66,
        CapabilityGroupNonUniformClustered = 67,
        CapabilityGroupNonUniformQuad = 68,
        CapabilityShaderLayer = 69,
        CapabilityShaderViewportIndex = 70,
        CapabilityUniformDecoration = 71,
        CapabilityCoreBuiltinsARM = 4165,
        CapabilityTileImageColorReadAccessEXT = 4166,
        CapabilityTileImageDepthReadAccessEXT = 4167,
        CapabilityTileImageStencilReadAccessEXT = 4168,
        CapabilityFragmentShadingRateKHR = 4422,
        CapabilitySubgroupBallotKHR = 4423,
        CapabilityDrawParameters = 4427,
        CapabilityWorkgroupMemoryExplicitLayoutKHR = 4428,
        CapabilityWorkgroupMemoryExplicitLayout8BitAccessKHR = 4429,
        CapabilityWorkgroupMemoryExplicitLayout16BitAccessKHR = 4430,
        CapabilitySubgroupVoteKHR = 4431,
        CapabilityStorageBuffer16BitAccess = 4433,
        CapabilityStorageUniformBufferBlock16 = 4433,
        CapabilityStorageUniform16 = 4434,
        CapabilityUniformAndStorageBuffer16BitAccess = 4434,
        CapabilityStoragePushConstant16 = 4435,
        CapabilityStorageInputOutput16 = 4436,
        CapabilityDeviceGroup = 4437,
        CapabilityMultiView = 4439,
        CapabilityVariablePointersStorageBuffer = 4441,
        CapabilityVariablePointers = 4442,
        CapabilityAtomicStorageOps = 4445,
        CapabilitySampleMaskPostDepthCoverage = 4447,
        CapabilityStorageBuffer8BitAccess = 4448,
        CapabilityUniformAndStorageBuffer8BitAccess = 4449,
        CapabilityStoragePushConstant8 = 4450,
        CapabilityDenormPreserve = 4464,
        CapabilityDenormFlushToZero = 4465,
        CapabilitySignedZeroInfNanPreserve = 4466,
        CapabilityRoundingModeRTE = 4467,
        CapabilityRoundingModeRTZ = 4468,
        CapabilityRayQueryProvisionalKHR = 4471,
        CapabilityRayQueryKHR = 4472,
        CapabilityRayTraversalPrimitiveCullingKHR = 4478,
        CapabilityRayTracingKHR = 4479,
        CapabilityTextureSampleWeightedQCOM = 4484,
        CapabilityTextureBoxFilterQCOM = 4485,
        CapabilityTextureBlockMatchQCOM = 4486,
        CapabilityFloat16ImageAMD = 5008,
        CapabilityImageGatherBiasLodAMD = 5009,
        CapabilityFragmentMaskAMD = 5010,
        CapabilityStencilExportEXT = 5013,
        CapabilityImageReadWriteLodAMD = 5015,
        CapabilityInt64ImageEXT = 5016,
        CapabilityShaderClockKHR = 5055,
        CapabilitySampleMaskOverrideCoverageNV = 5249,
        CapabilityGeometryShaderPassthroughNV = 5251,
        CapabilityShaderViewportIndexLayerEXT = 5254,
        CapabilityShaderViewportIndexLayerNV = 5254,
        CapabilityShaderViewportMaskNV = 5255,
        CapabilityShaderStereoViewNV = 5259,
        CapabilityPerViewAttributesNV = 5260,
        CapabilityFragmentFullyCoveredEXT = 5265,
        CapabilityMeshShadingNV = 5266,
        CapabilityImageFootprintNV = 5282,
        CapabilityMeshShadingEXT = 5283,
        CapabilityFragmentBarycentricKHR = 5284,
        CapabilityFragmentBarycentricNV = 5284,
        CapabilityComputeDerivativeGroupQuadsNV = 5288,
        CapabilityFragmentDensityEXT = 5291,
        CapabilityShadingRateNV = 5291,
        CapabilityGroupNonUniformPartitionedNV = 5297,
        CapabilityShaderNonUniform = 5301,
        CapabilityShaderNonUniformEXT = 5301,
        CapabilityRuntimeDescriptorArray = 5302,
        CapabilityRuntimeDescriptorArrayEXT = 5302,
        CapabilityInputAttachmentArrayDynamicIndexing = 5303,
        CapabilityInputAttachmentArrayDynamicIndexingEXT = 5303,
        CapabilityUniformTexelBufferArrayDynamicIndexing = 5304,
        CapabilityUniformTexelBufferArrayDynamicIndexingEXT = 5304,
        CapabilityStorageTexelBufferArrayDynamicIndexing = 5305,
        CapabilityStorageTexelBufferArrayDynamicIndexingEXT = 5305,
        CapabilityUniformBufferArrayNonUniformIndexing = 5306,
        CapabilityUniformBufferArrayNonUniformIndexingEXT = 5306,
        CapabilitySampledImageArrayNonUniformIndexing = 5307,
        CapabilitySampledImageArrayNonUniformIndexingEXT = 5307,
        CapabilityStorageBufferArrayNonUniformIndexing = 5308,
        CapabilityStorageBufferArrayNonUniformIndexingEXT = 5308,
        CapabilityStorageImageArrayNonUniformIndexing = 5309,
        CapabilityStorageImageArrayNonUniformIndexingEXT = 5309,
        CapabilityInputAttachmentArrayNonUniformIndexing = 5310,
        CapabilityInputAttachmentArrayNonUniformIndexingEXT = 5310,
        CapabilityUniformTexelBufferArrayNonUniformIndexing = 5311,
        CapabilityUniformTexelBufferArrayNonUniformIndexingEXT = 5311,
        CapabilityStorageTexelBufferArrayNonUniformIndexing = 5312,
        CapabilityStorageTexelBufferArrayNonUniformIndexingEXT = 5312,
        CapabilityRayTracingPositionFetchKHR = 5336,
        CapabilityRayTracingNV = 5340,
        CapabilityRayTracingMotionBlurNV = 5341,
        CapabilityVulkanMemoryModel = 5345,
        CapabilityVulkanMemoryModelKHR = 5345,
        CapabilityVulkanMemoryModelDeviceScope = 5346,
        CapabilityVulkanMemoryModelDeviceScopeKHR = 5346,
        CapabilityPhysicalStorageBufferAddresses = 5347,
        CapabilityPhysicalStorageBufferAddressesEXT = 5347,
        CapabilityComputeDerivativeGroupLinearNV = 5350,
        CapabilityRayTracingProvisionalKHR = 5353,
        CapabilityCooperativeMatrixNV = 5357,
        CapabilityFragmentShaderSampleInterlockEXT = 5363,
        CapabilityFragmentShaderShadingRateInterlockEXT = 5372,
        CapabilityShaderSMBuiltinsNV = 5373,
        CapabilityFragmentShaderPixelInterlockEXT = 5378,
        CapabilityDemoteToHelperInvocation = 5379,
        CapabilityDemoteToHelperInvocationEXT = 5379,
        CapabilityDisplacementMicromapNV = 5380,
        CapabilityRayTracingDisplacementMicromapNV = 5409,
        CapabilityRayTracingOpacityMicromapEXT = 5381,
        CapabilityShaderInvocationReorderNV = 5383,
        CapabilityBindlessTextureNV = 5390,
        CapabilityRayQueryPositionFetchKHR = 5391,
        CapabilitySubgroupShuffleINTEL = 5568,
        CapabilitySubgroupBufferBlockIOINTEL = 5569,
        CapabilitySubgroupImageBlockIOINTEL = 5570,
        CapabilitySubgroupImageMediaBlockIOINTEL = 5579,
        CapabilityRoundToInfinityINTEL = 5582,
        CapabilityFloatingPointModeINTEL = 5583,
        CapabilityIntegerFunctions2INTEL = 5584,
        CapabilityFunctionPointersINTEL = 5603,
        CapabilityIndirectReferencesINTEL = 5604,
        CapabilityAsmINTEL = 5606,
        CapabilityAtomicFloat32MinMaxEXT = 5612,
        CapabilityAtomicFloat64MinMaxEXT = 5613,
        CapabilityAtomicFloat16MinMaxEXT = 5616,
        CapabilityVectorComputeINTEL = 5617,
        CapabilityVectorAnyINTEL = 5619,
        CapabilityExpectAssumeKHR = 5629,
        CapabilitySubgroupAvcMotionEstimationINTEL = 5696,
        CapabilitySubgroupAvcMotionEstimationIntraINTEL = 5697,
        CapabilitySubgroupAvcMotionEstimationChromaINTEL = 5698,
        CapabilityVariableLengthArrayINTEL = 5817,
        CapabilityFunctionFloatControlINTEL = 5821,
        CapabilityFPGAMemoryAttributesINTEL = 5824,
        CapabilityFPFastMathModeINTEL = 5837,
        CapabilityArbitraryPrecisionIntegersINTEL = 5844,
        CapabilityArbitraryPrecisionFloatingPointINTEL = 5845,
        CapabilityUnstructuredLoopControlsINTEL = 5886,
        CapabilityFPGALoopControlsINTEL = 5888,
        CapabilityKernelAttributesINTEL = 5892,
        CapabilityFPGAKernelAttributesINTEL = 5897,
        CapabilityFPGAMemoryAccessesINTEL = 5898,
        CapabilityFPGAClusterAttributesINTEL = 5904,
        CapabilityLoopFuseINTEL = 5906,
        CapabilityFPGADSPControlINTEL = 5908,
        CapabilityMemoryAccessAliasingINTEL = 5910,
        CapabilityFPGAInvocationPipeliningAttributesINTEL = 5916,
        CapabilityFPGABufferLocationINTEL = 5920,
        CapabilityArbitraryPrecisionFixedPointINTEL = 5922,
        CapabilityUSMStorageClassesINTEL = 5935,
        CapabilityRuntimeAlignedAttributeINTEL = 5939,
        CapabilityIOPipesINTEL = 5943,
        CapabilityBlockingPipesINTEL = 5945,
        CapabilityFPGARegINTEL = 5948,
        CapabilityDotProductInputAll = 6016,
        CapabilityDotProductInputAllKHR = 6016,
        CapabilityDotProductInput4x8Bit = 6017,
        CapabilityDotProductInput4x8BitKHR = 6017,
        CapabilityDotProductInput4x8BitPacked = 6018,
        CapabilityDotProductInput4x8BitPackedKHR = 6018,
        CapabilityDotProduct = 6019,
        CapabilityDotProductKHR = 6019,
        CapabilityRayCullMaskKHR = 6020,
        CapabilityCooperativeMatrixKHR = 6022,
        CapabilityBitInstructions = 6025,
        CapabilityGroupNonUniformRotateKHR = 6026,
        CapabilityAtomicFloat32AddEXT = 6033,
        CapabilityAtomicFloat64AddEXT = 6034,
        CapabilityLongConstantCompositeINTEL = 6089,
        CapabilityOptNoneINTEL = 6094,
        CapabilityAtomicFloat16AddEXT = 6095,
        CapabilityDebugInfoModuleINTEL = 6114,
        CapabilitySplitBarrierINTEL = 6141,
        CapabilityFPGAArgumentInterfacesINTEL = 6174,
        CapabilityGroupUniformArithmeticKHR = 6400,
        CapabilityMax = 0x7fffffff,
    };

    enum RayFlagsShift {
        RayFlagsOpaqueKHRShift = 0,
        RayFlagsNoOpaqueKHRShift = 1,
        RayFlagsTerminateOnFirstHitKHRShift = 2,
        RayFlagsSkipClosestHitShaderKHRShift = 3,
        RayFlagsCullBackFacingTrianglesKHRShift = 4,
        RayFlagsCullFrontFacingTrianglesKHRShift = 5,
        RayFlagsCullOpaqueKHRShift = 6,
        RayFlagsCullNoOpaqueKHRShift = 7,
        RayFlagsSkipTrianglesKHRShift = 8,
        RayFlagsSkipAABBsKHRShift = 9,
        RayFlagsForceOpacityMicromap2StateEXTShift = 10,
        RayFlagsMax = 0x7fffffff,
    };

    enum RayFlagsMask {
        RayFlagsMaskNone = 0,
        RayFlagsOpaqueKHRMask = 0x00000001,
        RayFlagsNoOpaqueKHRMask = 0x00000002,
        RayFlagsTerminateOnFirstHitKHRMask = 0x00000004,
        RayFlagsSkipClosestHitShaderKHRMask = 0x00000008,
        RayFlagsCullBackFacingTrianglesKHRMask = 0x00000010,
        RayFlagsCullFrontFacingTrianglesKHRMask = 0x00000020,
        RayFlagsCullOpaqueKHRMask = 0x00000040,
        RayFlagsCullNoOpaqueKHRMask = 0x00000080,
        RayFlagsSkipTrianglesKHRMask = 0x00000100,
        RayFlagsSkipAABBsKHRMask = 0x00000200,
        RayFlagsForceOpacityMicromap2StateEXTMask = 0x00000400,
    };

    enum RayQueryIntersection {
        RayQueryIntersectionRayQueryCandidateIntersectionKHR = 0,
        RayQueryIntersectionRayQueryCommittedIntersectionKHR = 1,
        RayQueryIntersectionMax = 0x7fffffff,
    };

    enum RayQueryCommittedIntersectionType {
        RayQueryCommittedIntersectionTypeRayQueryCommittedIntersectionNoneKHR = 0,
        RayQueryCommittedIntersectionTypeRayQueryCommittedIntersectionTriangleKHR = 1,
        RayQueryCommittedIntersectionTypeRayQueryCommittedIntersectionGeneratedKHR = 2,
        RayQueryCommittedIntersectionTypeMax = 0x7fffffff,
    };

    enum RayQueryCandidateIntersectionType {
        RayQueryCandidateIntersectionTypeRayQueryCandidateIntersectionTriangleKHR = 0,
        RayQueryCandidateIntersectionTypeRayQueryCandidateIntersectionAABBKHR = 1,
        RayQueryCandidateIntersectionTypeMax = 0x7fffffff,
    };

    enum FragmentShadingRateShift {
        FragmentShadingRateVertical2PixelsShift = 0,
        FragmentShadingRateVertical4PixelsShift = 1,
        FragmentShadingRateHorizontal2PixelsShift = 2,
        FragmentShadingRateHorizontal4PixelsShift = 3,
        FragmentShadingRateMax = 0x7fffffff,
    };

    enum FragmentShadingRateMask {
        FragmentShadingRateMaskNone = 0,
        FragmentShadingRateVertical2PixelsMask = 0x00000001,
        FragmentShadingRateVertical4PixelsMask = 0x00000002,
        FragmentShadingRateHorizontal2PixelsMask = 0x00000004,
        FragmentShadingRateHorizontal4PixelsMask = 0x00000008,
    };

    enum FPDenormMode {
        FPDenormModePreserve = 0,
        FPDenormModeFlushToZero = 1,
        FPDenormModeMax = 0x7fffffff,
    };

    enum FPOperationMode {
        FPOperationModeIEEE = 0,
        FPOperationModeALT = 1,
        FPOperationModeMax = 0x7fffffff,
    };

    enum QuantizationModes {
        QuantizationModesTRN = 0,
        QuantizationModesTRN_ZERO = 1,
        QuantizationModesRND = 2,
        QuantizationModesRND_ZERO = 3,
        QuantizationModesRND_INF = 4,
        QuantizationModesRND_MIN_INF = 5,
        QuantizationModesRND_CONV = 6,
        QuantizationModesRND_CONV_ODD = 7,
        QuantizationModesMax = 0x7fffffff,
    };

    enum OverflowModes {
        OverflowModesWRAP = 0,
        OverflowModesSAT = 1,
        OverflowModesSAT_ZERO = 2,
        OverflowModesSAT_SYM = 3,
        OverflowModesMax = 0x7fffffff,
    };

    enum PackedVectorFormat {
        PackedVectorFormatPackedVectorFormat4x8Bit = 0,
        PackedVectorFormatPackedVectorFormat4x8BitKHR = 0,
        PackedVectorFormatMax = 0x7fffffff,
    };

    enum CooperativeMatrixOperandsShift {
        CooperativeMatrixOperandsMatrixASignedComponentsShift = 0,
        CooperativeMatrixOperandsMatrixBSignedComponentsShift = 1,
        CooperativeMatrixOperandsMatrixCSignedComponentsShift = 2,
        CooperativeMatrixOperandsMatrixResultSignedComponentsShift = 3,
        CooperativeMatrixOperandsSaturatingAccumulationShift = 4,
        CooperativeMatrixOperandsMax = 0x7fffffff,
    };

    enum CooperativeMatrixOperandsMask {
        CooperativeMatrixOperandsMaskNone = 0,
        CooperativeMatrixOperandsMatrixASignedComponentsMask = 0x00000001,
        CooperativeMatrixOperandsMatrixBSignedComponentsMask = 0x00000002,
        CooperativeMatrixOperandsMatrixCSignedComponentsMask = 0x00000004,
        CooperativeMatrixOperandsMatrixResultSignedComponentsMask = 0x00000008,
        CooperativeMatrixOperandsSaturatingAccumulationMask = 0x00000010,
    };

    enum CooperativeMatrixLayout {
        CooperativeMatrixLayoutCooperativeMatrixRowMajorKHR = 0,
        CooperativeMatrixLayoutCooperativeMatrixColumnMajorKHR = 1,
        CooperativeMatrixLayoutMax = 0x7fffffff,
    };

    enum CooperativeMatrixUse {
        CooperativeMatrixUseMatrixAKHR = 0,
        CooperativeMatrixUseMatrixBKHR = 1,
        CooperativeMatrixUseMatrixAccumulatorKHR = 2,
        CooperativeMatrixUseMax = 0x7fffffff,
    };

    enum Op {
        OpNop = 0,
        OpUndef = 1,
        OpSourceContinued = 2,
        OpSource = 3,
        OpSourceExtension = 4,
        OpName = 5,
        OpMemberName = 6,
        OpString = 7,
        OpLine = 8,
        OpExtension = 10,
        OpExtInstImport = 11,
        OpExtInst = 12,
        OpMemoryModel = 14,
        OpEntryPoint = 15,
        OpExecutionMode = 16,
        OpCapability = 17,
        OpTypeVoid = 19,
        OpTypeBool = 20,
        OpTypeInt = 21,
        OpTypeFloat = 22,
        OpTypeVector = 23,
        OpTypeMatrix = 24,
        OpTypeImage = 25,
        OpTypeSampler = 26,
        OpTypeSampledImage = 27,
        OpTypeArray = 28,
        OpTypeRuntimeArray = 29,
        OpTypeStruct = 30,
        OpTypeOpaque = 31,
        OpTypePointer = 32,
        OpTypeFunction = 33,
        OpTypeEvent = 34,
        OpTypeDeviceEvent = 35,
        OpTypeReserveId = 36,
        OpTypeQueue = 37,
        OpTypePipe = 38,
        OpTypeForwardPointer = 39,
        OpConstantTrue = 41,
        OpConstantFalse = 42,
        OpConstant = 43,
        OpConstantComposite = 44,
        OpConstantSampler = 45,
        OpConstantNull = 46,
        OpSpecConstantTrue = 48,
        OpSpecConstantFalse = 49,
        OpSpecConstant = 50,
        OpSpecConstantComposite = 51,
        OpSpecConstantOp = 52,
        OpFunction = 54,
        OpFunctionParameter = 55,
        OpFunctionEnd = 56,
        OpFunctionCall = 57,
        OpVariable = 59,
        OpImageTexelPointer = 60,
        OpLoad = 61,
        OpStore = 62,
        OpCopyMemory = 63,
        OpCopyMemorySized = 64,
        OpAccessChain = 65,
        OpInBoundsAccessChain = 66,
        OpPtrAccessChain = 67,
        OpArrayLength = 68,
        OpGenericPtrMemSemantics = 69,
        OpInBoundsPtrAccessChain = 70,
        OpDecorate = 71,
        OpMemberDecorate = 72,
        OpDecorationGroup = 73,
        OpGroupDecorate = 74,
        OpGroupMemberDecorate = 75,
        OpVectorExtractDynamic = 77,
        OpVectorInsertDynamic = 78,
        OpVectorShuffle = 79,
        OpCompositeConstruct = 80,
        OpCompositeExtract = 81,
        OpCompositeInsert = 82,
        OpCopyObject = 83,
        OpTranspose = 84,
        OpSampledImage = 86,
        OpImageSampleImplicitLod = 87,
        OpImageSampleExplicitLod = 88,
        OpImageSampleDrefImplicitLod = 89,
        OpImageSampleDrefExplicitLod = 90,
        OpImageSampleProjImplicitLod = 91,
        OpImageSampleProjExplicitLod = 92,
        OpImageSampleProjDrefImplicitLod = 93,
        OpImageSampleProjDrefExplicitLod = 94,
        OpImageFetch = 95,
        OpImageGather = 96,
        OpImageDrefGather = 97,
        OpImageRead = 98,
        OpImageWrite = 99,
        OpImage = 100,
        OpImageQueryFormat = 101,
        OpImageQueryOrder = 102,
        OpImageQuerySizeLod = 103,
        OpImageQuerySize = 104,
        OpImageQueryLod = 105,
        OpImageQueryLevels = 106,
        OpImageQuerySamples = 107,
        OpConvertFToU = 109,
        OpConvertFToS = 110,
        OpConvertSToF = 111,
        OpConvertUToF = 112,
        OpUConvert = 113,
        OpSConvert = 114,
        OpFConvert = 115,
        OpQuantizeToF16 = 116,
        OpConvertPtrToU = 117,
        OpSatConvertSToU = 118,
        OpSatConvertUToS = 119,
        OpConvertUToPtr = 120,
        OpPtrCastToGeneric = 121,
        OpGenericCastToPtr = 122,
        OpGenericCastToPtrExplicit = 123,
        OpBitcast = 124,
        OpSNegate = 126,
        OpFNegate = 127,
        OpIAdd = 128,
        OpFAdd = 129,
        OpISub = 130,
        OpFSub = 131,
        OpIMul = 132,
        OpFMul = 133,
        OpUDiv = 134,
        OpSDiv = 135,
        OpFDiv = 136,
        OpUMod = 137,
        OpSRem = 138,
        OpSMod = 139,
        OpFRem = 140,
        OpFMod = 141,
        OpVectorTimesScalar = 142,
        OpMatrixTimesScalar = 143,
        OpVectorTimesMatrix = 144,
        OpMatrixTimesVector = 145,
        OpMatrixTimesMatrix = 146,
        OpOuterProduct = 147,
        OpDot = 148,
        OpIAddCarry = 149,
        OpISubBorrow = 150,
        OpUMulExtended = 151,
        OpSMulExtended = 152,
        OpAny = 154,
        OpAll = 155,
        OpIsNan = 156,
        OpIsInf = 157,
        OpIsFinite = 158,
        OpIsNormal = 159,
        OpSignBitSet = 160,
        OpLessOrGreater = 161,
        OpOrdered = 162,
        OpUnordered = 163,
        OpLogicalEqual = 164,
        OpLogicalNotEqual = 165,
        OpLogicalOr = 166,
        OpLogicalAnd = 167,
        OpLogicalNot = 168,
        OpSelect = 169,
        OpIEqual = 170,
        OpINotEqual = 171,
        OpUGreaterThan = 172,
        OpSGreaterThan = 173,
        OpUGreaterThanEqual = 174,
        OpSGreaterThanEqual = 175,
        OpULessThan = 176,
        OpSLessThan = 177,
        OpULessThanEqual = 178,
        OpSLessThanEqual = 179,
        OpFOrdEqual = 180,
        OpFUnordEqual = 181,
        OpFOrdNotEqual = 182,
        OpFUnordNotEqual = 183,
        OpFOrdLessThan = 184,
        OpFUnordLessThan = 185,
        OpFOrdGreaterThan = 186,
        OpFUnordGreaterThan = 187,
        OpFOrdLessThanEqual = 188,
        OpFUnordLessThanEqual = 189,
        OpFOrdGreaterThanEqual = 190,
        OpFUnordGreaterThanEqual = 191,
        OpShiftRightLogical = 194,
        OpShiftRightArithmetic = 195,
        OpShiftLeftLogical = 196,
        OpBitwiseOr = 197,
        OpBitwiseXor = 198,
        OpBitwiseAnd = 199,
        OpNot = 200,
        OpBitFieldInsert = 201,
        OpBitFieldSExtract = 202,
        OpBitFieldUExtract = 203,
        OpBitReverse = 204,
        OpBitCount = 205,
        OpDPdx = 207,
        OpDPdy = 208,
        OpFwidth = 209,
        OpDPdxFine = 210,
        OpDPdyFine = 211,
        OpFwidthFine = 212,
        OpDPdxCoarse = 213,
        OpDPdyCoarse = 214,
        OpFwidthCoarse = 215,
        OpEmitVertex = 218,
        OpEndPrimitive = 219,
        OpEmitStreamVertex = 220,
        OpEndStreamPrimitive = 221,
        OpControlBarrier = 224,
        OpMemoryBarrier = 225,
        OpAtomicLoad = 227,
        OpAtomicStore = 228,
        OpAtomicExchange = 229,
        OpAtomicCompareExchange = 230,
        OpAtomicCompareExchangeWeak = 231,
        OpAtomicIIncrement = 232,
        OpAtomicIDecrement = 233,
        OpAtomicIAdd = 234,
        OpAtomicISub = 235,
        OpAtomicSMin = 236,
        OpAtomicUMin = 237,
        OpAtomicSMax = 238,
        OpAtomicUMax = 239,
        OpAtomicAnd = 240,
        OpAtomicOr = 241,
        OpAtomicXor = 242,
        OpPhi = 245,
        OpLoopMerge = 246,
        OpSelectionMerge = 247,
        OpLabel = 248,
        OpBranch = 249,
        OpBranchConditional = 250,
        OpSwitch = 251,
        OpKill = 252,
        OpReturn = 253,
        OpReturnValue = 254,
        OpUnreachable = 255,
        OpLifetimeStart = 256,
        OpLifetimeStop = 257,
        OpGroupAsyncCopy = 259,
        OpGroupWaitEvents = 260,
        OpGroupAll = 261,
        OpGroupAny = 262,
        OpGroupBroadcast = 263,
        OpGroupIAdd = 264,
        OpGroupFAdd = 265,
        OpGroupFMin = 266,
        OpGroupUMin = 267,
        OpGroupSMin = 268,
        OpGroupFMax = 269,
        OpGroupUMax = 270,
        OpGroupSMax = 271,
        OpReadPipe = 274,
        OpWritePipe = 275,
        OpReservedReadPipe = 276,
        OpReservedWritePipe = 277,
        OpReserveReadPipePackets = 278,
        OpReserveWritePipePackets = 279,
        OpCommitReadPipe = 280,
        OpCommitWritePipe = 281,
        OpIsValidReserveId = 282,
        OpGetNumPipePackets = 283,
        OpGetMaxPipePackets = 284,
        OpGroupReserveReadPipePackets = 285,
        OpGroupReserveWritePipePackets = 286,
        OpGroupCommitReadPipe = 287,
        OpGroupCommitWritePipe = 288,
        OpEnqueueMarker = 291,
        OpEnqueueKernel = 292,
        OpGetKernelNDrangeSubGroupCount = 293,
        OpGetKernelNDrangeMaxSubGroupSize = 294,
        OpGetKernelWorkGroupSize = 295,
        OpGetKernelPreferredWorkGroupSizeMultiple = 296,
        OpRetainEvent = 297,
        OpReleaseEvent = 298,
        OpCreateUserEvent = 299,
        OpIsValidEvent = 300,
        OpSetUserEventStatus = 301,
        OpCaptureEventProfilingInfo = 302,
        OpGetDefaultQueue = 303,
        OpBuildNDRange = 304,
        OpImageSparseSampleImplicitLod = 305,
        OpImageSparseSampleExplicitLod = 306,
        OpImageSparseSampleDrefImplicitLod = 307,
        OpImageSparseSampleDrefExplicitLod = 308,
        OpImageSparseSampleProjImplicitLod = 309,
        OpImageSparseSampleProjExplicitLod = 310,
        OpImageSparseSampleProjDrefImplicitLod = 311,
        OpImageSparseSampleProjDrefExplicitLod = 312,
        OpImageSparseFetch = 313,
        OpImageSparseGather = 314,
        OpImageSparseDrefGather = 315,
        OpImageSparseTexelsResident = 316,
        OpNoLine = 317,
        OpAtomicFlagTestAndSet = 318,
        OpAtomicFlagClear = 319,
        OpImageSparseRead = 320,
        OpSizeOf = 321,
        OpTypePipeStorage = 322,
        OpConstantPipeStorage = 323,
        OpCreatePipeFromPipeStorage = 324,
        OpGetKernelLocalSizeForSubgroupCount = 325,
        OpGetKernelMaxNumSubgroups = 326,
        OpTypeNamedBarrier = 327,
        OpNamedBarrierInitialize = 328,
        OpMemoryNamedBarrier = 329,
        OpModuleProcessed = 330,
        OpExecutionModeId = 331,
        OpDecorateId = 332,
        OpGroupNonUniformElect = 333,
        OpGroupNonUniformAll = 334,
        OpGroupNonUniformAny = 335,
        OpGroupNonUniformAllEqual = 336,
        OpGroupNonUniformBroadcast = 337,
        OpGroupNonUniformBroadcastFirst = 338,
        OpGroupNonUniformBallot = 339,
        OpGroupNonUniformInverseBallot = 340,
        OpGroupNonUniformBallotBitExtract = 341,
        OpGroupNonUniformBallotBitCount = 342,
        OpGroupNonUniformBallotFindLSB = 343,
        OpGroupNonUniformBallotFindMSB = 344,
        OpGroupNonUniformShuffle = 345,
        OpGroupNonUniformShuffleXor = 346,
        OpGroupNonUniformShuffleUp = 347,
        OpGroupNonUniformShuffleDown = 348,
        OpGroupNonUniformIAdd = 349,
        OpGroupNonUniformFAdd = 350,
        OpGroupNonUniformIMul = 351,
        OpGroupNonUniformFMul = 352,
        OpGroupNonUniformSMin = 353,
        OpGroupNonUniformUMin = 354,
        OpGroupNonUniformFMin = 355,
        OpGroupNonUniformSMax = 356,
        OpGroupNonUniformUMax = 357,
        OpGroupNonUniformFMax = 358,
        OpGroupNonUniformBitwiseAnd = 359,
        OpGroupNonUniformBitwiseOr = 360,
        OpGroupNonUniformBitwiseXor = 361,
        OpGroupNonUniformLogicalAnd = 362,
        OpGroupNonUniformLogicalOr = 363,
        OpGroupNonUniformLogicalXor = 364,
        OpGroupNonUniformQuadBroadcast = 365,
        OpGroupNonUniformQuadSwap = 366,
        OpCopyLogical = 400,
        OpPtrEqual = 401,
        OpPtrNotEqual = 402,
        OpPtrDiff = 403,
        OpColorAttachmentReadEXT = 4160,
        OpDepthAttachmentReadEXT = 4161,
        OpStencilAttachmentReadEXT = 4162,
        OpTerminateInvocation = 4416,
        OpSubgroupBallotKHR = 4421,
        OpSubgroupFirstInvocationKHR = 4422,
        OpSubgroupAllKHR = 4428,
        OpSubgroupAnyKHR = 4429,
        OpSubgroupAllEqualKHR = 4430,
        OpGroupNonUniformRotateKHR = 4431,
        OpSubgroupReadInvocationKHR = 4432,
        OpTraceRayKHR = 4445,
        OpExecuteCallableKHR = 4446,
        OpConvertUToAccelerationStructureKHR = 4447,
        OpIgnoreIntersectionKHR = 4448,
        OpTerminateRayKHR = 4449,
        OpSDot = 4450,
        OpSDotKHR = 4450,
        OpUDot = 4451,
        OpUDotKHR = 4451,
        OpSUDot = 4452,
        OpSUDotKHR = 4452,
        OpSDotAccSat = 4453,
        OpSDotAccSatKHR = 4453,
        OpUDotAccSat = 4454,
        OpUDotAccSatKHR = 4454,
        OpSUDotAccSat = 4455,
        OpSUDotAccSatKHR = 4455,
        OpTypeCooperativeMatrixKHR = 4456,
        OpCooperativeMatrixLoadKHR = 4457,
        OpCooperativeMatrixStoreKHR = 4458,
        OpCooperativeMatrixMulAddKHR = 4459,
        OpCooperativeMatrixLengthKHR = 4460,
        OpTypeRayQueryKHR = 4472,
        OpRayQueryInitializeKHR = 4473,
        OpRayQueryTerminateKHR = 4474,
        OpRayQueryGenerateIntersectionKHR = 4475,
        OpRayQueryConfirmIntersectionKHR = 4476,
        OpRayQueryProceedKHR = 4477,
        OpRayQueryGetIntersectionTypeKHR = 4479,
        OpImageSampleWeightedQCOM = 4480,
        OpImageBoxFilterQCOM = 4481,
        OpImageBlockMatchSSDQCOM = 4482,
        OpImageBlockMatchSADQCOM = 4483,
        OpGroupIAddNonUniformAMD = 5000,
        OpGroupFAddNonUniformAMD = 5001,
        OpGroupFMinNonUniformAMD = 5002,
        OpGroupUMinNonUniformAMD = 5003,
        OpGroupSMinNonUniformAMD = 5004,
        OpGroupFMaxNonUniformAMD = 5005,
        OpGroupUMaxNonUniformAMD = 5006,
        OpGroupSMaxNonUniformAMD = 5007,
        OpFragmentMaskFetchAMD = 5011,
        OpFragmentFetchAMD = 5012,
        OpReadClockKHR = 5056,
        OpHitObjectRecordHitMotionNV = 5249,
        OpHitObjectRecordHitWithIndexMotionNV = 5250,
        OpHitObjectRecordMissMotionNV = 5251,
        OpHitObjectGetWorldToObjectNV = 5252,
        OpHitObjectGetObjectToWorldNV = 5253,
        OpHitObjectGetObjectRayDirectionNV = 5254,
        OpHitObjectGetObjectRayOriginNV = 5255,
        OpHitObjectTraceRayMotionNV = 5256,
        OpHitObjectGetShaderRecordBufferHandleNV = 5257,
        OpHitObjectGetShaderBindingTableRecordIndexNV = 5258,
        OpHitObjectRecordEmptyNV = 5259,
        OpHitObjectTraceRayNV = 5260,
        OpHitObjectRecordHitNV = 5261,
        OpHitObjectRecordHitWithIndexNV = 5262,
        OpHitObjectRecordMissNV = 5263,
        OpHitObjectExecuteShaderNV = 5264,
        OpHitObjectGetCurrentTimeNV = 5265,
        OpHitObjectGetAttributesNV = 5266,
        OpHitObjectGetHitKindNV = 5267,
        OpHitObjectGetPrimitiveIndexNV = 5268,
        OpHitObjectGetGeometryIndexNV = 5269,
        OpHitObjectGetInstanceIdNV = 5270,
        OpHitObjectGetInstanceCustomIndexNV = 5271,
        OpHitObjectGetWorldRayDirectionNV = 5272,
        OpHitObjectGetWorldRayOriginNV = 5273,
        OpHitObjectGetRayTMaxNV = 5274,
        OpHitObjectGetRayTMinNV = 5275,
        OpHitObjectIsEmptyNV = 5276,
        OpHitObjectIsHitNV = 5277,
        OpHitObjectIsMissNV = 5278,
        OpReorderThreadWithHitObjectNV = 5279,
        OpReorderThreadWithHintNV = 5280,
        OpTypeHitObjectNV = 5281,
        OpImageSampleFootprintNV = 5283,
        OpEmitMeshTasksEXT = 5294,
        OpSetMeshOutputsEXT = 5295,
        OpGroupNonUniformPartitionNV = 5296,
        OpWritePackedPrimitiveIndices4x8NV = 5299,
        OpFetchMicroTriangleVertexPositionNV = 5300,
        OpFetchMicroTriangleVertexBarycentricNV = 5301,
        OpReportIntersectionKHR = 5334,
        OpReportIntersectionNV = 5334,
        OpIgnoreIntersectionNV = 5335,
        OpTerminateRayNV = 5336,
        OpTraceNV = 5337,
        OpTraceMotionNV = 5338,
        OpTraceRayMotionNV = 5339,
        OpRayQueryGetIntersectionTriangleVertexPositionsKHR = 5340,
        OpTypeAccelerationStructureKHR = 5341,
        OpTypeAccelerationStructureNV = 5341,
        OpExecuteCallableNV = 5344,
        OpTypeCooperativeMatrixNV = 5358,
        OpCooperativeMatrixLoadNV = 5359,
        OpCooperativeMatrixStoreNV = 5360,
        OpCooperativeMatrixMulAddNV = 5361,
        OpCooperativeMatrixLengthNV = 5362,
        OpBeginInvocationInterlockEXT = 5364,
        OpEndInvocationInterlockEXT = 5365,
        OpDemoteToHelperInvocation = 5380,
        OpDemoteToHelperInvocationEXT = 5380,
        OpIsHelperInvocationEXT = 5381,
        OpConvertUToImageNV = 5391,
        OpConvertUToSamplerNV = 5392,
        OpConvertImageToUNV = 5393,
        OpConvertSamplerToUNV = 5394,
        OpConvertUToSampledImageNV = 5395,
        OpConvertSampledImageToUNV = 5396,
        OpSamplerImageAddressingModeNV = 5397,
        OpSubgroupShuffleINTEL = 5571,
        OpSubgroupShuffleDownINTEL = 5572,
        OpSubgroupShuffleUpINTEL = 5573,
        OpSubgroupShuffleXorINTEL = 5574,
        OpSubgroupBlockReadINTEL = 5575,
        OpSubgroupBlockWriteINTEL = 5576,
        OpSubgroupImageBlockReadINTEL = 5577,
        OpSubgroupImageBlockWriteINTEL = 5578,
        OpSubgroupImageMediaBlockReadINTEL = 5580,
        OpSubgroupImageMediaBlockWriteINTEL = 5581,
        OpUCountLeadingZerosINTEL = 5585,
        OpUCountTrailingZerosINTEL = 5586,
        OpAbsISubINTEL = 5587,
        OpAbsUSubINTEL = 5588,
        OpIAddSatINTEL = 5589,
        OpUAddSatINTEL = 5590,
        OpIAverageINTEL = 5591,
        OpUAverageINTEL = 5592,
        OpIAverageRoundedINTEL = 5593,
        OpUAverageRoundedINTEL = 5594,
        OpISubSatINTEL = 5595,
        OpUSubSatINTEL = 5596,
        OpIMul32x16INTEL = 5597,
        OpUMul32x16INTEL = 5598,
        OpConstantFunctionPointerINTEL = 5600,
        OpFunctionPointerCallINTEL = 5601,
        OpAsmTargetINTEL = 5609,
        OpAsmINTEL = 5610,
        OpAsmCallINTEL = 5611,
        OpAtomicFMinEXT = 5614,
        OpAtomicFMaxEXT = 5615,
        OpAssumeTrueKHR = 5630,
        OpExpectKHR = 5631,
        OpDecorateString = 5632,
        OpDecorateStringGOOGLE = 5632,
        OpMemberDecorateString = 5633,
        OpMemberDecorateStringGOOGLE = 5633,
        OpVmeImageINTEL = 5699,
        OpTypeVmeImageINTEL = 5700,
        OpTypeAvcImePayloadINTEL = 5701,
        OpTypeAvcRefPayloadINTEL = 5702,
        OpTypeAvcSicPayloadINTEL = 5703,
        OpTypeAvcMcePayloadINTEL = 5704,
        OpTypeAvcMceResultINTEL = 5705,
        OpTypeAvcImeResultINTEL = 5706,
        OpTypeAvcImeResultSingleReferenceStreamoutINTEL = 5707,
        OpTypeAvcImeResultDualReferenceStreamoutINTEL = 5708,
        OpTypeAvcImeSingleReferenceStreaminINTEL = 5709,
        OpTypeAvcImeDualReferenceStreaminINTEL = 5710,
        OpTypeAvcRefResultINTEL = 5711,
        OpTypeAvcSicResultINTEL = 5712,
        OpSubgroupAvcMceGetDefaultInterBaseMultiReferencePenaltyINTEL = 5713,
        OpSubgroupAvcMceSetInterBaseMultiReferencePenaltyINTEL = 5714,
        OpSubgroupAvcMceGetDefaultInterShapePenaltyINTEL = 5715,
        OpSubgroupAvcMceSetInterShapePenaltyINTEL = 5716,
        OpSubgroupAvcMceGetDefaultInterDirectionPenaltyINTEL = 5717,
        OpSubgroupAvcMceSetInterDirectionPenaltyINTEL = 5718,
        OpSubgroupAvcMceGetDefaultIntraLumaShapePenaltyINTEL = 5719,
        OpSubgroupAvcMceGetDefaultInterMotionVectorCostTableINTEL = 5720,
        OpSubgroupAvcMceGetDefaultHighPenaltyCostTableINTEL = 5721,
        OpSubgroupAvcMceGetDefaultMediumPenaltyCostTableINTEL = 5722,
        OpSubgroupAvcMceGetDefaultLowPenaltyCostTableINTEL = 5723,
        OpSubgroupAvcMceSetMotionVectorCostFunctionINTEL = 5724,
        OpSubgroupAvcMceGetDefaultIntraLumaModePenaltyINTEL = 5725,
        OpSubgroupAvcMceGetDefaultNonDcLumaIntraPenaltyINTEL = 5726,
        OpSubgroupAvcMceGetDefaultIntraChromaModeBasePenaltyINTEL = 5727,
        OpSubgroupAvcMceSetAcOnlyHaarINTEL = 5728,
        OpSubgroupAvcMceSetSourceInterlacedFieldPolarityINTEL = 5729,
        OpSubgroupAvcMceSetSingleReferenceInterlacedFieldPolarityINTEL = 5730,
        OpSubgroupAvcMceSetDualReferenceInterlacedFieldPolaritiesINTEL = 5731,
        OpSubgroupAvcMceConvertToImePayloadINTEL = 5732,
        OpSubgroupAvcMceConvertToImeResultINTEL = 5733,
        OpSubgroupAvcMceConvertToRefPayloadINTEL = 5734,
        OpSubgroupAvcMceConvertToRefResultINTEL = 5735,
        OpSubgroupAvcMceConvertToSicPayloadINTEL = 5736,
        OpSubgroupAvcMceConvertToSicResultINTEL = 5737,
        OpSubgroupAvcMceGetMotionVectorsINTEL = 5738,
        OpSubgroupAvcMceGetInterDistortionsINTEL = 5739,
        OpSubgroupAvcMceGetBestInterDistortionsINTEL = 5740,
        OpSubgroupAvcMceGetInterMajorShapeINTEL = 5741,
        OpSubgroupAvcMceGetInterMinorShapeINTEL = 5742,
        OpSubgroupAvcMceGetInterDirectionsINTEL = 5743,
        OpSubgroupAvcMceGetInterMotionVectorCountINTEL = 5744,
        OpSubgroupAvcMceGetInterReferenceIdsINTEL = 5745,
        OpSubgroupAvcMceGetInterReferenceInterlacedFieldPolaritiesINTEL = 5746,
        OpSubgroupAvcImeInitializeINTEL = 5747,
        OpSubgroupAvcImeSetSingleReferenceINTEL = 5748,
        OpSubgroupAvcImeSetDualReferenceINTEL = 5749,
        OpSubgroupAvcImeRefWindowSizeINTEL = 5750,
        OpSubgroupAvcImeAdjustRefOffsetINTEL = 5751,
        OpSubgroupAvcImeConvertToMcePayloadINTEL = 5752,
        OpSubgroupAvcImeSetMaxMotionVectorCountINTEL = 5753,
        OpSubgroupAvcImeSetUnidirectionalMixDisableINTEL = 5754,
        OpSubgroupAvcImeSetEarlySearchTerminationThresholdINTEL = 5755,
        OpSubgroupAvcImeSetWeightedSadINTEL = 5756,
        OpSubgroupAvcImeEvaluateWithSingleReferenceINTEL = 5757,
        OpSubgroupAvcImeEvaluateWithDualReferenceINTEL = 5758,
        OpSubgroupAvcImeEvaluateWithSingleReferenceStreaminINTEL = 5759,
        OpSubgroupAvcImeEvaluateWithDualReferenceStreaminINTEL = 5760,
        OpSubgroupAvcImeEvaluateWithSingleReferenceStreamoutINTEL = 5761,
        OpSubgroupAvcImeEvaluateWithDualReferenceStreamoutINTEL = 5762,
        OpSubgroupAvcImeEvaluateWithSingleReferenceStreaminoutINTEL = 5763,
        OpSubgroupAvcImeEvaluateWithDualReferenceStreaminoutINTEL = 5764,
        OpSubgroupAvcImeConvertToMceResultINTEL = 5765,
        OpSubgroupAvcImeGetSingleReferenceStreaminINTEL = 5766,
        OpSubgroupAvcImeGetDualReferenceStreaminINTEL = 5767,
        OpSubgroupAvcImeStripSingleReferenceStreamoutINTEL = 5768,
        OpSubgroupAvcImeStripDualReferenceStreamoutINTEL = 5769,
        OpSubgroupAvcImeGetStreamoutSingleReferenceMajorShapeMotionVectorsINTEL = 5770,
        OpSubgroupAvcImeGetStreamoutSingleReferenceMajorShapeDistortionsINTEL = 5771,
        OpSubgroupAvcImeGetStreamoutSingleReferenceMajorShapeReferenceIdsINTEL = 5772,
        OpSubgroupAvcImeGetStreamoutDualReferenceMajorShapeMotionVectorsINTEL = 5773,
        OpSubgroupAvcImeGetStreamoutDualReferenceMajorShapeDistortionsINTEL = 5774,
        OpSubgroupAvcImeGetStreamoutDualReferenceMajorShapeReferenceIdsINTEL = 5775,
        OpSubgroupAvcImeGetBorderReachedINTEL = 5776,
        OpSubgroupAvcImeGetTruncatedSearchIndicationINTEL = 5777,
        OpSubgroupAvcImeGetUnidirectionalEarlySearchTerminationINTEL = 5778,
        OpSubgroupAvcImeGetWeightingPatternMinimumMotionVectorINTEL = 5779,
        OpSubgroupAvcImeGetWeightingPatternMinimumDistortionINTEL = 5780,
        OpSubgroupAvcFmeInitializeINTEL = 5781,
        OpSubgroupAvcBmeInitializeINTEL = 5782,
        OpSubgroupAvcRefConvertToMcePayloadINTEL = 5783,
        OpSubgroupAvcRefSetBidirectionalMixDisableINTEL = 5784,
        OpSubgroupAvcRefSetBilinearFilterEnableINTEL = 5785,
        OpSubgroupAvcRefEvaluateWithSingleReferenceINTEL = 5786,
        OpSubgroupAvcRefEvaluateWithDualReferenceINTEL = 5787,
        OpSubgroupAvcRefEvaluateWithMultiReferenceINTEL = 5788,
        OpSubgroupAvcRefEvaluateWithMultiReferenceInterlacedINTEL = 5789,
        OpSubgroupAvcRefConvertToMceResultINTEL = 5790,
        OpSubgroupAvcSicInitializeINTEL = 5791,
        OpSubgroupAvcSicConfigureSkcINTEL = 5792,
        OpSubgroupAvcSicConfigureIpeLumaINTEL = 5793,
        OpSubgroupAvcSicConfigureIpeLumaChromaINTEL = 5794,
        OpSubgroupAvcSicGetMotionVectorMaskINTEL = 5795,
        OpSubgroupAvcSicConvertToMcePayloadINTEL = 5796,
        OpSubgroupAvcSicSetIntraLumaShapePenaltyINTEL = 5797,
        OpSubgroupAvcSicSetIntraLumaModeCostFunctionINTEL = 5798,
        OpSubgroupAvcSicSetIntraChromaModeCostFunctionINTEL = 5799,
        OpSubgroupAvcSicSetBilinearFilterEnableINTEL = 5800,
        OpSubgroupAvcSicSetSkcForwardTransformEnableINTEL = 5801,
        OpSubgroupAvcSicSetBlockBasedRawSkipSadINTEL = 5802,
        OpSubgroupAvcSicEvaluateIpeINTEL = 5803,
        OpSubgroupAvcSicEvaluateWithSingleReferenceINTEL = 5804,
        OpSubgroupAvcSicEvaluateWithDualReferenceINTEL = 5805,
        OpSubgroupAvcSicEvaluateWithMultiReferenceINTEL = 5806,
        OpSubgroupAvcSicEvaluateWithMultiReferenceInterlacedINTEL = 5807,
        OpSubgroupAvcSicConvertToMceResultINTEL = 5808,
        OpSubgroupAvcSicGetIpeLumaShapeINTEL = 5809,
        OpSubgroupAvcSicGetBestIpeLumaDistortionINTEL = 5810,
        OpSubgroupAvcSicGetBestIpeChromaDistortionINTEL = 5811,
        OpSubgroupAvcSicGetPackedIpeLumaModesINTEL = 5812,
        OpSubgroupAvcSicGetIpeChromaModeINTEL = 5813,
        OpSubgroupAvcSicGetPackedSkcLumaCountThresholdINTEL = 5814,
        OpSubgroupAvcSicGetPackedSkcLumaSumThresholdINTEL = 5815,
        OpSubgroupAvcSicGetInterRawSadsINTEL = 5816,
        OpVariableLengthArrayINTEL = 5818,
        OpSaveMemoryINTEL = 5819,
        OpRestoreMemoryINTEL = 5820,
        OpArbitraryFloatSinCosPiINTEL = 5840,
        OpArbitraryFloatCastINTEL = 5841,
        OpArbitraryFloatCastFromIntINTEL = 5842,
        OpArbitraryFloatCastToIntINTEL = 5843,
        OpArbitraryFloatAddINTEL = 5846,
        OpArbitraryFloatSubINTEL = 5847,
        OpArbitraryFloatMulINTEL = 5848,
        OpArbitraryFloatDivINTEL = 5849,
        OpArbitraryFloatGTINTEL = 5850,
        OpArbitraryFloatGEINTEL = 5851,
        OpArbitraryFloatLTINTEL = 5852,
        OpArbitraryFloatLEINTEL = 5853,
        OpArbitraryFloatEQINTEL = 5854,
        OpArbitraryFloatRecipINTEL = 5855,
        OpArbitraryFloatRSqrtINTEL = 5856,
        OpArbitraryFloatCbrtINTEL = 5857,
        OpArbitraryFloatHypotINTEL = 5858,
        OpArbitraryFloatSqrtINTEL = 5859,
        OpArbitraryFloatLogINTEL = 5860,
        OpArbitraryFloatLog2INTEL = 5861,
        OpArbitraryFloatLog10INTEL = 5862,
        OpArbitraryFloatLog1pINTEL = 5863,
        OpArbitraryFloatExpINTEL = 5864,
        OpArbitraryFloatExp2INTEL = 5865,
        OpArbitraryFloatExp10INTEL = 5866,
        OpArbitraryFloatExpm1INTEL = 5867,
        OpArbitraryFloatSinINTEL = 5868,
        OpArbitraryFloatCosINTEL = 5869,
        OpArbitraryFloatSinCosINTEL = 5870,
        OpArbitraryFloatSinPiINTEL = 5871,
        OpArbitraryFloatCosPiINTEL = 5872,
        OpArbitraryFloatASinINTEL = 5873,
        OpArbitraryFloatASinPiINTEL = 5874,
        OpArbitraryFloatACosINTEL = 5875,
        OpArbitraryFloatACosPiINTEL = 5876,
        OpArbitraryFloatATanINTEL = 5877,
        OpArbitraryFloatATanPiINTEL = 5878,
        OpArbitraryFloatATan2INTEL = 5879,
        OpArbitraryFloatPowINTEL = 5880,
        OpArbitraryFloatPowRINTEL = 5881,
        OpArbitraryFloatPowNINTEL = 5882,
        OpLoopControlINTEL = 5887,
        OpAliasDomainDeclINTEL = 5911,
        OpAliasScopeDeclINTEL = 5912,
        OpAliasScopeListDeclINTEL = 5913,
        OpFixedSqrtINTEL = 5923,
        OpFixedRecipINTEL = 5924,
        OpFixedRsqrtINTEL = 5925,
        OpFixedSinINTEL = 5926,
        OpFixedCosINTEL = 5927,
        OpFixedSinCosINTEL = 5928,
        OpFixedSinPiINTEL = 5929,
        OpFixedCosPiINTEL = 5930,
        OpFixedSinCosPiINTEL = 5931,
        OpFixedLogINTEL = 5932,
        OpFixedExpINTEL = 5933,
        OpPtrCastToCrossWorkgroupINTEL = 5934,
        OpCrossWorkgroupCastToPtrINTEL = 5938,
        OpReadPipeBlockingINTEL = 5946,
        OpWritePipeBlockingINTEL = 5947,
        OpFPGARegINTEL = 5949,
        OpRayQueryGetRayTMinKHR = 6016,
        OpRayQueryGetRayFlagsKHR = 6017,
        OpRayQueryGetIntersectionTKHR = 6018,
        OpRayQueryGetIntersectionInstanceCustomIndexKHR = 6019,
        OpRayQueryGetIntersectionInstanceIdKHR = 6020,
        OpRayQueryGetIntersectionInstanceShaderBindingTableRecordOffsetKHR = 6021,
        OpRayQueryGetIntersectionGeometryIndexKHR = 6022,
        OpRayQueryGetIntersectionPrimitiveIndexKHR = 6023,
        OpRayQueryGetIntersectionBarycentricsKHR = 6024,
        OpRayQueryGetIntersectionFrontFaceKHR = 6025,
        OpRayQueryGetIntersectionCandidateAABBOpaqueKHR = 6026,
        OpRayQueryGetIntersectionObjectRayDirectionKHR = 6027,
        OpRayQueryGetIntersectionObjectRayOriginKHR = 6028,
        OpRayQueryGetWorldRayDirectionKHR = 6029,
        OpRayQueryGetWorldRayOriginKHR = 6030,
        OpRayQueryGetIntersectionObjectToWorldKHR = 6031,
        OpRayQueryGetIntersectionWorldToObjectKHR = 6032,
        OpAtomicFAddEXT = 6035,
        OpTypeBufferSurfaceINTEL = 6086,
        OpTypeStructContinuedINTEL = 6090,
        OpConstantCompositeContinuedINTEL = 6091,
        OpSpecConstantCompositeContinuedINTEL = 6092,
        OpControlBarrierArriveINTEL = 6142,
        OpControlBarrierWaitINTEL = 6143,
        OpGroupIMulKHR = 6401,
        OpGroupFMulKHR = 6402,
        OpGroupBitwiseAndKHR = 6403,
        OpGroupBitwiseOrKHR = 6404,
        OpGroupBitwiseXorKHR = 6405,
        OpGroupLogicalAndKHR = 6406,
        OpGroupLogicalOrKHR = 6407,
        OpGroupLogicalXorKHR = 6408,
        OpMax = 0x7fffffff,
    };

#ifdef SPV_ENABLE_UTILITY_CODE
#ifndef __cplusplus
#include <stdbool.h>
#endif
    inline void HasResultAndType(Op opcode, bool* hasResult, bool* hasResultType) {
        *hasResult = *hasResultType = false;
        switch (opcode) {
        default: /* unknown opcode */ break;
        case OpNop: *hasResult = false; *hasResultType = false; break;
        case OpUndef: *hasResult = true; *hasResultType = true; break;
        case OpSourceContinued: *hasResult = false; *hasResultType = false; break;
        case OpSource: *hasResult = false; *hasResultType = false; break;
        case OpSourceExtension: *hasResult = false; *hasResultType = false; break;
        case OpName: *hasResult = false; *hasResultType = false; break;
        case OpMemberName: *hasResult = false; *hasResultType = false; break;
        case OpString: *hasResult = true; *hasResultType = false; break;
        case OpLine: *hasResult = false; *hasResultType = false; break;
        case OpExtension: *hasResult = false; *hasResultType = false; break;
        case OpExtInstImport: *hasResult = true; *hasResultType = false; break;
        case OpExtInst: *hasResult = true; *hasResultType = true; break;
        case OpMemoryModel: *hasResult = false; *hasResultType = false; break;
        case OpEntryPoint: *hasResult = false; *hasResultType = false; break;
        case OpExecutionMode: *hasResult = false; *hasResultType = false; break;
        case OpCapability: *hasResult = false; *hasResultType = false; break;
        case OpTypeVoid: *hasResult = true; *hasResultType = false; break;
        case OpTypeBool: *hasResult = true; *hasResultType = false; break;
        case OpTypeInt: *hasResult = true; *hasResultType = false; break;
        case OpTypeFloat: *hasResult = true; *hasResultType = false; break;
        case OpTypeVector: *hasResult = true; *hasResultType = false; break;
        case OpTypeMatrix: *hasResult = true; *hasResultType = false; break;
        case OpTypeImage: *hasResult = true; *hasResultType = false; break;
        case OpTypeSampler: *hasResult = true; *hasResultType = false; break;
        case OpTypeSampledImage: *hasResult = true; *hasResultType = false; break;
        case OpTypeArray: *hasResult = true; *hasResultType = false; break;
        case OpTypeRuntimeArray: *hasResult = true; *hasResultType = false; break;
        case OpTypeStruct: *hasResult = true; *hasResultType = false; break;
        case OpTypeOpaque: *hasResult = true; *hasResultType = false; break;
        case OpTypePointer: *hasResult = true; *hasResultType = false; break;
        case OpTypeFunction: *hasResult = true; *hasResultType = false; break;
        case OpTypeEvent: *hasResult = true; *hasResultType = false; break;
        case OpTypeDeviceEvent: *hasResult = true; *hasResultType = false; break;
        case OpTypeReserveId: *hasResult = true; *hasResultType = false; break;
        case OpTypeQueue: *hasResult = true; *hasResultType = false; break;
        case OpTypePipe: *hasResult = true; *hasResultType = false; break;
        case OpTypeForwardPointer: *hasResult = false; *hasResultType = false; break;
        case OpConstantTrue: *hasResult = true; *hasResultType = true; break;
        case OpConstantFalse: *hasResult = true; *hasResultType = true; break;
        case OpConstant: *hasResult = true; *hasResultType = true; break;
        case OpConstantComposite: *hasResult = true; *hasResultType = true; break;
        case OpConstantSampler: *hasResult = true; *hasResultType = true; break;
        case OpConstantNull: *hasResult = true; *hasResultType = true; break;
        case OpSpecConstantTrue: *hasResult = true; *hasResultType = true; break;
        case OpSpecConstantFalse: *hasResult = true; *hasResultType = true; break;
        case OpSpecConstant: *hasResult = true; *hasResultType = true; break;
        case OpSpecConstantComposite: *hasResult = true; *hasResultType = true; break;
        case OpSpecConstantOp: *hasResult = true; *hasResultType = true; break;
        case OpFunction: *hasResult = true; *hasResultType = true; break;
        case OpFunctionParameter: *hasResult = true; *hasResultType = true; break;
        case OpFunctionEnd: *hasResult = false; *hasResultType = false; break;
        case OpFunctionCall: *hasResult = true; *hasResultType = true; break;
        case OpVariable: *hasResult = true; *hasResultType = true; break;
        case OpImageTexelPointer: *hasResult = true; *hasResultType = true; break;
        case OpLoad: *hasResult = true; *hasResultType = true; break;
        case OpStore: *hasResult = false; *hasResultType = false; break;
        case OpCopyMemory: *hasResult = false; *hasResultType = false; break;
        case OpCopyMemorySized: *hasResult = false; *hasResultType = false; break;
        case OpAccessChain: *hasResult = true; *hasResultType = true; break;
        case OpInBoundsAccessChain: *hasResult = true; *hasResultType = true; break;
        case OpPtrAccessChain: *hasResult = true; *hasResultType = true; break;
        case OpArrayLength: *hasResult = true; *hasResultType = true; break;
        case OpGenericPtrMemSemantics: *hasResult = true; *hasResultType = true; break;
        case OpInBoundsPtrAccessChain: *hasResult = true; *hasResultType = true; break;
        case OpDecorate: *hasResult = false; *hasResultType = false; break;
        case OpMemberDecorate: *hasResult = false; *hasResultType = false; break;
        case OpDecorationGroup: *hasResult = true; *hasResultType = false; break;
        case OpGroupDecorate: *hasResult = false; *hasResultType = false; break;
        case OpGroupMemberDecorate: *hasResult = false; *hasResultType = false; break;
        case OpVectorExtractDynamic: *hasResult = true; *hasResultType = true; break;
        case OpVectorInsertDynamic: *hasResult = true; *hasResultType = true; break;
        case OpVectorShuffle: *hasResult = true; *hasResultType = true; break;
        case OpCompositeConstruct: *hasResult = true; *hasResultType = true; break;
        case OpCompositeExtract: *hasResult = true; *hasResultType = true; break;
        case OpCompositeInsert: *hasResult = true; *hasResultType = true; break;
        case OpCopyObject: *hasResult = true; *hasResultType = true; break;
        case OpTranspose: *hasResult = true; *hasResultType = true; break;
        case OpSampledImage: *hasResult = true; *hasResultType = true; break;
        case OpImageSampleImplicitLod: *hasResult = true; *hasResultType = true; break;
        case OpImageSampleExplicitLod: *hasResult = true; *hasResultType = true; break;
        case OpImageSampleDrefImplicitLod: *hasResult = true; *hasResultType = true; break;
        case OpImageSampleDrefExplicitLod: *hasResult = true; *hasResultType = true; break;
        case OpImageSampleProjImplicitLod: *hasResult = true; *hasResultType = true; break;
        case OpImageSampleProjExplicitLod: *hasResult = true; *hasResultType = true; break;
        case OpImageSampleProjDrefImplicitLod: *hasResult = true; *hasResultType = true; break;
        case OpImageSampleProjDrefExplicitLod: *hasResult = true; *hasResultType = true; break;
        case OpImageFetch: *hasResult = true; *hasResultType = true; break;
        case OpImageGather: *hasResult = true; *hasResultType = true; break;
        case OpImageDrefGather: *hasResult = true; *hasResultType = true; break;
        case OpImageRead: *hasResult = true; *hasResultType = true; break;
        case OpImageWrite: *hasResult = false; *hasResultType = false; break;
        case OpImage: *hasResult = true; *hasResultType = true; break;
        case OpImageQueryFormat: *hasResult = true; *hasResultType = true; break;
        case OpImageQueryOrder: *hasResult = true; *hasResultType = true; break;
        case OpImageQuerySizeLod: *hasResult = true; *hasResultType = true; break;
        case OpImageQuerySize: *hasResult = true; *hasResultType = true; break;
        case OpImageQueryLod: *hasResult = true; *hasResultType = true; break;
        case OpImageQueryLevels: *hasResult = true; *hasResultType = true; break;
        case OpImageQuerySamples: *hasResult = true; *hasResultType = true; break;
        case OpConvertFToU: *hasResult = true; *hasResultType = true; break;
        case OpConvertFToS: *hasResult = true; *hasResultType = true; break;
        case OpConvertSToF: *hasResult = true; *hasResultType = true; break;
        case OpConvertUToF: *hasResult = true; *hasResultType = true; break;
        case OpUConvert: *hasResult = true; *hasResultType = true; break;
        case OpSConvert: *hasResult = true; *hasResultType = true; break;
        case OpFConvert: *hasResult = true; *hasResultType = true; break;
        case OpQuantizeToF16: *hasResult = true; *hasResultType = true; break;
        case OpConvertPtrToU: *hasResult = true; *hasResultType = true; break;
        case OpSatConvertSToU: *hasResult = true; *hasResultType = true; break;
        case OpSatConvertUToS: *hasResult = true; *hasResultType = true; break;
        case OpConvertUToPtr: *hasResult = true; *hasResultType = true; break;
        case OpPtrCastToGeneric: *hasResult = true; *hasResultType = true; break;
        case OpGenericCastToPtr: *hasResult = true; *hasResultType = true; break;
        case OpGenericCastToPtrExplicit: *hasResult = true; *hasResultType = true; break;
        case OpBitcast: *hasResult = true; *hasResultType = true; break;
        case OpSNegate: *hasResult = true; *hasResultType = true; break;
        case OpFNegate: *hasResult = true; *hasResultType = true; break;
        case OpIAdd: *hasResult = true; *hasResultType = true; break;
        case OpFAdd: *hasResult = true; *hasResultType = true; break;
        case OpISub: *hasResult = true; *hasResultType = true; break;
        case OpFSub: *hasResult = true; *hasResultType = true; break;
        case OpIMul: *hasResult = true; *hasResultType = true; break;
        case OpFMul: *hasResult = true; *hasResultType = true; break;
        case OpUDiv: *hasResult = true; *hasResultType = true; break;
        case OpSDiv: *hasResult = true; *hasResultType = true; break;
        case OpFDiv: *hasResult = true; *hasResultType = true; break;
        case OpUMod: *hasResult = true; *hasResultType = true; break;
        case OpSRem: *hasResult = true; *hasResultType = true; break;
        case OpSMod: *hasResult = true; *hasResultType = true; break;
        case OpFRem: *hasResult = true; *hasResultType = true; break;
        case OpFMod: *hasResult = true; *hasResultType = true; break;
        case OpVectorTimesScalar: *hasResult = true; *hasResultType = true; break;
        case OpMatrixTimesScalar: *hasResult = true; *hasResultType = true; break;
        case OpVectorTimesMatrix: *hasResult = true; *hasResultType = true; break;
        case OpMatrixTimesVector: *hasResult = true; *hasResultType = true; break;
        case OpMatrixTimesMatrix: *hasResult = true; *hasResultType = true; break;
        case OpOuterProduct: *hasResult = true; *hasResultType = true; break;
        case OpDot: *hasResult = true; *hasResultType = true; break;
        case OpIAddCarry: *hasResult = true; *hasResultType = true; break;
        case OpISubBorrow: *hasResult = true; *hasResultType = true; break;
        case OpUMulExtended: *hasResult = true; *hasResultType = true; break;
        case OpSMulExtended: *hasResult = true; *hasResultType = true; break;
        case OpAny: *hasResult = true; *hasResultType = true; break;
        case OpAll: *hasResult = true; *hasResultType = true; break;
        case OpIsNan: *hasResult = true; *hasResultType = true; break;
        case OpIsInf: *hasResult = true; *hasResultType = true; break;
        case OpIsFinite: *hasResult = true; *hasResultType = true; break;
        case OpIsNormal: *hasResult = true; *hasResultType = true; break;
        case OpSignBitSet: *hasResult = true; *hasResultType = true; break;
        case OpLessOrGreater: *hasResult = true; *hasResultType = true; break;
        case OpOrdered: *hasResult = true; *hasResultType = true; break;
        case OpUnordered: *hasResult = true; *hasResultType = true; break;
        case OpLogicalEqual: *hasResult = true; *hasResultType = true; break;
        case OpLogicalNotEqual: *hasResult = true; *hasResultType = true; break;
        case OpLogicalOr: *hasResult = true; *hasResultType = true; break;
        case OpLogicalAnd: *hasResult = true; *hasResultType = true; break;
        case OpLogicalNot: *hasResult = true; *hasResultType = true; break;
        case OpSelect: *hasResult = true; *hasResultType = true; break;
        case OpIEqual: *hasResult = true; *hasResultType = true; break;
        case OpINotEqual: *hasResult = true; *hasResultType = true; break;
        case OpUGreaterThan: *hasResult = true; *hasResultType = true; break;
        case OpSGreaterThan: *hasResult = true; *hasResultType = true; break;
        case OpUGreaterThanEqual: *hasResult = true; *hasResultType = true; break;
        case OpSGreaterThanEqual: *hasResult = true; *hasResultType = true; break;
        case OpULessThan: *hasResult = true; *hasResultType = true; break;
        case OpSLessThan: *hasResult = true; *hasResultType = true; break;
        case OpULessThanEqual: *hasResult = true; *hasResultType = true; break;
        case OpSLessThanEqual: *hasResult = true; *hasResultType = true; break;
        case OpFOrdEqual: *hasResult = true; *hasResultType = true; break;
        case OpFUnordEqual: *hasResult = true; *hasResultType = true; break;
        case OpFOrdNotEqual: *hasResult = true; *hasResultType = true; break;
        case OpFUnordNotEqual: *hasResult = true; *hasResultType = true; break;
        case OpFOrdLessThan: *hasResult = true; *hasResultType = true; break;
        case OpFUnordLessThan: *hasResult = true; *hasResultType = true; break;
        case OpFOrdGreaterThan: *hasResult = true; *hasResultType = true; break;
        case OpFUnordGreaterThan: *hasResult = true; *hasResultType = true; break;
        case OpFOrdLessThanEqual: *hasResult = true; *hasResultType = true; break;
        case OpFUnordLessThanEqual: *hasResult = true; *hasResultType = true; break;
        case OpFOrdGreaterThanEqual: *hasResult = true; *hasResultType = true; break;
        case OpFUnordGreaterThanEqual: *hasResult = true; *hasResultType = true; break;
        case OpShiftRightLogical: *hasResult = true; *hasResultType = true; break;
        case OpShiftRightArithmetic: *hasResult = true; *hasResultType = true; break;
        case OpShiftLeftLogical: *hasResult = true; *hasResultType = true; break;
        case OpBitwiseOr: *hasResult = true; *hasResultType = true; break;
        case OpBitwiseXor: *hasResult = true; *hasResultType = true; break;
        case OpBitwiseAnd: *hasResult = true; *hasResultType = true; break;
        case OpNot: *hasResult = true; *hasResultType = true; break;
        case OpBitFieldInsert: *hasResult = true; *hasResultType = true; break;
        case OpBitFieldSExtract: *hasResult = true; *hasResultType = true; break;
        case OpBitFieldUExtract: *hasResult = true; *hasResultType = true; break;
        case OpBitReverse: *hasResult = true; *hasResultType = true; break;
        case OpBitCount: *hasResult = true; *hasResultType = true; break;
        case OpDPdx: *hasResult = true; *hasResultType = true; break;
        case OpDPdy: *hasResult = true; *hasResultType = true; break;
        case OpFwidth: *hasResult = true; *hasResultType = true; break;
        case OpDPdxFine: *hasResult = true; *hasResultType = true; break;
        case OpDPdyFine: *hasResult = true; *hasResultType = true; break;
        case OpFwidthFine: *hasResult = true; *hasResultType = true; break;
        case OpDPdxCoarse: *hasResult = true; *hasResultType = true; break;
        case OpDPdyCoarse: *hasResult = true; *hasResultType = true; break;
        case OpFwidthCoarse: *hasResult = true; *hasResultType = true; break;
        case OpEmitVertex: *hasResult = false; *hasResultType = false; break;
        case OpEndPrimitive: *hasResult = false; *hasResultType = false; break;
        case OpEmitStreamVertex: *hasResult = false; *hasResultType = false; break;
        case OpEndStreamPrimitive: *hasResult = false; *hasResultType = false; break;
        case OpControlBarrier: *hasResult = false; *hasResultType = false; break;
        case OpMemoryBarrier: *hasResult = false; *hasResultType = false; break;
        case OpAtomicLoad: *hasResult = true; *hasResultType = true; break;
        case OpAtomicStore: *hasResult = false; *hasResultType = false; break;
        case OpAtomicExchange: *hasResult = true; *hasResultType = true; break;
        case OpAtomicCompareExchange: *hasResult = true; *hasResultType = true; break;
        case OpAtomicCompareExchangeWeak: *hasResult = true; *hasResultType = true; break;
        case OpAtomicIIncrement: *hasResult = true; *hasResultType = true; break;
        case OpAtomicIDecrement: *hasResult = true; *hasResultType = true; break;
        case OpAtomicIAdd: *hasResult = true; *hasResultType = true; break;
        case OpAtomicISub: *hasResult = true; *hasResultType = true; break;
        case OpAtomicSMin: *hasResult = true; *hasResultType = true; break;
        case OpAtomicUMin: *hasResult = true; *hasResultType = true; break;
        case OpAtomicSMax: *hasResult = true; *hasResultType = true; break;
        case OpAtomicUMax: *hasResult = true; *hasResultType = true; break;
        case OpAtomicAnd: *hasResult = true; *hasResultType = true; break;
        case OpAtomicOr: *hasResult = true; *hasResultType = true; break;
        case OpAtomicXor: *hasResult = true; *hasResultType = true; break;
        case OpPhi: *hasResult = true; *hasResultType = true; break;
        case OpLoopMerge: *hasResult = false; *hasResultType = false; break;
        case OpSelectionMerge: *hasResult = false; *hasResultType = false; break;
        case OpLabel: *hasResult = true; *hasResultType = false; break;
        case OpBranch: *hasResult = false; *hasResultType = false; break;
        case OpBranchConditional: *hasResult = false; *hasResultType = false; break;
        case OpSwitch: *hasResult = false; *hasResultType = false; break;
        case OpKill: *hasResult = false; *hasResultType = false; break;
        case OpReturn: *hasResult = false; *hasResultType = false; break;
        case OpReturnValue: *hasResult = false; *hasResultType = false; break;
        case OpUnreachable: *hasResult = false; *hasResultType = false; break;
        case OpLifetimeStart: *hasResult = false; *hasResultType = false; break;
        case OpLifetimeStop: *hasResult = false; *hasResultType = false; break;
        case OpGroupAsyncCopy: *hasResult = true; *hasResultType = true; break;
        case OpGroupWaitEvents: *hasResult = false; *hasResultType = false; break;
        case OpGroupAll: *hasResult = true; *hasResultType = true; break;
        case OpGroupAny: *hasResult = true; *hasResultType = true; break;
        case OpGroupBroadcast: *hasResult = true; *hasResultType = true; break;
        case OpGroupIAdd: *hasResult = true; *hasResultType = true; break;
        case OpGroupFAdd: *hasResult = true; *hasResultType = true; break;
        case OpGroupFMin: *hasResult = true; *hasResultType = true; break;
        case OpGroupUMin: *hasResult = true; *hasResultType = true; break;
        case OpGroupSMin: *hasResult = true; *hasResultType = true; break;
        case OpGroupFMax: *hasResult = true; *hasResultType = true; break;
        case OpGroupUMax: *hasResult = true; *hasResultType = true; break;
        case OpGroupSMax: *hasResult = true; *hasResultType = true; break;
        case OpReadPipe: *hasResult = true; *hasResultType = true; break;
        case OpWritePipe: *hasResult = true; *hasResultType = true; break;
        case OpReservedReadPipe: *hasResult = true; *hasResultType = true; break;
        case OpReservedWritePipe: *hasResult = true; *hasResultType = true; break;
        case OpReserveReadPipePackets: *hasResult = true; *hasResultType = true; break;
        case OpReserveWritePipePackets: *hasResult = true; *hasResultType = true; break;
        case OpCommitReadPipe: *hasResult = false; *hasResultType = false; break;
        case OpCommitWritePipe: *hasResult = false; *hasResultType = false; break;
        case OpIsValidReserveId: *hasResult = true; *hasResultType = true; break;
        case OpGetNumPipePackets: *hasResult = true; *hasResultType = true; break;
        case OpGetMaxPipePackets: *hasResult = true; *hasResultType = true; break;
        case OpGroupReserveReadPipePackets: *hasResult = true; *hasResultType = true; break;
        case OpGroupReserveWritePipePackets: *hasResult = true; *hasResultType = true; break;
        case OpGroupCommitReadPipe: *hasResult = false; *hasResultType = false; break;
        case OpGroupCommitWritePipe: *hasResult = false; *hasResultType = false; break;
        case OpEnqueueMarker: *hasResult = true; *hasResultType = true; break;
        case OpEnqueueKernel: *hasResult = true; *hasResultType = true; break;
        case OpGetKernelNDrangeSubGroupCount: *hasResult = true; *hasResultType = true; break;
        case OpGetKernelNDrangeMaxSubGroupSize: *hasResult = true; *hasResultType = true; break;
        case OpGetKernelWorkGroupSize: *hasResult = true; *hasResultType = true; break;
        case OpGetKernelPreferredWorkGroupSizeMultiple: *hasResult = true; *hasResultType = true; break;
        case OpRetainEvent: *hasResult = false; *hasResultType = false; break;
        case OpReleaseEvent: *hasResult = false; *hasResultType = false; break;
        case OpCreateUserEvent: *hasResult = true; *hasResultType = true; break;
        case OpIsValidEvent: *hasResult = true; *hasResultType = true; break;
        case OpSetUserEventStatus: *hasResult = false; *hasResultType = false; break;
        case OpCaptureEventProfilingInfo: *hasResult = false; *hasResultType = false; break;
        case OpGetDefaultQueue: *hasResult = true; *hasResultType = true; break;
        case OpBuildNDRange: *hasResult = true; *hasResultType = true; break;
        case OpImageSparseSampleImplicitLod: *hasResult = true; *hasResultType = true; break;
        case OpImageSparseSampleExplicitLod: *hasResult = true; *hasResultType = true; break;
        case OpImageSparseSampleDrefImplicitLod: *hasResult = true; *hasResultType = true; break;
        case OpImageSparseSampleDrefExplicitLod: *hasResult = true; *hasResultType = true; break;
        case OpImageSparseSampleProjImplicitLod: *hasResult = true; *hasResultType = true; break;
        case OpImageSparseSampleProjExplicitLod: *hasResult = true; *hasResultType = true; break;
        case OpImageSparseSampleProjDrefImplicitLod: *hasResult = true; *hasResultType = true; break;
        case OpImageSparseSampleProjDrefExplicitLod: *hasResult = true; *hasResultType = true; break;
        case OpImageSparseFetch: *hasResult = true; *hasResultType = true; break;
        case OpImageSparseGather: *hasResult = true; *hasResultType = true; break;
        case OpImageSparseDrefGather: *hasResult = true; *hasResultType = true; break;
        case OpImageSparseTexelsResident: *hasResult = true; *hasResultType = true; break;
        case OpNoLine: *hasResult = false; *hasResultType = false; break;
        case OpAtomicFlagTestAndSet: *hasResult = true; *hasResultType = true; break;
        case OpAtomicFlagClear: *hasResult = false; *hasResultType = false; break;
        case OpImageSparseRead: *hasResult = true; *hasResultType = true; break;
        case OpSizeOf: *hasResult = true; *hasResultType = true; break;
        case OpTypePipeStorage: *hasResult = true; *hasResultType = false; break;
        case OpConstantPipeStorage: *hasResult = true; *hasResultType = true; break;
        case OpCreatePipeFromPipeStorage: *hasResult = true; *hasResultType = true; break;
        case OpGetKernelLocalSizeForSubgroupCount: *hasResult = true; *hasResultType = true; break;
        case OpGetKernelMaxNumSubgroups: *hasResult = true; *hasResultType = true; break;
        case OpTypeNamedBarrier: *hasResult = true; *hasResultType = false; break;
        case OpNamedBarrierInitialize: *hasResult = true; *hasResultType = true; break;
        case OpMemoryNamedBarrier: *hasResult = false; *hasResultType = false; break;
        case OpModuleProcessed: *hasResult = false; *hasResultType = false; break;
        case OpExecutionModeId: *hasResult = false; *hasResultType = false; break;
        case OpDecorateId: *hasResult = false; *hasResultType = false; break;
        case OpGroupNonUniformElect: *hasResult = true; *hasResultType = true; break;
        case OpGroupNonUniformAll: *hasResult = true; *hasResultType = true; break;
        case OpGroupNonUniformAny: *hasResult = true; *hasResultType = true; break;
        case OpGroupNonUniformAllEqual: *hasResult = true; *hasResultType = true; break;
        case OpGroupNonUniformBroadcast: *hasResult = true; *hasResultType = true; break;
        case OpGroupNonUniformBroadcastFirst: *hasResult = true; *hasResultType = true; break;
        case OpGroupNonUniformBallot: *hasResult = true; *hasResultType = true; break;
        case OpGroupNonUniformInverseBallot: *hasResult = true; *hasResultType = true; break;
        case OpGroupNonUniformBallotBitExtract: *hasResult = true; *hasResultType = true; break;
        case OpGroupNonUniformBallotBitCount: *hasResult = true; *hasResultType = true; break;
        case OpGroupNonUniformBallotFindLSB: *hasResult = true; *hasResultType = true; break;
        case OpGroupNonUniformBallotFindMSB: *hasResult = true; *hasResultType = true; break;
        case OpGroupNonUniformShuffle: *hasResult = true; *hasResultType = true; break;
        case OpGroupNonUniformShuffleXor: *hasResult = true; *hasResultType = true; break;
        case OpGroupNonUniformShuffleUp: *hasResult = true; *hasResultType = true; break;
        case OpGroupNonUniformShuffleDown: *hasResult = true; *hasResultType = true; break;
        case OpGroupNonUniformIAdd: *hasResult = true; *hasResultType = true; break;
        case OpGroupNonUniformFAdd: *hasResult = true; *hasResultType = true; break;
        case OpGroupNonUniformIMul: *hasResult = true; *hasResultType = true; break;
        case OpGroupNonUniformFMul: *hasResult = true; *hasResultType = true; break;
        case OpGroupNonUniformSMin: *hasResult = true; *hasResultType = true; break;
        case OpGroupNonUniformUMin: *hasResult = true; *hasResultType = true; break;
        case OpGroupNonUniformFMin: *hasResult = true; *hasResultType = true; break;
        case OpGroupNonUniformSMax: *hasResult = true; *hasResultType = true; break;
        case OpGroupNonUniformUMax: *hasResult = true; *hasResultType = true; break;
        case OpGroupNonUniformFMax: *hasResult = true; *hasResultType = true; break;
        case OpGroupNonUniformBitwiseAnd: *hasResult = true; *hasResultType = true; break;
        case OpGroupNonUniformBitwiseOr: *hasResult = true; *hasResultType = true; break;
        case OpGroupNonUniformBitwiseXor: *hasResult = true; *hasResultType = true; break;
        case OpGroupNonUniformLogicalAnd: *hasResult = true; *hasResultType = true; break;
        case OpGroupNonUniformLogicalOr: *hasResult = true; *hasResultType = true; break;
        case OpGroupNonUniformLogicalXor: *hasResult = true; *hasResultType = true; break;
        case OpGroupNonUniformQuadBroadcast: *hasResult = true; *hasResultType = true; break;
        case OpGroupNonUniformQuadSwap: *hasResult = true; *hasResultType = true; break;
        case OpCopyLogical: *hasResult = true; *hasResultType = true; break;
        case OpPtrEqual: *hasResult = true; *hasResultType = true; break;
        case OpPtrNotEqual: *hasResult = true; *hasResultType = true; break;
        case OpPtrDiff: *hasResult = true; *hasResultType = true; break;
        case OpColorAttachmentReadEXT: *hasResult = true; *hasResultType = true; break;
        case OpDepthAttachmentReadEXT: *hasResult = true; *hasResultType = true; break;
        case OpStencilAttachmentReadEXT: *hasResult = true; *hasResultType = true; break;
        case OpTerminateInvocation: *hasResult = false; *hasResultType = false; break;
        case OpSubgroupBallotKHR: *hasResult = true; *hasResultType = true; break;
        case OpSubgroupFirstInvocationKHR: *hasResult = true; *hasResultType = true; break;
        case OpSubgroupAllKHR: *hasResult = true; *hasResultType = true; break;
        case OpSubgroupAnyKHR: *hasResult = true; *hasResultType = true; break;
        case OpSubgroupAllEqualKHR: *hasResult = true; *hasResultType = true; break;
        case OpGroupNonUniformRotateKHR: *hasResult = true; *hasResultType = true; break;
        case OpSubgroupReadInvocationKHR: *hasResult = true; *hasResultType = true; break;
        case OpTraceRayKHR: *hasResult = false; *hasResultType = false; break;
        case OpExecuteCallableKHR: *hasResult = false; *hasResultType = false; break;
        case OpConvertUToAccelerationStructureKHR: *hasResult = true; *hasResultType = true; break;
        case OpIgnoreIntersectionKHR: *hasResult = false; *hasResultType = false; break;
        case OpTerminateRayKHR: *hasResult = false; *hasResultType = false; break;
        case OpSDot: *hasResult = true; *hasResultType = true; break;
        case OpUDot: *hasResult = true; *hasResultType = true; break;
        case OpSUDot: *hasResult = true; *hasResultType = true; break;
        case OpSDotAccSat: *hasResult = true; *hasResultType = true; break;
        case OpUDotAccSat: *hasResult = true; *hasResultType = true; break;
        case OpSUDotAccSat: *hasResult = true; *hasResultType = true; break;
        case OpTypeCooperativeMatrixKHR: *hasResult = true; *hasResultType = false; break;
        case OpCooperativeMatrixLoadKHR: *hasResult = true; *hasResultType = true; break;
        case OpCooperativeMatrixStoreKHR: *hasResult = false; *hasResultType = false; break;
        case OpCooperativeMatrixMulAddKHR: *hasResult = true; *hasResultType = true; break;
        case OpCooperativeMatrixLengthKHR: *hasResult = true; *hasResultType = true; break;
        case OpTypeRayQueryKHR: *hasResult = true; *hasResultType = false; break;
        case OpRayQueryInitializeKHR: *hasResult = false; *hasResultType = false; break;
        case OpRayQueryTerminateKHR: *hasResult = false; *hasResultType = false; break;
        case OpRayQueryGenerateIntersectionKHR: *hasResult = false; *hasResultType = false; break;
        case OpRayQueryConfirmIntersectionKHR: *hasResult = false; *hasResultType = false; break;
        case OpRayQueryProceedKHR: *hasResult = true; *hasResultType = true; break;
        case OpRayQueryGetIntersectionTypeKHR: *hasResult = true; *hasResultType = true; break;
        case OpImageSampleWeightedQCOM: *hasResult = true; *hasResultType = true; break;
        case OpImageBoxFilterQCOM: *hasResult = true; *hasResultType = true; break;
        case OpImageBlockMatchSSDQCOM: *hasResult = true; *hasResultType = true; break;
        case OpImageBlockMatchSADQCOM: *hasResult = true; *hasResultType = true; break;
        case OpGroupIAddNonUniformAMD: *hasResult = true; *hasResultType = true; break;
        case OpGroupFAddNonUniformAMD: *hasResult = true; *hasResultType = true; break;
        case OpGroupFMinNonUniformAMD: *hasResult = true; *hasResultType = true; break;
        case OpGroupUMinNonUniformAMD: *hasResult = true; *hasResultType = true; break;
        case OpGroupSMinNonUniformAMD: *hasResult = true; *hasResultType = true; break;
        case OpGroupFMaxNonUniformAMD: *hasResult = true; *hasResultType = true; break;
        case OpGroupUMaxNonUniformAMD: *hasResult = true; *hasResultType = true; break;
        case OpGroupSMaxNonUniformAMD: *hasResult = true; *hasResultType = true; break;
        case OpFragmentMaskFetchAMD: *hasResult = true; *hasResultType = true; break;
        case OpFragmentFetchAMD: *hasResult = true; *hasResultType = true; break;
        case OpReadClockKHR: *hasResult = true; *hasResultType = true; break;
        case OpHitObjectRecordHitMotionNV: *hasResult = false; *hasResultType = false; break;
        case OpHitObjectRecordHitWithIndexMotionNV: *hasResult = false; *hasResultType = false; break;
        case OpHitObjectRecordMissMotionNV: *hasResult = false; *hasResultType = false; break;
        case OpHitObjectGetWorldToObjectNV: *hasResult = true; *hasResultType = true; break;
        case OpHitObjectGetObjectToWorldNV: *hasResult = true; *hasResultType = true; break;
        case OpHitObjectGetObjectRayDirectionNV: *hasResult = true; *hasResultType = true; break;
        case OpHitObjectGetObjectRayOriginNV: *hasResult = true; *hasResultType = true; break;
        case OpHitObjectTraceRayMotionNV: *hasResult = false; *hasResultType = false; break;
        case OpHitObjectGetShaderRecordBufferHandleNV: *hasResult = true; *hasResultType = true; break;
        case OpHitObjectGetShaderBindingTableRecordIndexNV: *hasResult = true; *hasResultType = true; break;
        case OpHitObjectRecordEmptyNV: *hasResult = false; *hasResultType = false; break;
        case OpHitObjectTraceRayNV: *hasResult = false; *hasResultType = false; break;
        case OpHitObjectRecordHitNV: *hasResult = false; *hasResultType = false; break;
        case OpHitObjectRecordHitWithIndexNV: *hasResult = false; *hasResultType = false; break;
        case OpHitObjectRecordMissNV: *hasResult = false; *hasResultType = false; break;
        case OpHitObjectExecuteShaderNV: *hasResult = false; *hasResultType = false; break;
        case OpHitObjectGetCurrentTimeNV: *hasResult = true; *hasResultType = true; break;
        case OpHitObjectGetAttributesNV: *hasResult = false; *hasResultType = false; break;
        case OpHitObjectGetHitKindNV: *hasResult = true; *hasResultType = true; break;
        case OpHitObjectGetPrimitiveIndexNV: *hasResult = true; *hasResultType = true; break;
        case OpHitObjectGetGeometryIndexNV: *hasResult = true; *hasResultType = true; break;
        case OpHitObjectGetInstanceIdNV: *hasResult = true; *hasResultType = true; break;
        case OpHitObjectGetInstanceCustomIndexNV: *hasResult = true; *hasResultType = true; break;
        case OpHitObjectGetWorldRayDirectionNV: *hasResult = true; *hasResultType = true; break;
        case OpHitObjectGetWorldRayOriginNV: *hasResult = true; *hasResultType = true; break;
        case OpHitObjectGetRayTMaxNV: *hasResult = true; *hasResultType = true; break;
        case OpHitObjectGetRayTMinNV: *hasResult = true; *hasResultType = true; break;
        case OpHitObjectIsEmptyNV: *hasResult = true; *hasResultType = true; break;
        case OpHitObjectIsHitNV: *hasResult = true; *hasResultType = true; break;
        case OpHitObjectIsMissNV: *hasResult = true; *hasResultType = true; break;
        case OpReorderThreadWithHitObjectNV: *hasResult = false; *hasResultType = false; break;
        case OpReorderThreadWithHintNV: *hasResult = false; *hasResultType = false; break;
        case OpTypeHitObjectNV: *hasResult = true; *hasResultType = false; break;
        case OpImageSampleFootprintNV: *hasResult = true; *hasResultType = true; break;
        case OpEmitMeshTasksEXT: *hasResult = false; *hasResultType = false; break;
        case OpSetMeshOutputsEXT: *hasResult = false; *hasResultType = false; break;
        case OpGroupNonUniformPartitionNV: *hasResult = true; *hasResultType = true; break;
        case OpWritePackedPrimitiveIndices4x8NV: *hasResult = false; *hasResultType = false; break;
        case OpReportIntersectionNV: *hasResult = true; *hasResultType = true; break;
        case OpIgnoreIntersectionNV: *hasResult = false; *hasResultType = false; break;
        case OpTerminateRayNV: *hasResult = false; *hasResultType = false; break;
        case OpTraceNV: *hasResult = false; *hasResultType = false; break;
        case OpTraceMotionNV: *hasResult = false; *hasResultType = false; break;
        case OpTraceRayMotionNV: *hasResult = false; *hasResultType = false; break;
        case OpRayQueryGetIntersectionTriangleVertexPositionsKHR: *hasResult = true; *hasResultType = true; break;
        case OpTypeAccelerationStructureNV: *hasResult = true; *hasResultType = false; break;
        case OpExecuteCallableNV: *hasResult = false; *hasResultType = false; break;
        case OpTypeCooperativeMatrixNV: *hasResult = true; *hasResultType = false; break;
        case OpCooperativeMatrixLoadNV: *hasResult = true; *hasResultType = true; break;
        case OpCooperativeMatrixStoreNV: *hasResult = false; *hasResultType = false; break;
        case OpCooperativeMatrixMulAddNV: *hasResult = true; *hasResultType = true; break;
        case OpCooperativeMatrixLengthNV: *hasResult = true; *hasResultType = true; break;
        case OpBeginInvocationInterlockEXT: *hasResult = false; *hasResultType = false; break;
        case OpEndInvocationInterlockEXT: *hasResult = false; *hasResultType = false; break;
        case OpDemoteToHelperInvocation: *hasResult = false; *hasResultType = false; break;
        case OpIsHelperInvocationEXT: *hasResult = true; *hasResultType = true; break;
        case OpConvertUToImageNV: *hasResult = true; *hasResultType = true; break;
        case OpConvertUToSamplerNV: *hasResult = true; *hasResultType = true; break;
        case OpConvertImageToUNV: *hasResult = true; *hasResultType = true; break;
        case OpConvertSamplerToUNV: *hasResult = true; *hasResultType = true; break;
        case OpConvertUToSampledImageNV: *hasResult = true; *hasResultType = true; break;
        case OpConvertSampledImageToUNV: *hasResult = true; *hasResultType = true; break;
        case OpSamplerImageAddressingModeNV: *hasResult = false; *hasResultType = false; break;
        case OpSubgroupShuffleINTEL: *hasResult = true; *hasResultType = true; break;
        case OpSubgroupShuffleDownINTEL: *hasResult = true; *hasResultType = true; break;
        case OpSubgroupShuffleUpINTEL: *hasResult = true; *hasResultType = true; break;
        case OpSubgroupShuffleXorINTEL: *hasResult = true; *hasResultType = true; break;
        case OpSubgroupBlockReadINTEL: *hasResult = true; *hasResultType = true; break;
        case OpSubgroupBlockWriteINTEL: *hasResult = false; *hasResultType = false; break;
        case OpSubgroupImageBlockReadINTEL: *hasResult = true; *hasResultType = true; break;
        case OpSubgroupImageBlockWriteINTEL: *hasResult = false; *hasResultType = false; break;
        case OpSubgroupImageMediaBlockReadINTEL: *hasResult = true; *hasResultType = true; break;
        case OpSubgroupImageMediaBlockWriteINTEL: *hasResult = false; *hasResultType = false; break;
        case OpUCountLeadingZerosINTEL: *hasResult = true; *hasResultType = true; break;
        case OpUCountTrailingZerosINTEL: *hasResult = true; *hasResultType = true; break;
        case OpAbsISubINTEL: *hasResult = true; *hasResultType = true; break;
        case OpAbsUSubINTEL: *hasResult = true; *hasResultType = true; break;
        case OpIAddSatINTEL: *hasResult = true; *hasResultType = true; break;
        case OpUAddSatINTEL: *hasResult = true; *hasResultType = true; break;
        case OpIAverageINTEL: *hasResult = true; *hasResultType = true; break;
        case OpUAverageINTEL: *hasResult = true; *hasResultType = true; break;
        case OpIAverageRoundedINTEL: *hasResult = true; *hasResultType = true; break;
        case OpUAverageRoundedINTEL: *hasResult = true; *hasResultType = true; break;
        case OpISubSatINTEL: *hasResult = true; *hasResultType = true; break;
        case OpUSubSatINTEL: *hasResult = true; *hasResultType = true; break;
        case OpIMul32x16INTEL: *hasResult = true; *hasResultType = true; break;
        case OpUMul32x16INTEL: *hasResult = true; *hasResultType = true; break;
        case OpConstantFunctionPointerINTEL: *hasResult = true; *hasResultType = true; break;
        case OpFunctionPointerCallINTEL: *hasResult = true; *hasResultType = true; break;
        case OpAsmTargetINTEL: *hasResult = true; *hasResultType = true; break;
        case OpAsmINTEL: *hasResult = true; *hasResultType = true; break;
        case OpAsmCallINTEL: *hasResult = true; *hasResultType = true; break;
        case OpAtomicFMinEXT: *hasResult = true; *hasResultType = true; break;
        case OpAtomicFMaxEXT: *hasResult = true; *hasResultType = true; break;
        case OpAssumeTrueKHR: *hasResult = false; *hasResultType = false; break;
        case OpExpectKHR: *hasResult = true; *hasResultType = true; break;
        case OpDecorateString: *hasResult = false; *hasResultType = false; break;
        case OpMemberDecorateString: *hasResult = false; *hasResultType = false; break;
        case OpVmeImageINTEL: *hasResult = true; *hasResultType = true; break;
        case OpTypeVmeImageINTEL: *hasResult = true; *hasResultType = false; break;
        case OpTypeAvcImePayloadINTEL: *hasResult = true; *hasResultType = false; break;
        case OpTypeAvcRefPayloadINTEL: *hasResult = true; *hasResultType = false; break;
        case OpTypeAvcSicPayloadINTEL: *hasResult = true; *hasResultType = false; break;
        case OpTypeAvcMcePayloadINTEL: *hasResult = true; *hasResultType = false; break;
        case OpTypeAvcMceResultINTEL: *hasResult = true; *hasResultType = false; break;
        case OpTypeAvcImeResultINTEL: *hasResult = true; *hasResultType = false; break;
        case OpTypeAvcImeResultSingleReferenceStreamoutINTEL: *hasResult = true; *hasResultType = false; break;
        case OpTypeAvcImeResultDualReferenceStreamoutINTEL: *hasResult = true; *hasResultType = false; break;
        case OpTypeAvcImeSingleReferenceStreaminINTEL: *hasResult = true; *hasResultType = false; break;
        case OpTypeAvcImeDualReferenceStreaminINTEL: *hasResult = true; *hasResultType = false; break;
        case OpTypeAvcRefResultINTEL: *hasResult = true; *hasResultType = false; break;
        case OpTypeAvcSicResultINTEL: *hasResult = true; *hasResultType = false; break;
        case OpSubgroupAvcMceGetDefaultInterBaseMultiReferencePenaltyINTEL: *hasResult = true; *hasResultType = true; break;
        case OpSubgroupAvcMceSetInterBaseMultiReferencePenaltyINTEL: *hasResult = true; *hasResultType = true; break;
        case OpSubgroupAvcMceGetDefaultInterShapePenaltyINTEL: *hasResult = true; *hasResultType = true; break;
        case OpSubgroupAvcMceSetInterShapePenaltyINTEL: *hasResult = true; *hasResultType = true; break;
        case OpSubgroupAvcMceGetDefaultInterDirectionPenaltyINTEL: *hasResult = true; *hasResultType = true; break;
        case OpSubgroupAvcMceSetInterDirectionPenaltyINTEL: *hasResult = true; *hasResultType = true; break;
        case OpSubgroupAvcMceGetDefaultIntraLumaShapePenaltyINTEL: *hasResult = true; *hasResultType = true; break;
        case OpSubgroupAvcMceGetDefaultInterMotionVectorCostTableINTEL: *hasResult = true; *hasResultType = true; break;
        case OpSubgroupAvcMceGetDefaultHighPenaltyCostTableINTEL: *hasResult = true; *hasResultType = true; break;
        case OpSubgroupAvcMceGetDefaultMediumPenaltyCostTableINTEL: *hasResult = true; *hasResultType = true; break;
        case OpSubgroupAvcMceGetDefaultLowPenaltyCostTableINTEL: *hasResult = true; *hasResultType = true; break;
        case OpSubgroupAvcMceSetMotionVectorCostFunctionINTEL: *hasResult = true; *hasResultType = true; break;
        case OpSubgroupAvcMceGetDefaultIntraLumaModePenaltyINTEL: *hasResult = true; *hasResultType = true; break;
        case OpSubgroupAvcMceGetDefaultNonDcLumaIntraPenaltyINTEL: *hasResult = true; *hasResultType = true; break;
        case OpSubgroupAvcMceGetDefaultIntraChromaModeBasePenaltyINTEL: *hasResult = true; *hasResultType = true; break;
        case OpSubgroupAvcMceSetAcOnlyHaarINTEL: *hasResult = true; *hasResultType = true; break;
        case OpSubgroupAvcMceSetSourceInterlacedFieldPolarityINTEL: *hasResult = true; *hasResultType = true; break;
        case OpSubgroupAvcMceSetSingleReferenceInterlacedFieldPolarityINTEL: *hasResult = true; *hasResultType = true; break;
        case OpSubgroupAvcMceSetDualReferenceInterlacedFieldPolaritiesINTEL: *hasResult = true; *hasResultType = true; break;
        case OpSubgroupAvcMceConvertToImePayloadINTEL: *hasResult = true; *hasResultType = true; break;
        case OpSubgroupAvcMceConvertToImeResultINTEL: *hasResult = true; *hasResultType = true; break;
        case OpSubgroupAvcMceConvertToRefPayloadINTEL: *hasResult = true; *hasResultType = true; break;
        case OpSubgroupAvcMceConvertToRefResultINTEL: *hasResult = true; *hasResultType = true; break;
        case OpSubgroupAvcMceConvertToSicPayloadINTEL: *hasResult = true; *hasResultType = true; break;
        case OpSubgroupAvcMceConvertToSicResultINTEL: *hasResult = true; *hasResultType = true; break;
        case OpSubgroupAvcMceGetMotionVectorsINTEL: *hasResult = true; *hasResultType = true; break;
        case OpSubgroupAvcMceGetInterDistortionsINTEL: *hasResult = true; *hasResultType = true; break;
        case OpSubgroupAvcMceGetBestInterDistortionsINTEL: *hasResult = true; *hasResultType = true; break;
        case OpSubgroupAvcMceGetInterMajorShapeINTEL: *hasResult = true; *hasResultType = true; break;
        case OpSubgroupAvcMceGetInterMinorShapeINTEL: *hasResult = true; *hasResultType = true; break;
        case OpSubgroupAvcMceGetInterDirectionsINTEL: *hasResult = true; *hasResultType = true; break;
        case OpSubgroupAvcMceGetInterMotionVectorCountINTEL: *hasResult = true; *hasResultType = true; break;
        case OpSubgroupAvcMceGetInterReferenceIdsINTEL: *hasResult = true; *hasResultType = true; break;
        case OpSubgroupAvcMceGetInterReferenceInterlacedFieldPolaritiesINTEL: *hasResult = true; *hasResultType = true; break;
        case OpSubgroupAvcImeInitializeINTEL: *hasResult = true; *hasResultType = true; break;
        case OpSubgroupAvcImeSetSingleReferenceINTEL: *hasResult = true; *hasResultType = true; break;
        case OpSubgroupAvcImeSetDualReferenceINTEL: *hasResult = true; *hasResultType = true; break;
        case OpSubgroupAvcImeRefWindowSizeINTEL: *hasResult = true; *hasResultType = true; break;
        case OpSubgroupAvcImeAdjustRefOffsetINTEL: *hasResult = true; *hasResultType = true; break;
        case OpSubgroupAvcImeConvertToMcePayloadINTEL: *hasResult = true; *hasResultType = true; break;
        case OpSubgroupAvcImeSetMaxMotionVectorCountINTEL: *hasResult = true; *hasResultType = true; break;
        case OpSubgroupAvcImeSetUnidirectionalMixDisableINTEL: *hasResult = true; *hasResultType = true; break;
        case OpSubgroupAvcImeSetEarlySearchTerminationThresholdINTEL: *hasResult = true; *hasResultType = true; break;
        case OpSubgroupAvcImeSetWeightedSadINTEL: *hasResult = true; *hasResultType = true; break;
        case OpSubgroupAvcImeEvaluateWithSingleReferenceINTEL: *hasResult = true; *hasResultType = true; break;
        case OpSubgroupAvcImeEvaluateWithDualReferenceINTEL: *hasResult = true; *hasResultType = true; break;
        case OpSubgroupAvcImeEvaluateWithSingleReferenceStreaminINTEL: *hasResult = true; *hasResultType = true; break;
        case OpSubgroupAvcImeEvaluateWithDualReferenceStreaminINTEL: *hasResult = true; *hasResultType = true; break;
        case OpSubgroupAvcImeEvaluateWithSingleReferenceStreamoutINTEL: *hasResult = true; *hasResultType = true; break;
        case OpSubgroupAvcImeEvaluateWithDualReferenceStreamoutINTEL: *hasResult = true; *hasResultType = true; break;
        case OpSubgroupAvcImeEvaluateWithSingleReferenceStreaminoutINTEL: *hasResult = true; *hasResultType = true; break;
        case OpSubgroupAvcImeEvaluateWithDualReferenceStreaminoutINTEL: *hasResult = true; *hasResultType = true; break;
        case OpSubgroupAvcImeConvertToMceResultINTEL: *hasResult = true; *hasResultType = true; break;
        case OpSubgroupAvcImeGetSingleReferenceStreaminINTEL: *hasResult = true; *hasResultType = true; break;
        case OpSubgroupAvcImeGetDualReferenceStreaminINTEL: *hasResult = true; *hasResultType = true; break;
        case OpSubgroupAvcImeStripSingleReferenceStreamoutINTEL: *hasResult = true; *hasResultType = true; break;
        case OpSubgroupAvcImeStripDualReferenceStreamoutINTEL: *hasResult = true; *hasResultType = true; break;
        case OpSubgroupAvcImeGetStreamoutSingleReferenceMajorShapeMotionVectorsINTEL: *hasResult = true; *hasResultType = true; break;
        case OpSubgroupAvcImeGetStreamoutSingleReferenceMajorShapeDistortionsINTEL: *hasResult = true; *hasResultType = true; break;
        case OpSubgroupAvcImeGetStreamoutSingleReferenceMajorShapeReferenceIdsINTEL: *hasResult = true; *hasResultType = true; break;
        case OpSubgroupAvcImeGetStreamoutDualReferenceMajorShapeMotionVectorsINTEL: *hasResult = true; *hasResultType = true; break;
        case OpSubgroupAvcImeGetStreamoutDualReferenceMajorShapeDistortionsINTEL: *hasResult = true; *hasResultType = true; break;
        case OpSubgroupAvcImeGetStreamoutDualReferenceMajorShapeReferenceIdsINTEL: *hasResult = true; *hasResultType = true; break;
        case OpSubgroupAvcImeGetBorderReachedINTEL: *hasResult = true; *hasResultType = true; break;
        case OpSubgroupAvcImeGetTruncatedSearchIndicationINTEL: *hasResult = true; *hasResultType = true; break;
        case OpSubgroupAvcImeGetUnidirectionalEarlySearchTerminationINTEL: *hasResult = true; *hasResultType = true; break;
        case OpSubgroupAvcImeGetWeightingPatternMinimumMotionVectorINTEL: *hasResult = true; *hasResultType = true; break;
        case OpSubgroupAvcImeGetWeightingPatternMinimumDistortionINTEL: *hasResult = true; *hasResultType = true; break;
        case OpSubgroupAvcFmeInitializeINTEL: *hasResult = true; *hasResultType = true; break;
        case OpSubgroupAvcBmeInitializeINTEL: *hasResult = true; *hasResultType = true; break;
        case OpSubgroupAvcRefConvertToMcePayloadINTEL: *hasResult = true; *hasResultType = true; break;
        case OpSubgroupAvcRefSetBidirectionalMixDisableINTEL: *hasResult = true; *hasResultType = true; break;
        case OpSubgroupAvcRefSetBilinearFilterEnableINTEL: *hasResult = true; *hasResultType = true; break;
        case OpSubgroupAvcRefEvaluateWithSingleReferenceINTEL: *hasResult = true; *hasResultType = true; break;
        case OpSubgroupAvcRefEvaluateWithDualReferenceINTEL: *hasResult = true; *hasResultType = true; break;
        case OpSubgroupAvcRefEvaluateWithMultiReferenceINTEL: *hasResult = true; *hasResultType = true; break;
        case OpSubgroupAvcRefEvaluateWithMultiReferenceInterlacedINTEL: *hasResult = true; *hasResultType = true; break;
        case OpSubgroupAvcRefConvertToMceResultINTEL: *hasResult = true; *hasResultType = true; break;
        case OpSubgroupAvcSicInitializeINTEL: *hasResult = true; *hasResultType = true; break;
        case OpSubgroupAvcSicConfigureSkcINTEL: *hasResult = true; *hasResultType = true; break;
        case OpSubgroupAvcSicConfigureIpeLumaINTEL: *hasResult = true; *hasResultType = true; break;
        case OpSubgroupAvcSicConfigureIpeLumaChromaINTEL: *hasResult = true; *hasResultType = true; break;
        case OpSubgroupAvcSicGetMotionVectorMaskINTEL: *hasResult = true; *hasResultType = true; break;
        case OpSubgroupAvcSicConvertToMcePayloadINTEL: *hasResult = true; *hasResultType = true; break;
        case OpSubgroupAvcSicSetIntraLumaShapePenaltyINTEL: *hasResult = true; *hasResultType = true; break;
        case OpSubgroupAvcSicSetIntraLumaModeCostFunctionINTEL: *hasResult = true; *hasResultType = true; break;
        case OpSubgroupAvcSicSetIntraChromaModeCostFunctionINTEL: *hasResult = true; *hasResultType = true; break;
        case OpSubgroupAvcSicSetBilinearFilterEnableINTEL: *hasResult = true; *hasResultType = true; break;
        case OpSubgroupAvcSicSetSkcForwardTransformEnableINTEL: *hasResult = true; *hasResultType = true; break;
        case OpSubgroupAvcSicSetBlockBasedRawSkipSadINTEL: *hasResult = true; *hasResultType = true; break;
        case OpSubgroupAvcSicEvaluateIpeINTEL: *hasResult = true; *hasResultType = true; break;
        case OpSubgroupAvcSicEvaluateWithSingleReferenceINTEL: *hasResult = true; *hasResultType = true; break;
        case OpSubgroupAvcSicEvaluateWithDualReferenceINTEL: *hasResult = true; *hasResultType = true; break;
        case OpSubgroupAvcSicEvaluateWithMultiReferenceINTEL: *hasResult = true; *hasResultType = true; break;
        case OpSubgroupAvcSicEvaluateWithMultiReferenceInterlacedINTEL: *hasResult = true; *hasResultType = true; break;
        case OpSubgroupAvcSicConvertToMceResultINTEL: *hasResult = true; *hasResultType = true; break;
        case OpSubgroupAvcSicGetIpeLumaShapeINTEL: *hasResult = true; *hasResultType = true; break;
        case OpSubgroupAvcSicGetBestIpeLumaDistortionINTEL: *hasResult = true; *hasResultType = true; break;
        case OpSubgroupAvcSicGetBestIpeChromaDistortionINTEL: *hasResult = true; *hasResultType = true; break;
        case OpSubgroupAvcSicGetPackedIpeLumaModesINTEL: *hasResult = true; *hasResultType = true; break;
        case OpSubgroupAvcSicGetIpeChromaModeINTEL: *hasResult = true; *hasResultType = true; break;
        case OpSubgroupAvcSicGetPackedSkcLumaCountThresholdINTEL: *hasResult = true; *hasResultType = true; break;
        case OpSubgroupAvcSicGetPackedSkcLumaSumThresholdINTEL: *hasResult = true; *hasResultType = true; break;
        case OpSubgroupAvcSicGetInterRawSadsINTEL: *hasResult = true; *hasResultType = true; break;
        case OpVariableLengthArrayINTEL: *hasResult = true; *hasResultType = true; break;
        case OpSaveMemoryINTEL: *hasResult = true; *hasResultType = true; break;
        case OpRestoreMemoryINTEL: *hasResult = false; *hasResultType = false; break;
        case OpArbitraryFloatSinCosPiINTEL: *hasResult = true; *hasResultType = true; break;
        case OpArbitraryFloatCastINTEL: *hasResult = true; *hasResultType = true; break;
        case OpArbitraryFloatCastFromIntINTEL: *hasResult = true; *hasResultType = true; break;
        case OpArbitraryFloatCastToIntINTEL: *hasResult = true; *hasResultType = true; break;
        case OpArbitraryFloatAddINTEL: *hasResult = true; *hasResultType = true; break;
        case OpArbitraryFloatSubINTEL: *hasResult = true; *hasResultType = true; break;
        case OpArbitraryFloatMulINTEL: *hasResult = true; *hasResultType = true; break;
        case OpArbitraryFloatDivINTEL: *hasResult = true; *hasResultType = true; break;
        case OpArbitraryFloatGTINTEL: *hasResult = true; *hasResultType = true; break;
        case OpArbitraryFloatGEINTEL: *hasResult = true; *hasResultType = true; break;
        case OpArbitraryFloatLTINTEL: *hasResult = true; *hasResultType = true; break;
        case OpArbitraryFloatLEINTEL: *hasResult = true; *hasResultType = true; break;
        case OpArbitraryFloatEQINTEL: *hasResult = true; *hasResultType = true; break;
        case OpArbitraryFloatRecipINTEL: *hasResult = true; *hasResultType = true; break;
        case OpArbitraryFloatRSqrtINTEL: *hasResult = true; *hasResultType = true; break;
        case OpArbitraryFloatCbrtINTEL: *hasResult = true; *hasResultType = true; break;
        case OpArbitraryFloatHypotINTEL: *hasResult = true; *hasResultType = true; break;
        case OpArbitraryFloatSqrtINTEL: *hasResult = true; *hasResultType = true; break;
        case OpArbitraryFloatLogINTEL: *hasResult = true; *hasResultType = true; break;
        case OpArbitraryFloatLog2INTEL: *hasResult = true; *hasResultType = true; break;
        case OpArbitraryFloatLog10INTEL: *hasResult = true; *hasResultType = true; break;
        case OpArbitraryFloatLog1pINTEL: *hasResult = true; *hasResultType = true; break;
        case OpArbitraryFloatExpINTEL: *hasResult = true; *hasResultType = true; break;
        case OpArbitraryFloatExp2INTEL: *hasResult = true; *hasResultType = true; break;
        case OpArbitraryFloatExp10INTEL: *hasResult = true; *hasResultType = true; break;
        case OpArbitraryFloatExpm1INTEL: *hasResult = true; *hasResultType = true; break;
        case OpArbitraryFloatSinINTEL: *hasResult = true; *hasResultType = true; break;
        case OpArbitraryFloatCosINTEL: *hasResult = true; *hasResultType = true; break;
        case OpArbitraryFloatSinCosINTEL: *hasResult = true; *hasResultType = true; break;
        case OpArbitraryFloatSinPiINTEL: *hasResult = true; *hasResultType = true; break;
        case OpArbitraryFloatCosPiINTEL: *hasResult = true; *hasResultType = true; break;
        case OpArbitraryFloatASinINTEL: *hasResult = true; *hasResultType = true; break;
        case OpArbitraryFloatASinPiINTEL: *hasResult = true; *hasResultType = true; break;
        case OpArbitraryFloatACosINTEL: *hasResult = true; *hasResultType = true; break;
        case OpArbitraryFloatACosPiINTEL: *hasResult = true; *hasResultType = true; break;
        case OpArbitraryFloatATanINTEL: *hasResult = true; *hasResultType = true; break;
        case OpArbitraryFloatATanPiINTEL: *hasResult = true; *hasResultType = true; break;
        case OpArbitraryFloatATan2INTEL: *hasResult = true; *hasResultType = true; break;
        case OpArbitraryFloatPowINTEL: *hasResult = true; *hasResultType = true; break;
        case OpArbitraryFloatPowRINTEL: *hasResult = true; *hasResultType = true; break;
        case OpArbitraryFloatPowNINTEL: *hasResult = true; *hasResultType = true; break;
        case OpLoopControlINTEL: *hasResult = false; *hasResultType = false; break;
        case OpAliasDomainDeclINTEL: *hasResult = true; *hasResultType = false; break;
        case OpAliasScopeDeclINTEL: *hasResult = true; *hasResultType = false; break;
        case OpAliasScopeListDeclINTEL: *hasResult = true; *hasResultType = false; break;
        case OpFixedSqrtINTEL: *hasResult = true; *hasResultType = true; break;
        case OpFixedRecipINTEL: *hasResult = true; *hasResultType = true; break;
        case OpFixedRsqrtINTEL: *hasResult = true; *hasResultType = true; break;
        case OpFixedSinINTEL: *hasResult = true; *hasResultType = true; break;
        case OpFixedCosINTEL: *hasResult = true; *hasResultType = true; break;
        case OpFixedSinCosINTEL: *hasResult = true; *hasResultType = true; break;
        case OpFixedSinPiINTEL: *hasResult = true; *hasResultType = true; break;
        case OpFixedCosPiINTEL: *hasResult = true; *hasResultType = true; break;
        case OpFixedSinCosPiINTEL: *hasResult = true; *hasResultType = true; break;
        case OpFixedLogINTEL: *hasResult = true; *hasResultType = true; break;
        case OpFixedExpINTEL: *hasResult = true; *hasResultType = true; break;
        case OpPtrCastToCrossWorkgroupINTEL: *hasResult = true; *hasResultType = true; break;
        case OpCrossWorkgroupCastToPtrINTEL: *hasResult = true; *hasResultType = true; break;
        case OpReadPipeBlockingINTEL: *hasResult = true; *hasResultType = true; break;
        case OpWritePipeBlockingINTEL: *hasResult = true; *hasResultType = true; break;
        case OpFPGARegINTEL: *hasResult = true; *hasResultType = true; break;
        case OpRayQueryGetRayTMinKHR: *hasResult = true; *hasResultType = true; break;
        case OpRayQueryGetRayFlagsKHR: *hasResult = true; *hasResultType = true; break;
        case OpRayQueryGetIntersectionTKHR: *hasResult = true; *hasResultType = true; break;
        case OpRayQueryGetIntersectionInstanceCustomIndexKHR: *hasResult = true; *hasResultType = true; break;
        case OpRayQueryGetIntersectionInstanceIdKHR: *hasResult = true; *hasResultType = true; break;
        case OpRayQueryGetIntersectionInstanceShaderBindingTableRecordOffsetKHR: *hasResult = true; *hasResultType = true; break;
        case OpRayQueryGetIntersectionGeometryIndexKHR: *hasResult = true; *hasResultType = true; break;
        case OpRayQueryGetIntersectionPrimitiveIndexKHR: *hasResult = true; *hasResultType = true; break;
        case OpRayQueryGetIntersectionBarycentricsKHR: *hasResult = true; *hasResultType = true; break;
        case OpRayQueryGetIntersectionFrontFaceKHR: *hasResult = true; *hasResultType = true; break;
        case OpRayQueryGetIntersectionCandidateAABBOpaqueKHR: *hasResult = true; *hasResultType = true; break;
        case OpRayQueryGetIntersectionObjectRayDirectionKHR: *hasResult = true; *hasResultType = true; break;
        case OpRayQueryGetIntersectionObjectRayOriginKHR: *hasResult = true; *hasResultType = true; break;
        case OpRayQueryGetWorldRayDirectionKHR: *hasResult = true; *hasResultType = true; break;
        case OpRayQueryGetWorldRayOriginKHR: *hasResult = true; *hasResultType = true; break;
        case OpRayQueryGetIntersectionObjectToWorldKHR: *hasResult = true; *hasResultType = true; break;
        case OpRayQueryGetIntersectionWorldToObjectKHR: *hasResult = true; *hasResultType = true; break;
        case OpAtomicFAddEXT: *hasResult = true; *hasResultType = true; break;
        case OpTypeBufferSurfaceINTEL: *hasResult = true; *hasResultType = false; break;
        case OpTypeStructContinuedINTEL: *hasResult = false; *hasResultType = false; break;
        case OpConstantCompositeContinuedINTEL: *hasResult = false; *hasResultType = false; break;
        case OpSpecConstantCompositeContinuedINTEL: *hasResult = false; *hasResultType = false; break;
        case OpControlBarrierArriveINTEL: *hasResult = false; *hasResultType = false; break;
        case OpControlBarrierWaitINTEL: *hasResult = false; *hasResultType = false; break;
        case OpGroupIMulKHR: *hasResult = true; *hasResultType = true; break;
        case OpGroupFMulKHR: *hasResult = true; *hasResultType = true; break;
        case OpGroupBitwiseAndKHR: *hasResult = true; *hasResultType = true; break;
        case OpGroupBitwiseOrKHR: *hasResult = true; *hasResultType = true; break;
        case OpGroupBitwiseXorKHR: *hasResult = true; *hasResultType = true; break;
        case OpGroupLogicalAndKHR: *hasResult = true; *hasResultType = true; break;
        case OpGroupLogicalOrKHR: *hasResult = true; *hasResultType = true; break;
        case OpGroupLogicalXorKHR: *hasResult = true; *hasResultType = true; break;
        }
    }
#endif /* SPV_ENABLE_UTILITY_CODE */

    // Overload bitwise operators for mask bit combining

    inline ImageOperandsMask operator|(ImageOperandsMask a, ImageOperandsMask b) { return ImageOperandsMask(unsigned(a) | unsigned(b)); }
    inline ImageOperandsMask operator&(ImageOperandsMask a, ImageOperandsMask b) { return ImageOperandsMask(unsigned(a) & unsigned(b)); }
    inline ImageOperandsMask operator^(ImageOperandsMask a, ImageOperandsMask b) { return ImageOperandsMask(unsigned(a) ^ unsigned(b)); }
    inline ImageOperandsMask operator~(ImageOperandsMask a) { return ImageOperandsMask(~unsigned(a)); }
    inline FPFastMathModeMask operator|(FPFastMathModeMask a, FPFastMathModeMask b) { return FPFastMathModeMask(unsigned(a) | unsigned(b)); }
    inline FPFastMathModeMask operator&(FPFastMathModeMask a, FPFastMathModeMask b) { return FPFastMathModeMask(unsigned(a) & unsigned(b)); }
    inline FPFastMathModeMask operator^(FPFastMathModeMask a, FPFastMathModeMask b) { return FPFastMathModeMask(unsigned(a) ^ unsigned(b)); }
    inline FPFastMathModeMask operator~(FPFastMathModeMask a) { return FPFastMathModeMask(~unsigned(a)); }
    inline SelectionControlMask operator|(SelectionControlMask a, SelectionControlMask b) { return SelectionControlMask(unsigned(a) | unsigned(b)); }
    inline SelectionControlMask operator&(SelectionControlMask a, SelectionControlMask b) { return SelectionControlMask(unsigned(a) & unsigned(b)); }
    inline SelectionControlMask operator^(SelectionControlMask a, SelectionControlMask b) { return SelectionControlMask(unsigned(a) ^ unsigned(b)); }
    inline SelectionControlMask operator~(SelectionControlMask a) { return SelectionControlMask(~unsigned(a)); }
    inline LoopControlMask operator|(LoopControlMask a, LoopControlMask b) { return LoopControlMask(unsigned(a) | unsigned(b)); }
    inline LoopControlMask operator&(LoopControlMask a, LoopControlMask b) { return LoopControlMask(unsigned(a) & unsigned(b)); }
    inline LoopControlMask operator^(LoopControlMask a, LoopControlMask b) { return LoopControlMask(unsigned(a) ^ unsigned(b)); }
    inline LoopControlMask operator~(LoopControlMask a) { return LoopControlMask(~unsigned(a)); }
    inline FunctionControlMask operator|(FunctionControlMask a, FunctionControlMask b) { return FunctionControlMask(unsigned(a) | unsigned(b)); }
    inline FunctionControlMask operator&(FunctionControlMask a, FunctionControlMask b) { return FunctionControlMask(unsigned(a) & unsigned(b)); }
    inline FunctionControlMask operator^(FunctionControlMask a, FunctionControlMask b) { return FunctionControlMask(unsigned(a) ^ unsigned(b)); }
    inline FunctionControlMask operator~(FunctionControlMask a) { return FunctionControlMask(~unsigned(a)); }
    inline MemorySemanticsMask operator|(MemorySemanticsMask a, MemorySemanticsMask b) { return MemorySemanticsMask(unsigned(a) | unsigned(b)); }
    inline MemorySemanticsMask operator&(MemorySemanticsMask a, MemorySemanticsMask b) { return MemorySemanticsMask(unsigned(a) & unsigned(b)); }
    inline MemorySemanticsMask operator^(MemorySemanticsMask a, MemorySemanticsMask b) { return MemorySemanticsMask(unsigned(a) ^ unsigned(b)); }
    inline MemorySemanticsMask operator~(MemorySemanticsMask a) { return MemorySemanticsMask(~unsigned(a)); }
    inline MemoryAccessMask operator|(MemoryAccessMask a, MemoryAccessMask b) { return MemoryAccessMask(unsigned(a) | unsigned(b)); }
    inline MemoryAccessMask operator&(MemoryAccessMask a, MemoryAccessMask b) { return MemoryAccessMask(unsigned(a) & unsigned(b)); }
    inline MemoryAccessMask operator^(MemoryAccessMask a, MemoryAccessMask b) { return MemoryAccessMask(unsigned(a) ^ unsigned(b)); }
    inline MemoryAccessMask operator~(MemoryAccessMask a) { return MemoryAccessMask(~unsigned(a)); }
    inline KernelProfilingInfoMask operator|(KernelProfilingInfoMask a, KernelProfilingInfoMask b) { return KernelProfilingInfoMask(unsigned(a) | unsigned(b)); }
    inline KernelProfilingInfoMask operator&(KernelProfilingInfoMask a, KernelProfilingInfoMask b) { return KernelProfilingInfoMask(unsigned(a) & unsigned(b)); }
    inline KernelProfilingInfoMask operator^(KernelProfilingInfoMask a, KernelProfilingInfoMask b) { return KernelProfilingInfoMask(unsigned(a) ^ unsigned(b)); }
    inline KernelProfilingInfoMask operator~(KernelProfilingInfoMask a) { return KernelProfilingInfoMask(~unsigned(a)); }
    inline RayFlagsMask operator|(RayFlagsMask a, RayFlagsMask b) { return RayFlagsMask(unsigned(a) | unsigned(b)); }
    inline RayFlagsMask operator&(RayFlagsMask a, RayFlagsMask b) { return RayFlagsMask(unsigned(a) & unsigned(b)); }
    inline RayFlagsMask operator^(RayFlagsMask a, RayFlagsMask b) { return RayFlagsMask(unsigned(a) ^ unsigned(b)); }
    inline RayFlagsMask operator~(RayFlagsMask a) { return RayFlagsMask(~unsigned(a)); }
    inline FragmentShadingRateMask operator|(FragmentShadingRateMask a, FragmentShadingRateMask b) { return FragmentShadingRateMask(unsigned(a) | unsigned(b)); }
    inline FragmentShadingRateMask operator&(FragmentShadingRateMask a, FragmentShadingRateMask b) { return FragmentShadingRateMask(unsigned(a) & unsigned(b)); }
    inline FragmentShadingRateMask operator^(FragmentShadingRateMask a, FragmentShadingRateMask b) { return FragmentShadingRateMask(unsigned(a) ^ unsigned(b)); }
    inline FragmentShadingRateMask operator~(FragmentShadingRateMask a) { return FragmentShadingRateMask(~unsigned(a)); }
    inline CooperativeMatrixOperandsMask operator|(CooperativeMatrixOperandsMask a, CooperativeMatrixOperandsMask b) { return CooperativeMatrixOperandsMask(unsigned(a) | unsigned(b)); }
    inline CooperativeMatrixOperandsMask operator&(CooperativeMatrixOperandsMask a, CooperativeMatrixOperandsMask b) { return CooperativeMatrixOperandsMask(unsigned(a) & unsigned(b)); }
    inline CooperativeMatrixOperandsMask operator^(CooperativeMatrixOperandsMask a, CooperativeMatrixOperandsMask b) { return CooperativeMatrixOperandsMask(unsigned(a) ^ unsigned(b)); }
    inline CooperativeMatrixOperandsMask operator~(CooperativeMatrixOperandsMask a) { return CooperativeMatrixOperandsMask(~unsigned(a)); }

}  // end namespace spv

#endif  // #ifndef spirv_HPP

#ifndef spvIR_H
#define spvIR_H


#include <algorithm>
#include <cassert>
#include <functional>
#include <iostream>
#include <memory>
#include <vector>
#include <set>

namespace spv {

    class Block;
    class Function;
    class Module;

    const Id NoResult = 0;
    const Id NoType = 0;

    const Decoration NoPrecision = DecorationMax;

#ifdef __GNUC__
#   define POTENTIALLY_UNUSED __attribute__((unused))
#else
#   define POTENTIALLY_UNUSED
#endif

    POTENTIALLY_UNUSED
        const MemorySemanticsMask MemorySemanticsAllMemory =
        (MemorySemanticsMask)(MemorySemanticsUniformMemoryMask |
            MemorySemanticsWorkgroupMemoryMask |
            MemorySemanticsAtomicCounterMemoryMask |
            MemorySemanticsImageMemoryMask);

    struct IdImmediate {
        bool isId;      // true if word is an Id, false if word is an immediate
        unsigned word;
        IdImmediate(bool i, unsigned w) : isId(i), word(w) {}
    };

    //
    // SPIR-V IR instruction.
    //

    class Instruction {
    public:
        Instruction(Id resultId, Id typeId, Op opCode) : resultId(resultId), typeId(typeId), opCode(opCode), block(nullptr) { }
        explicit Instruction(Op opCode) : resultId(NoResult), typeId(NoType), opCode(opCode), block(nullptr) { }
        virtual ~Instruction() {}
        void addIdOperand(Id id) {
            // ids can't be 0
            assert(id);
            operands.push_back(id);
            idOperand.push_back(true);
        }
        void addImmediateOperand(unsigned int immediate) {
            operands.push_back(immediate);
            idOperand.push_back(false);
        }
        void setImmediateOperand(unsigned idx, unsigned int immediate) {
            assert(!idOperand[idx]);
            operands[idx] = immediate;
        }

        void addStringOperand(const char* str)
        {
            unsigned int word = 0;
            unsigned int shiftAmount = 0;
            char c;

            do {
                c = *(str++);
                word |= ((unsigned int)c) << shiftAmount;
                shiftAmount += 8;
                if (shiftAmount == 32) {
                    addImmediateOperand(word);
                    word = 0;
                    shiftAmount = 0;
                }
            } while (c != 0);

            // deal with partial last word
            if (shiftAmount > 0) {
                addImmediateOperand(word);
            }
        }
        bool isIdOperand(int op) const { return idOperand[op]; }
        void setBlock(Block* b) { block = b; }
        Block* getBlock() const { return block; }
        Op getOpCode() const { return opCode; }
        int getNumOperands() const
        {
            assert(operands.size() == idOperand.size());
            return (int)operands.size();
        }
        Id getResultId() const { return resultId; }
        Id getTypeId() const { return typeId; }
        Id getIdOperand(int op) const {
            assert(idOperand[op]);
            return operands[op];
        }
        unsigned int getImmediateOperand(int op) const {
            assert(!idOperand[op]);
            return operands[op];
        }

        // Write out the binary form.
        void dump(std::vector<unsigned int>& out) const
        {
            // Compute the wordCount
            unsigned int wordCount = 1;
            if (typeId)
                ++wordCount;
            if (resultId)
                ++wordCount;
            wordCount += (unsigned int)operands.size();

            // Write out the beginning of the instruction
            out.push_back(((wordCount) << WordCountShift) | opCode);
            if (typeId)
                out.push_back(typeId);
            if (resultId)
                out.push_back(resultId);

            // Write out the operands
            for (int op = 0; op < (int)operands.size(); ++op)
                out.push_back(operands[op]);
        }

    protected:
        Instruction(const Instruction&);
        Id resultId;
        Id typeId;
        Op opCode;
        std::vector<Id> operands;     // operands, both <id> and immediates (both are unsigned int)
        std::vector<bool> idOperand;  // true for operands that are <id>, false for immediates
        Block* block;
    };

    //
    // SPIR-V IR block.
    //

    class Block {
    public:
        Block(Id id, Function& parent);
        virtual ~Block()
        {
        }

        Id getId() { return instructions.front()->getResultId(); }

        Function& getParent() const { return parent; }
        void addInstruction(std::unique_ptr<Instruction> inst);
        void addPredecessor(Block* pred) { predecessors.push_back(pred); pred->successors.push_back(this); }
        void addLocalVariable(std::unique_ptr<Instruction> inst) { localVariables.push_back(std::move(inst)); }
        const std::vector<Block*>& getPredecessors() const { return predecessors; }
        const std::vector<Block*>& getSuccessors() const { return successors; }
        const std::vector<std::unique_ptr<Instruction> >& getInstructions() const {
            return instructions;
        }
        const std::vector<std::unique_ptr<Instruction> >& getLocalVariables() const { return localVariables; }
        void setUnreachable() { unreachable = true; }
        bool isUnreachable() const { return unreachable; }
        // Returns the block's merge instruction, if one exists (otherwise null).
        const Instruction* getMergeInstruction() const {
            if (instructions.size() < 2) return nullptr;
            const Instruction* nextToLast = (instructions.cend() - 2)->get();
            switch (nextToLast->getOpCode()) {
            case OpSelectionMerge:
            case OpLoopMerge:
                return nextToLast;
            default:
                return nullptr;
            }
            return nullptr;
        }

        // Change this block into a canonical dead merge block.  Delete instructions
        // as necessary.  A canonical dead merge block has only an OpLabel and an
        // OpUnreachable.
        void rewriteAsCanonicalUnreachableMerge() {
            assert(localVariables.empty());
            // Delete all instructions except for the label.
            assert(instructions.size() > 0);
            instructions.resize(1);
            successors.clear();
            addInstruction(std::unique_ptr<Instruction>(new Instruction(OpUnreachable)));
        }
        // Change this block into a canonical dead continue target branching to the
        // given header ID.  Delete instructions as necessary.  A canonical dead continue
        // target has only an OpLabel and an unconditional branch back to the corresponding
        // header.
        void rewriteAsCanonicalUnreachableContinue(Block* header) {
            assert(localVariables.empty());
            // Delete all instructions except for the label.
            assert(instructions.size() > 0);
            instructions.resize(1);
            successors.clear();
            // Add OpBranch back to the header.
            assert(header != nullptr);
            Instruction* branch = new Instruction(OpBranch);
            branch->addIdOperand(header->getId());
            addInstruction(std::unique_ptr<Instruction>(branch));
            successors.push_back(header);
        }

        bool isTerminated() const
        {
            switch (instructions.back()->getOpCode()) {
            case OpBranch:
            case OpBranchConditional:
            case OpSwitch:
            case OpKill:
            case OpTerminateInvocation:
            case OpReturn:
            case OpReturnValue:
            case OpUnreachable:
                return true;
            default:
                return false;
            }
        }

        void dump(std::vector<unsigned int>& out) const
        {
            instructions[0]->dump(out);
            for (int i = 0; i < (int)localVariables.size(); ++i)
                localVariables[i]->dump(out);
            for (int i = 1; i < (int)instructions.size(); ++i)
                instructions[i]->dump(out);
        }

    protected:
        Block(const Block&);
        Block& operator=(Block&);

        // To enforce keeping parent and ownership in sync:
        friend Function;

        std::vector<std::unique_ptr<Instruction> > instructions;
        std::vector<Block*> predecessors, successors;
        std::vector<std::unique_ptr<Instruction> > localVariables;
        Function& parent;

        // track whether this block is known to be uncreachable (not necessarily
        // true for all unreachable blocks, but should be set at least
        // for the extraneous ones introduced by the builder).
        bool unreachable;
    };

    // The different reasons for reaching a block in the inReadableOrder traversal.
    enum ReachReason {
        // Reachable from the entry block via transfers of control, i.e. branches.
        ReachViaControlFlow = 0,
        // A continue target that is not reachable via control flow.
        ReachDeadContinue,
        // A merge block that is not reachable via control flow.
        ReachDeadMerge
    };

    // Traverses the control-flow graph rooted at root in an order suited for
    // readable code generation.  Invokes callback at every node in the traversal
    // order.  The callback arguments are:
    // - the block,
    // - the reason we reached the block,
    // - if the reason was that block is an unreachable continue or unreachable merge block
    //   then the last parameter is the corresponding header block.
    void inReadableOrder(Block* root, std::function<void(Block*, ReachReason, Block* header)> callback);

    //
    // SPIR-V IR Function.
    //

    class Function {
    public:
        Function(Id id, Id resultType, Id functionType, Id firstParam, LinkageType linkage, const std::string& name, Module& parent);
        virtual ~Function()
        {
            for (int i = 0; i < (int)parameterInstructions.size(); ++i)
                delete parameterInstructions[i];

            for (int i = 0; i < (int)blocks.size(); ++i)
                delete blocks[i];
        }
        Id getId() const { return functionInstruction.getResultId(); }
        Id getParamId(int p) const { return parameterInstructions[p]->getResultId(); }
        Id getParamType(int p) const { return parameterInstructions[p]->getTypeId(); }

        void addBlock(Block* block) { blocks.push_back(block); }
        void removeBlock(Block* block)
        {
            auto found = find(blocks.begin(), blocks.end(), block);
            assert(found != blocks.end());
            blocks.erase(found);
            delete block;
        }

        Module& getParent() const { return parent; }
        Block* getEntryBlock() const { return blocks.front(); }
        Block* getLastBlock() const { return blocks.back(); }
        const std::vector<Block*>& getBlocks() const { return blocks; }
        void addLocalVariable(std::unique_ptr<Instruction> inst);
        Id getReturnType() const { return functionInstruction.getTypeId(); }
        Id getFuncId() const { return functionInstruction.getResultId(); }
        void setReturnPrecision(Decoration precision)
        {
            if (precision == DecorationRelaxedPrecision)
                reducedPrecisionReturn = true;
        }
        Decoration getReturnPrecision() const
        {
            return reducedPrecisionReturn ? DecorationRelaxedPrecision : NoPrecision;
        }

        void setDebugLineInfo(Id fileName, int line, int column) {
            lineInstruction = std::unique_ptr<Instruction>{ new Instruction(OpLine) };
            lineInstruction->addIdOperand(fileName);
            lineInstruction->addImmediateOperand(line);
            lineInstruction->addImmediateOperand(column);
        }
        bool hasDebugLineInfo() const { return lineInstruction != nullptr; }

        void setImplicitThis() { implicitThis = true; }
        bool hasImplicitThis() const { return implicitThis; }

        void addParamPrecision(unsigned param, Decoration precision)
        {
            if (precision == DecorationRelaxedPrecision)
                reducedPrecisionParams.insert(param);
        }
        Decoration getParamPrecision(unsigned param) const
        {
            return reducedPrecisionParams.find(param) != reducedPrecisionParams.end() ?
                DecorationRelaxedPrecision : NoPrecision;
        }

        void dump(std::vector<unsigned int>& out) const
        {
            // OpLine
            if (lineInstruction != nullptr) {
                lineInstruction->dump(out);
            }

            // OpFunction
            functionInstruction.dump(out);

            // OpFunctionParameter
            for (int p = 0; p < (int)parameterInstructions.size(); ++p)
                parameterInstructions[p]->dump(out);

            // Blocks
            inReadableOrder(blocks[0], [&out](const Block* b, ReachReason, Block*) { b->dump(out); });
            Instruction end(0, 0, OpFunctionEnd);
            end.dump(out);
        }

        LinkageType getLinkType() const { return linkType; }
        const char* getExportName() const { return exportName.c_str(); }

    protected:
        Function(const Function&);
        Function& operator=(Function&);

        Module& parent;
        std::unique_ptr<Instruction> lineInstruction;
        Instruction functionInstruction;
        std::vector<Instruction*> parameterInstructions;
        std::vector<Block*> blocks;
        bool implicitThis;  // true if this is a member function expecting to be passed a 'this' as the first argument
        bool reducedPrecisionReturn;
        std::set<int> reducedPrecisionParams;  // list of parameter indexes that need a relaxed precision arg
        LinkageType linkType;
        std::string exportName;
    };

    //
    // SPIR-V IR Module.
    //

    class Module {
    public:
        Module() {}
        virtual ~Module()
        {
            // TODO delete things
        }

        void addFunction(Function* fun) { functions.push_back(fun); }

        void mapInstruction(Instruction* instruction)
        {
            spv::Id resultId = instruction->getResultId();
            // map the instruction's result id
            if (resultId >= idToInstruction.size())
                idToInstruction.resize(resultId + 16);
            idToInstruction[resultId] = instruction;
        }

        Instruction* getInstruction(Id id) const { return idToInstruction[id]; }
        const std::vector<Function*>& getFunctions() const { return functions; }
        spv::Id getTypeId(Id resultId) const {
            return idToInstruction[resultId] == nullptr ? NoType : idToInstruction[resultId]->getTypeId();
        }
        StorageClass getStorageClass(Id typeId) const
        {
            assert(idToInstruction[typeId]->getOpCode() == spv::OpTypePointer);
            return (StorageClass)idToInstruction[typeId]->getImmediateOperand(0);
        }

        void dump(std::vector<unsigned int>& out) const
        {
            for (int f = 0; f < (int)functions.size(); ++f)
                functions[f]->dump(out);
        }

    protected:
        Module(const Module&);
        std::vector<Function*> functions;

        // map from result id to instruction having that result id
        std::vector<Instruction*> idToInstruction;

        // map from a result id to its type id
    };

    //
    // Implementation (it's here due to circular type definitions).
    //

    // Add both
    // - the OpFunction instruction
    // - all the OpFunctionParameter instructions
    __inline Function::Function(Id id, Id resultType, Id functionType, Id firstParamId, LinkageType linkage, const std::string& name, Module& parent)
        : parent(parent), lineInstruction(nullptr),
        functionInstruction(id, resultType, OpFunction), implicitThis(false),
        reducedPrecisionReturn(false),
        linkType(linkage)
    {
        // OpFunction
        functionInstruction.addImmediateOperand(FunctionControlMaskNone);
        functionInstruction.addIdOperand(functionType);
        parent.mapInstruction(&functionInstruction);
        parent.addFunction(this);

        // OpFunctionParameter
        Instruction* typeInst = parent.getInstruction(functionType);
        int numParams = typeInst->getNumOperands() - 1;
        for (int p = 0; p < numParams; ++p) {
            Instruction* param = new Instruction(firstParamId + p, typeInst->getIdOperand(p + 1), OpFunctionParameter);
            parent.mapInstruction(param);
            parameterInstructions.push_back(param);
        }

        // If importing/exporting, save the function name (without the mangled parameters) for the linkage decoration
        if (linkType != LinkageTypeMax) {
            exportName = name.substr(0, name.find_first_of('('));
        }
    }

    __inline void Function::addLocalVariable(std::unique_ptr<Instruction> inst)
    {
        Instruction* raw_instruction = inst.get();
        blocks[0]->addLocalVariable(std::move(inst));
        parent.mapInstruction(raw_instruction);
    }

    __inline Block::Block(Id id, Function& parent) : parent(parent), unreachable(false)
    {
        instructions.push_back(std::unique_ptr<Instruction>(new Instruction(id, NoType, OpLabel)));
        instructions.back()->setBlock(this);
        parent.getParent().mapInstruction(instructions.back().get());
    }

    __inline void Block::addInstruction(std::unique_ptr<Instruction> inst)
    {
        Instruction* raw_instruction = inst.get();
        instructions.push_back(std::move(inst));
        raw_instruction->setBlock(this);
        if (raw_instruction->getResultId())
            parent.getParent().mapInstruction(raw_instruction);
    }

}  // end spv namespace

#endif // spvIR_H


namespace spv {

    // class to hold SPIR-V binary data for remapping, DCE, and debug stripping
    class spirvbin_t : public spirvbin_base_t
    {
    public:
        spirvbin_t(int verbose = 0) : entryPoint(spv::NoResult), largestNewId(0), verbose(verbose), errorLatch(false)
        { }

        virtual ~spirvbin_t() { }

        // remap on an existing binary in memory
        void remap(std::vector<std::uint32_t>& spv, const std::vector<std::string>& whiteListStrings,
            std::uint32_t opts = DO_EVERYTHING);

        // remap on an existing binary in memory - legacy interface without white list
        void remap(std::vector<std::uint32_t>& spv, std::uint32_t opts = DO_EVERYTHING);

        // Type for error/log handler functions
        typedef std::function<void(const std::string&)> errorfn_t;
        typedef std::function<void(const std::string&)> logfn_t;

        // Register error/log handling functions (can be lambda fn / functor / etc)
        static void registerErrorHandler(errorfn_t handler) { errorHandler = handler; }
        static void registerLogHandler(logfn_t handler) { logHandler = handler; }

    protected:
        // This can be overridden to provide other message behavior if needed
        virtual void msg(int minVerbosity, int indent, const std::string& txt) const;

    private:
        // Local to global, or global to local ID map
        typedef std::unordered_map<spv::Id, spv::Id> idmap_t;
        typedef std::unordered_set<spv::Id>          idset_t;
        typedef std::unordered_map<spv::Id, int>     blockmap_t;

        void remap(std::uint32_t opts = DO_EVERYTHING);

        // Map of names to IDs
        typedef std::unordered_map<std::string, spv::Id> namemap_t;

        typedef std::uint32_t spirword_t;

        typedef std::pair<unsigned, unsigned> range_t;
        typedef std::function<void(spv::Id&)>                idfn_t;
        typedef std::function<bool(spv::Op, unsigned start)> instfn_t;

        // Special Values for ID map:
        static const spv::Id unmapped;     // unchanged from default value
        static const spv::Id unused;       // unused ID
        static const int     header_size;  // SPIR header = 5 words

        class id_iterator_t;

        // For mapping type entries between different shaders
        typedef std::vector<spirword_t>        typeentry_t;
        typedef std::map<spv::Id, typeentry_t> globaltypes_t;

        // A set that preserves position order, and a reverse map
        typedef std::set<int>                    posmap_t;
        typedef std::unordered_map<spv::Id, int> posmap_rev_t;

        // Maps and ID to the size of its base type, if known.
        typedef std::unordered_map<spv::Id, unsigned> typesize_map_t;

        // handle error
        void error(const std::string& txt) const { errorLatch = true; errorHandler(txt); }

        bool     isConstOp(spv::Op opCode)      const;
        bool     isTypeOp(spv::Op opCode)       const;
        bool     isStripOp(spv::Op opCode)      const;
        bool     isFlowCtrl(spv::Op opCode)     const;
        range_t  literalRange(spv::Op opCode)   const;
        range_t  typeRange(spv::Op opCode)      const;
        range_t  constRange(spv::Op opCode)     const;
        unsigned typeSizeInWords(spv::Id id)    const;
        unsigned idTypeSizeInWords(spv::Id id)  const;

        bool isStripOp(spv::Op opCode, unsigned start) const;

        spv::Id& asId(unsigned word) { return spv[word]; }
        const spv::Id& asId(unsigned word)          const { return spv[word]; }
        spv::Op         asOpCode(unsigned word)      const { return opOpCode(spv[word]); }
        std::uint32_t   asOpCodeHash(unsigned word);
        spv::Decoration asDecoration(unsigned word)  const { return spv::Decoration(spv[word]); }
        unsigned        asWordCount(unsigned word)   const { return opWordCount(spv[word]); }
        spv::Id         asTypeConstId(unsigned word) const { return asId(word + (isTypeOp(asOpCode(word)) ? 1 : 2)); }
        unsigned        idPos(spv::Id id)            const;

        static unsigned opWordCount(spirword_t data) { return data >> spv::WordCountShift; }
        static spv::Op  opOpCode(spirword_t data) { return spv::Op(data & spv::OpCodeMask); }

        // Header access & set methods
        spirword_t  magic()    const { return spv[0]; } // return magic number
        spirword_t  bound()    const { return spv[3]; } // return Id bound from header
        spirword_t  bound(spirword_t b) { return spv[3] = b; }
        spirword_t  genmagic() const { return spv[2]; } // generator magic
        spirword_t  genmagic(spirword_t m) { return spv[2] = m; }
        spirword_t  schemaNum() const { return spv[4]; } // schema number from header

        // Mapping fns: get
        spv::Id     localId(spv::Id id) const { return idMapL[id]; }

        // Mapping fns: set
        inline spv::Id   localId(spv::Id id, spv::Id newId);
        void             countIds(spv::Id id);

        // Return next unused new local ID.
        // NOTE: boost::dynamic_bitset would be more efficient due to find_next(),
        // which std::vector<bool> doens't have.
        inline spv::Id   nextUnusedId(spv::Id id);

        void buildLocalMaps();
        std::string literalString(unsigned word) const; // Return literal as a std::string
        int literalStringWords(const std::string& str) const { return (int(str.size()) + 4) / 4; }

        bool isNewIdMapped(spv::Id newId)   const { return isMapped(newId); }
        bool isOldIdUnmapped(spv::Id oldId) const { return localId(oldId) == unmapped; }
        bool isOldIdUnused(spv::Id oldId)   const { return localId(oldId) == unused; }
        bool isOldIdMapped(spv::Id oldId)   const { return !isOldIdUnused(oldId) && !isOldIdUnmapped(oldId); }
        bool isFunction(spv::Id oldId)      const { return fnPos.find(oldId) != fnPos.end(); }

        // bool    matchType(const globaltypes_t& globalTypes, spv::Id lt, spv::Id gt) const;
        // spv::Id findType(const globaltypes_t& globalTypes, spv::Id lt) const;
        std::uint32_t hashType(unsigned typeStart) const;

        spirvbin_t& process(instfn_t, idfn_t, unsigned begin = 0, unsigned end = 0);
        int         processInstruction(unsigned word, instfn_t, idfn_t);

        void        validate() const;
        void        mapTypeConst();
        void        mapFnBodies();
        void        optLoadStore();
        void        dceFuncs();
        void        dceVars();
        void        dceTypes();
        void        mapNames();
        void        foldIds();  // fold IDs to smallest space
        void        forwardLoadStores(); // load store forwarding (EXPERIMENTAL)
        void        offsetIds(); // create relative offset IDs

        void        applyMap();            // remap per local name map
        void        mapRemainder();        // map any IDs we haven't touched yet
        void        stripDebug();          // strip all debug info
        void        stripDeadRefs();       // strips debug info for now-dead references after DCE
        void        strip();               // remove debug symbols

        std::vector<spirword_t> spv;      // SPIR words

        std::vector<std::string> stripWhiteList;

        namemap_t               nameMap;  // ID names from OpName

        // Since we want to also do binary ops, we can't use std::vector<bool>.  we could use
        // boost::dynamic_bitset, but we're trying to avoid a boost dependency.
        typedef std::uint64_t bits_t;
        std::vector<bits_t> mapped; // which new IDs have been mapped
        static const int mBits = sizeof(bits_t) * 4;

        bool isMapped(spv::Id id) const { return id < maxMappedId() && ((mapped[id / mBits] & (1LL << (id % mBits))) != 0); }
        void setMapped(spv::Id id) { resizeMapped(id); mapped[id / mBits] |= (1LL << (id % mBits)); }
        void resizeMapped(spv::Id id) { if (id >= maxMappedId()) mapped.resize(id / mBits + 1, 0); }
        size_t maxMappedId() const { return mapped.size() * mBits; }

        // Add a strip range for a given instruction starting at 'start'
        // Note: avoiding brace initializers to please older versions os MSVC.
        void stripInst(unsigned start) { stripRange.push_back(range_t(start, start + asWordCount(start))); }

        // Function start and end.  use unordered_map because we'll have
        // many fewer functions than IDs.
        std::unordered_map<spv::Id, range_t> fnPos;

        // Which functions are called, anywhere in the module, with a call count
        std::unordered_map<spv::Id, int> fnCalls;

        posmap_t       typeConstPos;  // word positions that define types & consts (ordered)
        posmap_rev_t   idPosR;        // reverse map from IDs to positions
        typesize_map_t idTypeSizeMap; // maps each ID to its type size, if known.

        std::vector<spv::Id>  idMapL;   // ID {M}ap from {L}ocal to {G}lobal IDs

        spv::Id entryPoint;      // module entry point
        spv::Id largestNewId;    // biggest new ID we have mapped anything to

        // Sections of the binary to strip, given as [begin,end)
        std::vector<range_t> stripRange;

        // processing options:
        std::uint32_t options;
        int           verbose;     // verbosity level

        // Error latch: this is set if the error handler is ever executed.  It would be better to
        // use a try/catch block and throw, but that's not desired for certain environments, so
        // this is the alternative.
        mutable bool errorLatch;

        static errorfn_t errorHandler;
        static logfn_t   logHandler;
    };

} // namespace SPV

#endif // SPIRVREMAPPER_H

#ifndef INCLUDE_SPIRV_TOOLS_LIBSPIRV_HPP_
#define INCLUDE_SPIRV_TOOLS_LIBSPIRV_HPP_

#include <functional>
#include <memory>
#include <string>
#include <vector>

#include "spirv-tools/libspirv.h"

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


#ifndef SPIRV_CROSS_GLSL_HPP
#define SPIRV_CROSS_GLSL_HPP

#ifndef GLSLstd450_H
#define GLSLstd450_H

static const int GLSLstd450Version = 100;
static const int GLSLstd450Revision = 1;

enum GLSLstd450 {
    GLSLstd450Bad = 0,              // Don't use

    GLSLstd450Round = 1,
    GLSLstd450RoundEven = 2,
    GLSLstd450Trunc = 3,
    GLSLstd450FAbs = 4,
    GLSLstd450SAbs = 5,
    GLSLstd450FSign = 6,
    GLSLstd450SSign = 7,
    GLSLstd450Floor = 8,
    GLSLstd450Ceil = 9,
    GLSLstd450Fract = 10,

    GLSLstd450Radians = 11,
    GLSLstd450Degrees = 12,
    GLSLstd450Sin = 13,
    GLSLstd450Cos = 14,
    GLSLstd450Tan = 15,
    GLSLstd450Asin = 16,
    GLSLstd450Acos = 17,
    GLSLstd450Atan = 18,
    GLSLstd450Sinh = 19,
    GLSLstd450Cosh = 20,
    GLSLstd450Tanh = 21,
    GLSLstd450Asinh = 22,
    GLSLstd450Acosh = 23,
    GLSLstd450Atanh = 24,
    GLSLstd450Atan2 = 25,

    GLSLstd450Pow = 26,
    GLSLstd450Exp = 27,
    GLSLstd450Log = 28,
    GLSLstd450Exp2 = 29,
    GLSLstd450Log2 = 30,
    GLSLstd450Sqrt = 31,
    GLSLstd450InverseSqrt = 32,

    GLSLstd450Determinant = 33,
    GLSLstd450MatrixInverse = 34,

    GLSLstd450Modf = 35,            // second operand needs an OpVariable to write to
    GLSLstd450ModfStruct = 36,      // no OpVariable operand
    GLSLstd450FMin = 37,
    GLSLstd450UMin = 38,
    GLSLstd450SMin = 39,
    GLSLstd450FMax = 40,
    GLSLstd450UMax = 41,
    GLSLstd450SMax = 42,
    GLSLstd450FClamp = 43,
    GLSLstd450UClamp = 44,
    GLSLstd450SClamp = 45,
    GLSLstd450FMix = 46,
    GLSLstd450IMix = 47,            // Reserved
    GLSLstd450Step = 48,
    GLSLstd450SmoothStep = 49,

    GLSLstd450Fma = 50,
    GLSLstd450Frexp = 51,            // second operand needs an OpVariable to write to
    GLSLstd450FrexpStruct = 52,      // no OpVariable operand
    GLSLstd450Ldexp = 53,

    GLSLstd450PackSnorm4x8 = 54,
    GLSLstd450PackUnorm4x8 = 55,
    GLSLstd450PackSnorm2x16 = 56,
    GLSLstd450PackUnorm2x16 = 57,
    GLSLstd450PackHalf2x16 = 58,
    GLSLstd450PackDouble2x32 = 59,
    GLSLstd450UnpackSnorm2x16 = 60,
    GLSLstd450UnpackUnorm2x16 = 61,
    GLSLstd450UnpackHalf2x16 = 62,
    GLSLstd450UnpackSnorm4x8 = 63,
    GLSLstd450UnpackUnorm4x8 = 64,
    GLSLstd450UnpackDouble2x32 = 65,

    GLSLstd450Length = 66,
    GLSLstd450Distance = 67,
    GLSLstd450Cross = 68,
    GLSLstd450Normalize = 69,
    GLSLstd450FaceForward = 70,
    GLSLstd450Reflect = 71,
    GLSLstd450Refract = 72,

    GLSLstd450FindILsb = 73,
    GLSLstd450FindSMsb = 74,
    GLSLstd450FindUMsb = 75,

    GLSLstd450InterpolateAtCentroid = 76,
    GLSLstd450InterpolateAtSample = 77,
    GLSLstd450InterpolateAtOffset = 78,

    GLSLstd450NMin = 79,
    GLSLstd450NMax = 80,
    GLSLstd450NClamp = 81,

    GLSLstd450Count
};

#endif  // #ifndef GLSLstd450_H

#ifndef SPIRV_CROSS_HPP
#define SPIRV_CROSS_HPP

#ifndef SPV_ENABLE_UTILITY_CODE
#define SPV_ENABLE_UTILITY_CODE
#endif

#ifndef SPIRV_CROSS_CFG_HPP
#define SPIRV_CROSS_CFG_HPP

#ifndef SPIRV_CROSS_COMMON_HPP
#define SPIRV_CROSS_COMMON_HPP

#ifndef SPV_ENABLE_UTILITY_CODE
#define SPV_ENABLE_UTILITY_CODE
#endif


#ifndef SPIRV_CROSS_CONTAINERS_HPP
#define SPIRV_CROSS_CONTAINERS_HPP

#ifndef SPIRV_CROSS_ERROR_HANDLING
#define SPIRV_CROSS_ERROR_HANDLING

#include <stdio.h>
#include <stdlib.h>
#include <string>
#ifndef SPIRV_CROSS_EXCEPTIONS_TO_ASSERTIONS
#include <stdexcept>
#endif

#ifdef SPIRV_CROSS_NAMESPACE_OVERRIDE
#define SPIRV_CROSS_NAMESPACE SPIRV_CROSS_NAMESPACE_OVERRIDE
#else
#define SPIRV_CROSS_NAMESPACE spirv_cross
#endif

namespace SPIRV_CROSS_NAMESPACE
{
#ifdef SPIRV_CROSS_EXCEPTIONS_TO_ASSERTIONS
#if !defined(_MSC_VER) || defined(__clang__)
    [[noreturn]]
#elif defined(_MSC_VER)
    __declspec(noreturn)
#endif
        inline void
        report_and_abort(const std::string& msg)
    {
#ifdef NDEBUG
        (void)msg;
#else
        fprintf(stderr, "There was a compiler error: %s\n", msg.c_str());
#endif
        fflush(stderr);
        abort();
    }

#define SPIRV_CROSS_THROW(x) report_and_abort(x)
#else
    class CompilerError : public std::runtime_error
    {
    public:
        explicit CompilerError(const std::string& str)
            : std::runtime_error(str)
        {
        }
    };

#define SPIRV_CROSS_THROW(x) throw CompilerError(x)
#endif

    // MSVC 2013 does not have noexcept. We need this for Variant to get move constructor to work correctly
    // instead of copy constructor.
    // MSVC 2013 ignores that move constructors cannot throw in std::vector, so just don't define it.
#if defined(_MSC_VER) && _MSC_VER < 1900
#define SPIRV_CROSS_NOEXCEPT
#else
#define SPIRV_CROSS_NOEXCEPT noexcept
#endif

#if __cplusplus >= 201402l
#define SPIRV_CROSS_DEPRECATED(reason) [[deprecated(reason)]]
#elif defined(__GNUC__)
#define SPIRV_CROSS_DEPRECATED(reason) __attribute__((deprecated))
#elif defined(_MSC_VER)
#define SPIRV_CROSS_DEPRECATED(reason) __declspec(deprecated(reason))
#else
#define SPIRV_CROSS_DEPRECATED(reason)
#endif
} // namespace SPIRV_CROSS_NAMESPACE

#endif

#include <algorithm>
#include <exception>
#include <functional>
#include <iterator>
#include <limits>
#include <memory>
#include <stack>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <type_traits>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

#ifdef SPIRV_CROSS_NAMESPACE_OVERRIDE
#define SPIRV_CROSS_NAMESPACE SPIRV_CROSS_NAMESPACE_OVERRIDE
#else
#define SPIRV_CROSS_NAMESPACE spirv_cross
#endif

namespace SPIRV_CROSS_NAMESPACE
{
#ifndef SPIRV_CROSS_FORCE_STL_TYPES
    // std::aligned_storage does not support size == 0, so roll our own.
    template <typename T, size_t N>
    class AlignedBuffer
    {
    public:
        T* data()
        {
#if defined(_MSC_VER) && _MSC_VER < 1900
            // MSVC 2013 workarounds, sigh ...
            // Only use this workaround on MSVC 2013 due to some confusion around default initialized unions.
            // Spec seems to suggest the memory will be zero-initialized, which is *not* what we want.
            return reinterpret_cast<T*>(u.aligned_char);
#else
            return reinterpret_cast<T*>(aligned_char);
#endif
        }

    private:
#if defined(_MSC_VER) && _MSC_VER < 1900
        // MSVC 2013 workarounds, sigh ...
        union
        {
            char aligned_char[sizeof(T) * N];
            double dummy_aligner;
        } u;
#else
        alignas(T) char aligned_char[sizeof(T) * N];
#endif
    };

    template <typename T>
    class AlignedBuffer<T, 0>
    {
    public:
        T* data()
        {
            return nullptr;
        }
    };

    // An immutable version of SmallVector which erases type information about storage.
    template <typename T>
    class VectorView
    {
    public:
        T& operator[](size_t i) SPIRV_CROSS_NOEXCEPT
        {
            return ptr[i];
        }

        const T& operator[](size_t i) const SPIRV_CROSS_NOEXCEPT
        {
            return ptr[i];
        }

        bool empty() const SPIRV_CROSS_NOEXCEPT
        {
            return buffer_size == 0;
        }

        size_t size() const SPIRV_CROSS_NOEXCEPT
        {
            return buffer_size;
        }

        T* data() SPIRV_CROSS_NOEXCEPT
        {
            return ptr;
        }

        const T* data() const SPIRV_CROSS_NOEXCEPT
        {
            return ptr;
        }

        T* begin() SPIRV_CROSS_NOEXCEPT
        {
            return ptr;
        }

        T* end() SPIRV_CROSS_NOEXCEPT
        {
            return ptr + buffer_size;
        }

        const T* begin() const SPIRV_CROSS_NOEXCEPT
        {
            return ptr;
        }

        const T* end() const SPIRV_CROSS_NOEXCEPT
        {
            return ptr + buffer_size;
        }

        T& front() SPIRV_CROSS_NOEXCEPT
        {
            return ptr[0];
        }

        const T& front() const SPIRV_CROSS_NOEXCEPT
        {
            return ptr[0];
        }

        T& back() SPIRV_CROSS_NOEXCEPT
        {
            return ptr[buffer_size - 1];
        }

        const T& back() const SPIRV_CROSS_NOEXCEPT
        {
            return ptr[buffer_size - 1];
        }

        // Makes it easier to consume SmallVector.
#if defined(_MSC_VER) && _MSC_VER < 1900
        explicit operator std::vector<T>() const
        {
            // Another MSVC 2013 workaround. It does not understand lvalue/rvalue qualified operations.
            return std::vector<T>(ptr, ptr + buffer_size);
        }
#else
    // Makes it easier to consume SmallVector.
        explicit operator std::vector<T>() const&
        {
            return std::vector<T>(ptr, ptr + buffer_size);
        }

        // If we are converting as an r-value, we can pilfer our elements.
        explicit operator std::vector<T>()&&
        {
            return std::vector<T>(std::make_move_iterator(ptr), std::make_move_iterator(ptr + buffer_size));
        }
#endif

        // Avoid sliced copies. Base class should only be read as a reference.
        VectorView(const VectorView&) = delete;
        void operator=(const VectorView&) = delete;

    protected:
        VectorView() = default;
        T* ptr = nullptr;
        size_t buffer_size = 0;
    };

    // Simple vector which supports up to N elements inline, without malloc/free.
    // We use a lot of throwaway vectors all over the place which triggers allocations.
    // This class only implements the subset of std::vector we need in SPIRV-Cross.
    // It is *NOT* a drop-in replacement in general projects.
    template <typename T, size_t N = 8>
    class SmallVector : public VectorView<T>
    {
    public:
        SmallVector() SPIRV_CROSS_NOEXCEPT
        {
            this->ptr = stack_storage.data();
            buffer_capacity = N;
        }

        template <typename U>
        SmallVector(const U* arg_list_begin, const U* arg_list_end) SPIRV_CROSS_NOEXCEPT : SmallVector()
        {
            auto count = size_t(arg_list_end - arg_list_begin);
            reserve(count);
            for (size_t i = 0; i < count; i++, arg_list_begin++)
                new (&this->ptr[i]) T(*arg_list_begin);
            this->buffer_size = count;
        }

        template <typename U>
        SmallVector(std::initializer_list<U> init) SPIRV_CROSS_NOEXCEPT : SmallVector(init.begin(), init.end())
        {
        }

        template <typename U, size_t M>
        explicit SmallVector(const U(&init)[M]) SPIRV_CROSS_NOEXCEPT : SmallVector(init, init + M)
        {
        }

        SmallVector(SmallVector&& other) SPIRV_CROSS_NOEXCEPT : SmallVector()
        {
            *this = std::move(other);
        }

        SmallVector& operator=(SmallVector&& other) SPIRV_CROSS_NOEXCEPT
        {
            clear();
            if (other.ptr != other.stack_storage.data())
            {
                // Pilfer allocated pointer.
                if (this->ptr != stack_storage.data())
                    free(this->ptr);
                this->ptr = other.ptr;
                this->buffer_size = other.buffer_size;
                buffer_capacity = other.buffer_capacity;
                other.ptr = nullptr;
                other.buffer_size = 0;
                other.buffer_capacity = 0;
            }
            else
            {
                // Need to move the stack contents individually.
                reserve(other.buffer_size);
                for (size_t i = 0; i < other.buffer_size; i++)
                {
                    new (&this->ptr[i]) T(std::move(other.ptr[i]));
                    other.ptr[i].~T();
                }
                this->buffer_size = other.buffer_size;
                other.buffer_size = 0;
            }
            return *this;
        }

        SmallVector(const SmallVector& other) SPIRV_CROSS_NOEXCEPT : SmallVector()
        {
            *this = other;
        }

        SmallVector& operator=(const SmallVector& other) SPIRV_CROSS_NOEXCEPT
        {
            if (this == &other)
                return *this;

            clear();
            reserve(other.buffer_size);
            for (size_t i = 0; i < other.buffer_size; i++)
                new (&this->ptr[i]) T(other.ptr[i]);
            this->buffer_size = other.buffer_size;
            return *this;
        }

        explicit SmallVector(size_t count) SPIRV_CROSS_NOEXCEPT : SmallVector()
        {
            resize(count);
        }

        ~SmallVector()
        {
            clear();
            if (this->ptr != stack_storage.data())
                free(this->ptr);
        }

        void clear() SPIRV_CROSS_NOEXCEPT
        {
            for (size_t i = 0; i < this->buffer_size; i++)
                this->ptr[i].~T();
            this->buffer_size = 0;
        }

        void push_back(const T& t) SPIRV_CROSS_NOEXCEPT
        {
            reserve(this->buffer_size + 1);
            new (&this->ptr[this->buffer_size]) T(t);
            this->buffer_size++;
        }

        void push_back(T&& t) SPIRV_CROSS_NOEXCEPT
        {
            reserve(this->buffer_size + 1);
            new (&this->ptr[this->buffer_size]) T(std::move(t));
            this->buffer_size++;
        }

        void pop_back() SPIRV_CROSS_NOEXCEPT
        {
            // Work around false positive warning on GCC 8.3.
            // Calling pop_back on empty vector is undefined.
            if (!this->empty())
                resize(this->buffer_size - 1);
        }

        template <typename... Ts>
        void emplace_back(Ts &&... ts) SPIRV_CROSS_NOEXCEPT
        {
            reserve(this->buffer_size + 1);
            new (&this->ptr[this->buffer_size]) T(std::forward<Ts>(ts)...);
            this->buffer_size++;
        }

        void reserve(size_t count) SPIRV_CROSS_NOEXCEPT
        {
            if ((count > (std::numeric_limits<size_t>::max)() / sizeof(T)) ||
                (count > (std::numeric_limits<size_t>::max)() / 2))
            {
                // Only way this should ever happen is with garbage input, terminate.
                std::terminate();
            }

            if (count > buffer_capacity)
            {
                size_t target_capacity = buffer_capacity;
                if (target_capacity == 0)
                    target_capacity = 1;

                // Weird parens works around macro issues on Windows if NOMINMAX is not used.
                target_capacity = (std::max)(target_capacity, N);

                // Need to ensure there is a POT value of target capacity which is larger than count,
                // otherwise this will overflow.
                while (target_capacity < count)
                    target_capacity <<= 1u;

                T* new_buffer =
                    target_capacity > N ? static_cast<T*>(malloc(target_capacity * sizeof(T))) : stack_storage.data();

                // If we actually fail this malloc, we are hosed anyways, there is no reason to attempt recovery.
                if (!new_buffer)
                    std::terminate();

                // In case for some reason two allocations both come from same stack.
                if (new_buffer != this->ptr)
                {
                    // We don't deal with types which can throw in move constructor.
                    for (size_t i = 0; i < this->buffer_size; i++)
                    {
                        new (&new_buffer[i]) T(std::move(this->ptr[i]));
                        this->ptr[i].~T();
                    }
                }

                if (this->ptr != stack_storage.data())
                    free(this->ptr);
                this->ptr = new_buffer;
                buffer_capacity = target_capacity;
            }
        }

        void insert(T* itr, const T* insert_begin, const T* insert_end) SPIRV_CROSS_NOEXCEPT
        {
            auto count = size_t(insert_end - insert_begin);
            if (itr == this->end())
            {
                reserve(this->buffer_size + count);
                for (size_t i = 0; i < count; i++, insert_begin++)
                    new (&this->ptr[this->buffer_size + i]) T(*insert_begin);
                this->buffer_size += count;
            }
            else
            {
                if (this->buffer_size + count > buffer_capacity)
                {
                    auto target_capacity = this->buffer_size + count;
                    if (target_capacity == 0)
                        target_capacity = 1;
                    if (target_capacity < N)
                        target_capacity = N;

                    while (target_capacity < count)
                        target_capacity <<= 1u;

                    // Need to allocate new buffer. Move everything to a new buffer.
                    T* new_buffer =
                        target_capacity > N ? static_cast<T*>(malloc(target_capacity * sizeof(T))) : stack_storage.data();

                    // If we actually fail this malloc, we are hosed anyways, there is no reason to attempt recovery.
                    if (!new_buffer)
                        std::terminate();

                    // First, move elements from source buffer to new buffer.
                    // We don't deal with types which can throw in move constructor.
                    auto* target_itr = new_buffer;
                    auto* original_source_itr = this->begin();

                    if (new_buffer != this->ptr)
                    {
                        while (original_source_itr != itr)
                        {
                            new (target_itr) T(std::move(*original_source_itr));
                            original_source_itr->~T();
                            ++original_source_itr;
                            ++target_itr;
                        }
                    }

                    // Copy-construct new elements.
                    for (auto* source_itr = insert_begin; source_itr != insert_end; ++source_itr, ++target_itr)
                        new (target_itr) T(*source_itr);

                    // Move over the other half.
                    if (new_buffer != this->ptr || insert_begin != insert_end)
                    {
                        while (original_source_itr != this->end())
                        {
                            new (target_itr) T(std::move(*original_source_itr));
                            original_source_itr->~T();
                            ++original_source_itr;
                            ++target_itr;
                        }
                    }

                    if (this->ptr != stack_storage.data())
                        free(this->ptr);
                    this->ptr = new_buffer;
                    buffer_capacity = target_capacity;
                }
                else
                {
                    // Move in place, need to be a bit careful about which elements are constructed and which are not.
                    // Move the end and construct the new elements.
                    auto* target_itr = this->end() + count;
                    auto* source_itr = this->end();
                    while (target_itr != this->end() && source_itr != itr)
                    {
                        --target_itr;
                        --source_itr;
                        new (target_itr) T(std::move(*source_itr));
                    }

                    // For already constructed elements we can move-assign.
                    std::move_backward(itr, source_itr, target_itr);

                    // For the inserts which go to already constructed elements, we can do a plain copy.
                    while (itr != this->end() && insert_begin != insert_end)
                        *itr++ = *insert_begin++;

                    // For inserts into newly allocated memory, we must copy-construct instead.
                    while (insert_begin != insert_end)
                    {
                        new (itr) T(*insert_begin);
                        ++itr;
                        ++insert_begin;
                    }
                }

                this->buffer_size += count;
            }
        }

        void insert(T* itr, const T& value) SPIRV_CROSS_NOEXCEPT
        {
            insert(itr, &value, &value + 1);
        }

        T* erase(T* itr) SPIRV_CROSS_NOEXCEPT
        {
            std::move(itr + 1, this->end(), itr);
            this->ptr[--this->buffer_size].~T();
            return itr;
        }

        void erase(T* start_erase, T* end_erase) SPIRV_CROSS_NOEXCEPT
        {
            if (end_erase == this->end())
            {
                resize(size_t(start_erase - this->begin()));
            }
            else
            {
                auto new_size = this->buffer_size - (end_erase - start_erase);
                std::move(end_erase, this->end(), start_erase);
                resize(new_size);
            }
        }

        void resize(size_t new_size) SPIRV_CROSS_NOEXCEPT
        {
            if (new_size < this->buffer_size)
            {
                for (size_t i = new_size; i < this->buffer_size; i++)
                    this->ptr[i].~T();
            }
            else if (new_size > this->buffer_size)
            {
                reserve(new_size);
                for (size_t i = this->buffer_size; i < new_size; i++)
                    new (&this->ptr[i]) T();
            }

            this->buffer_size = new_size;
        }

    private:
        size_t buffer_capacity = 0;
        AlignedBuffer<T, N> stack_storage;
    };

    // A vector without stack storage.
    // Could also be a typedef-ed to std::vector,
    // but might as well use the one we have.
    template <typename T>
    using Vector = SmallVector<T, 0>;

#else // SPIRV_CROSS_FORCE_STL_TYPES

    template <typename T, size_t N = 8>
    using SmallVector = std::vector<T>;
    template <typename T>
    using Vector = std::vector<T>;
    template <typename T>
    using VectorView = std::vector<T>;

#endif // SPIRV_CROSS_FORCE_STL_TYPES

    // An object pool which we use for allocating IVariant-derived objects.
    // We know we are going to allocate a bunch of objects of each type,
    // so amortize the mallocs.
    class ObjectPoolBase
    {
    public:
        virtual ~ObjectPoolBase() = default;
        virtual void deallocate_opaque(void* ptr) = 0;
    };

    template <typename T>
    class ObjectPool : public ObjectPoolBase
    {
    public:
        explicit ObjectPool(unsigned start_object_count_ = 16)
            : start_object_count(start_object_count_)
        {
        }

        template <typename... P>
        T* allocate(P &&... p)
        {
            if (vacants.empty())
            {
                unsigned num_objects = start_object_count << memory.size();
                T* ptr = static_cast<T*>(malloc(num_objects * sizeof(T)));
                if (!ptr)
                    return nullptr;

                for (unsigned i = 0; i < num_objects; i++)
                    vacants.push_back(&ptr[i]);

                memory.emplace_back(ptr);
            }

            T* ptr = vacants.back();
            vacants.pop_back();
            new (ptr) T(std::forward<P>(p)...);
            return ptr;
        }

        void deallocate(T* ptr)
        {
            ptr->~T();
            vacants.push_back(ptr);
        }

        void deallocate_opaque(void* ptr) override
        {
            deallocate(static_cast<T*>(ptr));
        }

        void clear()
        {
            vacants.clear();
            memory.clear();
        }

    protected:
        Vector<T*> vacants;

        struct MallocDeleter
        {
            void operator()(T* ptr)
            {
                ::free(ptr);
            }
        };

        SmallVector<std::unique_ptr<T, MallocDeleter>> memory;
        unsigned start_object_count;
    };

    template <size_t StackSize = 4096, size_t BlockSize = 4096>
    class StringStream
    {
    public:
        StringStream()
        {
            reset();
        }

        ~StringStream()
        {
            reset();
        }

        // Disable copies and moves. Makes it easier to implement, and we don't need it.
        StringStream(const StringStream&) = delete;
        void operator=(const StringStream&) = delete;

        template <typename T, typename std::enable_if<!std::is_floating_point<T>::value, int>::type = 0>
        StringStream& operator<<(const T& t)
        {
            auto s = std::to_string(t);
            append(s.data(), s.size());
            return *this;
        }

        // Only overload this to make float/double conversions ambiguous.
        StringStream& operator<<(uint32_t v)
        {
            auto s = std::to_string(v);
            append(s.data(), s.size());
            return *this;
        }

        StringStream& operator<<(char c)
        {
            append(&c, 1);
            return *this;
        }

        StringStream& operator<<(const std::string& s)
        {
            append(s.data(), s.size());
            return *this;
        }

        StringStream& operator<<(const char* s)
        {
            append(s, strlen(s));
            return *this;
        }

        template <size_t N>
        StringStream& operator<<(const char(&s)[N])
        {
            append(s, strlen(s));
            return *this;
        }

        std::string str() const
        {
            std::string ret;
            size_t target_size = 0;
            for (auto& saved : saved_buffers)
                target_size += saved.offset;
            target_size += current_buffer.offset;
            ret.reserve(target_size);

            for (auto& saved : saved_buffers)
                ret.insert(ret.end(), saved.buffer, saved.buffer + saved.offset);
            ret.insert(ret.end(), current_buffer.buffer, current_buffer.buffer + current_buffer.offset);
            return ret;
        }

        void reset()
        {
            for (auto& saved : saved_buffers)
                if (saved.buffer != stack_buffer)
                    free(saved.buffer);
            if (current_buffer.buffer != stack_buffer)
                free(current_buffer.buffer);

            saved_buffers.clear();
            current_buffer.buffer = stack_buffer;
            current_buffer.offset = 0;
            current_buffer.size = sizeof(stack_buffer);
        }

    private:
        struct Buffer
        {
            char* buffer = nullptr;
            size_t offset = 0;
            size_t size = 0;
        };
        Buffer current_buffer;
        char stack_buffer[StackSize];
        SmallVector<Buffer> saved_buffers;

        void append(const char* s, size_t len)
        {
            size_t avail = current_buffer.size - current_buffer.offset;
            if (avail < len)
            {
                if (avail > 0)
                {
                    memcpy(current_buffer.buffer + current_buffer.offset, s, avail);
                    s += avail;
                    len -= avail;
                    current_buffer.offset += avail;
                }

                saved_buffers.push_back(current_buffer);
                size_t target_size = len > BlockSize ? len : BlockSize;
                current_buffer.buffer = static_cast<char*>(malloc(target_size));
                if (!current_buffer.buffer)
                    SPIRV_CROSS_THROW("Out of memory.");

                memcpy(current_buffer.buffer, s, len);
                current_buffer.offset = len;
                current_buffer.size = target_size;
            }
            else
            {
                memcpy(current_buffer.buffer + current_buffer.offset, s, len);
                current_buffer.offset += len;
            }
        }
    };

} // namespace SPIRV_CROSS_NAMESPACE

#endif


#include <functional>

// A bit crude, but allows projects which embed SPIRV-Cross statically to
// effectively hide all the symbols from other projects.
// There is a case where we have:
// - Project A links against SPIRV-Cross statically.
// - Project A links against Project B statically.
// - Project B links against SPIRV-Cross statically (might be a different version).
// This leads to a conflict with extremely bizarre results.
// By overriding the namespace in one of the project builds, we can work around this.
// If SPIRV-Cross is embedded in dynamic libraries,
// prefer using -fvisibility=hidden on GCC/Clang instead.
#ifdef SPIRV_CROSS_NAMESPACE_OVERRIDE
#define SPIRV_CROSS_NAMESPACE SPIRV_CROSS_NAMESPACE_OVERRIDE
#else
#define SPIRV_CROSS_NAMESPACE spirv_cross
#endif

namespace SPIRV_CROSS_NAMESPACE
{
    namespace inner
    {
        template <typename T>
        void join_helper(StringStream<>& stream, T&& t)
        {
            stream << std::forward<T>(t);
        }

        template <typename T, typename... Ts>
        void join_helper(StringStream<>& stream, T&& t, Ts &&... ts)
        {
            stream << std::forward<T>(t);
            join_helper(stream, std::forward<Ts>(ts)...);
        }
    } // namespace inner

    class Bitset
    {
    public:
        Bitset() = default;
        explicit inline Bitset(uint64_t lower_)
            : lower(lower_)
        {
        }

        inline bool get(uint32_t bit) const
        {
            if (bit < 64)
                return (lower & (1ull << bit)) != 0;
            else
                return higher.count(bit) != 0;
        }

        inline void set(uint32_t bit)
        {
            if (bit < 64)
                lower |= 1ull << bit;
            else
                higher.insert(bit);
        }

        inline void clear(uint32_t bit)
        {
            if (bit < 64)
                lower &= ~(1ull << bit);
            else
                higher.erase(bit);
        }

        inline uint64_t get_lower() const
        {
            return lower;
        }

        inline void reset()
        {
            lower = 0;
            higher.clear();
        }

        inline void merge_and(const Bitset& other)
        {
            lower &= other.lower;
            std::unordered_set<uint32_t> tmp_set;
            for (auto& v : higher)
                if (other.higher.count(v) != 0)
                    tmp_set.insert(v);
            higher = std::move(tmp_set);
        }

        inline void merge_or(const Bitset& other)
        {
            lower |= other.lower;
            for (auto& v : other.higher)
                higher.insert(v);
        }

        inline bool operator==(const Bitset& other) const
        {
            if (lower != other.lower)
                return false;

            if (higher.size() != other.higher.size())
                return false;

            for (auto& v : higher)
                if (other.higher.count(v) == 0)
                    return false;

            return true;
        }

        inline bool operator!=(const Bitset& other) const
        {
            return !(*this == other);
        }

        template <typename Op>
        void for_each_bit(const Op& op) const
        {
            // TODO: Add ctz-based iteration.
            for (uint32_t i = 0; i < 64; i++)
            {
                if (lower & (1ull << i))
                    op(i);
            }

            if (higher.empty())
                return;

            // Need to enforce an order here for reproducible results,
            // but hitting this path should happen extremely rarely, so having this slow path is fine.
            SmallVector<uint32_t> bits;
            bits.reserve(higher.size());
            for (auto& v : higher)
                bits.push_back(v);
            std::sort(std::begin(bits), std::end(bits));

            for (auto& v : bits)
                op(v);
        }

        inline bool empty() const
        {
            return lower == 0 && higher.empty();
        }

    private:
        // The most common bits to set are all lower than 64,
        // so optimize for this case. Bits spilling outside 64 go into a slower data structure.
        // In almost all cases, higher data structure will not be used.
        uint64_t lower = 0;
        std::unordered_set<uint32_t> higher;
    };

    // Helper template to avoid lots of nasty string temporary munging.
    template <typename... Ts>
    std::string join(Ts &&... ts)
    {
        StringStream<> stream;
        inner::join_helper(stream, std::forward<Ts>(ts)...);
        return stream.str();
    }

    inline std::string merge(const SmallVector<std::string>& list, const char* between = ", ")
    {
        StringStream<> stream;
        for (auto& elem : list)
        {
            stream << elem;
            if (&elem != &list.back())
                stream << between;
        }
        return stream.str();
    }

    // Make sure we don't accidentally call this with float or doubles with SFINAE.
    // Have to use the radix-aware overload.
    template <typename T, typename std::enable_if<!std::is_floating_point<T>::value, int>::type = 0>
    inline std::string convert_to_string(const T& t)
    {
        return std::to_string(t);
    }

    static inline std::string convert_to_string(int32_t value)
    {
        // INT_MIN is ... special on some backends. If we use a decimal literal, and negate it, we
        // could accidentally promote the literal to long first, then negate.
        // To workaround it, emit int(0x80000000) instead.
        if (value == std::numeric_limits<int32_t>::min())
            return "int(0x80000000)";
        else
            return std::to_string(value);
    }

    static inline std::string convert_to_string(int64_t value, const std::string& int64_type, bool long_long_literal_suffix)
    {
        // INT64_MIN is ... special on some backends.
        // If we use a decimal literal, and negate it, we might overflow the representable numbers.
        // To workaround it, emit int(0x80000000) instead.
        if (value == std::numeric_limits<int64_t>::min())
            return join(int64_type, "(0x8000000000000000u", (long_long_literal_suffix ? "ll" : "l"), ")");
        else
            return std::to_string(value) + (long_long_literal_suffix ? "ll" : "l");
    }

    // Allow implementations to set a convenient standard precision
#ifndef SPIRV_CROSS_FLT_FMT
#define SPIRV_CROSS_FLT_FMT "%.32g"
#endif

// Disable sprintf and strcat warnings.
// We cannot rely on snprintf and family existing because, ..., MSVC.
#if defined(__clang__) || defined(__GNUC__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
#elif defined(_MSC_VER)
#pragma warning(push)
#pragma warning(disable : 4996)
#endif

    static inline void fixup_radix_point(char* str, char radix_point)
    {
        // Setting locales is a very risky business in multi-threaded program,
        // so just fixup locales instead. We only need to care about the radix point.
        if (radix_point != '.')
        {
            while (*str != '\0')
            {
                if (*str == radix_point)
                    *str = '.';
                str++;
            }
        }
    }

    static inline std::string convert_to_smallest_string(float f, char locale_radix_point) {
        char buf[16]; // e.g.: -0.12345678e-12, 16 bytes needed
        float r;
        for (int i = 1; i < 9; i++) {
            sprintf(buf, "%.*g", i, f);
            sscanf(buf, "%f", &r);
            if (r == f) {
                break;
            }
        }
        fixup_radix_point(buf, locale_radix_point);

        // Ensure that the literal is float.
        if (!strchr(buf, '.') && !strchr(buf, 'e'))
            strcat(buf, ".0");

        return { buf };
    }

    inline std::string convert_to_string(float t, char locale_radix_point)
    {
        return convert_to_smallest_string(t, locale_radix_point);

        // std::to_string for floating point values is broken.
        // Fallback to something more sane.
        char buf[64];
        sprintf(buf, SPIRV_CROSS_FLT_FMT, t);
        fixup_radix_point(buf, locale_radix_point);

        // Ensure that the literal is float.
        if (!strchr(buf, '.') && !strchr(buf, 'e'))
            strcat(buf, ".0");
        return buf;
    }

    inline std::string convert_to_string(double t, char locale_radix_point)
    {
        // std::to_string for floating point values is broken.
        // Fallback to something more sane.
        char buf[64];
        sprintf(buf, SPIRV_CROSS_FLT_FMT, t);
        fixup_radix_point(buf, locale_radix_point);

        // Ensure that the literal is float.
        if (!strchr(buf, '.') && !strchr(buf, 'e'))
            strcat(buf, ".0");
        return buf;
    }

    template <typename T>
    struct ValueSaver
    {
        explicit ValueSaver(T& current_)
            : current(current_)
            , saved(current_)
        {
        }

        void release()
        {
            current = saved;
        }

        ~ValueSaver()
        {
            release();
        }

        T& current;
        T saved;
    };

#if defined(__clang__) || defined(__GNUC__)
#pragma GCC diagnostic pop
#elif defined(_MSC_VER)
#pragma warning(pop)
#endif

    struct Instruction
    {
        uint16_t op = 0;
        uint16_t count = 0;
        // If offset is 0 (not a valid offset into the instruction stream),
        // we have an instruction stream which is embedded in the object.
        uint32_t offset = 0;
        uint32_t length = 0;

        inline bool is_embedded() const
        {
            return offset == 0;
        }
    };

    struct EmbeddedInstruction : Instruction
    {
        SmallVector<uint32_t> ops;
    };

    enum Types
    {
        TypeNone,
        TypeType,
        TypeVariable,
        TypeConstant,
        TypeFunction,
        TypeFunctionPrototype,
        TypeBlock,
        TypeExtension,
        TypeExpression,
        TypeConstantOp,
        TypeCombinedImageSampler,
        TypeAccessChain,
        TypeUndef,
        TypeString,
        TypeCount
    };

    template <Types type>
    class TypedID;

    template <>
    class TypedID<TypeNone>
    {
    public:
        TypedID() = default;
        TypedID(uint32_t id_)
            : id(id_)
        {
        }

        template <Types U>
        TypedID(const TypedID<U>& other)
        {
            *this = other;
        }

        template <Types U>
        TypedID& operator=(const TypedID<U>& other)
        {
            id = uint32_t(other);
            return *this;
        }

        // Implicit conversion to u32 is desired here.
        // As long as we block implicit conversion between TypedID<A> and TypedID<B> we're good.
        operator uint32_t() const
        {
            return id;
        }

        template <Types U>
        operator TypedID<U>() const
        {
            return TypedID<U>(*this);
        }

    private:
        uint32_t id = 0;
    };

    template <Types type>
    class TypedID
    {
    public:
        TypedID() = default;
        TypedID(uint32_t id_)
            : id(id_)
        {
        }

        explicit TypedID(const TypedID<TypeNone>& other)
            : id(uint32_t(other))
        {
        }

        operator uint32_t() const
        {
            return id;
        }

    private:
        uint32_t id = 0;
    };

    using VariableID = TypedID<TypeVariable>;
    using TypeID = TypedID<TypeType>;
    using ConstantID = TypedID<TypeConstant>;
    using FunctionID = TypedID<TypeFunction>;
    using BlockID = TypedID<TypeBlock>;
    using ID = TypedID<TypeNone>;

    // Helper for Variant interface.
    struct IVariant
    {
        virtual ~IVariant() = default;
        virtual IVariant* clone(ObjectPoolBase* pool) = 0;
        ID self = 0;

    protected:
        IVariant() = default;
        IVariant(const IVariant&) = default;
        IVariant& operator=(const IVariant&) = default;
    };

#define SPIRV_CROSS_DECLARE_CLONE(T)                                \
	IVariant *clone(ObjectPoolBase *pool) override                  \
	{                                                               \
		return static_cast<ObjectPool<T> *>(pool)->allocate(*this); \
	}

    struct SPIRUndef : IVariant
    {
        enum
        {
            type = TypeUndef
        };

        explicit SPIRUndef(TypeID basetype_)
            : basetype(basetype_)
        {
        }
        TypeID basetype;

        SPIRV_CROSS_DECLARE_CLONE(SPIRUndef)
    };

    struct SPIRString : IVariant
    {
        enum
        {
            type = TypeString
        };

        explicit SPIRString(std::string str_)
            : str(std::move(str_))
        {
        }

        std::string str;

        SPIRV_CROSS_DECLARE_CLONE(SPIRString)
    };

    // This type is only used by backends which need to access the combined image and sampler IDs separately after
    // the OpSampledImage opcode.
    struct SPIRCombinedImageSampler : IVariant
    {
        enum
        {
            type = TypeCombinedImageSampler
        };
        SPIRCombinedImageSampler(TypeID type_, VariableID image_, VariableID sampler_)
            : combined_type(type_)
            , image(image_)
            , sampler(sampler_)
        {
        }
        TypeID combined_type;
        VariableID image;
        VariableID sampler;

        SPIRV_CROSS_DECLARE_CLONE(SPIRCombinedImageSampler)
    };

    struct SPIRConstantOp : IVariant
    {
        enum
        {
            type = TypeConstantOp
        };

        SPIRConstantOp(TypeID result_type, spv::Op op, const uint32_t* args, uint32_t length)
            : opcode(op)
            , basetype(result_type)
        {
            arguments.reserve(length);
            for (uint32_t i = 0; i < length; i++)
                arguments.push_back(args[i]);
        }

        spv::Op opcode;
        SmallVector<uint32_t> arguments;
        TypeID basetype;

        SPIRV_CROSS_DECLARE_CLONE(SPIRConstantOp)
    };

    struct SPIRType : IVariant
    {
        enum
        {
            type = TypeType
        };

        enum BaseType
        {
            Unknown,
            Void,
            Boolean,
            SByte,
            UByte,
            Short,
            UShort,
            Int,
            UInt,
            Int64,
            UInt64,
            AtomicCounter,
            Half,
            Float,
            Double,
            Struct,
            Image,
            SampledImage,
            Sampler,
            AccelerationStructure,
            RayQuery,

            // Keep internal types at the end.
            ControlPointArray,
            Interpolant,
            Char
        };

        // Scalar/vector/matrix support.
        BaseType basetype = Unknown;
        uint32_t width = 0;
        uint32_t vecsize = 1;
        uint32_t columns = 1;

        // Arrays, support array of arrays by having a vector of array sizes.
        SmallVector<uint32_t> array;

        // Array elements can be either specialization constants or specialization ops.
        // This array determines how to interpret the array size.
        // If an element is true, the element is a literal,
        // otherwise, it's an expression, which must be resolved on demand.
        // The actual size is not really known until runtime.
        SmallVector<bool> array_size_literal;

        // Pointers
        // Keep track of how many pointer layers we have.
        uint32_t pointer_depth = 0;
        bool pointer = false;
        bool forward_pointer = false;

        spv::StorageClass storage = spv::StorageClassGeneric;

        SmallVector<TypeID> member_types;

        // If member order has been rewritten to handle certain scenarios with Offset,
        // allow codegen to rewrite the index.
        SmallVector<uint32_t> member_type_index_redirection;

        struct ImageType
        {
            TypeID type;
            spv::Dim dim;
            bool depth;
            bool arrayed;
            bool ms;
            uint32_t sampled;
            spv::ImageFormat format;
            spv::AccessQualifier access;
        } image;

        // Structs can be declared multiple times if they are used as part of interface blocks.
        // We want to detect this so that we only emit the struct definition once.
        // Since we cannot rely on OpName to be equal, we need to figure out aliases.
        TypeID type_alias = 0;

        // Denotes the type which this type is based on.
        // Allows the backend to traverse how a complex type is built up during access chains.
        TypeID parent_type = 0;

        // Used in backends to avoid emitting members with conflicting names.
        std::unordered_set<std::string> member_name_cache;

        SPIRV_CROSS_DECLARE_CLONE(SPIRType)
    };

    struct SPIRExtension : IVariant
    {
        enum
        {
            type = TypeExtension
        };

        enum Extension
        {
            Unsupported,
            GLSL,
            SPV_debug_info,
            SPV_AMD_shader_ballot,
            SPV_AMD_shader_explicit_vertex_parameter,
            SPV_AMD_shader_trinary_minmax,
            SPV_AMD_gcn_shader,
            NonSemanticDebugPrintf,
            NonSemanticShaderDebugInfo,
            NonSemanticGeneric
        };

        explicit SPIRExtension(Extension ext_)
            : ext(ext_)
        {
        }

        Extension ext;
        SPIRV_CROSS_DECLARE_CLONE(SPIRExtension)
    };

    // SPIREntryPoint is not a variant since its IDs are used to decorate OpFunction,
    // so in order to avoid conflicts, we can't stick them in the ids array.
    struct SPIREntryPoint
    {
        SPIREntryPoint(FunctionID self_, spv::ExecutionModel execution_model, const std::string& entry_name)
            : self(self_)
            , name(entry_name)
            , orig_name(entry_name)
            , model(execution_model)
        {
        }
        SPIREntryPoint() = default;

        FunctionID self = 0;
        std::string name;
        std::string orig_name;
        SmallVector<VariableID> interface_variables;

        Bitset flags;
        struct WorkgroupSize
        {
            uint32_t x = 0, y = 0, z = 0;
            uint32_t id_x = 0, id_y = 0, id_z = 0;
            uint32_t constant = 0; // Workgroup size can be expressed as a constant/spec-constant instead.
        } workgroup_size;
        uint32_t invocations = 0;
        uint32_t output_vertices = 0;
        uint32_t output_primitives = 0;
        spv::ExecutionModel model = spv::ExecutionModelMax;
        bool geometry_passthrough = false;
    };

    struct SPIRExpression : IVariant
    {
        enum
        {
            type = TypeExpression
        };

        // Only created by the backend target to avoid creating tons of temporaries.
        SPIRExpression(std::string expr, TypeID expression_type_, bool immutable_)
            : expression(std::move(expr))
            , expression_type(expression_type_)
            , immutable(immutable_)
        {
        }

        // If non-zero, prepend expression with to_expression(base_expression).
        // Used in amortizing multiple calls to to_expression()
        // where in certain cases that would quickly force a temporary when not needed.
        ID base_expression = 0;

        std::string expression;
        TypeID expression_type = 0;

        // If this expression is a forwarded load,
        // allow us to reference the original variable.
        ID loaded_from = 0;

        // If this expression will never change, we can avoid lots of temporaries
        // in high level source.
        // An expression being immutable can be speculative,
        // it is assumed that this is true almost always.
        bool immutable = false;

        // Before use, this expression must be transposed.
        // This is needed for targets which don't support row_major layouts.
        bool need_transpose = false;

        // Whether or not this is an access chain expression.
        bool access_chain = false;

        // A list of expressions which this expression depends on.
        SmallVector<ID> expression_dependencies;

        // By reading this expression, we implicitly read these expressions as well.
        // Used by access chain Store and Load since we read multiple expressions in this case.
        SmallVector<ID> implied_read_expressions;

        // The expression was emitted at a certain scope. Lets us track when an expression read means multiple reads.
        uint32_t emitted_loop_level = 0;

        SPIRV_CROSS_DECLARE_CLONE(SPIRExpression)
    };

    struct SPIRFunctionPrototype : IVariant
    {
        enum
        {
            type = TypeFunctionPrototype
        };

        explicit SPIRFunctionPrototype(TypeID return_type_)
            : return_type(return_type_)
        {
        }

        TypeID return_type;
        SmallVector<uint32_t> parameter_types;

        SPIRV_CROSS_DECLARE_CLONE(SPIRFunctionPrototype)
    };

    struct SPIRBlock : IVariant
    {
        enum
        {
            type = TypeBlock
        };

        enum Terminator
        {
            Unknown,
            Direct, // Emit next block directly without a particular condition.

            Select, // Block ends with an if/else block.
            MultiSelect, // Block ends with switch statement.

            Return, // Block ends with return.
            Unreachable, // Noop
            Kill, // Discard
            IgnoreIntersection, // Ray Tracing
            TerminateRay, // Ray Tracing
            EmitMeshTasks // Mesh shaders
        };

        enum Merge
        {
            MergeNone,
            MergeLoop,
            MergeSelection
        };

        enum Hints
        {
            HintNone,
            HintUnroll,
            HintDontUnroll,
            HintFlatten,
            HintDontFlatten
        };

        enum Method
        {
            MergeToSelectForLoop,
            MergeToDirectForLoop,
            MergeToSelectContinueForLoop
        };

        enum ContinueBlockType
        {
            ContinueNone,

            // Continue block is branchless and has at least one instruction.
            ForLoop,

            // Noop continue block.
            WhileLoop,

            // Continue block is conditional.
            DoWhileLoop,

            // Highly unlikely that anything will use this,
            // since it is really awkward/impossible to express in GLSL.
            ComplexLoop
        };

        enum : uint32_t
        {
            NoDominator = 0xffffffffu
        };

        Terminator terminator = Unknown;
        Merge merge = MergeNone;
        Hints hint = HintNone;
        BlockID next_block = 0;
        BlockID merge_block = 0;
        BlockID continue_block = 0;

        ID return_value = 0; // If 0, return nothing (void).
        ID condition = 0;
        BlockID true_block = 0;
        BlockID false_block = 0;
        BlockID default_block = 0;

        // If terminator is EmitMeshTasksEXT.
        struct
        {
            ID groups[3];
            ID payload;
        } mesh = {};

        SmallVector<Instruction> ops;

        struct Phi
        {
            ID local_variable; // flush local variable ...
            BlockID parent; // If we're in from_block and want to branch into this block ...
            VariableID function_variable; // to this function-global "phi" variable first.
        };

        // Before entering this block flush out local variables to magical "phi" variables.
        SmallVector<Phi> phi_variables;

        // Declare these temporaries before beginning the block.
        // Used for handling complex continue blocks which have side effects.
        SmallVector<std::pair<TypeID, ID>> declare_temporary;

        // Declare these temporaries, but only conditionally if this block turns out to be
        // a complex loop header.
        SmallVector<std::pair<TypeID, ID>> potential_declare_temporary;

        struct Case
        {
            uint64_t value;
            BlockID block;
        };
        SmallVector<Case> cases_32bit;
        SmallVector<Case> cases_64bit;

        // If we have tried to optimize code for this block but failed,
        // keep track of this.
        bool disable_block_optimization = false;

        // If the continue block is complex, fallback to "dumb" for loops.
        bool complex_continue = false;

        // Do we need a ladder variable to defer breaking out of a loop construct after a switch block?
        bool need_ladder_break = false;

        // If marked, we have explicitly handled Phi from this block, so skip any flushes related to that on a branch.
        // Used to handle an edge case with switch and case-label fallthrough where fall-through writes to Phi.
        BlockID ignore_phi_from_block = 0;

        // The dominating block which this block might be within.
        // Used in continue; blocks to determine if we really need to write continue.
        BlockID loop_dominator = 0;

        // All access to these variables are dominated by this block,
        // so before branching anywhere we need to make sure that we declare these variables.
        SmallVector<VariableID> dominated_variables;

        // These are variables which should be declared in a for loop header, if we
        // fail to use a classic for-loop,
        // we remove these variables, and fall back to regular variables outside the loop.
        SmallVector<VariableID> loop_variables;

        // Some expressions are control-flow dependent, i.e. any instruction which relies on derivatives or
        // sub-group-like operations.
        // Make sure that we only use these expressions in the original block.
        SmallVector<ID> invalidate_expressions;

        SPIRV_CROSS_DECLARE_CLONE(SPIRBlock)
    };

    struct SPIRFunction : IVariant
    {
        enum
        {
            type = TypeFunction
        };

        SPIRFunction(TypeID return_type_, TypeID function_type_)
            : return_type(return_type_)
            , function_type(function_type_)
        {
        }

        struct Parameter
        {
            TypeID type;
            ID id;
            uint32_t read_count;
            uint32_t write_count;

            // Set to true if this parameter aliases a global variable,
            // used mostly in Metal where global variables
            // have to be passed down to functions as regular arguments.
            // However, for this kind of variable, we should not care about
            // read and write counts as access to the function arguments
            // is not local to the function in question.
            bool alias_global_variable;
        };

        // When calling a function, and we're remapping separate image samplers,
        // resolve these arguments into combined image samplers and pass them
        // as additional arguments in this order.
        // It gets more complicated as functions can pull in their own globals
        // and combine them with parameters,
        // so we need to distinguish if something is local parameter index
        // or a global ID.
        struct CombinedImageSamplerParameter
        {
            VariableID id;
            VariableID image_id;
            VariableID sampler_id;
            bool global_image;
            bool global_sampler;
            bool depth;
        };

        TypeID return_type;
        TypeID function_type;
        SmallVector<Parameter> arguments;

        // Can be used by backends to add magic arguments.
        // Currently used by combined image/sampler implementation.

        SmallVector<Parameter> shadow_arguments;
        SmallVector<VariableID> local_variables;
        BlockID entry_block = 0;
        SmallVector<BlockID> blocks;
        SmallVector<CombinedImageSamplerParameter> combined_parameters;

        struct EntryLine
        {
            uint32_t file_id = 0;
            uint32_t line_literal = 0;
        };
        EntryLine entry_line;

        void add_local_variable(VariableID id)
        {
            local_variables.push_back(id);
        }

        void add_parameter(TypeID parameter_type, ID id, bool alias_global_variable = false)
        {
            // Arguments are read-only until proven otherwise.
            arguments.push_back({ parameter_type, id, 0u, 0u, alias_global_variable });
        }

        // Hooks to be run when the function returns.
        // Mostly used for lowering internal data structures onto flattened structures.
        // Need to defer this, because they might rely on things which change during compilation.
        // Intentionally not a small vector, this one is rare, and std::function can be large.
        Vector<std::function<void()>> fixup_hooks_out;

        // Hooks to be run when the function begins.
        // Mostly used for populating internal data structures from flattened structures.
        // Need to defer this, because they might rely on things which change during compilation.
        // Intentionally not a small vector, this one is rare, and std::function can be large.
        Vector<std::function<void()>> fixup_hooks_in;

        // On function entry, make sure to copy a constant array into thread addr space to work around
        // the case where we are passing a constant array by value to a function on backends which do not
        // consider arrays value types.
        SmallVector<ID> constant_arrays_needed_on_stack;

        bool active = false;
        bool flush_undeclared = true;
        bool do_combined_parameters = true;

        SPIRV_CROSS_DECLARE_CLONE(SPIRFunction)
    };

    struct SPIRAccessChain : IVariant
    {
        enum
        {
            type = TypeAccessChain
        };

        SPIRAccessChain(TypeID basetype_, spv::StorageClass storage_, std::string base_, std::string dynamic_index_,
            int32_t static_index_)
            : basetype(basetype_)
            , storage(storage_)
            , base(std::move(base_))
            , dynamic_index(std::move(dynamic_index_))
            , static_index(static_index_)
        {
        }

        // The access chain represents an offset into a buffer.
        // Some backends need more complicated handling of access chains to be able to use buffers, like HLSL
        // which has no usable buffer type ala GLSL SSBOs.
        // StructuredBuffer is too limited, so our only option is to deal with ByteAddressBuffer which works with raw addresses.

        TypeID basetype;
        spv::StorageClass storage;
        std::string base;
        std::string dynamic_index;
        int32_t static_index;

        VariableID loaded_from = 0;
        uint32_t matrix_stride = 0;
        uint32_t array_stride = 0;
        bool row_major_matrix = false;
        bool immutable = false;

        // By reading this expression, we implicitly read these expressions as well.
        // Used by access chain Store and Load since we read multiple expressions in this case.
        SmallVector<ID> implied_read_expressions;

        SPIRV_CROSS_DECLARE_CLONE(SPIRAccessChain)
    };

    struct SPIRVariable : IVariant
    {
        enum
        {
            type = TypeVariable
        };

        SPIRVariable() = default;
        SPIRVariable(TypeID basetype_, spv::StorageClass storage_, ID initializer_ = 0, VariableID basevariable_ = 0)
            : basetype(basetype_)
            , storage(storage_)
            , initializer(initializer_)
            , basevariable(basevariable_)
        {
        }

        TypeID basetype = 0;
        spv::StorageClass storage = spv::StorageClassGeneric;
        uint32_t decoration = 0;
        ID initializer = 0;
        VariableID basevariable = 0;

        SmallVector<uint32_t> dereference_chain;
        bool compat_builtin = false;

        // If a variable is shadowed, we only statically assign to it
        // and never actually emit a statement for it.
        // When we read the variable as an expression, just forward
        // shadowed_id as the expression.
        bool statically_assigned = false;
        ID static_expression = 0;

        // Temporaries which can remain forwarded as long as this variable is not modified.
        SmallVector<ID> dependees;

        bool deferred_declaration = false;
        bool phi_variable = false;

        // Used to deal with Phi variable flushes. See flush_phi().
        bool allocate_temporary_copy = false;

        bool remapped_variable = false;
        uint32_t remapped_components = 0;

        // The block which dominates all access to this variable.
        BlockID dominator = 0;
        // If true, this variable is a loop variable, when accessing the variable
        // outside a loop,
        // we should statically forward it.
        bool loop_variable = false;
        // Set to true while we're inside the for loop.
        bool loop_variable_enable = false;

        SPIRFunction::Parameter* parameter = nullptr;

        SPIRV_CROSS_DECLARE_CLONE(SPIRVariable)
    };

    struct SPIRConstant : IVariant
    {
        enum
        {
            type = TypeConstant
        };

        union Constant
        {
            uint32_t u32;
            int32_t i32;
            float f32;

            uint64_t u64;
            int64_t i64;
            double f64;
        };

        struct ConstantVector
        {
            Constant r[4];
            // If != 0, this element is a specialization constant, and we should keep track of it as such.
            ID id[4];
            uint32_t vecsize = 1;

            ConstantVector()
            {
                memset(r, 0, sizeof(r));
            }
        };

        struct ConstantMatrix
        {
            ConstantVector c[4];
            // If != 0, this column is a specialization constant, and we should keep track of it as such.
            ID id[4];
            uint32_t columns = 1;
        };

        static inline float f16_to_f32(uint16_t u16_value)
        {
            // Based on the GLM implementation.
            int s = (u16_value >> 15) & 0x1;
            int e = (u16_value >> 10) & 0x1f;
            int m = (u16_value >> 0) & 0x3ff;

            union
            {
                float f32;
                uint32_t u32;
            } u;

            if (e == 0)
            {
                if (m == 0)
                {
                    u.u32 = uint32_t(s) << 31;
                    return u.f32;
                }
                else
                {
                    while ((m & 0x400) == 0)
                    {
                        m <<= 1;
                        e--;
                    }

                    e++;
                    m &= ~0x400;
                }
            }
            else if (e == 31)
            {
                if (m == 0)
                {
                    u.u32 = (uint32_t(s) << 31) | 0x7f800000u;
                    return u.f32;
                }
                else
                {
                    u.u32 = (uint32_t(s) << 31) | 0x7f800000u | (m << 13);
                    return u.f32;
                }
            }

            e += 127 - 15;
            m <<= 13;
            u.u32 = (uint32_t(s) << 31) | (e << 23) | m;
            return u.f32;
        }

        inline uint32_t specialization_constant_id(uint32_t col, uint32_t row) const
        {
            return m.c[col].id[row];
        }

        inline uint32_t specialization_constant_id(uint32_t col) const
        {
            return m.id[col];
        }

        inline uint32_t scalar(uint32_t col = 0, uint32_t row = 0) const
        {
            return m.c[col].r[row].u32;
        }

        inline int16_t scalar_i16(uint32_t col = 0, uint32_t row = 0) const
        {
            return int16_t(m.c[col].r[row].u32 & 0xffffu);
        }

        inline uint16_t scalar_u16(uint32_t col = 0, uint32_t row = 0) const
        {
            return uint16_t(m.c[col].r[row].u32 & 0xffffu);
        }

        inline int8_t scalar_i8(uint32_t col = 0, uint32_t row = 0) const
        {
            return int8_t(m.c[col].r[row].u32 & 0xffu);
        }

        inline uint8_t scalar_u8(uint32_t col = 0, uint32_t row = 0) const
        {
            return uint8_t(m.c[col].r[row].u32 & 0xffu);
        }

        inline float scalar_f16(uint32_t col = 0, uint32_t row = 0) const
        {
            return f16_to_f32(scalar_u16(col, row));
        }

        inline float scalar_f32(uint32_t col = 0, uint32_t row = 0) const
        {
            return m.c[col].r[row].f32;
        }

        inline int32_t scalar_i32(uint32_t col = 0, uint32_t row = 0) const
        {
            return m.c[col].r[row].i32;
        }

        inline double scalar_f64(uint32_t col = 0, uint32_t row = 0) const
        {
            return m.c[col].r[row].f64;
        }

        inline int64_t scalar_i64(uint32_t col = 0, uint32_t row = 0) const
        {
            return m.c[col].r[row].i64;
        }

        inline uint64_t scalar_u64(uint32_t col = 0, uint32_t row = 0) const
        {
            return m.c[col].r[row].u64;
        }

        inline const ConstantVector& vector() const
        {
            return m.c[0];
        }

        inline uint32_t vector_size() const
        {
            return m.c[0].vecsize;
        }

        inline uint32_t columns() const
        {
            return m.columns;
        }

        inline void make_null(const SPIRType& constant_type_)
        {
            m = {};
            m.columns = constant_type_.columns;
            for (auto& c : m.c)
                c.vecsize = constant_type_.vecsize;
        }

        inline bool constant_is_null() const
        {
            if (specialization)
                return false;
            if (!subconstants.empty())
                return false;

            for (uint32_t col = 0; col < columns(); col++)
                for (uint32_t row = 0; row < vector_size(); row++)
                    if (scalar_u64(col, row) != 0)
                        return false;

            return true;
        }

        explicit SPIRConstant(uint32_t constant_type_)
            : constant_type(constant_type_)
        {
        }

        SPIRConstant() = default;

        SPIRConstant(TypeID constant_type_, const uint32_t* elements, uint32_t num_elements, bool specialized)
            : constant_type(constant_type_)
            , specialization(specialized)
        {
            subconstants.reserve(num_elements);
            for (uint32_t i = 0; i < num_elements; i++)
                subconstants.push_back(elements[i]);
            specialization = specialized;
        }

        // Construct scalar (32-bit).
        SPIRConstant(TypeID constant_type_, uint32_t v0, bool specialized)
            : constant_type(constant_type_)
            , specialization(specialized)
        {
            m.c[0].r[0].u32 = v0;
            m.c[0].vecsize = 1;
            m.columns = 1;
        }

        // Construct scalar (64-bit).
        SPIRConstant(TypeID constant_type_, uint64_t v0, bool specialized)
            : constant_type(constant_type_)
            , specialization(specialized)
        {
            m.c[0].r[0].u64 = v0;
            m.c[0].vecsize = 1;
            m.columns = 1;
        }

        // Construct vectors and matrices.
        SPIRConstant(TypeID constant_type_, const SPIRConstant* const* vector_elements, uint32_t num_elements,
            bool specialized)
            : constant_type(constant_type_)
            , specialization(specialized)
        {
            bool matrix = vector_elements[0]->m.c[0].vecsize > 1;

            if (matrix)
            {
                m.columns = num_elements;

                for (uint32_t i = 0; i < num_elements; i++)
                {
                    m.c[i] = vector_elements[i]->m.c[0];
                    if (vector_elements[i]->specialization)
                        m.id[i] = vector_elements[i]->self;
                }
            }
            else
            {
                m.c[0].vecsize = num_elements;
                m.columns = 1;

                for (uint32_t i = 0; i < num_elements; i++)
                {
                    m.c[0].r[i] = vector_elements[i]->m.c[0].r[0];
                    if (vector_elements[i]->specialization)
                        m.c[0].id[i] = vector_elements[i]->self;
                }
            }
        }

        TypeID constant_type = 0;
        ConstantMatrix m;

        // If this constant is a specialization constant (i.e. created with OpSpecConstant*).
        bool specialization = false;
        // If this constant is used as an array length which creates specialization restrictions on some backends.
        bool is_used_as_array_length = false;

        // If true, this is a LUT, and should always be declared in the outer scope.
        bool is_used_as_lut = false;

        // For composites which are constant arrays, etc.
        SmallVector<ConstantID> subconstants;

        // Non-Vulkan GLSL, HLSL and sometimes MSL emits defines for each specialization constant,
        // and uses them to initialize the constant. This allows the user
        // to still be able to specialize the value by supplying corresponding
        // preprocessor directives before compiling the shader.
        std::string specialization_constant_macro_name;

        SPIRV_CROSS_DECLARE_CLONE(SPIRConstant)
    };

    // Variants have a very specific allocation scheme.
    struct ObjectPoolGroup
    {
        std::unique_ptr<ObjectPoolBase> pools[TypeCount];
    };

    class Variant
    {
    public:
        explicit Variant(ObjectPoolGroup* group_)
            : group(group_)
        {
        }

        ~Variant()
        {
            if (holder)
                group->pools[type]->deallocate_opaque(holder);
        }

        // Marking custom move constructor as noexcept is important.
        Variant(Variant&& other) SPIRV_CROSS_NOEXCEPT
        {
            *this = std::move(other);
        }

        // We cannot copy from other variant without our own pool group.
        // Have to explicitly copy.
        Variant(const Variant& variant) = delete;

        // Marking custom move constructor as noexcept is important.
        Variant& operator=(Variant&& other) SPIRV_CROSS_NOEXCEPT
        {
            if (this != &other)
            {
                if (holder)
                    group->pools[type]->deallocate_opaque(holder);
                holder = other.holder;
                group = other.group;
                type = other.type;
                allow_type_rewrite = other.allow_type_rewrite;

                other.holder = nullptr;
                other.type = TypeNone;
            }
            return *this;
        }

        // This copy/clone should only be called in the Compiler constructor.
        // If this is called inside ::compile(), we invalidate any references we took higher in the stack.
        // This should never happen.
        Variant& operator=(const Variant& other)
        {
            //#define SPIRV_CROSS_COPY_CONSTRUCTOR_SANITIZE
#ifdef SPIRV_CROSS_COPY_CONSTRUCTOR_SANITIZE
            abort();
#endif
            if (this != &other)
            {
                if (holder)
                    group->pools[type]->deallocate_opaque(holder);

                if (other.holder)
                    holder = other.holder->clone(group->pools[other.type].get());
                else
                    holder = nullptr;

                type = other.type;
                allow_type_rewrite = other.allow_type_rewrite;
            }
            return *this;
        }

        void set(IVariant* val, Types new_type)
        {
            if (holder)
                group->pools[type]->deallocate_opaque(holder);
            holder = nullptr;

            if (!allow_type_rewrite && type != TypeNone && type != new_type)
            {
                if (val)
                    group->pools[new_type]->deallocate_opaque(val);
                SPIRV_CROSS_THROW("Overwriting a variant with new type.");
            }

            holder = val;
            type = new_type;
            allow_type_rewrite = false;
        }

        template <typename T, typename... Ts>
        T* allocate_and_set(Types new_type, Ts &&... ts)
        {
            T* val = static_cast<ObjectPool<T> &>(*group->pools[new_type]).allocate(std::forward<Ts>(ts)...);
            set(val, new_type);
            return val;
        }

        template <typename T>
        T& get()
        {
            if (!holder)
                SPIRV_CROSS_THROW("nullptr");
            if (static_cast<Types>(T::type) != type)
                SPIRV_CROSS_THROW("Bad cast");
            return *static_cast<T*>(holder);
        }

        template <typename T>
        const T& get() const
        {
            if (!holder)
                SPIRV_CROSS_THROW("nullptr");
            if (static_cast<Types>(T::type) != type)
                SPIRV_CROSS_THROW("Bad cast");
            return *static_cast<const T*>(holder);
        }

        Types get_type() const
        {
            return type;
        }

        ID get_id() const
        {
            return holder ? holder->self : ID(0);
        }

        bool empty() const
        {
            return !holder;
        }

        void reset()
        {
            if (holder)
                group->pools[type]->deallocate_opaque(holder);
            holder = nullptr;
            type = TypeNone;
        }

        void set_allow_type_rewrite()
        {
            allow_type_rewrite = true;
        }

    private:
        ObjectPoolGroup* group = nullptr;
        IVariant* holder = nullptr;
        Types type = TypeNone;
        bool allow_type_rewrite = false;
    };

    template <typename T>
    T& variant_get(Variant& var)
    {
        return var.get<T>();
    }

    template <typename T>
    const T& variant_get(const Variant& var)
    {
        return var.get<T>();
    }

    template <typename T, typename... P>
    T& variant_set(Variant& var, P &&... args)
    {
        auto* ptr = var.allocate_and_set<T>(static_cast<Types>(T::type), std::forward<P>(args)...);
        return *ptr;
    }

    struct AccessChainMeta
    {
        uint32_t storage_physical_type = 0;
        bool need_transpose = false;
        bool storage_is_packed = false;
        bool storage_is_invariant = false;
        bool flattened_struct = false;
        bool relaxed_precision = false;
    };

    enum ExtendedDecorations
    {
        // Marks if a buffer block is re-packed, i.e. member declaration might be subject to PhysicalTypeID remapping and padding.
        SPIRVCrossDecorationBufferBlockRepacked = 0,

        // A type in a buffer block might be declared with a different physical type than the logical type.
        // If this is not set, PhysicalTypeID == the SPIR-V type as declared.
        SPIRVCrossDecorationPhysicalTypeID,

        // Marks if the physical type is to be declared with tight packing rules, i.e. packed_floatN on MSL and friends.
        // If this is set, PhysicalTypeID might also be set. It can be set to same as logical type if all we're doing
        // is converting float3 to packed_float3 for example.
        // If this is marked on a struct, it means the struct itself must use only Packed types for all its members.
        SPIRVCrossDecorationPhysicalTypePacked,

        // The padding in bytes before declaring this struct member.
        // If used on a struct type, marks the target size of a struct.
        SPIRVCrossDecorationPaddingTarget,

        SPIRVCrossDecorationInterfaceMemberIndex,
        SPIRVCrossDecorationInterfaceOrigID,
        SPIRVCrossDecorationResourceIndexPrimary,
        // Used for decorations like resource indices for samplers when part of combined image samplers.
        // A variable might need to hold two resource indices in this case.
        SPIRVCrossDecorationResourceIndexSecondary,
        // Used for resource indices for multiplanar images when part of combined image samplers.
        SPIRVCrossDecorationResourceIndexTertiary,
        SPIRVCrossDecorationResourceIndexQuaternary,

        // Marks a buffer block for using explicit offsets (GLSL/HLSL).
        SPIRVCrossDecorationExplicitOffset,

        // Apply to a variable in the Input storage class; marks it as holding the base group passed to vkCmdDispatchBase(),
        // or the base vertex and instance indices passed to vkCmdDrawIndexed().
        // In MSL, this is used to adjust the WorkgroupId and GlobalInvocationId variables in compute shaders,
        // and to hold the BaseVertex and BaseInstance variables in vertex shaders.
        SPIRVCrossDecorationBuiltInDispatchBase,

        // Apply to a variable that is a function parameter; marks it as being a "dynamic"
        // combined image-sampler. In MSL, this is used when a function parameter might hold
        // either a regular combined image-sampler or one that has an attached sampler
        // Y'CbCr conversion.
        SPIRVCrossDecorationDynamicImageSampler,

        // Apply to a variable in the Input storage class; marks it as holding the size of the stage
        // input grid.
        // In MSL, this is used to hold the vertex and instance counts in a tessellation pipeline
        // vertex shader.
        SPIRVCrossDecorationBuiltInStageInputSize,

        // Apply to any access chain of a tessellation I/O variable; stores the type of the sub-object
        // that was chained to, as recorded in the input variable itself. This is used in case the pointer
        // is itself used as the base of an access chain, to calculate the original type of the sub-object
        // chained to, in case a swizzle needs to be applied. This should not happen normally with valid
        // SPIR-V, but the MSL backend can change the type of input variables, necessitating the
        // addition of swizzles to keep the generated code compiling.
        SPIRVCrossDecorationTessIOOriginalInputTypeID,

        // Apply to any access chain of an interface variable used with pull-model interpolation, where the variable is a
        // vector but the resulting pointer is a scalar; stores the component index that is to be accessed by the chain.
        // This is used when emitting calls to interpolation functions on the chain in MSL: in this case, the component
        // must be applied to the result, since pull-model interpolants in MSL cannot be swizzled directly, but the
        // results of interpolation can.
        SPIRVCrossDecorationInterpolantComponentExpr,

        // Apply to any struct type that is used in the Workgroup storage class.
        // This causes matrices in MSL prior to Metal 3.0 to be emitted using a special
        // class that is convertible to the standard matrix type, to work around the
        // lack of constructors in the 'threadgroup' address space.
        SPIRVCrossDecorationWorkgroupStruct,

        SPIRVCrossDecorationCount
    };

    struct Meta
    {
        struct Decoration
        {
            std::string alias;
            std::string qualified_alias;
            std::string hlsl_semantic;
            std::string user_type;
            Bitset decoration_flags;
            spv::BuiltIn builtin_type = spv::BuiltInMax;
            uint32_t location = 0;
            uint32_t component = 0;
            uint32_t set = 0;
            uint32_t binding = 0;
            uint32_t offset = 0;
            uint32_t xfb_buffer = 0;
            uint32_t xfb_stride = 0;
            uint32_t stream = 0;
            uint32_t array_stride = 0;
            uint32_t matrix_stride = 0;
            uint32_t input_attachment = 0;
            uint32_t spec_id = 0;
            uint32_t index = 0;
            spv::FPRoundingMode fp_rounding_mode = spv::FPRoundingModeMax;
            bool builtin = false;

            struct Extended
            {
                Extended()
                {
                    // MSVC 2013 workaround to init like this.
                    for (auto& v : values)
                        v = 0;
                }

                Bitset flags;
                uint32_t values[SPIRVCrossDecorationCount];
            } extended;
        };

        Decoration decoration;

        // Intentionally not a SmallVector. Decoration is large and somewhat rare.
        Vector<Decoration> members;

        std::unordered_map<uint32_t, uint32_t> decoration_word_offset;

        // For SPV_GOOGLE_hlsl_functionality1.
        bool hlsl_is_magic_counter_buffer = false;
        // ID for the sibling counter buffer.
        uint32_t hlsl_magic_counter_buffer = 0;
    };

    // A user callback that remaps the type of any variable.
    // var_name is the declared name of the variable.
    // name_of_type is the textual name of the type which will be used in the code unless written to by the callback.
    using VariableTypeRemapCallback =
        std::function<void(const SPIRType& type, const std::string& var_name, std::string& name_of_type)>;

    class Hasher
    {
    public:
        inline void u32(uint32_t value)
        {
            h = (h * 0x100000001b3ull) ^ value;
        }

        inline uint64_t get() const
        {
            return h;
        }

    private:
        uint64_t h = 0xcbf29ce484222325ull;
    };

    static inline bool type_is_floating_point(const SPIRType& type)
    {
        return type.basetype == SPIRType::Half || type.basetype == SPIRType::Float || type.basetype == SPIRType::Double;
    }

    static inline bool type_is_integral(const SPIRType& type)
    {
        return type.basetype == SPIRType::SByte || type.basetype == SPIRType::UByte || type.basetype == SPIRType::Short ||
            type.basetype == SPIRType::UShort || type.basetype == SPIRType::Int || type.basetype == SPIRType::UInt ||
            type.basetype == SPIRType::Int64 || type.basetype == SPIRType::UInt64;
    }

    static inline SPIRType::BaseType to_signed_basetype(uint32_t width)
    {
        switch (width)
        {
        case 8:
            return SPIRType::SByte;
        case 16:
            return SPIRType::Short;
        case 32:
            return SPIRType::Int;
        case 64:
            return SPIRType::Int64;
        default:
            SPIRV_CROSS_THROW("Invalid bit width.");
        }
    }

    static inline SPIRType::BaseType to_unsigned_basetype(uint32_t width)
    {
        switch (width)
        {
        case 8:
            return SPIRType::UByte;
        case 16:
            return SPIRType::UShort;
        case 32:
            return SPIRType::UInt;
        case 64:
            return SPIRType::UInt64;
        default:
            SPIRV_CROSS_THROW("Invalid bit width.");
        }
    }

    // Returns true if an arithmetic operation does not change behavior depending on signedness.
    static inline bool opcode_is_sign_invariant(spv::Op opcode)
    {
        switch (opcode)
        {
        case spv::OpIEqual:
        case spv::OpINotEqual:
        case spv::OpISub:
        case spv::OpIAdd:
        case spv::OpIMul:
        case spv::OpShiftLeftLogical:
        case spv::OpBitwiseOr:
        case spv::OpBitwiseXor:
        case spv::OpBitwiseAnd:
            return true;

        default:
            return false;
        }
    }

    static inline bool opcode_can_promote_integer_implicitly(spv::Op opcode)
    {
        switch (opcode)
        {
        case spv::OpSNegate:
        case spv::OpNot:
        case spv::OpBitwiseAnd:
        case spv::OpBitwiseOr:
        case spv::OpBitwiseXor:
        case spv::OpShiftLeftLogical:
        case spv::OpShiftRightLogical:
        case spv::OpShiftRightArithmetic:
        case spv::OpIAdd:
        case spv::OpISub:
        case spv::OpIMul:
        case spv::OpSDiv:
        case spv::OpUDiv:
        case spv::OpSRem:
        case spv::OpUMod:
        case spv::OpSMod:
            return true;

        default:
            return false;
        }
    }

    struct SetBindingPair
    {
        uint32_t desc_set;
        uint32_t binding;

        inline bool operator==(const SetBindingPair& other) const
        {
            return desc_set == other.desc_set && binding == other.binding;
        }

        inline bool operator<(const SetBindingPair& other) const
        {
            return desc_set < other.desc_set || (desc_set == other.desc_set && binding < other.binding);
        }
    };

    struct LocationComponentPair
    {
        uint32_t location;
        uint32_t component;

        inline bool operator==(const LocationComponentPair& other) const
        {
            return location == other.location && component == other.component;
        }

        inline bool operator<(const LocationComponentPair& other) const
        {
            return location < other.location || (location == other.location && component < other.component);
        }
    };

    struct StageSetBinding
    {
        spv::ExecutionModel model;
        uint32_t desc_set;
        uint32_t binding;

        inline bool operator==(const StageSetBinding& other) const
        {
            return model == other.model && desc_set == other.desc_set && binding == other.binding;
        }
    };

    struct InternalHasher
    {
        inline size_t operator()(const SetBindingPair& value) const
        {
            // Quality of hash doesn't really matter here.
            auto hash_set = std::hash<uint32_t>()(value.desc_set);
            auto hash_binding = std::hash<uint32_t>()(value.binding);
            return (hash_set * 0x10001b31) ^ hash_binding;
        }

        inline size_t operator()(const LocationComponentPair& value) const
        {
            // Quality of hash doesn't really matter here.
            auto hash_set = std::hash<uint32_t>()(value.location);
            auto hash_binding = std::hash<uint32_t>()(value.component);
            return (hash_set * 0x10001b31) ^ hash_binding;
        }

        inline size_t operator()(const StageSetBinding& value) const
        {
            // Quality of hash doesn't really matter here.
            auto hash_model = std::hash<uint32_t>()(value.model);
            auto hash_set = std::hash<uint32_t>()(value.desc_set);
            auto tmp_hash = (hash_model * 0x10001b31) ^ hash_set;
            return (tmp_hash * 0x10001b31) ^ value.binding;
        }
    };

    // Special constant used in a {MSL,HLSL}ResourceBinding desc_set
    // element to indicate the bindings for the push constants.
    static const uint32_t ResourceBindingPushConstantDescriptorSet = ~(0u);

    // Special constant used in a {MSL,HLSL}ResourceBinding binding
    // element to indicate the bindings for the push constants.
    static const uint32_t ResourceBindingPushConstantBinding = 0;
} // namespace SPIRV_CROSS_NAMESPACE

namespace std
{
    template <SPIRV_CROSS_NAMESPACE::Types type>
    struct hash<SPIRV_CROSS_NAMESPACE::TypedID<type>>
    {
        size_t operator()(const SPIRV_CROSS_NAMESPACE::TypedID<type>& value) const
        {
            return std::hash<uint32_t>()(value);
        }
    };
} // namespace std

#endif

#include <assert.h>

namespace SPIRV_CROSS_NAMESPACE
{
    class Compiler;
    class CFG
    {
    public:
        CFG(Compiler& compiler, const SPIRFunction& function);

        Compiler& get_compiler()
        {
            return compiler;
        }

        const Compiler& get_compiler() const
        {
            return compiler;
        }

        const SPIRFunction& get_function() const
        {
            return func;
        }

        uint32_t get_immediate_dominator(uint32_t block) const
        {
            auto itr = immediate_dominators.find(block);
            if (itr != std::end(immediate_dominators))
                return itr->second;
            else
                return 0;
        }

        bool is_reachable(uint32_t block) const
        {
            return visit_order.count(block) != 0;
        }

        uint32_t get_visit_order(uint32_t block) const
        {
            auto itr = visit_order.find(block);
            assert(itr != std::end(visit_order));
            int v = itr->second.get();
            assert(v > 0);
            return uint32_t(v);
        }

        uint32_t find_common_dominator(uint32_t a, uint32_t b) const;

        const SmallVector<uint32_t>& get_preceding_edges(uint32_t block) const
        {
            auto itr = preceding_edges.find(block);
            if (itr != std::end(preceding_edges))
                return itr->second;
            else
                return empty_vector;
        }

        const SmallVector<uint32_t>& get_succeeding_edges(uint32_t block) const
        {
            auto itr = succeeding_edges.find(block);
            if (itr != std::end(succeeding_edges))
                return itr->second;
            else
                return empty_vector;
        }

        template <typename Op>
        void walk_from(std::unordered_set<uint32_t>& seen_blocks, uint32_t block, const Op& op) const
        {
            if (seen_blocks.count(block))
                return;
            seen_blocks.insert(block);

            if (op(block))
            {
                for (auto b : get_succeeding_edges(block))
                    walk_from(seen_blocks, b, op);
            }
        }

        uint32_t find_loop_dominator(uint32_t block) const;

        bool node_terminates_control_flow_in_sub_graph(BlockID from, BlockID to) const;

    private:
        struct VisitOrder
        {
            int& get()
            {
                return v;
            }

            const int& get() const
            {
                return v;
            }

            int v = -1;
        };

        Compiler& compiler;
        const SPIRFunction& func;
        std::unordered_map<uint32_t, SmallVector<uint32_t>> preceding_edges;
        std::unordered_map<uint32_t, SmallVector<uint32_t>> succeeding_edges;
        std::unordered_map<uint32_t, uint32_t> immediate_dominators;
        std::unordered_map<uint32_t, VisitOrder> visit_order;
        SmallVector<uint32_t> post_order;
        SmallVector<uint32_t> empty_vector;

        void add_branch(uint32_t from, uint32_t to);
        void build_post_order_visit_order();
        void build_immediate_dominators();
        bool post_order_visit(uint32_t block);
        uint32_t visit_count = 0;

        bool is_back_edge(uint32_t to) const;
        bool has_visited_forward_edge(uint32_t to) const;
    };

    class DominatorBuilder
    {
    public:
        DominatorBuilder(const CFG& cfg);

        void add_block(uint32_t block);
        uint32_t get_dominator() const
        {
            return dominator;
        }

        void lift_continue_block_dominator();

    private:
        const CFG& cfg;
        uint32_t dominator = 0;
    };
} // namespace SPIRV_CROSS_NAMESPACE

#endif

#ifndef SPIRV_CROSS_PARSED_IR_HPP
#define SPIRV_CROSS_PARSED_IR_HPP

#include <stdint.h>
#include <unordered_map>

namespace SPIRV_CROSS_NAMESPACE
{

    // This data structure holds all information needed to perform cross-compilation and reflection.
    // It is the output of the Parser, but any implementation could create this structure.
    // It is intentionally very "open" and struct-like with some helper functions to deal with decorations.
    // Parser is the reference implementation of how this data structure should be filled in.

    class ParsedIR
    {
    private:
        // This must be destroyed after the "ids" vector.
        std::unique_ptr<ObjectPoolGroup> pool_group;

    public:
        ParsedIR();

        // Due to custom allocations from object pools, we cannot use a default copy constructor.
        ParsedIR(const ParsedIR& other);
        ParsedIR& operator=(const ParsedIR& other);

        // Moves are unproblematic, but we need to implement it anyways, since MSVC 2013 does not understand
        // how to default-implement these.
        ParsedIR(ParsedIR&& other) SPIRV_CROSS_NOEXCEPT;
        ParsedIR& operator=(ParsedIR&& other) SPIRV_CROSS_NOEXCEPT;

        // Resizes ids, meta and block_meta.
        void set_id_bounds(uint32_t bounds);

        // The raw SPIR-V, instructions and opcodes refer to this by offset + count.
        std::vector<uint32_t> spirv;

        // Holds various data structures which inherit from IVariant.
        SmallVector<Variant> ids;

        // Various meta data for IDs, decorations, names, etc.
        std::unordered_map<ID, Meta> meta;

        // Holds all IDs which have a certain type.
        // This is needed so we can iterate through a specific kind of resource quickly,
        // and in-order of module declaration.
        SmallVector<ID> ids_for_type[TypeCount];

        // Special purpose lists which contain a union of types.
        // This is needed so we can declare specialization constants and structs in an interleaved fashion,
        // among other things.
        // Constants can be undef or of struct type, and struct array sizes can use specialization constants.
        SmallVector<ID> ids_for_constant_undef_or_type;
        SmallVector<ID> ids_for_constant_or_variable;

        // We need to keep track of the width the Ops that contains a type for the
        // OpSwitch instruction, since this one doesn't contains the type in the
        // instruction itself. And in some case we need to cast the condition to
        // wider types. We only need the width to do the branch fixup since the
        // type check itself can be done at runtime
        std::unordered_map<ID, uint32_t> load_type_width;

        // Declared capabilities and extensions in the SPIR-V module.
        // Not really used except for reflection at the moment.
        SmallVector<spv::Capability> declared_capabilities;
        SmallVector<std::string> declared_extensions;

        // Meta data about blocks. The cross-compiler needs to query if a block is either of these types.
        // It is a bitset as there can be more than one tag per block.
        enum BlockMetaFlagBits
        {
            BLOCK_META_LOOP_HEADER_BIT = 1 << 0,
            BLOCK_META_CONTINUE_BIT = 1 << 1,
            BLOCK_META_LOOP_MERGE_BIT = 1 << 2,
            BLOCK_META_SELECTION_MERGE_BIT = 1 << 3,
            BLOCK_META_MULTISELECT_MERGE_BIT = 1 << 4
        };
        using BlockMetaFlags = uint8_t;
        SmallVector<BlockMetaFlags> block_meta;
        std::unordered_map<BlockID, BlockID> continue_block_to_loop_header;

        // Normally, we'd stick SPIREntryPoint in ids array, but it conflicts with SPIRFunction.
        // Entry points can therefore be seen as some sort of meta structure.
        std::unordered_map<FunctionID, SPIREntryPoint> entry_points;
        FunctionID default_entry_point = 0;

        struct Source
        {
            uint32_t version = 0;
            bool es = false;
            bool known = false;
            bool hlsl = false;

            Source() = default;
        };

        Source source;

        spv::AddressingModel addressing_model = spv::AddressingModelMax;
        spv::MemoryModel memory_model = spv::MemoryModelMax;

        // Decoration handling methods.
        // Can be useful for simple "raw" reflection.
        // However, most members are here because the Parser needs most of these,
        // and might as well just have the whole suite of decoration/name handling in one place.
        void set_name(ID id, const std::string& name);
        const std::string& get_name(ID id) const;
        void set_decoration(ID id, spv::Decoration decoration, uint32_t argument = 0);
        void set_decoration_string(ID id, spv::Decoration decoration, const std::string& argument);
        bool has_decoration(ID id, spv::Decoration decoration) const;
        uint32_t get_decoration(ID id, spv::Decoration decoration) const;
        const std::string& get_decoration_string(ID id, spv::Decoration decoration) const;
        const Bitset& get_decoration_bitset(ID id) const;
        void unset_decoration(ID id, spv::Decoration decoration);

        // Decoration handling methods (for members of a struct).
        void set_member_name(TypeID id, uint32_t index, const std::string& name);
        const std::string& get_member_name(TypeID id, uint32_t index) const;
        void set_member_decoration(TypeID id, uint32_t index, spv::Decoration decoration, uint32_t argument = 0);
        void set_member_decoration_string(TypeID id, uint32_t index, spv::Decoration decoration,
            const std::string& argument);
        uint32_t get_member_decoration(TypeID id, uint32_t index, spv::Decoration decoration) const;
        const std::string& get_member_decoration_string(TypeID id, uint32_t index, spv::Decoration decoration) const;
        bool has_member_decoration(TypeID id, uint32_t index, spv::Decoration decoration) const;
        const Bitset& get_member_decoration_bitset(TypeID id, uint32_t index) const;
        void unset_member_decoration(TypeID id, uint32_t index, spv::Decoration decoration);

        void mark_used_as_array_length(ID id);
        uint32_t increase_bound_by(uint32_t count);
        Bitset get_buffer_block_flags(const SPIRVariable& var) const;
        Bitset get_buffer_block_type_flags(const SPIRType& type) const;

        void add_typed_id(Types type, ID id);
        void remove_typed_id(Types type, ID id);

        class LoopLock
        {
        public:
            explicit LoopLock(uint32_t* counter);
            LoopLock(const LoopLock&) = delete;
            void operator=(const LoopLock&) = delete;
            LoopLock(LoopLock&& other) SPIRV_CROSS_NOEXCEPT;
            LoopLock& operator=(LoopLock&& other) SPIRV_CROSS_NOEXCEPT;
            ~LoopLock();

        private:
            uint32_t* lock;
        };

        // This must be held while iterating over a type ID array.
        // It is undefined if someone calls set<>() while we're iterating over a data structure, so we must
        // make sure that this case is avoided.

        // If we have a hard lock, it is an error to call set<>(), and an exception is thrown.
        // If we have a soft lock, we silently ignore any additions to the typed arrays.
        // This should only be used for physical ID remapping where we need to create an ID, but we will never
        // care about iterating over them.
        LoopLock create_loop_hard_lock() const;
        LoopLock create_loop_soft_lock() const;

        template <typename T, typename Op>
        void for_each_typed_id(const Op& op)
        {
            auto loop_lock = create_loop_hard_lock();
            for (auto& id : ids_for_type[T::type])
            {
                if (ids[id].get_type() == static_cast<Types>(T::type))
                    op(id, get<T>(id));
            }
        }

        template <typename T, typename Op>
        void for_each_typed_id(const Op& op) const
        {
            auto loop_lock = create_loop_hard_lock();
            for (auto& id : ids_for_type[T::type])
            {
                if (ids[id].get_type() == static_cast<Types>(T::type))
                    op(id, get<T>(id));
            }
        }

        template <typename T>
        void reset_all_of_type()
        {
            reset_all_of_type(static_cast<Types>(T::type));
        }

        void reset_all_of_type(Types type);

        Meta* find_meta(ID id);
        const Meta* find_meta(ID id) const;

        const std::string& get_empty_string() const
        {
            return empty_string;
        }

        void make_constant_null(uint32_t id, uint32_t type, bool add_to_typed_id_set);

        void fixup_reserved_names();

        static void sanitize_underscores(std::string& str);
        static void sanitize_identifier(std::string& str, bool member, bool allow_reserved_prefixes);
        static bool is_globally_reserved_identifier(std::string& str, bool allow_reserved_prefixes);

        uint32_t get_spirv_version() const;

    private:
        template <typename T>
        T& get(uint32_t id)
        {
            return variant_get<T>(ids[id]);
        }

        template <typename T>
        const T& get(uint32_t id) const
        {
            return variant_get<T>(ids[id]);
        }

        mutable uint32_t loop_iteration_depth_hard = 0;
        mutable uint32_t loop_iteration_depth_soft = 0;
        std::string empty_string;
        Bitset cleared_bitset;

        std::unordered_set<uint32_t> meta_needing_name_fixup;
    };
} // namespace SPIRV_CROSS_NAMESPACE

#endif


namespace SPIRV_CROSS_NAMESPACE
{
    struct Resource
    {
        // Resources are identified with their SPIR-V ID.
        // This is the ID of the OpVariable.
        ID id;

        // The type ID of the variable which includes arrays and all type modifications.
        // This type ID is not suitable for parsing OpMemberDecoration of a struct and other decorations in general
        // since these modifications typically happen on the base_type_id.
        TypeID type_id;

        // The base type of the declared resource.
        // This type is the base type which ignores pointers and arrays of the type_id.
        // This is mostly useful to parse decorations of the underlying type.
        // base_type_id can also be obtained with get_type(get_type(type_id).self).
        TypeID base_type_id;

        // The declared name (OpName) of the resource.
        // For Buffer blocks, the name actually reflects the externally
        // visible Block name.
        //
        // This name can be retrieved again by using either
        // get_name(id) or get_name(base_type_id) depending if it's a buffer block or not.
        //
        // This name can be an empty string in which case get_fallback_name(id) can be
        // used which obtains a suitable fallback identifier for an ID.
        std::string name;
    };

    struct BuiltInResource
    {
        // This is mostly here to support reflection of builtins such as Position/PointSize/CullDistance/ClipDistance.
        // This needs to be different from Resource since we can collect builtins from blocks.
        // A builtin present here does not necessarily mean it's considered an active builtin,
        // since variable ID "activeness" is only tracked on OpVariable level, not Block members.
        // For that, update_active_builtins() -> has_active_builtin() can be used to further refine the reflection.
        spv::BuiltIn builtin;

        // This is the actual value type of the builtin.
        // Typically float4, float, array<float, N> for the gl_PerVertex builtins.
        // If the builtin is a control point, the control point array type will be stripped away here as appropriate.
        TypeID value_type_id;

        // This refers to the base resource which contains the builtin.
        // If resource is a Block, it can hold multiple builtins, or it might not be a block.
        // For advanced reflection scenarios, all information in builtin/value_type_id can be deduced,
        // it's just more convenient this way.
        Resource resource;
    };

    struct ShaderResources
    {
        SmallVector<Resource> uniform_buffers;
        SmallVector<Resource> storage_buffers;
        SmallVector<Resource> stage_inputs;
        SmallVector<Resource> stage_outputs;
        SmallVector<Resource> subpass_inputs;
        SmallVector<Resource> storage_images;
        SmallVector<Resource> sampled_images;
        SmallVector<Resource> atomic_counters;
        SmallVector<Resource> acceleration_structures;

        // There can only be one push constant block,
        // but keep the vector in case this restriction is lifted in the future.
        SmallVector<Resource> push_constant_buffers;

        SmallVector<Resource> shader_record_buffers;

        // For Vulkan GLSL and HLSL source,
        // these correspond to separate texture2D and samplers respectively.
        SmallVector<Resource> separate_images;
        SmallVector<Resource> separate_samplers;

        SmallVector<BuiltInResource> builtin_inputs;
        SmallVector<BuiltInResource> builtin_outputs;
    };

    struct CombinedImageSampler
    {
        // The ID of the sampler2D variable.
        VariableID combined_id;
        // The ID of the texture2D variable.
        VariableID image_id;
        // The ID of the sampler variable.
        VariableID sampler_id;
    };

    struct SpecializationConstant
    {
        // The ID of the specialization constant.
        ConstantID id;
        // The constant ID of the constant, used in Vulkan during pipeline creation.
        uint32_t constant_id;
    };

    struct BufferRange
    {
        unsigned index;
        size_t offset;
        size_t range;
    };

    enum BufferPackingStandard
    {
        BufferPackingStd140,
        BufferPackingStd430,
        BufferPackingStd140EnhancedLayout,
        BufferPackingStd430EnhancedLayout,
        BufferPackingHLSLCbuffer,
        BufferPackingHLSLCbufferPackOffset,
        BufferPackingScalar,
        BufferPackingScalarEnhancedLayout
    };

    struct EntryPoint
    {
        std::string name;
        spv::ExecutionModel execution_model;
    };

    class Compiler
    {
    public:
        friend class CFG;
        friend class DominatorBuilder;

        // The constructor takes a buffer of SPIR-V words and parses it.
        // It will create its own parser, parse the SPIR-V and move the parsed IR
        // as if you had called the constructors taking ParsedIR directly.
        explicit Compiler(std::vector<uint32_t> ir);
        Compiler(const uint32_t* ir, size_t word_count);

        // This is more modular. We can also consume a ParsedIR structure directly, either as a move, or copy.
        // With copy, we can reuse the same parsed IR for multiple Compiler instances.
        explicit Compiler(const ParsedIR& ir);
        explicit Compiler(ParsedIR&& ir);

        virtual ~Compiler() = default;

        // After parsing, API users can modify the SPIR-V via reflection and call this
        // to disassemble the SPIR-V into the desired langauage.
        // Sub-classes actually implement this.
        virtual std::string compile();

        // Gets the identifier (OpName) of an ID. If not defined, an empty string will be returned.
        const std::string& get_name(ID id) const;

        // Applies a decoration to an ID. Effectively injects OpDecorate.
        void set_decoration(ID id, spv::Decoration decoration, uint32_t argument = 0);
        void set_decoration_string(ID id, spv::Decoration decoration, const std::string& argument);

        // Overrides the identifier OpName of an ID.
        // Identifiers beginning with underscores or identifiers which contain double underscores
        // are reserved by the implementation.
        void set_name(ID id, const std::string& name);

        // Gets a bitmask for the decorations which are applied to ID.
        // I.e. (1ull << spv::DecorationFoo) | (1ull << spv::DecorationBar)
        const Bitset& get_decoration_bitset(ID id) const;

        // Returns whether the decoration has been applied to the ID.
        bool has_decoration(ID id, spv::Decoration decoration) const;

        // Gets the value for decorations which take arguments.
        // If the decoration is a boolean (i.e. spv::DecorationNonWritable),
        // 1 will be returned.
        // If decoration doesn't exist or decoration is not recognized,
        // 0 will be returned.
        uint32_t get_decoration(ID id, spv::Decoration decoration) const;
        const std::string& get_decoration_string(ID id, spv::Decoration decoration) const;

        // Removes the decoration for an ID.
        void unset_decoration(ID id, spv::Decoration decoration);

        // Gets the SPIR-V type associated with ID.
        // Mostly used with Resource::type_id and Resource::base_type_id to parse the underlying type of a resource.
        const SPIRType& get_type(TypeID id) const;

        // Gets the SPIR-V type of a variable.
        const SPIRType& get_type_from_variable(VariableID id) const;

        // Gets the underlying storage class for an OpVariable.
        spv::StorageClass get_storage_class(VariableID id) const;

        // If get_name() is an empty string, get the fallback name which will be used
        // instead in the disassembled source.
        virtual const std::string get_fallback_name(ID id) const;

        // If get_name() of a Block struct is an empty string, get the fallback name.
        // This needs to be per-variable as multiple variables can use the same block type.
        virtual const std::string get_block_fallback_name(VariableID id) const;

        // Given an OpTypeStruct in ID, obtain the identifier for member number "index".
        // This may be an empty string.
        const std::string& get_member_name(TypeID id, uint32_t index) const;

        // Given an OpTypeStruct in ID, obtain the OpMemberDecoration for member number "index".
        uint32_t get_member_decoration(TypeID id, uint32_t index, spv::Decoration decoration) const;
        const std::string& get_member_decoration_string(TypeID id, uint32_t index, spv::Decoration decoration) const;

        // Sets the member identifier for OpTypeStruct ID, member number "index".
        void set_member_name(TypeID id, uint32_t index, const std::string& name);

        // Returns the qualified member identifier for OpTypeStruct ID, member number "index",
        // or an empty string if no qualified alias exists
        const std::string& get_member_qualified_name(TypeID type_id, uint32_t index) const;

        // Gets the decoration mask for a member of a struct, similar to get_decoration_mask.
        const Bitset& get_member_decoration_bitset(TypeID id, uint32_t index) const;

        // Returns whether the decoration has been applied to a member of a struct.
        bool has_member_decoration(TypeID id, uint32_t index, spv::Decoration decoration) const;

        // Similar to set_decoration, but for struct members.
        void set_member_decoration(TypeID id, uint32_t index, spv::Decoration decoration, uint32_t argument = 0);
        void set_member_decoration_string(TypeID id, uint32_t index, spv::Decoration decoration,
            const std::string& argument);

        // Unsets a member decoration, similar to unset_decoration.
        void unset_member_decoration(TypeID id, uint32_t index, spv::Decoration decoration);

        // Gets the fallback name for a member, similar to get_fallback_name.
        virtual const std::string get_fallback_member_name(uint32_t index) const
        {
            return join("_", index);
        }

        // Returns a vector of which members of a struct are potentially in use by a
        // SPIR-V shader. The granularity of this analysis is per-member of a struct.
        // This can be used for Buffer (UBO), BufferBlock/StorageBuffer (SSBO) and PushConstant blocks.
        // ID is the Resource::id obtained from get_shader_resources().
        SmallVector<BufferRange> get_active_buffer_ranges(VariableID id) const;

        // Returns the effective size of a buffer block.
        size_t get_declared_struct_size(const SPIRType& struct_type) const;

        // Returns the effective size of a buffer block, with a given array size
        // for a runtime array.
        // SSBOs are typically declared as runtime arrays. get_declared_struct_size() will return 0 for the size.
        // This is not very helpful for applications which might need to know the array stride of its last member.
        // This can be done through the API, but it is not very intuitive how to accomplish this, so here we provide a helper function
        // to query the size of the buffer, assuming that the last member has a certain size.
        // If the buffer does not contain a runtime array, array_size is ignored, and the function will behave as
        // get_declared_struct_size().
        // To get the array stride of the last member, something like:
        // get_declared_struct_size_runtime_array(type, 1) - get_declared_struct_size_runtime_array(type, 0) will work.
        size_t get_declared_struct_size_runtime_array(const SPIRType& struct_type, size_t array_size) const;

        // Returns the effective size of a buffer block struct member.
        size_t get_declared_struct_member_size(const SPIRType& struct_type, uint32_t index) const;

        // Returns a set of all global variables which are statically accessed
        // by the control flow graph from the current entry point.
        // Only variables which change the interface for a shader are returned, that is,
        // variables with storage class of Input, Output, Uniform, UniformConstant, PushConstant and AtomicCounter
        // storage classes are returned.
        //
        // To use the returned set as the filter for which variables are used during compilation,
        // this set can be moved to set_enabled_interface_variables().
        std::unordered_set<VariableID> get_active_interface_variables() const;

        // Sets the interface variables which are used during compilation.
        // By default, all variables are used.
        // Once set, compile() will only consider the set in active_variables.
        void set_enabled_interface_variables(std::unordered_set<VariableID> active_variables);

        // Query shader resources, use ids with reflection interface to modify or query binding points, etc.
        ShaderResources get_shader_resources() const;

        // Query shader resources, but only return the variables which are part of active_variables.
        // E.g.: get_shader_resources(get_active_variables()) to only return the variables which are statically
        // accessed.
        ShaderResources get_shader_resources(const std::unordered_set<VariableID>& active_variables) const;

        // Remapped variables are considered built-in variables and a backend will
        // not emit a declaration for this variable.
        // This is mostly useful for making use of builtins which are dependent on extensions.
        void set_remapped_variable_state(VariableID id, bool remap_enable);
        bool get_remapped_variable_state(VariableID id) const;

        // For subpassInput variables which are remapped to plain variables,
        // the number of components in the remapped
        // variable must be specified as the backing type of subpass inputs are opaque.
        void set_subpass_input_remapped_components(VariableID id, uint32_t components);
        uint32_t get_subpass_input_remapped_components(VariableID id) const;

        // All operations work on the current entry point.
        // Entry points can be swapped out with set_entry_point().
        // Entry points should be set right after the constructor completes as some reflection functions traverse the graph from the entry point.
        // Resource reflection also depends on the entry point.
        // By default, the current entry point is set to the first OpEntryPoint which appears in the SPIR-V module.

        // Some shader languages restrict the names that can be given to entry points, and the
        // corresponding backend will automatically rename an entry point name, during the call
        // to compile() if it is illegal. For example, the common entry point name main() is
        // illegal in MSL, and is renamed to an alternate name by the MSL backend.
        // Given the original entry point name contained in the SPIR-V, this function returns
        // the name, as updated by the backend during the call to compile(). If the name is not
        // illegal, and has not been renamed, or if this function is called before compile(),
        // this function will simply return the same name.

        // New variants of entry point query and reflection.
        // Names for entry points in the SPIR-V module may alias if they belong to different execution models.
        // To disambiguate, we must pass along with the entry point names the execution model.
        SmallVector<EntryPoint> get_entry_points_and_stages() const;
        void set_entry_point(const std::string& entry, spv::ExecutionModel execution_model);

        // Renames an entry point from old_name to new_name.
        // If old_name is currently selected as the current entry point, it will continue to be the current entry point,
        // albeit with a new name.
        // get_entry_points() is essentially invalidated at this point.
        void rename_entry_point(const std::string& old_name, const std::string& new_name,
            spv::ExecutionModel execution_model);
        const SPIREntryPoint& get_entry_point(const std::string& name, spv::ExecutionModel execution_model) const;
        SPIREntryPoint& get_entry_point(const std::string& name, spv::ExecutionModel execution_model);
        const std::string& get_cleansed_entry_point_name(const std::string& name,
            spv::ExecutionModel execution_model) const;

        // Traverses all reachable opcodes and sets active_builtins to a bitmask of all builtin variables which are accessed in the shader.
        void update_active_builtins();
        bool has_active_builtin(spv::BuiltIn builtin, spv::StorageClass storage) const;

        // Query and modify OpExecutionMode.
        const Bitset& get_execution_mode_bitset() const;

        void unset_execution_mode(spv::ExecutionMode mode);
        void set_execution_mode(spv::ExecutionMode mode, uint32_t arg0 = 0, uint32_t arg1 = 0, uint32_t arg2 = 0);

        // Gets argument for an execution mode (LocalSize, Invocations, OutputVertices).
        // For LocalSize or LocalSizeId, the index argument is used to select the dimension (X = 0, Y = 1, Z = 2).
        // For execution modes which do not have arguments, 0 is returned.
        // LocalSizeId query returns an ID. If LocalSizeId execution mode is not used, it returns 0.
        // LocalSize always returns a literal. If execution mode is LocalSizeId,
        // the literal (spec constant or not) is still returned.
        uint32_t get_execution_mode_argument(spv::ExecutionMode mode, uint32_t index = 0) const;
        spv::ExecutionModel get_execution_model() const;

        bool is_tessellation_shader() const;
        bool is_tessellating_triangles() const;

        // In SPIR-V, the compute work group size can be represented by a constant vector, in which case
        // the LocalSize execution mode is ignored.
        //
        // This constant vector can be a constant vector, specialization constant vector, or partly specialized constant vector.
        // To modify and query work group dimensions which are specialization constants, SPIRConstant values must be modified
        // directly via get_constant() rather than using LocalSize directly. This function will return which constants should be modified.
        //
        // To modify dimensions which are *not* specialization constants, set_execution_mode should be used directly.
        // Arguments to set_execution_mode which are specialization constants are effectively ignored during compilation.
        // NOTE: This is somewhat different from how SPIR-V works. In SPIR-V, the constant vector will completely replace LocalSize,
        // while in this interface, LocalSize is only ignored for specialization constants.
        //
        // The specialization constant will be written to x, y and z arguments.
        // If the component is not a specialization constant, a zeroed out struct will be written.
        // The return value is the constant ID of the builtin WorkGroupSize, but this is not expected to be useful
        // for most use cases.
        // If LocalSizeId is used, there is no uvec3 value representing the workgroup size, so the return value is 0,
        // but x, y and z are written as normal if the components are specialization constants.
        uint32_t get_work_group_size_specialization_constants(SpecializationConstant& x, SpecializationConstant& y,
            SpecializationConstant& z) const;

        // Analyzes all OpImageFetch (texelFetch) opcodes and checks if there are instances where
        // said instruction is used without a combined image sampler.
        // GLSL targets do not support the use of texelFetch without a sampler.
        // To workaround this, we must inject a dummy sampler which can be used to form a sampler2D at the call-site of
        // texelFetch as necessary.
        //
        // This must be called before build_combined_image_samplers().
        // build_combined_image_samplers() may refer to the ID returned by this method if the returned ID is non-zero.
        // The return value will be the ID of a sampler object if a dummy sampler is necessary, or 0 if no sampler object
        // is required.
        //
        // If the returned ID is non-zero, it can be decorated with set/bindings as desired before calling compile().
        // Calling this function also invalidates get_active_interface_variables(), so this should be called
        // before that function.
        VariableID build_dummy_sampler_for_combined_images();

        // Analyzes all separate image and samplers used from the currently selected entry point,
        // and re-routes them all to a combined image sampler instead.
        // This is required to "support" separate image samplers in targets which do not natively support
        // this feature, like GLSL/ESSL.
        //
        // This must be called before compile() if such remapping is desired.
        // This call will add new sampled images to the SPIR-V,
        // so it will appear in reflection if get_shader_resources() is called after build_combined_image_samplers.
        //
        // If any image/sampler remapping was found, no separate image/samplers will appear in the decompiled output,
        // but will still appear in reflection.
        //
        // The resulting samplers will be void of any decorations like name, descriptor sets and binding points,
        // so this can be added before compile() if desired.
        //
        // Combined image samplers originating from this set are always considered active variables.
        // Arrays of separate samplers are not supported, but arrays of separate images are supported.
        // Array of images + sampler -> Array of combined image samplers.
        void build_combined_image_samplers();

        // Gets a remapping for the combined image samplers.
        const SmallVector<CombinedImageSampler>& get_combined_image_samplers() const
        {
            return combined_image_samplers;
        }

        // Set a new variable type remap callback.
        // The type remapping is designed to allow global interface variable to assume more special types.
        // A typical example here is to remap sampler2D into samplerExternalOES, which currently isn't supported
        // directly by SPIR-V.
        //
        // In compile() while emitting code,
        // for every variable that is declared, including function parameters, the callback will be called
        // and the API user has a chance to change the textual representation of the type used to declare the variable.
        // The API user can detect special patterns in names to guide the remapping.
        void set_variable_type_remap_callback(VariableTypeRemapCallback cb)
        {
            variable_remap_callback = std::move(cb);
        }

        // API for querying which specialization constants exist.
        // To modify a specialization constant before compile(), use get_constant(constant.id),
        // then update constants directly in the SPIRConstant data structure.
        // For composite types, the subconstants can be iterated over and modified.
        // constant_type is the SPIRType for the specialization constant,
        // which can be queried to determine which fields in the unions should be poked at.
        SmallVector<SpecializationConstant> get_specialization_constants() const;
        SPIRConstant& get_constant(ConstantID id);
        const SPIRConstant& get_constant(ConstantID id) const;

        uint32_t get_current_id_bound() const
        {
            return uint32_t(ir.ids.size());
        }

        // API for querying buffer objects.
        // The type passed in here should be the base type of a resource, i.e.
        // get_type(resource.base_type_id)
        // as decorations are set in the basic Block type.
        // The type passed in here must have these decorations set, or an exception is raised.
        // Only UBOs and SSBOs or sub-structs which are part of these buffer types will have these decorations set.
        uint32_t type_struct_member_offset(const SPIRType& type, uint32_t index) const;
        uint32_t type_struct_member_array_stride(const SPIRType& type, uint32_t index) const;
        uint32_t type_struct_member_matrix_stride(const SPIRType& type, uint32_t index) const;

        // Gets the offset in SPIR-V words (uint32_t) for a decoration which was originally declared in the SPIR-V binary.
        // The offset will point to one or more uint32_t literals which can be modified in-place before using the SPIR-V binary.
        // Note that adding or removing decorations using the reflection API will not change the behavior of this function.
        // If the decoration was declared, sets the word_offset to an offset into the provided SPIR-V binary buffer and returns true,
        // otherwise, returns false.
        // If the decoration does not have any value attached to it (e.g. DecorationRelaxedPrecision), this function will also return false.
        bool get_binary_offset_for_decoration(VariableID id, spv::Decoration decoration, uint32_t& word_offset) const;

        // HLSL counter buffer reflection interface.
        // Append/Consume/Increment/Decrement in HLSL is implemented as two "neighbor" buffer objects where
        // one buffer implements the storage, and a single buffer containing just a lone "int" implements the counter.
        // To SPIR-V these will be exposed as two separate buffers, but glslang HLSL frontend emits a special indentifier
        // which lets us link the two buffers together.

        // Queries if a variable ID is a counter buffer which "belongs" to a regular buffer object.

        // If SPV_GOOGLE_hlsl_functionality1 is used, this can be used even with a stripped SPIR-V module.
        // Otherwise, this query is purely based on OpName identifiers as found in the SPIR-V module, and will
        // only return true if OpSource was reported HLSL.
        // To rely on this functionality, ensure that the SPIR-V module is not stripped.

        bool buffer_is_hlsl_counter_buffer(VariableID id) const;

        // Queries if a buffer object has a neighbor "counter" buffer.
        // If so, the ID of that counter buffer will be returned in counter_id.
        // If SPV_GOOGLE_hlsl_functionality1 is used, this can be used even with a stripped SPIR-V module.
        // Otherwise, this query is purely based on OpName identifiers as found in the SPIR-V module, and will
        // only return true if OpSource was reported HLSL.
        // To rely on this functionality, ensure that the SPIR-V module is not stripped.
        bool buffer_get_hlsl_counter_buffer(VariableID id, uint32_t& counter_id) const;

        // Gets the list of all SPIR-V Capabilities which were declared in the SPIR-V module.
        const SmallVector<spv::Capability>& get_declared_capabilities() const;

        // Gets the list of all SPIR-V extensions which were declared in the SPIR-V module.
        const SmallVector<std::string>& get_declared_extensions() const;

        // When declaring buffer blocks in GLSL, the name declared in the GLSL source
        // might not be the same as the name declared in the SPIR-V module due to naming conflicts.
        // In this case, SPIRV-Cross needs to find a fallback-name, and it might only
        // be possible to know this name after compiling to GLSL.
        // This is particularly important for HLSL input and UAVs which tends to reuse the same block type
        // for multiple distinct blocks. For these cases it is not possible to modify the name of the type itself
        // because it might be unique. Instead, you can use this interface to check after compilation which
        // name was actually used if your input SPIR-V tends to have this problem.
        // For other names like remapped names for variables, etc, it's generally enough to query the name of the variables
        // after compiling, block names are an exception to this rule.
        // ID is the name of a variable as returned by Resource::id, and must be a variable with a Block-like type.
        //
        // This also applies to HLSL cbuffers.
        std::string get_remapped_declared_block_name(VariableID id) const;

        // For buffer block variables, get the decorations for that variable.
        // Sometimes, decorations for buffer blocks are found in member decorations instead
        // of direct decorations on the variable itself.
        // The most common use here is to check if a buffer is readonly or writeonly.
        Bitset get_buffer_block_flags(VariableID id) const;

        // Returns whether the position output is invariant
        bool is_position_invariant() const
        {
            return position_invariant;
        }

    protected:
        const uint32_t* stream(const Instruction& instr) const
        {
            // If we're not going to use any arguments, just return nullptr.
            // We want to avoid case where we return an out of range pointer
            // that trips debug assertions on some platforms.
            if (!instr.length)
                return nullptr;

            if (instr.is_embedded())
            {
                auto& embedded = static_cast<const EmbeddedInstruction&>(instr);
                assert(embedded.ops.size() == instr.length);
                return embedded.ops.data();
            }
            else
            {
                if (instr.offset + instr.length > ir.spirv.size())
                    SPIRV_CROSS_THROW("Compiler::stream() out of range.");
                return &ir.spirv[instr.offset];
            }
        }

        uint32_t* stream_mutable(const Instruction& instr) const
        {
            return const_cast<uint32_t*>(stream(instr));
        }

        ParsedIR ir;
        // Marks variables which have global scope and variables which can alias with other variables
        // (SSBO, image load store, etc)
        SmallVector<uint32_t> global_variables;
        SmallVector<uint32_t> aliased_variables;

        SPIRFunction* current_function = nullptr;
        SPIRBlock* current_block = nullptr;
        uint32_t current_loop_level = 0;
        std::unordered_set<VariableID> active_interface_variables;
        bool check_active_interface_variables = false;

        void add_loop_level();

        void set_initializers(SPIRExpression& e)
        {
            e.emitted_loop_level = current_loop_level;
        }

        template <typename T>
        void set_initializers(const T&)
        {
        }

        // If our IDs are out of range here as part of opcodes, throw instead of
        // undefined behavior.
        template <typename T, typename... P>
        T& set(uint32_t id, P &&... args)
        {
            ir.add_typed_id(static_cast<Types>(T::type), id);
            auto& var = variant_set<T>(ir.ids[id], std::forward<P>(args)...);
            var.self = id;
            set_initializers(var);
            return var;
        }

        template <typename T>
        T& get(uint32_t id)
        {
            return variant_get<T>(ir.ids[id]);
        }

        template <typename T>
        T* maybe_get(uint32_t id)
        {
            if (id >= ir.ids.size())
                return nullptr;
            else if (ir.ids[id].get_type() == static_cast<Types>(T::type))
                return &get<T>(id);
            else
                return nullptr;
        }

        template <typename T>
        const T& get(uint32_t id) const
        {
            return variant_get<T>(ir.ids[id]);
        }

        template <typename T>
        const T* maybe_get(uint32_t id) const
        {
            if (id >= ir.ids.size())
                return nullptr;
            else if (ir.ids[id].get_type() == static_cast<Types>(T::type))
                return &get<T>(id);
            else
                return nullptr;
        }

        // Gets the id of SPIR-V type underlying the given type_id, which might be a pointer.
        uint32_t get_pointee_type_id(uint32_t type_id) const;

        // Gets the SPIR-V type underlying the given type, which might be a pointer.
        const SPIRType& get_pointee_type(const SPIRType& type) const;

        // Gets the SPIR-V type underlying the given type_id, which might be a pointer.
        const SPIRType& get_pointee_type(uint32_t type_id) const;

        // Gets the ID of the SPIR-V type underlying a variable.
        uint32_t get_variable_data_type_id(const SPIRVariable& var) const;

        // Gets the SPIR-V type underlying a variable.
        SPIRType& get_variable_data_type(const SPIRVariable& var);

        // Gets the SPIR-V type underlying a variable.
        const SPIRType& get_variable_data_type(const SPIRVariable& var) const;

        // Gets the SPIR-V element type underlying an array variable.
        SPIRType& get_variable_element_type(const SPIRVariable& var);

        // Gets the SPIR-V element type underlying an array variable.
        const SPIRType& get_variable_element_type(const SPIRVariable& var) const;

        // Sets the qualified member identifier for OpTypeStruct ID, member number "index".
        void set_member_qualified_name(uint32_t type_id, uint32_t index, const std::string& name);
        void set_qualified_name(uint32_t id, const std::string& name);

        // Returns if the given type refers to a sampled image.
        bool is_sampled_image_type(const SPIRType& type);

        const SPIREntryPoint& get_entry_point() const;
        SPIREntryPoint& get_entry_point();
        static bool is_tessellation_shader(spv::ExecutionModel model);

        virtual std::string to_name(uint32_t id, bool allow_alias = true) const;
        bool is_builtin_variable(const SPIRVariable& var) const;
        bool is_builtin_type(const SPIRType& type) const;
        bool is_hidden_variable(const SPIRVariable& var, bool include_builtins = false) const;
        bool is_immutable(uint32_t id) const;
        bool is_member_builtin(const SPIRType& type, uint32_t index, spv::BuiltIn* builtin) const;
        bool is_scalar(const SPIRType& type) const;
        bool is_vector(const SPIRType& type) const;
        bool is_matrix(const SPIRType& type) const;
        bool is_array(const SPIRType& type) const;
        uint32_t expression_type_id(uint32_t id) const;
        const SPIRType& expression_type(uint32_t id) const;
        bool expression_is_lvalue(uint32_t id) const;
        bool variable_storage_is_aliased(const SPIRVariable& var);
        SPIRVariable* maybe_get_backing_variable(uint32_t chain);

        void register_read(uint32_t expr, uint32_t chain, bool forwarded);
        void register_write(uint32_t chain);

        inline bool is_continue(uint32_t next) const
        {
            return (ir.block_meta[next] & ParsedIR::BLOCK_META_CONTINUE_BIT) != 0;
        }

        inline bool is_single_block_loop(uint32_t next) const
        {
            auto& block = get<SPIRBlock>(next);
            return block.merge == SPIRBlock::MergeLoop && block.continue_block == ID(next);
        }

        inline bool is_break(uint32_t next) const
        {
            return (ir.block_meta[next] &
                (ParsedIR::BLOCK_META_LOOP_MERGE_BIT | ParsedIR::BLOCK_META_MULTISELECT_MERGE_BIT)) != 0;
        }

        inline bool is_loop_break(uint32_t next) const
        {
            return (ir.block_meta[next] & ParsedIR::BLOCK_META_LOOP_MERGE_BIT) != 0;
        }

        inline bool is_conditional(uint32_t next) const
        {
            return (ir.block_meta[next] &
                (ParsedIR::BLOCK_META_SELECTION_MERGE_BIT | ParsedIR::BLOCK_META_MULTISELECT_MERGE_BIT)) != 0;
        }

        // Dependency tracking for temporaries read from variables.
        void flush_dependees(SPIRVariable& var);
        void flush_all_active_variables();
        void flush_control_dependent_expressions(uint32_t block);
        void flush_all_atomic_capable_variables();
        void flush_all_aliased_variables();
        void register_global_read_dependencies(const SPIRBlock& func, uint32_t id);
        void register_global_read_dependencies(const SPIRFunction& func, uint32_t id);
        std::unordered_set<uint32_t> invalid_expressions;

        void update_name_cache(std::unordered_set<std::string>& cache, std::string& name);

        // A variant which takes two sets of names. The secondary is only used to verify there are no collisions,
        // but the set is not updated when we have found a new name.
        // Used primarily when adding block interface names.
        void update_name_cache(std::unordered_set<std::string>& cache_primary,
            const std::unordered_set<std::string>& cache_secondary, std::string& name);

        bool function_is_pure(const SPIRFunction& func);
        bool block_is_pure(const SPIRBlock& block);

        bool execution_is_branchless(const SPIRBlock& from, const SPIRBlock& to) const;
        bool execution_is_direct_branch(const SPIRBlock& from, const SPIRBlock& to) const;
        bool execution_is_noop(const SPIRBlock& from, const SPIRBlock& to) const;
        SPIRBlock::ContinueBlockType continue_block_type(const SPIRBlock& continue_block) const;

        void force_recompile();
        void force_recompile_guarantee_forward_progress();
        void clear_force_recompile();
        bool is_forcing_recompilation() const;
        bool is_force_recompile = false;
        bool is_force_recompile_forward_progress = false;

        bool block_is_noop(const SPIRBlock& block) const;
        bool block_is_loop_candidate(const SPIRBlock& block, SPIRBlock::Method method) const;

        bool types_are_logically_equivalent(const SPIRType& a, const SPIRType& b) const;
        void inherit_expression_dependencies(uint32_t dst, uint32_t source);
        void add_implied_read_expression(SPIRExpression& e, uint32_t source);
        void add_implied_read_expression(SPIRAccessChain& e, uint32_t source);
        void add_active_interface_variable(uint32_t var_id);

        // For proper multiple entry point support, allow querying if an Input or Output
        // variable is part of that entry points interface.
        bool interface_variable_exists_in_entry_point(uint32_t id) const;

        SmallVector<CombinedImageSampler> combined_image_samplers;

        void remap_variable_type_name(const SPIRType& type, const std::string& var_name, std::string& type_name) const
        {
            if (variable_remap_callback)
                variable_remap_callback(type, var_name, type_name);
        }

        void set_ir(const ParsedIR& parsed);
        void set_ir(ParsedIR&& parsed);
        void parse_fixup();

        // Used internally to implement various traversals for queries.
        struct OpcodeHandler
        {
            virtual ~OpcodeHandler() = default;

            // Return true if traversal should continue.
            // If false, traversal will end immediately.
            virtual bool handle(spv::Op opcode, const uint32_t* args, uint32_t length) = 0;
            virtual bool handle_terminator(const SPIRBlock&)
            {
                return true;
            }

            virtual bool follow_function_call(const SPIRFunction&)
            {
                return true;
            }

            virtual void set_current_block(const SPIRBlock&)
            {
            }

            // Called after returning from a function or when entering a block,
            // can be called multiple times per block,
            // while set_current_block is only called on block entry.
            virtual void rearm_current_block(const SPIRBlock&)
            {
            }

            virtual bool begin_function_scope(const uint32_t*, uint32_t)
            {
                return true;
            }

            virtual bool end_function_scope(const uint32_t*, uint32_t)
            {
                return true;
            }
        };

        struct BufferAccessHandler : OpcodeHandler
        {
            BufferAccessHandler(const Compiler& compiler_, SmallVector<BufferRange>& ranges_, uint32_t id_)
                : compiler(compiler_)
                , ranges(ranges_)
                , id(id_)
            {
            }

            bool handle(spv::Op opcode, const uint32_t* args, uint32_t length) override;

            const Compiler& compiler;
            SmallVector<BufferRange>& ranges;
            uint32_t id;

            std::unordered_set<uint32_t> seen;
        };

        struct InterfaceVariableAccessHandler : OpcodeHandler
        {
            InterfaceVariableAccessHandler(const Compiler& compiler_, std::unordered_set<VariableID>& variables_)
                : compiler(compiler_)
                , variables(variables_)
            {
            }

            bool handle(spv::Op opcode, const uint32_t* args, uint32_t length) override;

            const Compiler& compiler;
            std::unordered_set<VariableID>& variables;
        };

        struct CombinedImageSamplerHandler : OpcodeHandler
        {
            CombinedImageSamplerHandler(Compiler& compiler_)
                : compiler(compiler_)
            {
            }
            bool handle(spv::Op opcode, const uint32_t* args, uint32_t length) override;
            bool begin_function_scope(const uint32_t* args, uint32_t length) override;
            bool end_function_scope(const uint32_t* args, uint32_t length) override;

            Compiler& compiler;

            // Each function in the call stack needs its own remapping for parameters so we can deduce which global variable each texture/sampler the parameter is statically bound to.
            std::stack<std::unordered_map<uint32_t, uint32_t>> parameter_remapping;
            std::stack<SPIRFunction*> functions;

            uint32_t remap_parameter(uint32_t id);
            void push_remap_parameters(const SPIRFunction& func, const uint32_t* args, uint32_t length);
            void pop_remap_parameters();
            void register_combined_image_sampler(SPIRFunction& caller, VariableID combined_id, VariableID texture_id,
                VariableID sampler_id, bool depth);
        };

        struct DummySamplerForCombinedImageHandler : OpcodeHandler
        {
            DummySamplerForCombinedImageHandler(Compiler& compiler_)
                : compiler(compiler_)
            {
            }
            bool handle(spv::Op opcode, const uint32_t* args, uint32_t length) override;

            Compiler& compiler;
            bool need_dummy_sampler = false;
        };

        struct ActiveBuiltinHandler : OpcodeHandler
        {
            ActiveBuiltinHandler(Compiler& compiler_)
                : compiler(compiler_)
            {
            }

            bool handle(spv::Op opcode, const uint32_t* args, uint32_t length) override;
            Compiler& compiler;

            void handle_builtin(const SPIRType& type, spv::BuiltIn builtin, const Bitset& decoration_flags);
            void add_if_builtin(uint32_t id);
            void add_if_builtin_or_block(uint32_t id);
            void add_if_builtin(uint32_t id, bool allow_blocks);
        };

        bool traverse_all_reachable_opcodes(const SPIRBlock& block, OpcodeHandler& handler) const;
        bool traverse_all_reachable_opcodes(const SPIRFunction& block, OpcodeHandler& handler) const;
        // This must be an ordered data structure so we always pick the same type aliases.
        SmallVector<uint32_t> global_struct_cache;

        ShaderResources get_shader_resources(const std::unordered_set<VariableID>* active_variables) const;

        VariableTypeRemapCallback variable_remap_callback;

        bool get_common_basic_type(const SPIRType& type, SPIRType::BaseType& base_type);

        std::unordered_set<uint32_t> forced_temporaries;
        std::unordered_set<uint32_t> forwarded_temporaries;
        std::unordered_set<uint32_t> suppressed_usage_tracking;
        std::unordered_set<uint32_t> hoisted_temporaries;
        std::unordered_set<uint32_t> forced_invariant_temporaries;

        Bitset active_input_builtins;
        Bitset active_output_builtins;
        uint32_t clip_distance_count = 0;
        uint32_t cull_distance_count = 0;
        bool position_invariant = false;

        void analyze_parameter_preservation(
            SPIRFunction& entry, const CFG& cfg,
            const std::unordered_map<uint32_t, std::unordered_set<uint32_t>>& variable_to_blocks,
            const std::unordered_map<uint32_t, std::unordered_set<uint32_t>>& complete_write_blocks);

        // If a variable ID or parameter ID is found in this set, a sampler is actually a shadow/comparison sampler.
        // SPIR-V does not support this distinction, so we must keep track of this information outside the type system.
        // There might be unrelated IDs found in this set which do not correspond to actual variables.
        // This set should only be queried for the existence of samplers which are already known to be variables or parameter IDs.
        // Similar is implemented for images, as well as if subpass inputs are needed.
        std::unordered_set<uint32_t> comparison_ids;
        bool need_subpass_input = false;
        bool need_subpass_input_ms = false;

        // In certain backends, we will need to use a dummy sampler to be able to emit code.
        // GLSL does not support texelFetch on texture2D objects, but SPIR-V does,
        // so we need to workaround by having the application inject a dummy sampler.
        uint32_t dummy_sampler_id = 0;

        void analyze_image_and_sampler_usage();

        struct CombinedImageSamplerDrefHandler : OpcodeHandler
        {
            CombinedImageSamplerDrefHandler(Compiler& compiler_)
                : compiler(compiler_)
            {
            }
            bool handle(spv::Op opcode, const uint32_t* args, uint32_t length) override;

            Compiler& compiler;
            std::unordered_set<uint32_t> dref_combined_samplers;
        };

        struct CombinedImageSamplerUsageHandler : OpcodeHandler
        {
            CombinedImageSamplerUsageHandler(Compiler& compiler_,
                const std::unordered_set<uint32_t>& dref_combined_samplers_)
                : compiler(compiler_)
                , dref_combined_samplers(dref_combined_samplers_)
            {
            }

            bool begin_function_scope(const uint32_t* args, uint32_t length) override;
            bool handle(spv::Op opcode, const uint32_t* args, uint32_t length) override;
            Compiler& compiler;
            const std::unordered_set<uint32_t>& dref_combined_samplers;

            std::unordered_map<uint32_t, std::unordered_set<uint32_t>> dependency_hierarchy;
            std::unordered_set<uint32_t> comparison_ids;

            void add_hierarchy_to_comparison_ids(uint32_t ids);
            bool need_subpass_input = false;
            bool need_subpass_input_ms = false;
            void add_dependency(uint32_t dst, uint32_t src);
        };

        void build_function_control_flow_graphs_and_analyze();
        std::unordered_map<uint32_t, std::unique_ptr<CFG>> function_cfgs;
        const CFG& get_cfg_for_current_function() const;
        const CFG& get_cfg_for_function(uint32_t id) const;

        struct CFGBuilder : OpcodeHandler
        {
            explicit CFGBuilder(Compiler& compiler_);

            bool follow_function_call(const SPIRFunction& func) override;
            bool handle(spv::Op op, const uint32_t* args, uint32_t length) override;
            Compiler& compiler;
            std::unordered_map<uint32_t, std::unique_ptr<CFG>> function_cfgs;
        };

        struct AnalyzeVariableScopeAccessHandler : OpcodeHandler
        {
            AnalyzeVariableScopeAccessHandler(Compiler& compiler_, SPIRFunction& entry_);

            bool follow_function_call(const SPIRFunction&) override;
            void set_current_block(const SPIRBlock& block) override;

            void notify_variable_access(uint32_t id, uint32_t block);
            bool id_is_phi_variable(uint32_t id) const;
            bool id_is_potential_temporary(uint32_t id) const;
            bool handle(spv::Op op, const uint32_t* args, uint32_t length) override;
            bool handle_terminator(const SPIRBlock& block) override;

            Compiler& compiler;
            SPIRFunction& entry;
            std::unordered_map<uint32_t, std::unordered_set<uint32_t>> accessed_variables_to_block;
            std::unordered_map<uint32_t, std::unordered_set<uint32_t>> accessed_temporaries_to_block;
            std::unordered_map<uint32_t, uint32_t> result_id_to_type;
            std::unordered_map<uint32_t, std::unordered_set<uint32_t>> complete_write_variables_to_block;
            std::unordered_map<uint32_t, std::unordered_set<uint32_t>> partial_write_variables_to_block;
            std::unordered_set<uint32_t> access_chain_expressions;
            // Access chains used in multiple blocks mean hoisting all the variables used to construct the access chain as not all backends can use pointers.
            // This is also relevant when forwarding opaque objects since we cannot lower these to temporaries.
            std::unordered_map<uint32_t, std::unordered_set<uint32_t>> rvalue_forward_children;
            const SPIRBlock* current_block = nullptr;
        };

        struct StaticExpressionAccessHandler : OpcodeHandler
        {
            StaticExpressionAccessHandler(Compiler& compiler_, uint32_t variable_id_);
            bool follow_function_call(const SPIRFunction&) override;
            bool handle(spv::Op op, const uint32_t* args, uint32_t length) override;

            Compiler& compiler;
            uint32_t variable_id;
            uint32_t static_expression = 0;
            uint32_t write_count = 0;
        };

        struct PhysicalBlockMeta
        {
            uint32_t alignment = 0;
        };

        struct PhysicalStorageBufferPointerHandler : OpcodeHandler
        {
            explicit PhysicalStorageBufferPointerHandler(Compiler& compiler_);
            bool handle(spv::Op op, const uint32_t* args, uint32_t length) override;
            Compiler& compiler;

            std::unordered_set<uint32_t> non_block_types;
            std::unordered_map<uint32_t, PhysicalBlockMeta> physical_block_type_meta;
            std::unordered_map<uint32_t, PhysicalBlockMeta*> access_chain_to_physical_block;

            void mark_aligned_access(uint32_t id, const uint32_t* args, uint32_t length);
            PhysicalBlockMeta* find_block_meta(uint32_t id) const;
            bool type_is_bda_block_entry(uint32_t type_id) const;
            void setup_meta_chain(uint32_t type_id, uint32_t var_id);
            uint32_t get_minimum_scalar_alignment(const SPIRType& type) const;
            void analyze_non_block_types_from_block(const SPIRType& type);
            uint32_t get_base_non_block_type_id(uint32_t type_id) const;
        };
        void analyze_non_block_pointer_types();
        SmallVector<uint32_t> physical_storage_non_block_pointer_types;
        std::unordered_map<uint32_t, PhysicalBlockMeta> physical_storage_type_to_alignment;

        void analyze_variable_scope(SPIRFunction& function, AnalyzeVariableScopeAccessHandler& handler);
        void find_function_local_luts(SPIRFunction& function, const AnalyzeVariableScopeAccessHandler& handler,
            bool single_function);
        bool may_read_undefined_variable_in_block(const SPIRBlock& block, uint32_t var);

        // Finds all resources that are written to from inside the critical section, if present.
        // The critical section is delimited by OpBeginInvocationInterlockEXT and
        // OpEndInvocationInterlockEXT instructions. In MSL and HLSL, any resources written
        // while inside the critical section must be placed in a raster order group.
        struct InterlockedResourceAccessHandler : OpcodeHandler
        {
            InterlockedResourceAccessHandler(Compiler& compiler_, uint32_t entry_point_id)
                : compiler(compiler_)
            {
                call_stack.push_back(entry_point_id);
            }

            bool handle(spv::Op op, const uint32_t* args, uint32_t length) override;
            bool begin_function_scope(const uint32_t* args, uint32_t length) override;
            bool end_function_scope(const uint32_t* args, uint32_t length) override;

            Compiler& compiler;
            bool in_crit_sec = false;

            uint32_t interlock_function_id = 0;
            bool split_function_case = false;
            bool control_flow_interlock = false;
            bool use_critical_section = false;
            bool call_stack_is_interlocked = false;
            SmallVector<uint32_t> call_stack;

            void access_potential_resource(uint32_t id);
        };

        struct InterlockedResourceAccessPrepassHandler : OpcodeHandler
        {
            InterlockedResourceAccessPrepassHandler(Compiler& compiler_, uint32_t entry_point_id)
                : compiler(compiler_)
            {
                call_stack.push_back(entry_point_id);
            }

            void rearm_current_block(const SPIRBlock& block) override;
            bool handle(spv::Op op, const uint32_t* args, uint32_t length) override;
            bool begin_function_scope(const uint32_t* args, uint32_t length) override;
            bool end_function_scope(const uint32_t* args, uint32_t length) override;

            Compiler& compiler;
            uint32_t interlock_function_id = 0;
            uint32_t current_block_id = 0;
            bool split_function_case = false;
            bool control_flow_interlock = false;
            SmallVector<uint32_t> call_stack;
        };

        void analyze_interlocked_resource_usage();
        // The set of all resources written while inside the critical section, if present.
        std::unordered_set<uint32_t> interlocked_resources;
        bool interlocked_is_complex = false;

        void make_constant_null(uint32_t id, uint32_t type);

        std::unordered_map<uint32_t, std::string> declared_block_names;

        bool instruction_to_result_type(uint32_t& result_type, uint32_t& result_id, spv::Op op, const uint32_t* args,
            uint32_t length);

        Bitset combined_decoration_for_member(const SPIRType& type, uint32_t index) const;
        static bool is_desktop_only_format(spv::ImageFormat format);

        bool is_depth_image(const SPIRType& type, uint32_t id) const;

        void set_extended_decoration(uint32_t id, ExtendedDecorations decoration, uint32_t value = 0);
        uint32_t get_extended_decoration(uint32_t id, ExtendedDecorations decoration) const;
        bool has_extended_decoration(uint32_t id, ExtendedDecorations decoration) const;
        void unset_extended_decoration(uint32_t id, ExtendedDecorations decoration);

        void set_extended_member_decoration(uint32_t type, uint32_t index, ExtendedDecorations decoration,
            uint32_t value = 0);
        uint32_t get_extended_member_decoration(uint32_t type, uint32_t index, ExtendedDecorations decoration) const;
        bool has_extended_member_decoration(uint32_t type, uint32_t index, ExtendedDecorations decoration) const;
        void unset_extended_member_decoration(uint32_t type, uint32_t index, ExtendedDecorations decoration);

        bool type_is_array_of_pointers(const SPIRType& type) const;
        bool type_is_top_level_physical_pointer(const SPIRType& type) const;
        bool type_is_top_level_pointer(const SPIRType& type) const;
        bool type_is_top_level_array(const SPIRType& type) const;
        bool type_is_block_like(const SPIRType& type) const;
        bool type_is_opaque_value(const SPIRType& type) const;

        bool reflection_ssbo_instance_name_is_significant() const;
        std::string get_remapped_declared_block_name(uint32_t id, bool fallback_prefer_instance_name) const;

        bool flush_phi_required(BlockID from, BlockID to) const;

        uint32_t evaluate_spec_constant_u32(const SPIRConstantOp& spec) const;
        uint32_t evaluate_constant_u32(uint32_t id) const;

        bool is_vertex_like_shader() const;

        // Get the correct case list for the OpSwitch, since it can be either a
        // 32 bit wide condition or a 64 bit, but the type is not embedded in the
        // instruction itself.
        const SmallVector<SPIRBlock::Case>& get_case_list(const SPIRBlock& block) const;

    private:
        // Used only to implement the old deprecated get_entry_point() interface.
        const SPIREntryPoint& get_first_entry_point(const std::string& name) const;
        SPIREntryPoint& get_first_entry_point(const std::string& name);
    };
} // namespace SPIRV_CROSS_NAMESPACE

#endif

#include <unordered_map>
#include <unordered_set>
#include <utility>

namespace SPIRV_CROSS_NAMESPACE
{
    enum PlsFormat
    {
        PlsNone = 0,

        PlsR11FG11FB10F,
        PlsR32F,
        PlsRG16F,
        PlsRGB10A2,
        PlsRGBA8,
        PlsRG16,

        PlsRGBA8I,
        PlsRG16I,

        PlsRGB10A2UI,
        PlsRGBA8UI,
        PlsRG16UI,
        PlsR32UI
    };

    struct PlsRemap
    {
        uint32_t id;
        PlsFormat format;
    };

    enum AccessChainFlagBits
    {
        ACCESS_CHAIN_INDEX_IS_LITERAL_BIT = 1 << 0,
        ACCESS_CHAIN_CHAIN_ONLY_BIT = 1 << 1,
        ACCESS_CHAIN_PTR_CHAIN_BIT = 1 << 2,
        ACCESS_CHAIN_SKIP_REGISTER_EXPRESSION_READ_BIT = 1 << 3,
        ACCESS_CHAIN_LITERAL_MSB_FORCE_ID = 1 << 4,
        ACCESS_CHAIN_FLATTEN_ALL_MEMBERS_BIT = 1 << 5,
        ACCESS_CHAIN_FORCE_COMPOSITE_BIT = 1 << 6
    };
    typedef uint32_t AccessChainFlags;

    class CompilerGLSL : public Compiler
    {
    public:
        struct Options
        {
            // The shading language version. Corresponds to #version $VALUE.
            uint32_t version = 450;

            // Emit the OpenGL ES shading language instead of desktop OpenGL.
            bool es = false;

            // Debug option to always emit temporary variables for all expressions.
            bool force_temporary = false;
            // Debug option, can be increased in an attempt to workaround SPIRV-Cross bugs temporarily.
            // If this limit has to be increased, it points to an implementation bug.
            // In certain scenarios, the maximum number of debug iterations may increase beyond this limit
            // as long as we can prove we're making certain kinds of forward progress.
            uint32_t force_recompile_max_debug_iterations = 3;

            // If true, Vulkan GLSL features are used instead of GL-compatible features.
            // Mostly useful for debugging SPIR-V files.
            bool vulkan_semantics = false;

            // If true, gl_PerVertex is explicitly redeclared in vertex, geometry and tessellation shaders.
            // The members of gl_PerVertex is determined by which built-ins are declared by the shader.
            // This option is ignored in ES versions, as redeclaration in ES is not required, and it depends on a different extension
            // (EXT_shader_io_blocks) which makes things a bit more fuzzy.
            bool separate_shader_objects = false;

            // Flattens multidimensional arrays, e.g. float foo[a][b][c] into single-dimensional arrays,
            // e.g. float foo[a * b * c].
            // This function does not change the actual SPIRType of any object.
            // Only the generated code, including declarations of interface variables are changed to be single array dimension.
            bool flatten_multidimensional_arrays = false;

            // For older desktop GLSL targets than version 420, the
            // GL_ARB_shading_language_420pack extensions is used to be able to support
            // layout(binding) on UBOs and samplers.
            // If disabled on older targets, binding decorations will be stripped.
            bool enable_420pack_extension = true;

            // In non-Vulkan GLSL, emit push constant blocks as UBOs rather than plain uniforms.
            bool emit_push_constant_as_uniform_buffer = false;

            // Always emit uniform blocks as plain uniforms, regardless of the GLSL version, even when UBOs are supported.
            // Does not apply to shader storage or push constant blocks.
            bool emit_uniform_buffer_as_plain_uniforms = false;

            // Emit OpLine directives if present in the module.
            // May not correspond exactly to original source, but should be a good approximation.
            bool emit_line_directives = false;

            // In cases where readonly/writeonly decoration are not used at all,
            // we try to deduce which qualifier(s) we should actually used, since actually emitting
            // read-write decoration is very rare, and older glslang/HLSL compilers tend to just emit readwrite as a matter of fact.
            // The default (true) is to enable automatic deduction for these cases, but if you trust the decorations set
            // by the SPIR-V, it's recommended to set this to false.
            bool enable_storage_image_qualifier_deduction = true;

            // On some targets (WebGPU), uninitialized variables are banned.
            // If this is enabled, all variables (temporaries, Private, Function)
            // which would otherwise be uninitialized will now be initialized to 0 instead.
            bool force_zero_initialized_variables = false;

            // In GLSL, force use of I/O block flattening, similar to
            // what happens on legacy GLSL targets for blocks and structs.
            bool force_flattened_io_blocks = false;

            // For opcodes where we have to perform explicit additional nan checks, very ugly code is generated.
            // If we opt-in, ignore these requirements.
            // In opcodes like NClamp/NMin/NMax and FP compare, ignore NaN behavior.
            // Use FClamp/FMin/FMax semantics for clamps and lets implementation choose ordered or unordered
            // compares.
            bool relax_nan_checks = false;

            // Loading row-major matrices from UBOs on older AMD Windows OpenGL drivers is problematic.
            // To load these types correctly, we must generate a wrapper. them in a dummy function which only purpose is to
            // ensure row_major decoration is actually respected.
            // This workaround may cause significant performance degeneration on some Android devices.
            bool enable_row_major_load_workaround = true;

            // If non-zero, controls layout(num_views = N) in; in GL_OVR_multiview2.
            uint32_t ovr_multiview_view_count = 0;

            enum Precision
            {
                DontCare,
                Lowp,
                Mediump,
                Highp
            };

            struct VertexOptions
            {
                // "Vertex-like shader" here is any shader stage that can write BuiltInPosition.

                // GLSL: In vertex-like shaders, rewrite [0, w] depth (Vulkan/D3D style) to [-w, w] depth (GL style).
                // MSL: In vertex-like shaders, rewrite [-w, w] depth (GL style) to [0, w] depth.
                // HLSL: In vertex-like shaders, rewrite [-w, w] depth (GL style) to [0, w] depth.
                bool fixup_clipspace = false;

                // In vertex-like shaders, inverts gl_Position.y or equivalent.
                bool flip_vert_y = false;

                // GLSL only, for HLSL version of this option, see CompilerHLSL.
                // If true, the backend will assume that InstanceIndex will need to apply
                // a base instance offset. Set to false if you know you will never use base instance
                // functionality as it might remove some internal uniforms.
                bool support_nonzero_base_instance = true;
            } vertex;

            struct FragmentOptions
            {
                // Add precision mediump float in ES targets when emitting GLES source.
                // Add precision highp int in ES targets when emitting GLES source.
                Precision default_float_precision = Mediump;
                Precision default_int_precision = Highp;
            } fragment;
        };

        void remap_pixel_local_storage(std::vector<PlsRemap> inputs, std::vector<PlsRemap> outputs)
        {
            pls_inputs = std::move(inputs);
            pls_outputs = std::move(outputs);
            remap_pls_variables();
        }

        // Redirect a subpassInput reading from input_attachment_index to instead load its value from
        // the color attachment at location = color_location. Requires ESSL.
        // If coherent, uses GL_EXT_shader_framebuffer_fetch, if not, uses noncoherent variant.
        void remap_ext_framebuffer_fetch(uint32_t input_attachment_index, uint32_t color_location, bool coherent);

        explicit CompilerGLSL(std::vector<uint32_t> spirv_)
            : Compiler(std::move(spirv_))
        {
            init();
        }

        CompilerGLSL(const uint32_t* ir_, size_t word_count)
            : Compiler(ir_, word_count)
        {
            init();
        }

        explicit CompilerGLSL(const ParsedIR& ir_)
            : Compiler(ir_)
        {
            init();
        }

        explicit CompilerGLSL(ParsedIR&& ir_)
            : Compiler(std::move(ir_))
        {
            init();
        }

        const Options& get_common_options() const
        {
            return options;
        }

        void set_common_options(const Options& opts)
        {
            options = opts;
        }

        std::string compile() override;

        // Returns the current string held in the conversion buffer. Useful for
        // capturing what has been converted so far when compile() throws an error.
        std::string get_partial_source();

        // Adds a line to be added right after #version in GLSL backend.
        // This is useful for enabling custom extensions which are outside the scope of SPIRV-Cross.
        // This can be combined with variable remapping.
        // A new-line will be added.
        //
        // While add_header_line() is a more generic way of adding arbitrary text to the header
        // of a GLSL file, require_extension() should be used when adding extensions since it will
        // avoid creating collisions with SPIRV-Cross generated extensions.
        //
        // Code added via add_header_line() is typically backend-specific.
        void add_header_line(const std::string& str);

        // Adds an extension which is required to run this shader, e.g.
        // require_extension("GL_KHR_my_extension");
        void require_extension(const std::string& ext);

        // Returns the list of required extensions. After compilation this will contains any other 
        // extensions that the compiler used automatically, in addition to the user specified ones.
        const SmallVector<std::string>& get_required_extensions() const;

        // Legacy GLSL compatibility method.
        // Takes a uniform or push constant variable and flattens it into a (i|u)vec4 array[N]; array instead.
        // For this to work, all types in the block must be the same basic type, e.g. mixing vec2 and vec4 is fine, but
        // mixing int and float is not.
        // The name of the uniform array will be the same as the interface block name.
        void flatten_buffer_block(VariableID id);

        // After compilation, query if a variable ID was used as a depth resource.
        // This is meaningful for MSL since descriptor types depend on this knowledge.
        // Cases which return true:
        // - Images which are declared with depth = 1 image type.
        // - Samplers which are statically used at least once with Dref opcodes.
        // - Images which are statically used at least once with Dref opcodes.
        bool variable_is_depth_or_compare(VariableID id) const;

        // If a shader output is active in this stage, but inactive in a subsequent stage,
        // this can be signalled here. This can be used to work around certain cross-stage matching problems
        // which plagues MSL and HLSL in certain scenarios.
        // An output which matches one of these will not be emitted in stage output interfaces, but rather treated as a private
        // variable.
        // This option is only meaningful for MSL and HLSL, since GLSL matches by location directly.
        // Masking builtins only takes effect if the builtin in question is part of the stage output interface.
        void mask_stage_output_by_location(uint32_t location, uint32_t component);
        void mask_stage_output_by_builtin(spv::BuiltIn builtin);

    protected:
        struct ShaderSubgroupSupportHelper
        {
            // lower enum value = greater priority
            enum Candidate
            {
                KHR_shader_subgroup_ballot,
                KHR_shader_subgroup_basic,
                KHR_shader_subgroup_vote,
                KHR_shader_subgroup_arithmetic,
                NV_gpu_shader_5,
                NV_shader_thread_group,
                NV_shader_thread_shuffle,
                ARB_shader_ballot,
                ARB_shader_group_vote,
                AMD_gcn_shader,

                CandidateCount
            };

            static const char* get_extension_name(Candidate c);
            static SmallVector<std::string> get_extra_required_extension_names(Candidate c);
            static const char* get_extra_required_extension_predicate(Candidate c);

            enum Feature
            {
                SubgroupMask = 0,
                SubgroupSize = 1,
                SubgroupInvocationID = 2,
                SubgroupID = 3,
                NumSubgroups = 4,
                SubgroupBroadcast_First = 5,
                SubgroupBallotFindLSB_MSB = 6,
                SubgroupAll_Any_AllEqualBool = 7,
                SubgroupAllEqualT = 8,
                SubgroupElect = 9,
                SubgroupBarrier = 10,
                SubgroupMemBarrier = 11,
                SubgroupBallot = 12,
                SubgroupInverseBallot_InclBitCount_ExclBitCout = 13,
                SubgroupBallotBitExtract = 14,
                SubgroupBallotBitCount = 15,
                SubgroupArithmeticIAddReduce = 16,
                SubgroupArithmeticIAddExclusiveScan = 17,
                SubgroupArithmeticIAddInclusiveScan = 18,
                SubgroupArithmeticFAddReduce = 19,
                SubgroupArithmeticFAddExclusiveScan = 20,
                SubgroupArithmeticFAddInclusiveScan = 21,
                SubgroupArithmeticIMulReduce = 22,
                SubgroupArithmeticIMulExclusiveScan = 23,
                SubgroupArithmeticIMulInclusiveScan = 24,
                SubgroupArithmeticFMulReduce = 25,
                SubgroupArithmeticFMulExclusiveScan = 26,
                SubgroupArithmeticFMulInclusiveScan = 27,
                FeatureCount
            };

            using FeatureMask = uint32_t;
            static_assert(sizeof(FeatureMask) * 8u >= FeatureCount, "Mask type needs more bits.");

            using CandidateVector = SmallVector<Candidate, CandidateCount>;
            using FeatureVector = SmallVector<Feature>;

            static FeatureVector get_feature_dependencies(Feature feature);
            static FeatureMask get_feature_dependency_mask(Feature feature);
            static bool can_feature_be_implemented_without_extensions(Feature feature);
            static Candidate get_KHR_extension_for_feature(Feature feature);

            struct Result
            {
                Result();
                uint32_t weights[CandidateCount];
            };

            void request_feature(Feature feature);
            bool is_feature_requested(Feature feature) const;
            Result resolve() const;

            static CandidateVector get_candidates_for_feature(Feature ft, const Result& r);

        private:
            static CandidateVector get_candidates_for_feature(Feature ft);
            static FeatureMask build_mask(const SmallVector<Feature>& features);
            FeatureMask feature_mask = 0;
        };

        // TODO remove this function when all subgroup ops are supported (or make it always return true)
        static bool is_supported_subgroup_op_in_opengl(spv::Op op, const uint32_t* ops);

        void reset(uint32_t iteration_count);
        void emit_function(SPIRFunction& func, const Bitset& return_flags);

        bool has_extension(const std::string& ext) const;
        void require_extension_internal(const std::string& ext);

        // Virtualize methods which need to be overridden by subclass targets like C++ and such.
        virtual void emit_function_prototype(SPIRFunction& func, const Bitset& return_flags);

        SPIRBlock* current_emitting_block = nullptr;
        SmallVector<SPIRBlock*> current_emitting_switch_stack;
        bool current_emitting_switch_fallthrough = false;

        virtual void emit_instruction(const Instruction& instr);
        struct TemporaryCopy
        {
            uint32_t dst_id;
            uint32_t src_id;
        };
        TemporaryCopy handle_instruction_precision(const Instruction& instr);
        void emit_block_instructions(SPIRBlock& block);

        // For relax_nan_checks.
        GLSLstd450 get_remapped_glsl_op(GLSLstd450 std450_op) const;
        spv::Op get_remapped_spirv_op(spv::Op op) const;

        virtual void emit_glsl_op(uint32_t result_type, uint32_t result_id, uint32_t op, const uint32_t* args,
            uint32_t count);
        virtual void emit_spv_amd_shader_ballot_op(uint32_t result_type, uint32_t result_id, uint32_t op,
            const uint32_t* args, uint32_t count);
        virtual void emit_spv_amd_shader_explicit_vertex_parameter_op(uint32_t result_type, uint32_t result_id, uint32_t op,
            const uint32_t* args, uint32_t count);
        virtual void emit_spv_amd_shader_trinary_minmax_op(uint32_t result_type, uint32_t result_id, uint32_t op,
            const uint32_t* args, uint32_t count);
        virtual void emit_spv_amd_gcn_shader_op(uint32_t result_type, uint32_t result_id, uint32_t op, const uint32_t* args,
            uint32_t count);
        virtual void emit_header();
        void emit_line_directive(uint32_t file_id, uint32_t line_literal);
        void build_workgroup_size(SmallVector<std::string>& arguments, const SpecializationConstant& x,
            const SpecializationConstant& y, const SpecializationConstant& z);

        void request_subgroup_feature(ShaderSubgroupSupportHelper::Feature feature);

        virtual void emit_sampled_image_op(uint32_t result_type, uint32_t result_id, uint32_t image_id, uint32_t samp_id);
        virtual void emit_texture_op(const Instruction& i, bool sparse);
        virtual std::string to_texture_op(const Instruction& i, bool sparse, bool* forward,
            SmallVector<uint32_t>& inherited_expressions);
        virtual void emit_subgroup_op(const Instruction& i);
        virtual std::string type_to_glsl(const SPIRType& type, uint32_t id = 0);
        virtual std::string builtin_to_glsl(spv::BuiltIn builtin, spv::StorageClass storage);
        virtual void emit_struct_member(const SPIRType& type, uint32_t member_type_id, uint32_t index,
            const std::string& qualifier = "", uint32_t base_offset = 0);
        virtual void emit_struct_padding_target(const SPIRType& type);
        virtual std::string image_type_glsl(const SPIRType& type, uint32_t id = 0);
        std::string constant_expression(const SPIRConstant& c,
            bool inside_block_like_struct_scope = false,
            bool inside_struct_scope = false);
        virtual std::string constant_op_expression(const SPIRConstantOp& cop);
        virtual std::string constant_expression_vector(const SPIRConstant& c, uint32_t vector);
        virtual void emit_fixup();
        virtual std::string variable_decl(const SPIRType& type, const std::string& name, uint32_t id = 0);
        virtual bool variable_decl_is_remapped_storage(const SPIRVariable& var, spv::StorageClass storage) const;
        virtual std::string to_func_call_arg(const SPIRFunction::Parameter& arg, uint32_t id);

        struct TextureFunctionBaseArguments
        {
            // GCC 4.8 workarounds, it doesn't understand '{}' constructor here, use explicit default constructor.
            TextureFunctionBaseArguments() = default;
            VariableID img = 0;
            const SPIRType* imgtype = nullptr;
            bool is_fetch = false, is_gather = false, is_proj = false;
        };

        struct TextureFunctionNameArguments
        {
            // GCC 4.8 workarounds, it doesn't understand '{}' constructor here, use explicit default constructor.
            TextureFunctionNameArguments() = default;
            TextureFunctionBaseArguments base;
            bool has_array_offsets = false, has_offset = false, has_grad = false;
            bool has_dref = false, is_sparse_feedback = false, has_min_lod = false;
            uint32_t lod = 0;
        };
        virtual std::string to_function_name(const TextureFunctionNameArguments& args);

        struct TextureFunctionArguments
        {
            // GCC 4.8 workarounds, it doesn't understand '{}' constructor here, use explicit default constructor.
            TextureFunctionArguments() = default;
            TextureFunctionBaseArguments base;
            uint32_t coord = 0, coord_components = 0, dref = 0;
            uint32_t grad_x = 0, grad_y = 0, lod = 0, offset = 0;
            uint32_t bias = 0, component = 0, sample = 0, sparse_texel = 0, min_lod = 0;
            bool nonuniform_expression = false;
        };
        virtual std::string to_function_args(const TextureFunctionArguments& args, bool* p_forward);

        void emit_sparse_feedback_temporaries(uint32_t result_type_id, uint32_t id, uint32_t& feedback_id,
            uint32_t& texel_id);
        uint32_t get_sparse_feedback_texel_id(uint32_t id) const;
        virtual void emit_buffer_block(const SPIRVariable& type);
        virtual void emit_push_constant_block(const SPIRVariable& var);
        virtual void emit_uniform(const SPIRVariable& var);
        virtual std::string unpack_expression_type(std::string expr_str, const SPIRType& type, uint32_t physical_type_id,
            bool packed_type, bool row_major);

        virtual bool builtin_translates_to_nonarray(spv::BuiltIn builtin) const;

        virtual bool is_user_type_structured(uint32_t id) const;

        void emit_copy_logical_type(uint32_t lhs_id, uint32_t lhs_type_id, uint32_t rhs_id, uint32_t rhs_type_id,
            SmallVector<uint32_t> chain);

        StringStream<> buffer;

        template <typename T>
        inline void statement_inner(T&& t)
        {
            buffer << std::forward<T>(t);
            statement_count++;
        }

        template <typename T, typename... Ts>
        inline void statement_inner(T&& t, Ts &&... ts)
        {
            buffer << std::forward<T>(t);
            statement_count++;
            statement_inner(std::forward<Ts>(ts)...);
        }

        template <typename... Ts>
        inline void statement(Ts &&... ts)
        {
            if (is_forcing_recompilation())
            {
                // Do not bother emitting code while force_recompile is active.
                // We will compile again.
                statement_count++;
                return;
            }

            if (redirect_statement)
            {
                redirect_statement->push_back(join(std::forward<Ts>(ts)...));
                statement_count++;
            }
            else
            {
                for (uint32_t i = 0; i < indent; i++)
                    buffer << "    ";
                statement_inner(std::forward<Ts>(ts)...);
                buffer << '\n';
            }
        }

        template <typename... Ts>
        inline void statement_no_indent(Ts &&... ts)
        {
            auto old_indent = indent;
            indent = 0;
            statement(std::forward<Ts>(ts)...);
            indent = old_indent;
        }

        // Used for implementing continue blocks where
        // we want to obtain a list of statements we can merge
        // on a single line separated by comma.
        SmallVector<std::string>* redirect_statement = nullptr;
        const SPIRBlock* current_continue_block = nullptr;
        bool block_temporary_hoisting = false;

        void begin_scope();
        void end_scope();
        void end_scope(const std::string& trailer);
        void end_scope_decl();
        void end_scope_decl(const std::string& decl);

        Options options;

        virtual std::string type_to_array_glsl(
            const SPIRType& type); // Allow Metal to use the array<T> template to make arrays a value type
        std::string to_array_size(const SPIRType& type, uint32_t index);
        uint32_t to_array_size_literal(const SPIRType& type, uint32_t index) const;
        uint32_t to_array_size_literal(const SPIRType& type) const;
        virtual std::string variable_decl(const SPIRVariable& variable); // Threadgroup arrays can't have a wrapper type
        std::string variable_decl_function_local(SPIRVariable& variable);

        void add_local_variable_name(uint32_t id);
        void add_resource_name(uint32_t id);
        void add_member_name(SPIRType& type, uint32_t name);
        void add_function_overload(const SPIRFunction& func);

        virtual bool is_non_native_row_major_matrix(uint32_t id);
        virtual bool member_is_non_native_row_major_matrix(const SPIRType& type, uint32_t index);
        bool member_is_remapped_physical_type(const SPIRType& type, uint32_t index) const;
        bool member_is_packed_physical_type(const SPIRType& type, uint32_t index) const;
        virtual std::string convert_row_major_matrix(std::string exp_str, const SPIRType& exp_type,
            uint32_t physical_type_id, bool is_packed,
            bool relaxed = false);

        std::unordered_set<std::string> local_variable_names;
        std::unordered_set<std::string> resource_names;
        std::unordered_set<std::string> block_input_names;
        std::unordered_set<std::string> block_output_names;
        std::unordered_set<std::string> block_ubo_names;
        std::unordered_set<std::string> block_ssbo_names;
        std::unordered_set<std::string> block_names; // A union of all block_*_names.
        std::unordered_map<std::string, std::unordered_set<uint64_t>> function_overloads;
        std::unordered_map<uint32_t, std::string> preserved_aliases;
        void preserve_alias_on_reset(uint32_t id);
        void reset_name_caches();

        bool processing_entry_point = false;

        // Can be overriden by subclass backends for trivial things which
        // shouldn't need polymorphism.
        struct BackendVariations
        {
            std::string discard_literal = "discard";
            std::string demote_literal = "demote";
            std::string null_pointer_literal = "";
            bool float_literal_suffix = false;
            bool double_literal_suffix = true;
            bool uint32_t_literal_suffix = true;
            bool long_long_literal_suffix = false;
            const char* basic_int_type = "int";
            const char* basic_uint_type = "uint";
            const char* basic_int8_type = "int8_t";
            const char* basic_uint8_type = "uint8_t";
            const char* basic_int16_type = "int16_t";
            const char* basic_uint16_type = "uint16_t";
            const char* int16_t_literal_suffix = "s";
            const char* uint16_t_literal_suffix = "us";
            const char* nonuniform_qualifier = "nonuniformEXT";
            const char* boolean_mix_function = "mix";
            SPIRType::BaseType boolean_in_struct_remapped_type = SPIRType::Boolean;
            bool swizzle_is_function = false;
            bool shared_is_implied = false;
            bool unsized_array_supported = true;
            bool explicit_struct_type = false;
            bool use_initializer_list = false;
            bool use_typed_initializer_list = false;
            bool can_declare_struct_inline = true;
            bool can_declare_arrays_inline = true;
            bool native_row_major_matrix = true;
            bool use_constructor_splatting = true;
            bool allow_precision_qualifiers = false;
            bool can_swizzle_scalar = false;
            bool force_gl_in_out_block = false;
            bool force_merged_mesh_block = false;
            bool can_return_array = true;
            bool allow_truncated_access_chain = false;
            bool supports_extensions = false;
            bool supports_empty_struct = false;
            bool array_is_value_type = true;
            bool array_is_value_type_in_buffer_blocks = true;
            bool comparison_image_samples_scalar = false;
            bool native_pointers = false;
            bool support_small_type_sampling_result = false;
            bool support_case_fallthrough = true;
            bool use_array_constructor = false;
            bool needs_row_major_load_workaround = false;
            bool support_pointer_to_pointer = false;
            bool support_precise_qualifier = false;
            bool support_64bit_switch = false;
            bool workgroup_size_is_hidden = false;
            bool requires_relaxed_precision_analysis = false;
            bool implicit_c_integer_promotion_rules = false;
        } backend;

        void emit_struct(SPIRType& type);
        void emit_resources();
        void emit_extension_workarounds(spv::ExecutionModel model);
        void emit_subgroup_arithmetic_workaround(const std::string& func, spv::Op op, spv::GroupOperation group_op);
        void emit_polyfills(uint32_t polyfills, bool relaxed);
        void emit_buffer_block_native(const SPIRVariable& var);
        void emit_buffer_reference_block(uint32_t type_id, bool forward_declaration);
        void emit_buffer_block_legacy(const SPIRVariable& var);
        void emit_buffer_block_flattened(const SPIRVariable& type);
        void fixup_implicit_builtin_block_names(spv::ExecutionModel model);
        void emit_declared_builtin_block(spv::StorageClass storage, spv::ExecutionModel model);
        bool should_force_emit_builtin_block(spv::StorageClass storage);
        void emit_push_constant_block_vulkan(const SPIRVariable& var);
        void emit_push_constant_block_glsl(const SPIRVariable& var);
        void emit_interface_block(const SPIRVariable& type);
        void emit_flattened_io_block(const SPIRVariable& var, const char* qual);
        void emit_flattened_io_block_struct(const std::string& basename, const SPIRType& type, const char* qual,
            const SmallVector<uint32_t>& indices);
        void emit_flattened_io_block_member(const std::string& basename, const SPIRType& type, const char* qual,
            const SmallVector<uint32_t>& indices);
        void emit_block_chain(SPIRBlock& block);
        void emit_hoisted_temporaries(SmallVector<std::pair<TypeID, ID>>& temporaries);
        std::string constant_value_macro_name(uint32_t id);
        int get_constant_mapping_to_workgroup_component(const SPIRConstant& constant) const;
        void emit_constant(const SPIRConstant& constant);
        void emit_specialization_constant_op(const SPIRConstantOp& constant);
        std::string emit_continue_block(uint32_t continue_block, bool follow_true_block, bool follow_false_block);
        bool attempt_emit_loop_header(SPIRBlock& block, SPIRBlock::Method method);

        void branch(BlockID from, BlockID to);
        void branch_to_continue(BlockID from, BlockID to);
        void branch(BlockID from, uint32_t cond, BlockID true_block, BlockID false_block);
        void flush_phi(BlockID from, BlockID to);
        void flush_variable_declaration(uint32_t id);
        void flush_undeclared_variables(SPIRBlock& block);
        void emit_variable_temporary_copies(const SPIRVariable& var);

        bool should_dereference(uint32_t id);
        bool should_forward(uint32_t id) const;
        bool should_suppress_usage_tracking(uint32_t id) const;
        void emit_mix_op(uint32_t result_type, uint32_t id, uint32_t left, uint32_t right, uint32_t lerp);
        void emit_nminmax_op(uint32_t result_type, uint32_t id, uint32_t op0, uint32_t op1, GLSLstd450 op);
        void emit_emulated_ahyper_op(uint32_t result_type, uint32_t result_id, uint32_t op0, GLSLstd450 op);
        bool to_trivial_mix_op(const SPIRType& type, std::string& op, uint32_t left, uint32_t right, uint32_t lerp);
        void emit_quaternary_func_op(uint32_t result_type, uint32_t result_id, uint32_t op0, uint32_t op1, uint32_t op2,
            uint32_t op3, const char* op);
        void emit_trinary_func_op(uint32_t result_type, uint32_t result_id, uint32_t op0, uint32_t op1, uint32_t op2,
            const char* op);
        void emit_binary_func_op(uint32_t result_type, uint32_t result_id, uint32_t op0, uint32_t op1, const char* op);
        void emit_atomic_func_op(uint32_t result_type, uint32_t result_id, uint32_t op0, uint32_t op1, const char* op);
        void emit_atomic_func_op(uint32_t result_type, uint32_t result_id, uint32_t op0, uint32_t op1, uint32_t op2, const char* op);

        void emit_unary_func_op_cast(uint32_t result_type, uint32_t result_id, uint32_t op0, const char* op,
            SPIRType::BaseType input_type, SPIRType::BaseType expected_result_type);
        void emit_binary_func_op_cast(uint32_t result_type, uint32_t result_id, uint32_t op0, uint32_t op1, const char* op,
            SPIRType::BaseType input_type, bool skip_cast_if_equal_type);
        void emit_binary_func_op_cast_clustered(uint32_t result_type, uint32_t result_id, uint32_t op0, uint32_t op1,
            const char* op, SPIRType::BaseType input_type);
        void emit_trinary_func_op_cast(uint32_t result_type, uint32_t result_id, uint32_t op0, uint32_t op1, uint32_t op2,
            const char* op, SPIRType::BaseType input_type);
        void emit_trinary_func_op_bitextract(uint32_t result_type, uint32_t result_id, uint32_t op0, uint32_t op1,
            uint32_t op2, const char* op, SPIRType::BaseType expected_result_type,
            SPIRType::BaseType input_type0, SPIRType::BaseType input_type1,
            SPIRType::BaseType input_type2);
        void emit_bitfield_insert_op(uint32_t result_type, uint32_t result_id, uint32_t op0, uint32_t op1, uint32_t op2,
            uint32_t op3, const char* op, SPIRType::BaseType offset_count_type);

        void emit_unary_func_op(uint32_t result_type, uint32_t result_id, uint32_t op0, const char* op);
        void emit_unrolled_unary_op(uint32_t result_type, uint32_t result_id, uint32_t operand, const char* op);
        void emit_binary_op(uint32_t result_type, uint32_t result_id, uint32_t op0, uint32_t op1, const char* op);
        void emit_unrolled_binary_op(uint32_t result_type, uint32_t result_id, uint32_t op0, uint32_t op1, const char* op,
            bool negate, SPIRType::BaseType expected_type);
        void emit_binary_op_cast(uint32_t result_type, uint32_t result_id, uint32_t op0, uint32_t op1, const char* op,
            SPIRType::BaseType input_type, bool skip_cast_if_equal_type, bool implicit_integer_promotion);

        SPIRType binary_op_bitcast_helper(std::string& cast_op0, std::string& cast_op1, SPIRType::BaseType& input_type,
            uint32_t op0, uint32_t op1, bool skip_cast_if_equal_type);

        virtual bool emit_complex_bitcast(uint32_t result_type, uint32_t id, uint32_t op0);

        std::string to_ternary_expression(const SPIRType& result_type, uint32_t select, uint32_t true_value,
            uint32_t false_value);

        void emit_unary_op(uint32_t result_type, uint32_t result_id, uint32_t op0, const char* op);
        void emit_unary_op_cast(uint32_t result_type, uint32_t result_id, uint32_t op0, const char* op);
        virtual void emit_mesh_tasks(SPIRBlock& block);
        bool expression_is_forwarded(uint32_t id) const;
        bool expression_suppresses_usage_tracking(uint32_t id) const;
        bool expression_read_implies_multiple_reads(uint32_t id) const;
        SPIRExpression& emit_op(uint32_t result_type, uint32_t result_id, const std::string& rhs, bool forward_rhs,
            bool suppress_usage_tracking = false);

        void access_chain_internal_append_index(std::string& expr, uint32_t base, const SPIRType* type,
            AccessChainFlags flags, bool& access_chain_is_arrayed, uint32_t index);

        std::string access_chain_internal(uint32_t base, const uint32_t* indices, uint32_t count, AccessChainFlags flags,
            AccessChainMeta* meta);

        spv::StorageClass get_expression_effective_storage_class(uint32_t ptr);
        virtual bool access_chain_needs_stage_io_builtin_translation(uint32_t base);

        virtual void check_physical_type_cast(std::string& expr, const SPIRType* type, uint32_t physical_type);
        virtual void prepare_access_chain_for_scalar_access(std::string& expr, const SPIRType& type,
            spv::StorageClass storage, bool& is_packed);

        std::string access_chain(uint32_t base, const uint32_t* indices, uint32_t count, const SPIRType& target_type,
            AccessChainMeta* meta = nullptr, bool ptr_chain = false);

        std::string flattened_access_chain(uint32_t base, const uint32_t* indices, uint32_t count,
            const SPIRType& target_type, uint32_t offset, uint32_t matrix_stride,
            uint32_t array_stride, bool need_transpose);
        std::string flattened_access_chain_struct(uint32_t base, const uint32_t* indices, uint32_t count,
            const SPIRType& target_type, uint32_t offset);
        std::string flattened_access_chain_matrix(uint32_t base, const uint32_t* indices, uint32_t count,
            const SPIRType& target_type, uint32_t offset, uint32_t matrix_stride,
            bool need_transpose);
        std::string flattened_access_chain_vector(uint32_t base, const uint32_t* indices, uint32_t count,
            const SPIRType& target_type, uint32_t offset, uint32_t matrix_stride,
            bool need_transpose);
        std::pair<std::string, uint32_t> flattened_access_chain_offset(const SPIRType& basetype, const uint32_t* indices,
            uint32_t count, uint32_t offset,
            uint32_t word_stride, bool* need_transpose = nullptr,
            uint32_t* matrix_stride = nullptr,
            uint32_t* array_stride = nullptr,
            bool ptr_chain = false);

        const char* index_to_swizzle(uint32_t index);
        std::string remap_swizzle(const SPIRType& result_type, uint32_t input_components, const std::string& expr);
        std::string declare_temporary(uint32_t type, uint32_t id);
        void emit_uninitialized_temporary(uint32_t type, uint32_t id);
        SPIRExpression& emit_uninitialized_temporary_expression(uint32_t type, uint32_t id);
        void append_global_func_args(const SPIRFunction& func, uint32_t index, SmallVector<std::string>& arglist);
        std::string to_non_uniform_aware_expression(uint32_t id);
        std::string to_expression(uint32_t id, bool register_expression_read = true);
        std::string to_composite_constructor_expression(const SPIRType& parent_type, uint32_t id, bool block_like_type);
        std::string to_rerolled_array_expression(const SPIRType& parent_type, const std::string& expr, const SPIRType& type);
        std::string to_enclosed_expression(uint32_t id, bool register_expression_read = true);
        std::string to_unpacked_expression(uint32_t id, bool register_expression_read = true);
        std::string to_unpacked_row_major_matrix_expression(uint32_t id);
        std::string to_enclosed_unpacked_expression(uint32_t id, bool register_expression_read = true);
        std::string to_dereferenced_expression(uint32_t id, bool register_expression_read = true);
        std::string to_pointer_expression(uint32_t id, bool register_expression_read = true);
        std::string to_enclosed_pointer_expression(uint32_t id, bool register_expression_read = true);
        std::string to_extract_component_expression(uint32_t id, uint32_t index);
        std::string to_extract_constant_composite_expression(uint32_t result_type, const SPIRConstant& c,
            const uint32_t* chain, uint32_t length);
        static bool needs_enclose_expression(const std::string& expr);
        std::string enclose_expression(const std::string& expr);
        std::string dereference_expression(const SPIRType& expression_type, const std::string& expr);
        std::string address_of_expression(const std::string& expr);
        void strip_enclosed_expression(std::string& expr);
        std::string to_member_name(const SPIRType& type, uint32_t index);
        virtual std::string to_member_reference(uint32_t base, const SPIRType& type, uint32_t index, bool ptr_chain_is_resolved);
        std::string to_multi_member_reference(const SPIRType& type, const SmallVector<uint32_t>& indices);
        std::string type_to_glsl_constructor(const SPIRType& type);
        std::string argument_decl(const SPIRFunction::Parameter& arg);
        virtual std::string to_qualifiers_glsl(uint32_t id);
        void fixup_io_block_patch_primitive_qualifiers(const SPIRVariable& var);
        void emit_output_variable_initializer(const SPIRVariable& var);
        std::string to_precision_qualifiers_glsl(uint32_t id);
        virtual const char* to_storage_qualifiers_glsl(const SPIRVariable& var);
        std::string flags_to_qualifiers_glsl(const SPIRType& type, const Bitset& flags);
        const char* format_to_glsl(spv::ImageFormat format);
        virtual std::string layout_for_member(const SPIRType& type, uint32_t index);
        virtual std::string to_interpolation_qualifiers(const Bitset& flags);
        std::string layout_for_variable(const SPIRVariable& variable);
        std::string to_combined_image_sampler(VariableID image_id, VariableID samp_id);
        virtual bool skip_argument(uint32_t id) const;
        virtual bool emit_array_copy(const char* expr, uint32_t lhs_id, uint32_t rhs_id,
            spv::StorageClass lhs_storage, spv::StorageClass rhs_storage);
        virtual void emit_block_hints(const SPIRBlock& block);
        virtual std::string to_initializer_expression(const SPIRVariable& var);
        virtual std::string to_zero_initialized_expression(uint32_t type_id);
        bool type_can_zero_initialize(const SPIRType& type) const;

        bool buffer_is_packing_standard(const SPIRType& type, BufferPackingStandard packing,
            uint32_t* failed_index = nullptr, uint32_t start_offset = 0,
            uint32_t end_offset = ~(0u));
        std::string buffer_to_packing_standard(const SPIRType& type, bool support_std430_without_scalar_layout);

        uint32_t type_to_packed_base_size(const SPIRType& type, BufferPackingStandard packing);
        uint32_t type_to_packed_alignment(const SPIRType& type, const Bitset& flags, BufferPackingStandard packing);
        uint32_t type_to_packed_array_stride(const SPIRType& type, const Bitset& flags, BufferPackingStandard packing);
        uint32_t type_to_packed_size(const SPIRType& type, const Bitset& flags, BufferPackingStandard packing);
        uint32_t type_to_location_count(const SPIRType& type) const;

        std::string bitcast_glsl(const SPIRType& result_type, uint32_t arg);
        virtual std::string bitcast_glsl_op(const SPIRType& result_type, const SPIRType& argument_type);

        std::string bitcast_expression(SPIRType::BaseType target_type, uint32_t arg);
        std::string bitcast_expression(const SPIRType& target_type, SPIRType::BaseType expr_type, const std::string& expr);

        std::string build_composite_combiner(uint32_t result_type, const uint32_t* elems, uint32_t length);
        bool remove_duplicate_swizzle(std::string& op);
        bool remove_unity_swizzle(uint32_t base, std::string& op);

        // Can modify flags to remote readonly/writeonly if image type
        // and force recompile.
        bool check_atomic_image(uint32_t id);

        virtual void replace_illegal_names();
        void replace_illegal_names(const std::unordered_set<std::string>& keywords);
        virtual void emit_entry_point_declarations();

        void replace_fragment_output(SPIRVariable& var);
        void replace_fragment_outputs();
        std::string legacy_tex_op(const std::string& op, const SPIRType& imgtype, uint32_t id);

        void forward_relaxed_precision(uint32_t dst_id, const uint32_t* args, uint32_t length);
        void analyze_precision_requirements(uint32_t type_id, uint32_t dst_id, uint32_t* args, uint32_t length);
        Options::Precision analyze_expression_precision(const uint32_t* args, uint32_t length) const;

        uint32_t indent = 0;

        std::unordered_set<uint32_t> emitted_functions;

        // Ensure that we declare phi-variable copies even if the original declaration isn't deferred
        std::unordered_set<uint32_t> flushed_phi_variables;

        std::unordered_set<uint32_t> flattened_buffer_blocks;
        std::unordered_map<uint32_t, bool> flattened_structs;

        ShaderSubgroupSupportHelper shader_subgroup_supporter;

        std::string load_flattened_struct(const std::string& basename, const SPIRType& type);
        std::string to_flattened_struct_member(const std::string& basename, const SPIRType& type, uint32_t index);
        void store_flattened_struct(uint32_t lhs_id, uint32_t value);
        void store_flattened_struct(const std::string& basename, uint32_t rhs, const SPIRType& type,
            const SmallVector<uint32_t>& indices);
        std::string to_flattened_access_chain_expression(uint32_t id);

        // Usage tracking. If a temporary is used more than once, use the temporary instead to
        // avoid AST explosion when SPIRV is generated with pure SSA and doesn't write stuff to variables.
        std::unordered_map<uint32_t, uint32_t> expression_usage_counts;
        void track_expression_read(uint32_t id);

        SmallVector<std::string> forced_extensions;
        SmallVector<std::string> header_lines;

        // Used when expressions emit extra opcodes with their own unique IDs,
        // and we need to reuse the IDs across recompilation loops.
        // Currently used by NMin/Max/Clamp implementations.
        std::unordered_map<uint32_t, uint32_t> extra_sub_expressions;

        SmallVector<TypeID> workaround_ubo_load_overload_types;
        void request_workaround_wrapper_overload(TypeID id);
        void rewrite_load_for_wrapped_row_major(std::string& expr, TypeID loaded_type, ID ptr);

        uint32_t statement_count = 0;

        inline bool is_legacy() const
        {
            return (options.es && options.version < 300) || (!options.es && options.version < 130);
        }

        inline bool is_legacy_es() const
        {
            return options.es && options.version < 300;
        }

        inline bool is_legacy_desktop() const
        {
            return !options.es && options.version < 130;
        }

        enum Polyfill : uint32_t
        {
            PolyfillTranspose2x2 = 1 << 0,
            PolyfillTranspose3x3 = 1 << 1,
            PolyfillTranspose4x4 = 1 << 2,
            PolyfillDeterminant2x2 = 1 << 3,
            PolyfillDeterminant3x3 = 1 << 4,
            PolyfillDeterminant4x4 = 1 << 5,
            PolyfillMatrixInverse2x2 = 1 << 6,
            PolyfillMatrixInverse3x3 = 1 << 7,
            PolyfillMatrixInverse4x4 = 1 << 8,
        };

        uint32_t required_polyfills = 0;
        uint32_t required_polyfills_relaxed = 0;
        void require_polyfill(Polyfill polyfill, bool relaxed);

        bool ray_tracing_is_khr = false;
        bool barycentric_is_nv = false;
        void ray_tracing_khr_fixup_locations();

        bool args_will_forward(uint32_t id, const uint32_t* args, uint32_t num_args, bool pure);
        void register_call_out_argument(uint32_t id);
        void register_impure_function_call();
        void register_control_dependent_expression(uint32_t expr);

        // GL_EXT_shader_pixel_local_storage support.
        std::vector<PlsRemap> pls_inputs;
        std::vector<PlsRemap> pls_outputs;
        std::string pls_decl(const PlsRemap& variable);
        const char* to_pls_qualifiers_glsl(const SPIRVariable& variable);
        void emit_pls();
        void remap_pls_variables();

        // GL_EXT_shader_framebuffer_fetch support.
        std::vector<std::pair<uint32_t, uint32_t>> subpass_to_framebuffer_fetch_attachment;
        std::vector<std::pair<uint32_t, bool>> inout_color_attachments;
        bool location_is_framebuffer_fetch(uint32_t location) const;
        bool location_is_non_coherent_framebuffer_fetch(uint32_t location) const;
        bool subpass_input_is_framebuffer_fetch(uint32_t id) const;
        void emit_inout_fragment_outputs_copy_to_subpass_inputs();
        const SPIRVariable* find_subpass_input_by_attachment_index(uint32_t index) const;
        const SPIRVariable* find_color_output_by_location(uint32_t location) const;

        // A variant which takes two sets of name. The secondary is only used to verify there are no collisions,
        // but the set is not updated when we have found a new name.
        // Used primarily when adding block interface names.
        void add_variable(std::unordered_set<std::string>& variables_primary,
            const std::unordered_set<std::string>& variables_secondary, std::string& name);

        void check_function_call_constraints(const uint32_t* args, uint32_t length);
        void handle_invalid_expression(uint32_t id);
        void force_temporary_and_recompile(uint32_t id);
        void find_static_extensions();

        uint32_t consume_temporary_in_precision_context(uint32_t type_id, uint32_t id, Options::Precision precision);
        std::unordered_map<uint32_t, uint32_t> temporary_to_mirror_precision_alias;
        std::unordered_set<uint32_t> composite_insert_overwritten;
        std::unordered_set<uint32_t> block_composite_insert_overwrite;

        std::string emit_for_loop_initializers(const SPIRBlock& block);
        void emit_while_loop_initializers(const SPIRBlock& block);
        bool for_loop_initializers_are_same_type(const SPIRBlock& block);
        bool optimize_read_modify_write(const SPIRType& type, const std::string& lhs, const std::string& rhs);
        void fixup_image_load_store_access();

        bool type_is_empty(const SPIRType& type);

        bool can_use_io_location(spv::StorageClass storage, bool block);
        const Instruction* get_next_instruction_in_block(const Instruction& instr);
        static uint32_t mask_relevant_memory_semantics(uint32_t semantics);

        std::string convert_half_to_string(const SPIRConstant& value, uint32_t col, uint32_t row);
        std::string convert_float_to_string(const SPIRConstant& value, uint32_t col, uint32_t row);
        std::string convert_double_to_string(const SPIRConstant& value, uint32_t col, uint32_t row);

        std::string convert_separate_image_to_expression(uint32_t id);

        // Builtins in GLSL are always specific signedness, but the SPIR-V can declare them
        // as either unsigned or signed.
        // Sometimes we will need to automatically perform casts on load and store to make this work.
        virtual void cast_to_variable_store(uint32_t target_id, std::string& expr, const SPIRType& expr_type);
        virtual void cast_from_variable_load(uint32_t source_id, std::string& expr, const SPIRType& expr_type);
        void unroll_array_from_complex_load(uint32_t target_id, uint32_t source_id, std::string& expr);
        bool unroll_array_to_complex_store(uint32_t target_id, uint32_t source_id);
        void convert_non_uniform_expression(std::string& expr, uint32_t ptr_id);

        void handle_store_to_invariant_variable(uint32_t store_id, uint32_t value_id);
        void disallow_forwarding_in_expression_chain(const SPIRExpression& expr);

        bool expression_is_constant_null(uint32_t id) const;
        bool expression_is_non_value_type_array(uint32_t ptr);
        virtual void emit_store_statement(uint32_t lhs_expression, uint32_t rhs_expression);

        uint32_t get_integer_width_for_instruction(const Instruction& instr) const;
        uint32_t get_integer_width_for_glsl_instruction(GLSLstd450 op, const uint32_t* arguments, uint32_t length) const;

        bool variable_is_lut(const SPIRVariable& var) const;

        char current_locale_radix_character = '.';

        void fixup_type_alias();
        void reorder_type_alias();
        void fixup_anonymous_struct_names();
        void fixup_anonymous_struct_names(std::unordered_set<uint32_t>& visited, const SPIRType& type);

        static const char* vector_swizzle(int vecsize, int index);

        bool is_stage_output_location_masked(uint32_t location, uint32_t component) const;
        bool is_stage_output_builtin_masked(spv::BuiltIn builtin) const;
        bool is_stage_output_variable_masked(const SPIRVariable& var) const;
        bool is_stage_output_block_member_masked(const SPIRVariable& var, uint32_t index, bool strip_array) const;
        bool is_per_primitive_variable(const SPIRVariable& var) const;
        uint32_t get_accumulated_member_location(const SPIRVariable& var, uint32_t mbr_idx, bool strip_array) const;
        uint32_t get_declared_member_location(const SPIRVariable& var, uint32_t mbr_idx, bool strip_array) const;
        std::unordered_set<LocationComponentPair, InternalHasher> masked_output_locations;
        std::unordered_set<uint32_t> masked_output_builtins;

    private:
        void init();

        SmallVector<ConstantID> get_composite_constant_ids(ConstantID const_id);
        void fill_composite_constant(SPIRConstant& constant, TypeID type_id, const SmallVector<ConstantID>& initializers);
        void set_composite_constant(ConstantID const_id, TypeID type_id, const SmallVector<ConstantID>& initializers);
        TypeID get_composite_member_type(TypeID type_id, uint32_t member_idx);
        std::unordered_map<uint32_t, SmallVector<ConstantID>> const_composite_insert_ids;
    };
} // namespace SPIRV_CROSS_NAMESPACE

#endif

#ifndef SPIRV_CROSS_MSL_HPP
#define SPIRV_CROSS_MSL_HPP

#ifndef SPIRV_CROSS_GLSL_HPP
#define SPIRV_CROSS_GLSL_HPP

#include "GLSL.std.450.h"
#include "spirv_cross.hpp"
#include <unordered_map>
#include <unordered_set>
#include <utility>

namespace SPIRV_CROSS_NAMESPACE
{
    enum PlsFormat
    {
        PlsNone = 0,

        PlsR11FG11FB10F,
        PlsR32F,
        PlsRG16F,
        PlsRGB10A2,
        PlsRGBA8,
        PlsRG16,

        PlsRGBA8I,
        PlsRG16I,

        PlsRGB10A2UI,
        PlsRGBA8UI,
        PlsRG16UI,
        PlsR32UI
    };

    struct PlsRemap
    {
        uint32_t id;
        PlsFormat format;
    };

    enum AccessChainFlagBits
    {
        ACCESS_CHAIN_INDEX_IS_LITERAL_BIT = 1 << 0,
        ACCESS_CHAIN_CHAIN_ONLY_BIT = 1 << 1,
        ACCESS_CHAIN_PTR_CHAIN_BIT = 1 << 2,
        ACCESS_CHAIN_SKIP_REGISTER_EXPRESSION_READ_BIT = 1 << 3,
        ACCESS_CHAIN_LITERAL_MSB_FORCE_ID = 1 << 4,
        ACCESS_CHAIN_FLATTEN_ALL_MEMBERS_BIT = 1 << 5,
        ACCESS_CHAIN_FORCE_COMPOSITE_BIT = 1 << 6
    };
    typedef uint32_t AccessChainFlags;

    class CompilerGLSL : public Compiler
    {
    public:
        struct Options
        {
            // The shading language version. Corresponds to #version $VALUE.
            uint32_t version = 450;

            // Emit the OpenGL ES shading language instead of desktop OpenGL.
            bool es = false;

            // Debug option to always emit temporary variables for all expressions.
            bool force_temporary = false;
            // Debug option, can be increased in an attempt to workaround SPIRV-Cross bugs temporarily.
            // If this limit has to be increased, it points to an implementation bug.
            // In certain scenarios, the maximum number of debug iterations may increase beyond this limit
            // as long as we can prove we're making certain kinds of forward progress.
            uint32_t force_recompile_max_debug_iterations = 3;

            // If true, Vulkan GLSL features are used instead of GL-compatible features.
            // Mostly useful for debugging SPIR-V files.
            bool vulkan_semantics = false;

            // If true, gl_PerVertex is explicitly redeclared in vertex, geometry and tessellation shaders.
            // The members of gl_PerVertex is determined by which built-ins are declared by the shader.
            // This option is ignored in ES versions, as redeclaration in ES is not required, and it depends on a different extension
            // (EXT_shader_io_blocks) which makes things a bit more fuzzy.
            bool separate_shader_objects = false;

            // Flattens multidimensional arrays, e.g. float foo[a][b][c] into single-dimensional arrays,
            // e.g. float foo[a * b * c].
            // This function does not change the actual SPIRType of any object.
            // Only the generated code, including declarations of interface variables are changed to be single array dimension.
            bool flatten_multidimensional_arrays = false;

            // For older desktop GLSL targets than version 420, the
            // GL_ARB_shading_language_420pack extensions is used to be able to support
            // layout(binding) on UBOs and samplers.
            // If disabled on older targets, binding decorations will be stripped.
            bool enable_420pack_extension = true;

            // In non-Vulkan GLSL, emit push constant blocks as UBOs rather than plain uniforms.
            bool emit_push_constant_as_uniform_buffer = false;

            // Always emit uniform blocks as plain uniforms, regardless of the GLSL version, even when UBOs are supported.
            // Does not apply to shader storage or push constant blocks.
            bool emit_uniform_buffer_as_plain_uniforms = false;

            // Emit OpLine directives if present in the module.
            // May not correspond exactly to original source, but should be a good approximation.
            bool emit_line_directives = false;

            // In cases where readonly/writeonly decoration are not used at all,
            // we try to deduce which qualifier(s) we should actually used, since actually emitting
            // read-write decoration is very rare, and older glslang/HLSL compilers tend to just emit readwrite as a matter of fact.
            // The default (true) is to enable automatic deduction for these cases, but if you trust the decorations set
            // by the SPIR-V, it's recommended to set this to false.
            bool enable_storage_image_qualifier_deduction = true;

            // On some targets (WebGPU), uninitialized variables are banned.
            // If this is enabled, all variables (temporaries, Private, Function)
            // which would otherwise be uninitialized will now be initialized to 0 instead.
            bool force_zero_initialized_variables = false;

            // In GLSL, force use of I/O block flattening, similar to
            // what happens on legacy GLSL targets for blocks and structs.
            bool force_flattened_io_blocks = false;

            // For opcodes where we have to perform explicit additional nan checks, very ugly code is generated.
            // If we opt-in, ignore these requirements.
            // In opcodes like NClamp/NMin/NMax and FP compare, ignore NaN behavior.
            // Use FClamp/FMin/FMax semantics for clamps and lets implementation choose ordered or unordered
            // compares.
            bool relax_nan_checks = false;

            // Loading row-major matrices from UBOs on older AMD Windows OpenGL drivers is problematic.
            // To load these types correctly, we must generate a wrapper. them in a dummy function which only purpose is to
            // ensure row_major decoration is actually respected.
            // This workaround may cause significant performance degeneration on some Android devices.
            bool enable_row_major_load_workaround = true;

            // If non-zero, controls layout(num_views = N) in; in GL_OVR_multiview2.
            uint32_t ovr_multiview_view_count = 0;

            enum Precision
            {
                DontCare,
                Lowp,
                Mediump,
                Highp
            };

            struct VertexOptions
            {
                // "Vertex-like shader" here is any shader stage that can write BuiltInPosition.

                // GLSL: In vertex-like shaders, rewrite [0, w] depth (Vulkan/D3D style) to [-w, w] depth (GL style).
                // MSL: In vertex-like shaders, rewrite [-w, w] depth (GL style) to [0, w] depth.
                // HLSL: In vertex-like shaders, rewrite [-w, w] depth (GL style) to [0, w] depth.
                bool fixup_clipspace = false;

                // In vertex-like shaders, inverts gl_Position.y or equivalent.
                bool flip_vert_y = false;

                // GLSL only, for HLSL version of this option, see CompilerHLSL.
                // If true, the backend will assume that InstanceIndex will need to apply
                // a base instance offset. Set to false if you know you will never use base instance
                // functionality as it might remove some internal uniforms.
                bool support_nonzero_base_instance = true;
            } vertex;

            struct FragmentOptions
            {
                // Add precision mediump float in ES targets when emitting GLES source.
                // Add precision highp int in ES targets when emitting GLES source.
                Precision default_float_precision = Mediump;
                Precision default_int_precision = Highp;
            } fragment;
        };

        void remap_pixel_local_storage(std::vector<PlsRemap> inputs, std::vector<PlsRemap> outputs)
        {
            pls_inputs = std::move(inputs);
            pls_outputs = std::move(outputs);
            remap_pls_variables();
        }

        // Redirect a subpassInput reading from input_attachment_index to instead load its value from
        // the color attachment at location = color_location. Requires ESSL.
        // If coherent, uses GL_EXT_shader_framebuffer_fetch, if not, uses noncoherent variant.
        void remap_ext_framebuffer_fetch(uint32_t input_attachment_index, uint32_t color_location, bool coherent);

        explicit CompilerGLSL(std::vector<uint32_t> spirv_)
            : Compiler(std::move(spirv_))
        {
            init();
        }

        CompilerGLSL(const uint32_t* ir_, size_t word_count)
            : Compiler(ir_, word_count)
        {
            init();
        }

        explicit CompilerGLSL(const ParsedIR& ir_)
            : Compiler(ir_)
        {
            init();
        }

        explicit CompilerGLSL(ParsedIR&& ir_)
            : Compiler(std::move(ir_))
        {
            init();
        }

        const Options& get_common_options() const
        {
            return options;
        }

        void set_common_options(const Options& opts)
        {
            options = opts;
        }

        std::string compile() override;

        // Returns the current string held in the conversion buffer. Useful for
        // capturing what has been converted so far when compile() throws an error.
        std::string get_partial_source();

        // Adds a line to be added right after #version in GLSL backend.
        // This is useful for enabling custom extensions which are outside the scope of SPIRV-Cross.
        // This can be combined with variable remapping.
        // A new-line will be added.
        //
        // While add_header_line() is a more generic way of adding arbitrary text to the header
        // of a GLSL file, require_extension() should be used when adding extensions since it will
        // avoid creating collisions with SPIRV-Cross generated extensions.
        //
        // Code added via add_header_line() is typically backend-specific.
        void add_header_line(const std::string& str);

        // Adds an extension which is required to run this shader, e.g.
        // require_extension("GL_KHR_my_extension");
        void require_extension(const std::string& ext);

        // Returns the list of required extensions. After compilation this will contains any other 
        // extensions that the compiler used automatically, in addition to the user specified ones.
        const SmallVector<std::string>& get_required_extensions() const;

        // Legacy GLSL compatibility method.
        // Takes a uniform or push constant variable and flattens it into a (i|u)vec4 array[N]; array instead.
        // For this to work, all types in the block must be the same basic type, e.g. mixing vec2 and vec4 is fine, but
        // mixing int and float is not.
        // The name of the uniform array will be the same as the interface block name.
        void flatten_buffer_block(VariableID id);

        // After compilation, query if a variable ID was used as a depth resource.
        // This is meaningful for MSL since descriptor types depend on this knowledge.
        // Cases which return true:
        // - Images which are declared with depth = 1 image type.
        // - Samplers which are statically used at least once with Dref opcodes.
        // - Images which are statically used at least once with Dref opcodes.
        bool variable_is_depth_or_compare(VariableID id) const;

        // If a shader output is active in this stage, but inactive in a subsequent stage,
        // this can be signalled here. This can be used to work around certain cross-stage matching problems
        // which plagues MSL and HLSL in certain scenarios.
        // An output which matches one of these will not be emitted in stage output interfaces, but rather treated as a private
        // variable.
        // This option is only meaningful for MSL and HLSL, since GLSL matches by location directly.
        // Masking builtins only takes effect if the builtin in question is part of the stage output interface.
        void mask_stage_output_by_location(uint32_t location, uint32_t component);
        void mask_stage_output_by_builtin(spv::BuiltIn builtin);

    protected:
        struct ShaderSubgroupSupportHelper
        {
            // lower enum value = greater priority
            enum Candidate
            {
                KHR_shader_subgroup_ballot,
                KHR_shader_subgroup_basic,
                KHR_shader_subgroup_vote,
                KHR_shader_subgroup_arithmetic,
                NV_gpu_shader_5,
                NV_shader_thread_group,
                NV_shader_thread_shuffle,
                ARB_shader_ballot,
                ARB_shader_group_vote,
                AMD_gcn_shader,

                CandidateCount
            };

            static const char* get_extension_name(Candidate c);
            static SmallVector<std::string> get_extra_required_extension_names(Candidate c);
            static const char* get_extra_required_extension_predicate(Candidate c);

            enum Feature
            {
                SubgroupMask = 0,
                SubgroupSize = 1,
                SubgroupInvocationID = 2,
                SubgroupID = 3,
                NumSubgroups = 4,
                SubgroupBroadcast_First = 5,
                SubgroupBallotFindLSB_MSB = 6,
                SubgroupAll_Any_AllEqualBool = 7,
                SubgroupAllEqualT = 8,
                SubgroupElect = 9,
                SubgroupBarrier = 10,
                SubgroupMemBarrier = 11,
                SubgroupBallot = 12,
                SubgroupInverseBallot_InclBitCount_ExclBitCout = 13,
                SubgroupBallotBitExtract = 14,
                SubgroupBallotBitCount = 15,
                SubgroupArithmeticIAddReduce = 16,
                SubgroupArithmeticIAddExclusiveScan = 17,
                SubgroupArithmeticIAddInclusiveScan = 18,
                SubgroupArithmeticFAddReduce = 19,
                SubgroupArithmeticFAddExclusiveScan = 20,
                SubgroupArithmeticFAddInclusiveScan = 21,
                SubgroupArithmeticIMulReduce = 22,
                SubgroupArithmeticIMulExclusiveScan = 23,
                SubgroupArithmeticIMulInclusiveScan = 24,
                SubgroupArithmeticFMulReduce = 25,
                SubgroupArithmeticFMulExclusiveScan = 26,
                SubgroupArithmeticFMulInclusiveScan = 27,
                FeatureCount
            };

            using FeatureMask = uint32_t;
            static_assert(sizeof(FeatureMask) * 8u >= FeatureCount, "Mask type needs more bits.");

            using CandidateVector = SmallVector<Candidate, CandidateCount>;
            using FeatureVector = SmallVector<Feature>;

            static FeatureVector get_feature_dependencies(Feature feature);
            static FeatureMask get_feature_dependency_mask(Feature feature);
            static bool can_feature_be_implemented_without_extensions(Feature feature);
            static Candidate get_KHR_extension_for_feature(Feature feature);

            struct Result
            {
                Result();
                uint32_t weights[CandidateCount];
            };

            void request_feature(Feature feature);
            bool is_feature_requested(Feature feature) const;
            Result resolve() const;

            static CandidateVector get_candidates_for_feature(Feature ft, const Result& r);

        private:
            static CandidateVector get_candidates_for_feature(Feature ft);
            static FeatureMask build_mask(const SmallVector<Feature>& features);
            FeatureMask feature_mask = 0;
        };

        // TODO remove this function when all subgroup ops are supported (or make it always return true)
        static bool is_supported_subgroup_op_in_opengl(spv::Op op, const uint32_t* ops);

        void reset(uint32_t iteration_count);
        void emit_function(SPIRFunction& func, const Bitset& return_flags);

        bool has_extension(const std::string& ext) const;
        void require_extension_internal(const std::string& ext);

        // Virtualize methods which need to be overridden by subclass targets like C++ and such.
        virtual void emit_function_prototype(SPIRFunction& func, const Bitset& return_flags);

        SPIRBlock* current_emitting_block = nullptr;
        SmallVector<SPIRBlock*> current_emitting_switch_stack;
        bool current_emitting_switch_fallthrough = false;

        virtual void emit_instruction(const Instruction& instr);
        struct TemporaryCopy
        {
            uint32_t dst_id;
            uint32_t src_id;
        };
        TemporaryCopy handle_instruction_precision(const Instruction& instr);
        void emit_block_instructions(SPIRBlock& block);

        // For relax_nan_checks.
        GLSLstd450 get_remapped_glsl_op(GLSLstd450 std450_op) const;
        spv::Op get_remapped_spirv_op(spv::Op op) const;

        virtual void emit_glsl_op(uint32_t result_type, uint32_t result_id, uint32_t op, const uint32_t* args,
            uint32_t count);
        virtual void emit_spv_amd_shader_ballot_op(uint32_t result_type, uint32_t result_id, uint32_t op,
            const uint32_t* args, uint32_t count);
        virtual void emit_spv_amd_shader_explicit_vertex_parameter_op(uint32_t result_type, uint32_t result_id, uint32_t op,
            const uint32_t* args, uint32_t count);
        virtual void emit_spv_amd_shader_trinary_minmax_op(uint32_t result_type, uint32_t result_id, uint32_t op,
            const uint32_t* args, uint32_t count);
        virtual void emit_spv_amd_gcn_shader_op(uint32_t result_type, uint32_t result_id, uint32_t op, const uint32_t* args,
            uint32_t count);
        virtual void emit_header();
        void emit_line_directive(uint32_t file_id, uint32_t line_literal);
        void build_workgroup_size(SmallVector<std::string>& arguments, const SpecializationConstant& x,
            const SpecializationConstant& y, const SpecializationConstant& z);

        void request_subgroup_feature(ShaderSubgroupSupportHelper::Feature feature);

        virtual void emit_sampled_image_op(uint32_t result_type, uint32_t result_id, uint32_t image_id, uint32_t samp_id);
        virtual void emit_texture_op(const Instruction& i, bool sparse);
        virtual std::string to_texture_op(const Instruction& i, bool sparse, bool* forward,
            SmallVector<uint32_t>& inherited_expressions);
        virtual void emit_subgroup_op(const Instruction& i);
        virtual std::string type_to_glsl(const SPIRType& type, uint32_t id = 0);
        virtual std::string builtin_to_glsl(spv::BuiltIn builtin, spv::StorageClass storage);
        virtual void emit_struct_member(const SPIRType& type, uint32_t member_type_id, uint32_t index,
            const std::string& qualifier = "", uint32_t base_offset = 0);
        virtual void emit_struct_padding_target(const SPIRType& type);
        virtual std::string image_type_glsl(const SPIRType& type, uint32_t id = 0);
        std::string constant_expression(const SPIRConstant& c,
            bool inside_block_like_struct_scope = false,
            bool inside_struct_scope = false);
        virtual std::string constant_op_expression(const SPIRConstantOp& cop);
        virtual std::string constant_expression_vector(const SPIRConstant& c, uint32_t vector);
        virtual void emit_fixup();
        virtual std::string variable_decl(const SPIRType& type, const std::string& name, uint32_t id = 0);
        virtual bool variable_decl_is_remapped_storage(const SPIRVariable& var, spv::StorageClass storage) const;
        virtual std::string to_func_call_arg(const SPIRFunction::Parameter& arg, uint32_t id);

        struct TextureFunctionBaseArguments
        {
            // GCC 4.8 workarounds, it doesn't understand '{}' constructor here, use explicit default constructor.
            TextureFunctionBaseArguments() = default;
            VariableID img = 0;
            const SPIRType* imgtype = nullptr;
            bool is_fetch = false, is_gather = false, is_proj = false;
        };

        struct TextureFunctionNameArguments
        {
            // GCC 4.8 workarounds, it doesn't understand '{}' constructor here, use explicit default constructor.
            TextureFunctionNameArguments() = default;
            TextureFunctionBaseArguments base;
            bool has_array_offsets = false, has_offset = false, has_grad = false;
            bool has_dref = false, is_sparse_feedback = false, has_min_lod = false;
            uint32_t lod = 0;
        };
        virtual std::string to_function_name(const TextureFunctionNameArguments& args);

        struct TextureFunctionArguments
        {
            // GCC 4.8 workarounds, it doesn't understand '{}' constructor here, use explicit default constructor.
            TextureFunctionArguments() = default;
            TextureFunctionBaseArguments base;
            uint32_t coord = 0, coord_components = 0, dref = 0;
            uint32_t grad_x = 0, grad_y = 0, lod = 0, offset = 0;
            uint32_t bias = 0, component = 0, sample = 0, sparse_texel = 0, min_lod = 0;
            bool nonuniform_expression = false;
        };
        virtual std::string to_function_args(const TextureFunctionArguments& args, bool* p_forward);

        void emit_sparse_feedback_temporaries(uint32_t result_type_id, uint32_t id, uint32_t& feedback_id,
            uint32_t& texel_id);
        uint32_t get_sparse_feedback_texel_id(uint32_t id) const;
        virtual void emit_buffer_block(const SPIRVariable& type);
        virtual void emit_push_constant_block(const SPIRVariable& var);
        virtual void emit_uniform(const SPIRVariable& var);
        virtual std::string unpack_expression_type(std::string expr_str, const SPIRType& type, uint32_t physical_type_id,
            bool packed_type, bool row_major);

        virtual bool builtin_translates_to_nonarray(spv::BuiltIn builtin) const;

        virtual bool is_user_type_structured(uint32_t id) const;

        void emit_copy_logical_type(uint32_t lhs_id, uint32_t lhs_type_id, uint32_t rhs_id, uint32_t rhs_type_id,
            SmallVector<uint32_t> chain);

        StringStream<> buffer;

        template <typename T>
        inline void statement_inner(T&& t)
        {
            buffer << std::forward<T>(t);
            statement_count++;
        }

        template <typename T, typename... Ts>
        inline void statement_inner(T&& t, Ts &&... ts)
        {
            buffer << std::forward<T>(t);
            statement_count++;
            statement_inner(std::forward<Ts>(ts)...);
        }

        template <typename... Ts>
        inline void statement(Ts &&... ts)
        {
            if (is_forcing_recompilation())
            {
                // Do not bother emitting code while force_recompile is active.
                // We will compile again.
                statement_count++;
                return;
            }

            if (redirect_statement)
            {
                redirect_statement->push_back(join(std::forward<Ts>(ts)...));
                statement_count++;
            }
            else
            {
                for (uint32_t i = 0; i < indent; i++)
                    buffer << "    ";
                statement_inner(std::forward<Ts>(ts)...);
                buffer << '\n';
            }
        }

        template <typename... Ts>
        inline void statement_no_indent(Ts &&... ts)
        {
            auto old_indent = indent;
            indent = 0;
            statement(std::forward<Ts>(ts)...);
            indent = old_indent;
        }

        // Used for implementing continue blocks where
        // we want to obtain a list of statements we can merge
        // on a single line separated by comma.
        SmallVector<std::string>* redirect_statement = nullptr;
        const SPIRBlock* current_continue_block = nullptr;
        bool block_temporary_hoisting = false;

        void begin_scope();
        void end_scope();
        void end_scope(const std::string& trailer);
        void end_scope_decl();
        void end_scope_decl(const std::string& decl);

        Options options;

        virtual std::string type_to_array_glsl(
            const SPIRType& type); // Allow Metal to use the array<T> template to make arrays a value type
        std::string to_array_size(const SPIRType& type, uint32_t index);
        uint32_t to_array_size_literal(const SPIRType& type, uint32_t index) const;
        uint32_t to_array_size_literal(const SPIRType& type) const;
        virtual std::string variable_decl(const SPIRVariable& variable); // Threadgroup arrays can't have a wrapper type
        std::string variable_decl_function_local(SPIRVariable& variable);

        void add_local_variable_name(uint32_t id);
        void add_resource_name(uint32_t id);
        void add_member_name(SPIRType& type, uint32_t name);
        void add_function_overload(const SPIRFunction& func);

        virtual bool is_non_native_row_major_matrix(uint32_t id);
        virtual bool member_is_non_native_row_major_matrix(const SPIRType& type, uint32_t index);
        bool member_is_remapped_physical_type(const SPIRType& type, uint32_t index) const;
        bool member_is_packed_physical_type(const SPIRType& type, uint32_t index) const;
        virtual std::string convert_row_major_matrix(std::string exp_str, const SPIRType& exp_type,
            uint32_t physical_type_id, bool is_packed,
            bool relaxed = false);

        std::unordered_set<std::string> local_variable_names;
        std::unordered_set<std::string> resource_names;
        std::unordered_set<std::string> block_input_names;
        std::unordered_set<std::string> block_output_names;
        std::unordered_set<std::string> block_ubo_names;
        std::unordered_set<std::string> block_ssbo_names;
        std::unordered_set<std::string> block_names; // A union of all block_*_names.
        std::unordered_map<std::string, std::unordered_set<uint64_t>> function_overloads;
        std::unordered_map<uint32_t, std::string> preserved_aliases;
        void preserve_alias_on_reset(uint32_t id);
        void reset_name_caches();

        bool processing_entry_point = false;

        // Can be overriden by subclass backends for trivial things which
        // shouldn't need polymorphism.
        struct BackendVariations
        {
            std::string discard_literal = "discard";
            std::string demote_literal = "demote";
            std::string null_pointer_literal = "";
            bool float_literal_suffix = false;
            bool double_literal_suffix = true;
            bool uint32_t_literal_suffix = true;
            bool long_long_literal_suffix = false;
            const char* basic_int_type = "int";
            const char* basic_uint_type = "uint";
            const char* basic_int8_type = "int8_t";
            const char* basic_uint8_type = "uint8_t";
            const char* basic_int16_type = "int16_t";
            const char* basic_uint16_type = "uint16_t";
            const char* int16_t_literal_suffix = "s";
            const char* uint16_t_literal_suffix = "us";
            const char* nonuniform_qualifier = "nonuniformEXT";
            const char* boolean_mix_function = "mix";
            SPIRType::BaseType boolean_in_struct_remapped_type = SPIRType::Boolean;
            bool swizzle_is_function = false;
            bool shared_is_implied = false;
            bool unsized_array_supported = true;
            bool explicit_struct_type = false;
            bool use_initializer_list = false;
            bool use_typed_initializer_list = false;
            bool can_declare_struct_inline = true;
            bool can_declare_arrays_inline = true;
            bool native_row_major_matrix = true;
            bool use_constructor_splatting = true;
            bool allow_precision_qualifiers = false;
            bool can_swizzle_scalar = false;
            bool force_gl_in_out_block = false;
            bool force_merged_mesh_block = false;
            bool can_return_array = true;
            bool allow_truncated_access_chain = false;
            bool supports_extensions = false;
            bool supports_empty_struct = false;
            bool array_is_value_type = true;
            bool array_is_value_type_in_buffer_blocks = true;
            bool comparison_image_samples_scalar = false;
            bool native_pointers = false;
            bool support_small_type_sampling_result = false;
            bool support_case_fallthrough = true;
            bool use_array_constructor = false;
            bool needs_row_major_load_workaround = false;
            bool support_pointer_to_pointer = false;
            bool support_precise_qualifier = false;
            bool support_64bit_switch = false;
            bool workgroup_size_is_hidden = false;
            bool requires_relaxed_precision_analysis = false;
            bool implicit_c_integer_promotion_rules = false;
        } backend;

        void emit_struct(SPIRType& type);
        void emit_resources();
        void emit_extension_workarounds(spv::ExecutionModel model);
        void emit_subgroup_arithmetic_workaround(const std::string& func, spv::Op op, spv::GroupOperation group_op);
        void emit_polyfills(uint32_t polyfills, bool relaxed);
        void emit_buffer_block_native(const SPIRVariable& var);
        void emit_buffer_reference_block(uint32_t type_id, bool forward_declaration);
        void emit_buffer_block_legacy(const SPIRVariable& var);
        void emit_buffer_block_flattened(const SPIRVariable& type);
        void fixup_implicit_builtin_block_names(spv::ExecutionModel model);
        void emit_declared_builtin_block(spv::StorageClass storage, spv::ExecutionModel model);
        bool should_force_emit_builtin_block(spv::StorageClass storage);
        void emit_push_constant_block_vulkan(const SPIRVariable& var);
        void emit_push_constant_block_glsl(const SPIRVariable& var);
        void emit_interface_block(const SPIRVariable& type);
        void emit_flattened_io_block(const SPIRVariable& var, const char* qual);
        void emit_flattened_io_block_struct(const std::string& basename, const SPIRType& type, const char* qual,
            const SmallVector<uint32_t>& indices);
        void emit_flattened_io_block_member(const std::string& basename, const SPIRType& type, const char* qual,
            const SmallVector<uint32_t>& indices);
        void emit_block_chain(SPIRBlock& block);
        void emit_hoisted_temporaries(SmallVector<std::pair<TypeID, ID>>& temporaries);
        std::string constant_value_macro_name(uint32_t id);
        int get_constant_mapping_to_workgroup_component(const SPIRConstant& constant) const;
        void emit_constant(const SPIRConstant& constant);
        void emit_specialization_constant_op(const SPIRConstantOp& constant);
        std::string emit_continue_block(uint32_t continue_block, bool follow_true_block, bool follow_false_block);
        bool attempt_emit_loop_header(SPIRBlock& block, SPIRBlock::Method method);

        void branch(BlockID from, BlockID to);
        void branch_to_continue(BlockID from, BlockID to);
        void branch(BlockID from, uint32_t cond, BlockID true_block, BlockID false_block);
        void flush_phi(BlockID from, BlockID to);
        void flush_variable_declaration(uint32_t id);
        void flush_undeclared_variables(SPIRBlock& block);
        void emit_variable_temporary_copies(const SPIRVariable& var);

        bool should_dereference(uint32_t id);
        bool should_forward(uint32_t id) const;
        bool should_suppress_usage_tracking(uint32_t id) const;
        void emit_mix_op(uint32_t result_type, uint32_t id, uint32_t left, uint32_t right, uint32_t lerp);
        void emit_nminmax_op(uint32_t result_type, uint32_t id, uint32_t op0, uint32_t op1, GLSLstd450 op);
        void emit_emulated_ahyper_op(uint32_t result_type, uint32_t result_id, uint32_t op0, GLSLstd450 op);
        bool to_trivial_mix_op(const SPIRType& type, std::string& op, uint32_t left, uint32_t right, uint32_t lerp);
        void emit_quaternary_func_op(uint32_t result_type, uint32_t result_id, uint32_t op0, uint32_t op1, uint32_t op2,
            uint32_t op3, const char* op);
        void emit_trinary_func_op(uint32_t result_type, uint32_t result_id, uint32_t op0, uint32_t op1, uint32_t op2,
            const char* op);
        void emit_binary_func_op(uint32_t result_type, uint32_t result_id, uint32_t op0, uint32_t op1, const char* op);
        void emit_atomic_func_op(uint32_t result_type, uint32_t result_id, uint32_t op0, uint32_t op1, const char* op);
        void emit_atomic_func_op(uint32_t result_type, uint32_t result_id, uint32_t op0, uint32_t op1, uint32_t op2, const char* op);

        void emit_unary_func_op_cast(uint32_t result_type, uint32_t result_id, uint32_t op0, const char* op,
            SPIRType::BaseType input_type, SPIRType::BaseType expected_result_type);
        void emit_binary_func_op_cast(uint32_t result_type, uint32_t result_id, uint32_t op0, uint32_t op1, const char* op,
            SPIRType::BaseType input_type, bool skip_cast_if_equal_type);
        void emit_binary_func_op_cast_clustered(uint32_t result_type, uint32_t result_id, uint32_t op0, uint32_t op1,
            const char* op, SPIRType::BaseType input_type);
        void emit_trinary_func_op_cast(uint32_t result_type, uint32_t result_id, uint32_t op0, uint32_t op1, uint32_t op2,
            const char* op, SPIRType::BaseType input_type);
        void emit_trinary_func_op_bitextract(uint32_t result_type, uint32_t result_id, uint32_t op0, uint32_t op1,
            uint32_t op2, const char* op, SPIRType::BaseType expected_result_type,
            SPIRType::BaseType input_type0, SPIRType::BaseType input_type1,
            SPIRType::BaseType input_type2);
        void emit_bitfield_insert_op(uint32_t result_type, uint32_t result_id, uint32_t op0, uint32_t op1, uint32_t op2,
            uint32_t op3, const char* op, SPIRType::BaseType offset_count_type);

        void emit_unary_func_op(uint32_t result_type, uint32_t result_id, uint32_t op0, const char* op);
        void emit_unrolled_unary_op(uint32_t result_type, uint32_t result_id, uint32_t operand, const char* op);
        void emit_binary_op(uint32_t result_type, uint32_t result_id, uint32_t op0, uint32_t op1, const char* op);
        void emit_unrolled_binary_op(uint32_t result_type, uint32_t result_id, uint32_t op0, uint32_t op1, const char* op,
            bool negate, SPIRType::BaseType expected_type);
        void emit_binary_op_cast(uint32_t result_type, uint32_t result_id, uint32_t op0, uint32_t op1, const char* op,
            SPIRType::BaseType input_type, bool skip_cast_if_equal_type, bool implicit_integer_promotion);

        SPIRType binary_op_bitcast_helper(std::string& cast_op0, std::string& cast_op1, SPIRType::BaseType& input_type,
            uint32_t op0, uint32_t op1, bool skip_cast_if_equal_type);

        virtual bool emit_complex_bitcast(uint32_t result_type, uint32_t id, uint32_t op0);

        std::string to_ternary_expression(const SPIRType& result_type, uint32_t select, uint32_t true_value,
            uint32_t false_value);

        void emit_unary_op(uint32_t result_type, uint32_t result_id, uint32_t op0, const char* op);
        void emit_unary_op_cast(uint32_t result_type, uint32_t result_id, uint32_t op0, const char* op);
        virtual void emit_mesh_tasks(SPIRBlock& block);
        bool expression_is_forwarded(uint32_t id) const;
        bool expression_suppresses_usage_tracking(uint32_t id) const;
        bool expression_read_implies_multiple_reads(uint32_t id) const;
        SPIRExpression& emit_op(uint32_t result_type, uint32_t result_id, const std::string& rhs, bool forward_rhs,
            bool suppress_usage_tracking = false);

        void access_chain_internal_append_index(std::string& expr, uint32_t base, const SPIRType* type,
            AccessChainFlags flags, bool& access_chain_is_arrayed, uint32_t index);

        std::string access_chain_internal(uint32_t base, const uint32_t* indices, uint32_t count, AccessChainFlags flags,
            AccessChainMeta* meta);

        spv::StorageClass get_expression_effective_storage_class(uint32_t ptr);
        virtual bool access_chain_needs_stage_io_builtin_translation(uint32_t base);

        virtual void check_physical_type_cast(std::string& expr, const SPIRType* type, uint32_t physical_type);
        virtual void prepare_access_chain_for_scalar_access(std::string& expr, const SPIRType& type,
            spv::StorageClass storage, bool& is_packed);

        std::string access_chain(uint32_t base, const uint32_t* indices, uint32_t count, const SPIRType& target_type,
            AccessChainMeta* meta = nullptr, bool ptr_chain = false);

        std::string flattened_access_chain(uint32_t base, const uint32_t* indices, uint32_t count,
            const SPIRType& target_type, uint32_t offset, uint32_t matrix_stride,
            uint32_t array_stride, bool need_transpose);
        std::string flattened_access_chain_struct(uint32_t base, const uint32_t* indices, uint32_t count,
            const SPIRType& target_type, uint32_t offset);
        std::string flattened_access_chain_matrix(uint32_t base, const uint32_t* indices, uint32_t count,
            const SPIRType& target_type, uint32_t offset, uint32_t matrix_stride,
            bool need_transpose);
        std::string flattened_access_chain_vector(uint32_t base, const uint32_t* indices, uint32_t count,
            const SPIRType& target_type, uint32_t offset, uint32_t matrix_stride,
            bool need_transpose);
        std::pair<std::string, uint32_t> flattened_access_chain_offset(const SPIRType& basetype, const uint32_t* indices,
            uint32_t count, uint32_t offset,
            uint32_t word_stride, bool* need_transpose = nullptr,
            uint32_t* matrix_stride = nullptr,
            uint32_t* array_stride = nullptr,
            bool ptr_chain = false);

        const char* index_to_swizzle(uint32_t index);
        std::string remap_swizzle(const SPIRType& result_type, uint32_t input_components, const std::string& expr);
        std::string declare_temporary(uint32_t type, uint32_t id);
        void emit_uninitialized_temporary(uint32_t type, uint32_t id);
        SPIRExpression& emit_uninitialized_temporary_expression(uint32_t type, uint32_t id);
        void append_global_func_args(const SPIRFunction& func, uint32_t index, SmallVector<std::string>& arglist);
        std::string to_non_uniform_aware_expression(uint32_t id);
        std::string to_expression(uint32_t id, bool register_expression_read = true);
        std::string to_composite_constructor_expression(const SPIRType& parent_type, uint32_t id, bool block_like_type);
        std::string to_rerolled_array_expression(const SPIRType& parent_type, const std::string& expr, const SPIRType& type);
        std::string to_enclosed_expression(uint32_t id, bool register_expression_read = true);
        std::string to_unpacked_expression(uint32_t id, bool register_expression_read = true);
        std::string to_unpacked_row_major_matrix_expression(uint32_t id);
        std::string to_enclosed_unpacked_expression(uint32_t id, bool register_expression_read = true);
        std::string to_dereferenced_expression(uint32_t id, bool register_expression_read = true);
        std::string to_pointer_expression(uint32_t id, bool register_expression_read = true);
        std::string to_enclosed_pointer_expression(uint32_t id, bool register_expression_read = true);
        std::string to_extract_component_expression(uint32_t id, uint32_t index);
        std::string to_extract_constant_composite_expression(uint32_t result_type, const SPIRConstant& c,
            const uint32_t* chain, uint32_t length);
        static bool needs_enclose_expression(const std::string& expr);
        std::string enclose_expression(const std::string& expr);
        std::string dereference_expression(const SPIRType& expression_type, const std::string& expr);
        std::string address_of_expression(const std::string& expr);
        void strip_enclosed_expression(std::string& expr);
        std::string to_member_name(const SPIRType& type, uint32_t index);
        virtual std::string to_member_reference(uint32_t base, const SPIRType& type, uint32_t index, bool ptr_chain_is_resolved);
        std::string to_multi_member_reference(const SPIRType& type, const SmallVector<uint32_t>& indices);
        std::string type_to_glsl_constructor(const SPIRType& type);
        std::string argument_decl(const SPIRFunction::Parameter& arg);
        virtual std::string to_qualifiers_glsl(uint32_t id);
        void fixup_io_block_patch_primitive_qualifiers(const SPIRVariable& var);
        void emit_output_variable_initializer(const SPIRVariable& var);
        std::string to_precision_qualifiers_glsl(uint32_t id);
        virtual const char* to_storage_qualifiers_glsl(const SPIRVariable& var);
        std::string flags_to_qualifiers_glsl(const SPIRType& type, const Bitset& flags);
        const char* format_to_glsl(spv::ImageFormat format);
        virtual std::string layout_for_member(const SPIRType& type, uint32_t index);
        virtual std::string to_interpolation_qualifiers(const Bitset& flags);
        std::string layout_for_variable(const SPIRVariable& variable);
        std::string to_combined_image_sampler(VariableID image_id, VariableID samp_id);
        virtual bool skip_argument(uint32_t id) const;
        virtual bool emit_array_copy(const char* expr, uint32_t lhs_id, uint32_t rhs_id,
            spv::StorageClass lhs_storage, spv::StorageClass rhs_storage);
        virtual void emit_block_hints(const SPIRBlock& block);
        virtual std::string to_initializer_expression(const SPIRVariable& var);
        virtual std::string to_zero_initialized_expression(uint32_t type_id);
        bool type_can_zero_initialize(const SPIRType& type) const;

        bool buffer_is_packing_standard(const SPIRType& type, BufferPackingStandard packing,
            uint32_t* failed_index = nullptr, uint32_t start_offset = 0,
            uint32_t end_offset = ~(0u));
        std::string buffer_to_packing_standard(const SPIRType& type, bool support_std430_without_scalar_layout);

        uint32_t type_to_packed_base_size(const SPIRType& type, BufferPackingStandard packing);
        uint32_t type_to_packed_alignment(const SPIRType& type, const Bitset& flags, BufferPackingStandard packing);
        uint32_t type_to_packed_array_stride(const SPIRType& type, const Bitset& flags, BufferPackingStandard packing);
        uint32_t type_to_packed_size(const SPIRType& type, const Bitset& flags, BufferPackingStandard packing);
        uint32_t type_to_location_count(const SPIRType& type) const;

        std::string bitcast_glsl(const SPIRType& result_type, uint32_t arg);
        virtual std::string bitcast_glsl_op(const SPIRType& result_type, const SPIRType& argument_type);

        std::string bitcast_expression(SPIRType::BaseType target_type, uint32_t arg);
        std::string bitcast_expression(const SPIRType& target_type, SPIRType::BaseType expr_type, const std::string& expr);

        std::string build_composite_combiner(uint32_t result_type, const uint32_t* elems, uint32_t length);
        bool remove_duplicate_swizzle(std::string& op);
        bool remove_unity_swizzle(uint32_t base, std::string& op);

        // Can modify flags to remote readonly/writeonly if image type
        // and force recompile.
        bool check_atomic_image(uint32_t id);

        virtual void replace_illegal_names();
        void replace_illegal_names(const std::unordered_set<std::string>& keywords);
        virtual void emit_entry_point_declarations();

        void replace_fragment_output(SPIRVariable& var);
        void replace_fragment_outputs();
        std::string legacy_tex_op(const std::string& op, const SPIRType& imgtype, uint32_t id);

        void forward_relaxed_precision(uint32_t dst_id, const uint32_t* args, uint32_t length);
        void analyze_precision_requirements(uint32_t type_id, uint32_t dst_id, uint32_t* args, uint32_t length);
        Options::Precision analyze_expression_precision(const uint32_t* args, uint32_t length) const;

        uint32_t indent = 0;

        std::unordered_set<uint32_t> emitted_functions;

        // Ensure that we declare phi-variable copies even if the original declaration isn't deferred
        std::unordered_set<uint32_t> flushed_phi_variables;

        std::unordered_set<uint32_t> flattened_buffer_blocks;
        std::unordered_map<uint32_t, bool> flattened_structs;

        ShaderSubgroupSupportHelper shader_subgroup_supporter;

        std::string load_flattened_struct(const std::string& basename, const SPIRType& type);
        std::string to_flattened_struct_member(const std::string& basename, const SPIRType& type, uint32_t index);
        void store_flattened_struct(uint32_t lhs_id, uint32_t value);
        void store_flattened_struct(const std::string& basename, uint32_t rhs, const SPIRType& type,
            const SmallVector<uint32_t>& indices);
        std::string to_flattened_access_chain_expression(uint32_t id);

        // Usage tracking. If a temporary is used more than once, use the temporary instead to
        // avoid AST explosion when SPIRV is generated with pure SSA and doesn't write stuff to variables.
        std::unordered_map<uint32_t, uint32_t> expression_usage_counts;
        void track_expression_read(uint32_t id);

        SmallVector<std::string> forced_extensions;
        SmallVector<std::string> header_lines;

        // Used when expressions emit extra opcodes with their own unique IDs,
        // and we need to reuse the IDs across recompilation loops.
        // Currently used by NMin/Max/Clamp implementations.
        std::unordered_map<uint32_t, uint32_t> extra_sub_expressions;

        SmallVector<TypeID> workaround_ubo_load_overload_types;
        void request_workaround_wrapper_overload(TypeID id);
        void rewrite_load_for_wrapped_row_major(std::string& expr, TypeID loaded_type, ID ptr);

        uint32_t statement_count = 0;

        inline bool is_legacy() const
        {
            return (options.es && options.version < 300) || (!options.es && options.version < 130);
        }

        inline bool is_legacy_es() const
        {
            return options.es && options.version < 300;
        }

        inline bool is_legacy_desktop() const
        {
            return !options.es && options.version < 130;
        }

        enum Polyfill : uint32_t
        {
            PolyfillTranspose2x2 = 1 << 0,
            PolyfillTranspose3x3 = 1 << 1,
            PolyfillTranspose4x4 = 1 << 2,
            PolyfillDeterminant2x2 = 1 << 3,
            PolyfillDeterminant3x3 = 1 << 4,
            PolyfillDeterminant4x4 = 1 << 5,
            PolyfillMatrixInverse2x2 = 1 << 6,
            PolyfillMatrixInverse3x3 = 1 << 7,
            PolyfillMatrixInverse4x4 = 1 << 8,
        };

        uint32_t required_polyfills = 0;
        uint32_t required_polyfills_relaxed = 0;
        void require_polyfill(Polyfill polyfill, bool relaxed);

        bool ray_tracing_is_khr = false;
        bool barycentric_is_nv = false;
        void ray_tracing_khr_fixup_locations();

        bool args_will_forward(uint32_t id, const uint32_t* args, uint32_t num_args, bool pure);
        void register_call_out_argument(uint32_t id);
        void register_impure_function_call();
        void register_control_dependent_expression(uint32_t expr);

        // GL_EXT_shader_pixel_local_storage support.
        std::vector<PlsRemap> pls_inputs;
        std::vector<PlsRemap> pls_outputs;
        std::string pls_decl(const PlsRemap& variable);
        const char* to_pls_qualifiers_glsl(const SPIRVariable& variable);
        void emit_pls();
        void remap_pls_variables();

        // GL_EXT_shader_framebuffer_fetch support.
        std::vector<std::pair<uint32_t, uint32_t>> subpass_to_framebuffer_fetch_attachment;
        std::vector<std::pair<uint32_t, bool>> inout_color_attachments;
        bool location_is_framebuffer_fetch(uint32_t location) const;
        bool location_is_non_coherent_framebuffer_fetch(uint32_t location) const;
        bool subpass_input_is_framebuffer_fetch(uint32_t id) const;
        void emit_inout_fragment_outputs_copy_to_subpass_inputs();
        const SPIRVariable* find_subpass_input_by_attachment_index(uint32_t index) const;
        const SPIRVariable* find_color_output_by_location(uint32_t location) const;

        // A variant which takes two sets of name. The secondary is only used to verify there are no collisions,
        // but the set is not updated when we have found a new name.
        // Used primarily when adding block interface names.
        void add_variable(std::unordered_set<std::string>& variables_primary,
            const std::unordered_set<std::string>& variables_secondary, std::string& name);

        void check_function_call_constraints(const uint32_t* args, uint32_t length);
        void handle_invalid_expression(uint32_t id);
        void force_temporary_and_recompile(uint32_t id);
        void find_static_extensions();

        uint32_t consume_temporary_in_precision_context(uint32_t type_id, uint32_t id, Options::Precision precision);
        std::unordered_map<uint32_t, uint32_t> temporary_to_mirror_precision_alias;
        std::unordered_set<uint32_t> composite_insert_overwritten;
        std::unordered_set<uint32_t> block_composite_insert_overwrite;

        std::string emit_for_loop_initializers(const SPIRBlock& block);
        void emit_while_loop_initializers(const SPIRBlock& block);
        bool for_loop_initializers_are_same_type(const SPIRBlock& block);
        bool optimize_read_modify_write(const SPIRType& type, const std::string& lhs, const std::string& rhs);
        void fixup_image_load_store_access();

        bool type_is_empty(const SPIRType& type);

        bool can_use_io_location(spv::StorageClass storage, bool block);
        const Instruction* get_next_instruction_in_block(const Instruction& instr);
        static uint32_t mask_relevant_memory_semantics(uint32_t semantics);

        std::string convert_half_to_string(const SPIRConstant& value, uint32_t col, uint32_t row);
        std::string convert_float_to_string(const SPIRConstant& value, uint32_t col, uint32_t row);
        std::string convert_double_to_string(const SPIRConstant& value, uint32_t col, uint32_t row);

        std::string convert_separate_image_to_expression(uint32_t id);

        // Builtins in GLSL are always specific signedness, but the SPIR-V can declare them
        // as either unsigned or signed.
        // Sometimes we will need to automatically perform casts on load and store to make this work.
        virtual void cast_to_variable_store(uint32_t target_id, std::string& expr, const SPIRType& expr_type);
        virtual void cast_from_variable_load(uint32_t source_id, std::string& expr, const SPIRType& expr_type);
        void unroll_array_from_complex_load(uint32_t target_id, uint32_t source_id, std::string& expr);
        bool unroll_array_to_complex_store(uint32_t target_id, uint32_t source_id);
        void convert_non_uniform_expression(std::string& expr, uint32_t ptr_id);

        void handle_store_to_invariant_variable(uint32_t store_id, uint32_t value_id);
        void disallow_forwarding_in_expression_chain(const SPIRExpression& expr);

        bool expression_is_constant_null(uint32_t id) const;
        bool expression_is_non_value_type_array(uint32_t ptr);
        virtual void emit_store_statement(uint32_t lhs_expression, uint32_t rhs_expression);

        uint32_t get_integer_width_for_instruction(const Instruction& instr) const;
        uint32_t get_integer_width_for_glsl_instruction(GLSLstd450 op, const uint32_t* arguments, uint32_t length) const;

        bool variable_is_lut(const SPIRVariable& var) const;

        char current_locale_radix_character = '.';

        void fixup_type_alias();
        void reorder_type_alias();
        void fixup_anonymous_struct_names();
        void fixup_anonymous_struct_names(std::unordered_set<uint32_t>& visited, const SPIRType& type);

        static const char* vector_swizzle(int vecsize, int index);

        bool is_stage_output_location_masked(uint32_t location, uint32_t component) const;
        bool is_stage_output_builtin_masked(spv::BuiltIn builtin) const;
        bool is_stage_output_variable_masked(const SPIRVariable& var) const;
        bool is_stage_output_block_member_masked(const SPIRVariable& var, uint32_t index, bool strip_array) const;
        bool is_per_primitive_variable(const SPIRVariable& var) const;
        uint32_t get_accumulated_member_location(const SPIRVariable& var, uint32_t mbr_idx, bool strip_array) const;
        uint32_t get_declared_member_location(const SPIRVariable& var, uint32_t mbr_idx, bool strip_array) const;
        std::unordered_set<LocationComponentPair, InternalHasher> masked_output_locations;
        std::unordered_set<uint32_t> masked_output_builtins;

    private:
        void init();

        SmallVector<ConstantID> get_composite_constant_ids(ConstantID const_id);
        void fill_composite_constant(SPIRConstant& constant, TypeID type_id, const SmallVector<ConstantID>& initializers);
        void set_composite_constant(ConstantID const_id, TypeID type_id, const SmallVector<ConstantID>& initializers);
        TypeID get_composite_member_type(TypeID type_id, uint32_t member_idx);
        std::unordered_map<uint32_t, SmallVector<ConstantID>> const_composite_insert_ids;
    };
} // namespace SPIRV_CROSS_NAMESPACE

#endif

#include <map>
#include <set>
#include <stddef.h>
#include <unordered_map>
#include <unordered_set>

namespace SPIRV_CROSS_NAMESPACE
{

    // Indicates the format of a shader interface variable. Currently limited to specifying
    // if the input is an 8-bit unsigned integer, 16-bit unsigned integer, or
    // some other format.
    enum MSLShaderVariableFormat
    {
        MSL_SHADER_VARIABLE_FORMAT_OTHER = 0,
        MSL_SHADER_VARIABLE_FORMAT_UINT8 = 1,
        MSL_SHADER_VARIABLE_FORMAT_UINT16 = 2,
        MSL_SHADER_VARIABLE_FORMAT_ANY16 = 3,
        MSL_SHADER_VARIABLE_FORMAT_ANY32 = 4,

        // Deprecated aliases.
        MSL_VERTEX_FORMAT_OTHER = MSL_SHADER_VARIABLE_FORMAT_OTHER,
        MSL_VERTEX_FORMAT_UINT8 = MSL_SHADER_VARIABLE_FORMAT_UINT8,
        MSL_VERTEX_FORMAT_UINT16 = MSL_SHADER_VARIABLE_FORMAT_UINT16,
        MSL_SHADER_INPUT_FORMAT_OTHER = MSL_SHADER_VARIABLE_FORMAT_OTHER,
        MSL_SHADER_INPUT_FORMAT_UINT8 = MSL_SHADER_VARIABLE_FORMAT_UINT8,
        MSL_SHADER_INPUT_FORMAT_UINT16 = MSL_SHADER_VARIABLE_FORMAT_UINT16,
        MSL_SHADER_INPUT_FORMAT_ANY16 = MSL_SHADER_VARIABLE_FORMAT_ANY16,
        MSL_SHADER_INPUT_FORMAT_ANY32 = MSL_SHADER_VARIABLE_FORMAT_ANY32,

        MSL_SHADER_VARIABLE_FORMAT_INT_MAX = 0x7fffffff
    };

    // Indicates the rate at which a variable changes value, one of: per-vertex,
    // per-primitive, or per-patch.
    enum MSLShaderVariableRate
    {
        MSL_SHADER_VARIABLE_RATE_PER_VERTEX = 0,
        MSL_SHADER_VARIABLE_RATE_PER_PRIMITIVE = 1,
        MSL_SHADER_VARIABLE_RATE_PER_PATCH = 2,

        MSL_SHADER_VARIABLE_RATE_INT_MAX = 0x7fffffff,
    };

    // Defines MSL characteristics of a shader interface variable at a particular location.
    // After compilation, it is possible to query whether or not this location was used.
    // If vecsize is nonzero, it must be greater than or equal to the vecsize declared in the shader,
    // or behavior is undefined.
    struct MSLShaderInterfaceVariable
    {
        uint32_t location = 0;
        uint32_t component = 0;
        MSLShaderVariableFormat format = MSL_SHADER_VARIABLE_FORMAT_OTHER;
        spv::BuiltIn builtin = spv::BuiltInMax;
        uint32_t vecsize = 0;
        MSLShaderVariableRate rate = MSL_SHADER_VARIABLE_RATE_PER_VERTEX;
    };

    // Matches the binding index of a MSL resource for a binding within a descriptor set.
    // Taken together, the stage, desc_set and binding combine to form a reference to a resource
    // descriptor used in a particular shading stage. The count field indicates the number of
    // resources consumed by this binding, if the binding represents an array of resources.
    // If the resource array is a run-time-sized array, which are legal in GLSL or SPIR-V, this value
    // will be used to declare the array size in MSL, which does not support run-time-sized arrays.
    // If pad_argument_buffer_resources is enabled, the base_type and count values are used to
    // specify the base type and array size of the resource in the argument buffer, if that resource
    // is not defined and used by the shader. With pad_argument_buffer_resources enabled, this
    // information will be used to pad the argument buffer structure, in order to align that
    // structure consistently for all uses, across all shaders, of the descriptor set represented
    // by the arugment buffer. If pad_argument_buffer_resources is disabled, base_type does not
    // need to be populated, and if the resource is also not a run-time sized array, the count
    // field does not need to be populated.
    // If using MSL 2.0 argument buffers, the descriptor set is not marked as a discrete descriptor set,
    // and (for iOS only) the resource is not a storage image (sampled != 2), the binding reference we
    // remap to will become an [[id(N)]] attribute within the "descriptor set" argument buffer structure.
    // For resources which are bound in the "classic" MSL 1.0 way or discrete descriptors, the remap will
    // become a [[buffer(N)]], [[texture(N)]] or [[sampler(N)]] depending on the resource types used.
    struct MSLResourceBinding
    {
        spv::ExecutionModel stage = spv::ExecutionModelMax;
        SPIRType::BaseType basetype = SPIRType::Unknown;
        uint32_t desc_set = 0;
        uint32_t binding = 0;
        uint32_t count = 0;
        uint32_t msl_buffer = 0;
        uint32_t msl_texture = 0;
        uint32_t msl_sampler = 0;
    };

    enum MSLSamplerCoord
    {
        MSL_SAMPLER_COORD_NORMALIZED = 0,
        MSL_SAMPLER_COORD_PIXEL = 1,
        MSL_SAMPLER_INT_MAX = 0x7fffffff
    };

    enum MSLSamplerFilter
    {
        MSL_SAMPLER_FILTER_NEAREST = 0,
        MSL_SAMPLER_FILTER_LINEAR = 1,
        MSL_SAMPLER_FILTER_INT_MAX = 0x7fffffff
    };

    enum MSLSamplerMipFilter
    {
        MSL_SAMPLER_MIP_FILTER_NONE = 0,
        MSL_SAMPLER_MIP_FILTER_NEAREST = 1,
        MSL_SAMPLER_MIP_FILTER_LINEAR = 2,
        MSL_SAMPLER_MIP_FILTER_INT_MAX = 0x7fffffff
    };

    enum MSLSamplerAddress
    {
        MSL_SAMPLER_ADDRESS_CLAMP_TO_ZERO = 0,
        MSL_SAMPLER_ADDRESS_CLAMP_TO_EDGE = 1,
        MSL_SAMPLER_ADDRESS_CLAMP_TO_BORDER = 2,
        MSL_SAMPLER_ADDRESS_REPEAT = 3,
        MSL_SAMPLER_ADDRESS_MIRRORED_REPEAT = 4,
        MSL_SAMPLER_ADDRESS_INT_MAX = 0x7fffffff
    };

    enum MSLSamplerCompareFunc
    {
        MSL_SAMPLER_COMPARE_FUNC_NEVER = 0,
        MSL_SAMPLER_COMPARE_FUNC_LESS = 1,
        MSL_SAMPLER_COMPARE_FUNC_LESS_EQUAL = 2,
        MSL_SAMPLER_COMPARE_FUNC_GREATER = 3,
        MSL_SAMPLER_COMPARE_FUNC_GREATER_EQUAL = 4,
        MSL_SAMPLER_COMPARE_FUNC_EQUAL = 5,
        MSL_SAMPLER_COMPARE_FUNC_NOT_EQUAL = 6,
        MSL_SAMPLER_COMPARE_FUNC_ALWAYS = 7,
        MSL_SAMPLER_COMPARE_FUNC_INT_MAX = 0x7fffffff
    };

    enum MSLSamplerBorderColor
    {
        MSL_SAMPLER_BORDER_COLOR_TRANSPARENT_BLACK = 0,
        MSL_SAMPLER_BORDER_COLOR_OPAQUE_BLACK = 1,
        MSL_SAMPLER_BORDER_COLOR_OPAQUE_WHITE = 2,
        MSL_SAMPLER_BORDER_COLOR_INT_MAX = 0x7fffffff
    };

    enum MSLFormatResolution
    {
        MSL_FORMAT_RESOLUTION_444 = 0,
        MSL_FORMAT_RESOLUTION_422,
        MSL_FORMAT_RESOLUTION_420,
        MSL_FORMAT_RESOLUTION_INT_MAX = 0x7fffffff
    };

    enum MSLChromaLocation
    {
        MSL_CHROMA_LOCATION_COSITED_EVEN = 0,
        MSL_CHROMA_LOCATION_MIDPOINT,
        MSL_CHROMA_LOCATION_INT_MAX = 0x7fffffff
    };

    enum MSLComponentSwizzle
    {
        MSL_COMPONENT_SWIZZLE_IDENTITY = 0,
        MSL_COMPONENT_SWIZZLE_ZERO,
        MSL_COMPONENT_SWIZZLE_ONE,
        MSL_COMPONENT_SWIZZLE_R,
        MSL_COMPONENT_SWIZZLE_G,
        MSL_COMPONENT_SWIZZLE_B,
        MSL_COMPONENT_SWIZZLE_A,
        MSL_COMPONENT_SWIZZLE_INT_MAX = 0x7fffffff
    };

    enum MSLSamplerYCbCrModelConversion
    {
        MSL_SAMPLER_YCBCR_MODEL_CONVERSION_RGB_IDENTITY = 0,
        MSL_SAMPLER_YCBCR_MODEL_CONVERSION_YCBCR_IDENTITY,
        MSL_SAMPLER_YCBCR_MODEL_CONVERSION_YCBCR_BT_709,
        MSL_SAMPLER_YCBCR_MODEL_CONVERSION_YCBCR_BT_601,
        MSL_SAMPLER_YCBCR_MODEL_CONVERSION_YCBCR_BT_2020,
        MSL_SAMPLER_YCBCR_MODEL_CONVERSION_INT_MAX = 0x7fffffff
    };

    enum MSLSamplerYCbCrRange
    {
        MSL_SAMPLER_YCBCR_RANGE_ITU_FULL = 0,
        MSL_SAMPLER_YCBCR_RANGE_ITU_NARROW,
        MSL_SAMPLER_YCBCR_RANGE_INT_MAX = 0x7fffffff
    };

    struct MSLConstexprSampler
    {
        MSLSamplerCoord coord = MSL_SAMPLER_COORD_NORMALIZED;
        MSLSamplerFilter min_filter = MSL_SAMPLER_FILTER_NEAREST;
        MSLSamplerFilter mag_filter = MSL_SAMPLER_FILTER_NEAREST;
        MSLSamplerMipFilter mip_filter = MSL_SAMPLER_MIP_FILTER_NONE;
        MSLSamplerAddress s_address = MSL_SAMPLER_ADDRESS_CLAMP_TO_EDGE;
        MSLSamplerAddress t_address = MSL_SAMPLER_ADDRESS_CLAMP_TO_EDGE;
        MSLSamplerAddress r_address = MSL_SAMPLER_ADDRESS_CLAMP_TO_EDGE;
        MSLSamplerCompareFunc compare_func = MSL_SAMPLER_COMPARE_FUNC_NEVER;
        MSLSamplerBorderColor border_color = MSL_SAMPLER_BORDER_COLOR_TRANSPARENT_BLACK;
        float lod_clamp_min = 0.0f;
        float lod_clamp_max = 1000.0f;
        int max_anisotropy = 1;

        // Sampler Y'CbCr conversion parameters
        uint32_t planes = 0;
        MSLFormatResolution resolution = MSL_FORMAT_RESOLUTION_444;
        MSLSamplerFilter chroma_filter = MSL_SAMPLER_FILTER_NEAREST;
        MSLChromaLocation x_chroma_offset = MSL_CHROMA_LOCATION_COSITED_EVEN;
        MSLChromaLocation y_chroma_offset = MSL_CHROMA_LOCATION_COSITED_EVEN;
        MSLComponentSwizzle swizzle[4]; // IDENTITY, IDENTITY, IDENTITY, IDENTITY
        MSLSamplerYCbCrModelConversion ycbcr_model = MSL_SAMPLER_YCBCR_MODEL_CONVERSION_RGB_IDENTITY;
        MSLSamplerYCbCrRange ycbcr_range = MSL_SAMPLER_YCBCR_RANGE_ITU_FULL;
        uint32_t bpc = 8;

        bool compare_enable = false;
        bool lod_clamp_enable = false;
        bool anisotropy_enable = false;
        bool ycbcr_conversion_enable = false;

        MSLConstexprSampler()
        {
            for (uint32_t i = 0; i < 4; i++)
                swizzle[i] = MSL_COMPONENT_SWIZZLE_IDENTITY;
        }
        bool swizzle_is_identity() const
        {
            return (swizzle[0] == MSL_COMPONENT_SWIZZLE_IDENTITY && swizzle[1] == MSL_COMPONENT_SWIZZLE_IDENTITY &&
                swizzle[2] == MSL_COMPONENT_SWIZZLE_IDENTITY && swizzle[3] == MSL_COMPONENT_SWIZZLE_IDENTITY);
        }
        bool swizzle_has_one_or_zero() const
        {
            return (swizzle[0] == MSL_COMPONENT_SWIZZLE_ZERO || swizzle[0] == MSL_COMPONENT_SWIZZLE_ONE ||
                swizzle[1] == MSL_COMPONENT_SWIZZLE_ZERO || swizzle[1] == MSL_COMPONENT_SWIZZLE_ONE ||
                swizzle[2] == MSL_COMPONENT_SWIZZLE_ZERO || swizzle[2] == MSL_COMPONENT_SWIZZLE_ONE ||
                swizzle[3] == MSL_COMPONENT_SWIZZLE_ZERO || swizzle[3] == MSL_COMPONENT_SWIZZLE_ONE);
        }
    };

    // Special constant used in a MSLResourceBinding desc_set
    // element to indicate the bindings for the push constants.
    // Kinda deprecated. Just use ResourceBindingPushConstant{DescriptorSet,Binding} directly.
    static const uint32_t kPushConstDescSet = ResourceBindingPushConstantDescriptorSet;

    // Special constant used in a MSLResourceBinding binding
    // element to indicate the bindings for the push constants.
    // Kinda deprecated. Just use ResourceBindingPushConstant{DescriptorSet,Binding} directly.
    static const uint32_t kPushConstBinding = ResourceBindingPushConstantBinding;

    // Special constant used in a MSLResourceBinding binding
    // element to indicate the buffer binding for swizzle buffers.
    static const uint32_t kSwizzleBufferBinding = ~(1u);

    // Special constant used in a MSLResourceBinding binding
    // element to indicate the buffer binding for buffer size buffers to support OpArrayLength.
    static const uint32_t kBufferSizeBufferBinding = ~(2u);

    // Special constant used in a MSLResourceBinding binding
    // element to indicate the buffer binding used for the argument buffer itself.
    // This buffer binding should be kept as small as possible as all automatic bindings for buffers
    // will start at max(kArgumentBufferBinding) + 1.
    static const uint32_t kArgumentBufferBinding = ~(3u);

    static const uint32_t kMaxArgumentBuffers = 8;

    // The arbitrary maximum for the nesting of array of array copies.
    static const uint32_t kArrayCopyMultidimMax = 6;

    // Decompiles SPIR-V to Metal Shading Language
    class CompilerMSL : public CompilerGLSL
    {
    public:
        // Options for compiling to Metal Shading Language
        struct Options
        {
            typedef enum
            {
                iOS = 0,
                macOS = 1
            } Platform;

            Platform platform = macOS;
            uint32_t msl_version = make_msl_version(1, 2);
            uint32_t texel_buffer_texture_width = 4096; // Width of 2D Metal textures used as 1D texel buffers
            uint32_t r32ui_linear_texture_alignment = 4;
            uint32_t r32ui_alignment_constant_id = 65535;
            uint32_t swizzle_buffer_index = 30;
            uint32_t indirect_params_buffer_index = 29;
            uint32_t shader_output_buffer_index = 28;
            uint32_t shader_patch_output_buffer_index = 27;
            uint32_t shader_tess_factor_buffer_index = 26;
            uint32_t buffer_size_buffer_index = 25;
            uint32_t view_mask_buffer_index = 24;
            uint32_t dynamic_offsets_buffer_index = 23;
            uint32_t shader_input_buffer_index = 22;
            uint32_t shader_index_buffer_index = 21;
            uint32_t shader_patch_input_buffer_index = 20;
            uint32_t shader_input_wg_index = 0;
            uint32_t device_index = 0;
            uint32_t enable_frag_output_mask = 0xffffffff;
            // Metal doesn't allow setting a fixed sample mask directly in the pipeline.
            // We can evade this restriction by ANDing the internal sample_mask output
            // of the shader with the additional fixed sample mask.
            uint32_t additional_fixed_sample_mask = 0xffffffff;
            bool enable_point_size_builtin = true;
            bool enable_frag_depth_builtin = true;
            bool enable_frag_stencil_ref_builtin = true;
            bool disable_rasterization = false;
            bool capture_output_to_buffer = false;
            bool swizzle_texture_samples = false;
            bool tess_domain_origin_lower_left = false;
            bool multiview = false;
            bool multiview_layered_rendering = true;
            bool view_index_from_device_index = false;
            bool dispatch_base = false;
            bool texture_1D_as_2D = false;

            // Enable use of Metal argument buffers.
            // MSL 2.0 must also be enabled.
            bool argument_buffers = false;

            // Defines Metal argument buffer tier levels.
            // Uses same values as Metal MTLArgumentBuffersTier enumeration.
            enum class ArgumentBuffersTier
            {
                Tier1 = 0,
                Tier2 = 1,
            };

            // When using Metal argument buffers, indicates the Metal argument buffer tier level supported by the Metal platform.
            // Ignored when Options::argument_buffers is disabled.
            // - Tier1 supports writable images on macOS, but not on iOS.
            // - Tier2 supports writable images on macOS and iOS, and higher resource count limits.
            // Tier capabilities based on recommendations from Apple engineering.
            ArgumentBuffersTier argument_buffers_tier = ArgumentBuffersTier::Tier1;

            // Ensures vertex and instance indices start at zero. This reflects the behavior of HLSL with SV_VertexID and SV_InstanceID.
            bool enable_base_index_zero = false;

            // Fragment output in MSL must have at least as many components as the render pass.
            // Add support to explicit pad out components.
            bool pad_fragment_output_components = false;

            // Specifies whether the iOS target version supports the [[base_vertex]] and [[base_instance]] attributes.
            bool ios_support_base_vertex_instance = false;

            // Use Metal's native frame-buffer fetch API for subpass inputs.
            bool use_framebuffer_fetch_subpasses = false;

            // Enables use of "fma" intrinsic for invariant float math
            bool invariant_float_math = false;

            // Emulate texturecube_array with texture2d_array for iOS where this type is not available
            bool emulate_cube_array = false;

            // Allow user to enable decoration binding
            bool enable_decoration_binding = false;

            // Requires MSL 2.1, use the native support for texel buffers.
            bool texture_buffer_native = false;

            // Forces all resources which are part of an argument buffer to be considered active.
            // This ensures ABI compatibility between shaders where some resources might be unused,
            // and would otherwise declare a different IAB.
            bool force_active_argument_buffer_resources = false;

            // Aligns each resource in an argument buffer to its assigned index value, id(N),
            // by adding synthetic padding members in the argument buffer struct for any resources
            // in the argument buffer that are not defined and used by the shader. This allows
            // the shader to index into the correct argument in a descriptor set argument buffer
            // that is shared across shaders, where not all resources in the argument buffer are
            // defined in each shader. For this to work, an MSLResourceBinding must be provided for
            // all descriptors in any descriptor set held in an argument buffer in the shader, and
            // that MSLResourceBinding must have the basetype and count members populated correctly.
            // The implementation here assumes any inline blocks in the argument buffer is provided
            // in a Metal buffer, and doesn't take into consideration inline blocks that are
            // optionally embedded directly into the argument buffer via add_inline_uniform_block().
            bool pad_argument_buffer_resources = false;

            // Forces the use of plain arrays, which works around certain driver bugs on certain versions
            // of Intel Macbooks. See https://github.com/KhronosGroup/SPIRV-Cross/issues/1210.
            // May reduce performance in scenarios where arrays are copied around as value-types.
            bool force_native_arrays = false;

            // If a shader writes clip distance, also emit user varyings which
            // can be read in subsequent stages.
            bool enable_clip_distance_user_varying = true;

            // In a tessellation control shader, assume that more than one patch can be processed in a
            // single workgroup. This requires changes to the way the InvocationId and PrimitiveId
            // builtins are processed, but should result in more efficient usage of the GPU.
            bool multi_patch_workgroup = false;

            // Use storage buffers instead of vertex-style attributes for tessellation evaluation
            // input. This may require conversion of inputs in the generated post-tessellation
            // vertex shader, but allows the use of nested arrays.
            bool raw_buffer_tese_input = false;

            // If set, a vertex shader will be compiled as part of a tessellation pipeline.
            // It will be translated as a compute kernel, so it can use the global invocation ID
            // to index the output buffer.
            bool vertex_for_tessellation = false;

            // Assume that SubpassData images have multiple layers. Layered input attachments
            // are addressed relative to the Layer output from the vertex pipeline. This option
            // has no effect with multiview, since all input attachments are assumed to be layered
            // and will be addressed using the current ViewIndex.
            bool arrayed_subpass_input = false;

            // Whether to use SIMD-group or quadgroup functions to implement group non-uniform
            // operations. Some GPUs on iOS do not support the SIMD-group functions, only the
            // quadgroup functions.
            bool ios_use_simdgroup_functions = false;

            // If set, the subgroup size will be assumed to be one, and subgroup-related
            // builtins and operations will be emitted accordingly. This mode is intended to
            // be used by MoltenVK on hardware/software configurations which do not provide
            // sufficient support for subgroups.
            bool emulate_subgroups = false;

            // If nonzero, a fixed subgroup size to assume. Metal, similarly to VK_EXT_subgroup_size_control,
            // allows the SIMD-group size (aka thread execution width) to vary depending on
            // register usage and requirements. In certain circumstances--for example, a pipeline
            // in MoltenVK without VK_PIPELINE_SHADER_STAGE_CREATE_ALLOW_VARYING_SUBGROUP_SIZE_BIT_EXT--
            // this is undesirable. This fixes the value of the SubgroupSize builtin, instead of
            // mapping it to the Metal builtin [[thread_execution_width]]. If the thread
            // execution width is reduced, the extra invocations will appear to be inactive.
            // If zero, the SubgroupSize will be allowed to vary, and the builtin will be mapped
            // to the Metal [[thread_execution_width]] builtin.
            uint32_t fixed_subgroup_size = 0;

            enum class IndexType
            {
                None = 0,
                UInt16 = 1,
                UInt32 = 2
            };

            // The type of index in the index buffer, if present. For a compute shader, Metal
            // requires specifying the indexing at pipeline creation, rather than at draw time
            // as with graphics pipelines. This means we must create three different pipelines,
            // for no indexing, 16-bit indices, and 32-bit indices. Each requires different
            // handling for the gl_VertexIndex builtin. We may as well, then, create three
            // different shaders for these three scenarios.
            IndexType vertex_index_type = IndexType::None;

            // If set, a dummy [[sample_id]] input is added to a fragment shader if none is present.
            // This will force the shader to run at sample rate, assuming Metal does not optimize
            // the extra threads away.
            bool force_sample_rate_shading = false;

            // If set, gl_HelperInvocation will be set manually whenever a fragment is discarded.
            // Some Metal devices have a bug where simd_is_helper_thread() does not return true
            // after a fragment has been discarded. This is a workaround that is only expected to be needed
            // until the bug is fixed in Metal; it is provided as an option to allow disabling it when that occurs.
            bool manual_helper_invocation_updates = true;

            // If set, extra checks will be emitted in fragment shaders to prevent writes
            // from discarded fragments. Some Metal devices have a bug where writes to storage resources
            // from discarded fragment threads continue to occur, despite the fragment being
            // discarded. This is a workaround that is only expected to be needed until the
            // bug is fixed in Metal; it is provided as an option so it can be enabled
            // only when the bug is present.
            bool check_discarded_frag_stores = false;

            // If set, Lod operands to OpImageSample*DrefExplicitLod for 1D and 2D array images
            // will be implemented using a gradient instead of passing the level operand directly.
            // Some Metal devices have a bug where the level() argument to depth2d_array<T>::sample_compare()
            // in a fragment shader is biased by some unknown amount, possibly dependent on the
            // partial derivatives of the texture coordinates. This is a workaround that is only
            // expected to be needed until the bug is fixed in Metal; it is provided as an option
            // so it can be enabled only when the bug is present.
            bool sample_dref_lod_array_as_grad = false;

            bool is_ios() const
            {
                return platform == iOS;
            }

            bool is_macos() const
            {
                return platform == macOS;
            }

            bool use_quadgroup_operation() const
            {
                return is_ios() && !ios_use_simdgroup_functions;
            }

            void set_msl_version(uint32_t major, uint32_t minor = 0, uint32_t patch = 0)
            {
                msl_version = make_msl_version(major, minor, patch);
            }

            bool supports_msl_version(uint32_t major, uint32_t minor = 0, uint32_t patch = 0) const
            {
                return msl_version >= make_msl_version(major, minor, patch);
            }

            static uint32_t make_msl_version(uint32_t major, uint32_t minor = 0, uint32_t patch = 0)
            {
                return (major * 10000) + (minor * 100) + patch;
            }
        };

        const Options& get_msl_options() const
        {
            return msl_options;
        }

        void set_msl_options(const Options& opts)
        {
            msl_options = opts;
        }

        // Provide feedback to calling API to allow runtime to disable pipeline
        // rasterization if vertex shader requires rasterization to be disabled.
        bool get_is_rasterization_disabled() const
        {
            return is_rasterization_disabled && (get_entry_point().model == spv::ExecutionModelVertex ||
                get_entry_point().model == spv::ExecutionModelTessellationControl ||
                get_entry_point().model == spv::ExecutionModelTessellationEvaluation);
        }

        // Provide feedback to calling API to allow it to pass an auxiliary
        // swizzle buffer if the shader needs it.
        bool needs_swizzle_buffer() const
        {
            return used_swizzle_buffer;
        }

        // Provide feedback to calling API to allow it to pass a buffer
        // containing STORAGE_BUFFER buffer sizes to support OpArrayLength.
        bool needs_buffer_size_buffer() const
        {
            return !buffers_requiring_array_length.empty();
        }

        bool buffer_requires_array_length(VariableID id) const
        {
            return buffers_requiring_array_length.count(id) != 0;
        }

        // Provide feedback to calling API to allow it to pass a buffer
        // containing the view mask for the current multiview subpass.
        bool needs_view_mask_buffer() const
        {
            return msl_options.multiview && !msl_options.view_index_from_device_index;
        }

        // Provide feedback to calling API to allow it to pass a buffer
        // containing the dispatch base workgroup ID.
        bool needs_dispatch_base_buffer() const
        {
            return msl_options.dispatch_base && !msl_options.supports_msl_version(1, 2);
        }

        // Provide feedback to calling API to allow it to pass an output
        // buffer if the shader needs it.
        bool needs_output_buffer() const
        {
            return capture_output_to_buffer && stage_out_var_id != ID(0);
        }

        // Provide feedback to calling API to allow it to pass a patch output
        // buffer if the shader needs it.
        bool needs_patch_output_buffer() const
        {
            return capture_output_to_buffer && patch_stage_out_var_id != ID(0);
        }

        // Provide feedback to calling API to allow it to pass an input threadgroup
        // buffer if the shader needs it.
        bool needs_input_threadgroup_mem() const
        {
            return capture_output_to_buffer && stage_in_var_id != ID(0);
        }

        explicit CompilerMSL(std::vector<uint32_t> spirv);
        CompilerMSL(const uint32_t* ir, size_t word_count);
        explicit CompilerMSL(const ParsedIR& ir);
        explicit CompilerMSL(ParsedIR&& ir);

        // input is a shader interface variable description used to fix up shader input variables.
        // If shader inputs are provided, is_msl_shader_input_used() will return true after
        // calling ::compile() if the location were used by the MSL code.
        void add_msl_shader_input(const MSLShaderInterfaceVariable& input);

        // output is a shader interface variable description used to fix up shader output variables.
        // If shader outputs are provided, is_msl_shader_output_used() will return true after
        // calling ::compile() if the location were used by the MSL code.
        void add_msl_shader_output(const MSLShaderInterfaceVariable& output);

        // resource is a resource binding to indicate the MSL buffer,
        // texture or sampler index to use for a particular SPIR-V description set
        // and binding. If resource bindings are provided,
        // is_msl_resource_binding_used() will return true after calling ::compile() if
        // the set/binding combination was used by the MSL code.
        void add_msl_resource_binding(const MSLResourceBinding& resource);

        // desc_set and binding are the SPIR-V descriptor set and binding of a buffer resource
        // in this shader. index is the index within the dynamic offset buffer to use. This
        // function marks that resource as using a dynamic offset (VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC
        // or VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC). This function only has any effect if argument buffers
        // are enabled. If so, the buffer will have its address adjusted at the beginning of the shader with
        // an offset taken from the dynamic offset buffer.
        void add_dynamic_buffer(uint32_t desc_set, uint32_t binding, uint32_t index);

        // desc_set and binding are the SPIR-V descriptor set and binding of a buffer resource
        // in this shader. This function marks that resource as an inline uniform block
        // (VK_DESCRIPTOR_TYPE_INLINE_UNIFORM_BLOCK_EXT). This function only has any effect if argument buffers
        // are enabled. If so, the buffer block will be directly embedded into the argument
        // buffer, instead of being referenced indirectly via pointer.
        void add_inline_uniform_block(uint32_t desc_set, uint32_t binding);

        // When using MSL argument buffers, we can force "classic" MSL 1.0 binding schemes for certain descriptor sets.
        // This corresponds to VK_KHR_push_descriptor in Vulkan.
        void add_discrete_descriptor_set(uint32_t desc_set);

        // If an argument buffer is large enough, it may need to be in the device storage space rather than
        // constant. Opt-in to this behavior here on a per set basis.
        void set_argument_buffer_device_address_space(uint32_t desc_set, bool device_storage);

        // Query after compilation is done. This allows you to check if an input location was used by the shader.
        bool is_msl_shader_input_used(uint32_t location);

        // Query after compilation is done. This allows you to check if an output location were used by the shader.
        bool is_msl_shader_output_used(uint32_t location);

        // If not using add_msl_shader_input, it's possible
        // that certain builtin attributes need to be automatically assigned locations.
        // This is typical for tessellation builtin inputs such as tess levels, gl_Position, etc.
        // This returns k_unknown_location if the location was explicitly assigned with
        // add_msl_shader_input or the builtin is not used, otherwise returns N in [[attribute(N)]].
        uint32_t get_automatic_builtin_input_location(spv::BuiltIn builtin) const;

        // If not using add_msl_shader_output, it's possible
        // that certain builtin attributes need to be automatically assigned locations.
        // This is typical for tessellation builtin outputs such as tess levels, gl_Position, etc.
        // This returns k_unknown_location if the location were explicitly assigned with
        // add_msl_shader_output or the builtin were not used, otherwise returns N in [[attribute(N)]].
        uint32_t get_automatic_builtin_output_location(spv::BuiltIn builtin) const;

        // NOTE: Only resources which are remapped using add_msl_resource_binding will be reported here.
        // Constexpr samplers are always assumed to be emitted.
        // No specific MSLResourceBinding remapping is required for constexpr samplers as long as they are remapped
        // by remap_constexpr_sampler(_by_binding).
        bool is_msl_resource_binding_used(spv::ExecutionModel model, uint32_t set, uint32_t binding) const;

        // This must only be called after a successful call to CompilerMSL::compile().
        // For a variable resource ID obtained through reflection API, report the automatically assigned resource index.
        // If the descriptor set was part of an argument buffer, report the [[id(N)]],
        // or [[buffer/texture/sampler]] binding for other resources.
        // If the resource was a combined image sampler, report the image binding here,
        // use the _secondary version of this call to query the sampler half of the resource.
        // If no binding exists, uint32_t(-1) is returned.
        uint32_t get_automatic_msl_resource_binding(uint32_t id) const;

        // Same as get_automatic_msl_resource_binding, but should only be used for combined image samplers, in which case the
        // sampler's binding is returned instead. For any other resource type, -1 is returned.
        // Secondary bindings are also used for the auxillary image atomic buffer.
        uint32_t get_automatic_msl_resource_binding_secondary(uint32_t id) const;

        // Same as get_automatic_msl_resource_binding, but should only be used for combined image samplers for multiplanar images,
        // in which case the second plane's binding is returned instead. For any other resource type, -1 is returned.
        uint32_t get_automatic_msl_resource_binding_tertiary(uint32_t id) const;

        // Same as get_automatic_msl_resource_binding, but should only be used for combined image samplers for triplanar images,
        // in which case the third plane's binding is returned instead. For any other resource type, -1 is returned.
        uint32_t get_automatic_msl_resource_binding_quaternary(uint32_t id) const;

        // Compiles the SPIR-V code into Metal Shading Language.
        std::string compile() override;

        // Remap a sampler with ID to a constexpr sampler.
        // Older iOS targets must use constexpr samplers in certain cases (PCF),
        // so a static sampler must be used.
        // The sampler will not consume a binding, but be declared in the entry point as a constexpr sampler.
        // This can be used on both combined image/samplers (sampler2D) or standalone samplers.
        // The remapped sampler must not be an array of samplers.
        // Prefer remap_constexpr_sampler_by_binding unless you're also doing reflection anyways.
        void remap_constexpr_sampler(VariableID id, const MSLConstexprSampler& sampler);

        // Same as remap_constexpr_sampler, except you provide set/binding, rather than variable ID.
        // Remaps based on ID take priority over set/binding remaps.
        void remap_constexpr_sampler_by_binding(uint32_t desc_set, uint32_t binding, const MSLConstexprSampler& sampler);

        // If using CompilerMSL::Options::pad_fragment_output_components, override the number of components we expect
        // to use for a particular location. The default is 4 if number of components is not overridden.
        void set_fragment_output_components(uint32_t location, uint32_t components);

        void set_combined_sampler_suffix(const char* suffix);
        const char* get_combined_sampler_suffix() const;

    protected:
        // An enum of SPIR-V functions that are implemented in additional
        // source code that is added to the shader if necessary.
        enum SPVFuncImpl : uint8_t
        {
            SPVFuncImplNone,
            SPVFuncImplMod,
            SPVFuncImplRadians,
            SPVFuncImplDegrees,
            SPVFuncImplFindILsb,
            SPVFuncImplFindSMsb,
            SPVFuncImplFindUMsb,
            SPVFuncImplSSign,
            SPVFuncImplArrayCopyMultidimBase,
            // Unfortunately, we cannot use recursive templates in the MSL compiler properly,
            // so stamp out variants up to some arbitrary maximum.
            SPVFuncImplArrayCopy = SPVFuncImplArrayCopyMultidimBase + 1,
            SPVFuncImplArrayOfArrayCopy2Dim = SPVFuncImplArrayCopyMultidimBase + 2,
            SPVFuncImplArrayOfArrayCopy3Dim = SPVFuncImplArrayCopyMultidimBase + 3,
            SPVFuncImplArrayOfArrayCopy4Dim = SPVFuncImplArrayCopyMultidimBase + 4,
            SPVFuncImplArrayOfArrayCopy5Dim = SPVFuncImplArrayCopyMultidimBase + 5,
            SPVFuncImplArrayOfArrayCopy6Dim = SPVFuncImplArrayCopyMultidimBase + 6,
            SPVFuncImplTexelBufferCoords,
            SPVFuncImplImage2DAtomicCoords, // Emulate texture2D atomic operations
            SPVFuncImplFMul,
            SPVFuncImplFAdd,
            SPVFuncImplFSub,
            SPVFuncImplQuantizeToF16,
            SPVFuncImplCubemapTo2DArrayFace,
            SPVFuncImplUnsafeArray, // Allow Metal to use the array<T> template to make arrays a value type
            SPVFuncImplStorageMatrix, // Allow threadgroup construction of matrices
            SPVFuncImplInverse4x4,
            SPVFuncImplInverse3x3,
            SPVFuncImplInverse2x2,
            // It is very important that this come before *Swizzle and ChromaReconstruct*, to ensure it's
            // emitted before them.
            SPVFuncImplForwardArgs,
            // Likewise, this must come before *Swizzle.
            SPVFuncImplGetSwizzle,
            SPVFuncImplTextureSwizzle,
            SPVFuncImplGatherSwizzle,
            SPVFuncImplGatherCompareSwizzle,
            SPVFuncImplSubgroupBroadcast,
            SPVFuncImplSubgroupBroadcastFirst,
            SPVFuncImplSubgroupBallot,
            SPVFuncImplSubgroupBallotBitExtract,
            SPVFuncImplSubgroupBallotFindLSB,
            SPVFuncImplSubgroupBallotFindMSB,
            SPVFuncImplSubgroupBallotBitCount,
            SPVFuncImplSubgroupAllEqual,
            SPVFuncImplSubgroupShuffle,
            SPVFuncImplSubgroupShuffleXor,
            SPVFuncImplSubgroupShuffleUp,
            SPVFuncImplSubgroupShuffleDown,
            SPVFuncImplQuadBroadcast,
            SPVFuncImplQuadSwap,
            SPVFuncImplReflectScalar,
            SPVFuncImplRefractScalar,
            SPVFuncImplFaceForwardScalar,
            SPVFuncImplChromaReconstructNearest2Plane,
            SPVFuncImplChromaReconstructNearest3Plane,
            SPVFuncImplChromaReconstructLinear422CositedEven2Plane,
            SPVFuncImplChromaReconstructLinear422CositedEven3Plane,
            SPVFuncImplChromaReconstructLinear422Midpoint2Plane,
            SPVFuncImplChromaReconstructLinear422Midpoint3Plane,
            SPVFuncImplChromaReconstructLinear420XCositedEvenYCositedEven2Plane,
            SPVFuncImplChromaReconstructLinear420XCositedEvenYCositedEven3Plane,
            SPVFuncImplChromaReconstructLinear420XMidpointYCositedEven2Plane,
            SPVFuncImplChromaReconstructLinear420XMidpointYCositedEven3Plane,
            SPVFuncImplChromaReconstructLinear420XCositedEvenYMidpoint2Plane,
            SPVFuncImplChromaReconstructLinear420XCositedEvenYMidpoint3Plane,
            SPVFuncImplChromaReconstructLinear420XMidpointYMidpoint2Plane,
            SPVFuncImplChromaReconstructLinear420XMidpointYMidpoint3Plane,
            SPVFuncImplExpandITUFullRange,
            SPVFuncImplExpandITUNarrowRange,
            SPVFuncImplConvertYCbCrBT709,
            SPVFuncImplConvertYCbCrBT601,
            SPVFuncImplConvertYCbCrBT2020,
            SPVFuncImplDynamicImageSampler,
        };

        // If the underlying resource has been used for comparison then duplicate loads of that resource must be too
        // Use Metal's native frame-buffer fetch API for subpass inputs.
        void emit_texture_op(const Instruction& i, bool sparse) override;
        void emit_binary_ptr_op(uint32_t result_type, uint32_t result_id, uint32_t op0, uint32_t op1, const char* op);
        std::string to_ptr_expression(uint32_t id, bool register_expression_read = true);
        void emit_binary_unord_op(uint32_t result_type, uint32_t result_id, uint32_t op0, uint32_t op1, const char* op);
        void emit_instruction(const Instruction& instr) override;
        void emit_glsl_op(uint32_t result_type, uint32_t result_id, uint32_t op, const uint32_t* args,
            uint32_t count) override;
        void emit_spv_amd_shader_trinary_minmax_op(uint32_t result_type, uint32_t result_id, uint32_t op,
            const uint32_t* args, uint32_t count) override;
        void emit_header() override;
        void emit_function_prototype(SPIRFunction& func, const Bitset& return_flags) override;
        void emit_sampled_image_op(uint32_t result_type, uint32_t result_id, uint32_t image_id, uint32_t samp_id) override;
        void emit_subgroup_op(const Instruction& i) override;
        std::string to_texture_op(const Instruction& i, bool sparse, bool* forward,
            SmallVector<uint32_t>& inherited_expressions) override;
        void emit_fixup() override;
        std::string to_struct_member(const SPIRType& type, uint32_t member_type_id, uint32_t index,
            const std::string& qualifier = "");
        void emit_struct_member(const SPIRType& type, uint32_t member_type_id, uint32_t index,
            const std::string& qualifier = "", uint32_t base_offset = 0) override;
        void emit_struct_padding_target(const SPIRType& type) override;
        std::string type_to_glsl(const SPIRType& type, uint32_t id, bool member);
        std::string type_to_glsl(const SPIRType& type, uint32_t id = 0) override;
        void emit_block_hints(const SPIRBlock& block) override;

        // Allow Metal to use the array<T> template to make arrays a value type
        std::string type_to_array_glsl(const SPIRType& type) override;
        std::string constant_op_expression(const SPIRConstantOp& cop) override;

        // Threadgroup arrays can't have a wrapper type
        std::string variable_decl(const SPIRVariable& variable) override;

        bool variable_decl_is_remapped_storage(const SPIRVariable& variable, spv::StorageClass storage) const override;

        // GCC workaround of lambdas calling protected functions (for older GCC versions)
        std::string variable_decl(const SPIRType& type, const std::string& name, uint32_t id = 0) override;

        std::string image_type_glsl(const SPIRType& type, uint32_t id = 0) override;
        std::string sampler_type(const SPIRType& type, uint32_t id);
        std::string builtin_to_glsl(spv::BuiltIn builtin, spv::StorageClass storage) override;
        std::string to_func_call_arg(const SPIRFunction::Parameter& arg, uint32_t id) override;
        std::string to_name(uint32_t id, bool allow_alias = true) const override;
        std::string to_function_name(const TextureFunctionNameArguments& args) override;
        std::string to_function_args(const TextureFunctionArguments& args, bool* p_forward) override;
        std::string to_initializer_expression(const SPIRVariable& var) override;
        std::string to_zero_initialized_expression(uint32_t type_id) override;

        std::string unpack_expression_type(std::string expr_str, const SPIRType& type, uint32_t physical_type_id,
            bool is_packed, bool row_major) override;

        // Returns true for BuiltInSampleMask because gl_SampleMask[] is an array in SPIR-V, but [[sample_mask]] is a scalar in Metal.
        bool builtin_translates_to_nonarray(spv::BuiltIn builtin) const override;

        std::string bitcast_glsl_op(const SPIRType& result_type, const SPIRType& argument_type) override;
        bool emit_complex_bitcast(uint32_t result_id, uint32_t id, uint32_t op0) override;
        bool skip_argument(uint32_t id) const override;
        std::string to_member_reference(uint32_t base, const SPIRType& type, uint32_t index, bool ptr_chain_is_resolved) override;
        std::string to_qualifiers_glsl(uint32_t id) override;
        void replace_illegal_names() override;
        void declare_constant_arrays();

        void replace_illegal_entry_point_names();
        void sync_entry_point_aliases_and_names();

        static const std::unordered_set<std::string>& get_reserved_keyword_set();
        static const std::unordered_set<std::string>& get_illegal_func_names();

        // Constant arrays of non-primitive types (i.e. matrices) won't link properly into Metal libraries
        void declare_complex_constant_arrays();

        bool is_patch_block(const SPIRType& type);
        bool is_non_native_row_major_matrix(uint32_t id) override;
        bool member_is_non_native_row_major_matrix(const SPIRType& type, uint32_t index) override;
        std::string convert_row_major_matrix(std::string exp_str, const SPIRType& exp_type, uint32_t physical_type_id,
            bool is_packed, bool relaxed) override;

        bool is_tesc_shader() const;
        bool is_tese_shader() const;

        void preprocess_op_codes();
        void localize_global_variables();
        void extract_global_variables_from_functions();
        void mark_packable_structs();
        void mark_as_packable(SPIRType& type);
        void mark_as_workgroup_struct(SPIRType& type);

        std::unordered_map<uint32_t, std::set<uint32_t>> function_global_vars;
        void extract_global_variables_from_function(uint32_t func_id, std::set<uint32_t>& added_arg_ids,
            std::unordered_set<uint32_t>& global_var_ids,
            std::unordered_set<uint32_t>& processed_func_ids);
        uint32_t add_interface_block(spv::StorageClass storage, bool patch = false);
        uint32_t add_interface_block_pointer(uint32_t ib_var_id, spv::StorageClass storage);

        struct InterfaceBlockMeta
        {
            struct LocationMeta
            {
                uint32_t base_type_id = 0;
                uint32_t num_components = 0;
                bool flat = false;
                bool noperspective = false;
                bool centroid = false;
                bool sample = false;
            };
            std::unordered_map<uint32_t, LocationMeta> location_meta;
            bool strip_array = false;
            bool allow_local_declaration = false;
        };

        std::string to_tesc_invocation_id();
        void emit_local_masked_variable(const SPIRVariable& masked_var, bool strip_array);
        void add_variable_to_interface_block(spv::StorageClass storage, const std::string& ib_var_ref, SPIRType& ib_type,
            SPIRVariable& var, InterfaceBlockMeta& meta);
        void add_composite_variable_to_interface_block(spv::StorageClass storage, const std::string& ib_var_ref,
            SPIRType& ib_type, SPIRVariable& var, InterfaceBlockMeta& meta);
        void add_plain_variable_to_interface_block(spv::StorageClass storage, const std::string& ib_var_ref,
            SPIRType& ib_type, SPIRVariable& var, InterfaceBlockMeta& meta);
        bool add_component_variable_to_interface_block(spv::StorageClass storage, const std::string& ib_var_ref,
            SPIRVariable& var, const SPIRType& type,
            InterfaceBlockMeta& meta);
        void add_plain_member_variable_to_interface_block(spv::StorageClass storage,
            const std::string& ib_var_ref, SPIRType& ib_type,
            SPIRVariable& var, SPIRType& var_type,
            uint32_t mbr_idx, InterfaceBlockMeta& meta,
            const std::string& mbr_name_qual,
            const std::string& var_chain_qual,
            uint32_t& location, uint32_t& var_mbr_idx);
        void add_composite_member_variable_to_interface_block(spv::StorageClass storage,
            const std::string& ib_var_ref, SPIRType& ib_type,
            SPIRVariable& var, SPIRType& var_type,
            uint32_t mbr_idx, InterfaceBlockMeta& meta,
            const std::string& mbr_name_qual,
            const std::string& var_chain_qual,
            uint32_t& location, uint32_t& var_mbr_idx);
        void add_tess_level_input_to_interface_block(const std::string& ib_var_ref, SPIRType& ib_type, SPIRVariable& var);
        void add_tess_level_input(const std::string& base_ref, const std::string& mbr_name, SPIRVariable& var);

        void fix_up_interface_member_indices(spv::StorageClass storage, uint32_t ib_type_id);

        void mark_location_as_used_by_shader(uint32_t location, const SPIRType& type,
            spv::StorageClass storage, bool fallback = false);
        uint32_t ensure_correct_builtin_type(uint32_t type_id, spv::BuiltIn builtin);
        uint32_t ensure_correct_input_type(uint32_t type_id, uint32_t location, uint32_t component,
            uint32_t num_components, bool strip_array);

        void emit_custom_templates();
        void emit_custom_functions();
        void emit_resources();
        void emit_specialization_constants_and_structs();
        void emit_interface_block(uint32_t ib_var_id);
        bool maybe_emit_array_assignment(uint32_t id_lhs, uint32_t id_rhs);
        uint32_t get_resource_array_size(uint32_t id) const;

        void fix_up_shader_inputs_outputs();

        std::string func_type_decl(SPIRType& type);
        std::string entry_point_args_classic(bool append_comma);
        std::string entry_point_args_argument_buffer(bool append_comma);
        std::string entry_point_arg_stage_in();
        void entry_point_args_builtin(std::string& args);
        void entry_point_args_discrete_descriptors(std::string& args);
        std::string append_member_name(const std::string& qualifier, const SPIRType& type, uint32_t index);
        std::string ensure_valid_name(std::string name, std::string pfx);
        std::string to_sampler_expression(uint32_t id);
        std::string to_swizzle_expression(uint32_t id);
        std::string to_buffer_size_expression(uint32_t id);
        bool is_sample_rate() const;
        bool is_intersection_query() const;
        bool is_direct_input_builtin(spv::BuiltIn builtin);
        std::string builtin_qualifier(spv::BuiltIn builtin);
        std::string builtin_type_decl(spv::BuiltIn builtin, uint32_t id = 0);
        std::string built_in_func_arg(spv::BuiltIn builtin, bool prefix_comma);
        std::string member_attribute_qualifier(const SPIRType& type, uint32_t index);
        std::string member_location_attribute_qualifier(const SPIRType& type, uint32_t index);
        std::string argument_decl(const SPIRFunction::Parameter& arg);
        const char* descriptor_address_space(uint32_t id, spv::StorageClass storage, const char* plain_address_space) const;
        std::string round_fp_tex_coords(std::string tex_coords, bool coord_is_fp);
        uint32_t get_metal_resource_index(SPIRVariable& var, SPIRType::BaseType basetype, uint32_t plane = 0);
        uint32_t get_member_location(uint32_t type_id, uint32_t index, uint32_t* comp = nullptr) const;
        uint32_t get_or_allocate_builtin_input_member_location(spv::BuiltIn builtin,
            uint32_t type_id, uint32_t index, uint32_t* comp = nullptr);
        uint32_t get_or_allocate_builtin_output_member_location(spv::BuiltIn builtin,
            uint32_t type_id, uint32_t index, uint32_t* comp = nullptr);

        uint32_t get_physical_tess_level_array_size(spv::BuiltIn builtin) const;

        // MSL packing rules. These compute the effective packing rules as observed by the MSL compiler in the MSL output.
        // These values can change depending on various extended decorations which control packing rules.
        // We need to make these rules match up with SPIR-V declared rules.
        uint32_t get_declared_type_size_msl(const SPIRType& type, bool packed, bool row_major) const;
        uint32_t get_declared_type_array_stride_msl(const SPIRType& type, bool packed, bool row_major) const;
        uint32_t get_declared_type_matrix_stride_msl(const SPIRType& type, bool packed, bool row_major) const;
        uint32_t get_declared_type_alignment_msl(const SPIRType& type, bool packed, bool row_major) const;

        uint32_t get_declared_struct_member_size_msl(const SPIRType& struct_type, uint32_t index) const;
        uint32_t get_declared_struct_member_array_stride_msl(const SPIRType& struct_type, uint32_t index) const;
        uint32_t get_declared_struct_member_matrix_stride_msl(const SPIRType& struct_type, uint32_t index) const;
        uint32_t get_declared_struct_member_alignment_msl(const SPIRType& struct_type, uint32_t index) const;

        uint32_t get_declared_input_size_msl(const SPIRType& struct_type, uint32_t index) const;
        uint32_t get_declared_input_array_stride_msl(const SPIRType& struct_type, uint32_t index) const;
        uint32_t get_declared_input_matrix_stride_msl(const SPIRType& struct_type, uint32_t index) const;
        uint32_t get_declared_input_alignment_msl(const SPIRType& struct_type, uint32_t index) const;

        const SPIRType& get_physical_member_type(const SPIRType& struct_type, uint32_t index) const;
        SPIRType get_presumed_input_type(const SPIRType& struct_type, uint32_t index) const;

        uint32_t get_declared_struct_size_msl(const SPIRType& struct_type, bool ignore_alignment = false,
            bool ignore_padding = false) const;

        std::string to_component_argument(uint32_t id);
        void align_struct(SPIRType& ib_type, std::unordered_set<uint32_t>& aligned_structs);
        void mark_scalar_layout_structs(const SPIRType& ib_type);
        void mark_struct_members_packed(const SPIRType& type);
        void ensure_member_packing_rules_msl(SPIRType& ib_type, uint32_t index);
        bool validate_member_packing_rules_msl(const SPIRType& type, uint32_t index) const;
        std::string get_argument_address_space(const SPIRVariable& argument);
        std::string get_type_address_space(const SPIRType& type, uint32_t id, bool argument = false);
        const char* to_restrict(uint32_t id, bool space);
        SPIRType& get_stage_in_struct_type();
        SPIRType& get_stage_out_struct_type();
        SPIRType& get_patch_stage_in_struct_type();
        SPIRType& get_patch_stage_out_struct_type();
        std::string get_tess_factor_struct_name();
        SPIRType& get_uint_type();
        uint32_t get_uint_type_id();
        void emit_atomic_func_op(uint32_t result_type, uint32_t result_id, const char* op, spv::Op opcode,
            uint32_t mem_order_1, uint32_t mem_order_2, bool has_mem_order_2, uint32_t op0, uint32_t op1 = 0,
            bool op1_is_pointer = false, bool op1_is_literal = false, uint32_t op2 = 0);
        const char* get_memory_order(uint32_t spv_mem_sem);
        void add_pragma_line(const std::string& line);
        void add_typedef_line(const std::string& line);
        void emit_barrier(uint32_t id_exe_scope, uint32_t id_mem_scope, uint32_t id_mem_sem);
        bool emit_array_copy(const char* expr, uint32_t lhs_id, uint32_t rhs_id,
            spv::StorageClass lhs_storage, spv::StorageClass rhs_storage) override;
        void build_implicit_builtins();
        uint32_t build_constant_uint_array_pointer();
        void emit_entry_point_declarations() override;
        bool uses_explicit_early_fragment_test();

        uint32_t builtin_frag_coord_id = 0;
        uint32_t builtin_sample_id_id = 0;
        uint32_t builtin_sample_mask_id = 0;
        uint32_t builtin_helper_invocation_id = 0;
        uint32_t builtin_vertex_idx_id = 0;
        uint32_t builtin_base_vertex_id = 0;
        uint32_t builtin_instance_idx_id = 0;
        uint32_t builtin_base_instance_id = 0;
        uint32_t builtin_view_idx_id = 0;
        uint32_t builtin_layer_id = 0;
        uint32_t builtin_invocation_id_id = 0;
        uint32_t builtin_primitive_id_id = 0;
        uint32_t builtin_subgroup_invocation_id_id = 0;
        uint32_t builtin_subgroup_size_id = 0;
        uint32_t builtin_dispatch_base_id = 0;
        uint32_t builtin_stage_input_size_id = 0;
        uint32_t builtin_local_invocation_index_id = 0;
        uint32_t builtin_workgroup_size_id = 0;
        uint32_t swizzle_buffer_id = 0;
        uint32_t buffer_size_buffer_id = 0;
        uint32_t view_mask_buffer_id = 0;
        uint32_t dynamic_offsets_buffer_id = 0;
        uint32_t uint_type_id = 0;
        uint32_t argument_buffer_padding_buffer_type_id = 0;
        uint32_t argument_buffer_padding_image_type_id = 0;
        uint32_t argument_buffer_padding_sampler_type_id = 0;

        bool does_shader_write_sample_mask = false;
        bool frag_shader_needs_discard_checks = false;

        void cast_to_variable_store(uint32_t target_id, std::string& expr, const SPIRType& expr_type) override;
        void cast_from_variable_load(uint32_t source_id, std::string& expr, const SPIRType& expr_type) override;
        void emit_store_statement(uint32_t lhs_expression, uint32_t rhs_expression) override;

        void analyze_sampled_image_usage();

        bool access_chain_needs_stage_io_builtin_translation(uint32_t base) override;
        void prepare_access_chain_for_scalar_access(std::string& expr, const SPIRType& type, spv::StorageClass storage,
            bool& is_packed) override;
        void fix_up_interpolant_access_chain(const uint32_t* ops, uint32_t length);
        void check_physical_type_cast(std::string& expr, const SPIRType* type, uint32_t physical_type) override;

        bool emit_tessellation_access_chain(const uint32_t* ops, uint32_t length);
        bool emit_tessellation_io_load(uint32_t result_type, uint32_t id, uint32_t ptr);
        bool is_out_of_bounds_tessellation_level(uint32_t id_lhs);

        void ensure_builtin(spv::StorageClass storage, spv::BuiltIn builtin);

        void mark_implicit_builtin(spv::StorageClass storage, spv::BuiltIn builtin, uint32_t id);

        std::string convert_to_f32(const std::string& expr, uint32_t components);

        Options msl_options;
        std::set<SPVFuncImpl> spv_function_implementations;
        // Must be ordered to ensure declarations are in a specific order.
        std::map<LocationComponentPair, MSLShaderInterfaceVariable> inputs_by_location;
        std::unordered_map<uint32_t, MSLShaderInterfaceVariable> inputs_by_builtin;
        std::map<LocationComponentPair, MSLShaderInterfaceVariable> outputs_by_location;
        std::unordered_map<uint32_t, MSLShaderInterfaceVariable> outputs_by_builtin;
        std::unordered_set<uint32_t> location_inputs_in_use;
        std::unordered_set<uint32_t> location_inputs_in_use_fallback;
        std::unordered_set<uint32_t> location_outputs_in_use;
        std::unordered_set<uint32_t> location_outputs_in_use_fallback;
        std::unordered_map<uint32_t, uint32_t> fragment_output_components;
        std::unordered_map<uint32_t, uint32_t> builtin_to_automatic_input_location;
        std::unordered_map<uint32_t, uint32_t> builtin_to_automatic_output_location;
        std::set<std::string> pragma_lines;
        std::set<std::string> typedef_lines;
        SmallVector<uint32_t> vars_needing_early_declaration;

        std::unordered_map<StageSetBinding, std::pair<MSLResourceBinding, bool>, InternalHasher> resource_bindings;
        std::unordered_map<StageSetBinding, uint32_t, InternalHasher> resource_arg_buff_idx_to_binding_number;

        uint32_t next_metal_resource_index_buffer = 0;
        uint32_t next_metal_resource_index_texture = 0;
        uint32_t next_metal_resource_index_sampler = 0;
        // Intentionally uninitialized, works around MSVC 2013 bug.
        uint32_t next_metal_resource_ids[kMaxArgumentBuffers];

        VariableID stage_in_var_id = 0;
        VariableID stage_out_var_id = 0;
        VariableID patch_stage_in_var_id = 0;
        VariableID patch_stage_out_var_id = 0;
        VariableID stage_in_ptr_var_id = 0;
        VariableID stage_out_ptr_var_id = 0;
        VariableID tess_level_inner_var_id = 0;
        VariableID tess_level_outer_var_id = 0;
        VariableID stage_out_masked_builtin_type_id = 0;

        // Handle HLSL-style 0-based vertex/instance index.
        enum class TriState
        {
            Neutral,
            No,
            Yes
        };
        TriState needs_base_vertex_arg = TriState::Neutral;
        TriState needs_base_instance_arg = TriState::Neutral;

        bool has_sampled_images = false;
        bool builtin_declaration = false; // Handle HLSL-style 0-based vertex/instance index.

        bool is_using_builtin_array = false; // Force the use of C style array declaration.
        bool using_builtin_array() const;

        bool is_rasterization_disabled = false;
        bool capture_output_to_buffer = false;
        bool needs_swizzle_buffer_def = false;
        bool used_swizzle_buffer = false;
        bool added_builtin_tess_level = false;
        bool needs_subgroup_invocation_id = false;
        bool needs_subgroup_size = false;
        bool needs_sample_id = false;
        bool needs_helper_invocation = false;
        std::string qual_pos_var_name;
        std::string stage_in_var_name = "in";
        std::string stage_out_var_name = "out";
        std::string patch_stage_in_var_name = "patchIn";
        std::string patch_stage_out_var_name = "patchOut";
        std::string sampler_name_suffix = "Smplr";
        std::string swizzle_name_suffix = "Swzl";
        std::string buffer_size_name_suffix = "BufferSize";
        std::string plane_name_suffix = "Plane";
        std::string input_wg_var_name = "gl_in";
        std::string input_buffer_var_name = "spvIn";
        std::string output_buffer_var_name = "spvOut";
        std::string patch_input_buffer_var_name = "spvPatchIn";
        std::string patch_output_buffer_var_name = "spvPatchOut";
        std::string tess_factor_buffer_var_name = "spvTessLevel";
        std::string index_buffer_var_name = "spvIndices";
        spv::Op previous_instruction_opcode = spv::OpNop;

        // Must be ordered since declaration is in a specific order.
        std::map<uint32_t, MSLConstexprSampler> constexpr_samplers_by_id;
        std::unordered_map<SetBindingPair, MSLConstexprSampler, InternalHasher> constexpr_samplers_by_binding;
        const MSLConstexprSampler* find_constexpr_sampler(uint32_t id) const;

        std::unordered_set<uint32_t> buffers_requiring_array_length;
        SmallVector<uint32_t> buffer_arrays_discrete;
        SmallVector<std::pair<uint32_t, uint32_t>> buffer_aliases_argument;
        SmallVector<uint32_t> buffer_aliases_discrete;
        std::unordered_set<uint32_t> atomic_image_vars; // Emulate texture2D atomic operations
        std::unordered_set<uint32_t> pull_model_inputs;

        // Must be ordered since array is in a specific order.
        std::map<SetBindingPair, std::pair<uint32_t, uint32_t>> buffers_requiring_dynamic_offset;

        SmallVector<uint32_t> disabled_frag_outputs;

        std::unordered_set<SetBindingPair, InternalHasher> inline_uniform_blocks;

        uint32_t argument_buffer_ids[kMaxArgumentBuffers];
        uint32_t argument_buffer_discrete_mask = 0;
        uint32_t argument_buffer_device_storage_mask = 0;

        void analyze_argument_buffers();
        bool descriptor_set_is_argument_buffer(uint32_t desc_set) const;
        MSLResourceBinding& get_argument_buffer_resource(uint32_t desc_set, uint32_t arg_idx);
        void add_argument_buffer_padding_buffer_type(SPIRType& struct_type, uint32_t& mbr_idx, uint32_t& arg_buff_index, MSLResourceBinding& rez_bind);
        void add_argument_buffer_padding_image_type(SPIRType& struct_type, uint32_t& mbr_idx, uint32_t& arg_buff_index, MSLResourceBinding& rez_bind);
        void add_argument_buffer_padding_sampler_type(SPIRType& struct_type, uint32_t& mbr_idx, uint32_t& arg_buff_index, MSLResourceBinding& rez_bind);
        void add_argument_buffer_padding_type(uint32_t mbr_type_id, SPIRType& struct_type, uint32_t& mbr_idx, uint32_t& arg_buff_index, uint32_t count);

        uint32_t get_target_components_for_fragment_location(uint32_t location) const;
        uint32_t build_extended_vector_type(uint32_t type_id, uint32_t components,
            SPIRType::BaseType basetype = SPIRType::Unknown);
        uint32_t build_msl_interpolant_type(uint32_t type_id, bool is_noperspective);

        bool suppress_missing_prototypes = false;

        void add_spv_func_and_recompile(SPVFuncImpl spv_func);

        void activate_argument_buffer_resources();

        bool type_is_msl_framebuffer_fetch(const SPIRType& type) const;
        bool type_is_pointer(const SPIRType& type) const;
        bool type_is_pointer_to_pointer(const SPIRType& type) const;
        bool is_supported_argument_buffer_type(const SPIRType& type) const;

        bool variable_storage_requires_stage_io(spv::StorageClass storage) const;

        bool needs_manual_helper_invocation_updates() const
        {
            return msl_options.manual_helper_invocation_updates && msl_options.supports_msl_version(2, 3);
        }
        bool needs_frag_discard_checks() const
        {
            return get_execution_model() == spv::ExecutionModelFragment && msl_options.supports_msl_version(2, 3) &&
                msl_options.check_discarded_frag_stores && frag_shader_needs_discard_checks;
        }

        bool has_additional_fixed_sample_mask() const { return msl_options.additional_fixed_sample_mask != 0xffffffff; }
        std::string additional_fixed_sample_mask_str() const;

        // OpcodeHandler that handles several MSL preprocessing operations.
        struct OpCodePreprocessor : OpcodeHandler
        {
            OpCodePreprocessor(CompilerMSL& compiler_)
                : compiler(compiler_)
            {
            }

            bool handle(spv::Op opcode, const uint32_t* args, uint32_t length) override;
            CompilerMSL::SPVFuncImpl get_spv_func_impl(spv::Op opcode, const uint32_t* args);
            void check_resource_write(uint32_t var_id);

            CompilerMSL& compiler;
            std::unordered_map<uint32_t, uint32_t> result_types;
            std::unordered_map<uint32_t, uint32_t> image_pointers; // Emulate texture2D atomic operations
            bool suppress_missing_prototypes = false;
            bool uses_atomics = false;
            bool uses_image_write = false;
            bool uses_buffer_write = false;
            bool uses_discard = false;
            bool needs_subgroup_invocation_id = false;
            bool needs_subgroup_size = false;
            bool needs_sample_id = false;
            bool needs_helper_invocation = false;
        };

        // OpcodeHandler that scans for uses of sampled images
        struct SampledImageScanner : OpcodeHandler
        {
            SampledImageScanner(CompilerMSL& compiler_)
                : compiler(compiler_)
            {
            }

            bool handle(spv::Op opcode, const uint32_t* args, uint32_t) override;

            CompilerMSL& compiler;
        };

        // Sorts the members of a SPIRType and associated Meta info based on a settable sorting
        // aspect, which defines which aspect of the struct members will be used to sort them.
        // Regardless of the sorting aspect, built-in members always appear at the end of the struct.
        struct MemberSorter
        {
            enum SortAspect
            {
                LocationThenBuiltInType,
                Offset
            };

            void sort();
            bool operator()(uint32_t mbr_idx1, uint32_t mbr_idx2);
            MemberSorter(SPIRType& t, Meta& m, SortAspect sa);

            SPIRType& type;
            Meta& meta;
            SortAspect sort_aspect;
        };
    };
} // namespace SPIRV_CROSS_NAMESPACE

#endif


#include "backend/DriverEnums.h"
#include "sca/builtinResource.h"
#include "sca/GLSLTools.h"

#include "shaders/CodeGenerator.h"
#include "shaders/MaterialInfo.h"
#include "shaders/SibGenerator.h"

#include "MetalArgumentBuffer.h"
#include "SpirvFixup.h"
#include "utils/ostream.h"

#include <filament/MaterialEnums.h>

#include <utils/Log.h>

#include <sstream>
#include <unordered_map>
#include <vector>

using namespace glslang;
using namespace spirv_cross;
using namespace spvtools;
using namespace filament;
using namespace filament::backend;

namespace filamat {

using namespace utils;

namespace msl {  // this is only used for MSL

using BindingIndexMap = std::unordered_map<std::string, uint16_t>;

static void collectSibs(const GLSLPostProcessor::Config& config, SibVector& sibs) {
    switch (config.domain) {
        case MaterialDomain::SURFACE:
            UTILS_NOUNROLL
            for (uint8_t blockIndex = 0; blockIndex < CONFIG_SAMPLER_BINDING_COUNT; blockIndex++) {
                if (blockIndex == SamplerBindingPoints::PER_MATERIAL_INSTANCE) {
                    continue;
                }
                auto const* sib =
                        SibGenerator::getSib((SamplerBindingPoints)blockIndex, config.variant);
                if (sib && hasShaderType(sib->getStageFlags(), config.shaderType)) {
                    sibs.emplace_back(blockIndex, sib);
                }
            }
        case MaterialDomain::POST_PROCESS:
        case MaterialDomain::COMPUTE:
            break;
    }
    sibs.emplace_back((uint8_t) SamplerBindingPoints::PER_MATERIAL_INSTANCE,
            &config.materialInfo->sib);
}

} // namespace msl

GLSLPostProcessor::GLSLPostProcessor(MaterialBuilder::Optimization optimization, uint32_t flags)
        : mOptimization(optimization),
          mPrintShaders(flags & PRINT_SHADERS),
          mGenerateDebugInfo(flags & GENERATE_DEBUG_INFO) {
    // SPIRV error handler registration needs to occur only once. To avoid a race we do it up here
    // in the constructor, which gets invoked before MaterialBuilder kicks off jobs.
    spv::spirvbin_t::registerErrorHandler([](const std::string& str) {
        slog.e << str << io::endl;
    });
}

GLSLPostProcessor::~GLSLPostProcessor() = default;

static bool filterSpvOptimizerMessage(spv_message_level_t level) {
#ifdef NDEBUG
    // In release builds, only log errors.
    if (level == SPV_MSG_WARNING ||
        level == SPV_MSG_INFO ||
        level == SPV_MSG_DEBUG) {
        return false;
    }
#endif
    return true;
}

static std::string stringifySpvOptimizerMessage(spv_message_level_t level, const char* source,
        const spv_position_t& position, const char* message) {
    const char* levelString = nullptr;
    switch (level) {
        case SPV_MSG_FATAL:
            levelString = "FATAL";
            break;
        case SPV_MSG_INTERNAL_ERROR:
            levelString = "INTERNAL ERROR";
            break;
        case SPV_MSG_ERROR:
            levelString = "ERROR";
            break;
        case SPV_MSG_WARNING:
            levelString = "WARNING";
            break;
        case SPV_MSG_INFO:
            levelString = "INFO";
            break;
        case SPV_MSG_DEBUG:
            levelString = "DEBUG";
            break;
    }

    std::ostringstream oss;
    oss << levelString << ": ";
    if (source) oss << source << ":";
    oss << position.line << ":" << position.column << ":";
    oss << position.index << ": ";
    if (message) oss << message;

    return oss.str();
}

void GLSLPostProcessor::spirvToMsl(const SpirvBlob *spirv, std::string *outMsl,
        filament::backend::ShaderModel shaderModel, bool useFramebufferFetch, const SibVector& sibs,
        const ShaderMinifier* minifier) {

    using namespace msl;

    CompilerMSL mslCompiler(*spirv);
    CompilerGLSL::Options const options;
    mslCompiler.set_common_options(options);

    const CompilerMSL::Options::Platform platform =
        shaderModel == ShaderModel::MOBILE ?
            CompilerMSL::Options::Platform::iOS : CompilerMSL::Options::Platform::macOS;

    CompilerMSL::Options mslOptions = {};
    mslOptions.platform = platform,
    mslOptions.msl_version = shaderModel == ShaderModel::MOBILE ?
        CompilerMSL::Options::make_msl_version(2, 0) : CompilerMSL::Options::make_msl_version(2, 2);

    if (useFramebufferFetch) {
        mslOptions.use_framebuffer_fetch_subpasses = true;
        // On macOS, framebuffer fetch is only available starting with MSL 2.3. Filament will only
        // use framebuffer fetch materials on devices that support it.
        if (shaderModel == ShaderModel::DESKTOP) {
            mslOptions.msl_version = CompilerMSL::Options::make_msl_version(2, 3);
        }
    }

    mslOptions.argument_buffers = true;
    mslOptions.ios_support_base_vertex_instance = true;

    // We're using argument buffers for texture resources, however, we cannot rely on spirv-cross to
    // generate the argument buffer definitions.
    //
    // Consider a shader with 3 textures:
    // layout (set = 0, binding = 0) uniform sampler2D texture1;
    // layout (set = 0, binding = 1) uniform sampler2D texture2;
    // layout (set = 0, binding = 2) uniform sampler2D texture3;
    //
    // If only texture1 and texture2 are used in the material, then texture3 will be optimized away.
    // This results in an argument buffer like the following:
    // struct spvDescriptorSetBuffer0 {
    //     texture2d<float> texture1 [[id(0)]];
    //     sampler texture1Smplr [[id(1)]];
    //     texture2d<float> texture2 [[id(2)]];
    //     sampler texture2Smplr [[id(3)]];
    // };
    // Note that this happens even if "pad_argument_buffer_resources" and
    // "force_active_argument_buffer_resources" are true.
    //
    // This would be fine, except older Apple devices don't like it when the argument buffer in the
    // shader doesn't precisely match the one generated at runtime.
    //
    // So, we use the MetalArgumentBuffer class to replace spirv-cross' argument buffer definitions
    // with our own that contain all the textures/samples, even those optimized away.
    std::vector<MetalArgumentBuffer*> argumentBuffers;

    mslCompiler.set_msl_options(mslOptions);

    auto executionModel = mslCompiler.get_execution_model();

    // Metal Descriptor Sets
    // Descriptor set       Name                    Binding
    // ----------------------------------------------------------------------
    // 0                    Uniforms                Individual bindings
    // 1-4                  Sampler groups          [[buffer(27-30)]]
    // 5-7                  Unused
    //
    // Here we enumerate each sampler in each sampler group and map it to a Metal resource. Each
    // sampler group is its own descriptor set, and each descriptor set becomes an argument buffer.
    //
    // For example, in GLSL, we might have the following:
    // layout( set = 1, binding = 0 ) uniform sampler2D textureA;
    // layout( set = 1, binding = 1 ) uniform sampler2D textureB;
    //
    // This becomes the following MSL argument buffer:
    // struct spvDescriptorSetBuffer1 {
    //     texture2d<float> textureA [[id(0)]];
    //     sampler textureASmplr [[id(1)]];
    //     texture2d<float> textureB [[id(2)]];
    //     sampler textureBSmplr [[id(3)]];
    // };
    //
    // Which is then bound to the vertex/fragment functions:
    // constant spvDescriptorSetBuffer1& spvDescriptorSet1 [[buffer(27)]]
    for (auto [bindingPoint, sib] : sibs) {
        const auto& infoList = sib->getSamplerInfoList();

        // bindingPoint + 1, because the first descriptor set is for uniforms
        auto argBufferBuilder = MetalArgumentBuffer::Builder()
                .name("spvDescriptorSetBuffer" + std::to_string(int(bindingPoint + 1)));

        for (const auto& info: infoList) {
            const std::string name = info.uniformName.c_str();
            argBufferBuilder
                    .texture(info.offset * 2, name, info.type, info.format, info.multisample)
                    .sampler(info.offset * 2 + 1, name + "Smplr");
        }

        argumentBuffers.push_back(argBufferBuilder.build());

        // This MSLResourceBinding is how we control the [[buffer(n)]] binding of the argument
        // buffer itself;
        MSLResourceBinding argBufferBinding;
        // the baseType doesn't matter, but can't be UNKNOWN
        argBufferBinding.basetype = SPIRType::BaseType::Float;
        argBufferBinding.stage = executionModel;
        argBufferBinding.desc_set = bindingPoint + 1;
        argBufferBinding.binding = kArgumentBufferBinding;
        argBufferBinding.count = 1;
        argBufferBinding.msl_buffer =
                CodeGenerator::METAL_SAMPLER_GROUP_BINDING_START + bindingPoint;
        mslCompiler.add_msl_resource_binding(argBufferBinding);
    }

    auto updateResourceBindingDefault = [executionModel, &mslCompiler](const auto& resource) {
        auto set = mslCompiler.get_decoration(resource.id, spv::DecorationDescriptorSet);
        auto binding = mslCompiler.get_decoration(resource.id, spv::DecorationBinding);
        MSLResourceBinding newBinding;
        newBinding.basetype = SPIRType::BaseType::Void;
        newBinding.stage = executionModel;
        newBinding.desc_set = set;
        newBinding.binding = binding;
        newBinding.count = 1;
        newBinding.msl_texture =
        newBinding.msl_sampler =
        newBinding.msl_buffer = binding;
        mslCompiler.add_msl_resource_binding(newBinding);
    };

    auto uniformResources = mslCompiler.get_shader_resources();
    for (const auto& resource : uniformResources.uniform_buffers) {
        updateResourceBindingDefault(resource);
    }
    auto ssboResources = mslCompiler.get_shader_resources();
    for (const auto& resource : ssboResources.storage_buffers) {
        updateResourceBindingDefault(resource);
    }

    // Descriptor set 0 is uniforms. The add_discrete_descriptor_set call here prevents the uniforms
    // from becoming argument buffers.
    mslCompiler.add_discrete_descriptor_set(0);

    *outMsl = mslCompiler.compile();
    if (minifier) {
        *outMsl = minifier->removeWhitespace(*outMsl);
    }

    // Replace spirv-cross' generated argument buffers with our own.
    for (auto* argBuffer : argumentBuffers) {
        auto argBufferMsl = argBuffer->getMsl();
        MetalArgumentBuffer::replaceInShader(*outMsl, argBuffer->getName(), argBufferMsl);
        MetalArgumentBuffer::destroy(&argBuffer);
    }
}

bool GLSLPostProcessor::process(const std::string& inputShader, Config const& config,
        std::string* outputGlsl, SpirvBlob* outputSpirv, std::string* outputMsl) {
    using TargetLanguage = MaterialBuilder::TargetLanguage;

    if (config.targetLanguage == TargetLanguage::GLSL) {
        *outputGlsl = inputShader;
        if (mPrintShaders) {
            slog.i << *outputGlsl << io::endl;
        }
        return true;
    }

    InternalConfig internalConfig{
            .glslOutput = outputGlsl,
            .spirvOutput = outputSpirv,
            .mslOutput = outputMsl,
    };

    switch (config.shaderType) {
        case ShaderStage::VERTEX:
            internalConfig.shLang = EShLangVertex;
            break;
        case ShaderStage::FRAGMENT:
            internalConfig.shLang = EShLangFragment;
            break;
        case ShaderStage::COMPUTE:
            internalConfig.shLang = EShLangCompute;
            break;
    }

    TProgram program;
    TShader tShader(internalConfig.shLang);

    // The cleaner must be declared after the TShader to prevent ASAN failures.
    GLSLangCleaner const cleaner;

    const char* shaderCString = inputShader.c_str();
    tShader.setStrings(&shaderCString, 1);

    // This allows shaders to query if they will be run through glslang.
    // OpenGL shaders without optimization, for example, won't have this define.
    tShader.setPreamble("#define FILAMENT_GLSLANG\n");

    internalConfig.langVersion = GLSLTools::getGlslDefaultVersion(config.shaderModel);
    GLSLTools::prepareShaderParser(config.targetApi, config.targetLanguage, tShader,
            internalConfig.shLang, internalConfig.langVersion);

    EShMessages msg = GLSLTools::glslangFlagsFromTargetApi(config.targetApi, config.targetLanguage);
    if (config.hasFramebufferFetch) {
        // FIXME: subpasses require EShMsgVulkanRules, which I think is a mistake.
        //        SpvRules should be enough.
        //        I think this could cause the compilation to fail on gl_VertexID.
        using Type = std::underlying_type_t<EShMessages>;
        msg = EShMessages(Type(msg) | Type(EShMessages::EShMsgVulkanRules));
    }

    bool const ok = tShader.parse(&DefaultTBuiltInResource, internalConfig.langVersion, false, msg);
    if (!ok) {
        slog.e << tShader.getInfoLog() << io::endl;
        return false;
    }

    // add texture lod bias
    if (config.shaderType == backend::ShaderStage::FRAGMENT &&
        config.domain == MaterialDomain::SURFACE) {
        GLSLTools::textureLodBias(tShader);
    }

    program.addShader(&tShader);
    // Even though we only have a single shader stage, linking is still necessary to finalize
    // SPIR-V types
    bool const linkOk = program.link(msg);
    if (!linkOk) {
        slog.e << tShader.getInfoLog() << io::endl;
        return false;
    }

    switch (mOptimization) {
        case MaterialBuilder::Optimization::NONE:
            if (internalConfig.spirvOutput) {
                SpvOptions options;
                options.generateDebugInfo = mGenerateDebugInfo;
                GlslangToSpv(*program.getIntermediate(internalConfig.shLang),
                        *internalConfig.spirvOutput, &options);
                fixupClipDistance(*internalConfig.spirvOutput, config);
                if (internalConfig.mslOutput) {
                    auto sibs = SibVector::with_capacity(CONFIG_SAMPLER_BINDING_COUNT);
                    msl::collectSibs(config, sibs);
                    spirvToMsl(internalConfig.spirvOutput, internalConfig.mslOutput,
                            config.shaderModel, config.hasFramebufferFetch, sibs,
                            mGenerateDebugInfo ? &internalConfig.minifier : nullptr);
                }
            } else {
                slog.e << "GLSL post-processor invoked with optimization level NONE"
                        << io::endl;
            }
            break;
        case MaterialBuilder::Optimization::PREPROCESSOR:
            preprocessOptimization(tShader, config, internalConfig);
            break;
        case MaterialBuilder::Optimization::SIZE:
        case MaterialBuilder::Optimization::PERFORMANCE:
            if (!fullOptimization(tShader, config, internalConfig)) {
                return false;
            }
            break;
    }

    if (internalConfig.glslOutput) {
        if (!mGenerateDebugInfo) {
            *internalConfig.glslOutput =
                    internalConfig.minifier.removeWhitespace(
                            *internalConfig.glslOutput,
                            mOptimization == MaterialBuilder::Optimization::SIZE);

            // In theory this should only be enabled for SIZE, but in practice we often use PERFORMANCE.
            if (mOptimization != MaterialBuilder::Optimization::NONE) {
                *internalConfig.glslOutput =
                        internalConfig.minifier.renameStructFields(*internalConfig.glslOutput);
            }
        }
        if (mPrintShaders) {
            slog.i << *internalConfig.glslOutput << io::endl;
        }
    }
    return true;
}

void GLSLPostProcessor::preprocessOptimization(glslang::TShader& tShader,
        GLSLPostProcessor::Config const& config, InternalConfig& internalConfig) const {
    using TargetApi = MaterialBuilder::TargetApi;
    assert_invariant(bool(internalConfig.spirvOutput) == (config.targetApi != TargetApi::OPENGL));

    std::string glsl;
    TShader::ForbidIncluder forbidIncluder;

    const int version = GLSLTools::getGlslDefaultVersion(config.shaderModel);
    EShMessages const msg =
            GLSLTools::glslangFlagsFromTargetApi(config.targetApi, config.targetLanguage);
    bool ok = tShader.preprocess(&DefaultTBuiltInResource, version, ENoProfile, false, false,
            msg, &glsl, forbidIncluder);

    if (!ok) {
        slog.e << tShader.getInfoLog() << io::endl;
    }

    if (internalConfig.spirvOutput) {
        TProgram program;
        TShader spirvShader(internalConfig.shLang);

        // The cleaner must be declared after the TShader/TProgram which are setting the current
        // pool in the tls
        GLSLangCleaner const cleaner;

        const char* shaderCString = glsl.c_str();
        spirvShader.setStrings(&shaderCString, 1);
        GLSLTools::prepareShaderParser(config.targetApi, config.targetLanguage, spirvShader,
                internalConfig.shLang, internalConfig.langVersion);
        ok = spirvShader.parse(&DefaultTBuiltInResource, internalConfig.langVersion, false, msg);
        program.addShader(&spirvShader);
        // Even though we only have a single shader stage, linking is still necessary to finalize
        // SPIR-V types
        bool const linkOk = program.link(msg);
        if (!ok || !linkOk) {
            slog.e << spirvShader.getInfoLog() << io::endl;
        } else {
            SpvOptions options;
            options.generateDebugInfo = mGenerateDebugInfo;
            GlslangToSpv(*program.getIntermediate(internalConfig.shLang),
                    *internalConfig.spirvOutput, &options);
            fixupClipDistance(*internalConfig.spirvOutput, config);
        }
    }

    if (internalConfig.mslOutput) {
        auto sibs = SibVector::with_capacity(CONFIG_SAMPLER_BINDING_COUNT);
        msl::collectSibs(config, sibs);
        spirvToMsl(internalConfig.spirvOutput, internalConfig.mslOutput, config.shaderModel,
                config.hasFramebufferFetch, sibs,
                mGenerateDebugInfo ? &internalConfig.minifier : nullptr);
    }

    if (internalConfig.glslOutput) {
        *internalConfig.glslOutput = glsl;
    }
}

bool GLSLPostProcessor::fullOptimization(const TShader& tShader,
        GLSLPostProcessor::Config const& config, InternalConfig& internalConfig) const {
    SpirvBlob spirv;

    bool const optimizeForSize = mOptimization == MaterialBuilderBase::Optimization::SIZE;

    // Compile GLSL to to SPIR-V
    SpvOptions options;
    options.generateDebugInfo = mGenerateDebugInfo;
    GlslangToSpv(*tShader.getIntermediate(), spirv, &options);

    if (internalConfig.spirvOutput) {
        // Run the SPIR-V optimizer
        OptimizerPtr const optimizer = createOptimizer(mOptimization, config);
        optimizeSpirv(optimizer, spirv);
    } else {
        if (!optimizeForSize) {
            OptimizerPtr const optimizer = createOptimizer(mOptimization, config);
            optimizeSpirv(optimizer, spirv);
        }
    }

    fixupClipDistance(spirv, config);

    if (internalConfig.spirvOutput) {
        *internalConfig.spirvOutput = spirv;
    }

    if (internalConfig.mslOutput) {
        auto sibs = SibVector::with_capacity(CONFIG_SAMPLER_BINDING_COUNT);
        msl::collectSibs(config, sibs);
        spirvToMsl(&spirv, internalConfig.mslOutput, config.shaderModel, config.hasFramebufferFetch,
                sibs, mGenerateDebugInfo ? &internalConfig.minifier : nullptr);
    }

    // Transpile back to GLSL
    if (internalConfig.glslOutput) {
        CompilerGLSL::Options glslOptions;
        auto version = GLSLTools::getShadingLanguageVersion(
                config.shaderModel, config.featureLevel);
        glslOptions.es = version.second;
        glslOptions.version = version.first;
        glslOptions.enable_420pack_extension = glslOptions.version >= 420;
        glslOptions.fragment.default_float_precision = glslOptions.es ?
                CompilerGLSL::Options::Precision::Mediump : CompilerGLSL::Options::Precision::Highp;
        glslOptions.fragment.default_int_precision = glslOptions.es ?
                CompilerGLSL::Options::Precision::Mediump : CompilerGLSL::Options::Precision::Highp;

        // TODO: this should be done only on the "feature level 0" variant
        if (config.featureLevel == 0) {
            // convert UBOs to plain uniforms if we're at feature level 0
            glslOptions.emit_uniform_buffer_as_plain_uniforms = true;
        }

        if (config.variant.hasStereo() && config.shaderType == ShaderStage::VERTEX) {
            switch (config.materialInfo->stereoscopicType) {
            case StereoscopicType::INSTANCED:
                // Nothing to generate
                break;
            case StereoscopicType::MULTIVIEW:
                // For stereo variants using multiview feature, this generates the shader code below.
                //   #extension GL_OVR_multiview2 : require
                //   layout(num_views = 2) in;
                glslOptions.ovr_multiview_view_count = config.materialInfo->stereoscopicEyeCount;
                break;
            }
        }

        CompilerGLSL glslCompiler(std::move(spirv));
        glslCompiler.set_common_options(glslOptions);

        if (!glslOptions.es) {
            // enable GL_ARB_shading_language_packing if available
            glslCompiler.add_header_line("#extension GL_ARB_shading_language_packing : enable");
        }

        if (tShader.getStage() == EShLangFragment && glslOptions.es) {
            for (auto i : config.glsl.subpassInputToColorLocation) {
                glslCompiler.remap_ext_framebuffer_fetch(i.first, i.second, true);
            }
        }

#ifdef SPIRV_CROSS_EXCEPTIONS_TO_ASSERTIONS
        *internalConfig.glslOutput = glslCompiler.compile();
#else
        try {
            *internalConfig.glslOutput = glslCompiler.compile();
        } catch (spirv_cross::CompilerError e) {
            slog.e << "ERROR: " << e.what() << io::endl;
            return false;
        }
#endif

        // spirv-cross automatically redeclares gl_ClipDistance if it's used. Some drivers don't
        // like this, so we simply remove it.
        // According to EXT_clip_cull_distance, gl_ClipDistance can be
        // "implicitly sized by indexing it only with integral constant expressions".
        std::string& str = *internalConfig.glslOutput;
        const std::string clipDistanceDefinition = "out float gl_ClipDistance[2];";
        size_t const found = str.find(clipDistanceDefinition);
        if (found != std::string::npos) {
            str.replace(found, clipDistanceDefinition.length(), "");
        }
    }
    return true;
}

std::shared_ptr<spvtools::Optimizer> GLSLPostProcessor::createOptimizer(
        MaterialBuilder::Optimization optimization, Config const& config) {
    auto optimizer = std::make_shared<spvtools::Optimizer>(SPV_ENV_UNIVERSAL_1_3);

    optimizer->SetMessageConsumer([](spv_message_level_t level,
            const char* source, const spv_position_t& position, const char* message) {
        if (!filterSpvOptimizerMessage(level)) {
            return;
        }
        slog.e << stringifySpvOptimizerMessage(level, source, position, message)
                << io::endl;
    });

    if (optimization == MaterialBuilder::Optimization::SIZE) {
        registerSizePasses(*optimizer, config);
    } else if (optimization == MaterialBuilder::Optimization::PERFORMANCE) {
        registerPerformancePasses(*optimizer, config);
    }

    // Metal doesn't support relaxed precision, but does have support for float16 math operations.
    if (config.targetApi == MaterialBuilder::TargetApi::METAL) {
        optimizer->RegisterPass(CreateConvertRelaxedToHalfPass());
        optimizer->RegisterPass(CreateSimplificationPass());
        optimizer->RegisterPass(CreateRedundancyEliminationPass());
        optimizer->RegisterPass(CreateAggressiveDCEPass());
    }

    return optimizer;
}

void GLSLPostProcessor::optimizeSpirv(OptimizerPtr optimizer, SpirvBlob& spirv) const {
    if (!optimizer->Run(spirv.data(), spirv.size(), &spirv)) {
        slog.e << "SPIR-V optimizer pass failed" << io::endl;
        return;
    }

    // Remove dead module-level objects: functions, types, vars
    spv::spirvbin_t remapper(0);
    remapper.remap(spirv, spv::spirvbin_base_t::DCE_ALL);
}

void GLSLPostProcessor::fixupClipDistance(
        SpirvBlob& spirv, GLSLPostProcessor::Config const& config) const {
    if (!config.usesClipDistance) {
        return;
    }
    // This should match the version of SPIR-V used in GLSLTools::prepareShaderParser.
    SpirvTools const tools(SPV_ENV_UNIVERSAL_1_3);
    std::string disassembly;
    const bool result = tools.Disassemble(spirv, &disassembly);
    assert_invariant(result);
    if (filamat::fixupClipDistance(disassembly)) {
        spirv.clear();
        tools.Assemble(disassembly, &spirv);
        assert_invariant(tools.Validate(spirv));
    }
}

// CreateMergeReturnPass() causes these issues:
// - triggers a segfault with AMD OpenGL drivers on macOS
// - triggers a crash on some Adreno drivers (b/291140208, b/289401984, b/289393290)
// However Metal requires this pass in order to correctly generate half-precision MSL
//
// CreateSimplificationPass() creates a lot of problems:
// - Adreno GPU show artifacts after running simplification passes (Vulkan)
// - spirv-cross fails generating working glsl
//      (https://github.com/KhronosGroup/SPIRV-Cross/issues/2162)
// - generally it makes the code more complicated, e.g.: replacing for loops with
//   while-if-break, unclear if it helps for anything.
// However, the simplification passes below are necessary when targeting Metal, otherwise the
// result is mismatched half / float assignments in MSL.


void GLSLPostProcessor::registerPerformancePasses(Optimizer& optimizer, Config const& config) {
    auto RegisterPass = [&](spvtools::Optimizer::PassToken&& pass,
            MaterialBuilder::TargetApi apiFilter = MaterialBuilder::TargetApi::ALL) {
        if (!(config.targetApi & apiFilter)) {
            return;
        }
        optimizer.RegisterPass(std::move(pass));
    };

    RegisterPass(CreateWrapOpKillPass());
    RegisterPass(CreateDeadBranchElimPass());
    RegisterPass(CreateMergeReturnPass(), MaterialBuilder::TargetApi::METAL);
    RegisterPass(CreateInlineExhaustivePass());
    RegisterPass(CreateAggressiveDCEPass());
    RegisterPass(CreatePrivateToLocalPass());
    RegisterPass(CreateLocalSingleBlockLoadStoreElimPass());
    RegisterPass(CreateLocalSingleStoreElimPass());
    RegisterPass(CreateAggressiveDCEPass());
    RegisterPass(CreateScalarReplacementPass());
    RegisterPass(CreateLocalAccessChainConvertPass());
    RegisterPass(CreateLocalSingleBlockLoadStoreElimPass());
    RegisterPass(CreateLocalSingleStoreElimPass());
    RegisterPass(CreateAggressiveDCEPass());
    RegisterPass(CreateLocalMultiStoreElimPass());
    RegisterPass(CreateAggressiveDCEPass());
    RegisterPass(CreateCCPPass());
    RegisterPass(CreateAggressiveDCEPass());
    RegisterPass(CreateRedundancyEliminationPass());
    RegisterPass(CreateCombineAccessChainsPass());
    RegisterPass(CreateSimplificationPass(), MaterialBuilder::TargetApi::METAL);
    RegisterPass(CreateVectorDCEPass());
    RegisterPass(CreateDeadInsertElimPass());
    RegisterPass(CreateDeadBranchElimPass());
    RegisterPass(CreateSimplificationPass(), MaterialBuilder::TargetApi::METAL);
    RegisterPass(CreateIfConversionPass());
    RegisterPass(CreateCopyPropagateArraysPass());
    RegisterPass(CreateReduceLoadSizePass());
    RegisterPass(CreateAggressiveDCEPass());
    RegisterPass(CreateBlockMergePass());
    RegisterPass(CreateRedundancyEliminationPass());
    RegisterPass(CreateDeadBranchElimPass());
    RegisterPass(CreateBlockMergePass());
    RegisterPass(CreateSimplificationPass(), MaterialBuilder::TargetApi::METAL);
}

void GLSLPostProcessor::registerSizePasses(Optimizer& optimizer, Config const& config) {
    auto RegisterPass = [&](spvtools::Optimizer::PassToken&& pass,
            MaterialBuilder::TargetApi apiFilter = MaterialBuilder::TargetApi::ALL) {
        if (!(config.targetApi & apiFilter)) {
            return;
        }
        optimizer.RegisterPass(std::move(pass));
    };

    RegisterPass(CreateWrapOpKillPass());
    RegisterPass(CreateDeadBranchElimPass());
    RegisterPass(CreateMergeReturnPass(), MaterialBuilder::TargetApi::METAL);
    RegisterPass(CreateInlineExhaustivePass());
    RegisterPass(CreateEliminateDeadFunctionsPass());
    RegisterPass(CreatePrivateToLocalPass());
    RegisterPass(CreateScalarReplacementPass(0));
    RegisterPass(CreateLocalMultiStoreElimPass());
    RegisterPass(CreateCCPPass());
    RegisterPass(CreateLoopUnrollPass(true));
    RegisterPass(CreateDeadBranchElimPass());
    RegisterPass(CreateSimplificationPass(), MaterialBuilder::TargetApi::METAL);
    RegisterPass(CreateScalarReplacementPass(0));
    RegisterPass(CreateLocalSingleStoreElimPass());
    RegisterPass(CreateIfConversionPass());
    RegisterPass(CreateSimplificationPass(), MaterialBuilder::TargetApi::METAL);
    RegisterPass(CreateAggressiveDCEPass());
    RegisterPass(CreateDeadBranchElimPass());
    RegisterPass(CreateBlockMergePass());
    RegisterPass(CreateLocalAccessChainConvertPass());
    RegisterPass(CreateLocalSingleBlockLoadStoreElimPass());
    RegisterPass(CreateAggressiveDCEPass());
    RegisterPass(CreateCopyPropagateArraysPass());
    RegisterPass(CreateVectorDCEPass());
    RegisterPass(CreateDeadInsertElimPass());
    // this breaks UBO layout
    //RegisterPass(CreateEliminateDeadMembersPass());
    RegisterPass(CreateLocalSingleStoreElimPass());
    RegisterPass(CreateBlockMergePass());
    RegisterPass(CreateLocalMultiStoreElimPass());
    RegisterPass(CreateRedundancyEliminationPass());
    RegisterPass(CreateSimplificationPass(), MaterialBuilder::TargetApi::METAL);
    RegisterPass(CreateAggressiveDCEPass());
    RegisterPass(CreateCFGCleanupPass());
}

} // namespace filamat
