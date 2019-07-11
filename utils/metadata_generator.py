import os
import sys
import re
import json
from typing import Union, Dict, List
import io


def extract_macros_params(line: str, macros_name: str) -> Union[str, None]:
    expression = re.compile(macros_name + "\((.*)\)")
    result = expression.search(line)
    if result:
        index = result.start()
        if line.find('//', 0, index) != -1:
            return None
        extracted = result.group(1)
        return extracted
    return None


def parse_file(full_path: str) -> List[Union[Dict, None]]:
    result = []
    with open(full_path, 'r') as file:
        for line in file:
            macros_params = extract_macros_params(line, "DECLARE_SAVEABLE")
            if macros_params:
                result.append(parse_declare_saved_begin(file, macros_params, full_path))
    return result


def parse_declare_saved_begin(file: io.TextIOWrapper, class_and_base: str, filename: str) -> dict:
    class_and_base_array = class_and_base.split(',')
    class_name = class_and_base_array[0].strip()
    base_class_name = class_and_base_array[1].strip()

    line = file.readline()
    class_type = extract_macros_params(line, "REGISTER_CLASS_AS")
    if not class_type:
        raise Exception("Next line after 'DECLARE_SAVEABLE' macros does not contain 'REGISTER_CLASS_AS' macros")

    variables = []
    on_load_calls = []

    while True:
        line = file.readline()
        result = extract_macros_params(line, "KV_SAVEABLE")
        if result:
            variables.append(result.strip())
            continue
        result = extract_macros_params(line, "KV_ON_LOAD_CALL")
        if result:
            on_load_calls.append(result.strip())
            continue
        result = extract_macros_params(line, "END_DECLARE")
        if result:
            if result.strip() != class_name:
                raise Exception("'END_DECLARE' does not match 'DECLARE_SAVEABLE'")
            break

    subdirectory = "sources"
    header_index = filename.find(subdirectory) + len(subdirectory) + 1
    header = filename[header_index:]

    class_entry = {
        "class": class_name,
        "header": header,
        "base_class": base_class_name,
        "type": class_type,
        "variables": variables,
        "on_load_calls": on_load_calls
    }

    return class_entry


def show_help(file_name: str) -> None:
    print("Usage: {} [<source dir> <json output>]".format(file_name))
    sys.exit(0)


def metadata_to_json(metadata: Dict[str, List]) -> str:
    return json.JSONEncoder(indent=4).encode(metadata)


def generate_metadata(directory: str) -> List[dict]:
    result = []
    for directory, directory_names, filenames in os.walk(directory):
        directory_names.sort()
        filenames.sort()
        for filename in [f for f in filenames if f.endswith(".h")]:
            full_path = os.path.join(directory, filename)
            if "KvMacros.h" not in full_path:
                parsed_data = parse_file(full_path)
                if parsed_data:
                    result += parsed_data
    return result


def main():
    directory = "sources"
    json_name = "metadata.json"
    metadata = {
        "version": "v0.1.1",
        "classes": []
    }

    if len(sys.argv) > 3:
        raise Exception("Too many command line options, use --help for more info")

    if len(sys.argv) == 3:
        directory = sys.argv[1]
        json_name = sys.argv[2]

    if len(sys.argv) == 2:
        if sys.argv[1] == "--help":
            show_help(sys.argv[0])
        else:
            raise Exception("Too few command line options, use --help for more info")

    metadata['classes'] = generate_metadata(directory)

    json_data = metadata_to_json(metadata)
    with open(json_name, "w") as json_file:
        json_file.write(json_data)


if __name__ == "__main__":
    print("Metadata generation started")
    main()
    print("Metadata generation finished")
