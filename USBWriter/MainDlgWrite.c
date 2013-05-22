/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * <etienne.doms@gmail.com> wrote this file. As long as you retain this notice
 * you can do whatever you want with this stuff. If we meet some day, and you
 * think this stuff is worth it, you can buy me a beer in return. Etienne Doms
 * ----------------------------------------------------------------------------
 * 2013/05/16 <pierrevr@mindgoo.be> - Changed buffer size to reduce I/O operations
 */

#include "MainDlgWrite.h"
#include "resource.h"
#include <commctrl.h>

#define BUFFER_SIZE 32 * 1024 * 1024

static DWORD WINAPI ThreadRoutine(LPVOID lpParam) {
    HWND hwndDlg = (HWND) lpParam;
    TCHAR szFilePathName[MAX_PATH];
    HANDLE hSourceFile;

    GetDlgItemText(hwndDlg, IDC_MAINDLG_SOURCE, szFilePathName, ARRAYSIZE(szFilePathName));
    hSourceFile = CreateFile(szFilePathName, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

    if (hSourceFile != INVALID_HANDLE_VALUE) {
        LRESULT index = SendDlgItemMessage(hwndDlg, IDC_MAINDLG_TARGET_LIST, CB_GETCURSEL, 0, 0);

        if (index != CB_ERR) {
            TCHAR szLbText[sizeof "X:"];
            TCHAR szVolumePathName[sizeof "\\\\.\\X:"];
            HANDLE hTargetVolume;

            SendDlgItemMessage(hwndDlg, IDC_MAINDLG_TARGET_LIST, CB_GETLBTEXT, index, (LPARAM) szLbText);
            wsprintf(szVolumePathName, TEXT("\\\\.\\%c:"), szLbText[0]);
            hTargetVolume = CreateFile(szVolumePathName, GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);

            if (hTargetVolume != INVALID_HANDLE_VALUE) {
                DWORD bytesReturned;

                if (DeviceIoControl(hTargetVolume, FSCTL_LOCK_VOLUME, NULL, 0, NULL, 0, &bytesReturned, NULL)) {
                    VOLUME_DISK_EXTENTS volumeDiskExtents;
                    TCHAR szDevicePathName[sizeof "\\\\.\\PhysicalDrive99"];
                    HANDLE hTargetDevice;

                    DeviceIoControl(hTargetVolume, FSCTL_DISMOUNT_VOLUME, NULL, 0, NULL, 0, &bytesReturned, NULL);
                    DeviceIoControl(hTargetVolume, IOCTL_VOLUME_GET_VOLUME_DISK_EXTENTS, NULL, 0, &volumeDiskExtents, sizeof volumeDiskExtents, &bytesReturned, NULL);
                    wsprintf(szDevicePathName, TEXT("\\\\.\\PhysicalDrive%d"), volumeDiskExtents.Extents[0].DiskNumber);
                    hTargetDevice = CreateFile(szDevicePathName, GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);

                    if (hTargetDevice != INVALID_HANDLE_VALUE) {
                        LARGE_INTEGER fileSize;
                        DWORD totalNumberOfBytesWritten = 0;
                        DWORD pbmPos = 0;
                        LPVOID lpBuffer = HeapAlloc(GetProcessHeap(), 0, BUFFER_SIZE);

                        GetFileSizeEx(hSourceFile, &fileSize);

                        while (TRUE) {
                            DWORD numberOfBytesRead;

                            if (ReadFile(hSourceFile, lpBuffer, BUFFER_SIZE, &numberOfBytesRead, NULL)) {
                                if (numberOfBytesRead == 0) {
                                    MessageBox(hwndDlg, TEXT("Source file successfully written to target device."), TEXT("Success"), MB_ICONINFORMATION);
                                    break;
                                } else {
                                    DWORD numberOfBytesWritten;

                                    if (WriteFile(hTargetDevice, lpBuffer, numberOfBytesRead, &numberOfBytesWritten, NULL)) {
                                        DWORD nextPbmPos;

                                        totalNumberOfBytesWritten += numberOfBytesWritten;
                                        nextPbmPos = (DWORD) (100.f * totalNumberOfBytesWritten / fileSize.QuadPart);

                                        if (pbmPos < nextPbmPos) {
                                            SendDlgItemMessage(hwndDlg, IDC_MAINDLG_PROGRESSBAR, PBM_SETPOS, nextPbmPos, 0);
                                            pbmPos = nextPbmPos;
                                        }
                                    } else {
                                        MessageBox(hwndDlg, TEXT("An error occurred while writing to the target device."), TEXT("Error"), MB_ICONERROR);
                                        break;
                                    }
                                }
                            } else {
                                MessageBox(hwndDlg, TEXT("An error occurred while reading the source file."), TEXT("Error"), MB_ICONERROR);
                                break;
                            }
                        }

                        HeapFree(GetProcessHeap(), 0, lpBuffer);
                        CloseHandle(hTargetDevice);
                    } else {
                        MessageBox(hwndDlg, TEXT("An error occurred while opening the target device."), TEXT("Error"), MB_ICONERROR);
                    }
                } else {
                    MessageBox(hwndDlg, TEXT("An error occurred while locking the target volume."), TEXT("Error"), MB_ICONERROR); 
                }
            } else {
                MessageBox(hwndDlg, TEXT("An error occurred while opening the target volume."), TEXT("Error"), MB_ICONERROR);
            }

            CloseHandle(hTargetVolume);
        } else {
            MessageBox(hwndDlg, TEXT("Please select a target."), TEXT("Error"), MB_ICONEXCLAMATION);
        }

        CloseHandle(hSourceFile);
    } else {
        MessageBox(hwndDlg, TEXT("Please select a readable source file."), TEXT("Error"), MB_ICONEXCLAMATION);
    }

    EnableWindow(GetDlgItem(hwndDlg, IDC_MAINDLG_SOURCE), TRUE);
    EnableWindow(GetDlgItem(hwndDlg, IDC_MAINDLG_SELECT), TRUE);
    EnableWindow(GetDlgItem(hwndDlg, IDC_MAINDLG_TARGET_LIST), TRUE);
    EnableWindow(GetDlgItem(hwndDlg, IDC_MAINDLG_REFRESH), TRUE);
    SendDlgItemMessage(hwndDlg, IDC_MAINDLG_PROGRESSBAR, PBM_SETPOS, 0, 0);
    EnableWindow(GetDlgItem(hwndDlg, IDC_MAINDLG_WRITE), TRUE);
    
    return 0;
}

INT_PTR MainDlgWriteClick(HWND hwndDlg) {
    if (MessageBox(hwndDlg, TEXT("All data on the target device will be lost. Confirm?"), TEXT("Warning"), MB_YESNO | MB_ICONINFORMATION) == IDYES) {
        EnableWindow(GetDlgItem(hwndDlg, IDC_MAINDLG_SOURCE), FALSE);
        EnableWindow(GetDlgItem(hwndDlg, IDC_MAINDLG_SELECT), FALSE);
        EnableWindow(GetDlgItem(hwndDlg, IDC_MAINDLG_TARGET_LIST), FALSE);
        EnableWindow(GetDlgItem(hwndDlg, IDC_MAINDLG_REFRESH), FALSE);
        SendDlgItemMessage(hwndDlg, IDC_MAINDLG_PROGRESSBAR, PBM_SETPOS, 0, 0);
        EnableWindow(GetDlgItem(hwndDlg, IDC_MAINDLG_WRITE), FALSE);
        CloseHandle(CreateThread(NULL, 0, ThreadRoutine, hwndDlg, 0, NULL));    
    }

    return TRUE;
}