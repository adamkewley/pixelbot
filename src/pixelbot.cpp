#include <Windows.h>

#include <iostream>
#include <unordered_set>
#include <thread>

namespace {
    struct Rect {

        struct {
            float x, y;
        } origin;

        struct {
            float width, height;
        } size;
    };

    struct WinFile final {
        HANDLE h;

        WinFile(HANDLE h_) : h{h_} {}
        WinFile(WinFile const&) = delete;
        WinFile(WinFile&& tmp) : h{tmp.h} { h = NULL; }
        WinFile& operator=(WinFile const&) = delete;
        WinFile& operator=(WinFile&& tmp) { std::swap(h, tmp.h); return *this; }
        ~WinFile() noexcept { if (h) { CloseHandle(h); } }

        [[nodiscard]] constexpr HANDLE get() noexcept { return h; }
    };

    [[nodiscard]] WinFile OpenSerial(int port) {
        char portname[64];
        std::snprintf(portname, sizeof(portname), "\\\\.\\COM%i", port);

    }
}



void doSerial() {

    HANDLE serialHandle = CreateFile("\\\\.\\COM4", GENERIC_WRITE, 0, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);

    // Do some basic settings
    DCB serialParams{};
    serialParams.DCBlength = sizeof(serialParams);

    GetCommState(serialHandle, &serialParams);
    serialParams.BaudRate = 9600;
    serialParams.ByteSize = 8;
    serialParams.StopBits = ONE5STOPBITS;
    serialParams.Parity = NOPARITY;
    SetCommState(serialHandle, &serialParams);

    // Set timeouts
    COMMTIMEOUTS timeout{};
    timeout.ReadIntervalTimeout = 50;
    timeout.ReadTotalTimeoutConstant = 50;
    timeout.ReadTotalTimeoutMultiplier = 50;
    timeout.WriteTotalTimeoutConstant = 50;
    timeout.WriteTotalTimeoutMultiplier = 10;
    SetCommTimeouts(serialHandle, &timeout);

    uint8_t buf[64] = {0xde, 0xad, 0xbe, 0xef};

    DWORD written = 0;
    if (!WriteFile(serialHandle, buf, 4, &written, NULL)) {
        std::cerr << "err" << std::endl;
    }
}

int main(int, char**) {
    doSerial();
    return 0;

    // from https://github.com/octalmage/robotjs/blob/master/src/screengrab.c

    Rect rect;
    rect.size.height = 1;
    rect.size.width = 2;
    rect.origin.x = 0;
    rect.origin.y = 0;

    void *data;
    HDC screen = NULL, screenMem = NULL;
    HBITMAP dib;
    BITMAPINFO bi;

    /* Initialize bitmap info. */
    bi.bmiHeader.biSize = sizeof(bi.bmiHeader);
    bi.bmiHeader.biWidth = (long)rect.size.width;
    bi.bmiHeader.biHeight = -(long)rect.size.height; /* Non-cartesian, please */
    bi.bmiHeader.biPlanes = 1;
    bi.bmiHeader.biBitCount = 32;
    bi.bmiHeader.biCompression = BI_RGB;
    bi.bmiHeader.biSizeImage = (DWORD)(4 * rect.size.width * rect.size.height);
    bi.bmiHeader.biXPelsPerMeter = 0;
    bi.bmiHeader.biYPelsPerMeter = 0;
    bi.bmiHeader.biClrUsed = 0;
    bi.bmiHeader.biClrImportant = 0;

    //HWND hwnd = FindWindow(NULL, "World of Warcraft");


    screen = GetDC(NULL); /* Get entire screen */
    if (screen == NULL) return NULL;

    /* Get screen data in display device context. */
    dib = CreateDIBSection(screen, &bi, DIB_RGB_COLORS, &data, NULL, 0);

    screenMem = CreateCompatibleDC(screen); // == NULL on err
    SelectObject(screenMem, dib); // == NULL on err

    while (true) {
        /* Copy the data into a bitmap struct. */
        if (!BitBlt(screenMem,
                    (int)0,
                    (int)0,
                    (int)rect.size.width,
                    (int)rect.size.height,
                    screen,
                    rect.origin.x,
                    rect.origin.y,
                    SRCCOPY)) {

            /* Error copying data. */
            ReleaseDC(NULL, screen);
            DeleteObject(dib);
            if (screenMem != NULL) DeleteDC(screenMem);

            return NULL;
        }


        uint32_t buf[2];
        memcpy(buf, data, bi.bmiHeader.biBitCount/8 * rect.size.width * rect.size.height);
        std::cerr << std::hex << buf[0] << ' ' << buf[1] << '\n';
        std::this_thread::sleep_for(std::chrono::seconds{1});
    }






    ReleaseDC(NULL, screen);
    DeleteObject(dib);
    DeleteDC(screenMem);

    return 0;
}
