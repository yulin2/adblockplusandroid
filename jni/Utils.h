/*
 * This file is part of Adblock Plus <http://adblockplus.org/>,
 * Copyright (C) 2006-2014 Eyeo GmbH
 *
 * Adblock Plus is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 3 as
 * published by the Free Software Foundation.
 *
 * Adblock Plus is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Adblock Plus.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef UTILS_H
#define UTILS_H

#include <string>
#include <algorithm>
#include <jni.h>
#include <stdexcept>

#include <AdblockPlus.h>
#include <AdblockPlus/tr1_memory.h>

#include "v8/v8stdint.h"

#define PKG(x) "org/adblockplus/libadblockplus/" x
#define TYP(x) "L" PKG(x) ";"

#define ABP_JNI_VERSION JNI_VERSION_1_6

void JniThrowException(JNIEnv* env, const std::string& message);

void JniThrowException(JNIEnv* env, const std::exception& e);

void JniThrowException(JNIEnv* env);

class JNIEnvAcquire
{
public:
  JNIEnvAcquire(JavaVM* javaVM);
  ~JNIEnvAcquire();

  JNIEnv* operator*()
  {
    return jniEnv;
  }

  JNIEnv* operator->()
  {
    return jniEnv;
  }

private:
  JavaVM* javaVM;
  JNIEnv* jniEnv;
  int attachmentStatus;
};

template<typename T>
class JniGlobalReference
{
public:
  JniGlobalReference(JNIEnv* env, T reference)
  {
    env->GetJavaVM(&javaVM);
    this->reference = static_cast<T>(env->NewGlobalRef(static_cast<jobject>(reference)));
  }

  ~JniGlobalReference()
  {
    JNIEnvAcquire env(javaVM);
    env->DeleteGlobalRef(static_cast<jobject>(reference));
  }

  JniGlobalReference(const JniGlobalReference& other);
  JniGlobalReference& operator=(const JniGlobalReference& other);

  T Get()
  {
    return reference;
  }

  typedef std::tr1::shared_ptr<JniGlobalReference<T> > Ptr;

private:
  T reference;
  JavaVM* javaVM;
};

inline void* JniLongToPtr(jlong value)
{
  return reinterpret_cast<void*>((size_t)value);
}

inline jlong JniPtrToLong(void* ptr)
{
  return (jlong)reinterpret_cast<size_t>(ptr);
}

// TODO: It's feeling a bit dirty casting to shared_ptr<T: JsValue> directly, so maybe
// implement a special cast functions that first reinterpret_casts to shared_ptr<JsValue> and then
// dynamic_casts to shared_ptr<T: JsValue> ... also as the same inheritance is mirrored on the Java
// side (and Java will throw a class cast exception on error) this shouldn't be an issue (TM)
template<typename T>
inline T* JniLongToTypePtr(jlong value)
{
  return reinterpret_cast<T*>((size_t)value);
}

jobject NewJniArrayList(JNIEnv* env);

std::string JniJavaToStdString(JNIEnv* env, jstring str);

void JniAddObjectToList(JNIEnv* env, jobject list, jobject value);

inline std::string JniGetStringField(JNIEnv* env, jclass clazz, jobject jObj, const char* name)
{
  return JniJavaToStdString(env, reinterpret_cast<jstring>(env->GetObjectField(jObj, env->GetFieldID(clazz, name, "Ljava/lang/String;"))));
}

inline bool JniGetBooleanField(JNIEnv* env, jclass clazz, jobject jObj, const char* name)
{
  return env->GetBooleanField(jObj, env->GetFieldID(clazz, name, "Z")) == JNI_TRUE;
}

inline int32_t JniGetIntField(JNIEnv* env, jclass clazz, jobject jObj, const char* name)
{
  return (int32_t)env->GetIntField(jObj, env->GetFieldID(clazz, name, "I"));
}

inline int64_t JniGetLongField(JNIEnv* env, jclass clazz, jobject jObj, const char* name)
{
  return (int64_t)env->GetLongField(jObj, env->GetFieldID(clazz, name, "J"));
}

inline jobject NewJniFilter(JNIEnv* env, const AdblockPlus::FilterPtr& filter)
{
  if (!filter.get())
  {
    return 0;
  }

  jclass clazz = env->FindClass(PKG("Filter"));
  jmethodID method = env->GetMethodID(clazz, "<init>", "(J)V");
  return env->NewObject(clazz, method, JniPtrToLong(new AdblockPlus::FilterPtr(filter)));
}

inline jobject NewJniSubscription(JNIEnv* env, const AdblockPlus::SubscriptionPtr& subscription)
{
  if (!subscription.get())
  {
    return 0;
  }

  jclass clazz = env->FindClass(PKG("Subscription"));
  jmethodID method = env->GetMethodID(clazz, "<init>", "(J)V");
  return env->NewObject(clazz, method, JniPtrToLong(new AdblockPlus::SubscriptionPtr(subscription)));
}

#define CATCH_AND_THROW(jEnv) \
  catch (const std::exception& except) \
  { \
    JniThrowException(jEnv, except); \
  } \
  catch (...) \
  { \
    JniThrowException(jEnv); \
  }

#define CATCH_THROW_AND_RETURN(jEnv, retVal) \
  catch (const std::exception& except) \
  { \
    JniThrowException(jEnv, except); \
    return retVal; \
  } \
  catch (...) \
  { \
    JniThrowException(jEnv); \
    return retVal; \
  }

#endif
