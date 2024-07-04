import os
import json
import subprocess

def build_project():
    subprocess.run(["platformio", "run"])

def get_platform_from_ini():
    try:
        with open('../../platformio.ini', 'r') as f:
            lines = f.readlines()
            for line in lines:
                if line.startswith('platform'):
                    return line.split('=')[1].strip()
    except FileNotFoundError:
        return 'esp32c3'

def fix_platform_specific(library_json_path, platform):
    cFlag = 0
    with open(library_json_path, 'r') as f:
        library_json = json.load(f)

    if platform == 'ststm32':
        dir = "../.."
        if "+<src/platform/tasmota/>" in library_json['build']['srcFilter']:
            library_json['build']['srcFilter'].remove("+<src/platform/tasmota/>")
            cFlag = 1
        if "-<src/platform/arduino/>" in library_json['build']['srcFilter']:
            library_json['build']['srcFilter'].remove("-<src/platform/arduino/>")
            cFlag = 1

        if "-<src/platform/tasmota/>" not in library_json['build']['srcFilter']:
            library_json['build']['srcFilter'].append("-<src/platform/tasmota/>")
            cFlag = 1
        if "+<src/platform/arduino/>" not in library_json['build']['srcFilter']:
            library_json['build']['srcFilter'].append("+<src/platform/arduino/>")
            cFlag = 1
    else:
        dir = ""
        if "-<src/platform/tasmota/>" in library_json['build']['srcFilter']:
            library_json['build']['srcFilter'].remove("-<src/platform/tasmota/>")
            cFlag = 1
        if "+<src/platform/arduino/>" in library_json['build']['srcFilter']:
            library_json['build']['srcFilter'].remove("+<src/platform/arduino/>")
            cFlag = 1

        if "-<src/platform/arduino/>" not in library_json['build']['srcFilter']:
            library_json['build']['srcFilter'].append("-<src/platform/arduino/>")
            cFlag = 1
        if "+<src/platform/tasmota/>" not in library_json['build']['srcFilter']:
            library_json['build']['srcFilter'].append("+<src/platform/tasmota/>")
            cFlag = 1

    with open(library_json_path, 'w') as f:
        json.dump(library_json, f, indent=2)

    if cFlag == 1:
        os.chdir(dir)
        build_project()

platform = get_platform_from_ini()
fix_platform_specific('library.json', platform)
