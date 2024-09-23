/* DO NOT EDIT THIS FILE - it is machine generated */
#include <jni.h>
/* Header for class com_acces_aiousb_CounterSubsystem */

#ifndef _Included_com_acces_aiousb_CounterSubsystem
#define _Included_com_acces_aiousb_CounterSubsystem
#ifdef __cplusplus
extern "C" {
#endif
#undef com_acces_aiousb_CounterSubsystem_COUNTERS_PER_BLOCK
#define com_acces_aiousb_CounterSubsystem_COUNTERS_PER_BLOCK 3L
/*
 * Class:     com_acces_aiousb_CounterSubsystem
 * Method:    getNumCounterBlocks
 * Signature: (I)I
 */
JNIEXPORT jint JNICALL Java_com_acces_aiousb_CounterSubsystem_getNumCounterBlocks
  (JNIEnv *, jobject, jint);

/*
 * Class:     com_acces_aiousb_CounterSubsystem
 * Method:    readLatched
 * Signature: (I[C)I
 */
JNIEXPORT jint JNICALL Java_com_acces_aiousb_CounterSubsystem_readLatched
  (JNIEnv *, jobject, jint, jcharArray);

/*
 * Class:     com_acces_aiousb_CounterSubsystem
 * Method:    readAll
 * Signature: (I[C)I
 */
JNIEXPORT jint JNICALL Java_com_acces_aiousb_CounterSubsystem_readAll
  (JNIEnv *, jobject, jint, jcharArray);

/*
 * Class:     com_acces_aiousb_CounterSubsystem
 * Method:    selectGate
 * Signature: (II)I
 */
JNIEXPORT jint JNICALL Java_com_acces_aiousb_CounterSubsystem_selectGate
  (JNIEnv *, jobject, jint, jint);

/*
 * Class:     com_acces_aiousb_CounterSubsystem
 * Method:    startClock
 * Signature: (II[D)I
 */
JNIEXPORT jint JNICALL Java_com_acces_aiousb_CounterSubsystem_startClock
  (JNIEnv *, jobject, jint, jint, jdoubleArray);

#ifdef __cplusplus
}
#endif
#endif
