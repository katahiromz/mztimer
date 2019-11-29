// MZ Timer --- Stopwatch and Timer by katahiromz
// License: MIT
#include <windows.h>
#include <windowsx.h>
#include <commctrl.h>
#include <tchar.h>
#include <stdlib.h>

#define BLACK_COLOR     RGB(0, 0, 0)
#define RED_COLOR       RGB(255, 96, 96)

HINSTANCE g_hInstance;              // the main instance
HWND g_hMainWnd;                    // the main window handle

static LARGE_INTEGER s_freq;        // performance counter frequency
static LARGE_INTEGER s_now;         // performance counter of now
static LARGE_INTEGER s_start;       // performance counter of starting point
static LARGE_INTEGER s_stop;        // performance counter of ending point
static LARGE_INTEGER s_deltamsec;   // time amount in milliseconds

static BOOL s_fRunning = TRUE;      // is it running?
static BOOL s_fStopWatch = TRUE;    // is it a stopwatch?
static BOOL s_fStopped = FALSE;     // is it stopped?
static BOOL s_fAlert = FALSE;       // is it alerming?
static BOOL s_fFlash = FALSE;       // is it flashing?
static HWND s_hEdt1;                // text box (EDIT control) 1
static HWND s_hEdt2;                // text box (EDIT control) 2
static HWND s_hEdt3;                // text box (EDIT control) 3
static HWND s_hEdt4;                // text box (EDIT control) 4
static HBRUSH s_hbrRed;             // red brush

static DWORD s_hour;    // hours
static DWORD s_min;     // minutes
static DWORD s_sec;     // seconds
static DWORD s_msec;    // milliseconds

inline LPTSTR LoadStringDx(INT ids)
{
    static TCHAR sz[256];
    LoadString(g_hInstance, ids, sz, 256);
    return sz;
}

inline VOID SetWindowInt02(HWND hWnd, INT n)
{
    TCHAR szBuf[32];
    szBuf[0] = TCHAR('0' + n / 10 % 10);
    szBuf[1] = TCHAR('0' + n % 10);
    szBuf[2] = 0;
    SetWindowText(hWnd, szBuf);
}

inline VOID SetWindowInt03(HWND hWnd, INT n)
{
    TCHAR szBuf[32];
    szBuf[0] = TCHAR('0' + n / 100 % 10);
    szBuf[1] = TCHAR('0' + n / 10 % 10);
    szBuf[2] = TCHAR('0' + n % 10);
    szBuf[3] = 0;
    SetWindowText(hWnd, szBuf);
}

#define TIMER_ID 999
#define TIMER_INTERVAL 20

inline VOID Alert(HWND hwnd)
{
    KillTimer(hwnd, TIMER_ID);
    SetWindowText(s_hEdt1, TEXT("0"));
    SetWindowText(s_hEdt2, TEXT("00"));
    SetWindowText(s_hEdt3, TEXT("00"));
    SetWindowText(s_hEdt4, TEXT("000"));
    PlaySound(LoadStringDx(2), NULL, SND_ASYNC | SND_FILENAME | SND_LOOP);
    FlashWindow(hwnd, TRUE);
    ShowWindow(hwnd, SW_SHOWNORMAL);
    SendMessage(hwnd, DM_REPOSITION, 0, 0);
    s_fAlert = TRUE;
    s_fFlash = TRUE;
    SetTimer(hwnd, TIMER_ID, TIMER_INTERVAL, NULL);
    InvalidateRect(hwnd, NULL, TRUE);
}

VOID Update(HWND hwnd)
{
    DWORD hour, min, sec, msec;
    DWORD count;

    QueryPerformanceCounter(&s_now);
    if (s_fStopWatch)
    {
        count = DWORD(((s_now.QuadPart - s_start.QuadPart) * 1000) / 
                         s_freq.QuadPart + s_deltamsec.QuadPart);
        hour = count / (1000 * 60 * 60);
        min = count / (1000 * 60) % 60;
        sec = count / 1000 % 60;
        msec = count % 1000;

        TCHAR szBuf[32];
        _ultot(hour, szBuf, 10);
        SetWindowText(s_hEdt1, szBuf);
        SetWindowInt02(s_hEdt2, min);
        SetWindowInt02(s_hEdt3, sec);
        SetWindowInt03(s_hEdt4, msec);
    }
    else
    {
        if (s_stop.QuadPart * 1000 < s_now.QuadPart * 1000 - 
                                     s_deltamsec.QuadPart)
        {
            Alert(hwnd);
            return;
        }
        count = DWORD(((s_stop.QuadPart - s_now.QuadPart) * 1000 + 
                          s_deltamsec.QuadPart) / s_freq.QuadPart);
        hour = count / (1000 * 60 * 60);
        min = count / (1000 * 60) % 60;
        sec = count / 1000 % 60;
        msec = count % 1000;

        TCHAR szBuf[32];
        _ultot(hour, szBuf, 10);
        SetWindowText(s_hEdt1, szBuf);
        SetWindowInt02(s_hEdt2, min);
        SetWindowInt02(s_hEdt3, sec);
        SetWindowInt03(s_hEdt4, msec);
    }
}

