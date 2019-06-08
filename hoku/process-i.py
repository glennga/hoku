#!/usr/bin/env python3
from argparse import Namespace


def get_arguments() -> Namespace:
    from argparse import ArgumentParser

    parser = ArgumentParser(description='Identify an image several times and output the running times.')
    list(map(lambda a: parser.add_argument(a[0], help=a[1], type=a[2], choices=a[3]), [
        ['-db', 'Location of the nibble database.', str, None],
        ['-hip', 'Name of the HIP table.', str, None],
        ['-bright', 'Name of the BRIGHT table.', str, None],
        ['-rtable', 'Name of the reference table to use.', str, None],
        ['-strategy', 'Name of the strategy to use.', str,
         ['ANGLE', 'DOT', 'PLANE', 'SPHERE', 'PYRAMID', 'COMPOSITE']
         ],
        ['-ep1', 'Value of epsilon 1.', float, None],
        ['-ep2', 'Value of epsilon 2.', float, None],
        ['-ep3', 'Value of epsilon 3.', float, None],
        ['-ep4', 'Value of epsilon 4.', float, None],
        ['-nulimit', 'Maximum number of query star subset selections.', int, None],
        ['-samples', 'Number of identifications to perform.', int, None],
        ['-imfov', 'Field-of-view of the camera.', float, None],
        ['-bkbsz', 'Length of one kernel side used in normalized box filter.', int, None],
        ['-minced', 'Minimum threshold of intensity gradient (hystersis thresholding).', int, None],
        ['-maxced', 'Maximum threshold of intensity gradient (hystersis thresholding).', int, None],
        ['-dpp', 'Degrees per pixel of the camera.', float, None]
    ]))

    return parser.parse_args()


if __name__ == '__main__':
    from os.path import abspath
    from subprocess import call
    from pathlib import Path

    if not Path(abspath(__file__).replace('/process-i.py', '') + '/../bin/ProcessI').is_file():
        print("hoku/bin/ProcessI does not exist. Run CMake + make before this.")

    else:
        arguments = get_arguments()
        call([
            abspath(__file__).replace('/process-i.py', '') + '/../bin/ProcessI',
            arguments.db,
            arguments.hip,
            arguments.bright,
            arguments.rtable,
            arguments.strategy,
            str(arguments.ep1),
            str(arguments.ep2),
            str(arguments.ep3),
            str(arguments.ep4),
            str(arguments.nulimit),
            str(arguments.samples),
            str(arguments.imfov),
            str(arguments.bkbsz),
            str(arguments.minced),
            str(arguments.maxced),
            str(arguments.dpp)
        ])
