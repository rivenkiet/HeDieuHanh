#include <windows.h>
#include "function.h"
#include <stdio.h>
#include <string>
#include <math.h>
#include <iostream>
#include <iomanip>
#include <vector>
#include <sstream>
using namespace std;

#define dir "DIR" //directory
#define unk "UNK" //unknown
const wchar_t* CURRENT_VOLUME = L"\\\\.\\E:";
using namespace std;

struct Entries
{
    vector<string> fileName;
    vector<string> fileType;
    vector<unsigned int> r_cluster;
    vector<unsigned int> size;
};
Entries entries = Entries();


string HextoStr(const string hex) // da reverse chuoi hex theo value offset
{
    string result;
    for (int i = (int)hex.length(); i >= 2; i -= 2)
    {
        string c_part = hex.substr(i - 2, 2);
        char ch = (char)stoul(c_part, nullptr, 16);
        if (ch != ' ')
        {
            result += ch;
        }
    }
    return result;
}

string valueOffset(BYTE sector[512], string hex, unsigned int byte_num) // tai offset 'hex', lay byte_num bytes tu phai sang
{
    string result;
    int j = (int)hexToDec(hex);
    for (int i = j + byte_num - 1; i >= j; --i)
    {
        result.append(decToHex(short(sector[i])));
    }
    return result;
}



void readEntry(BYTE e_sector[32], Entries& entries)
{
    string offset_0 = valueOffset(e_sector, "00", 1);
    string offset_B = valueOffset(e_sector, "0B", 1);
    if (offset_0 == "E5" || offset_0 == "00" || offset_B == "0F" || offset_B == "08")
    {
        return;
    }
    unsigned int begin_clus = hexToDec(valueOffset(e_sector, "14", 2) + valueOffset(e_sector, "1A", 2));
    // 4 bytes high word + low word

    if (offset_B == "10") // folder - subdir
    {
        if (offset_0 == "2E" && valueOffset(e_sector, "01", 1) == "2E") // .. mang thong tin thu muc cha
        {
            entries.fileName.push_back("..");

        }
        else if (offset_0 == "2E" && valueOffset(e_sector, "01", 1) == "20") // . mang thong tin thu muc hien tai
        {
            entries.fileName.push_back(".");
        }
        else
        {
            entries.fileName.push_back(HextoStr(valueOffset(e_sector, "00", 8)));
        }
        entries.fileType.push_back(dir);
        begin_clus = (begin_clus == 0) ? 2 : begin_clus; //neu bang 0 thi set lai default cluster = 2, khong thi giu nguyen
    }
    else if (offset_B == "20") // file - archive
    {
        entries.fileName.push_back(HextoStr(valueOffset(e_sector, "00", 8)));
        entries.fileType.push_back(HextoStr(valueOffset(e_sector, "08", 3)));
    }
    else
    {
        entries.fileName.push_back(HextoStr(valueOffset(e_sector, "00", 8)));
        entries.fileType.push_back(unk);
    }
    entries.r_cluster.push_back(begin_clus);
    entries.size.push_back(hexToDec(valueOffset(e_sector, "1C", 4)));
}

void printEntries(Entries entries)
{
    cout << "TYPE       " << "SIZE(BYTES)       " << "FILENAME      " << "1ST_CLUSTER" << endl;
    for (int i = 0; i < entries.fileName.size(); ++i)
    {
        cout << entries.fileType[i] << setw(12) << entries.size[i] << setw(20)
            << entries.fileName[i] << setw(12) << entries.r_cluster[i] << endl;
    }
}

void readDir(Entries& entries, unsigned int read_point)
{
    BYTE cur_sector[512];
    ReadSector(CURRENT_VOLUME, read_point * 512, cur_sector);

    for (int i = 0; i < 16; ++i)
    {
        BYTE e_sector[32];
        int k = 0;
        for (int j = 32 * i; j < (32 * i) + 32; ++j)
        {
            e_sector[k++] = cur_sector[j];
        }
        if (valueOffset(e_sector, "00", 1) == "00")
        {
            break;
        }
        readEntry(e_sector, entries);
    }
}

vector<string> split(const string& s, char delim = ' ')
{
    vector<string> result;
    stringstream ss(s);
    string item;

    while (getline(ss, item, delim))
    {
        result.push_back(item);
    }

    return result;
}

void showPath(vector<string> PATH)
{
    for (int i = 0; i < PATH.size(); ++i)
    {
        cout << PATH[i] + "\\";
    }
    cout << ">";
}

void handle_txtFile(BYTE content[512], bool& is_eof)
{
    for (int i = 0; i < 512; ++i)
    {
        if (decToHex((short)content[i]) == "00")
        {
            is_eof = true;
            break;
        }
        else
        {
            cout << (char)stoul(decToHex((short)content[i]), nullptr, 16);
        }
    }
    cout << endl;
}

