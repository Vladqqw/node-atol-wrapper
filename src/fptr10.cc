#include <stdio.h>
#include "fptr10.h"

Nan::Persistent<v8::FunctionTemplate> Fptr10::constructor;

NAN_MODULE_INIT(Fptr10::Init) {
  v8::Local<v8::FunctionTemplate> ctor = Nan::New<v8::FunctionTemplate>(Fptr10::New);
  constructor.Reset(ctor);
  ctor->InstanceTemplate()->SetInternalFieldCount(1);
  ctor->SetClassName(Nan::New("Fptr10").ToLocalChecked());

  // link our getters and setter to the object property
  Nan::SetAccessor(ctor->InstanceTemplate(), Nan::New("x").ToLocalChecked(), Fptr10::HandleGetters, Fptr10::HandleSetters);

  Nan::SetPrototypeMethod(ctor, "test", Test);
  Nan::SetPrototypeMethod(ctor, "create", Create);
  Nan::SetPrototypeMethod(ctor, "destroy", Destroy);
  Nan::SetPrototypeMethod(ctor, "getSettings", GetSettings);
  Nan::SetPrototypeMethod(ctor, "setSettings", SetSettings);
  Nan::SetPrototypeMethod(ctor, "open", Open);
  Nan::SetPrototypeMethod(ctor, "close", Close);
  Nan::SetPrototypeMethod(ctor, "processJson", ProcessJson);

  target->Set(Nan::New("Fptr10").ToLocalChecked(), ctor->GetFunction());
}

NAN_METHOD(Fptr10::New) {
  // throw an error if constructor is called without new keyword
  if(!info.IsConstructCall()) {
    return Nan::ThrowError(Nan::New("Fptr10::New - called without new keyword").ToLocalChecked());
  }

  // create a new instance and wrap our javascript instance
  Fptr10* fptr = new Fptr10();
  fptr->Wrap(info.Holder());

  // initialize it's values
  fptr->x = Nan::New(123.0) -> NumberValue();

  // return the wrapped javascript instance
  info.GetReturnValue().Set(info.Holder());
}

NAN_METHOD(Fptr10::Test) {
  // unwrap
  Fptr10* self = Nan::ObjectWrap::Unwrap<Fptr10>(info.This());
  self->x = self->x + 1.0;
  info.GetReturnValue().Set(Nan::New(self->x));
}

NAN_GETTER(Fptr10::HandleGetters) {
  Fptr10* self = Nan::ObjectWrap::Unwrap<Fptr10>(info.This());

  std::string propertyName = std::string(*Nan::Utf8String(property));
  if (propertyName == "x") {
    info.GetReturnValue().Set(self->x);
  } else {
    info.GetReturnValue().Set(Nan::Undefined());
  }
}

NAN_SETTER(Fptr10::HandleSetters) {
  Fptr10* self = Nan::ObjectWrap::Unwrap<Fptr10>(info.This());

  if(!value->IsNumber()) {
    return Nan::ThrowError(Nan::New("expected value to be a number").ToLocalChecked());
  }

  std::string propertyName = std::string(*Nan::Utf8String(property));
  if (propertyName == "x") {
    self->x = value->NumberValue();
  }
}

NAN_METHOD(Fptr10::Create) {
  Fptr10* self = Nan::ObjectWrap::Unwrap<Fptr10>(info.This());
  libfptr_create(&(self->fptr));
  info.GetReturnValue().Set(Nan::Undefined());
}

NAN_METHOD(Fptr10::Destroy) {
  Fptr10* self = Nan::ObjectWrap::Unwrap<Fptr10>(info.This());
  libfptr_destroy(&(self->fptr));
  info.GetReturnValue().Set(Nan::Undefined());
}

NAN_METHOD(Fptr10::GetSettings) {
  Fptr10* self = Nan::ObjectWrap::Unwrap<Fptr10>(info.This());
  std::vector<wchar_t> settings(1024);
  int size = libfptr_get_settings(self->fptr, &settings[0], settings.size());
  if (size > settings.size())
  {
      settings.resize(size);
      libfptr_get_settings(self->fptr, &settings[0], settings.size());
  }
  std::string strSett = ws2s(std::wstring(&settings[0]));
  Nan::JSON NanJSON;
  Nan::MaybeLocal<v8::Value> result = NanJSON.Parse(Nan::New(strSett).ToLocalChecked());
  if (!result.IsEmpty()) {
    info.GetReturnValue().Set(result.ToLocalChecked());
  } else {
    info.GetReturnValue().Set(Nan::Undefined());
  }
}

