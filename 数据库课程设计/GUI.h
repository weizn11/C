#ifndef _GUI_H_
#define _GUI_H_

#include <graphics.h>

class Graph
{
public:
    Graph();
    void Init_GUI();
    void print_1(char *str,int color);
    void print_2(char *str,int color);
    static DWORD WINAPI show_time(LPVOID);
private:
    IMAGE image;
    inline void cover(int x,int y,int width,int height);
};

int GetMouse();

#endif
