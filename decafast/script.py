import os
import difflib

ref_dir = 'references/dev'
out_dir = 'output/dev'

# Get list of .out files in reference and output directories
ref_files = {f for f in os.listdir(ref_dir) if f.endswith('.out')}
out_files = {f for f in os.listdir(out_dir) if f.endswith('.out')}

# Find common files
common_files = ref_files & out_files

if not common_files:
    print("No matching .out files found in both directories.")
else:
    for filename in sorted(common_files):
        with open(os.path.join(ref_dir, filename)) as ref_file:
            ref_content = ref_file.readlines()

        with open(os.path.join(out_dir, filename)) as out_file:
            out_content = out_file.readlines()

        if ref_content != out_content:
            print(f"\nDifferences found in file: {filename}")
            diff = difflib.unified_diff(
                ref_content, out_content,
                fromfile=f'references/{filename}',
                tofile=f'output/{filename}',
                lineterm=''
            )
            print('\n'.join(diff))
        else:
            print(f"{filename} - OK (no differences)")

