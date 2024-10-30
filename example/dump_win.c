/**
 * @file dump_win.c
 * @author tayne3@dingtalk.com
 * @date 2024.2.6
 */
#include "dump_win.h"

#include "app.h"
#include "mech/ct_log.h"

#ifdef CT_OS_WIN

#include <DbgHelp.h>
#include <tlhelp32.h>

#ifdef _MSC_VER
#pragma comment(lib, "Dbghelp.lib")
#endif

// -------------------------[STATIC DECLARATION]-------------------------

#define TRACE_MAX_STACK_FRAMES         1024
#define TRACE_MAX_FUNCTION_NAME_LENGTH 1024

// Function pointer types for DbgHelp functions
typedef WINBOOL (*SymInitializeFunc)(HANDLE, PCSTR, BOOL);
typedef WINBOOL (*SymCleanupFunc)(HANDLE);
typedef PVOID (*SymFunctionTableAccess64Func)(HANDLE, DWORD64);
typedef DWORD64 (*SymGetModuleBase64Func)(HANDLE, DWORD64);
typedef WINBOOL (*StackWalk64Func)(DWORD, HANDLE, HANDLE, LPSTACKFRAME64, PVOID, PREAD_PROCESS_MEMORY_ROUTINE64,
								   PFUNCTION_TABLE_ACCESS_ROUTINE64, PGET_MODULE_BASE_ROUTINE64,
								   PTRANSLATE_ADDRESS_ROUTINE64);
typedef WINBOOL (*SymFromAddrFunc)(HANDLE, DWORD64, PDWORD64, PSYMBOL_INFO);
typedef WINBOOL (*SymGetLineFromAddr64Func)(HANDLE, DWORD64, PDWORD, PIMAGEHLP_LINE64);

// Global variables to store function pointers
SymInitializeFunc            pSymInitialize;
SymCleanupFunc               pSymCleanup;
SymFunctionTableAccess64Func pSymFunctionTableAccess64;
SymGetModuleBase64Func       pSymGetModuleBase64;
StackWalk64Func              pStackWalk64;
SymFromAddrFunc              pSymFromAddr;
SymGetLineFromAddr64Func     pSymGetLineFromAddr64;

// Critical section for thread-safe output
CRITICAL_SECTION gCriticalSection;

/**
 * @brief 信号处理函数
 * @param sig 接收到的信号
 */
static inline void signal_handler(int sig);

/**
 * @brief 未处理异常过滤器
 * @param exp 异常指针
 * @return LONG 异常处理结果
 */
LONG WINAPI MyUnhandledExceptionFilter(EXCEPTION_POINTERS *exp);

/**
 * @brief 控制台控制处理程序
 * @param CtrlType 控制类型
 * @return WINBOOL 处理结果
 */
BOOL WINAPI MyConsoleCtrlHandler(DWORD CtrlType);

/**
 * @brief 写入迷你转储文件
 * @param hProcess 进程句柄
 * @param ProcessId 进程ID
 * @param hFile 文件句柄
 * @param ExceptionParam 异常信息参数
 * @param CallbackParam 回调信息参数
 */
static inline void miniDumpWriteDump(HANDLE hProcess, DWORD ProcessId, HANDLE hFile,
									 CONST PMINIDUMP_EXCEPTION_INFORMATION ExceptionParam,
									 CONST PMINIDUMP_CALLBACK_INFORMATION  CallbackParam);

/**
 * @brief 迷你转储回调函数
 * @param pParam 回调参数
 * @param input 回调输入
 * @param output 回调输出
 * @return BOOL 回调结果
 */
static inline BOOL CALLBACK MyMiniDumpCallback(PVOID pParam, const PMINIDUMP_CALLBACK_INPUT input,
											   PMINIDUMP_CALLBACK_OUTPUT output);

/**
 * @brief 写入转储文件
 * @param exp 异常指针
 * @param path 转储文件路径
 */
static inline void WriteDump(EXCEPTION_POINTERS *exp, const char *path);

