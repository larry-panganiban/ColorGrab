// Copyright (c) 2024 Jay Prall
// SPDX-License-Identifier: MIT

// ColorGrab.cpp : Application entry point and core initialization logic.
// Handles single‑instance enforcement, portable‑mode detection, settings
// loading/saving, and creation of the main ColorGrab dialog.

#include "pch.h"
#include "ColorGrab.h"
#include "ColorGrabDlg.h"
#include "Defaults.h"
#include <shlobj.h>  // SHGetKnownFolderPath, FOLDERID_LocalAppData

#define INI_FILE _T("\\ColorGrab.dat")
#define INI_FILE_DIR _T("\\ColorGrab")

#define MONITOR_CENTER 0x0001    // center rect to monitor
#define MONITOR_CLIP 0x0000      // clip rect to monitor
#define MONITOR_WORKAREA 0x0002  // use monitor work area
#define MONITOR_AREA 0x0000      // use monitor entire area

/////////////////////////////////////////////////////////////////////////////
// CColorGrabApp

BEGIN_MESSAGE_MAP(CColorGrabApp, CWinApp)
    //{{AFX_MSG_MAP(CColorGrabApp) // NOLINT(whitespace/comments)
    // NOTE - the ClassWizard will add and remove mapping macros here.
    //    DO NOT EDIT what you see in these blocks of generated code!
    //}}AFX_MSG // NOLINT(whitespace/comments)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CColorGrabApp construction


CColorGrabApp::CColorGrabApp() {
    m_hMutex = nullptr;
}

CColorGrabApp::~CColorGrabApp() {
    if (m_hMutex != nullptr) {
        CloseHandle(m_hMutex);
        m_hMutex = nullptr;
    }
}

/////////////////////////////////////////////////////////////////////////////
// The one and only CColorGrabApp object

CColorGrabApp theApp;

/////////////////////////////////////////////////////////////////////////////
// CColorGrabApp initialization

BOOL CColorGrabApp::InitInstance() {
    // Enforce single‑instance mode unless explicitly overridden by the
    // MultipleInstances flag. If another instance is already running,
    // activate its window and terminate this one immediately.
    if (!(dlg.m_Appflags & MultipleInstances)) {
        if (InstanceRunning()) {
            BringExistingInstanceToFront();
            return FALSE;  // stop launching a second instance
        }
    }

    // Initialize and display the main dialog for this instance.
    dlg.m_PortableMode = m_PortableMode;
    m_pMainWnd = &dlg;

    int nResponse = dlg.DoModal();  // run the ColorGrab dialog modally

    // Persist settings on normal exit paths (OK or Cancel).
    if (nResponse == IDOK || nResponse == IDCANCEL) {
        CloseApplication();
    }

    return FALSE;  // terminate the application after the dialog closes
}

bool CColorGrabApp::BringExistingInstanceToFront() {
    // Locate the existing ColorGrab dialog by its default dialog class (#32770)
    // and its exact window caption ("ColorGrab"). This works because the dialog
    // uses the standard Windows dialog class and the caption is fixed in the
    // dialog resource. If either changes (e.g., localization or dynamic titles),
    // this lookup must be updated.
    HWND hWnd = FindWindow(_T("#32770"), _T("ColorGrab"));
    if (!hWnd) {
        TRACE(_T("Unable to find ColorGrab window to bring to front\n"));
        return false;
    }

    AllowSetForegroundWindow(ASFW_ANY);

    if (IsIconic(hWnd)) {
        ShowWindow(hWnd, SW_RESTORE);
    } else {
        ShowWindow(hWnd, SW_SHOWNORMAL);
    }

    SetForegroundWindow(hWnd);
    BringWindowToTop(hWnd);

    return true;
}

// uses a Mutex to figure out if there is an instance of ColorGrab running
bool CColorGrabApp::InstanceRunning() {
    if (!m_hMutex) {
        m_hMutex = CreateMutex(NULL, TRUE, _T("ColorGrab_Mutex"));
    }
    return (m_hMutex && GetLastError() == ERROR_ALREADY_EXISTS);
}

