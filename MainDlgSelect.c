/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * <etienne.doms@gmail.com> wrote this file. As long as you retain this notice
 * you can do whatever you want with this stuff. If we meet some day, and you
 * think this stuff is worth it, you can buy me a beer in return. Etienne Doms
 * ----------------------------------------------------------------------------
 * 2014/02/08 <mail@michael-kaufmann.ch> - Don't show the "Open as read-only" checkbox
 * 2014/11/22 <mail@michael-kaufmann.ch> - File browsing: Provide the file filters "Disk image files" and "All files"
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
    ofn.lpstrFilter = TEXT("Disk image files (*.img; *.iso)\0*.IMG;*.ISO\0All files (*.*)\0*.*\0\0");
    ofn.lpstrFile = lpstrFile;
    ofn.nMaxFile = MAX_PATH;
    ofn.Flags = OFN_FILEMUSTEXIST | OFN_HIDEREADONLY;

    if (GetOpenFileName(&ofn)) {
        SetDlgItemText(hwndDlg, IDC_MAINDLG_SOURCE, ofn.lpstrFile);
    }

    return TRUE;
}
