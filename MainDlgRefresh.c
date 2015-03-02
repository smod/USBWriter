/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * <etienne.doms@gmail.com> wrote this file. As long as you retain this notice
 * you can do whatever you want with this stuff. If we meet some day, and you
 * think this stuff is worth it, you can buy me a beer in return. Etienne Doms
 * ----------------------------------------------------------------------------
 * 2014/11/22 <mail@michael-kaufmann.ch> - Keep the current selection when refreshing the list of target devices
 * 2014/11/22 <mail@michael-kaufmann.ch> - Show the size of the target devices
 */

#include "MainDlgRefresh.h"
#include "resource.h"
#include <winioctl.h>

INT_PTR MainDlgRefreshClick(HWND hwndDlg) {
    TCHAR driveLetter;
    TCHAR selectedDriveLetter = TEXT('\0');

    LRESULT index = SendDlgItemMessage(hwndDlg, IDC_MAINDLG_TARGET_LIST, CB_GETCURSEL, 0, 0);
    if (index != CB_ERR) {
        selectedDriveLetter = (TCHAR) SendDlgItemMessage(hwndDlg, IDC_MAINDLG_TARGET_LIST, CB_GETITEMDATA, index, 0);
    }

    SendDlgItemMessage(hwndDlg, IDC_MAINDLG_TARGET_LIST, CB_RESETCONTENT, 0, 0);

    for (driveLetter = TEXT('A'); driveLetter <= TEXT('Z'); ++driveLetter) {
        TCHAR szRootPathName[sizeof "\\\\.\\A:\\"];
        TCHAR szVolumePathName[sizeof "\\\\.\\A:"];
        ULONGLONG totalNumberOfBytes = 0;

        wsprintf(szRootPathName, TEXT("\\\\.\\%c:\\"), driveLetter);
        wsprintf(szVolumePathName, TEXT("\\\\.\\%c:"), driveLetter);

        if (GetDriveType(szRootPathName) == DRIVE_REMOVABLE) {
            TCHAR szLbText[1024];

            HANDLE hTargetVolume = CreateFile(szVolumePathName, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);
            if (hTargetVolume != INVALID_HANDLE_VALUE) {
                DISK_GEOMETRY diskGeometry;
                DWORD bytesReturned;

                if (DeviceIoControl(hTargetVolume, IOCTL_DISK_GET_DRIVE_GEOMETRY, NULL, 0, &diskGeometry, sizeof diskGeometry, &bytesReturned, NULL)) {
                    totalNumberOfBytes = diskGeometry.Cylinders.QuadPart * diskGeometry.TracksPerCylinder * diskGeometry.SectorsPerTrack * diskGeometry.BytesPerSector;
                }

                CloseHandle(hTargetVolume);
            }

            if (totalNumberOfBytes > 0) {
                ULONG sizeInGbTimes10 = (ULONG)(10 * totalNumberOfBytes / 1024 / 1024 / 1024);
                ULONG sizeInGbInt = sizeInGbTimes10 / 10;
                ULONG sizeInGbFrac = sizeInGbTimes10 % 10;
                wsprintf(szLbText, TEXT("%c: [%lu.%lu GB]"), driveLetter, sizeInGbInt, sizeInGbFrac);
            }
            else {
                wsprintf(szLbText, TEXT("%c:"), driveLetter);
            }

            index = SendDlgItemMessage(hwndDlg, IDC_MAINDLG_TARGET_LIST, CB_ADDSTRING, 0, (LPARAM) szLbText);
            SendDlgItemMessage(hwndDlg, IDC_MAINDLG_TARGET_LIST, CB_SETITEMDATA, (WPARAM) index, (LPARAM) driveLetter);

            if (driveLetter == selectedDriveLetter) {
                SendDlgItemMessage(hwndDlg, IDC_MAINDLG_TARGET_LIST, CB_SETCURSEL, (WPARAM) index, 0);
            }
        }
    }

    return TRUE;
}
