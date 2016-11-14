import os
import sys
import re
import json

metadata = {"version" : "v0.1.1",
            "classes" : []}

def extract_macros_params(line, macros_name):
    expr = re.compile(macros_name + "\((.*)\)")
    result = expr.search(line)
    if result:
        extracted = result.group(1)
        return extracted
    return None

def parse_declare_saved_begin(file, class_and_base, filename):
    class_and_base_array = class_and_base.split(',')
    class_name = class_and_base_array[0].strip()
    base_class_name = class_and_base_array[1].strip()
    line = file.next()
    class_type = extract_macros_params(line, "DECLARE_GET_TYPE_ITEM")
    if not class_type:
        raise ParseError("Next line after 'DECLARE_SAVED' macros does not contain 'DECLARE_GET_TYPE_ITEM' macros")

    variables = []
    while True:
        line = file.next()
        result = extract_macros_params(line, "KV_SAVEBLE")
        if result:
            variables.append(result.strip())
            continue
        result = extract_macros_params(line, "ADD_TO_TYPELIST")
        if result:
            if result.strip() != class_name:
                raise ParseError("'ADD_TO_TYPELIST' does not match 'DECLARE_SAVED'")
            break

    SUBDIR_NAME_CONST = "ources"
    header_index = filename.find(SUBDIR_NAME_CONST) + len(SUBDIR_NAME_CONST) + 1
    header = filename[header_index:]
            
    class_entry = {"class" : class_name,
                   "header" : header,
                   "base_class" : base_class_name,
                   "type" : class_type,
                   "variables" : variables}

    metadata["classes"].append(class_entry)
      
def parse_file(fullpath, filename):
    file = open(fullpath, 'r')
    for line in file:
        result = extract_macros_params(line, "DECLARE_SAVED")
        if result:
            parse_declare_saved_begin(file, result, fullpath)      

def path_equal(path1, path2):
    return os.path.abspath(path1) == os.path.abspath(path2)

def main():
    dir_name = "sources"
    json_name = "metadata.json"

    if len(sys.argv) > 3:
        raise ParamsParseError("Too many command line options, use --help for more info")

    if len(sys.argv) == 3:
        dir_name = sys.argv[1]
        json_name = sys.argv[2]

    if len(sys.argv) == 2:
        if sys.argv[1] == "--help":
            print("Usage: " + sys.argv[0] + " [<source dir> <json output>]")
            return
        raise ParamsParseError("Too few command line options, use --help for more info")

    for directory, directory_names, filenames in os.walk(dir_name):
        for filename in [f for f in filenames if f.endswith(".h")]:
            fullpath = os.path.join(directory, filename)
            if not "Macroses.h" in fullpath:
                #print("Parse file '" + fullpath + "'")
                parse_file(fullpath, filename)

    json_data = json.JSONEncoder().encode(metadata)
    with open(json_name, "w") as json_file:
        json_file.write(json_data)

if __name__ == "__main__":
    print("Metadata generation started")
    main()
    print("Metadata generation finished")
