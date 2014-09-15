image('atlas.xcf')
grid(12, 12)
scale(10)

def character(X, Y):
    return [floodfill((3 * X + x + 0.5, 3 * Y + y + 0.5), n=20)
            for (x, y) in [(0, 0), (1, 0), (2, 0), (1, 1), (2, 2)]]

def door(X, Y):
    return floodfill((X, Y), scale=8, origin='centroid')

sprites(
    title=floodfill((3, 2), scale=7, origin='centroid'),
    play=[
        floodfill((10, 2), scale=8, origin='center'),
        floodfill((10, 5), scale=8, origin='center')
    ],
)
