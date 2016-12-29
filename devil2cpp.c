#include <dlfcn.h>
#include <jni.h>
#include <pthread.h>
#include <unistd.h>

#define DEBUG_TAG "DEVIL"
#define INFO_TAG "L2CPP"
#define ANDROID_LOG_INFO 4

#define logDebug(...) \
	if(__android_log_print != 0) \
		__android_log_print(ANDROID_LOG_INFO, DEBUG_TAG, ##__VA_ARGS__);
#define logInfo(...) \
	if(__android_log_print != 0) \
		__android_log_print(ANDROID_LOG_INFO, INFO_TAG, ##__VA_ARGS__);

void* libLogHandlers = 0;
void* libRealUnityHandlers = 0;
int injected = 0;

typedef int(*__android_log_print_t)(int prio, const char *tag, const char *fmt, ...);
__android_log_print_t __android_log_print = 0;

typedef void(*UnitySendMessage_t)(const char* ob, const char* method, const char* msg);
UnitySendMessage_t realUnitySendMessage = 0;

typedef jint(JNICALL* realJNI_OnLoad_t)(JavaVM* vm, void* reserved);
realJNI_OnLoad_t realJNIOnLoad = 0;

typedef void(JNICALL* realJNI_OnUnload_t)(JavaVM* vm, void* reserved);
realJNI_OnUnload_t realJNIOnUnload = 0;

void loadLibLogHandler() {
	if(libLogHandlers == 0)
		libLogHandlers = dlopen("liblog.so", RTLD_NOW);
	if(__android_log_print == 0)
		__android_log_print = dlsym(libLogHandlers, "__android_log_print");
}

void* main_thread(void * arg)
{
	sleep(20);
	injected = 1;
	loadLibLogHandler();
	logInfo("injected");
}

void start_main()
{
	pthread_t thrd;
	pthread_create(&thrd, 0, main_thread, 0);
}

void CallUSM(const char* ob, const char* method, const char* msg)
{
	if (!libRealUnityHandlers)
		libRealUnityHandlers = dlopen("librealunity.so", RTLD_NOW);
	if (!realUnitySendMessage)
		realUnitySendMessage = dlsym(libRealUnityHandlers, "UnitySendMessage");
	realUnitySendMessage(ob, method, msg);
}

void UnitySendMessage(const char* ob, const char* method, const char* msg)
{
	loadLibLogHandler();

	if (!injected)
		start_main();
	CallUSM(ob, method, msg);
}

JNIEXPORT jint JNICALL callJNIOL(JavaVM* vm, void* reserved)
{
	if (!libRealUnityHandlers)
		libRealUnityHandlers = dlopen("librealunity.so", RTLD_NOW);
	if (!realJNIOnLoad)
		realJNIOnLoad = dlsym(libRealUnityHandlers, "JNI_OnLoad");
	return realJNIOnLoad(vm, reserved);
}

JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM* vm, void* reserved)
{
	loadLibLogHandler();
	logDebug("JNI_OnLoad");

	if (!injected)
		start_main();
	return callJNIOL(vm, reserved);
}

JNIEXPORT void JNICALL callJNIUL(JavaVM* vm, void* reserved)
{
	if (!libRealUnityHandlers)
		libRealUnityHandlers = dlopen("librealunity.so", RTLD_NOW);
	if (!realJNIOnUnload)
		realJNIOnUnload = dlsym(libRealUnityHandlers, "JNI_OnUnload");
	return realJNIOnUnload(vm, reserved);
}

JNIEXPORT void JNICALL JNI_OnUnload(JavaVM* vm, void* reserved)
{
	loadLibLogHandler();
	logDebug("JNI_OnUnload");

	if (!injected)
		start_main();
	callJNIUL(vm, reserved);
}
