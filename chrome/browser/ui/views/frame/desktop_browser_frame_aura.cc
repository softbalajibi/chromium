// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/browser/ui/views/frame/desktop_browser_frame_aura.h"

#include "chrome/app/chrome_command_ids.h"
#include "chrome/browser/ui/views/frame/browser_desktop_window_tree_host.h"
#include "chrome/browser/ui/views/frame/browser_shutdown.h"
#include "chrome/browser/ui/views/frame/browser_view.h"
#include "chrome/browser/ui/views/frame/desktop_user_action_handler_aura.h"
#include "grit/chromium_strings.h"
#include "grit/generated_resources.h"
#include "ui/aura/client/aura_constants.h"
#include "ui/aura/window.h"
#include "ui/aura/window_event_dispatcher.h"
#include "ui/aura/window_observer.h"
#include "ui/base/hit_test.h"
#include "ui/base/l10n/l10n_util.h"
#include "ui/base/models/simple_menu_model.h"
#include "ui/gfx/font.h"
#include "ui/views/corewm/visibility_controller.h"
#include "ui/views/view.h"

using aura::Window;

///////////////////////////////////////////////////////////////////////////////
// DesktopBrowserFrameAura, public:

DesktopBrowserFrameAura::DesktopBrowserFrameAura(
    BrowserFrame* browser_frame,
    BrowserView* browser_view)
    : views::DesktopNativeWidgetAura(browser_frame),
      browser_view_(browser_view),
      browser_frame_(browser_frame),
      browser_desktop_window_tree_host_(NULL) {
  GetNativeWindow()->SetName("BrowserFrameAura");
}

///////////////////////////////////////////////////////////////////////////////
// DesktopBrowserFrameAura, protected:

DesktopBrowserFrameAura::~DesktopBrowserFrameAura() {
}

///////////////////////////////////////////////////////////////////////////////
// DesktopBrowserFrameAura, views::DesktopNativeWidgetAura overrides:

void DesktopBrowserFrameAura::OnHostClosed() {
  // Destroy any remaining WebContents early on. Doing so may result in
  // calling back to one of the Views/LayoutManagers or supporting classes of
  // BrowserView. By destroying here we ensure all said classes are valid.
  DestroyBrowserWebContents(browser_view_->browser());
  aura::client::SetVisibilityClient(GetNativeView()->GetRootWindow(), NULL);
  DesktopNativeWidgetAura::OnHostClosed();
}

void DesktopBrowserFrameAura::InitNativeWidget(
    const views::Widget::InitParams& params) {
  browser_desktop_window_tree_host_ =
      BrowserDesktopWindowTreeHost::CreateBrowserDesktopWindowTreeHost(
          browser_frame_,
          this,
          browser_view_,
          browser_frame_);
  views::Widget::InitParams modified_params = params;
  modified_params.desktop_window_tree_host =
      browser_desktop_window_tree_host_->AsDesktopWindowTreeHost();
  DesktopNativeWidgetAura::InitNativeWidget(modified_params);

  user_action_client_.reset(
      new DesktopUserActionHandlerAura(browser_view_->browser()));
  aura::client::SetUserActionClient(GetNativeView()->GetRootWindow(),
                                    user_action_client_.get());

  visibility_controller_.reset(new views::corewm::VisibilityController);
  aura::client::SetVisibilityClient(GetNativeView()->GetRootWindow(),
                                    visibility_controller_.get());
  views::corewm::SetChildWindowVisibilityChangesAnimated(
      GetNativeView()->GetRootWindow());
}

////////////////////////////////////////////////////////////////////////////////
// DesktopBrowserFrameAura, NativeBrowserFrame implementation:

views::NativeWidget* DesktopBrowserFrameAura::AsNativeWidget() {
  return this;
}

const views::NativeWidget* DesktopBrowserFrameAura::AsNativeWidget() const {
  return this;
}

bool DesktopBrowserFrameAura::UsesNativeSystemMenu() const {
  return browser_desktop_window_tree_host_->UsesNativeSystemMenu();
}

int DesktopBrowserFrameAura::GetMinimizeButtonOffset() const {
  return browser_desktop_window_tree_host_->GetMinimizeButtonOffset();
}
