#include <stdio.h>
#include <stdlib.h>
#include <Python.h>
#include <Windows.h>
#include <WinUser.h>
// #include <hook.h>
#pragma comment(lib, "user32.lib") // 真正实现SetWindowsHookEx的方法

/*
    windows消息队列:
        鼠标/键盘事件  ---> 系统消息队列  ---->   [hook1,hook2....](钩子链)  ----> 应用程序消息队列
*/

// 钩子
static HHOOK mouse_hook = NULL;
static HHOOK keyboard_hook = NULL;
static PyObject *mouse_hook_cb = NULL;
static PyObject *keyboard_hook_cb = NULL;


// mouse hook callback
LRESULT CALLBACK KeyBoardProc(int nCode, WPARAM wParam, LPARAM lParam)
{
    /* ll
        typedef struct tagKBDLLHOOKSTRUCT {
        DWORD     vkCode;		// 按键代号
        DWORD     scanCode;		// 硬件扫描代号，同 vkCode 也可以作为按键的代号。
        DWORD     flags;		// 事件类型，一般按键按下为 0 抬起为 128。
        DWORD     time;			// 消息时间戳
        ULONG_PTR dwExtraInfo;	// 消息附加信息，一般为 0。
        }KBDLLHOOKSTRUCT,*LPKBDLLHOOKSTRUCT,*PKBDLLHOOKSTRUCT;
    */
    KBDLLHOOKSTRUCT *ks = (KBDLLHOOKSTRUCT *)lParam;
    // printf("get keyboard event :%I64i -- %i \n",wParam,ks.vkCode);
    
    // 调用PYTHON 回调函数处理
    PyGILState_STATE gstate;
    BOOL ignore = FALSE;
    PyObject *arglist = NULL;
    gstate = PyGILState_Ensure();
    if (!PyCallable_Check(keyboard_hook_cb))
    {
        printf("hook callback is not callable");
    }
    else
    {
        // parse argument list
        arglist = Py_BuildValue("(ll)", wParam,ks->vkCode);
        // printf("call mouse hook callback \n");
        ignore = PyObject_IsTrue(PyObject_CallObject(keyboard_hook_cb, arglist));
        // PyObject_Print(ignore,stdout, 0);
    }
    Py_DECREF(arglist);
    PyGILState_Release(gstate);
    if (!ignore)
    {
        return CallNextHookEx(NULL, nCode, wParam, lParam);
    }
    else
    {   
        printf("ignore this keyboard event");
        return 1;
    }
}

// mouse hook处理函数
LRESULT CALLBACK MouseProc(int nCode, WPARAM wParam, LPARAM lParam)
{
    /*
    typedef struct tagMOUSEHOOKSTRUCT {
        POINT   pt;					// Point数据
        HWND    hwnd;				// 接收鼠标消息的窗体的句柄
        UINT    wHitTestCode;		// 指定点击测试值
        ULONG_PTR dwExtraInfo;		// 指定和该消息相关联的附加信息。
    } MOUSEHOOKSTRUCT, FAR* LPMOUSEHOOKSTRUCT, * PMOUSEHOOKSTRUCT;
    */

    MOUSEHOOKSTRUCT *ms = (MOUSEHOOKSTRUCT *)lParam;
    // printf("get mouse event :%I64i \n",wParam);

    // 调用PYTHON 回调函数处理
    PyGILState_STATE gstate;
    BOOL ignore = FALSE;
    PyObject *arglist = NULL;
    gstate = PyGILState_Ensure();
    if (!PyCallable_Check(mouse_hook_cb))
    {
        printf("hook callback is not callable");
    }
    else
    {
        // parse argument list
        arglist = Py_BuildValue("(l)", wParam);
        ignore = PyObject_IsTrue(PyObject_CallObject(mouse_hook_cb, arglist));
        // PyObject_Print(ignore,stdout, 0);
    }
    Py_DECREF(arglist);
    PyGILState_Release(gstate);
    if (!ignore)
    {
        return CallNextHookEx(NULL, nCode, wParam, lParam);
    }
    else
    {   
        printf("ignore this mouse event");
        return 1;
    }
}

// python函数作为回调参数
static PyObject *AddMouseHookCallbackFunc(PyObject *self, PyObject *args)
{
    // static PyObject *callback = NULL;
    if (PyArg_ParseTuple(args, "O:set_callback", &mouse_hook_cb))
    {
        if (!PyCallable_Check(mouse_hook_cb))
        {
            PyErr_SetString(PyExc_TypeError, "param is not callable");
            return NULL;
        }
        else
        {
            printf("add mouse hook callback success");
            // PyObject_CallFunction(mouse_hook_cb, NULL);
            return PyBool_FromLong(1);
        }
    }
    return PyBool_FromLong(1);
}

// python函数作为回调参数
static PyObject *AddKeyBoardCallbackFunc(PyObject *self, PyObject *args)
{
    // static PyObject *callback = NULL;
    if (PyArg_ParseTuple(args, "O:set_callback", &keyboard_hook_cb))
    {
        if (!PyCallable_Check(keyboard_hook_cb))
        {
            PyErr_SetString(PyExc_TypeError, "param is not callable");
            return NULL;
        }
        else
        {
            printf("add keyboard callback success");
            // PyObject_CallFunction(mouse_hook_cb, NULL);
            return PyBool_FromLong(1);
        }
    }
    return PyBool_FromLong(1);
}