// Function to initialize DbgHelp functions
static inline BOOL InitializeDbgHelp(void);
// Function to print stack trace for a given thread
void print_thread_stack_trace(HANDLE process, HANDLE thread, DWORD threadId);
// Function to print stack traces for all threads
static inline void print_all_thread_stack_traces(void);

// -------------------------[GLOBAL DEFINITION]-------------------------

void print_stack_trace(void) {
	if (!InitializeDbgHelp()) {
		return;
	}

	HANDLE process = GetCurrentProcess();
	pSymInitialize(process, NULL, TRUE);

	InitializeCriticalSection(&gCriticalSection);

	print_all_thread_stack_traces();

	DeleteCriticalSection(&gCriticalSection);

	pSymCleanup(process);
}

void exception_init(void) {
	SetUnhandledExceptionFilter(MyUnhandledExceptionFilter);
	SetConsoleCtrlHandler(MyConsoleCtrlHandler, TRUE);

	signal(SIGINT, signal_handler);
	signal(SIGTERM, signal_handler);
	signal(SIGABRT, signal_handler);
}

// -------------------------[STATIC DEFINITION]-------------------------

static inline void signal_handler(int sig) {
	switch (sig) {
		case SIGINT: gapp_crash(sig, "SIGINT pressed"); break;
		case SIGTERM: gapp_crash(sig, "SIGTERM pressed"); break;
		case SIGABRT: gapp_crash(sig, "SIGABRT pressed"); break;
	}
}

LONG WINAPI MyUnhandledExceptionFilter(EXCEPTION_POINTERS *exp) {
	char             dumpFileName[MAX_PATH];
	const time_t     t  = time(NULL);
	const struct tm *tm = localtime(&t);

	sprintf_s(dumpFileName, MAX_PATH, "crash_%04d%02d%02d_%02d%02d%02d.dmp", tm->tm_year + 1900, tm->tm_mon + 1,
			  tm->tm_mday, tm->tm_hour, tm->tm_min, tm->tm_sec);

	WriteDump(exp, dumpFileName);

	logE("dump file saved to: %s" STR_NEWLINE, dumpFileName);
	global_exit(EXIT_FAILURE, "crash");
	return EXCEPTION_EXECUTE_HANDLER;
}

BOOL WINAPI MyConsoleCtrlHandler(DWORD CtrlType) {
	switch (CtrlType) {
		case CTRL_C_EVENT: global_exit(EXIT_FAILURE, "CTRL+C pressed"); return TRUE;
		case CTRL_BREAK_EVENT: global_exit(EXIT_FAILURE, "CTRL+BREAK pressed"); return TRUE;
		case CTRL_CLOSE_EVENT: global_exit(EXIT_FAILURE, "Window close event"); return TRUE;
	}
	return FALSE;
}

#if defined(__GNUC__) && (__GNUC__ >= 7)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wcast-function-type"
#endif
static inline void miniDumpWriteDump(HANDLE hProcess, DWORD ProcessId, HANDLE hFile,
									 CONST PMINIDUMP_EXCEPTION_INFORMATION ExceptionParam,
									 CONST PMINIDUMP_CALLBACK_INFORMATION  CallbackParam) {
	typedef WINBOOL (*MiniDumpWriteDumpPtr)(HANDLE hProcess, DWORD ProcessId, HANDLE hFile, MINIDUMP_TYPE DumpType,
											CONST PMINIDUMP_EXCEPTION_INFORMATION   ExceptionParam,
											CONST PMINIDUMP_USER_STREAM_INFORMATION UserStreamParam,
											CONST PMINIDUMP_CALLBACK_INFORMATION    CallbackParam);

	HMODULE module = LoadLibraryW(L"Dbghelp.dll");
	if (module) {
		MiniDumpWriteDumpPtr mini_dump_write_dump;
		mini_dump_write_dump = (MiniDumpWriteDumpPtr)GetProcAddress(module, "MiniDumpWriteDump");
		if (mini_dump_write_dump) {
			mini_dump_write_dump(hProcess, ProcessId, hFile, (MINIDUMP_TYPE)80, ExceptionParam, NULL, CallbackParam);
		}
	}
}
#if defined(__GNUC__) && (__GNUC__ >= 7)
#pragma GCC diagnostic pop
#endif

