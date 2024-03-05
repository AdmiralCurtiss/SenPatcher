#include <cstdio>
#include <filesystem>
#include <vector>

#include "file.h"
#include "sha1.h"

int main(int argc, char** argv) {
    std::vector<char> tmp;
    char buffer[500];

    for (int i = 0; i <= 1024; ++i) {
        auto hash = SenPatcher::CalculateSHA1(tmp.data(), tmp.size());

        sprintf(buffer,
                "sha1test_%04d_%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%"
                "02x%02x%02x%02x.bin",
                i,
                ((int)hash.Hash[0]) & 0xff,
                ((int)hash.Hash[1]) & 0xff,
                ((int)hash.Hash[2]) & 0xff,
                ((int)hash.Hash[3]) & 0xff,
                ((int)hash.Hash[4]) & 0xff,
                ((int)hash.Hash[5]) & 0xff,
                ((int)hash.Hash[6]) & 0xff,
                ((int)hash.Hash[7]) & 0xff,
                ((int)hash.Hash[8]) & 0xff,
                ((int)hash.Hash[9]) & 0xff,
                ((int)hash.Hash[10]) & 0xff,
                ((int)hash.Hash[11]) & 0xff,
                ((int)hash.Hash[12]) & 0xff,
                ((int)hash.Hash[13]) & 0xff,
                ((int)hash.Hash[14]) & 0xff,
                ((int)hash.Hash[15]) & 0xff,
                ((int)hash.Hash[16]) & 0xff,
                ((int)hash.Hash[17]) & 0xff,
                ((int)hash.Hash[18]) & 0xff,
                ((int)hash.Hash[19]) & 0xff);
        SenPatcher::IO::File f(std::filesystem::path(buffer), SenPatcher::IO::OpenMode::Write);
        f.Write(tmp.data(), tmp.size());

        tmp.push_back((char)i);
    }

    return 0;
}