inline VOID SetReadOnly(BOOL fReadOnly)
{
    SendMessage(s_hEdt1, EM_SETREADONLY, fReadOnly, 0);
    SendMessage(s_hEdt2, EM_SETREADONLY, fReadOnly, 0);
    SendMessage(s_hEdt3, EM_SETREADONLY, fReadOnly, 0);
    SendMessage(s_hEdt4, EM_SETREADONLY, fReadOnly, 0);
    EnableWindow(GetDlgItem(g_hMainWnd, 1000), !fReadOnly);
    EnableWindow(GetDlgItem(g_hMainWnd, 1001), !fReadOnly);
    EnableWindow(GetDlgItem(g_hMainWnd, 1002), !fReadOnly);
    EnableWindow(GetDlgItem(g_hMainWnd, 1003), !fReadOnly);
    EnableWindow(GetDlgItem(g_hMainWnd, psh1), !fReadOnly);
}

inline BOOL OnInitDialog(HWND hwnd, HWND hwndFocus, LPARAM lParam)
{
    g_hMainWnd = hwnd;
    s_hEdt1 = GetDlgItem(hwnd, edt1);
    s_hEdt2 = GetDlgItem(hwnd, edt2);
    s_hEdt3 = GetDlgItem(hwnd, edt3);
    s_hEdt4 = GetDlgItem(hwnd, edt4);
    SendDlgItemMessage(hwnd, 1000, UDM_SETRANGE, 0, MAKELPARAM(99, 0));
    SendDlgItemMessage(hwnd, 1001, UDM_SETRANGE, 0, MAKELPARAM(99, 0));
    SendDlgItemMessage(hwnd, 1002, UDM_SETRANGE, 0, MAKELPARAM(99, 0));
    SendDlgItemMessage(hwnd, 1003, UDM_SETRANGE, 0, MAKELPARAM(999, 0));
    HICON hIcon = LoadIcon(g_hInstance, MAKEINTRESOURCE(1));
    HICON hIconSmall = reinterpret_cast<HICON>(LoadImage(
        g_hInstance, 
        MAKEINTRESOURCE(1),
        IMAGE_ICON, 
        GetSystemMetrics(SM_CXSMICON), 
        GetSystemMetrics(SM_CYSMICON),
        NULL));
    SendMessage(hwnd, WM_SETICON, ICON_BIG, reinterpret_cast<LPARAM>(hIcon));
    SendMessage(hwnd, WM_SETICON, ICON_SMALL, reinterpret_cast<LPARAM>(hIconSmall));
    s_hbrRed = CreateSolidBrush(RED_COLOR);
    s_fRunning = FALSE;
    return TRUE;
}

inline void OnPsh1(HWND hwnd)
{
    DWORD hour, min, sec, msec;
    LARGE_INTEGER li;

    if (!s_fRunning)
    {
        QueryPerformanceCounter(&s_start);
        if (s_fStopped)
        {
            hour = GetDlgItemInt(hwnd, edt1, NULL, FALSE);
            min = GetDlgItemInt(hwnd, edt2, NULL, FALSE);
            sec = GetDlgItemInt(hwnd, edt3, NULL, FALSE);
            msec = GetDlgItemInt(hwnd, edt4, NULL, FALSE);
            s_deltamsec.QuadPart = ((hour * 60 + min) * 60 + sec);
            s_deltamsec.QuadPart *= 1000;
            s_deltamsec.QuadPart += msec;
        }
        else
        {
            s_deltamsec.QuadPart = 0;
        }
        if (!s_fStopWatch)
        {
            if (GetDlgItemInt(hwnd, edt1, NULL, FALSE) == 0 &&
                GetDlgItemInt(hwnd, edt2, NULL, FALSE) == 0 &&
                GetDlgItemInt(hwnd, edt3, NULL, FALSE) == 0 &&
                GetDlgItemInt(hwnd, edt4, NULL, FALSE) == 0)
            {
                s_fStopWatch = TRUE;
            }
        }
        if (!s_fStopWatch)
        {
            hour = GetDlgItemInt(hwnd, edt1, NULL, FALSE);
            min = GetDlgItemInt(hwnd, edt2, NULL, FALSE);
            sec = GetDlgItemInt(hwnd, edt3, NULL, FALSE);
            msec = GetDlgItemInt(hwnd, edt4, NULL, FALSE);
            li.QuadPart = ((hour * 60 + min) * 60 + sec);
            li.QuadPart *= 1000;
            li.QuadPart += msec;
            s_stop.QuadPart = s_start.QuadPart + 
                              li.QuadPart * s_freq.QuadPart / 1000;
            if (!s_fStopped)
            {
                s_hour = hour;
                s_min = min;
                s_sec = sec;
                s_msec = msec;
            }
        }
        s_fRunning = TRUE;
        SetReadOnly(TRUE);
        MessageBeep(0xFFFFFFFF);
        SetTimer(hwnd, TIMER_ID, 90, NULL);
    }
}

