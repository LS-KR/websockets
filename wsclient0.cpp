#include "easywsclient.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <thread>
#include <vector>
#include <iostream>
#include <chrono>
#include <atomic>
#include <functional>
#include <windows.h>

using easywsclient::WebSocket;
static WebSocket::pointer ws = NULL;

void handle_message(const std::string& message)
{
    std::cout << "Got message: " << message << std::endl;
}

std::string url;

int main(int argc, char** argv)
{
    void ReConnect();
    std::string message;
    //std::cout << "Enter url: ws://";
    //std::cin >> url;
    //ws = WebSocket::from_url(("ws://" + url).c_str());
    ws = WebSocket::from_url("ws://localhost:9766");
    assert(ws);
    std::thread t = std::thread(ReConnect);
    t.join();
    do
    {
        std::cout << ">>>";
        std::cin >> message;
        ws->send(message);
        while (ws->getReadyState() != WebSocket::CLOSED)
        {
            ws->poll();
            ws->dispatch(handle_message);
        }
    } while (message != "\\exit");
    delete ws;
    return 0;
}

//线程thread  维护重连连接
void ReConnect()
{
    bool conn = false;
    //ws = WebSocket::from_url(("ws://" + url).c_str());
    ws = WebSocket::from_url("ws://localhost:9766");

    //如果你需要多线程，可以在一个thread 维护该ws的连接重连机制
    while (1) //判断ws是否正常连接
    {
        if (ws != NULL)
        {
            ws->poll(0);//这个必须要调用，否则不能发送，发送跟接收都是异步的，都是在这个poll函数里监测处    理的
            ws->dispatch(handle_message);
            if (ws->getReadyState() == WebSocket::CLOSED)
            {
                //ws连接断开 重连
                delete ws;
                ws = NULL;
                ws = WebSocket::from_url("ws://localhost:8126/foo");
            }
            else if (ws->getReadyState() == WebSocket::OPEN)
            {
                //ws连接ok
                //    ws->send("goodbye");
                ws->send("hello");
            }

        }
        else
        {
            ws = WebSocket::from_url("ws://localhost:8126/foo");
            Sleep(1);
        }
        Sleep(100);
    }
    if (ws != NULL)
        delete ws;
}