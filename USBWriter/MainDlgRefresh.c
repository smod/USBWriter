/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * <etienne.doms@gmail.com> wrote this file. As long as you retain this notice
 * you can do whatever you want with this stuff. If we meet some day, and you
 * think this stuff is worth it, you can buy me a beer in return. Etienne Doms
 * ----------------------------------------------------------------------------
 */

#include "MainDlgRefresh.h"
#include "resource.h"

INT_PTR MainDlgRefreshClick(HWND hwndDlg) {
    TCHAR driveLetter;

    SendDlgItemMessage(hwndDlg, IDC_MAINDLG_TARGET_LIST, CB_RESETCONTENT, 0, 0);

    for (driveLetter = TEXT('A'); driveLetter <= TEXT('Z'); ++driveLetter) {
        TCHAR szRootPathName[sizeof "\\\\.\\X:\\"];

        wsprintf(szRootPathName, TEXT("\\\\.\\%c:\\"), driveLetter);

        if (GetDriveType(szRootPathName) == DRIVE_REMOVABLE) {
            TCHAR szLbText[sizeof "X:"];

            wsprintf(szLbText, TEXT("%c:"), driveLetter);
            SendDlgItemMessage(hwndDlg, IDC_MAINDLG_TARGET_LIST, CB_ADDSTRING, 0, (LPARAM) szLbText);
        }
    }

    return TRUE;
}