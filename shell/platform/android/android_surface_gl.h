// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_SHELL_PLATFORM_ANDROID_ANDROID_SURFACE_GL_H_
#define FLUTTER_SHELL_PLATFORM_ANDROID_ANDROID_SURFACE_GL_H_

#include <jni.h>

#include <memory>

#include "flutter/fml/macros.h"
#include "flutter/shell/gpu/gpu_surface_gl.h"
#include "flutter/shell/platform/android/android_context_gl.h"
#include "flutter/shell/platform/android/android_environment_gl.h"
#include "flutter/shell/platform/android/jni/platform_view_android_jni.h"
#include "flutter/shell/platform/android/surface/android_surface.h"

namespace flutter {

class AndroidSurfaceGL final : public GPUSurfaceGLDelegate,
                               public AndroidSurface {
 public:
  AndroidSurfaceGL(const std::shared_ptr<AndroidContext>& android_context,
                   std::shared_ptr<PlatformViewAndroidJNI> jni_facade);

  ~AndroidSurfaceGL() override;

  // |AndroidSurface|
  bool IsValid() const override;

  // |AndroidSurface|
  std::unique_ptr<Surface> CreateGPUSurface(
      GrDirectContext* gr_context) override;

  // |AndroidSurface|
  void TeardownOnScreenContext() override;

  // |AndroidSurface|
  bool OnScreenSurfaceResize(const SkISize& size) override;

  // |AndroidSurface|
  bool ResourceContextMakeCurrent() override;

  // |AndroidSurface|
  bool ResourceContextClearCurrent() override;

  // |AndroidSurface|
  bool SetNativeWindow(fml::RefPtr<AndroidNativeWindow> window) override;

  // |AndroidSurface|
  virtual std::unique_ptr<Surface> CreateSnapshotSurface() override;

  // |GPUSurfaceGLDelegate|
  std::unique_ptr<GLContextResult> GLContextMakeCurrent() override;

  // |GPUSurfaceGLDelegate|
  bool GLContextClearCurrent() override;

  // |GPUSurfaceGLDelegate|
  bool GLContextPresent(uint32_t fbo_id) override;

  // |GPUSurfaceGLDelegate|
  intptr_t GLContextFBO(GLFrameInfo frame_info) const override;

  // |GPUSurfaceGLDelegate|
  sk_sp<const GrGLInterface> GetGLInterface() const override;

  // Obtain a raw pointer to the on-screen AndroidEGLSurface.
  //
  // This method is intended for use in tests. Callers must not
  // delete the returned pointer.
  AndroidEGLSurface* GetOnscreenSurface() const {
    return onscreen_surface_.get();
  }

 private:
  fml::RefPtr<AndroidNativeWindow> native_window_;
  std::unique_ptr<AndroidEGLSurface> onscreen_surface_;
  std::unique_ptr<AndroidEGLSurface> offscreen_surface_;

  //----------------------------------------------------------------------------
  /// @brief      Takes the super class AndroidSurface's AndroidContext and
  ///             return a raw pointer to an AndroidContextGL.
  ///
  AndroidContextGL* GLContextPtr() const;

  FML_DISALLOW_COPY_AND_ASSIGN(AndroidSurfaceGL);
};

}  // namespace flutter

#endif  // FLUTTER_SHELL_PLATFORM_ANDROID_ANDROID_SURFACE_GL_H_
