/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * <etienne.doms@gmail.com> wrote this file. As long as you retain this notice
 * you can do whatever you want with this stuff. If we meet some day, and you
 * think this stuff is worth it, you can buy me a beer in return. Etienne Doms
 * ----------------------------------------------------------------------------
 */

#include "MainDlgSelect.h"
#include "resource.h"

INT_PTR MainDlgSelectClick(HWND hwndDlg) {
    OPENFILENAME ofn;
    TCHAR lpstrFile[MAX_PATH];

    ZeroMemory(&ofn, sizeof ofn);
    ZeroMemory(lpstrFile, sizeof lpstrFile);

    ofn.lStructSize = sizeof ofn;
    ofn.hwndOwner = hwndDlg;
    ofn.lpstrFile = lpstrFile;
    ofn.nMaxFile = MAX_PATH;
    ofn.Flags = OFN_FILEMUSTEXIST;

    if (GetOpenFileName(&ofn)) {
        SetDlgItemText(hwndDlg, IDC_MAINDLG_SOURCE, ofn.lpstrFile);
    }

    return TRUE;
}