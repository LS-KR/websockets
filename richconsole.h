
#ifndef RICHCONSOLE_H
#define RICHCONSOLE_H

#include <iostream>
#include <string>
#include <vector>
#include <cmath>
#include <cstdlib>

/*
字背景颜色范围: 40--49           字颜色: 30--39
40: 黑                          30: 黑
41: 红                          31: 红
42: 绿                          32: 绿
43: 黄                          33: 黄
44: 蓝                          34: 蓝
45: 紫                          35: 紫
46: 深绿                        36: 深绿
47: 白色                        37: 白色
ANSI控制码:
QUOTE:
\033[0m  关闭所有属性 
\033[1m  设置高亮度 
\03[4m  下划线 
\033[5m  闪烁 
\033[7m  反显 
\033[8m  消隐 
\033[30m  --  \033[37m  设置前景色 
\033[40m  --  \033[47m  设置背景色 
\033[nA  光标上移n行 
\03[nB  光标下移n行 
\033[nC  光标右移n行 
\033[nD  光标左移n行 
\033[y;xH设置光标位置 
\033[2J  清屏 
\033[K  清除从光标到行尾的内容 
\033[s  保存光标位置 
\033[u  恢复光标位置 
\033[?25l  隐藏光标 

\33[?25h  显示光标
*/

enum TextColor {
    BLACK = 40,
    RED = 41,
    GREEN = 42,
    YELLOW = 43,
    BLUE = 44,
    PURPLE = 45,
    DARK_GREEN = 46,
    WHITE = 47
};

enum BackGroundColor {
    BLACK_BACKGROUND = 30,
    RED_BACKGROUND = 31,
    GREEN_BACKGROUND = 32,
    YELLOW_BACKGROUND = 33,
    BLUE_BACKGROUND = 34,
    PURPLE_BACKGROUND = 35,
    DARK_GREEN_BACKGROUND = 36,
    WHITE_BACKGROUND = 37
};

class RichConsole
{
    public:
        void Print(std::string str);
        void Print(std::string str, TextColor textColor, BackGroundColor backGroundColor);
        void CloseAllControl(void);
        void SetCursorPosition(int x, int y);
        void HideCursor(void);
        void ShowCursor(void);
        void ClearScreen(void);
        void ClearLine(void);
        void SetColor(TextColor textColor, BackGroundColor backGroundColor);
    private:
        
    protected:
        const std::string bgcolor[8] = { "\033[40m", "\033[41m", "\033[42m", "\033[43m", "\033[44m", "\033[45m", "\033[46m", "\033[47m"};
        const std::string ttcolor[8] = { "\033[30m", "\033[31m", "\033[32m", "\033[33m", "\033[34m", "\033[35m", "\033[36m", "\033[37m"};
};

void RichConsole::Print(std::string str)
{
    std::cout << str;
}

void RichConsole::Print(std::string str, TextColor textColor, BackGroundColor backGroundColor)
{
    std::cout << bgcolor[(int)backGroundColor - 40] << ttcolor[(int)textColor - 30] << str << "\033[0m";
}

void RichConsole::CloseAllControl(void)
{
    std::cout << "\033[0m";
}

void RichConsole::SetCursorPosition(int x, int y)
{
    std::cout << "\033[" << y << ";" << x << "H";
}

void RichConsole::HideCursor(void)
{
    std::cout << "\033[?25l";
}

void RichConsole::ShowCursor(void)
{
    std::cout << "\033[?25h";
}

void RichConsole::ClearScreen(void)
{
    std::cout << "\033[2J";
}

void RichConsole::ClearLine(void)
{
    std::cout << "\033[K";
}

void RichConsole::SetColor(TextColor textColor, BackGroundColor backGroundColor)
{
    std::cout << bgcolor[(int)backGroundColor - 40] << ttcolor[(int)textColor - 30];
}

#endif // RICHCONSOLE_H
