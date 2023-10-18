#include "function.h"

int ReadSector(LPCWSTR  drive, int readPoint, BYTE sector[])
{
    int retCode = 0;
    DWORD bytesRead;
    HANDLE device = NULL;

    device = CreateFile(drive,    // Drive to open
        GENERIC_READ,           // Access mode
        FILE_SHARE_READ | FILE_SHARE_WRITE,        // Share Mode
        NULL,                   // Security Descriptor
        OPEN_EXISTING,          // How to create
        0,                      // File attributes
        NULL);                  // Handle to template

    if (device == INVALID_HANDLE_VALUE) // Open Error
    {
        printf("CreateFile: %u\n", GetLastError());
        return 1;
    }

    SetFilePointer(device, readPoint, NULL, FILE_BEGIN);//Set a Point to Read

    if (!ReadFile(device, sector, 512, &bytesRead, NULL))
    {
        printf("ReadFile: %u\n", GetLastError());
        return 1;
    }
    else
    {
        printf("Success!\n");
        return 0;
    }
}




string decToHex(int decimalNumber) {
    string hexNumber = "";
    while (decimalNumber > 0) {
        int remainder = decimalNumber % 16;
        if (remainder < 10) {
            hexNumber = char(remainder + '0') + hexNumber;
        }
        else {
            hexNumber = char(remainder - 10 + 'A') + hexNumber;
        }
        decimalNumber /= 16;
    }
    if (hexNumber == "") {
        return "00";
    }
    else if (hexNumber.length() == 1) {
        return "0" + hexNumber;
    }
    return hexNumber;
}

int hexCharToInt(char hexChar) {
    if (hexChar >= '0' && hexChar <= '9') {
        return hexChar - '0';
    }
    else if (hexChar >= 'A' && hexChar <= 'F') {
        return hexChar - 'A' + 10;
    }
    else if (hexChar >= 'a' && hexChar <= 'f') {
        return hexChar - 'a' + 10;
    }
    return -1; // Ký tự không hợp lệ
}

int hexToDec(string hexString) {
    int decimalNumber = 0;
    int j = hexString.length() - 1;
    for (int i = 0; i < hexString.length(); ++i) {
        int hexValue = hexCharToInt(hexString[i]);
        if (hexValue == -1) {
            std::cout << "Ký tự không hợp lệ: " << hexString[i] << std::endl;
            return -1; // Trả về -1 nếu có ký tự không hợp lệ
        }
        decimalNumber += hexValue * pow(16, j--);
    }
    return decimalNumber;
}


int getValueOffset(BYTE* sector, string offset, int size) {
    int start = hexToDec(offset);
    int pos = start + size - 1;
    string hexString = "";
    while (pos >= start) {
        hexString += decToHex(sector[pos--]);
    }

    return hexToDec(hexString);
}


int bytesPerSector(BYTE sector[512])
{
    return getValueOffset(sector, "0B", 2); // lay 2 bytes tai offset 0B
}

// tinh so sector moi cluster
int sectorsPerCluster(BYTE sector[512])
{
    return getValueOffset(sector, "0D", 1); // lay 1 byte tai offset 0D
}

// tinh so sector truoc vung FAT (thuoc boot sector)
int reversedSector(BYTE sector[512])
{
    return getValueOffset(sector, "0E", 2); // lay 2 byte tai offset 0E
}

// tinh so bang FAT
int numOf_FATtbl(BYTE sector[512])
{
    return getValueOffset(sector, "10", 2); // lay 2 byte tai offset 10
}
// kich thuoc 1 bang FAT
int FAT_volume(BYTE sector[512])
{
    return getValueOffset(sector, "24", 4); // lay 4 byte tai offset 24
}

// tinh tong sector tren volume
int totalSector(BYTE sector[512])
{
    return getValueOffset(sector, "20", 4); // lay 4 byte tai offset 20
}
// Chuyen doi cluster - sector
int clusterToSector(BYTE sector[512], unsigned int cluster)
{
    unsigned int reversedSctr = reversedSector(sector);
    unsigned int num_FATtlb = numOf_FATtbl(sector);
    unsigned int fat_vol = FAT_volume(sector);
    unsigned int sec_per_clus = sectorsPerCluster(sector);
    return reversedSctr + num_FATtlb * fat_vol + (cluster - 2) * sec_per_clus;
}