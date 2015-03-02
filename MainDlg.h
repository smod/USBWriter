/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * <etienne.doms@gmail.com> wrote this file. As long as you retain this notice
 * you can do whatever you want with this stuff. If we meet some day, and you
 * think this stuff is worth it, you can buy me a beer in return. Etienne Doms
 * ----------------------------------------------------------------------------
 */

#ifndef MAINDLG_H
#define MAINDLG_H

#include <windows.h>

#ifndef DBT_DEVICEARRIVAL
#define DBT_DEVICEARRIVAL 0x8000
#endif
#ifndef DBT_DEVICEREMOVECOMPLETE
#define DBT_DEVICEREMOVECOMPLETE 0x8004
#endif

INT_PTR MainDlg(HINSTANCE hInstance);

#endif
