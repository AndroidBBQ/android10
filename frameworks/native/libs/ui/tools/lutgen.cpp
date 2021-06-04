/*
 * Copyright 2017 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <algorithm>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <string>

#include <getopt.h>

#include <ui/ColorSpace.h>

using namespace android;
using namespace std;

uint32_t gSize = 32;
ColorSpace gColorSpaceSrc = ColorSpace::DisplayP3();
ColorSpace gColorSpaceDst = ColorSpace::extendedSRGB();
string gNameSrc = "DisplayP3";
string gNameDst = "extendedSRGB";

static void printHelp() {
    cout << "lutgen -d SIZE -s SOURCE -t TARGET <lut file>" << endl;
    cout << endl;
    cout << "Generate a 3D LUT to convert between two color spaces." << endl;
    cout << endl;
    cout << "If <lut file> ends in .inc, data is generated without the array declaration." << endl;
    cout << endl;
    cout << "Options:" << endl;
    cout << "  --help, -h" << endl;
    cout << "    print this message" << endl;
    cout << "  --dimension=, -d" << endl;
    cout << "    the dimension of the 3D LUT. Example: 17 for a 17x17x17 LUT. 32 by default" << endl;
    cout << "  --source=COLORSPACE, -s" << endl;
    cout << "    the source color space, see below for available names. DisplayP3 by default" << endl;
    cout << "  --target=COLORSPACE, -t" << endl;
    cout << "    the target color space, see below for available names. extendedSRGB by default" << endl;
    cout << endl;
    cout << "Colorspace names:" << endl;
    cout << "    sRGB" << endl;
    cout << "    linearSRGB" << endl;
    cout << "    extendedSRGB" << endl;
    cout << "    linearExtendedSRGB" << endl;
    cout << "    NTSC" << endl;
    cout << "    BT709" << endl;
    cout << "    BT2020" << endl;
    cout << "    AdobeRGB" << endl;
    cout << "    ProPhotoRGB" << endl;
    cout << "    DisplayP3" << endl;
    cout << "    DCIP3" << endl;
    cout << "    ACES" << endl;
    cout << "    ACEScg" << endl;
}

static const ColorSpace findColorSpace(const string& name) {
    if (name == "linearSRGB") return ColorSpace::linearSRGB();
    if (name == "extendedSRGB") return ColorSpace::extendedSRGB();
    if (name == "linearExtendedSRGB") return ColorSpace::linearExtendedSRGB();
    if (name == "NTSC") return ColorSpace::NTSC();
    if (name == "BT709") return ColorSpace::BT709();
    if (name == "BT2020") return ColorSpace::BT2020();
    if (name == "AdobeRGB") return ColorSpace::AdobeRGB();
    if (name == "ProPhotoRGB") return ColorSpace::ProPhotoRGB();
    if (name == "DisplayP3") return ColorSpace::DisplayP3();
    if (name == "DCIP3") return ColorSpace::DCIP3();
    if (name == "ACES") return ColorSpace::ACES();
    if (name == "ACEScg") return ColorSpace::ACEScg();
    return ColorSpace::sRGB();
}

static int handleCommandLineArgments(int argc, char* argv[]) {
    static constexpr const char* OPTSTR = "h:d:s:t:";
    static const struct option OPTIONS[] = {
            { "help",       no_argument,       nullptr, 'h' },
            { "dimension",  required_argument, nullptr, 'd' },
            { "source",     required_argument, nullptr, 's' },
            { "target",     required_argument, nullptr, 't' },
            { nullptr, 0, nullptr, 0 }  // termination of the option list
    };

    int opt;
    int index = 0;

    while ((opt = getopt_long(argc, argv, OPTSTR, OPTIONS, &index)) >= 0) {
        string arg(optarg ? optarg : "");
        switch (opt) {
            default:
            case 'h':
                printHelp();
                exit(0);
                break;
            case 'd':
                gSize = max(2, min(stoi(arg), 256));
                break;
            case 's':
                gNameSrc = arg;
                gColorSpaceSrc = findColorSpace(arg);
                break;
            case 't':
                gNameDst = arg;
                gColorSpaceDst = findColorSpace(arg);
                break;
        }
    }

    return optind;
}

int main(int argc, char* argv[]) {
    int optionIndex = handleCommandLineArgments(argc, argv);
    int numArgs = argc - optionIndex;

    if (numArgs < 1) {
        printHelp();
        return 1;
    }
    
    bool isInclude = false;

    string filename(argv[optionIndex]);
    size_t index = filename.find_last_of('.');

    if (index != string::npos) {
        string extension(filename.substr(index + 1));
        isInclude = extension == "inc";
    }

    ofstream outputStream(filename, ios::trunc);
    if (outputStream.good()) {
        auto lut = ColorSpace::createLUT(gSize, gColorSpaceSrc, gColorSpaceDst);
        auto data = lut.get();

        outputStream << "// generated with lutgen " << filename.c_str() << endl;
        outputStream << "// 3D LUT stored as an RGB16F texture, in GL order" << endl;
        outputStream << "// Size is " << gSize << "x" << gSize << "x" << gSize << endl;

        string src(gNameSrc);
        string dst(gNameDst);

        if (!isInclude) {
            transform(src.begin(), src.end(), src.begin(), ::toupper);
            transform(dst.begin(), dst.end(), dst.begin(), ::toupper);

            outputStream << "const size_t LUT_" << src << "_TO_" << dst << "_SIZE = " << gSize << endl;
            outputStream << "const uint16_t LUT_" << src << "_TO_" << dst << "[] = {";
        } else {
            outputStream << "// From " << src << " to " << dst << endl;
        }

        for (size_t z = 0; z < gSize; z++) {
            for (size_t y = 0; y < gSize; y++) {
                for (size_t x = 0; x < gSize; x++) {
                    if (x % 4 == 0) outputStream << endl << "    ";

                    half3 rgb = half3(*data++);

                    const uint16_t r = rgb.r.getBits();
                    const uint16_t g = rgb.g.getBits();
                    const uint16_t b = rgb.b.getBits();

                    outputStream << "0x" << setfill('0') << setw(4) << hex << r << ", ";
                    outputStream << "0x" << setfill('0') << setw(4) << hex << g << ", ";
                    outputStream << "0x" << setfill('0') << setw(4) << hex << b << ", ";
                }
            }
        }

        if (!isInclude) {
            outputStream << endl << "}; // end LUT" << endl;
        }

        outputStream << endl;
        outputStream.flush();
        outputStream.close();
    } else {
        cerr << "Could not write to file: " << filename << endl;
        return 1;

    }

    return 0;
}
