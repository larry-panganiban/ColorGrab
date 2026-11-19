// Copyright (c) 2024 Jay Prall
// SPDX-License-Identifier: MIT

// ColorGrab.h : main header file for the ColorGrab application
//

#pragma once

#ifndef __AFXWIN_H__
#error include 'pch.h' before including this file for PCH
#endif

#include <filesystem>

#include "resource.h"     // main symbols
#include "ColorGrabDlg.h"  // Added by ClassView

/////////////////////////////////////////////////////////////////////////////
// CColorGrabApp:
// See ColorGrab.cpp for the implementation of this class
//
class CColorGrabApp : public CWinApp {
     public:
    bool m_PortableMode = false;  // set via command-line switches

    CColorGrabDlg dlg;
    CColorGrabApp();
    ~CColorGrabApp();
    /* Returns the filesystem path used to store the application's persistent
     * state, including ColorGrab.dat and other configuration files. In portable
     * mode, this is the directory containing the executable. In installed mode,
     * it is the user's LocalAppData\ColorGrab folder. The returned value is a
     * std::filesystem::path, allowing safe and Unicode‑aware path handling.
     */
    std::filesystem::path GetSettingsFolder();

     protected:
    void CloseApplication();
    void LoadDefaultSettings();
    BOOL GetShellFolderPath(LPCTSTR pShellFolder, LPTSTR pShellPath);
    void ClipOrCenterWindowToMonitor(HWND hwnd, UINT flags);
    void ClipOrCenterRectToMonitor(LPRECT prc, UINT flags);
    bool InstanceRunning();
    HANDLE m_hMutex;

    // Overrides
    // ClassWizard generated virtual function overrides
    //{{AFX_VIRTUAL(CColorGrabApp) // NOLINT(whitespace/comments)
     public:
    virtual BOOL InitInstance();
    bool BringExistingInstanceToFront();
    virtual BOOL InitApplication();
    virtual void Serialize(CArchive& ar);
    //}}AFX_VIRTUAL // NOLINT(whitespace/comments)

    // Implementation

    //{{AFX_MSG(CColorGrabApp) // NOLINT(whitespace/comments)
    // NOTE - the ClassWizard will add and remove member functions here.
    //    DO NOT EDIT what you see in these blocks of generated code !
    //}}AFX_MSG // NOLINT(whitespace/comments)
    DECLARE_MESSAGE_MAP()
};


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}} // NOLINT(whitespace/comments)
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.
