#pragma once

#define MAIN() WINAPI wWinMain(_In_ HINSTANCE, _In_opt_ HINSTANCE, _In_ PWSTR, _In_ INT)
 
class WINDOW
{
public:
    static WINDOW& instance();
    void create(
        LPCWSTR windowTitle = L"pipi",
        int clientWidth = 1270,
        int clientHeight = 720,
        bool fullscreen = false,
        int clientPosX = -1,
        int clientPosY = -1);
    void destroy();
    bool quit();
    static LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp);
    int clientWidth() { return ClientWidth; };
    int clientHeight() { return ClientHeight; };
    HWND hWnd() { return HWnd; }
private:
    LPCWSTR	WindowTitle = L"";
    int ClientWidth = 0;
    int ClientHeight = 0;
    float Aspect = 0.0f;
    HWND HWnd = NULL;
    MSG Msg = {};
    //ƒVƒ“ƒOƒ‹ƒgƒ“
    WINDOW() {};
    WINDOW(const WINDOW&) = delete;
    void operator=(const WINDOW&) = delete;
};