// Copyright 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ASH_WM_CAPTION_BUTTONS_FRAME_CAPTION_BUTTON_CONTAINER_VIEW_H_
#define ASH_WM_CAPTION_BUTTONS_FRAME_CAPTION_BUTTON_CONTAINER_VIEW_H_

#include "ash/ash_export.h"
#include "ash/wm/caption_buttons/alternate_frame_size_button_delegate.h"
#include "ui/gfx/image/image_skia.h"
#include "ui/views/controls/button/button.h"
#include "ui/views/view.h"

namespace views {
class Widget;
}

namespace ash {
class FrameCaptionButton;
class FrameMaximizeButton;

// Container view for the frame caption buttons. It performs the appropriate
// action when a caption button is clicked.
class ASH_EXPORT FrameCaptionButtonContainerView
    : public views::View,
      public views::ButtonListener,
      public AlternateFrameSizeButtonDelegate {
 public:
  static const char kViewClassName[];

  // Whether the frame can be minimized (either via the maximize/restore button
  // or via a dedicated button).
  enum MinimizeAllowed {
    MINIMIZE_ALLOWED,
    MINIMIZE_DISALLOWED
  };

  // |frame| is the views::Widget that the caption buttons act on.
  // |minimize_allowed| indicates whether the frame can be minimized (either via
  // the maximize/restore button or via a dedicated button).
  FrameCaptionButtonContainerView(views::Widget* frame,
                                  MinimizeAllowed minimize_allowed);
  virtual ~FrameCaptionButtonContainerView();

  // For testing.
  class TestApi {
   public:
    explicit TestApi(FrameCaptionButtonContainerView* container_view)
        : container_view_(container_view) {
    }

    FrameCaptionButton* minimize_button() const {
      return container_view_->minimize_button_;
    }

    FrameCaptionButton* size_button() const {
      return container_view_->size_button_;
    }

    FrameCaptionButton* close_button() const {
      return container_view_->close_button_;
    }

   private:
    FrameCaptionButtonContainerView* container_view_;

    DISALLOW_COPY_AND_ASSIGN(TestApi);
  };

  // Returns the size button if using the old caption button style, returns NULL
  // otherwise.
  FrameMaximizeButton* GetOldStyleSizeButton();

  // Sets the resource ids of the images to paint the button for |icon|. The
  // FrameCaptionButtonContainerView will keep track of the images to use for
  // |icon| even if none of the buttons currently use |icon|.
  void SetButtonImages(CaptionButtonIcon icon,
                       int normal_image_id,
                       int hovered_image_id,
                       int pressed_image_id);

  // Tell the window controls to reset themselves to the normal state.
  void ResetWindowControls();

  // Determines the window HT* code for the caption button at |point|. Returns
  // HTNOWHERE if |point| is not over any of the caption buttons. |point| must
  // be in the coordinates of the FrameCaptionButtonContainerView.
  int NonClientHitTest(const gfx::Point& point) const;

  // views::View overrides:
  virtual gfx::Size GetPreferredSize() OVERRIDE;
  virtual void Layout() OVERRIDE;
  virtual const char* GetClassName() const OVERRIDE;
  virtual void OnPaint(gfx::Canvas* canvas) OVERRIDE;

 private:
  friend class FrameCaptionButtonContainerViewTest;

  struct ButtonIconIds {
    ButtonIconIds();
    ButtonIconIds(int normal_id, int hovered_id, int pressed_id);
    ~ButtonIconIds();

    int normal_image_id;
    int hovered_image_id;
    int pressed_image_id;
  };

  // Sets |button|'s icon to |icon|. If |animate| is ANIMATE_YES, the button
  // will crossfade to the new icon. If |animate| is ANIMATE_NO and
  // |icon| == |button|->icon(), the crossfade animation is progressed to the
  // end.
  void SetButtonIcon(FrameCaptionButton* button,
                     CaptionButtonIcon icon,
                     Animate animate);

  // views::ButtonListener override:
  virtual void ButtonPressed(views::Button* sender,
                             const ui::Event& event) OVERRIDE;

  // AlternateFrameSizeButton::Delegate overrides:
  virtual bool IsMinimizeButtonVisible() const OVERRIDE;
  virtual void SetButtonsToNormal(Animate animate) OVERRIDE;
  virtual void SetButtonIcons(CaptionButtonIcon minimize_button_icon,
                              CaptionButtonIcon close_button_icon,
                              Animate animate) OVERRIDE;
  virtual const FrameCaptionButton* GetButtonClosestTo(
      const gfx::Point& position_in_screen) const OVERRIDE;
  virtual void SetHoveredAndPressedButtons(
      const FrameCaptionButton* to_hover,
      const FrameCaptionButton* to_press) OVERRIDE;

  // The widget that the buttons act on.
  views::Widget* frame_;

  // The close button separator.
  gfx::ImageSkia button_separator_;

  // The buttons. In the normal button style, at most one of |minimize_button_|
  // and |size_button_| is visible.
  FrameCaptionButton* minimize_button_;
  FrameCaptionButton* size_button_;
  FrameCaptionButton* close_button_;

  // Mapping of the images needed to paint a button for each of the values of
  // CaptionButtonIcon.
  std::map<CaptionButtonIcon, ButtonIconIds> button_icon_id_map_;

  DISALLOW_COPY_AND_ASSIGN(FrameCaptionButtonContainerView);
};

}  // namesapace ash

#endif  // ASH_WM_CAPTION_BUTTONS_FRAME_CAPTION_BUTTON_CONTAINER_VIEW_H_
