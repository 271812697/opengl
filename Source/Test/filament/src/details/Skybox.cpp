/*
 * Copyright (C) 2016 The Android Open Source Project
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

#include "details/Skybox.h"

#include "details/Engine.h"
#include "details/Texture.h"
#include "details/VertexBuffer.h"
#include "details/IndexBuffer.h"
#include "details/IndirectLight.h"
#include "details/Material.h"
#include "details/MaterialInstance.h"

#include "FilamentAPI-impl.h"

#include <filament/TextureSampler.h>

#include <backend/DriverEnums.h>

#include <utils/Panic.h>
#include <filament/Skybox.h>


#ifndef MATERIALS_H_
#define MATERIALS_H_

#include <stdint.h>

extern "C" {
    extern const uint8_t MATERIALS_PACKAGE[];
    extern int MATERIALS_FXAA_OFFSET;
    extern int MATERIALS_FXAA_SIZE;
    extern int MATERIALS_TAA_OFFSET;
    extern int MATERIALS_TAA_SIZE;
    extern int MATERIALS_BLITDEPTH_OFFSET;
    extern int MATERIALS_BLITDEPTH_SIZE;
    extern int MATERIALS_BLITLOW_OFFSET;
    extern int MATERIALS_BLITLOW_SIZE;
    extern int MATERIALS_BLITARRAY_OFFSET;
    extern int MATERIALS_BLITARRAY_SIZE;
    extern int MATERIALS_BLOOMDOWNSAMPLE_OFFSET;
    extern int MATERIALS_BLOOMDOWNSAMPLE_SIZE;
    extern int MATERIALS_BLOOMDOWNSAMPLE2X_OFFSET;
    extern int MATERIALS_BLOOMDOWNSAMPLE2X_SIZE;
    extern int MATERIALS_BLOOMDOWNSAMPLE9_OFFSET;
    extern int MATERIALS_BLOOMDOWNSAMPLE9_SIZE;
    extern int MATERIALS_BLOOMUPSAMPLE_OFFSET;
    extern int MATERIALS_BLOOMUPSAMPLE_SIZE;
    extern int MATERIALS_COLORGRADING_OFFSET;
    extern int MATERIALS_COLORGRADING_SIZE;
    extern int MATERIALS_COLORGRADINGASSUBPASS_OFFSET;
    extern int MATERIALS_COLORGRADINGASSUBPASS_SIZE;
    extern int MATERIALS_CUSTOMRESOLVEASSUBPASS_OFFSET;
    extern int MATERIALS_CUSTOMRESOLVEASSUBPASS_SIZE;
    extern int MATERIALS_DEBUGSHADOWCASCADES_OFFSET;
    extern int MATERIALS_DEBUGSHADOWCASCADES_SIZE;
    extern int MATERIALS_DEFAULTMATERIAL_OFFSET;
    extern int MATERIALS_DEFAULTMATERIAL_SIZE;
    extern int MATERIALS_DEFAULTMATERIAL_FL0_OFFSET;
    extern int MATERIALS_DEFAULTMATERIAL_FL0_SIZE;
    extern int MATERIALS_DOF_OFFSET;
    extern int MATERIALS_DOF_SIZE;
    extern int MATERIALS_DOFCOC_OFFSET;
    extern int MATERIALS_DOFCOC_SIZE;
    extern int MATERIALS_DOFCOMBINE_OFFSET;
    extern int MATERIALS_DOFCOMBINE_SIZE;
    extern int MATERIALS_DOFDILATE_OFFSET;
    extern int MATERIALS_DOFDILATE_SIZE;
    extern int MATERIALS_DOFDOWNSAMPLE_OFFSET;
    extern int MATERIALS_DOFDOWNSAMPLE_SIZE;
    extern int MATERIALS_DOFMEDIAN_OFFSET;
    extern int MATERIALS_DOFMEDIAN_SIZE;
    extern int MATERIALS_DOFMIPMAP_OFFSET;
    extern int MATERIALS_DOFMIPMAP_SIZE;
    extern int MATERIALS_DOFTILES_OFFSET;
    extern int MATERIALS_DOFTILES_SIZE;
    extern int MATERIALS_DOFTILESSWIZZLE_OFFSET;
    extern int MATERIALS_DOFTILESSWIZZLE_SIZE;
    extern int MATERIALS_FLARE_OFFSET;
    extern int MATERIALS_FLARE_SIZE;
    extern int MATERIALS_FSR_EASU_OFFSET;
    extern int MATERIALS_FSR_EASU_SIZE;
    extern int MATERIALS_FSR_EASU_MOBILE_OFFSET;
    extern int MATERIALS_FSR_EASU_MOBILE_SIZE;
    extern int MATERIALS_FSR_EASU_MOBILEF_OFFSET;
    extern int MATERIALS_FSR_EASU_MOBILEF_SIZE;
    extern int MATERIALS_FSR_RCAS_OFFSET;
    extern int MATERIALS_FSR_RCAS_SIZE;
    extern int MATERIALS_RESOLVEDEPTH_OFFSET;
    extern int MATERIALS_RESOLVEDEPTH_SIZE;
    extern int MATERIALS_SEPARABLEGAUSSIANBLUR_OFFSET;
    extern int MATERIALS_SEPARABLEGAUSSIANBLUR_SIZE;
    extern int MATERIALS_SKYBOX_OFFSET;
    extern int MATERIALS_SKYBOX_SIZE;
    extern int MATERIALS_SKYBOX_FL0_OFFSET;
    extern int MATERIALS_SKYBOX_FL0_SIZE;
    extern int MATERIALS_SHADOWMAP_OFFSET;
    extern int MATERIALS_SHADOWMAP_SIZE;
    extern int MATERIALS_BILATERALBLUR_OFFSET;
    extern int MATERIALS_BILATERALBLUR_SIZE;
    extern int MATERIALS_BILATERALBLURBENTNORMALS_OFFSET;
    extern int MATERIALS_BILATERALBLURBENTNORMALS_SIZE;
    extern int MATERIALS_MIPMAPDEPTH_OFFSET;
    extern int MATERIALS_MIPMAPDEPTH_SIZE;
    extern int MATERIALS_SAO_OFFSET;
    extern int MATERIALS_SAO_SIZE;
    extern int MATERIALS_SAOBENTNORMALS_OFFSET;
    extern int MATERIALS_SAOBENTNORMALS_SIZE;
    extern int MATERIALS_VSMMIPMAP_OFFSET;
    extern int MATERIALS_VSMMIPMAP_SIZE;
}
#define MATERIALS_FXAA_DATA (MATERIALS_PACKAGE + MATERIALS_FXAA_OFFSET)
#define MATERIALS_TAA_DATA (MATERIALS_PACKAGE + MATERIALS_TAA_OFFSET)
#define MATERIALS_BLITDEPTH_DATA (MATERIALS_PACKAGE + MATERIALS_BLITDEPTH_OFFSET)
#define MATERIALS_BLITLOW_DATA (MATERIALS_PACKAGE + MATERIALS_BLITLOW_OFFSET)
#define MATERIALS_BLITARRAY_DATA (MATERIALS_PACKAGE + MATERIALS_BLITARRAY_OFFSET)
#define MATERIALS_BLOOMDOWNSAMPLE_DATA (MATERIALS_PACKAGE + MATERIALS_BLOOMDOWNSAMPLE_OFFSET)
#define MATERIALS_BLOOMDOWNSAMPLE2X_DATA (MATERIALS_PACKAGE + MATERIALS_BLOOMDOWNSAMPLE2X_OFFSET)
#define MATERIALS_BLOOMDOWNSAMPLE9_DATA (MATERIALS_PACKAGE + MATERIALS_BLOOMDOWNSAMPLE9_OFFSET)
#define MATERIALS_BLOOMUPSAMPLE_DATA (MATERIALS_PACKAGE + MATERIALS_BLOOMUPSAMPLE_OFFSET)
#define MATERIALS_COLORGRADING_DATA (MATERIALS_PACKAGE + MATERIALS_COLORGRADING_OFFSET)
#define MATERIALS_COLORGRADINGASSUBPASS_DATA (MATERIALS_PACKAGE + MATERIALS_COLORGRADINGASSUBPASS_OFFSET)
#define MATERIALS_CUSTOMRESOLVEASSUBPASS_DATA (MATERIALS_PACKAGE + MATERIALS_CUSTOMRESOLVEASSUBPASS_OFFSET)
#define MATERIALS_DEBUGSHADOWCASCADES_DATA (MATERIALS_PACKAGE + MATERIALS_DEBUGSHADOWCASCADES_OFFSET)
#define MATERIALS_DEFAULTMATERIAL_DATA (MATERIALS_PACKAGE + MATERIALS_DEFAULTMATERIAL_OFFSET)
#define MATERIALS_DEFAULTMATERIAL_FL0_DATA (MATERIALS_PACKAGE + MATERIALS_DEFAULTMATERIAL_FL0_OFFSET)
#define MATERIALS_DOF_DATA (MATERIALS_PACKAGE + MATERIALS_DOF_OFFSET)
#define MATERIALS_DOFCOC_DATA (MATERIALS_PACKAGE + MATERIALS_DOFCOC_OFFSET)
#define MATERIALS_DOFCOMBINE_DATA (MATERIALS_PACKAGE + MATERIALS_DOFCOMBINE_OFFSET)
#define MATERIALS_DOFDILATE_DATA (MATERIALS_PACKAGE + MATERIALS_DOFDILATE_OFFSET)
#define MATERIALS_DOFDOWNSAMPLE_DATA (MATERIALS_PACKAGE + MATERIALS_DOFDOWNSAMPLE_OFFSET)
#define MATERIALS_DOFMEDIAN_DATA (MATERIALS_PACKAGE + MATERIALS_DOFMEDIAN_OFFSET)
#define MATERIALS_DOFMIPMAP_DATA (MATERIALS_PACKAGE + MATERIALS_DOFMIPMAP_OFFSET)
#define MATERIALS_DOFTILES_DATA (MATERIALS_PACKAGE + MATERIALS_DOFTILES_OFFSET)
#define MATERIALS_DOFTILESSWIZZLE_DATA (MATERIALS_PACKAGE + MATERIALS_DOFTILESSWIZZLE_OFFSET)
#define MATERIALS_FLARE_DATA (MATERIALS_PACKAGE + MATERIALS_FLARE_OFFSET)
#define MATERIALS_FSR_EASU_DATA (MATERIALS_PACKAGE + MATERIALS_FSR_EASU_OFFSET)
#define MATERIALS_FSR_EASU_MOBILE_DATA (MATERIALS_PACKAGE + MATERIALS_FSR_EASU_MOBILE_OFFSET)
#define MATERIALS_FSR_EASU_MOBILEF_DATA (MATERIALS_PACKAGE + MATERIALS_FSR_EASU_MOBILEF_OFFSET)
#define MATERIALS_FSR_RCAS_DATA (MATERIALS_PACKAGE + MATERIALS_FSR_RCAS_OFFSET)
#define MATERIALS_RESOLVEDEPTH_DATA (MATERIALS_PACKAGE + MATERIALS_RESOLVEDEPTH_OFFSET)
#define MATERIALS_SEPARABLEGAUSSIANBLUR_DATA (MATERIALS_PACKAGE + MATERIALS_SEPARABLEGAUSSIANBLUR_OFFSET)
#define MATERIALS_SKYBOX_DATA (MATERIALS_PACKAGE + MATERIALS_SKYBOX_OFFSET)
#define MATERIALS_SKYBOX_FL0_DATA (MATERIALS_PACKAGE + MATERIALS_SKYBOX_FL0_OFFSET)
#define MATERIALS_SHADOWMAP_DATA (MATERIALS_PACKAGE + MATERIALS_SHADOWMAP_OFFSET)
#define MATERIALS_BILATERALBLUR_DATA (MATERIALS_PACKAGE + MATERIALS_BILATERALBLUR_OFFSET)
#define MATERIALS_BILATERALBLURBENTNORMALS_DATA (MATERIALS_PACKAGE + MATERIALS_BILATERALBLURBENTNORMALS_OFFSET)
#define MATERIALS_MIPMAPDEPTH_DATA (MATERIALS_PACKAGE + MATERIALS_MIPMAPDEPTH_OFFSET)
#define MATERIALS_SAO_DATA (MATERIALS_PACKAGE + MATERIALS_SAO_OFFSET)
#define MATERIALS_SAOBENTNORMALS_DATA (MATERIALS_PACKAGE + MATERIALS_SAOBENTNORMALS_OFFSET)
#define MATERIALS_VSMMIPMAP_DATA (MATERIALS_PACKAGE + MATERIALS_VSMMIPMAP_OFFSET)

#endif


using namespace filament::math;
namespace filament {

struct Skybox::BuilderDetails {
    Texture* mEnvironmentMap = nullptr;
    float4 mColor{0, 0, 0, 1};
    float mIntensity = FIndirectLight::DEFAULT_INTENSITY;
    bool mShowSun = false;
};

using BuilderType = Skybox;
BuilderType::Builder::Builder() noexcept = default;
BuilderType::Builder::~Builder() noexcept = default;
BuilderType::Builder::Builder(BuilderType::Builder const& rhs) noexcept = default;
BuilderType::Builder::Builder(BuilderType::Builder&& rhs) noexcept = default;
BuilderType::Builder& BuilderType::Builder::operator=(BuilderType::Builder const& rhs) noexcept = default;
BuilderType::Builder& BuilderType::Builder::operator=(BuilderType::Builder&& rhs) noexcept = default;


Skybox::Builder& Skybox::Builder::environment(Texture* cubemap) noexcept {
    mImpl->mEnvironmentMap = cubemap;
    return *this;
}

Skybox::Builder& Skybox::Builder::intensity(float envIntensity) noexcept {
    mImpl->mIntensity = envIntensity;
    return *this;
}

Skybox::Builder& Skybox::Builder::color(math::float4 color) noexcept {
    mImpl->mColor = color;
    return *this;
}

Skybox::Builder& Skybox::Builder::showSun(bool show) noexcept {
    mImpl->mShowSun = show;
    return *this;
}

Skybox* Skybox::Builder::build(Engine& engine) {
    FTexture* cubemap = downcast(mImpl->mEnvironmentMap);

    ASSERT_PRECONDITION(!cubemap || cubemap->isCubemap(),
            "environment maps must be a cubemap");

    return downcast(engine).createSkybox(*this);
}

// ------------------------------------------------------------------------------------------------

FSkybox::FSkybox(FEngine& engine, const Builder& builder) noexcept
        : mSkyboxTexture(downcast(builder->mEnvironmentMap)),
          mRenderableManager(engine.getRenderableManager()),
          mIntensity(builder->mIntensity) {

    FMaterial const* material = engine.getSkyboxMaterial();
    mSkyboxMaterialInstance = material->createInstance("Skybox");

    TextureSampler const sampler(TextureSampler::MagFilter::LINEAR, TextureSampler::WrapMode::REPEAT);
    auto *pInstance = static_cast<MaterialInstance*>(mSkyboxMaterialInstance);
    FTexture const* texture = mSkyboxTexture ? mSkyboxTexture : engine.getDummyCubemap();
    pInstance->setParameter("skybox", texture, sampler);
    pInstance->setParameter("showSun", builder->mShowSun);
    pInstance->setParameter("constantColor", mSkyboxTexture == nullptr);
    pInstance->setParameter("color", builder->mColor);

    mSkybox = engine.getEntityManager().create();

    RenderableManager::Builder(1)
            .geometry(0, RenderableManager::PrimitiveType::TRIANGLES,
                    engine.getFullScreenVertexBuffer(),
                    engine.getFullScreenIndexBuffer())
            .material(0, mSkyboxMaterialInstance)
            .castShadows(false)
            .receiveShadows(false)
            .priority(0x7)
            .culling(false)
            .build(engine, mSkybox);
}

FMaterial const* FSkybox::createMaterial(FEngine& engine) {
    Material::Builder builder;
#ifdef FILAMENT_ENABLE_FEATURE_LEVEL_0
    if (UTILS_UNLIKELY(engine.getActiveFeatureLevel() == Engine::FeatureLevel::FEATURE_LEVEL_0)) {
        builder.package(MATERIALS_SKYBOX_FL0_DATA, MATERIALS_SKYBOX_FL0_SIZE);
    } else
#endif
    {
        switch (engine.getConfig().stereoscopicType) {
            case Engine::StereoscopicType::INSTANCED:
                builder.package(MATERIALS_SKYBOX_DATA, MATERIALS_SKYBOX_SIZE);
                break;
            case Engine::StereoscopicType::MULTIVIEW:
#ifdef FILAMENT_ENABLE_MULTIVIEW
                builder.package(MATERIALS_SKYBOX_MULTIVIEW_DATA, MATERIALS_SKYBOX_MULTIVIEW_SIZE);
#else
                assert_invariant(false);
#endif
                break;
        }
    }
    auto material = builder.build(engine);
    return downcast(material);
}

void FSkybox::terminate(FEngine& engine) noexcept {
    // use Engine::destroy because FEngine::destroy is inlined
    Engine& e = engine;
    e.destroy(mSkyboxMaterialInstance);
    e.destroy(mSkybox);

    engine.getEntityManager().destroy(mSkybox);

    mSkyboxMaterialInstance = nullptr;
    mSkybox = {};
}

void FSkybox::setLayerMask(uint8_t select, uint8_t values) noexcept {
    auto& rcm = mRenderableManager;
    rcm.setLayerMask(rcm.getInstance(mSkybox), select, values);
    // we keep a checked version
    mLayerMask = (mLayerMask & ~select) | (values & select);
}

void FSkybox::setColor(math::float4 color) noexcept {
    mSkyboxMaterialInstance->setParameter("color", color);
}

void FSkybox::commit(backend::DriverApi& driver) noexcept {
    mSkyboxMaterialInstance->commit(driver);
}

} // namespace filament
