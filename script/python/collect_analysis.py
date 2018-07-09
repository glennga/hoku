from sqlite3 import connect
from os import environ
from numpy import std
from pprint import PrettyPrinter

if __name__ == '__main__':
    results = {}
    attach_r = lambda ell, r: results.update(zip(ell, cur.execute(r).fetchall()) if isinstance(ell, list)
                                             else {ell: cur.execute(r).fetchone()})
    attach_std = lambda ell, r: results.update({ell: (std(cur.execute(r).fetchall()),)})

    conn = connect(environ['HOKU_PROJECT_PATH'] + '/data/lumberjack-all-triad.db')
    cur = conn.cursor()

    # Say goodbye to maintainability!!

    attach_std(
        "Plane Query STD T",
        """
            SELECT RunningTime
            FROM QUERY
            WHERE IdentificationMethod LIKE 'Plane'
        """
    )

    attach_std(
        "Sphere Query STD T",
        """
            SELECT RunningTime
            FROM QUERY
            WHERE IdentificationMethod LIKE 'Sphere'
        """
    )

    attach_std(
        "Composite Query STD T",
        """
            SELECT RunningTime
            FROM QUERY
            WHERE IdentificationMethod LIKE 'Composite'
        """
    )

    attach_std(
        "Pyramid Query STD T",
        """
            SELECT RunningTime
            FROM QUERY
            WHERE IdentificationMethod LIKE 'Pyramid'
        """
    )

    attach_std(
        "Dot Query STD T",
        """
            SELECT RunningTime
            FROM QUERY
            WHERE IdentificationMethod LIKE 'Dot'
        """
    )

    attach_std(
        "Angle Query STD T",
        """
            SELECT RunningTime
            FROM QUERY
            WHERE IdentificationMethod LIKE 'Angle'
        """
    )

    attach_r(
        ["Angle AVG Query CSS", "Composite AVG Query CSS", "Dot AVG Query CSS", "Plane AVG Query CSS",
         "Pyramid AVG Query CSS", "Sphere AVG Query CSS"],
        """
            SELECT AVG(CandidateSetSize)
            FROM QUERY
            GROUP BY IdentificationMethod
        """
    )

    attach_r(
        "Pyramid Query COUNT ACC NOT 1",
        """
            SELECT COUNT(*)
            FROM QUERY
            WHERE IdentificationMethod LIKE 'Pyramid'
            AND SExistence <> 1
        """
    )

    attach_r(
        ["Angle AVG Query T", "Composite AVG Query T", "Dot AVG Query T", "Plane AVG Query T", "Pyramid AVG Query T",
         "Sphere AVG Query T"],
        """
            SELECT AVG(RunningTime)
            FROM QUERY
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
        "Composite Query COUNT CSS > 1",
        """
            SELECT COUNT(*)
            FROM QUERY
            WHERE IdentificationMethod LIKE 'Composite'
            AND CandidateSetSize <> 1
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

    attach_std(
        "Composite Reduction STD QC QC != 1 10^-4 Shift",
        """
        SELECT QueryCount
        FROM REDUCTION
        WHERE rowid IN (
            SELECT rowid
            FROM REDUCTION
            WHERE IdentificationMethod LIKE 'Composite'
            AND ShiftDeviation < 1.0e-3 AND ShiftDeviation > 1.0e-5 AND FalseStars = 0
            AND QueryCount > 1
        )
        """
    )

    attach_std(
        "Plane Reduction STD QC QC != 1 10^-4 Shift",
        """
        SELECT QueryCount
        FROM REDUCTION
        WHERE rowid IN (
            SELECT rowid
            FROM REDUCTION
            WHERE IdentificationMethod LIKE 'Plane'
            AND ShiftDeviation < 1.0e-3 AND ShiftDeviation > 1.0e-5 AND FalseStars = 0
            AND QueryCount > 1
        )
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
        ["Angle Reduction AVG ACC NN", "Composite Reduction AVG ACC NN", "Dot Reduction AVG ACC NN",
         "Plane Reduction AVG ACC NN", "Pyramid Reduction AVG ACC NN", "Sphere Reduction AVG ACC NN"],
        """
            SELECT AVG(PercentageCorrect), IdentificationMethod
            FROM REDUCTION
            WHERE ShiftDeviation < 1.0e-6 AND FalseStars = 0
            GROUP BY IdentificationMethod
            ORDER BY IdentificationMethod
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
        "(Composite, Triangle) Reduction AVG T NN",
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
        "(Composite, Triangle) Reduction STD T NN",
        """
            SELECT AVG(TimeToResult)
            FROM REDUCTION
            WHERE ShiftDeviation < 1.0e-6 AND FalseStars = 0
            AND (IdentificationMethod LIKE 'Composite' OR
                 IdentificationMethod LIKE 'Plane' OR
                 IdentificationMethod LIKE 'Sphere')
            GROUP BY IdentificationMethod
        """
    )

    attach_r(
        "(Composite, Triangle) Reduction ACC 0 to 10^-5 Shift",
        """
            SELECT 1.0 - AVG(PercentageCorrect)
            FROM REDUCTION
            WHERE ShiftDeviation > 1.0e-6 AND ShiftDeviation < 1.0e-4
            AND (IdentificationMethod LIKE 'Composite' OR
                 IdentificationMethod LIKE 'Plane' OR
                 IdentificationMethod LIKE 'Sphere')
        """
    )

    attach_r(
        "(Pyramid, Angle) Reduction ACC 10^-4 to 10^-3 Shift",
        """
            SELECT A1 - A2
            FROM 
                (SELECT AVG(PercentageCorrect) AS A1
                FROM REDUCTION
                WHERE ShiftDeviation > 1.0e-5 AND ShiftDeviation < 1.0e-3
                AND (IdentificationMethod LIKE 'Angle' OR
                     IdentificationMethod LIKE 'Pyramid')),
                (SELECT AVG(PercentageCorrect) AS A2
                FROM REDUCTION
                WHERE ShiftDeviation > 1.0e-4 AND ShiftDeviation < 1.0e-2
                AND (IdentificationMethod LIKE 'Angle' OR
                     IdentificationMethod LIKE 'Pyramid'))
        """
    )

    attach_r(
        "Dot Reduction ACC 10^-3 to 10^-2 Shift",
        """
            SELECT A1 - A2
            FROM
                (SELECT AVG(PercentageCorrect) AS A1
                FROM REDUCTION
                WHERE ShiftDeviation > 1.0e-4 AND ShiftDeviation < 1.0e-2
                AND IdentificationMethod LIKE 'Dot'),
                (SELECT AVG(PercentageCorrect) AS A2
                FROM REDUCTION
                WHERE ShiftDeviation > 1.0e-3 AND ShiftDeviation < 1.0e-1
                AND IdentificationMethod LIKE 'Dot')
        """
    )

    attach_r(
        "(Composite, Triangle) Reduction AVG ACC 10^-4 Shift",
        """
            SELECT AVG(PercentageCorrect)
            FROM REDUCTION
            WHERE ShiftDeviation > 1.0e-5 AND ShiftDeviation < 1.0e-3
            AND (IdentificationMethod LIKE 'Composite' OR
                 IdentificationMethod LIKE 'Plane' OR
                 IdentificationMethod LIKE 'Sphere')
        """
    )

    attach_std(
        "(Composite, Triangle) Reduction STD ACC 10^-4 Shift",
        """
            SELECT AVG(PercentageCorrect)
            FROM REDUCTION
            WHERE ShiftDeviation > 1.0e-5 AND ShiftDeviation < 1.0e-3
            AND (IdentificationMethod LIKE 'Composite' OR
                 IdentificationMethod LIKE 'Plane' OR
                 IdentificationMethod LIKE 'Sphere')
            GROUP BY IdentificationMethod
        """
    )

    attach_r(
        "(Composite, Triangle) Reduction AVG ACC 10^-3 Shift",
        """
            SELECT AVG(PercentageCorrect)
            FROM REDUCTION
            WHERE ShiftDeviation > 1.0e-4 AND ShiftDeviation < 1.0e-2
            AND (IdentificationMethod LIKE 'Composite' OR
                 IdentificationMethod LIKE 'Plane' OR
                 IdentificationMethod LIKE 'Sphere')
        """
    )

    attach_r(
        "(Composite, Triangle) Reduction AVG ACC 10^-2 Shift",
        """
            SELECT AVG(PercentageCorrect)
            FROM REDUCTION
            WHERE ShiftDeviation > 1.0e-3 AND ShiftDeviation < 1.0e-1
            AND (IdentificationMethod LIKE 'Composite' OR
                 IdentificationMethod LIKE 'Plane' OR
                 IdentificationMethod LIKE 'Sphere')
        """
    )

    attach_r(
        "Angle Reduction T 10^-4 to 10^-3 Shift",
        """
            SELECT R1 - R2
            FROM
                (SELECT AVG(TimeToResult) AS R1
                FROM REDUCTION
                WHERE ShiftDeviation > 1.0e-5 AND ShiftDeviation < 1.0e-3
                AND IdentificationMethod LIKE 'Angle'),
                (SELECT AVG(TimeToResult) AS R2
                FROM REDUCTION
                WHERE ShiftDeviation > 1.0e-4 AND ShiftDeviation < 1.0e-2
                AND IdentificationMethod LIKE 'Angle')
        """
    )

    attach_r(
        "Angle Reduction QC 10^-4 to 10^-3 Shift",
        """
            SELECT R1 - R2
            FROM
                (SELECT AVG(QueryCount) AS R1
                FROM REDUCTION
                WHERE ShiftDeviation > 1.0e-5 AND ShiftDeviation < 1.0e-3
                AND IdentificationMethod LIKE 'Angle'),
                (SELECT AVG(QueryCount) AS R2
                FROM REDUCTION
                WHERE ShiftDeviation > 1.0e-4 AND ShiftDeviation < 1.0e-2
                AND IdentificationMethod LIKE 'Angle')
        """
    )

    attach_r(
        "(Composite, Triangle) Reduction T 10^-5 to 10^-4 Shift",
        """
            SELECT R2 - R1
            FROM 
                (SELECT AVG(TimeToResult) AS R1
                FROM REDUCTION
                WHERE ShiftDeviation > 1.0e-6 AND ShiftDeviation < 1.0e-4
                AND (IdentificationMethod LIKE 'Composite' OR
                     IdentificationMethod LIKE 'Plane' OR
                     IdentificationMethod LIKE 'Sphere')),
                (SELECT AVG(TimeToResult) AS R2
                FROM REDUCTION
                WHERE ShiftDeviation > 1.0e-5 AND ShiftDeviation < 1.0e-3
                AND (IdentificationMethod LIKE 'Composite' OR
                     IdentificationMethod LIKE 'Plane' OR
                     IdentificationMethod LIKE 'Sphere'))
        """
    )

    attach_r(
        "(Composite, Triangle) Reduction QC 10^-5 to 10^-4 Shift",
        """
            SELECT R2 - R1
            FROM 
                (SELECT AVG(QueryCount) AS R1
                FROM REDUCTION
                WHERE ShiftDeviation > 1.0e-6 AND ShiftDeviation < 1.0e-4
                AND (IdentificationMethod LIKE 'Composite' OR
                     IdentificationMethod LIKE 'Plane' OR
                     IdentificationMethod LIKE 'Sphere')),
                (SELECT AVG(QueryCount) AS R2
                FROM REDUCTION
                WHERE ShiftDeviation > 1.0e-5 AND ShiftDeviation < 1.0e-3
                AND (IdentificationMethod LIKE 'Composite' OR
                     IdentificationMethod LIKE 'Plane' OR
                     IdentificationMethod LIKE 'Sphere'))
        """
    )

    attach_r(
        "(Composite, Triangle) Reduction T 10^-4 to 10^-3 Shift",
        """
            SELECT R2 - R1
            FROM 
                (SELECT AVG(TimeToResult) AS R1
                FROM REDUCTION
                WHERE ShiftDeviation > 1.0e-5 AND ShiftDeviation < 1.0e-3
                AND (IdentificationMethod LIKE 'Composite' OR
                     IdentificationMethod LIKE 'Plane' OR
                     IdentificationMethod LIKE 'Sphere')),
                (SELECT AVG(TimeToResult) AS R2
                FROM REDUCTION
                WHERE ShiftDeviation > 1.0e-4 AND ShiftDeviation < 1.0e-2
                AND (IdentificationMethod LIKE 'Composite' OR
                     IdentificationMethod LIKE 'Plane' OR
                     IdentificationMethod LIKE 'Sphere'))
        """
    )

    attach_r(
        "(Composite, Triangle) Reduction QC 10^-4 to 10^-3 Shift",
        """
            SELECT R2 - R1
            FROM 
                (SELECT AVG(QueryCount) AS R1
                FROM REDUCTION
                WHERE ShiftDeviation > 1.0e-5 AND ShiftDeviation < 1.0e-3
                AND (IdentificationMethod LIKE 'Composite' OR
                     IdentificationMethod LIKE 'Plane' OR
                     IdentificationMethod LIKE 'Sphere')),
                (SELECT AVG(QueryCount) AS R2
                FROM REDUCTION
                WHERE ShiftDeviation > 1.0e-4 AND ShiftDeviation < 1.0e-2
                AND (IdentificationMethod LIKE 'Composite' OR
                     IdentificationMethod LIKE 'Plane' OR
                     IdentificationMethod LIKE 'Sphere'))
        """
    )

    attach_r(
        "(Composite, Triangle) Reduction QC 10^-4 / QC 10^-3 Shift",
        """
            SELECT R2 / R1
            FROM 
                (SELECT AVG(QueryCount) AS R1
                FROM REDUCTION
                WHERE ShiftDeviation > 1.0e-5 AND ShiftDeviation < 1.0e-3
                AND (IdentificationMethod LIKE 'Composite' OR
                     IdentificationMethod LIKE 'Plane' OR
                     IdentificationMethod LIKE 'Sphere')),
                (SELECT AVG(QueryCount) AS R2
                FROM REDUCTION
                WHERE ShiftDeviation > 1.0e-4 AND ShiftDeviation < 1.0e-2
                AND (IdentificationMethod LIKE 'Composite' OR
                     IdentificationMethod LIKE 'Plane' OR
                     IdentificationMethod LIKE 'Sphere'))
        """
    )

    attach_r(
        "Dot Reduction T 10^-3 to 10^-2 Shift",
        """
            SELECT R2 - R1
            FROM 
                (SELECT AVG(TimeToResult) AS R1
                FROM REDUCTION
                WHERE ShiftDeviation > 1.0e-4 AND ShiftDeviation < 1.0e-2
                AND IdentificationMethod LIKE 'Dot'),
                (SELECT AVG(TimeToResult) AS R2
                FROM REDUCTION
                WHERE ShiftDeviation > 1.0e-3 AND ShiftDeviation < 1.0e-1
                AND IdentificationMethod LIKE 'Dot')
        """
    )

    attach_r(
        "Dot Reduction QC 10^-3 to 10^-2 Shift",
        """
            SELECT R2 - R1
            FROM 
                (SELECT AVG(QueryCount) AS R1
                FROM REDUCTION
                WHERE ShiftDeviation > 1.0e-4 AND ShiftDeviation < 1.0e-2
                AND IdentificationMethod LIKE 'Dot'),
                (SELECT AVG(QueryCount) AS R2
                FROM REDUCTION
                WHERE ShiftDeviation > 1.0e-3 AND ShiftDeviation < 1.0e-1
                AND IdentificationMethod LIKE 'Dot')
        """
    )

    attach_r(
        "Pyramid Reduction AVG T Shift",
        """
            SELECT AVG(TimeToResult)
            FROM REDUCTION
            WHERE IdentificationMethod Like 'Pyramid'
            AND FalseStars = 0
        """
    )

    attach_std(
        "Pyramid Reduction STD T Shift",
        """
            SELECT AVG(TimeToResult)
            FROM REDUCTION
            WHERE IdentificationMethod Like 'Pyramid'
            AND FalseStars = 0
            GROUP BY ShiftDeviation
        """
    )

    attach_r(
        "Pyramid Reduction QC 10^-4 to 10^-5 Shift",
        """
            SELECT A2 - A1
            FROM 
                (SELECT AVG(QueryCount) AS A1
                FROM REDUCTION
                WHERE IdentificationMethod LIKE 'Pyramid'
                AND ShiftDeviation > 1.0e-6 AND ShiftDeviation < 1.0e-4),
                (SELECT AVG(QueryCount) AS A2
                FROM REDUCTION
                WHERE IdentificationMethod LIKE 'Pyramid'
                AND ShiftDeviation > 1.0e-5 AND ShiftDeviation < 1.0e-3)
        """
    )

    attach_r(
        ["Angle Reduction ACC 0 to 12 False", "Composite Reduction ACC 0 to 12 False",
         "Dot Reduction ACC 0 to 12 False", "Plane Reduction ACC 0 to 12 False", "Pyramid Reduction ACC 0 to 12 False",
         "Sphere Reduction ACC 0 to 12 False"],
        """
            SELECT R1.A1 - R2.A2
            FROM 
                -- sigma_{R1.I = R2.I}(R1 x R2), i forgot how joins work ): --
                (SELECT AVG(PercentageCorrect) AS A1, IdentificationMethod AS I
                FROM REDUCTION
                WHERE ShiftDeviation < 1.0e-7 AND FalseStars = 0
                GROUP BY IdentificationMethod) AS R1,
                (SELECT AVG(PercentageCorrect) AS A2, IdentificationMethod AS I
                FROM REDUCTION
                WHERE FalseStars = 12
                GROUP BY IdentificationMethod) AS R2
            WHERE R1.I = R2.I
            ORDER BY R1.I
        """
    )

    attach_r(
        ["Angle Reduction QC 0 to 12 False", "Composite Reduction QC 0 to 12 False",
         "Dot Reduction QC 0 to 12 False", "Plane Reduction QC 0 to 12 False", "Pyramid Reduction QC 0 to 12 False",
         "Sphere Reduction QC 0 to 12 False"],
        """
            SELECT R2.A2 - R1.A1
            FROM 
                (SELECT AVG(QueryCount) AS A1, IdentificationMethod AS I
                FROM REDUCTION
                WHERE ShiftDeviation < 1.0e-7 AND FalseStars = 0
                GROUP BY IdentificationMethod) AS R1,
                (SELECT AVG(QueryCount) AS A2, IdentificationMethod AS I
                FROM REDUCTION
                WHERE FalseStars = 12
                GROUP BY IdentificationMethod) AS R2
            WHERE R1.I = R2.I
            ORDER BY R1.I
        """
    )

    attach_r(
        ["Angle Reduction T 0 to 12 False", "Composite Reduction T 0 to 12 False",
         "Dot Reduction T 0 to 12 False", "Plane Reduction T 0 to 12 False", "Pyramid Reduction T 0 to 12 False",
         "Sphere Reduction T 0 to 12 False"],
        """
            SELECT R2.A2 - R1.A1
            FROM 
                (SELECT AVG(TimeToResult) AS A1, IdentificationMethod AS I
                FROM REDUCTION
                WHERE ShiftDeviation < 1.0e-7 AND FalseStars = 0
                GROUP BY IdentificationMethod) AS R1,
                (SELECT AVG(TimeToResult) AS A2, IdentificationMethod AS I
                FROM REDUCTION
                WHERE FalseStars = 12
                GROUP BY IdentificationMethod) AS R2
            WHERE R1.I = R2.I
            ORDER BY R1.I
        """
    )

    attach_r(
        "Pyramid Reduction QS 0 to 12 False",
        """
            SELECT ( A2 - A1 )/ 3.0
            FROM 
                (SELECT AVG(QueryCount) AS A1
                FROM REDUCTION
                WHERE ShiftDeviation < 1.0e-7 AND FalseStars = 0
                AND IdentificationMethod LIKE 'Pyramid'),
                (SELECT AVG(QueryCount) AS A2
                FROM REDUCTION
                WHERE FalseStars = 12
                AND IdentificationMethod LIKE 'Pyramid')
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
    # (I'm sorry, just know it requires 3 nested loops).
    is_method = lambda i_0, j_0, k_0: ((('Triangle' in k_0 and (j_0 == 'Plane' or j_0 == 'Sphere')) or j_0 in k_0)
                                       and i_0 in k_0)
    list(map(lambda k: list(map(lambda j: list(map(
        lambda i: p[j][i].update({k.replace(j + ' ', '').replace(i + ' ', ''):
                                      results[k][0] if len(results[k]) == 1 else results[k]})
        if is_method(i, j, k) else None, p[j])), p)), results))

    pp = PrettyPrinter(indent=1, width=120)
    pp.pprint(p)
