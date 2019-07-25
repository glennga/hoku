#!/usr/bin/env python3
from argparse import Namespace


def get_arguments() -> Namespace:
    from argparse import ArgumentParser

    parser = ArgumentParser(description='Generate tables to populate the Nibble database.')
    list(map(lambda a: parser.add_argument(a[0], help=a[1], type=a[2], choices=a[3]), [
        ['-db', 'Location of the nibble database.', str, None],
        ['-cat', 'Location of the Hipparcos catalog.', str, None],
        ['-hip', 'Name of the HIP table.', str, None],
        ['-bright', 'Name of the BRIGHT table.', str, None],
        ['-t', 'Current time in format: month-year.', str, None],
        ['-m', 'Magnitude to restrict BRIGHT table with.', float, None],
        ['-fov', 'Field-of-view restriction for strategy-specific relations.', float, None],
        ['-table', 'Type of table to generate.', str,
         ['HIP', 'ANGLE', 'DOT', 'SPHERE', 'PLANE', 'PYRAMID', 'COMPOSITE']
         ],
        ['-tablename', 'Name of the table to generate.', str, None]
    ]))

    return parser.parse_args()


if __name__ == '__main__':
    from os.path import abspath
    from subprocess import call
    from pathlib import Path

    if not Path(abspath(__file__).replace('/generate-n.py', '') + '/../bin/GenerateN').is_file():
        print("hoku/bin/GenerateN does not exist. Run CMake + make before this.")

    else:
        arguments = get_arguments()
        call([
            abspath(__file__).replace('/generate-n.py', '') + '/../bin/GenerateN',
            arguments.db,
            arguments.cat,
            arguments.hip,
            arguments.bright,
            arguments.t,
            str(arguments.m),
            str(arguments.fov),
            arguments.table,
            arguments.tablename
        ])