static PyObject *InstallMouseHook(PyObject *self,PyObject *args)
{
    if (mouse_hook_cb == NULL)
    {
        PyErr_SetString(PyExc_TypeError, "please add mouse hook callback first!");
        return NULL;
    }
    HINSTANCE hk = GetModuleHandle(NULL);                               // NULL则会返回当前进程ID
    mouse_hook = SetWindowsHookEx(WH_MOUSE_LL, MouseProc, hk, 0); // WH_MOUSE_LL表示全局键盘钩子
    return PyBool_FromLong(1);
}

static PyObject *InstallKeyBoardHook(PyObject *self,PyObject *args)
{   
    // printf("thread id in install func:%i",GetCurrentThreadId());
    if (keyboard_hook_cb == NULL)
    {
        PyErr_SetString(PyExc_TypeError, "please add keyboard hook callback first!");
        return NULL;
    }
    HINSTANCE hk = GetModuleHandle(NULL);                                  // NULL则会返回当前进程ID
    keyboard_hook = SetWindowsHookEx(WH_KEYBOARD_LL, KeyBoardProc, hk, 0); // WH_KEYBOARD_LL表示全局键盘钩子
    return PyBool_FromLong(1);
}

// start hook
static PyObject *start(PyObject *self,PyObject *args)
{
    // 关于C-EXTENSION 中的GIL问题
    //  https://stackoverflow.com/questions/42006337/python-c-api-is-it-thread-safe
    //  https://www.cnblogs.com/traditional/p/13289905.html
    //  https://stackoverflow.com/questions/15470367/pyeval-initthreads-in-python-3-how-when-to-call-it-the-saga-continues-ad-naus
    // https://github.com/zpoint/CPython-Internals/blob/master/Interpreter/gil/gil.md
    Py_BEGIN_ALLOW_THREADS; // 去掉GIL限制，这里如果想访问任务PYTHON 相关API，必须重新获取 GIL
    MSG msg;
    BOOL bRet;

    while (bRet = GetMessageW(&msg, NULL, 0, 0) != 0)
    {   
        if (bRet == -1)
        {
            // handle the error and possibly exit
            printf("error ---> getMessage error");
        }
        else
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }
    Py_END_ALLOW_THREADS;
    UnhookWindowsHookEx(mouse_hook);
    return PyBool_FromLong(1);
}

// stop hook
static PyObject *stop(PyObject *self,PyObject *args){
    // HINSTANCE hk = GetModuleHandle(NULL); 
    DWORD t_id;
    if (mouse_hook == NULL && keyboard_hook ==NULL)
    {
        PyErr_SetString(PyExc_TypeError, "please START HOOK first!");
        return NULL;
    } 
    if (PyArg_ParseTuple(args, "i", &t_id))
    {
        // printf("stop getMessage in thread:%i",t_id);
        PostThreadMessageW(t_id,WM_QUIT,0,0);
        return Py_True;
    }else{
        PyErr_SetString(PyExc_ValueError, "thread id can not be None");
        return NULL;
    }
}

// e-extension相关的初始函数，包括c-extension里面对应函数和模块信息
// 方法信息，可以包含多个方法
static PyMethodDef HookMethods[] = {
    {
        "install_mouse_hook",  // PYthon调用时对应的方法
        InstallMouseHook,      // 对应的C-EXTENSION里面的方法
        METH_NOARGS,           // 标记,告诉PYTHON解释器这个方法无位置参数
        "install mouse hook"}, // 函数说明
    {
        "install_keyboard_hook",  // PYthon调用时对应的方法
        InstallKeyBoardHook,      // 对应的C-EXTENSION里面的方法
        METH_NOARGS,           // 标记,告诉PYTHON解释器这个方法无位置参数
        "install keyboard hook"}, // 函数说明        
    {
        "add_mouse_hook_cb",                 // PYthon调用时对应的方法
        AddMouseHookCallbackFunc,            // 对应的C-EXTENSION里面的方法
        METH_VARARGS,                        // 标记,告诉PYTHON解释器有位置参数
        "add mouse hook callback function"}, // 函数说明
    {
        "add_keyboard_hook_cb",                 // PYthon调用时对应的方法
        AddKeyBoardCallbackFunc,            // 对应的C-EXTENSION里面的方法
        METH_VARARGS,                        // 标记,告诉PYTHON解释器有位置参数
        "add keyboard hook callback function"}, // 函数说明
    {
        "start",             // PYthon调用时对应的方法
        start,               // 对应的C-EXTENSION里面的方法
        METH_NOARGS,         // 标记,告诉PYTHON解释器这个方法无位置参数
        "start  hook"}, // 函数说明
    {
        "stop",             // PYthon调用时对应的方法
        stop,               // 对应的C-EXTENSION里面的方法
        METH_VARARGS,         // 标记,告诉PYTHON解释器这个方法无位置参数
        "stop  hook"}, // 函数说明
    {NULL, NULL, 0, NULL}                    // 哨兵, 一定要加上这个，否则pyd无法正常导入
};

// module信息
static struct PyModuleDef HookModule = {
    PyModuleDef_HEAD_INIT,
    "hook",                            // module name
    "python hook api from c extension", // module docstring
    -1,                                 // ？
    HookMethods                         // 该module对应的方法列表
};

// PyMODINIT_FUNC：python import时运行的方法 PyInit_{{module name}}
PyMODINIT_FUNC PyInit_hook(void)
{
    PyObject *module = PyModule_Create(&HookModule);
    return module;
}
