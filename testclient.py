import os
import sys
import time
import win32pipe
import win32file
import win32con
import struct

# 命名管道路径
PIPE_NAME = r'\\.\pipe\WindowZBandPipe'

def create_pipe_client():
    # 打开命名管道
    try:
        pipe = win32file.CreateFile(
            PIPE_NAME,
            win32con.GENERIC_READ | win32con.GENERIC_WRITE,
            0,  # 不共享
            None,  # 默认安全性
            win32con.OPEN_EXISTING,
            0,  # 默认属性
            None  # 默认模板
        )
    except Exception as e:
        print(f"无法连接到管道: {e}")
        sys.exit(1)
    return pipe

def send_message(pipe, message):
    # 发送消息
    try:
        # 将消息转换为字节流发送
        win32file.WriteFile(pipe, message.encode('utf-8'))
    except Exception as e:
        print(f"发送消息失败: {e}")
        return False
    return True

def receive_message(pipe):
    # 接收消息
    try:
        hr, data = win32file.ReadFile(pipe, 1024)
        return data.decode('utf-8')
    except Exception as e:
        print(f"接收消息失败: {e}")
        return None

def main():
    # 创建与服务端的管道连接
    pipe = create_pipe_client()

    # 客户端与服务端交互过程
    while True:
        # 客户端发送第一个HWND消息
        hwnd_msg = "397426"  # 假设是一个HWND的标识符
        if not send_message(pipe, hwnd_msg):
            break

        # 服务端返回 "DONE"
        response = receive_message(pipe)
        if response != "DONE":
            print("接收到的响应不是'DONE'")
            break

        # 客户端发送第二个HWND消息
        hwnd_msg = "NULL"
        if not send_message(pipe, hwnd_msg):
            break

        # 服务端再次返回 "DONE"
        response = receive_message(pipe)
        if response != "DONE":
            print("接收到的响应不是'DONE'")
            break

        # 客户端发送DWORD消息
        dword_msg = struct.pack("I", 2)  # 假设发送一个DWORD类型的整数
        if not send_message(pipe, dword_msg):
            break

        # 服务端返回FIN
        response = receive_message(pipe)
        print(response)

        # 客户端退出
        print("客户端交互完成")
        break

    # 关闭管道
    win32file.CloseHandle(pipe)

if __name__ == "__main__":
    main()
