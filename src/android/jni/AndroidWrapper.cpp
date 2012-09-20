#include "AndroidWrapper.h"

#include <stdint.h>

AndroidWrapper* AndroidWrapper::Instance = NULL;

AndroidWrapper::AndroidWrapper(JNIEnv* jniEnv) :
    mJNIEnvironment(jniEnv)
{
    const char* strDSActivity = "com/bigyama/dancingsquid/DSActivity";
    mDSActivityClass = jniEnv->FindClass(strDSActivity);

    mJavaMethod_OpenAsset = jniEnv->GetStaticMethodID
    (
        mDSActivityClass,
        "open_asset",
        "(Ljava/lang/String;)Z"
    );

    mJavaMethod_OpenAsset = jniEnv->GetStaticMethodID
    (
        mDSActivityClass,
        "does_asset_exist",
        "(Ljava/lang/String;)Z"
    );
}

bool AndroidWrapper::OpenAsset(const char* name)
{
    jstring jStrPath = mJNIEnvironment->NewStringUTF(name);
    bool result = mJNIEnvironment->CallStaticBooleanMethod
    (
        mDSActivityClass,
        mJavaMethod_OpenAsset,
        jStrPath
    );
    mJNIEnvironment->DeleteLocalRef(jStrPath);
    return result;
}

bool AndroidWrapper::DoesAssetExist(const char* name)
{
    jstring jStrPath = mJNIEnvironment->NewStringUTF(name);
    bool result = mJNIEnvironment->CallStaticBooleanMethod
    (
        mDSActivityClass,
        mJavaMethod_DoesAssetExist,
        jStrPath
    );
    mJNIEnvironment->DeleteLocalRef(jStrPath);
    return result;
}

