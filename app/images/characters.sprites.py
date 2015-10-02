image('characters.xcf')
grid(16, 16)
scale(10)

def character(x, y):
    x *= 4
    y *= 4
    return [s
            for row in rows(((x, y), (x + 4, y + 6)), origin=('aabb', (0, 0)))
            for s in row]

sprites(
    characters=[character(x, y) for (x, y) in [(0, 0), (1, 0), (2, 0), (3, 0), (0, 2)]],
    dart=floodfill((6, 9), scale=5),
    flame=floodfill((12, 9), scale=5, origin=(8, 9)),
    dot=floodfill((5, 11), scale=4),
    pause=[
        floodfill((11, 14), scale=8, origin='center'),
        floodfill((14, 14), scale=8, origin='center')
    ],
)
