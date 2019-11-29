#include <windows.h>
#include <commctrl.h>
#include <dlgs.h>
#include <tchar.h>
#include <stdlib.h>

HINSTANCE g_hInstance;
HWND g_hMainWnd;
LARGE_INTEGER g_freq;
LARGE_INTEGER g_now;
LARGE_INTEGER g_start;
LARGE_INTEGER g_stop;
LARGE_INTEGER g_deltamsec;

BOOL g_fRunning = TRUE;
BOOL g_fStopWatch = TRUE;
BOOL g_fStopped = FALSE;
BOOL g_fAlert = FALSE;
BOOL g_fFlash = FALSE;
HWND g_hEdit1;
HWND g_hEdit2;
HWND g_hEdit3;
HWND g_hEdit4;

const COLORREF g_rgbRed = RGB(255, 96, 96);
HBRUSH g_hbrRed;

TCHAR g_sz[32];

DWORD g_count;
DWORD g_hour, g_min, g_sec, g_msec;

static const TCHAR g_sz0[] = TEXT("0");
static const TCHAR g_sz00[] = TEXT("00");
static const TCHAR g_sz000[] = TEXT("000");


LPTSTR LoadStringDx(INT ids)
{
    static TCHAR sz[256];
    LoadString(g_hInstance, ids, sz, 256);
    return sz;
}

inline VOID SetWindowInt02(HWND hWnd, INT n)
{
    g_sz[0] = (TCHAR)('0' + n / 10 % 10);
    g_sz[1] = (TCHAR)('0' + n % 10);
    g_sz[2] = (TCHAR)0;
    SetWindowText(hWnd, g_sz);
}

inline VOID SetWindowInt03(HWND hWnd, INT n)
{
    g_sz[0] = (TCHAR)('0' + n / 100 % 10);
    g_sz[1] = (TCHAR)('0' + n / 10 % 10);
    g_sz[2] = (TCHAR)('0' + n % 10);
    g_sz[3] = (TCHAR)0;
    SetWindowText(hWnd, g_sz);
}

inline VOID Alert(HWND hDlg)
{
    KillTimer(hDlg, 999);
    SetWindowText(g_hEdit1, g_sz0);
    SetWindowText(g_hEdit2, g_sz00);
    SetWindowText(g_hEdit3, g_sz00);
    SetWindowText(g_hEdit4, g_sz000);
    PlaySound(LoadStringDx(2), NULL, SND_ASYNC | SND_FILENAME | SND_LOOP);
    FlashWindow(hDlg, TRUE);
    ShowWindow(hDlg, SW_SHOWNORMAL);
    SendMessage(hDlg, DM_REPOSITION, 0, 0);
    g_fAlert = TRUE;
    g_fFlash = TRUE;
    SetTimer(hDlg, 999, 250, NULL);
    InvalidateRect(hDlg, NULL, TRUE);
}

VOID Update(HWND hDlg)
{
    DWORD hour, min, sec, msec;
    QueryPerformanceCounter(&g_now);
    if (g_fStopWatch)
    {
        g_count = (DWORD)(((g_now.QuadPart - g_start.QuadPart) * 1000) / 
                          g_freq.QuadPart + g_deltamsec.QuadPart);
        hour = g_count / (1000 * 60 * 60);
        min = g_count / (1000 * 60) % 60;
        sec = g_count / 1000 % 60;
        msec = g_count % 1000;
        _ultot(hour, g_sz, 10);
        SetWindowText(g_hEdit1, g_sz);
        SetWindowInt02(g_hEdit2, min);
        SetWindowInt02(g_hEdit3, sec);
        SetWindowInt03(g_hEdit4, msec);
    }
    else
    {
        if (g_stop.QuadPart * 1000 < g_now.QuadPart * 1000 - 
                                     g_deltamsec.QuadPart)
        {
            Alert(hDlg);
            return;
        }
        g_count = (DWORD)(((g_stop.QuadPart - g_now.QuadPart) * 1000 + 
                           g_deltamsec.QuadPart) / g_freq.QuadPart);
        hour = g_count / (1000 * 60 * 60);
        min = g_count / (1000 * 60) % 60;
        sec = g_count / 1000 % 60;
        msec = g_count % 1000;
        _ultot(hour, g_sz, 10);
        SetWindowText(g_hEdit1, g_sz);
        SetWindowInt02(g_hEdit2, min);
        SetWindowInt02(g_hEdit3, sec);
        SetWindowInt03(g_hEdit4, msec);
    }
}

