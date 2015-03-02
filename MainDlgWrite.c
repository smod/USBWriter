/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * <etienne.doms@gmail.com> wrote this file. As long as you retain this notice
 * you can do whatever you want with this stuff. If we meet some day, and you
 * think this stuff is worth it, you can buy me a beer in return. Etienne Doms
 * ----------------------------------------------------------------------------
 * 2013/05/16 <pierrevr@mindgoo.be> - Changed buffer size to reduce I/O operations
 * 2014/02/08 <mail@michael-kaufmann.ch> - Calculate the progress correctly for files > 4 GB
 * 2015/02/16 <mail@michael-kaufmann.ch> -  Check that the target device is removeable before writing to it
 */

#include "MainDlgWrite.h"
#include "resource.h"
#include <commctrl.h>
#include <winioctl.h>

#define BUFFER_SIZE (32 * 1024 * 1024)

static DWORD WINAPI ThreadRoutine(LPVOID lpParam) {
    HWND hwndDlg = (HWND) lpParam;
    TCHAR szFilePathName[MAX_PATH];
    HANDLE hSourceFile;

    GetDlgItemText(hwndDlg, IDC_MAINDLG_SOURCE, szFilePathName, sizeof(szFilePathName) / sizeof(szFilePathName[0]));
    hSourceFile = CreateFile(szFilePathName, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

    if (hSourceFile != INVALID_HANDLE_VALUE) {
        LRESULT index = SendDlgItemMessage(hwndDlg, IDC_MAINDLG_TARGET_LIST, CB_GETCURSEL, 0, 0);

        if (index != CB_ERR) {
            TCHAR driveLetter;
            TCHAR szRootPathName[sizeof "\\\\.\\A:\\"];
            TCHAR szVolumePathName[sizeof "\\\\.\\A:"];

            driveLetter = (TCHAR) SendDlgItemMessage(hwndDlg, IDC_MAINDLG_TARGET_LIST, CB_GETITEMDATA, index, 0);

            wsprintf(szRootPathName, TEXT("\\\\.\\%c:\\"), driveLetter);
            wsprintf(szVolumePathName, TEXT("\\\\.\\%c:"), driveLetter);

            if (GetDriveType(szRootPathName) == DRIVE_REMOVABLE) {
                HANDLE hTargetVolume = CreateFile(szVolumePathName, GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);

                if (hTargetVolume != INVALID_HANDLE_VALUE) {
                    DWORD bytesReturned;

                    if (DeviceIoControl(hTargetVolume, FSCTL_LOCK_VOLUME, NULL, 0, NULL, 0, &bytesReturned, NULL)) {
                        VOLUME_DISK_EXTENTS volumeDiskExtents;
                        TCHAR szDevicePathName[sizeof "\\\\.\\PhysicalDrive9999999999"];
                        HANDLE hTargetDevice;

                        DeviceIoControl(hTargetVolume, FSCTL_DISMOUNT_VOLUME, NULL, 0, NULL, 0, &bytesReturned, NULL);
                        DeviceIoControl(hTargetVolume, IOCTL_VOLUME_GET_VOLUME_DISK_EXTENTS, NULL, 0, &volumeDiskExtents, sizeof volumeDiskExtents, &bytesReturned, NULL);
                        wsprintf(szDevicePathName, TEXT("\\\\.\\PhysicalDrive%lu"), (unsigned long)volumeDiskExtents.Extents[0].DiskNumber);
                        hTargetDevice = CreateFile(szDevicePathName, GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);

                        if (hTargetDevice != INVALID_HANDLE_VALUE) {
                            LARGE_INTEGER fileSize;
                            LARGE_INTEGER totalNumberOfBytesWritten;
                            DWORD pbmPos = 0;

                            totalNumberOfBytesWritten.QuadPart = 0;
                            GetFileSizeEx(hSourceFile, &fileSize);

                            for(;;) {
                                static CHAR lpBuffer[BUFFER_SIZE];
                                DWORD numberOfBytesRead;

                                if (ReadFile(hSourceFile, lpBuffer, BUFFER_SIZE, &numberOfBytesRead, NULL)) {
                                    if (numberOfBytesRead == 0) {
                                        MessageBox(hwndDlg, TEXT("Source file successfully written to target device."), TEXT("Success"), MB_ICONINFORMATION);
                                        break;
                                    } else {
                                        DWORD numberOfBytesWritten;

                                        if (WriteFile(hTargetDevice, lpBuffer, numberOfBytesRead, &numberOfBytesWritten, NULL)) {
                                            DWORD nextPbmPos;

                                            totalNumberOfBytesWritten.QuadPart += numberOfBytesWritten;
                                            nextPbmPos = (DWORD) (100.f * totalNumberOfBytesWritten.QuadPart / fileSize.QuadPart);

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
                MessageBox(hwndDlg, TEXT("The target is invalid. Please refresh the list of target devices."), TEXT("Error"), MB_ICONEXCLAMATION);
            }
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
    EnableWindow(GetDlgItem(hwndDlg, IDC_MAINDLG_CLOSE), TRUE);
    
    return 0;
}

INT_PTR MainDlgWriteClick(HWND hwndDlg) {
    if (MessageBox(hwndDlg, TEXT("All data on the target device will be lost. Confirm?"), TEXT("Warning"), MB_YESNO | MB_ICONEXCLAMATION) == IDYES) {
        EnableWindow(GetDlgItem(hwndDlg, IDC_MAINDLG_SOURCE), FALSE);
        EnableWindow(GetDlgItem(hwndDlg, IDC_MAINDLG_SELECT), FALSE);
        EnableWindow(GetDlgItem(hwndDlg, IDC_MAINDLG_TARGET_LIST), FALSE);
        EnableWindow(GetDlgItem(hwndDlg, IDC_MAINDLG_REFRESH), FALSE);
        SendDlgItemMessage(hwndDlg, IDC_MAINDLG_PROGRESSBAR, PBM_SETPOS, 0, 0);
        EnableWindow(GetDlgItem(hwndDlg, IDC_MAINDLG_WRITE), FALSE);
        EnableWindow(GetDlgItem(hwndDlg, IDC_MAINDLG_CLOSE), FALSE);
        CloseHandle(CreateThread(NULL, 0, ThreadRoutine, hwndDlg, 0, NULL));    
    }

    return TRUE;
}
