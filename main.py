import os.path
import os
import sys
from strsimpy import SIFT4
import time
sift = SIFT4()


def parse_args(argv):
    if len(argv) in [4, 5]:
        try:
            tres = float(argv[3])
        except ValueError:
            pass
        else:
            if 0 <= tres <= 1 and len(
                    argv) == 4 or argv[4] == '--show-progress':
                return argv[1:3] + [tres, len(argv) == 5]
    print('usage:')
    print(f'{sys.executable} {__file__} dir1 dir2 treshold [--show-progress]')
    print('''
Where:
    dir1, dir2 - paths to directories to compare
    treshold - float value in [0...1], 1 - require full similarity, 0 - treat all files as similar
    --show-progress will print approximate percentage progress messages no more than once per second
          '''.strip())
    exit(1)


dir1, dir2, tres, prog = parse_args(sys.argv)


class file:
    def __init__(self, dir, name):
        self.dir = dir
        self.name = name
        self.content = open(os.path.join(dir, name), 'rb').read()
        self.present_in_other_dir = False

    def __repr__(self) -> str:
        return f'{self.dir}/{self.name}'


files1 = [file(dir1, name) for name in os.listdir(dir1)]
files2 = [file(dir2, name) for name in os.listdir(dir2)]

identical = []
similar = []

count = 0
print_time = time.monotonic()

for file1 in files1:
    for file2 in files2:
        if prog and time.monotonic() - print_time > 1:
            print('progress:', count / len(files1) / len(files2))
            print_time = time.monotonic()
        count += 1
        if file1.content == file2.content:
            file1.present_in_other_dir = True
            file2.present_in_other_dir = True
            identical.append((file1, file2))
            continue
        dist = sift.distance(file1.content, file2.content)
        max_len = max(len(file1.content), len(file2.content))
        sim = (max_len - dist) / max_len
        if sim >= tres:
            file1.present_in_other_dir = True
            file2.present_in_other_dir = True
            similar.append((file1, file2))

print('identical files:')
if identical:
    for file1, file2 in identical:
        print(f'    {file1} - {file2}')
else:
    print('    (no such files)')
print()

print('similar files:')
if similar:
    for file1, file2 in similar:
        print(f'    {file1} - {file2}')
else:
    print('    (no such files)')
print()

print('present only in dir1:')
found = False
for file1 in files1:
    if not file1.present_in_other_dir:
        print(f'    {file1}')
        found = True
if not found:
    print('    (no such files)')
print()

print('present only in dir2:')
found = False
for file2 in files2:
    if not file2.present_in_other_dir:
        print(f'    {file2}')
        found = True
if not found:
    print('    (no such files)')
