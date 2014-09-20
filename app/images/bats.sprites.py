image('bats.xcf')
grid(12, 12)
scale(10)

def bat(*points):
    frames = [(floodfill(p), 0.15) for p in points]
    return [
        loop(*frames),
            # this was a disgraceful way of getting this going - will clean it up!
        loop(*frames),
        loop(*frames),
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
    bats=[
        bat((1.5, 2.27), (3.98, 2.21), (6.32, 1.86), (8.86, 2.21)),
        bat((1.33, 10.27), (4.13, 10.27), (6.42, 10.02), (8.9 , 10.27)),
    ],
)
