from __future__ import print_function

import os
import sys
import json

print('Autogen started')

with open("metadata.json", "r") as json_file:
    raw_json = json_file.read()

metadata = json.JSONDecoder().decode(raw_json)

def get_class_data(class_name):
    for class_data in metadata["classes"]:
        if class_data["class"] == class_name:
            return class_data
    return None

header_list = []

for class_data in metadata["classes"]:
    header = class_data["header"]
    if header not in header_list:
        header_list.append(header)

with open("AutogenMetadata.cpp", "w") as autogen_file:
    print('#include "AutogenMetadata.h"', file = autogen_file)
    print('#include "core/Idptr.h"', file = autogen_file)
    print("", file = autogen_file)
    for header in header_list:
        print('#include "' + header + '"', file = autogen_file)
    print("", file = autogen_file)
    print("std::unordered_map<QString, setters_for_type>& get_setters_for_types()", file = autogen_file)
    print("{", file = autogen_file)
    print("    static std::unordered_map<QString, setters_for_type> map;", file = autogen_file)
    print("    return map;", file = autogen_file)
    print("}", file = autogen_file)
    print("", file = autogen_file)
    print("int IMainObject::REAL_TYPE_ITEM;", file = autogen_file)
    for class_data in metadata["classes"]:
        print("int " + class_data["class"] + "::REAL_TYPE_ITEM;", file = autogen_file)
    print("", file = autogen_file)
    print("void InitRealTypes()", file = autogen_file)
    print("{", file = autogen_file)
    print("    IMainObject::REAL_TYPE_ITEM = 0;", file = autogen_file)
    for class_data in metadata["classes"]:
        index = str(metadata["classes"].index(class_data) + 1)
        print("    " + class_data["class"] + "::REAL_TYPE_ITEM = " + index + ";", file = autogen_file)
    print("}", file = autogen_file)
    print("", file = autogen_file)
    print("void InitCreators()", file = autogen_file)
    print("{", file = autogen_file)
    for class_data in metadata["classes"]:
        class_name = class_data["class"]
        print("    (*items_creators())[" + class_name + "::T_ITEM_S()] = &" + class_name + "::_Z_creator;", file = autogen_file)
        print("    (*items_void_creators())[" + class_name + "::T_ITEM_S()] = &" + class_name + "::_Z_creatorSaved;", file = autogen_file)
    print("}", file = autogen_file)
    print("", file = autogen_file)
    print("void InitSettersForTypes()", file = autogen_file)
    print("{", file = autogen_file)
    for class_data in metadata["classes"]:
        class_data_loc = class_data
        while class_data_loc:
            for variable in class_data_loc["variables"]:
                print('    get_setters_for_types()["' + class_data["type"] + '"]["' + variable + '"] = &' + class_data_loc["class"] + '::_Z_KV_SETTERS' + variable + ';', file = autogen_file)
            class_data_loc = get_class_data(class_data_loc["base_class"])
        #print("", file = autogen_file)
    print("}", file = autogen_file)
print('Autogen finished')
    