BOOL CColorGrabApp::GetShellFolderPath(LPCTSTR pShellFolder, LPTSTR pShellPath) {
    // pShellFolder can be one of the following
    // AppData, Cache, Cookies, Desktop, Favorites, Fonts, History, NetHood,
    // Personal, Printhood, Programs, Recent, SendTo, Start Menu, Startup,
    // Templates, ShellNew
    DWORD rc;
    DWORD length = MAX_PATH;
    DWORD type = REG_SZ;
    HKEY hkey;

    rc = RegOpenKeyEx(HKEY_CURRENT_USER,
                      _T("Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Shell Folders"),
                      0, KEY_READ, &hkey);

    if (rc == ERROR_SUCCESS) {
        rc = RegQueryValueEx(hkey, pShellFolder, NULL, &type,
                             reinterpret_cast<BYTE*>(pShellPath), &length);
        RegCloseKey(hkey);
    }

    if (rc == ERROR_SUCCESS) {
        return TRUE;
    } else {
        return FALSE;
    }
}

std::filesystem::path CColorGrabApp::GetSettingsFolder() {
    if (m_PortableMode) {
        // Portable mode → directory containing the EXE
        wchar_t exePath[MAX_PATH];
        GetModuleFileNameW(nullptr, exePath, MAX_PATH);
        std::filesystem::path exe(exePath);
        return exe.parent_path();
    }

    // Installed mode → LocalAppData\ColorGrab
    PWSTR raw = nullptr;
    if (SUCCEEDED(SHGetKnownFolderPath(FOLDERID_LocalAppData, 0, nullptr, &raw))) {
        std::filesystem::path localAppData(raw);
        CoTaskMemFree(raw);
        return localAppData / "ColorGrab";
    }

    // Fallback if LocalAppData cannot be resolved
    return std::filesystem::current_path() / "ColorGrab";
}

void CColorGrabApp::ClipOrCenterWindowToMonitor(HWND hwnd, UINT flags) {
    (void) flags;  // explicitly mark unused to avoid a warning
    RECT rc;
    GetWindowRect(hwnd, &rc);
    SetWindowPos(hwnd, NULL, rc.left, rc.top, 0, 0, SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE);
    dlg.WinLocX = rc.left;
    dlg.WinLocY = rc.top;
}

BOOL CColorGrabApp::InitApplication() {
    // Detect portable‑mode command‑line switches (restores pre‑5.3 behavior)
    CString cmd = GetCommandLine();
    cmd.MakeLower();

    if (cmd.Find(_T("-portable")) != -1 || cmd.Find(_T("--portable")) != -1 ||
        cmd.Find(_T("/portable")) != -1 || cmd.Find(_T("/p")) != -1) {
        m_PortableMode = true;
    }
    std::filesystem::path settingsFolder = GetSettingsFolder();
    std::filesystem::path iniPath = settingsFolder / "ColorGrab.dat";
    CString strInitFile(iniPath.wstring().c_str());

    TRACE(_T("Portable mode: %d\n"), m_PortableMode);
    TRACE(_T("INI path: %s\n"), strInitFile.GetString());

    CFile file;
    if (file.Open(strInitFile, CFile::modeRead)) {
        try {
            CArchive ar(&file, CArchive::load);
            Serialize(ar);
        } catch (CArchiveException* e) {
            e->Delete();
            TRACE(_T("Settings file corrupt or incompatible. Loading defaults.\n"));
            LoadDefaultSettings();
            ClipOrCenterWindowToMonitor(::GetForegroundWindow(), MONITOR_CENTER);
        }
    } else {
        // First time running ColorGrab
        if (!m_PortableMode) {
            CreateDirectory(settingsFolder.wstring().c_str(), NULL);
        }
        LoadDefaultSettings();
        ClipOrCenterWindowToMonitor(::GetForegroundWindow(), MONITOR_CENTER);
    }


    return CWinApp::InitApplication();
}