static inline BOOL CALLBACK MyMiniDumpCallback(PVOID pParam, const PMINIDUMP_CALLBACK_INPUT input,
											   PMINIDUMP_CALLBACK_OUTPUT output) {
	ct_unused(pParam);
	if (input == NULL || output == NULL) {
		return FALSE;
	}

	BOOL ret = FALSE;
	switch (input->CallbackType) {
		case IncludeModuleCallback:
		case IncludeThreadCallback:
		case ThreadCallback:
		case ThreadExCallback: ret = TRUE; break;
		case ModuleCallback: {
			if (!(output->ModuleWriteFlags & ModuleReferencedByMemory)) {
				output->ModuleWriteFlags &= ~ModuleWriteModule;
			}
			ret = TRUE;
		} break;
		default: break;
	}
	return ret;
}

static inline void WriteDump(EXCEPTION_POINTERS *exp, const char *path) {
	HANDLE h = CreateFile(path, GENERIC_WRITE | GENERIC_READ, FILE_SHARE_WRITE | FILE_SHARE_READ, NULL, CREATE_ALWAYS,
						  FILE_ATTRIBUTE_NORMAL, NULL);
	MINIDUMP_EXCEPTION_INFORMATION info;
	info.ThreadId          = GetCurrentThreadId();
	info.ExceptionPointers = exp;
	info.ClientPointers    = FALSE;
	MINIDUMP_CALLBACK_INFORMATION mci;
	mci.CallbackRoutine = (MINIDUMP_CALLBACK_ROUTINE)MyMiniDumpCallback;
	mci.CallbackParam   = NULL;
	miniDumpWriteDump(GetCurrentProcess(), GetCurrentProcessId(), h, &info, &mci);
	CloseHandle(h);
}

#if defined(__GNUC__) && (__GNUC__ >= 7)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wcast-function-type"
#endif
// Function to initialize DbgHelp functions
BOOL InitializeDbgHelp(void) {
	HMODULE hDbgHelp = LoadLibraryW(L"Dbghelp.dll");
	if (!hDbgHelp) {
		logE("failed to load Dbghelp.dll\n");
		return FALSE;
	}

	pSymInitialize            = (SymInitializeFunc)GetProcAddress(hDbgHelp, "SymInitialize");
	pSymCleanup               = (SymCleanupFunc)GetProcAddress(hDbgHelp, "SymCleanup");
	pSymFunctionTableAccess64 = (SymFunctionTableAccess64Func)GetProcAddress(hDbgHelp, "SymFunctionTableAccess64");
	pSymGetModuleBase64       = (SymGetModuleBase64Func)GetProcAddress(hDbgHelp, "SymGetModuleBase64");
	pStackWalk64              = (StackWalk64Func)GetProcAddress(hDbgHelp, "StackWalk64");
	pSymFromAddr              = (SymFromAddrFunc)GetProcAddress(hDbgHelp, "SymFromAddr");
	pSymGetLineFromAddr64     = (SymGetLineFromAddr64Func)GetProcAddress(hDbgHelp, "SymGetLineFromAddr64");
	if (!pSymInitialize || !pSymCleanup || !pSymFunctionTableAccess64 || !pSymGetModuleBase64 || !pStackWalk64 ||
		!pSymFromAddr || !pSymGetLineFromAddr64) {
		logE("failed to get function addresses from Dbghelp.dll\n");
		FreeLibrary(hDbgHelp);
		return FALSE;
	}

	return TRUE;
}
#if defined(__GNUC__) && (__GNUC__ >= 7)
#pragma GCC diagnostic pop
#endif

