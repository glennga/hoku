#!/usr/bin/env python3
from argparse import Namespace

SCHEMAS = {
    "QUERY": """
        IdentificationMethod TEXT,
        Timestamp TEXT,
        Epsilon1 FLOAT,
        Epsilon2 FLOAT,
        Epsilon3 FLOAT,
        CandidateSetSize FLOAT,
        RunningTime FLOAT,
        SExistence INT
    """,
    "REDUCTION": """
        IdentificationMethod TEXT,
        Timestamp TEXT,
        Epsilon1 FLOAT,
        Epsilon2 FLOAT,
        Epsilon3 FLOAT,
        ShiftDeviation FLOAT,
        FalseStars INT,
        RemovedBlobs INT,
        QueryCount INT,
        TimeToResult FLOAT,
        PercentageCorrect FLOAT
    """,
    "MAP": """
        IdentificationMethod TEXT,
        Timestamp TEXT,
        Epsilon1 FLOAT,
        Epsilon2 FLOAT,
        Epsilon3 FLOAT,
        Epsilon4 FLOAT,
        ShiftDeviation FLOAT,
        FalseStars INT,
        RemovedBlobs INT,
        QueryCount INT,
        TimeToResult FLOAT,
        PercentageCorrect FLOAT,
        IsErrorOut INT
    """
}
PREPARED = {
    "QUERY": ''.join(['?, ' for _ in range(7)]) + '?',
    "REDUCTION": ''.join(['?, ' for _ in range(10)]) + '?',
    "MAP": ''.join(['?, ' for _ in range(12)]) + '?'
}


def get_arguments() -> Namespace:
    from argparse import ArgumentParser

    parser = ArgumentParser(description='Perform experiments for Hoku research.')
    list(map(lambda a: parser.add_argument(a[0], help=a[1], type=a[2], choices=a[3]), [
        ['-pnum', 'Number of processes to spawn.', int, None],
        ['-refdb', 'Location of the nibble database.', str, None],
        ['-recdb', 'Location of the lumberjack database.', str, None],
        ['-hip', 'Name of the HIP table.', str, None],
        ['-bright', 'Name of the BRIGHT table.', str, None],
        ['-rtable', 'Name of the reference table to use.', str, None],
        ['-etable', 'Name of the experiment table in lumberjack to use.', str, None],
        ['-strategy', 'Name of the strategy to use.', str,
         ['ANGLE', 'DOT', 'PLANE', 'SPHERE', 'PYRAMID', 'COMPOSITE']
         ],
        ['-prefix', 'Prefix (identifier) to attach to the method.', str, None],
        ['-ep1', 'Value of epsilon 1.', float, None],
        ['-ep2', 'Value of epsilon 2.', float, None],
        ['-ep3', 'Value of epsilon 3.', float, None],
        ['-ep4', 'Value of epsilon 4.', float, None],
        ['-nlimit', 'Number of stars to restrict image to (-1 = no limit).', int, None],
        ['-mlimit', 'Magnitude to restrict image by.', float, None],
        ['-nulimit', 'Maximum number of query star subset selections.', int, None],
        ['-exper', 'Name of the simulation to perform.', str, ['QUERY', 'REDUCTION', 'MAP']],
        ['-samples', 'Number of simulations to perform.', int, None],
        ['-imfov', 'Field of view for the resulting image.', float, None],
        ['-ssiter', 'Number of different shift simulations.', int, None],
        ['-ssstep', 'Step size (in degrees) of shift per iter.', float, None],
        ['-esmin', 'Minimum number of false positives.', int, None],
        ['-esiter', 'Number of different false positive simulations.', int, None],
        ['-esstep', 'Step size (of false positives) per iter.', int, None],
        ['-rmiter', 'Number of different false negative simulations.', int, None],
        ['-rmstep', 'Step size (of removed blobs) per iter.', int, None],
        ['-rmsigma', 'Size of removed blob.', float, None]
    ]))

    return parser.parse_args()


def execute_chunk(recdb, samples):
    global arguments  # Must be run after retrieving the arguments in main!

    call([
        abspath(__file__).replace('/perform-e.py', '') + '/../bin/PerformE',
        arguments.refdb,
        recdb,
        arguments.hip,
        arguments.bright,
        arguments.rtable,
        arguments.etable,
        SCHEMAS[arguments.exper],
        arguments.strategy,
        arguments.prefix,
        str(arguments.ep1),
        str(arguments.ep2),
        str(arguments.ep3),
        str(arguments.ep4),
        str(arguments.nlimit),
        str(arguments.mlimit),
        str(arguments.nulimit),
        arguments.exper,
        str(samples),
        str(arguments.imfov),
        str(arguments.ssiter),
        str(arguments.ssstep),
        str(arguments.esmin),
        str(arguments.esiter),
        str(arguments.esstep),
        str(arguments.rmiter),
        str(arguments.rmstep),
        str(arguments.rmsigma)
    ])


if __name__ == '__main__':
    from multiprocessing import Pool
    from os import rename, remove
    from os.path import abspath
    from subprocess import call
    from sqlite3 import connect
    from pathlib import Path
    from shutil import copy

    if not Path(abspath(__file__).replace('/perform-e.py', '') + '/../bin/PerformE').is_file():
        print("hoku/bin/PerformE does not exist. Run CMake + make before this.")

    else:
        arguments = get_arguments()
        simulations = round(arguments.samples / arguments.pnum)

        # Execute in parallel.
        Pool(arguments.pnum).starmap(execute_chunk, zip(
            [arguments.recdb + f'-{i}' for i in range(arguments.pnum)], [simulations for _ in range(arguments.pnum)]
        ))

        # Resource collection.
        main_db = connect(arguments.recdb)
        main_cursor = main_db.cursor()
        main_cursor.execute(f"""
            CREATE TABLE IF NOT EXISTS {arguments.etable} (
                {SCHEMAS[arguments.exper]}
            );
        """)

        for sec_db in list(map(lambda a: connect(a), [arguments.recdb + f'-{i}' for i in range(0, arguments.pnum)])):
            sec_cursor = sec_db.cursor()
            sec_records = sec_cursor.execute(f"""
                SELECT *
                FROM {arguments.etable}
            """).fetchall()

            for record in sec_records:
                main_cursor.execute(f"""
                    INSERT INTO {arguments.etable}
                    VALUES ({PREPARED[arguments.exper]})
                """, record)
            sec_db.close()

        main_db.commit()
        list(map(lambda a: remove(arguments.recdb + f'-{a}'), range(0, arguments.pnum)))