// Load all default settings. Called when no valid .DAT file exists
void CColorGrabApp::LoadDefaultSettings() {
    const COLORREF seed = kDefaultSeedColor;

    // extract RGB components from the seed color in order to set the initial color values
    dlg.m_Reddec = static_cast<int>(seed & 0xFF);
    dlg.m_Greendec = static_cast<int>((seed >> 8) & 0xFF);
    dlg.m_Bluedec = static_cast<int>((seed >> 16) & 0xFF);

    dlg.m_Appflags = AlwaysOnTop | AutoCopytoClip | ModeHTML | EasyMove |
                     Sampling1 | ExpandedDialog | MultipleInstances |
                     MAGWHILEEYEDROP | SpaceRGB;

    // Window defaults
    dlg.WinLocX = kDefaultWinLocX;
    dlg.WinLocY = kDefaultWinLocY;
    dlg.m_iSamplingOffset = kDefaultSamplingOffset;

    // Custom colors
    for (int i = 0; i < kCustomColorCount; ++i)
        dlg.CustColorBank[i] = kDefaultCustomColor;

    // History colors
    for (int i = 0; i < kHistoryCount; ++i)
        dlg.ColorHistory[i] = kDefaultColorHistory[i];
}

void CColorGrabApp::CloseApplication() {
    std::filesystem::path settingsFolder = GetSettingsFolder();
    std::filesystem::path iniPath = settingsFolder / "ColorGrab.dat";
    CString strInitFile(iniPath.wstring().c_str());

    TRACE(_T("Portable mode: %d\n"), m_PortableMode);
    TRACE(_T("INI path: %s\n"), strInitFile.GetString());


    if (!m_PortableMode) {
        // Ensure folder exists in installed mode
        CreateDirectory(settingsFolder.wstring().c_str(), NULL);
    }

    CFile file;
    if (file.Open(strInitFile, CFile::modeWrite | CFile::modeCreate)) {
        CArchive ar(&file, CArchive::store);
        Serialize(ar);
    }
}

void CColorGrabApp::Serialize(CArchive& ar) {
    if (ar.IsStoring()) {
        // storing code
        try {
            ar << dlg.m_Reddec;
            ar << dlg.m_Greendec;
            ar << dlg.m_Bluedec;
            ar << dlg.m_Appflags;
            ar << dlg.WinLocX;
            ar << dlg.WinLocY;
            for (int j = 0; j < kCustomColorCount; j++)  // load custom color values to array
                ar << dlg.CustColorBank[j];
            for (int w = 0; w < kHistoryCount; w++)  // save color history values
                ar << dlg.ColorHistory[w];

            ar << dlg.m_iSamplingOffset;
        } catch (CArchiveException& e) {
            TRACE(_T("Archive exception caught. Cause = %d, File = %s\n"),
                  e.m_cause,
                  e.m_strFileName.IsEmpty() ? _T("(unknown)") : e.m_strFileName.GetString());
            AfxMessageBox(IDS_ERROR_SAVING);
        }
    } else {
        // loading code
        try {
            ar >> dlg.m_Reddec;
            ar >> dlg.m_Greendec;
            ar >> dlg.m_Bluedec;
            ar >> dlg.m_Appflags;
            ar >> dlg.WinLocX;
            ar >> dlg.WinLocY;
            // load custom color values to array
            for (int j = 0; j < kCustomColorCount; j++) {
                ar >> dlg.CustColorBank[j];
            }
            // load color history values to array
            for (int w = 0; w < kHistoryCount; w++) {
                ar >> dlg.ColorHistory[w];
            }
            ar >> dlg.m_iSamplingOffset;
        } catch (CArchiveException& e) {
            TRACE(_T("Archive exception caught. Cause = %d, File = %s\n"),
                  e.m_cause,
                  e.m_strFileName.IsEmpty() ? _T("(unknown)") : e.m_strFileName.GetString());
            AfxMessageBox(IDS_ERROR_LOADING);
        }
    }
}
