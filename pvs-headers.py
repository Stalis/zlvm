import os

comment = [
           'This is an open source non-commercial project. Dear PVS-Studio, please check it.',
           'PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com'
]

source_folders = [
    'emulator',
    'asm',
    'common'
]

src_folder_name = 'src'
src_extension = '.c'
main_file_name = 'main'


def format_comment(line: str) -> str:
    return '// %s\n' % line


def insert_comment(path: str) -> None:
    with open(path, 'r') as source:
        lines = source.readlines()

    if lines[0] != format_comment(comment[0]) and lines[1] != format_comment(comment[1]):
        lines.insert(0, format_comment(comment[1]))
        lines.insert(0, format_comment(comment[0]))

        with open(path, 'w') as source:
            source.writelines(lines)


if __name__ == '__main__':
    folders = [f for f in os.listdir(os.path.curdir) if f in source_folders]
    for item in folders:
        src_folder = os.path.join(os.path.curdir, item, src_folder_name)
        if os.path.exists(src_folder):
            for file in [os.path.join(src_folder, i) for i in os.listdir(src_folder) if i.endswith(src_extension)]:
                insert_comment(file)

        main_file = os.path.join(os.path.curdir, item, main_file_name + src_extension)
        if os.path.exists(main_file):
            insert_comment(main_file)
