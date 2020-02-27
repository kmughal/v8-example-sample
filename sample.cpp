#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/param.h>
#include <sys/sysctl.h>
#include <exception>
#include <string>

#include "include/libplatform/libplatform.h"
#include "include/v8.h"

using v8::Context;
using v8::Function;
using v8::FunctionTemplate;
using v8::Global;
using v8::Handle;
using v8::HandleScope;
using v8::Isolate;
using v8::Local;
using v8::NewStringType;
using v8::ObjectTemplate;
using v8::Persistent;
using v8::PropertyCallbackInfo;
using v8::String;
using v8::Value;
using v8::Promise;

int getNumberOfCores() {
  int nm[2];
  size_t len = 4;
  uint32_t count;

  nm[0] = CTL_HW;
  nm[1] = HW_AVAILCPU;
  sysctl(nm, 2, &count, &len, NULL, 0);

  if (count < 1) {
    nm[1] = HW_NCPU;
    sysctl(nm, 2, &count, &len, NULL, 0);
    if (count < 1) {
      count = 1;
    }
  }
  return count;
}

inline bool exist(const std::string& name) {
  std::ifstream file(name);
  if (!file)       // If the file was not found, then file is 0, i.e. !file=1 or
                   // true.
    return false;  // The file was not found.
  else             // If the file was found, then file is non-0.
    return true;   // The file was found.
}

void FindCors(Local<String> name, const PropertyCallbackInfo<Value>& info) {
  int cors = getNumberOfCores();
  info.GetReturnValue().Set(cors);
}

void HelloWorld(const v8::FunctionCallbackInfo<v8::Value>& args) {
  Isolate* isolate = args.GetIsolate();
  auto context = isolate->GetCurrentContext();
  auto resolver = v8::Promise::Resolver::New(context).ToLocalChecked();

  args.GetReturnValue().Set(resolver->GetPromise());
  resolver->Resolve(context,String::NewFromUtf8(isolate, "world"));
}

char* Read_File(char* filename) {
  if (!exist(filename)) {
    char* str1 = (char*)"This is GeeksForGeeks";
    throw "file not found";
  }
  FILE* f = fopen(filename, "rb");
  fseek(f, 0, SEEK_END);
  long fsize = ftell(f);
  fseek(f, 0, SEEK_SET);

  char* string = (char*)malloc(fsize + 1);
  fread(string, fsize, 1, f);
  fclose(f);

  string[fsize] = 0;
  return string;
}

void ReadFileFromSystem(const v8::FunctionCallbackInfo<v8::Value>& args) {
  if (args.Length() < 1) return;

  Isolate* isolate = args.GetIsolate();
  HandleScope scope(isolate);

  Local<Value> result = args[0];
  Local<Function> localFunction = Local<Function>::Cast(args[1]);

  if (localFunction->IsFunction()) {
    char* file_contents;
    Handle<Value> argv[2];
    argv[0] = v8::Null(isolate);
    try {
      v8::String::Utf8Value utf8(isolate, result);
      file_contents = Read_File(*utf8);
    } catch (const char* msg) {
      argv[0] = v8::String::NewFromUtf8(isolate, msg);
    }

    argv[1] = v8::String::NewFromUtf8(isolate, file_contents);
    localFunction->Call(isolate->GetCurrentContext()->Global(), 2, argv);
  }
}

char* GetFile(char* path) {
  FILE* f = fopen(
     path
      "fs-example.js",
      "rb");
  fseek(f, 0, SEEK_END);
  long fsize = ftell(f);
  fseek(f, 0, SEEK_SET);

  char* string = (char*)malloc(fsize + 1);
  fread(string, fsize, 1, f);
  fclose(f);

  string[fsize] = 0;
  return string;
}

void LogCallback(const v8::FunctionCallbackInfo<v8::Value>& args) {
  if (args.Length() < 1) return;
  Isolate* isolate = args.GetIsolate();
  HandleScope scope(isolate);
  Local<Value> result = args[0];
  v8::String::Utf8Value utf8(isolate, result);
  printf("%s\n", *utf8);
}

int main(int argc, char* argv[]) {
  // Initialize V8.
  v8::V8::InitializeICUDefaultLocation(argv[0]);
  v8::V8::InitializeExternalStartupData(argv[0]);
  std::unique_ptr<v8::Platform> platform = v8::platform::NewDefaultPlatform();
  v8::V8::InitializePlatform(platform.get());
  v8::V8::Initialize();

  // Create a new Isolate and make it the current one.
  v8::Isolate::CreateParams create_params;
  create_params.array_buffer_allocator =
      v8::ArrayBuffer::Allocator::NewDefaultAllocator();
  v8::Isolate* isolate = v8::Isolate::New(create_params);
  {
    v8::Isolate::Scope isolate_scope(isolate);

    // Create a stack-allocated handle scope.
    v8::HandleScope handle_scope(isolate);

    // Create a new context.
    v8::Local<v8::Context> context = v8::Context::New(isolate);

    // Enter the context for compiling and running the hello world script.
    v8::Context::Scope context_scope(context);
    {
      Global<Context> context_;

      Local<ObjectTemplate> global = ObjectTemplate::New(isolate);

      global->Set(String::NewFromUtf8(isolate, "log", NewStringType::kNormal)
                      .ToLocalChecked(),
                  FunctionTemplate::New(isolate, LogCallback));

      global->SetAccessor(
          String::NewFromUtf8(isolate, "cors", NewStringType::kInternalized)
              .ToLocalChecked(),
          FindCors);

      global->Set(
          String::NewFromUtf8(isolate, "readFile", NewStringType::kNormal)
              .ToLocalChecked(),
          FunctionTemplate::New(isolate, ReadFileFromSystem));

      global->Set(
          String::NewFromUtf8(isolate, "helloWorld", NewStringType::kNormal)
              .ToLocalChecked(),
          FunctionTemplate::New(isolate, HelloWorld));

      // Add a test property to the global object...
      global->Set(isolate, "func_property", v8::Number::New(isolate, 1));

      v8::Local<v8::Context> context = Context::New(isolate, NULL, global);
      v8::Local<v8::String> source =
          v8::String::NewFromUtf8(isolate, GetFile(),
                                  v8::NewStringType::kNormal)
              .ToLocalChecked();

      // Compile the source code.
      v8::Local<v8::Script> script =
          v8::Script::Compile(context, source).ToLocalChecked();

      v8::Local<v8::Value> result = script->Run(context).ToLocalChecked();
    }
  }

  // Dispose the isolate and tear down V8.
  isolate->Dispose();
  v8::V8::Dispose();
  v8::V8::ShutdownPlatform();
  delete create_params.array_buffer_allocator;
  return 0;
}

// g++ -I. -Iinclude samples/sample.cc -o sample -lv8_monolith -Lout.gn/x64.release.sample/obj/ -pthread -std=c++0x
