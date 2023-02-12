# py-simple-win-hook
一个简单的,即插即用window 鼠标键盘消息监听脚本.钩子注入部分基于CPYTHON-API,python这边只需要提供回调函数.


## 使用方法

- 1. 编译`hook.c`文件.编译成PYD格式即可直接引用.编译参考:[compile2pyd](https://docs.python.org/3/extending/building.html)
- 2. 直接在py里面写好回调,启动即可,具体参考[demo.py](demo.py)

## 关于回调的返回值
- 1. 当回调函数的返回值为True时.windows会结束这次鼠标/键盘事件.不会再去继续调用windows的鼠标/键盘事件响应事件链.当返回值为False时,则会继续往下调用钩子链，和windows自带的鼠标/键盘事件处理.比如写了个键盘的会调函数，启动钩子后,当输入char,如果返回True.则这次输入会被忽略.返回False.跟正常输入一致.可参考[windows-api-callnexthookex](https://learn.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-callnexthookex)
- 2. 回调函数的参数:
    a:鼠标回调函数:`mouse_event`:鼠标事件对应的windows码,具体可参考:[windows_mouse_event](https://learn.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-mouse_event)
    b:键盘回调函数:`kb_virtual_code`:按下的键盘码.`key_event`:键盘事件:按下/松开。具体参考[win_keyboard_event](https://learn.microsoft.com/en-us/windows/apps/design/input/keyboard-events). [win_keyboard_code](https://learn.microsoft.com/en-us/windows/win32/inputdev/virtual-key-codes)
