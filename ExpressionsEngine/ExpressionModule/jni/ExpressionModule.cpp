// Write C++ code here.
//
// Do not forget to dynamically load the C++ library into your application.
//
// For instance,
//
// In MainActivity.java:
//    static {
//       System.loadLibrary("ExpressionModule");
//    }
//
// Or, in MainActivity.kt:
//    companion object {
//      init {
//         System.loadLibrary("ExpressionModule")
//      }
//    }
#include <jni.h>
#include "cstring"
#include "Source/expressions.h"

using namespace std;
extern "C"{
    const char* PrintHello(char* str){
        return str;
    }

    string HelloCPPString(){
        return "STD Hello";
    }

    float SimpleMultiplication(float num1, float num2){
        return num1 * num2;
    }

    int AddTwoIntegers(int a, int b) {
        return a + b;
    }

    float AddTwoFloats(float a, float b) {
        return a + b;
    }

    int ExpressionModuleInitializer(int input){
        ExpEvaluator evaluator;
        evaluator.insert_value("a1", input);
        evaluator.insert_expression("a", "a1+1");

        std::map<std::string, double> result = evaluator.evaluate();
        return int(evaluator.get_result("a"));
    }
}

extern "C"
JNIEXPORT jstring JNICALL
Java_com_byjus_testapp_MainActivity_JavaCallJNI(JNIEnv *env, jobject thiz) {
    // TODO: implement JavaCallJNI()
    PrintHello("abskdbaks");
}