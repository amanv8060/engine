// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_SHELL_PLATFORM_WINDOWS_FLUTTER_WINDOWS_VIEW_H_
#define FLUTTER_SHELL_PLATFORM_WINDOWS_FLUTTER_WINDOWS_VIEW_H_

#include <windowsx.h>

#include <memory>
#include <mutex>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

#include "flutter/shell/platform/common/client_wrapper/include/flutter/plugin_registrar.h"
#include "flutter/shell/platform/common/geometry.h"
#include "flutter/shell/platform/embedder/embedder.h"
#include "flutter/shell/platform/windows/angle_surface_manager.h"
#include "flutter/shell/platform/windows/cursor_handler.h"
#include "flutter/shell/platform/windows/flutter_windows_engine.h"
#include "flutter/shell/platform/windows/keyboard_handler_base.h"
#include "flutter/shell/platform/windows/keyboard_key_embedder_handler.h"
#include "flutter/shell/platform/windows/platform_handler.h"
#include "flutter/shell/platform/windows/public/flutter_windows.h"
#include "flutter/shell/platform/windows/text_input_plugin.h"
#include "flutter/shell/platform/windows/text_input_plugin_delegate.h"
#include "flutter/shell/platform/windows/window_binding_handler.h"
#include "flutter/shell/platform/windows/window_binding_handler_delegate.h"
#include "flutter/shell/platform/windows/window_state.h"

