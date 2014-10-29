image('bats.xcf')
grid(12, 12)
scale(12)

def bat(*points):
    frames = [(floodfill(p), 0.15) for p in points]
    return [
        loop(*frames),
            # this was a disgraceful way of getting this going - will clean it up!
        loop(
             (floodfill((1.23, 0.65)), 0.15),
             (floodfill((3.9, 0.65)), 0.15),
             (floodfill((6.05, 0.42)), 0.15),
             (floodfill((3.9, 0.65)), 0.15),
        ),
        loop(
             (floodfill((1.23, 3.7)), 0.15),
             (floodfill((3.82, 3.7)), 0.15),
             (floodfill((6.2, 3.55)), 0.15),
             (floodfill((3.82, 3.7)), 0.15),
             ),
        loop(
             (floodfill((10.8, 0.9)), 0.15),
             (floodfill((10.8, 2.45)), 0.15),
             (floodfill((10.7, 4)), 0.15),
             (floodfill((11.1, 5.55)), 0.15),
        ),
        loop(
             (floodfill((1, 5.5), scale=20), 0.1),
             (floodfill((3, 5.5), scale=20), 0.1),
             (floodfill((5, 5.5), scale=20), 0.1),
             (floodfill((7, 5.5), scale=20), 0.1),
             (floodfill((9, 5.5), scale=20), 0.1),
             (floodfill((1, 7.5), scale=20), 0.1),
             (floodfill((3, 7.5), scale=20), 0.1),
             (floodfill((5, 7.5), scale=20), 0.1),
             (floodfill((7, 7.5), scale=20), 0.1),
             (floodfill((9, 7.5), scale=20), 0.1),
        ),
    ]

sprites(
    bat = floodfill((4, 0.5)),
    wings=floodfill((0.5, 1), scale=20, origin='center'),
    bats=[
        bat((1.5, 2.27), (3.98, 2.21), (6.32, 1.86), (8.86, 2.21)),
        bat((1.33, 10.27), (4.13, 10.27), (6.42, 10.02), (8.9 , 10.27)),
    ],
)
