/*
 * Copyright 2016, The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <jni.h>

namespace android {

/* JNI Helpers for wifi_hal to WifiNative bridge implementation */

class JNIHelper;

template<typename T>
class JNIObject {
protected:
    JNIHelper &mHelper;
    T mObj;
public:
    JNIObject(JNIHelper &helper, T obj);
    JNIObject(const JNIObject<T>& rhs);
    virtual ~JNIObject();
    JNIHelper& getHelper() const {
        return mHelper;
    }
    T get() const {
        return mObj;
    }
    operator T() const {
        return mObj;
    }
    bool isNull() const {
        return mObj == NULL;
    }
    void release();
    T detach() {
        T tObj = mObj;
        mObj = NULL;
        return tObj;
    }
    T clone();
    JNIObject<T>& operator = (const JNIObject<T>& rhs) {
        release();
        mHelper = rhs.mHelper;
        mObj = rhs.mObj;
        return *this;
    }
    void print() {
        ALOGD("holding %p", mObj);
    }

private:
    template<typename T2>
    JNIObject(const JNIObject<T2>& rhs);  // NOLINT(implicit)
};

class JNIHelper {
    JavaVM *mVM;
    JNIEnv *mEnv;

public :
    explicit JNIHelper(JavaVM *vm);
    explicit JNIHelper(JNIEnv *env);
    ~JNIHelper();

    /* helpers to deal with static members */
    JNIObject<jbyteArray> newByteArray(int num);
    void setByteArrayRegion(jbyteArray array, int from, int to, const jbyte *bytes);

private:
    /* Jni wrappers */
    friend class JNIObject<jbyteArray>;
    jobject newLocalRef(jobject obj);
    void deleteLocalRef(jobject obj);
};

template<typename T>
JNIObject<T>::JNIObject(JNIHelper &helper, T obj)
      : mHelper(helper), mObj(obj)
{ }

template<typename T>
JNIObject<T>::JNIObject(const JNIObject<T>& rhs)
      : mHelper(rhs.mHelper), mObj(NULL)
{
      mObj = (T)mHelper.newLocalRef(rhs.mObj);
}

template<typename T>
JNIObject<T>::~JNIObject() {
      release();
}

template<typename T>
void JNIObject<T>::release()
{
      if (mObj != NULL) {
          mHelper.deleteLocalRef(mObj);
          mObj = NULL;
      }
}

template<typename T>
T JNIObject<T>::clone()
{
      return mHelper.newLocalRef(mObj);
}
}

#define THROW(env, message)      (env).throwException(message, __LINE__)
