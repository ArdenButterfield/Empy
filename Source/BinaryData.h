/* =========================================================================================

   This is an auto-generated file: Any edits you make may be overwritten!

*/

#pragma once

namespace BinaryData
{
    extern const char*   GontserratRegular_ttf;
    const int            GontserratRegular_ttfSize = 163692;

    extern const char*   bedsteadboldsemicondensed_otf;
    const int            bedsteadboldsemicondensed_otfSize = 187220;

    extern const char*   bedsteadsemicondensed_otf;
    const int            bedsteadsemicondensed_otfSize = 186720;

    extern const char*   bedsteadboldextended_otf;
    const int            bedsteadboldextended_otfSize = 170768;

    // Number of elements in the namedResourceList and originalFileNames arrays.
    const int namedResourceListSize = 4;

    // Points to the start of a list of resource names.
    extern const char* namedResourceList[];

    // Points to the start of a list of resource filenames.
    extern const char* originalFilenames[];

    // If you provide the name of one of the binary resource variables above, this function will
    // return the corresponding data and its size (or a null pointer if the name isn't found).
    const char* getNamedResource (const char* resourceNameUTF8, int& dataSizeInBytes);

    // If you provide the name of one of the binary resource variables above, this function will
    // return the corresponding original, non-mangled filename (or a null pointer if the name isn't found).
    const char* getNamedResourceOriginalFilename (const char* resourceNameUTF8);
}