// Function to print stack trace for a given thread
void print_thread_stack_trace(HANDLE process, HANDLE thread, DWORD threadId) {
	CONTEXT context;
	memset(&context, 0, sizeof(CONTEXT));
	context.ContextFlags = CONTEXT_FULL;

	// Get thread context without suspending the thread
	if (!GetThreadContext(thread, &context)) {
		logE("failed to get thread context for thread %lu." STR_NEWLINE, threadId);
		return;
	}

	DWORD        image;
	STACKFRAME64 stackframe;
	ZeroMemory(&stackframe, sizeof(STACKFRAME64));

#ifdef _M_IX86
	image                       = IMAGE_FILE_MACHINE_I386;
	stackframe.AddrPC.Offset    = context.Eip;
	stackframe.AddrPC.Mode      = AddrModeFlat;
	stackframe.AddrFrame.Offset = context.Ebp;
	stackframe.AddrFrame.Mode   = AddrModeFlat;
	stackframe.AddrStack.Offset = context.Esp;
	stackframe.AddrStack.Mode   = AddrModeFlat;
#elif _M_X64
	image                       = IMAGE_FILE_MACHINE_AMD64;
	stackframe.AddrPC.Offset    = context.Rip;
	stackframe.AddrPC.Mode      = AddrModeFlat;
	stackframe.AddrFrame.Offset = context.Rsp;
	stackframe.AddrFrame.Mode   = AddrModeFlat;
	stackframe.AddrStack.Offset = context.Rsp;
	stackframe.AddrStack.Mode   = AddrModeFlat;
#else
#error "Platform not supported!"
#endif

	EnterCriticalSection(&gCriticalSection);
	logE_n("thread %lu [running]:" STR_NEWLINE, threadId);
	for (int i = 0; i < TRACE_MAX_STACK_FRAMES; i++) {
		BOOL result = pStackWalk64(image, process, thread, &stackframe, &context, NULL, pSymFunctionTableAccess64,
								   pSymGetModuleBase64, NULL);

		if (!result) {
			break;
		}

		char         buffer[sizeof(SYMBOL_INFO) + TRACE_MAX_FUNCTION_NAME_LENGTH * sizeof(TCHAR)];
		PSYMBOL_INFO pSymbol  = (PSYMBOL_INFO)buffer;
		pSymbol->SizeOfStruct = sizeof(SYMBOL_INFO);
		pSymbol->MaxNameLen   = TRACE_MAX_FUNCTION_NAME_LENGTH;

		DWORD64         displacement     = 0;
		DWORD           lineDisplacement = 0;
		IMAGEHLP_LINE64 line;
		line.SizeOfStruct = sizeof(IMAGEHLP_LINE64);

		if (pSymFromAddr(process, stackframe.AddrPC.Offset, &displacement, pSymbol)) {
			if (pSymGetLineFromAddr64(process, stackframe.AddrPC.Offset, &lineDisplacement, &line)) {
				logE_n("%s()\n\t%s:%lu +0x%I64x\n", pSymbol->Name, line.FileName, line.LineNumber, displacement);
			} else {
				logE_n("%s()\n\t?? +0x%I64x\n", pSymbol->Name, displacement);
			}
		} else {
			logE_n("\t0x%I64x\n", stackframe.AddrPC.Offset);
		}
	}
	logE_n(STR_NEWLINE);
	LeaveCriticalSection(&gCriticalSection);
}

// Function to print stack traces for all threads
void print_all_thread_stack_traces(void) {
	HANDLE hThreadSnap = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, 0);
	if (hThreadSnap == INVALID_HANDLE_VALUE) {
		logE_n("Failed to create thread snapshot\n");
		return;
	}

	THREADENTRY32 te32;
	te32.dwSize = sizeof(THREADENTRY32);

	if (!Thread32First(hThreadSnap, &te32)) {
		logE_n("Failed to get first thread\n");
		CloseHandle(hThreadSnap);
		return;
	}

	DWORD  currentProcessId = GetCurrentProcessId();
	HANDLE process          = GetCurrentProcess();

	do {
		if (te32.th32OwnerProcessID == currentProcessId) {
			HANDLE hThread = OpenThread(THREAD_ALL_ACCESS, FALSE, te32.th32ThreadID);
			if (hThread) {
				print_thread_stack_trace(process, hThread, te32.th32ThreadID);
				CloseHandle(hThread);
			}
		}
	} while (Thread32Next(hThreadSnap, &te32));

	CloseHandle(hThreadSnap);
}

#endif  // CT_OS_WIN
