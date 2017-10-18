#include <windows.h>

void _init_tty(int fd) {
    /* see https://docs.microsoft.com/en-us/windows/console/setconsolemode */

    DWORD  stdhandle = fd == 1 ? STD_OUTPUT_HANDLE : fd == 2 ? STD_ERROR_HANDLE : 0;
    if (stdhandle == 0) {
        return;
    }

    HANDLE handle    = GetStdHandle(fd == 1 ? STD_OUTPUT_HANDLE : STD_ERROR_HANDLE);
    if (handle != INVALID_HANDLE_VALUE) {
        DWORD mode;
        if (GetConsoleMode(handle, &mode) == TRUE) {
            // ENABLE_VIRTUAL_TERMINAL_PROCESSING   => 0x0004
            // ENABLE_LVB_GRID_WORLDWIDE            => 0x0010
            mode |= 0x0014;
            SetConsoleMode(handle, mode);
        }
    }
}
