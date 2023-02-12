
import threading
import hook 
import time


def keyboard_callback(key_event:int,kb_virtual_code:int):
    """
        kb_virtual_code:按下键盘对应的键盘码.参考 windows api
        return:
            True:结束这次事件
            False:屏蔽这次键盘事件
    
    """
    if key_event ==256:
        print(f"键盘码为:{kb_virtual_code}的建被按下了")
    elif key_event ==257:
        print(f"键盘码为:{kb_virtual_code}的建被松开了")

    return False

def mouse_callback(mouse_event:int):
    """
        mouse_event:按下键盘对应的事件.参考 windows api
        return:
            True:结束这次事件·
            False:屏蔽这次鼠标事件
    s
    """
    print(f"鼠标事件 --> {mouse_event}")
    return False

def stop_hook(hook_running_thread_id=None):
    assert hook_running_thread_id is not None,"hook running thread id can not be None"
    print("stop hook....")
    hook.stop(hook_running_thread_id)
    
def start():
    hook.add_keyboard_hook_cb(keyboard_callback)
    hook.install_keyboard_hook()
    hook.add_mouse_hook_cb(mouse_callback)
    hook.install_mouse_hook()
    hook.start()

if __name__ =="__main__":

    t = threading.Thread(target=start,args=())
    t.daemon=True
    t.start()
    time.sleep(5)
    stop_hook(t.native_id)
    time.sleep(5)
# for i in range(1):
#     # print("2222222ssssssss")
#     time.sleep(1)


