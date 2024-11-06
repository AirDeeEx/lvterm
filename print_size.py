#!/usr/bin/env python
import json
import sys
import os
import shutil
import subprocess

bin_dir = sys.argv[1]
main_path = os.path.join(bin_dir, "main")
main_striped_path = os.path.join(bin_dir, "main_striped")

last_size_file = os.path.join(bin_dir, "last_size.json")

def check_dir(path: str):
    if not os.path.isdir(path):
        print("Bad dir:", path)
        sys.exit(1)
        
def check_file(path: str):
    if not os.path.isfile(path):
        print("Bad file:", path)
        sys.exit(1)


def humanize_bytes(size, precision=1):
    suffixes=['B','KB','MB','GB','TB']
    suffixIndex = 0
    while size > 1024:
        suffixIndex += 1 #increment the index of the suffix
        size = size/1024.0 #apply the division
    return "%.*f %s"%(precision,size,suffixes[suffixIndex])

def print_sym_group_size(sym_dict):
    name = sym_dict['name']
    size = sym_dict['cumulative_size']
    print(f"{name}: {humanize_bytes(size)}")
    

def print_symbols_size():
    raw_json = subprocess.run(["elf-size-analyze", "-FHj", main_path], capture_output=True).stdout
    j = json.loads(raw_json)
    
    for k,v in j['/']['children'].items():
        print_sym_group_size(v)

def save_size(bin_size, strip_size):
    open(last_size_file, 'w').write(json.dumps((bin_size, strip_size)))

def read_last_size():
    return json.loads(open(last_size_file, 'r').read())

def main() -> int:
    
    check_dir(bin_dir)
    
    shutil.copy(main_path, main_striped_path)
    os.system(f"strip {main_striped_path}");
    
    main_size = os.path.getsize(main_path)
    main_striped_size = os.path.getsize(main_striped_path)
    
    try:
        last_size = read_last_size()
    except:
        last_size = (0,0)
    
    save_size(main_size, main_striped_size)
    
    det = "=============="
    print(det)
    print()
    print(f"Binnary size: {main_size} (+{humanize_bytes(main_size-last_size[0])})")
    print("Striped binnary size:", main_striped_size)
    print()
    #print_symbols_size()
    
    print(det)
    
    return 0
    
if __name__ == '__main__':
    sys.exit(main())