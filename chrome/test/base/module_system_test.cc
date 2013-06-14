// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/test/base/module_system_test.h"

#include "base/callback.h"
#include "base/file_util.h"
#include "base/files/file_path.h"
#include "base/memory/scoped_ptr.h"
#include "base/path_service.h"
#include "base/strings/string_piece.h"
#include "chrome/common/chrome_paths.h"
#include "chrome/renderer/extensions/chrome_v8_context.h"
#include "chrome/renderer/extensions/logging_native_handler.h"
#include "chrome/renderer/extensions/object_backed_native_handler.h"
#include "ui/base/resource/resource_bundle.h"

#include <map>
#include <string>

using extensions::ModuleSystem;
using extensions::NativeHandler;
using extensions::ObjectBackedNativeHandler;

// Native JS functions for doing asserts.
class AssertNatives : public ObjectBackedNativeHandler {
 public:
  explicit AssertNatives(extensions::ChromeV8Context* context)
      : ObjectBackedNativeHandler(context),
        assertion_made_(false),
        failed_(false) {
    RouteFunction("AssertTrue", base::Bind(&AssertNatives::AssertTrue,
        base::Unretained(this)));
    RouteFunction("AssertFalse", base::Bind(&AssertNatives::AssertFalse,
        base::Unretained(this)));
  }

  bool assertion_made() { return assertion_made_; }
  bool failed() { return failed_; }

  void AssertTrue(const v8::FunctionCallbackInfo<v8::Value>& args) {
    CHECK_EQ(1, args.Length());
    assertion_made_ = true;
    failed_ = failed_ || !args[0]->ToBoolean()->Value();
  }

  void AssertFalse(const v8::FunctionCallbackInfo<v8::Value>& args) {
    CHECK_EQ(1, args.Length());
    assertion_made_ = true;
    failed_ = failed_ || args[0]->ToBoolean()->Value();
  }

 private:
  bool assertion_made_;
  bool failed_;
};

// Source map that operates on std::strings.
class StringSourceMap : public ModuleSystem::SourceMap {
 public:
  StringSourceMap() {}
  virtual ~StringSourceMap() {}

  virtual v8::Handle<v8::Value> GetSource(const std::string& name) OVERRIDE {
    if (source_map_.count(name) == 0)
      return v8::Undefined();
    return v8::String::New(source_map_[name].c_str());
  }

  virtual bool Contains(const std::string& name) OVERRIDE {
    return source_map_.count(name);
  }

  void RegisterModule(const std::string& name, const std::string& source) {
    CHECK_EQ(0u, source_map_.count(name)) << "Module " << name << " not found";
    source_map_[name] = source;
  }

 private:
  std::map<std::string, std::string> source_map_;
};

class FailsOnException : public ModuleSystem::ExceptionHandler {
 public:
  virtual void HandleUncaughtException(const v8::TryCatch& try_catch) OVERRIDE {
    FAIL() << "Uncaught exception: " << CreateExceptionString(try_catch);
  }
};

ModuleSystemTest::ModuleSystemTest()
    : isolate_(v8::Isolate::GetCurrent()),
      handle_scope_(isolate_),
      context_(
          new extensions::ChromeV8Context(
              v8::Context::New(isolate_),
              NULL,  // WebFrame
              NULL,  // Extension
              extensions::Feature::UNSPECIFIED_CONTEXT)),
      source_map_(new StringSourceMap()),
      should_assertions_be_made_(true) {
  context_->v8_context()->Enter();
  assert_natives_ = new AssertNatives(context_.get());
  module_system_.reset(new ModuleSystem(context_.get(), source_map_.get()));
  module_system_->RegisterNativeHandler("assert", scoped_ptr<NativeHandler>(
      assert_natives_));
  module_system_->RegisterNativeHandler("logging", scoped_ptr<NativeHandler>(
      new extensions::LoggingNativeHandler(context_.get())));
  module_system_->SetExceptionHandlerForTest(
      scoped_ptr<ModuleSystem::ExceptionHandler>(new FailsOnException));
}

ModuleSystemTest::~ModuleSystemTest() {
  module_system_.reset();
  context_->v8_context()->Exit();
}

void ModuleSystemTest::RegisterModule(const std::string& name,
                                      const std::string& code) {
  source_map_->RegisterModule(name, code);
}

void ModuleSystemTest::RegisterModule(const std::string& name,
                                      int resource_id) {
  const std::string& code = ResourceBundle::GetSharedInstance().
      GetRawDataResource(resource_id).as_string();
  source_map_->RegisterModule(name, code);
}

void ModuleSystemTest::OverrideNativeHandler(const std::string& name,
                                             const std::string& code) {
  RegisterModule(name, code);
  module_system_->OverrideNativeHandlerForTest(name);
}

void ModuleSystemTest::RegisterTestFile(const std::string& module_name,
                                        const std::string& file_name) {
  base::FilePath test_js_file_path;
  ASSERT_TRUE(PathService::Get(chrome::DIR_TEST_DATA, &test_js_file_path));
  test_js_file_path = test_js_file_path.AppendASCII("extensions")
                                       .AppendASCII(file_name);
  std::string test_js;
  ASSERT_TRUE(file_util::ReadFileToString(test_js_file_path, &test_js));
  source_map_->RegisterModule(module_name, test_js);
}

void ModuleSystemTest::TearDown() {
  // All tests must assert at least once unless otherwise specified.
  EXPECT_EQ(should_assertions_be_made_,
            assert_natives_->assertion_made());
  EXPECT_FALSE(assert_natives_->failed());
}

void ModuleSystemTest::ExpectNoAssertionsMade() {
  should_assertions_be_made_ = false;
}

v8::Handle<v8::Object> ModuleSystemTest::CreateGlobal(const std::string& name) {
  v8::HandleScope handle_scope;
  v8::Handle<v8::Object> object = v8::Object::New();
  v8::Context::GetCurrent()->Global()->Set(v8::String::New(name.c_str()),
                                           object);
  return handle_scope.Close(object);
}