NAN_METHOD(Fptr10::SetSettings) {
   // expect exactly 1 argument
  if(info.Length() != 1) {
    return Nan::ThrowError(Nan::New("Fptr10::SetSettings - expected 1 json argument").ToLocalChecked());
  }
  // argument must be object
  if(!info[0]->IsObject()) {
    return Nan::ThrowError(Nan::New("Fptr10::SetSettings - expected argument to be object").ToLocalChecked());
  }
  Fptr10* self = Nan::ObjectWrap::Unwrap<Fptr10>(info.This());
  Nan::JSON NanJSON;
  Nan::MaybeLocal<v8::String> result = NanJSON.Stringify(info[0]->ToObject());
  if (!result.IsEmpty()) {
    v8::Local<v8::String> strSett = result.ToLocalChecked();
    Nan::Utf8String utf8Sett(strSett); // take the string arg and convert it to v8::string
    std::string sSett(*utf8Sett); // take the v8::string convert it to c++ class string
    std::wstring wSett = s2ws(sSett);

//    printf("sett - %S", &wSett[0]);
    libfptr_set_settings(self->fptr, &wSett[0]);
    info.GetReturnValue().Set(Nan::True());
  } else {
    info.GetReturnValue().Set(Nan::False());
  }
}

NAN_METHOD(Fptr10::Open){
  Fptr10* self = Nan::ObjectWrap::Unwrap<Fptr10>(info.This());
  libfptr_open(self->fptr);
  info.GetReturnValue().Set(Nan::True());
}

NAN_METHOD(Fptr10::Close){
  Fptr10* self = Nan::ObjectWrap::Unwrap<Fptr10>(info.This());
  libfptr_close(self->fptr);
  info.GetReturnValue().Set(Nan::True());
}

NAN_METHOD(Fptr10::ProcessJson){
   // expect exactly 1 argument
  if(info.Length() != 1) {
    return Nan::ThrowError(Nan::New("Fptr10::ProcessJson - expected 1 json argument").ToLocalChecked());
  }
  // argument must be object
  if(!info[0]->IsObject()) {
    return Nan::ThrowError(Nan::New("Fptr10::ProcessJson - expected argument to be object").ToLocalChecked());
  }

  Fptr10* self = Nan::ObjectWrap::Unwrap<Fptr10>(info.This());
  Nan::JSON NanJSON;
  Nan::MaybeLocal<v8::String> task = NanJSON.Stringify(info[0]->ToObject());
  if (!task.IsEmpty()) {
      Nan::Utf8String utf8Sett(task.ToLocalChecked()); // take the string arg and convert it to v8::string
      std::string sSett(*utf8Sett); // take the v8::string convert it to c++ class string
      std::wstring wSett = s2ws(sSett);

      libfptr_set_param_str(self->fptr, LIBFPTR_PARAM_JSON_DATA, &wSett[0]);
      libfptr_process_json(self->fptr);
      // TODO: checkErrors;


      std::vector<wchar_t> result(128);
      int size = libfptr_get_param_str(self->fptr, LIBFPTR_PARAM_JSON_DATA, &result[0], result.size());
      if (size > result.size())
      {
        result.resize(size);
        libfptr_get_param_str(self->fptr, LIBFPTR_PARAM_JSON_DATA, &result[0], result.size());
      }
      std::string strResult = ws2s(std::wstring(&result[0]));
      Nan::JSON NanJSON;
      Nan::MaybeLocal<v8::Value> jsResult = NanJSON.Parse(Nan::New(strResult).ToLocalChecked());
      if (!jsResult.IsEmpty()) {
        info.GetReturnValue().Set(jsResult.ToLocalChecked());
      } else {
        info.GetReturnValue().Set(Nan::Undefined());
      }
  } else {
    info.GetReturnValue().Set(Nan::Undefined());
  }
}