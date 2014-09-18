image('reload.xcf')
grid(12, 12)
scale(10)

def character(X, Y):
    return [floodfill((3 * X + x + 0.5, 3 * Y + y + 0.5), n=20)
            for (x, y) in [(0, 0), (1, 0), (2, 0), (1, 1), (2, 2)]]

def door(X, Y):
    return floodfill((X, Y), scale=8, origin='centroid')

sprites(
    reload=[
        loop(
             #(floodfill((0.5, 0.5), scale=10, origin=(0.5, 0.5)), 0.1),
             (floodfill((1, 7), scale=10, origin=(0.9, 6.65)), 0.1),
             (floodfill((2, 0.5), scale=10, origin=(2.2, 0.5)), 0.1),
             (floodfill((4.5, 0.5), scale=10, origin=(4.4, 0.5)), 0.1),
             (floodfill((7, 0.8), scale=10, origin=(6.67, 0.7)), 0.1),
             (floodfill((8.5, 1), scale=10, origin=(8.6, 0.9)), 0.1),
             (floodfill((11, 1), scale=10, origin=(10.75, 0.77)), 0.1),
             (floodfill((1, 3), scale=10, origin=(0.73, 2.97)), 0.1),
             (floodfill((3, 3), scale=10, origin=(2.82, 2.95)), 0.1),
             (floodfill((5, 3), scale=10, origin=(5.16, 3)), 0.1),
             (floodfill((8, 3), scale=10, origin=(7.8, 2.92)), 0.1),
             (floodfill((10.5, 3), scale=10, origin=(10.46, 2.7)), 0.1),
             (floodfill((1, 5), scale=10, origin=(0.9, 4.9)), 0.1),
             (floodfill((3.5, 5), scale=10, origin=(3.42, 4.94)), 0.1),
             (floodfill((5.5, 5), scale=10, origin=(5.4, 4.85)), 0.1),
             (floodfill((7.5, 5), scale=10, origin=(7.6, 4.9)), 0.1),
             (floodfill((10, 5), scale=10, origin=(9.76, 4.94)), 0.1),
             (floodfill((1, 7), scale=10, origin=(0.9, 6.65)), 0.1),
        )
    ],
)
