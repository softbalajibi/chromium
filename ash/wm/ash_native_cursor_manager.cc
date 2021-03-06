// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ash/wm/ash_native_cursor_manager.h"

#include "ash/display/cursor_window_controller.h"
#include "ash/display/display_controller.h"
#include "ash/shell.h"
#include "ash/wm/image_cursors.h"
#include "base/logging.h"
#include "ui/aura/env.h"
#include "ui/aura/window_event_dispatcher.h"
#include "ui/base/cursor/cursor.h"

namespace ash {
namespace  {

void SetCursorOnAllRootWindows(gfx::NativeCursor cursor) {
  aura::Window::Windows root_windows =
      Shell::GetInstance()->GetAllRootWindows();
  for (aura::Window::Windows::iterator iter = root_windows.begin();
       iter != root_windows.end(); ++iter)
    (*iter)->GetDispatcher()->host()->SetCursor(cursor);
#if defined(OS_CHROMEOS)
  Shell::GetInstance()->display_controller()->
      cursor_window_controller()->SetCursor(cursor);
#endif
}

void NotifyCursorVisibilityChange(bool visible) {
  aura::Window::Windows root_windows =
      Shell::GetInstance()->GetAllRootWindows();
  for (aura::Window::Windows::iterator iter = root_windows.begin();
       iter != root_windows.end(); ++iter)
    (*iter)->GetDispatcher()->host()->OnCursorVisibilityChanged(visible);
#if defined(OS_CHROMEOS)
  Shell::GetInstance()->display_controller()->cursor_window_controller()->
      SetVisibility(visible);
#endif
}

void NotifyMouseEventsEnableStateChange(bool enabled) {
  aura::Window::Windows root_windows =
      Shell::GetInstance()->GetAllRootWindows();
  for (aura::Window::Windows::iterator iter = root_windows.begin();
       iter != root_windows.end(); ++iter)
    (*iter)->GetDispatcher()->OnMouseEventsEnableStateChanged(enabled);
  // Mirror window never process events.
}

}  // namespace

AshNativeCursorManager::AshNativeCursorManager()
    : native_cursor_enabled_(true),
      image_cursors_(new ImageCursors) {
}

AshNativeCursorManager::~AshNativeCursorManager() {
}


void AshNativeCursorManager::SetNativeCursorEnabled(bool enabled) {
  native_cursor_enabled_ = enabled;

  views::corewm::CursorManager* cursor_manager =
      Shell::GetInstance()->cursor_manager();
  SetCursor(cursor_manager->GetCursor(), cursor_manager);
}

void AshNativeCursorManager::SetDisplay(
    const gfx::Display& display,
    views::corewm::NativeCursorManagerDelegate* delegate) {
  if (image_cursors_->SetDisplay(display))
    SetCursor(delegate->GetCursor(), delegate);
#if defined(OS_CHROMEOS)
  Shell::GetInstance()->display_controller()->cursor_window_controller()->
      SetDisplay(display);
#endif
}

void AshNativeCursorManager::SetCursor(
    gfx::NativeCursor cursor,
    views::corewm::NativeCursorManagerDelegate* delegate) {
  gfx::NativeCursor new_cursor = cursor;
  if (native_cursor_enabled_) {
    image_cursors_->SetPlatformCursor(&new_cursor);
  } else {
    gfx::NativeCursor invisible_cursor(ui::kCursorNone);
    image_cursors_->SetPlatformCursor(&invisible_cursor);
    if (new_cursor == ui::kCursorCustom) {
      new_cursor = invisible_cursor;
    } else {
      new_cursor.SetPlatformCursor(invisible_cursor.platform());
    }
  }
  new_cursor.set_device_scale_factor(
      image_cursors_->GetDisplay().device_scale_factor());

  delegate->CommitCursor(new_cursor);

  if (delegate->IsCursorVisible())
    SetCursorOnAllRootWindows(new_cursor);
}

void AshNativeCursorManager::SetCursorSet(
    ui::CursorSetType cursor_set,
    views::corewm::NativeCursorManagerDelegate* delegate) {
  image_cursors_->SetCursorSet(cursor_set);
  delegate->CommitCursorSet(cursor_set);

  // Sets the cursor to reflect the scale change immediately.
  if (delegate->IsCursorVisible())
    SetCursor(delegate->GetCursor(), delegate);

#if defined(OS_CHROMEOS)
  Shell::GetInstance()->display_controller()->cursor_window_controller()->
      SetCursorSet(cursor_set);
#endif
}

void AshNativeCursorManager::SetScale(
    float scale,
    views::corewm::NativeCursorManagerDelegate* delegate) {
  image_cursors_->SetScale(scale);
  delegate->CommitScale(scale);

  // Sets the cursor to reflect the scale change immediately.
  SetCursor(delegate->GetCursor(), delegate);
}

void AshNativeCursorManager::SetVisibility(
    bool visible,
    views::corewm::NativeCursorManagerDelegate* delegate) {
  delegate->CommitVisibility(visible);

  if (visible) {
    SetCursor(delegate->GetCursor(), delegate);
  } else {
    gfx::NativeCursor invisible_cursor(ui::kCursorNone);
    image_cursors_->SetPlatformCursor(&invisible_cursor);
    SetCursorOnAllRootWindows(invisible_cursor);
  }

  NotifyCursorVisibilityChange(visible);
}

void AshNativeCursorManager::SetMouseEventsEnabled(
    bool enabled,
    views::corewm::NativeCursorManagerDelegate* delegate) {
  delegate->CommitMouseEventsEnabled(enabled);

  if (enabled) {
    aura::Env::GetInstance()->set_last_mouse_location(
        disabled_cursor_location_);
  } else {
    disabled_cursor_location_ = aura::Env::GetInstance()->last_mouse_location();
  }

  SetVisibility(delegate->IsCursorVisible(), delegate);
  NotifyMouseEventsEnableStateChange(enabled);
}

}  // namespace ash
