#!/usr/bin/env python3

# Copyright (C) 2018 The Android Open Source Project
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

import os
import sys

# list (pretty, cpp)
data_types = [
    ("Int32", "int32_t"),
    ("Uint32", "uint32_t"),
    ("Int64", "int64_t"),
    ("Uint64", "uint64_t"),
    ("Float", "float"),
    ("Double", "double"),
    ("Bool", "bool"),
    ("Char", "char16_t"),
    ("Byte", "int8_t"),
]

non_contiguously_addressable = {"Bool"}

def replaceFileTags(path, content, start_tag, end_tag):
    print("Updating", path)
    with open(path, "r+") as f:
        lines = f.readlines()

        start = lines.index("// @" + start_tag + "\n")
        end = lines.index("// @" + end_tag + "\n")

        if end <= start or start < 0 or end < 0:
            print("Failed to find tags in", path)
            exit(1)

        f.seek(0)
        f.write("".join(lines[:start+1]) + content + "".join(lines[end:]))
        f.truncate()

def main():
    if len(sys.argv) != 1:
        print("No arguments.")
        exit(1)

    ABT = os.environ.get('ANDROID_BUILD_TOP', None)
    if ABT is None:
        print("Can't get ANDROID_BUILD_TOP. Lunch?")
        exit(1)
    ROOT = ABT + "/frameworks/native/libs/binder/ndk/"

    print("Updating auto-generated code")

    pre_header = ""
    header = ""
    source = ""
    cpp_helper = ""

    for pretty, cpp in data_types:
        header += "/**\n"
        header += " * Writes " + cpp + " value to the next location in a non-null parcel.\n"
        header += " *\n"
        header += " * \\param parcel the parcel to write to.\n"
        header += " * \\param value the value to write to the parcel.\n"
        header += " *\n"
        header += " * \\return STATUS_OK on successful write.\n"
        header += " */\n"
        header += "binder_status_t AParcel_write" + pretty + "(AParcel* parcel, " + cpp + " value) __INTRODUCED_IN(29);\n\n"
        source += "binder_status_t AParcel_write" + pretty + "(AParcel* parcel, " + cpp + " value) {\n"
        source += "    status_t status = parcel->get()->write" + pretty + "(value);\n"
        source += "    return PruneStatusT(status);\n"
        source += "}\n\n"

    for pretty, cpp in data_types:
        header += "/**\n"
        header += " * Reads into " + cpp + " value from the next location in a non-null parcel.\n"
        header += " *\n"
        header += " * \\param parcel the parcel to read from.\n"
        header += " * \\param value the value to read from the parcel.\n"
        header += " *\n"
        header += " * \\return STATUS_OK on successful read.\n"
        header += " */\n"
        header += "binder_status_t AParcel_read" + pretty + "(const AParcel* parcel, " + cpp + "* value) __INTRODUCED_IN(29);\n\n"
        source += "binder_status_t AParcel_read" + pretty + "(const AParcel* parcel, " + cpp + "* value) {\n"
        source += "    status_t status = parcel->get()->read" + pretty + "(value);\n"
        source += "    return PruneStatusT(status);\n"
        source += "}\n\n"

    for pretty, cpp in data_types:
        nca = pretty in non_contiguously_addressable

        arg_types = "const " + cpp + "* arrayData, int32_t length"
        if nca: arg_types = "const void* arrayData, int32_t length, AParcel_" + pretty.lower() + "ArrayGetter getter"
        args = "arrayData, length"
        if nca: args = "arrayData, length, getter, &Parcel::write" + pretty

        header += "/**\n"
        header += " * Writes an array of " + cpp + " to the next location in a non-null parcel.\n"
        if nca:
            header += " *\n"
            header += " * getter(arrayData, i) will be called for each i in [0, length) in order to get the underlying values to write "
            header += "to the parcel.\n"
        header += " *\n"
        header += " * \\param parcel the parcel to write to.\n"
        if nca:
            header += " * \\param arrayData some external representation of an array.\n"
            header += " * \\param length the length of arrayData (or -1 if this represents a null array).\n"
            header += " * \\param getter the callback to retrieve data at specific locations in the array.\n"
        else:
            header += " * \\param arrayData an array of size 'length' (or null if length is -1, may be null if length is 0).\n"
            header += " * \\param length the length of arrayData or -1 if this represents a null array.\n"
        header += " *\n"
        header += " * \\return STATUS_OK on successful write.\n"
        header += " */\n"
        header += "binder_status_t AParcel_write" + pretty + "Array(AParcel* parcel, " + arg_types + ") __INTRODUCED_IN(29);\n\n"
        source += "binder_status_t AParcel_write" + pretty + "Array(AParcel* parcel, " + arg_types + ") {\n"
        source += "    return WriteArray<" + cpp + ">(parcel, " + args + ");\n";
        source += "}\n\n"

    for pretty, cpp in data_types:
        nca = pretty in non_contiguously_addressable

        read_func = "AParcel_read" + pretty + "Array"
        write_func = "AParcel_write" + pretty + "Array"
        allocator_type = "AParcel_" + pretty.lower() + "ArrayAllocator"
        getter_type = "AParcel_" + pretty.lower() + "ArrayGetter"
        setter_type = "AParcel_" + pretty.lower() + "ArraySetter"

        if nca:
            pre_header += "/**\n"
            pre_header += " * This allocates an array of size 'length' inside of arrayData and returns whether or not there was "
            pre_header += "a success. If length is -1, then this should allocate some representation of a null array.\n"
            pre_header += " *\n"
            pre_header += " * See also " + read_func + "\n"
            pre_header += " *\n"
            pre_header += " * \\param arrayData some external representation of an array of " + cpp + ".\n"
            pre_header += " * \\param length the length to allocate arrayData to (or -1 if this represents a null array).\n"
            pre_header += " *\n"
            pre_header += " * \\return whether the allocation succeeded.\n"
            pre_header += " */\n"
            pre_header += "typedef bool (*" + allocator_type + ")(void* arrayData, int32_t length);\n\n"

            pre_header += "/**\n"
            pre_header += " * This is called to get the underlying data from an arrayData object at index.\n"
            pre_header += " *\n"
            pre_header += " * See also " + write_func + "\n"
            pre_header += " *\n"
            pre_header += " * \\param arrayData some external representation of an array of " + cpp + ".\n"
            pre_header += " * \\param index the index of the value to be retrieved.\n"
            pre_header += " *\n"
            pre_header += " * \\return the value of the array at index index.\n"
            pre_header += " */\n"
            pre_header += "typedef " + cpp + " (*" + getter_type + ")(const void* arrayData, size_t index);\n\n"

            pre_header += "/**\n"
            pre_header += " * This is called to set an underlying value in an arrayData object at index.\n"
            pre_header += " *\n"
            pre_header += " * See also " + read_func + "\n"
            pre_header += " *\n"
            pre_header += " * \\param arrayData some external representation of an array of " + cpp + ".\n"
            pre_header += " * \\param index the index of the value to be set.\n"
            pre_header += " * \\param value the value to set at index index.\n"
            pre_header += " */\n"
            pre_header += "typedef void (*" + setter_type + ")(void* arrayData, size_t index, " + cpp + " value);\n\n"
        else:
            pre_header += "/**\n"
            pre_header += " * This is called to get the underlying data from an arrayData object.\n"
            pre_header += " *\n"
            pre_header += " * The implementation of this function should allocate a contiguous array of size 'length' and "
            pre_header += "return that underlying buffer to be filled out. If there is an error or length is 0, null may be "
            pre_header += "returned. If length is -1, this should allocate some representation of a null array.\n"
            pre_header += " *\n"
            pre_header += " * See also " + read_func + "\n"
            pre_header += " *\n"
            pre_header += " * \\param arrayData some external representation of an array of " + cpp + ".\n"
            pre_header += " * \\param length the length to allocate arrayData to.\n"
            pre_header += " * \\param outBuffer a buffer of " + cpp + " of size 'length' (if length is >= 0, if length is 0, "
            pre_header += "this may be nullptr).\n"
            pre_header += " *\n"
            pre_header += " * \\return whether or not the allocation was successful (or whether a null array is represented when length is -1).\n"
            pre_header += " */\n"
            pre_header += "typedef bool (*" + allocator_type + ")(void* arrayData, int32_t length, " + cpp + "** outBuffer);\n\n"

        read_array_args = [("const AParcel*", "parcel")]
        read_array_args += [("void*", "arrayData")]
        read_array_args += [(allocator_type, "allocator")]
        if nca: read_array_args += [(setter_type, "setter")]

        read_type_args = ", ".join((varType + " " + name for varType, name in read_array_args))
        read_call_args = ", ".join((name for varType, name in read_array_args))

        header += "/**\n"
        header += " * Reads an array of " + cpp + " from the next location in a non-null parcel.\n"
        header += " *\n"
        if nca:
            header += " * First, allocator will be called with the length of the array. Then, for every i in [0, length), "
            header += "setter(arrayData, i, x) will be called where x is the value at the associated index.\n"
        else:
            header += " * First, allocator will be called with the length of the array. If the allocation succeeds and the "
            header += "length is greater than zero, the buffer returned by the allocator will be filled with the corresponding data\n"
        header += " *\n"
        header += " * \\param parcel the parcel to read from.\n"
        header += " * \\param arrayData some external representation of an array.\n"
        header += " * \\param allocator the callback that will be called to allocate the array.\n"
        if nca:
            header += " * \\param setter the callback that will be called to set a value at a specific location in the array.\n"
        header += " *\n"
        header += " * \\return STATUS_OK on successful read.\n"
        header += " */\n"
        header += "binder_status_t " + read_func + "(" + read_type_args + ") __INTRODUCED_IN(29);\n\n"
        source += "binder_status_t " + read_func + "(" + read_type_args + ") {\n"
        additional_args = ""
        if nca: additional_args = ", &Parcel::read" + pretty
        source += "    return ReadArray<" + cpp + ">(" + read_call_args + additional_args + ");\n";
        source += "}\n\n"

        cpp_helper += "/**\n"
        cpp_helper += " * Writes a vector of " + cpp + " to the next location in a non-null parcel.\n"
        cpp_helper += " */\n"
        cpp_helper += "inline binder_status_t AParcel_writeVector(AParcel* parcel, const std::vector<" + cpp + ">& vec) {\n"
        write_args = "vec.data(), vec.size()"
        if nca: write_args = "static_cast<const void*>(&vec), vec.size(), AParcel_stdVectorGetter<" + cpp + ">"
        cpp_helper += "    return AParcel_write" + pretty + "Array(parcel, " + write_args + ");\n"
        cpp_helper += "}\n\n"

        cpp_helper += "/**\n"
        cpp_helper += " * Writes an optional vector of " + cpp + " to the next location in a non-null parcel.\n"
        cpp_helper += " */\n"
        cpp_helper += "inline binder_status_t AParcel_writeVector(AParcel* parcel, const std::optional<std::vector<" + cpp + ">>& vec) {\n"
        extra_args = ""
        if nca: extra_args = ", AParcel_stdVectorGetter<" + cpp + ">"
        cpp_helper += "    if (!vec) return AParcel_write" + pretty + "Array(parcel, nullptr, -1" + extra_args + ");\n"
        cpp_helper += "    return AParcel_writeVector(parcel, *vec);\n"
        cpp_helper += "}\n\n"

        cpp_helper += "/**\n"
        cpp_helper += " * Reads a vector of " + cpp + " from the next location in a non-null parcel.\n"
        cpp_helper += " */\n"
        cpp_helper += "inline binder_status_t AParcel_readVector(const AParcel* parcel, std::vector<" + cpp + ">* vec) {\n"
        cpp_helper += "    void* vectorData = static_cast<void*>(vec);\n"
        read_args = []
        read_args += ["parcel"]
        read_args += ["vectorData"]
        if nca:
            read_args += ["AParcel_stdVectorExternalAllocator<bool>"]
            read_args += ["AParcel_stdVectorSetter<" + cpp + ">"]
        else:
            read_args += ["AParcel_stdVectorAllocator<" + cpp + ">"]
        cpp_helper += "    return AParcel_read" + pretty + "Array(" + ", ".join(read_args) + ");\n"
        cpp_helper += "}\n\n"

        cpp_helper += "/**\n"
        cpp_helper += " * Reads an optional vector of " + cpp + " from the next location in a non-null parcel.\n"
        cpp_helper += " */\n"
        cpp_helper += "inline binder_status_t AParcel_readVector(const AParcel* parcel, std::optional<std::vector<" + cpp + ">>* vec) {\n"
        cpp_helper += "    void* vectorData = static_cast<void*>(vec);\n"
        read_args = []
        read_args += ["parcel"]
        read_args += ["vectorData"]
        if nca:
            read_args += ["AParcel_nullableStdVectorExternalAllocator<bool>"]
            read_args += ["AParcel_nullableStdVectorSetter<" + cpp + ">"]
        else:
            read_args += ["AParcel_nullableStdVectorAllocator<" + cpp + ">"]
        cpp_helper += "    return AParcel_read" + pretty + "Array(" + ", ".join(read_args) + ");\n"
        cpp_helper += "}\n\n"

    replaceFileTags(ROOT + "include_ndk/android/binder_parcel.h", pre_header, "START-PRIMITIVE-VECTOR-GETTERS", "END-PRIMITIVE-VECTOR-GETTERS")
    replaceFileTags(ROOT + "include_ndk/android/binder_parcel.h", header, "START-PRIMITIVE-READ-WRITE", "END-PRIMITIVE-READ-WRITE")
    replaceFileTags(ROOT + "parcel.cpp", source, "START", "END")
    replaceFileTags(ROOT + "include_ndk/android/binder_parcel_utils.h", cpp_helper, "START", "END")

    print("Updating DONE.")

if __name__ == "__main__":
    main()