namespace flutter {

// ID for the window frame buffer.
inline constexpr uint32_t kWindowFrameBufferID = 0;

// An OS-windowing neutral abstration for flutter
// view that works with win32 hwnds and Windows::UI::Composition visuals.
class FlutterWindowsView : public WindowBindingHandlerDelegate,
                           public TextInputPluginDelegate {
 public:
  // Creates a FlutterWindowsView with the given implementor of
  // WindowBindingHandler.
  //
  // In order for object to render Flutter content the SetEngine method must be
  // called with a valid FlutterWindowsEngine instance.
  FlutterWindowsView(std::unique_ptr<WindowBindingHandler> window_binding);

  virtual ~FlutterWindowsView();

  // Configures the window instance with an instance of a running Flutter
  // engine.
  void SetEngine(std::unique_ptr<FlutterWindowsEngine> engine);

  // Creates rendering surface for Flutter engine to draw into.
  // Should be called before calling FlutterEngineRun using this view.
  void CreateRenderSurface();

  // Destroys current rendering surface if one has been allocated.
  void DestroyRenderSurface();

  // Return the currently configured WindowsRenderTarget.
  WindowsRenderTarget* GetRenderTarget() const;

  // Return the currently configured PlatformWindow.
  PlatformWindow GetPlatformWindow() const;

  // Returns the engine backing this view.
  FlutterWindowsEngine* GetEngine();

  // Tells the engine to generate a new frame
  void ForceRedraw();

  // Callbacks for clearing context, settings context and swapping buffers,
  // these are typically called on an engine-controlled (non-platform) thread.
  bool ClearContext();
  bool MakeCurrent();
  bool MakeResourceCurrent();
  bool SwapBuffers();

  // Callback for presenting a software bitmap.
  bool PresentSoftwareBitmap(const void* allocation,
                             size_t row_bytes,
                             size_t height);

  // Send initial bounds to embedder.  Must occur after engine has initialized.
  void SendInitialBounds();

  // Returns the frame buffer id for the engine to render to.
  uint32_t GetFrameBufferId(size_t width, size_t height);

  // Invoked by the engine right before the engine is restarted.
  //
  // This should reset necessary states to as if the view has just been
  // created. This is typically caused by a hot restart (Shift-R in CLI.)
  void OnPreEngineRestart();

  // |WindowBindingHandlerDelegate|
  void OnWindowSizeChanged(size_t width, size_t height) override;

  // |WindowBindingHandlerDelegate|
  void OnPointerMove(double x,
                     double y,
                     FlutterPointerDeviceKind device_kind,
                     int32_t device_id) override;

  // |WindowBindingHandlerDelegate|
  void OnPointerDown(double x,
                     double y,
                     FlutterPointerDeviceKind device_kind,
                     int32_t device_id,
                     FlutterPointerMouseButtons button) override;

  // |WindowBindingHandlerDelegate|
  void OnPointerUp(double x,
                   double y,
                   FlutterPointerDeviceKind device_kind,
                   int32_t device_id,
                   FlutterPointerMouseButtons button) override;

  // |WindowBindingHandlerDelegate|
  void OnPointerLeave(FlutterPointerDeviceKind device_kind,
                      int32_t device_id = 0) override;

  // |WindowBindingHandlerDelegate|
  void OnText(const std::u16string&) override;

  // |WindowBindingHandlerDelegate|
  void OnKey(int key,
             int scancode,
             int action,
             char32_t character,
             bool extended,
             bool was_down,
             KeyEventCallback callback) override;

  // |WindowBindingHandlerDelegate|
  void OnComposeBegin() override;

  // |WindowBindingHandlerDelegate|
  void OnComposeCommit() override;

  // |WindowBindingHandlerDelegate|
  void OnComposeEnd() override;

  // |WindowBindingHandlerDelegate|
  void OnComposeChange(const std::u16string& text, int cursor_pos) override;

  // |WindowBindingHandlerDelegate|
  void OnScroll(double x,
                double y,
                double delta_x,
                double delta_y,
                int scroll_offset_multiplier,
                FlutterPointerDeviceKind device_kind,
                int32_t device_id) override;

  // |WindowBindingHandlerDelegate|
  void OnPlatformBrightnessChanged() override;

  // |WindowBindingHandlerDelegate|
  virtual void OnUpdateSemanticsEnabled(bool enabled) override;

  // |WindowBindingHandlerDelegate|
  virtual gfx::NativeViewAccessible GetNativeViewAccessible() override;

  // |TextInputPluginDelegate|
  void OnCursorRectUpdated(const Rect& rect) override;

  // |TextInputPluginDelegate|
  void OnResetImeComposing() override;

 protected:
  // Called to create keyboard key handler.
  //
  // The provided |dispatch_event| is where to inject events into the system,
  // while |get_key_state| is where to acquire keyboard states. They will be
  // the system APIs in production classes, but might be replaced with mock
  // functions in unit tests.
  virtual std::unique_ptr<KeyboardHandlerBase> CreateKeyboardKeyHandler(
      BinaryMessenger* messenger,
      KeyboardKeyEmbedderHandler::GetKeyStateHandler get_key_state);

  // Called to create text input plugin.
  virtual std::unique_ptr<TextInputPlugin> CreateTextInputPlugin(
      BinaryMessenger* messenger);

 private:
  // Struct holding the state of an individual pointer. The engine doesn't keep
  // track of which buttons have been pressed, so it's the embedding's
  // responsibility.
  struct PointerState {
    // The device kind.
    FlutterPointerDeviceKind device_kind = kFlutterPointerDeviceKindMouse;

    // A virtual pointer ID that is unique across all device kinds.
    int32_t pointer_id = 0;

    // True if the last event sent to Flutter had at least one button pressed.
    bool flutter_state_is_down = false;

    // True if kAdd has been sent to Flutter. Used to determine whether
    // to send a kAdd event before sending an incoming pointer event, since
    // Flutter expects pointers to be added before events are sent for them.
    bool flutter_state_is_added = false;

    // The currently pressed buttons, as represented in FlutterPointerEvent.
    uint64_t buttons = 0;
  };

  // States a resize event can be in.
  enum class ResizeState {
    // When a resize event has started but is in progress.
    kResizeStarted,
    // After a resize event starts and the framework has been notified to
    // generate a frame for the right size.
    kFrameGenerated,
    // Default state for when no resize is in progress. Also used to indicate
    // that during a resize event, a frame with the right size has been rendered
    // and the buffers have been swapped.
    kDone,
  };

  // Initialize states related to keyboard.
  //
  // This is called when the view is first created, or restarted.
  void InitializeKeyboard();

  // Sends a window metrics update to the Flutter engine using current window
  // dimensions in physical
  void SendWindowMetrics(size_t width, size_t height, double dpiscale) const;

  // Reports a mouse movement to Flutter engine.
  void SendPointerMove(double x, double y, PointerState* state);

  // Reports mouse press to Flutter engine.
  void SendPointerDown(double x, double y, PointerState* state);

  // Reports mouse release to Flutter engine.
  void SendPointerUp(double x, double y, PointerState* state);

  // Reports mouse left the window client area.
  //
  // Win32 api doesn't have "mouse enter" event. Therefore, there is no
  // SendPointerEnter method. A mouse enter event is tracked then the "move"
  // event is called.
  void SendPointerLeave(PointerState* state);

  // Reports a keyboard character to Flutter engine.
  void SendText(const std::u16string&);

  // Reports a raw keyboard message to Flutter engine.
  void SendKey(int key,
               int scancode,
               int action,
               char32_t character,
               bool extended,
               bool was_down,
               KeyEventCallback callback);

  // Reports an IME compose begin event.
  //
  // Triggered when the user begins editing composing text using a multi-step
  // input method such as in CJK text input.
  void SendComposeBegin();

  // Reports an IME compose commit event.
  //
  // Triggered when the user commits the current composing text while using a
  // multi-step input method such as in CJK text input. Composing continues with
  // the next keypress.
  void SendComposeCommit();

  // Reports an IME compose end event.
  //
  // Triggered when the user commits the composing text while using a multi-step
  // input method such as in CJK text input.
  void SendComposeEnd();

  // Reports an IME composing region change event.
  //
  // Triggered when the user edits the composing text while using a multi-step
  // input method such as in CJK text input.
  void SendComposeChange(const std::u16string& text, int cursor_pos);

  // Reports scroll wheel events to Flutter engine.
  void SendScroll(double x,
                  double y,
                  double delta_x,
                  double delta_y,
                  int scroll_offset_multiplier,
                  FlutterPointerDeviceKind device_kind,
                  int32_t device_id);

  // Creates a PointerState object unless it already exists.
  PointerState* GetOrCreatePointerState(FlutterPointerDeviceKind device_kind,
                                        int32_t device_id);

  // Sets |event_data|'s phase to either kMove or kHover depending on the
  // current primary mouse button state.
  void SetEventPhaseFromCursorButtonState(FlutterPointerEvent* event_data,
                                          const PointerState* state) const;

  // Sends a pointer event to the Flutter engine based on given data.  Since
  // all input messages are passed in physical pixel values, no translation is
  // needed before passing on to engine.
  void SendPointerEventWithData(const FlutterPointerEvent& event_data,
                                PointerState* state);

  // Reports platform brightness change to Flutter engine.
  void SendPlatformBrightnessChanged();

  // Currently configured WindowsRenderTarget for this view used by
  // surface_manager for creation of render surfaces and bound to the physical
  // os window.
  std::unique_ptr<WindowsRenderTarget> render_target_;

  // The engine associated with this view.
  std::unique_ptr<FlutterWindowsEngine> engine_;

  // Keeps track of pointer states in relation to the window.
  std::unordered_map<int32_t, std::unique_ptr<PointerState>> pointer_states_;

  // The plugin registrar managing internal plugins.
  std::unique_ptr<PluginRegistrar> internal_plugin_registrar_;

  // Handlers for keyboard events from Windows.
  std::unique_ptr<KeyboardHandlerBase> keyboard_key_handler_;

  // Handlers for text events from Windows.
  std::unique_ptr<TextInputPlugin> text_input_plugin_;

  // Handler for the flutter/platform channel.
  std::unique_ptr<PlatformHandler> platform_handler_;

  // Handler for cursor events.
  std::unique_ptr<CursorHandler> cursor_handler_;

  // Currently configured WindowBindingHandler for view.
  std::unique_ptr<WindowBindingHandler> binding_handler_;

  // Resize events are synchronized using this mutex and the corresponding
  // condition variable.
  std::mutex resize_mutex_;
  std::condition_variable resize_cv_;

  // Indicates the state of a window resize event. Platform thread will be
  // blocked while this is not done. Guarded by resize_mutex_.
  ResizeState resize_status_ = ResizeState::kDone;

  // Target for the window width. Valid when resize_pending_ is set. Guarded by
  // resize_mutex_.
  size_t resize_target_width_ = 0;

  // Target for the window width. Valid when resize_pending_ is set. Guarded by
  // resize_mutex_.
  size_t resize_target_height_ = 0;

  // True when flutter's semantics tree is enabled.
  bool semantics_enabled_ = false;
};

}  // namespace flutter

#endif  // FLUTTER_SHELL_PLATFORM_WINDOWS_FLUTTER_WINDOWS_VIEW_H_