inline void OnPsh2(HWND hwnd)
{
    MessageBeep(0xFFFFFFFF);
    if (s_fRunning)
    {
        KillTimer(hwnd, TIMER_ID);
        Update(hwnd);
        SetReadOnly(FALSE);
        if (!s_fStopWatch && s_fAlert)
        {
            SetDlgItemInt(hwnd, edt1, s_hour, FALSE);
            SetDlgItemInt(hwnd, edt2, s_min, FALSE);
            SetDlgItemInt(hwnd, edt3, s_sec, FALSE);
            SetDlgItemInt(hwnd, edt4, s_msec, FALSE);
        }
        s_fRunning = FALSE;
        s_fStopped = TRUE;
    }
    else
    {
        SetWindowText(s_hEdt1, TEXT("0"));
        SetWindowText(s_hEdt2, TEXT("00"));
        SetWindowText(s_hEdt3, TEXT("00"));
        SetWindowText(s_hEdt4, TEXT("000"));
        s_fStopWatch = TRUE;
        s_fStopped = FALSE;
    }
    InvalidateRect(hwnd, NULL, TRUE);
    s_fAlert = FALSE;
    KillTimer(hwnd, TIMER_ID);
    PlaySound(NULL, NULL, 0);
}

inline void OnEdt1to4(HWND hwnd, UINT codeNotify)
{
    if (!s_fRunning && codeNotify == EN_CHANGE)
    {
        s_fStopWatch = FALSE;
        s_fStopped = FALSE;
    }
}

inline void OnCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify)
{
    switch (id)
    {
    case psh1:
        OnPsh1(hwnd);
        break;

    case psh2:
        OnPsh2(hwnd);
        break;

    case edt1:
    case edt2:
    case edt3:
    case edt4:
        OnEdt1to4(hwnd, codeNotify);
        break;

    case IDCANCEL:
        EndDialog(hwnd, id);
        break;
    }
}

inline HBRUSH OnCtlColor(HWND hwnd, HDC hdc, HWND hwndChild, int type)
{
    if (s_fAlert && s_fFlash)
    {
        SetTextColor(hdc, BLACK_COLOR);
        SetBkColor(hdc, RED_COLOR);
        return s_hbrRed;
    }
    return NULL;
}

inline void OnTimer(HWND hwnd, UINT id)
{
    if (!s_fAlert)
    {
        Update(hwnd);
    }
    else
    {
        s_fFlash = !s_fFlash;
        InvalidateRect(hwnd, NULL, TRUE);
    }
}

INT_PTR CALLBACK DialogProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch(uMsg)
    {
        HANDLE_MSG(hwnd, WM_INITDIALOG, OnInitDialog);
        HANDLE_MSG(hwnd, WM_COMMAND, OnCommand);
        HANDLE_MSG(hwnd, WM_CTLCOLORDLG, OnCtlColor);
        HANDLE_MSG(hwnd, WM_TIMER, OnTimer);
    }
    return 0;
}

extern "C"
INT WINAPI
WinMain(HINSTANCE   hInstance,
        HINSTANCE   hPrevInstance,
        LPSTR       pszCmdLine,
        INT         nCmdShow)
{
    g_hInstance = hInstance;
    InitCommonControls();

    if (QueryPerformanceFrequency(&s_freq))
    {
        DialogBox(hInstance, MAKEINTRESOURCE(1), NULL, DialogProc);
    }
    else
    {
        MessageBox(NULL, LoadStringDx(1), NULL, MB_ICONERROR);
    }

    return 0;
}
