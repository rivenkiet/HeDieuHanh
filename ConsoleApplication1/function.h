#pragma once
#include <windows.h>
#include <stdio.h>
#include <string>
#include <math.h>
#include <iostream>
#include <iomanip>
using namespace std;

int ReadSector(LPCWSTR  drive, int readPoint, BYTE sector[]);
string decToHex(int decimalNumber);
int hexToDec(string hexString);
int getValueOffset(BYTE* sector, string offset, int size);
int bytesPerSector(BYTE sector[512]);
int sectorsPerCluster(BYTE sector[512]);
int reversedSector(BYTE sector[512]);
int numOf_FATtbl(BYTE sector[512]);
int FAT_volume(BYTE sector[512]);
int totalSector(BYTE sector[512]);
int clusterToSector(BYTE sector[512], unsigned int cluster);