VOID SetReadOnly(BOOL fReadOnly)
{
    SendMessage(g_hEdit1, EM_SETREADONLY, fReadOnly, 0);
    SendMessage(g_hEdit2, EM_SETREADONLY, fReadOnly, 0);
    SendMessage(g_hEdit3, EM_SETREADONLY, fReadOnly, 0);
    SendMessage(g_hEdit4, EM_SETREADONLY, fReadOnly, 0);
    EnableWindow(GetDlgItem(g_hMainWnd, 1000), !fReadOnly);
    EnableWindow(GetDlgItem(g_hMainWnd, 1001), !fReadOnly);
    EnableWindow(GetDlgItem(g_hMainWnd, 1002), !fReadOnly);
    EnableWindow(GetDlgItem(g_hMainWnd, 1003), !fReadOnly);
    EnableWindow(GetDlgItem(g_hMainWnd, psh1), !fReadOnly);
}

BOOL CALLBACK DialogProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    LARGE_INTEGER li;
    DWORD hour, min, sec, msec;
    HICON hIcon, hIconSmall;

    switch(uMsg)
    {
    case WM_INITDIALOG:
        g_hMainWnd = hDlg;
        g_hEdit1 = GetDlgItem(hDlg, edt1);
        g_hEdit2 = GetDlgItem(hDlg, edt2);
        g_hEdit3 = GetDlgItem(hDlg, edt3);
        g_hEdit4 = GetDlgItem(hDlg, edt4);
        SendDlgItemMessage(hDlg, 1000, UDM_SETRANGE, 0, MAKELPARAM(99, 0));
        SendDlgItemMessage(hDlg, 1001, UDM_SETRANGE, 0, MAKELPARAM(99, 0));
        SendDlgItemMessage(hDlg, 1002, UDM_SETRANGE, 0, MAKELPARAM(99, 0));
        SendDlgItemMessage(hDlg, 1003, UDM_SETRANGE, 0, MAKELPARAM(999, 0));
        hIcon = LoadIcon(g_hInstance, MAKEINTRESOURCE(1));
        hIconSmall = (HICON)LoadImage(
            g_hInstance, 
            MAKEINTRESOURCE(1),
            IMAGE_ICON, 
            GetSystemMetrics(SM_CXSMICON), 
            GetSystemMetrics(SM_CYSMICON),
            NULL);
        SendMessage(hDlg, WM_SETICON, ICON_BIG, (LPARAM)hIcon);
        SendMessage(hDlg, WM_SETICON, ICON_SMALL, (LPARAM)hIconSmall);
        g_hbrRed = CreateSolidBrush(g_rgbRed);
        g_fRunning = FALSE;
        break;

    case WM_COMMAND:
        switch(LOWORD(wParam))
        {
        case psh1:
            if (!g_fRunning)
            {
                QueryPerformanceCounter(&g_start);
                if (g_fStopped)
                {
                    hour = GetDlgItemInt(hDlg, edt1, NULL, FALSE);
                    min = GetDlgItemInt(hDlg, edt2, NULL, FALSE);
                    sec = GetDlgItemInt(hDlg, edt3, NULL, FALSE);
                    msec = GetDlgItemInt(hDlg, edt4, NULL, FALSE);
                    g_deltamsec.QuadPart = ((hour * 60 + min) * 60 + sec);
                    g_deltamsec.QuadPart *= 1000;
                    g_deltamsec.QuadPart += msec;
                }
                else
                {
                    g_deltamsec.QuadPart = 0;
                }
                if (!g_fStopWatch)
                {
                    if (GetDlgItemInt(hDlg, edt1, NULL, FALSE) == 0 &&
                        GetDlgItemInt(hDlg, edt2, NULL, FALSE) == 0 &&
                        GetDlgItemInt(hDlg, edt3, NULL, FALSE) == 0 &&
                        GetDlgItemInt(hDlg, edt4, NULL, FALSE) == 0)
                    {
                        g_fStopWatch = TRUE;
                    }
                }
                if (!g_fStopWatch)
                {
                    hour = GetDlgItemInt(hDlg, edt1, NULL, FALSE);
                    min = GetDlgItemInt(hDlg, edt2, NULL, FALSE);
                    sec = GetDlgItemInt(hDlg, edt3, NULL, FALSE);
                    msec = GetDlgItemInt(hDlg, edt4, NULL, FALSE);
                    li.QuadPart = ((hour * 60 + min) * 60 + sec);
                    li.QuadPart *= 1000;
                    li.QuadPart += msec;
                    g_stop.QuadPart = g_start.QuadPart + 
                                      li.QuadPart * g_freq.QuadPart / 1000;
                    if (!g_fStopped)
                    {
                        g_hour = hour;
                        g_min = min;
                        g_sec = sec;
                        g_msec = msec;
                    }
                }
                g_fRunning = TRUE;
                SetReadOnly(TRUE);
                MessageBeep(0xFFFFFFFF);
                SetTimer(hDlg, 999, 90, NULL);
            }
            break;

        case psh2:
            MessageBeep(0xFFFFFFFF);
            if (g_fRunning)
            {
                KillTimer(hDlg, 999);
                Update(hDlg);
                SetReadOnly(FALSE);
                if (!g_fStopWatch && g_fAlert)
                {
                    SetDlgItemInt(hDlg, edt1, g_hour, FALSE);
                    SetDlgItemInt(hDlg, edt2, g_min, FALSE);
                    SetDlgItemInt(hDlg, edt3, g_sec, FALSE);
                    SetDlgItemInt(hDlg, edt4, g_msec, FALSE);
                }
                g_fRunning = FALSE;
                g_fStopped = TRUE;
            }
            else
            {
                SetWindowText(g_hEdit1, g_sz0);
                SetWindowText(g_hEdit2, g_sz00);
                SetWindowText(g_hEdit3, g_sz00);
                SetWindowText(g_hEdit4, g_sz000);
                g_fStopWatch = TRUE;
                g_fStopped = FALSE;
            }
            InvalidateRect(hDlg, NULL, TRUE);
            g_fAlert = FALSE;
            KillTimer(hDlg, 999);
            PlaySound(NULL, NULL, 0);
            break;

        case edt1:
        case edt2:
        case edt3:
        case edt4:
            if (!g_fRunning && HIWORD(wParam) == EN_CHANGE)
            {
                g_fStopWatch = FALSE;
                g_fStopped = FALSE;
            }
            break;

        case IDCANCEL:
            EndDialog(hDlg, IDCANCEL);
            break;
        }
        break;

    case WM_CTLCOLORDLG:
    case WM_CTLCOLORSTATIC:
        if (g_fAlert && g_fFlash)
        {
            SetTextColor((HDC)wParam, RGB(0, 0, 0));
            SetBkColor((HDC)wParam, g_rgbRed);
            return (BOOL)g_hbrRed;
        }
        break;

    case WM_TIMER:
        if (!g_fAlert)
        {
            Update(hDlg);
        }
        else
        {
            g_fFlash = !g_fFlash;
            InvalidateRect(hDlg, NULL, TRUE);
        }
        break;
    }
    return FALSE;
}

INT WINAPI WinMain(
    HINSTANCE   hInstance,
    HINSTANCE   hPrevInstance,
    LPSTR       pszCmdLine,
    INT         nCmdShow)
{
    g_hInstance = hInstance;
    InitCommonControls();

    if (QueryPerformanceFrequency(&g_freq))
    {
        DialogBox(hInstance, MAKEINTRESOURCE(1), NULL, DialogProc);
    }
    else
    {
        MessageBox(NULL, LoadStringDx(1), NULL, MB_ICONERROR);
    }

    return 0;
}