// function chuc nang chinh
void mfunction(BYTE bootsector[512], vector<string>& PATH, unsigned int& cur_cluster, unsigned& cur_sector)
{
    while (true)
    {
        showPath(PATH);
        string command;
        getline(cin, command);
        vector<string> args = split(command);
        entries = Entries();
        readDir(entries, cur_sector);

        if (args[0] == "exit")
        {
            break;
        }
        else if (args[0] == "dir")
        {
            printEntries(entries);
        }
        else if (args[0] == "cd" && args.size() == 2)
        {
            if (args[1] == ".")
            {
                continue;
            }
            if (args[1] == ".." && PATH.size() != 1)
            {
                PATH.pop_back();
                auto it = find(entries.fileName.begin(), entries.fileName.end(), ".."); // quay lai tim thu muc cha
                if (it != entries.fileName.end())
                {
                    cur_cluster = entries.r_cluster[it - entries.fileName.begin()];
                    cur_sector = clusterToSector(bootsector, cur_cluster);
                }
            }
            else // chuyen den mot thu muc
            {
                auto it = find(entries.fileName.begin(), entries.fileName.end(), args[1]);
                if (it != entries.fileName.end() && entries.fileType[it - entries.fileName.begin()] == dir)
                {                                           // xet co phai dir khong
                    PATH.push_back(args[1]);
                    cur_cluster = entries.r_cluster[it - entries.fileName.begin()];
                    cur_sector = clusterToSector(bootsector, cur_cluster);
                }
                else
                {
                    cout << "Cannot find the directory" << endl;
                }
            }
        }
        else if (args[0] == "open" && args.size() == 2)
        {
            auto it = find(entries.fileName.begin(), entries.fileName.end(), args[1]);
            if (it != entries.fileName.end() && entries.fileType[it - entries.fileName.begin()] == "TXT")
            {
                unsigned content_clus = entries.r_cluster[it - entries.fileName.begin()];
                unsigned content_sector = clusterToSector(bootsector, content_clus);
                bool is_eof = false;
                for (int i = 0; i < 8; ++i)
                {
                    BYTE content[512];
                    ReadSector(CURRENT_VOLUME, content_sector * 512, content);
                    handle_txtFile(content, is_eof);
                    if (is_eof)
                    {
                        break;
                    }
                    else
                    {
                        content_sector += 1;
                    }
                }
            }
            else
            {
                cout << "Cannot open file because it does not exist or file type is not supported" << endl;
            }
        }
        else
        {
            cout << "Unrecognized command" << endl;
        }
    }
}



int main(int argc, char** argv)
{
    //int arr[200];
    //int size;
    //unsigned int begin_RDET;
    //byte sector[512];
    //ReadSector(L"\\\\.\\E:", 0, sector);
    //for (int i = 0; i < 512; i++) {
    //    if (i % 16 == 0) {
    //        printf("\n");
    //    }
    //    cout << decToHex(sector[i]) << " ";
    //}
    //cout << endl;
    //cout << "So byte tren 1 sector          : " << setw(10) << bytesPerSector(sector) << " byte(s) " << endl;
    //cout << "So sector tren 1 cluster       : " << setw(10) << sectorsPerCluster(sector) << " sector(s)" << endl;
    //cout << "So sector truoc bang FAT       : " << setw(10) << reversedSector(sector) << " sector(s)" << endl;
    //cout << "So luong bang FAT              : " << setw(10) << numOf_FATtbl(sector) << " table(s)" << endl;
    //cout << "Kich thuoc moi bang FAT        : " << setw(10) << FAT_volume(sector) << " sector(s)" << endl;
    //begin_RDET = getValueOffset(sector, "2C", 4); // cluster bat dau cua RDET
    //cout << "Sector bat dau cua RDET        : " << setw(10) << clusterToSector(sector, begin_RDET) << endl;
    //cout << "Sector chua thong tin phu      : " << setw(10) << getValueOffset(sector, "30", 2) << endl; // sector chua thong tin phu tai offset 30
    //cout << "Sector chua backup Boot Sector : " << setw(10) << getValueOffset(sector, "32", 2) << endl; // sector chua ban luu tai offset 32
    //cout << "Kich thuoc cua volume hien tai : " << setw(10) << totalSector(sector) << " sector(s) ~ "
    //    << static_cast<float>(totalSector(sector)) / 2097152 << " GB" << endl;
    BYTE bootsector[512];
    wstring WDRIVE(L"\\\\.\\E:");
    string DRIVE(WDRIVE.end() - 2, WDRIVE.end()); // get name of current drive
    vector<string> PATH;
    PATH.push_back(DRIVE);
    ReadSector(L"\\\\.\\E:", 0, bootsector);
    unsigned int cur_cluster = getValueOffset(bootsector, "2C", 4);
    unsigned int cur_sector = clusterToSector(bootsector, cur_cluster);
    mfunction(bootsector, PATH, cur_cluster, cur_sector);

}