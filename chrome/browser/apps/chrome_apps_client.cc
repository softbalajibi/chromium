// Copyright 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/browser/apps/chrome_apps_client.h"

#include "apps/app_window.h"
#include "base/memory/singleton.h"
#include "chrome/browser/browser_process.h"
#include "chrome/browser/profiles/profile_manager.h"
#include "chrome/browser/ui/apps/app_metro_infobar_delegate_win.h"
#include "extensions/common/extension.h"

#if defined(OS_WIN)
#include "win8/util/win8_util.h"
#endif

// TODO(jamescook): We probably shouldn't compile this class at all on Android.
// See http://crbug.com/343612
#if !defined(OS_ANDROID)
#include "chrome/browser/lifetime/application_lifetime.h"
#include "chrome/browser/ui/apps/chrome_app_window_delegate.h"
#endif

ChromeAppsClient::ChromeAppsClient() {}

ChromeAppsClient::~ChromeAppsClient() {}

// static
ChromeAppsClient* ChromeAppsClient::GetInstance() {
  return Singleton<ChromeAppsClient,
                   LeakySingletonTraits<ChromeAppsClient> >::get();
}

std::vector<content::BrowserContext*>
    ChromeAppsClient::GetLoadedBrowserContexts() {
  std::vector<Profile*> profiles =
      g_browser_process->profile_manager()->GetLoadedProfiles();
  return std::vector<content::BrowserContext*>(profiles.begin(),
                                               profiles.end());
}

bool ChromeAppsClient::CheckAppLaunch(content::BrowserContext* context,
                                      const extensions::Extension* extension) {
#if defined(OS_WIN)
  // On Windows 8's single window Metro mode we can not launch platform apps.
  // Offer to switch Chrome to desktop mode.
  if (win8::IsSingleWindowMetroMode()) {
    AppMetroInfoBarDelegateWin::Create(
        Profile::FromBrowserContext(context),
        AppMetroInfoBarDelegateWin::LAUNCH_PACKAGED_APP,
        extension->id());
    return false;
  }
#endif
  return true;
}

apps::AppWindow* ChromeAppsClient::CreateAppWindow(
    content::BrowserContext* context,
    const extensions::Extension* extension) {
#if defined(OS_ANDROID)
  return NULL;
#else
  return new apps::AppWindow(context, new ChromeAppWindowDelegate, extension);
#endif
}

void ChromeAppsClient::StartKeepAlive() {
#if !defined(OS_ANDROID)
  chrome::StartKeepAlive();
#endif
}

void ChromeAppsClient::EndKeepAlive() {
#if !defined(OS_ANDROID)
  chrome::EndKeepAlive();
#endif
}
