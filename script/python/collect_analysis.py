from sqlite3 import connect
from os import environ
from numpy import std
from pprint import PrettyPrinter

if __name__ == '__main__':
    results = {}
    attach_r = lambda ell, r: results.update(zip(ell, cur.execute(r).fetchall()) if isinstance(ell, list)
                                             else {ell: cur.execute(r).fetchone()})
    attach_std = lambda ell, r: results.update({ell: (std(cur.execute(r).fetchall()),)})

    conn = connect(environ['HOKU_PROJECT_PATH'] + '/data/lumberjack-all-O0.db')
    cur = conn.cursor()

    # Say goodbye to maintainability!!

    attach_r(
        ["Angle AVG Query CSS", "Dot AVG Query CSS", "Plane AVG Query CSS",
         "Pyramid AVG Query CSS", "Sphere AVG Query CSS"],
        """
            SELECT AVG(CandidateSetSize)
            FROM QUERY
            WHERE IdentificationMethod NOT LIKE 'Composite'
            GROUP BY IdentificationMethod
        """
    )

    attach_r(
        ["Angle AVG Query T", "Dot AVG Query T", "Plane AVG Query T", "Pyramid AVG Query T",
         "Sphere AVG Query T"],
        """
            SELECT AVG(RunningTime)
            FROM QUERY
            WHERE IdentificationMethod NOT LIKE 'Composite'
            GROUP BY IdentificationMethod
        """
    )

    attach_r(
        "Pyramid AVG Query ACC",
        """
            SELECT AVG(SExistence)
            FROM QUERY
            WHERE IdentificationMethod LIKE 'Pyramid'
        """
    )

    attach_r(
        "Pyramid Query COUNT CSS > 1",
        """
            SELECT COUNT(*)
            FROM QUERY
            WHERE IdentificationMethod LIKE 'Pyramid'
            AND SExistence <> 1  
        """
    )

    attach_r(
        "Triangle Query T - Angle Query T",
        """
            SELECT ((A1 + A2) / 2.0) - A3
            FROM
                (SELECT AVG(RunningTime) AS A1
                FROM QUERY
                WHERE IdentificationMethod LIKE 'Sphere'),
                (SELECT AVG(RunningTime) AS A2
                FROM QUERY
                WHERE IdentificationMethod LIKE 'Plane'),
                (SELECT AVG(RunningTime) AS A3
                FROM QUERY
                WHERE IdentificationMethod LIKE 'Angle')
        """
    )

    attach_r(
        "Pyramid Query T - Triangle Query T",
        """
            SELECT A4 - ((A1 + A2) / 2.0)
            FROM
                (SELECT AVG(RunningTime) AS A1
                FROM QUERY
                WHERE IdentificationMethod LIKE 'Sphere'),
                (SELECT AVG(RunningTime) AS A2
                FROM QUERY
                WHERE IdentificationMethod LIKE 'Plane'),
                (SELECT AVG(RunningTime) AS A4
                FROM QUERY
                WHERE IdentificationMethod LIKE 'Pyramid')
        """
    )

    attach_r(
        "Dot Query T / Angle Query T",
        """
            SELECT A4 / A3
            FROM
                (SELECT AVG(RunningTime) AS A3
                FROM QUERY
                WHERE IdentificationMethod LIKE 'Angle'),
                (SELECT AVG(RunningTime) AS A4
                FROM QUERY
                WHERE IdentificationMethod LIKE 'Dot')
        """
    )

    attach_r(
        "AVG Triangle Query CSS",
        """
            SELECT (A1 + A2) / 2.0
            FROM
                (SELECT AVG(CandidateSetSize) AS A1
                FROM QUERY
                WHERE IdentificationMethod LIKE 'Plane'),
                (SELECT AVG(CandidateSetSize) AS A2
                FROM QUERY
                WHERE IdentificationMethod LIKE 'Sphere')
        """
    )

    attach_r(
        ["Plane Query COUNT CSS > 1", "Sphere Query COUNT CSS > 1"],
        """
            SELECT COUNT(*)
            FROM QUERY
            WHERE IdentificationMethod LIKE 'Plane'
            AND CandidateSetSize <> 1
            UNION ALL
            SELECT COUNT(*)
            FROM QUERY
            WHERE IdentificationMethod LIKE 'Sphere'
            AND CandidateSetSize <> 1
        """
    )

    attach_r(
        ["Plane Query CSS CSS > 1", "Sphere Query CSS CSS > 1"],
        """
            SELECT AVG(CandidateSetSize)
            FROM QUERY
            WHERE IdentificationMethod LIKE 'Plane'
            AND CandidateSetSize <> 1
            UNION ALL
            SELECT AVG(CandidateSetSize)
            FROM QUERY
            WHERE IdentificationMethod LIKE 'Sphere'
            AND CandidateSetSize <> 1
        """
    )

    attach_r(
        "Pyramid Query COUNT CSS > 1",
        """
            SELECT COUNT(*)
            FROM QUERY
            Where IdentificationMethod LIKE 'Pyramid'
            AND CandidateSetSize <> 1
        """
    )

    attach_r(
        "Pyramid Query CSS CSS > 1",
        """
            SELECT AVG(CandidateSetSize)
            FROM QUERY
            WHERE IdentificationMethod LIKE 'Pyramid'
            AND CandidateSetSize
        """
    )

    attach_r(
        "Dot Query COUNT CSS > 1",
        """
            SELECT COUNT(*)
            FROM QUERY
            Where IdentificationMethod LIKE 'Dot'
            AND CandidateSetSize <> 1
        """
    )

    attach_r(
        "Dot Query CSS CSS > 1",
        """
            SELECT AVG(CandidateSetSize)
            FROM QUERY
            WHERE IdentificationMethod LIKE 'Dot'
            AND CandidateSetSize <> 1
        """
    )

    attach_r(
        "Angle Query CSS / Triangle Query CSS",
        """
            SELECT A1 / ((A2 + A3) / 2.0)
            FROM
                (SELECT AVG(CandidateSetSize) AS A1
                FROM QUERY
                WHERE IdentificationMethod LIKE 'Angle'),
                (SELECT AVG(CandidateSetSize) AS A2
                FROM QUERY
                WHERE IdentificationMethod LIKE 'Sphere'),
                (SELECT AVG(CandidateSetSize) AS A3
                FROM QUERY
                WHERE IdentificationMethod LIKE 'Plane')
        """
    )

    attach_r(
        "Angle Query COUNT CSS > 1",
        """
            SELECT COUNT(*)
            FROM QUERY
            WHERE IdentificationMethod LIKE 'Angle'
            AND CandidateSetSize <> 1
        """
    )

    attach_r(
        ["Pyramid ACC Reduction NN", "Angle ACC Reduction NN"],
        """
            SELECT AVG(PercentageCorrect)
            FROM REDUCTION
            WHERE (IdentificationMethod LIKE 'Pyramid'
            OR IdentificationMethod LIKE 'Angle')
            AND ShiftDeviation < 1.0e-6
            AND FalseStars = 0
            GROUP BY IdentificationMethod
            ORDER BY IdentificationMethod
        """
    )

    attach_r(
        "Angle Reduction QC NN / (Plane, Sphere, Dot, Composite, Pyramid) Reduction QC NN",
        """
            SELECT A1 / A2
            FROM
                (SELECT AVG(B) AS A2
                FROM
                    (SELECT AVG(QueryCount) AS B
                    FROM REDUCTION
                    WHERE ShiftDeviation < 1.0e-6 AND FalseStars = 0
                    AND IdentificationMethod NOT LIKE 'Angle'
                    GROUP BY IdentificationMethod)
                ),
                (SELECT AVG(QueryCount) AS A1
                FROM REDUCTION
                WHERE ShiftDeviation < 1.0e-6 AND FalseStars = 0
                AND IdentificationMethod LIKE 'Angle')
        """
    )

    attach_r(
        "Angle Reduction T NN / Other Reduction T NN",
        """
            SELECT A1 / A2
            FROM
                (SELECT AVG(B) AS A2
                FROM
                    (SELECT AVG(TimeToResult) AS B
                    FROM REDUCTION
                    WHERE ShiftDeviation < 1.0e-6 AND FalseStars = 0
                    AND IdentificationMethod NOT LIKE 'Angle'
                    GROUP BY IdentificationMethod)
                ),
                (SELECT AVG(TimeToResult) AS A1
                FROM REDUCTION
                WHERE ShiftDeviation < 1.0e-6 AND FalseStars = 0
                AND IdentificationMethod LIKE 'Angle')
        """
    )

    attach_r(
        ["Angle MAX Reduction T NN", "Composite MAX Reduction T NN", "Dot MAX Reduction T NN",
         "Plane MAX Reduction T NN", "Pyramid MAX Reduction T NN"],
        """
            SELECT MAX(TimeToResult)
            FROM REDUCTION
            WHERE ShiftDeviation < 1.0e-6 AND FalseStars = 0
            GROUP BY IdentificationMethod
            ORDER BY IdentificationMethod
        """
    )

    attach_r(
        "Angle COUNT Reduction QC = 1 NN",
        """
            SELECT COUNT(*)
            FROM REDUCTION
            WHERE IdentificationMethod LIKE 'Angle'
            AND ShiftDeviation < 1.0e-6 AND FalseStars = 0
            AND QueryCount = 1
        """
    )

    attach_r(
        "Pyramid Query ACC - Pyramid Reduction ACC NN",
        """
            SELECT Q1 - Q2
            FROM 
                (SELECT AVG(SExistence) AS Q1
                FROM QUERY
                WHERE IdentificationMethod LIKE 'Pyramid'),
                (SELECT AVG(PercentageCorrect) AS Q2
                FROM REDUCTION 
                WHERE ShiftDeviation < 1.0e-6 AND FalseStars = 0
                AND IdentificationMethod LIKE 'Pyramid')
        """
    )

    attach_r(
        ["Angle Reduction AVG QC NN", "Composite Reduction AVG QC NN", "Dot Reduction AVG QC NN",
         "Plane Reduction AVG QC NN", "Pyramid Reduction AVG QC NN", "Sphere Reduction AVG QC NN"],
        """
            SELECT AVG(QueryCount)
            FROM REDUCTION
            WHERE ShiftDeviation < 1.0e-6 AND FalseStars = 0
            GROUP BY IdentificationMethod
            ORDER BY IdentificationMethod
        """
    )

    attach_r(
        ["Angle Reduction AVG T NN", "Composite Reduction AVG T NN", "Dot Reduction AVG T NN",
         "Plane Reduction AVG T NN", "Pyramid Reduction AVG T NN", "Sphere Reduction AVG T NN"],
        """
            SELECT AVG(TimeToResult)
            FROM REDUCTION
            WHERE ShiftDeviation < 1.0e-6 AND FalseStars = 0
            GROUP BY IdentificationMethod
            ORDER BY IdentificationMethod
        """
    )

    attach_r(
        ["Angle Reduction COUNT QC = 1 NN", "Composite Reduction COUNT QC = 1 NN", "Dot Reduction COUNT QC = 1 NN",
         "Plane Reduction COUNT QC = 1 NN", "Sphere Reduction COUNT QC = 1 NN"],
        """
           SELECT COUNT(*)
           FROM REDUCTION
           WHERE ShiftDeviation < 1.0e-6 AND FalseStars = 0
           AND QueryCount = 1
           AND IdentificationMethod NOT LIKE 'Pyramid'
           GROUP BY IdentificationMethod
           ORDER BY IdentificationMethod
       """
    )

    attach_r(
        "(Composite, Triangle) Reduction COUNT QC > 1 NN",
        """
           SELECT COUNT(*)
           FROM REDUCTION
           WHERE ShiftDeviation < 1.0e-6 AND FalseStars = 0
           AND QueryCount > 1
           AND (IdentificationMethod LIKE 'Composite' OR
                IdentificationMethod LIKE 'Plane' OR 
                IdentificationMethod LIKE 'Sphere')
        """
    )

    attach_r(
        "(Composite, Triangle) Reduction QC QC > 1 NN",
        """
           SELECT AVG(QueryCount)
           FROM REDUCTION
           WHERE ShiftDeviation < 1.0e-6 AND FalseStars = 0
           AND QueryCount > 1
           AND (IdentificationMethod LIKE 'Composite' OR
                IdentificationMethod LIKE 'Plane' OR 
                IdentificationMethod LIKE 'Sphere')
        """
    )

    attach_r(
        "Dot Reduction COUNT QC > 1 NN",
        """
            SELECT COUNT(*)
            FROM REDUCTION
            WHERE ShiftDeviation < 1.0e-6 AND FalseStars = 0
            AND IdentificationMethod LIKE 'Dot'
            AND QueryCount <> 1
        """
    )

    attach_r(
        "(Dot Reduction COUNT QC > 1) / 1000 NN",
        """
           SELECT COUNT(*) / 1000.0
           FROM REDUCTION
           WHERE ShiftDeviation < 1.0e-6 AND FalseStars = 0
           AND IdentificationMethod LIKE 'Dot'
           AND QueryCount <> 1
       """
    )

    attach_r(
        "Dot Reduction QC QC > 1 NN",
        """
            SELECT AVG(QueryCount)
            FROM REDUCTION
            WHERE ShiftDeviation < 1.0e-6 AND FalseStars = 0
            AND IdentificationMethod LIKE 'Dot'
            AND QueryCount <> 1
        """
    )

    attach_r(
        "Pyramid Reduction AVG QS NN",
        """
            SELECT AVG(QueryCount) / 3.0
            FROM REDUCTION
            WHERE ShiftDeviation < 1.0e-6 AND FalseStars = 0
            AND IdentificationMethod LIKE 'Pyramid'
        """
    )

    attach_r(
        "Pyramid Reduction COUNT QC = 3 NN",
        """
            SELECT COUNT(*)
            FROM REDUCTION
            WHERE ShiftDeviation < 1.0e-6 AND FalseStars = 0
            AND IdentificationMethod LIKE 'Pyramid'
            AND QueryCount = 3
        """
    )

    attach_r(
        "(Angle, Dot, Pyramid) AVG Reduction T NN - (Composite, Triangle) AVG Reduction T NN",
        """
            SELECT A1 - A2
            FROM
                (SELECT AVG(TimeToResult) AS A1
                FROM REDUCTION
                WHERE ShiftDeviation < 1.0e-6 AND FalseStars = 0
                AND (IdentificationMethod LIKE 'Pyramid' OR
                     IdentificationMethod LIKE 'Dot' OR
                     IdentificationMethod LIKE 'Angle')
                ),
                (SELECT AVG(TimeToResult) AS A2
                FROM REDUCTION
                WHERE ShiftDeviation < 1.0e-6 AND FalseStars = 0
                AND (IdentificationMethod LIKE 'Composite' OR
                     IdentificationMethod LIKE 'Plane' OR
                     IdentificationMethod LIKE 'Sphere')
                )
        """
    )

    attach_r(
        "(Composite, Triangle) AVG Reduction T",
        """
            SELECT AVG(TimeToResult)
            FROM REDUCTION
            WHERE ShiftDeviation < 1.0e-6 AND FalseStars = 0
            AND (IdentificationMethod LIKE 'Composite' OR
                 IdentificationMethod LIKE 'Plane' OR
                 IdentificationMethod LIKE 'Sphere')
        """
    )

    attach_std(
        "(Composite, Triangle) STD Reduction T",
        """
            SELECT TimeToResult
            FROM REDUCTION
            WHERE ShiftDeviation < 1.0e-6 AND FalseStars = 0
            AND (IdentificationMethod LIKE 'Composite' OR
                 IdentificationMethod LIKE 'Plane' OR
                 IdentificationMethod LIKE 'Sphere')
        """
    )

    # Reformat the results dictionary, and print the results.
    p = {'Angle': {'Query': {}, 'Reduction': {}, 'Identification': {}},
         'Composite': {'Query': {}, 'Reduction': {}, 'Identification': {}},
         'Dot': {'Query': {}, 'Reduction': {}, 'Identification': {}},
         'Plane': {'Query': {}, 'Reduction': {}, 'Identification': {}},
         'Pyramid': {'Query': {}, 'Reduction': {}, 'Identification': {}},
         'Sphere': {'Query': {}, 'Reduction': {}, 'Identification': {}}}

    # Your scientists were so preoccupied with whether or not they could, they didn't stop to think if they should.
    # (I'm sorry).
    is_method = lambda i_0, j_0, k_0: ((('Triangle' in k_0 and (j_0 == 'Plane' or j_0 == 'Sphere')) or j_0 in k_0)
                                       and i_0 in k_0)
    list(map(lambda k: list(map(lambda j: list(map(
        lambda i: p[j][i].update({k.replace(j + ' ', '').replace(i + ' ', ''):
                                      results[k][0] if len(results[k]) == 1 else results[k]})
        if is_method(i, j, k) else None, p[j])), p)), results))

    pp = PrettyPrinter(indent=4)
    pp.pprint(p)
