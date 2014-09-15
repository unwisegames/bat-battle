image('bats.xcf')
grid(12, 12)
scale(10)

def character(X, Y):
    return [floodfill((3 * X + x + 0.5, 3 * Y + y + 0.5), n=20)
            for (x, y) in [(0, 0), (1, 0), (2, 0), (1, 1), (2, 2)]]

def bat(X, Y):
    return floodfill((X, Y), scale=8, origin=(X+1, Y))

sprites(
    greybat=[
        loop(
             (floodfill((1, 2.1), scale=10, origin=(1.5, 2.27)), 0.15),
             (floodfill((4, 2.1), scale=10, origin=(3.98, 2.21)), 0.15),
             (floodfill((6, 2), scale=10, origin=(6.32, 1.86)), 0.15),
             (floodfill((9, 2.1), scale=10, origin=(8.86, 2.21)), 0.15),
        )
    ],
    yellowbat=[
        loop(
             (floodfill((1, 7.1), scale=10, origin=(1.33, 7.27)), 0.15),
             (floodfill((4, 7.1), scale=10, origin=(4.13, 7.27)), 0.15),
             (floodfill((6, 7.1), scale=10, origin=(6.42, 7.02)), 0.15),
             (floodfill((9, 7.1), scale=10, origin=(8.9, 7.27)), 0.15),
        )
    ],
)
