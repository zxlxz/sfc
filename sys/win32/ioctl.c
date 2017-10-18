#include <windows.h>
#include <wincon.h>

struct winsize
{
    unsigned short int ws_row;
    unsigned short int ws_col;
};

int _ioctl_winsize(int fd, struct winsize* w) {
    (void)fd;

    static int      has_init = 0;
    static HANDLE   console_handle;

    if (has_init == 0) {
        has_init = 1;
        console_handle = GetStdHandle(STD_OUTPUT_HANDLE);
    }

    CONSOLE_SCREEN_BUFFER_INFO info;
    BOOL ret = GetConsoleScreenBufferInfo(console_handle, &info);

    if (ret == FALSE) {
        return -1;
    }

    w->ws_col = info.dwSize.X;
    w->ws_row = info.dwSize.Y;
    return 0;
}
