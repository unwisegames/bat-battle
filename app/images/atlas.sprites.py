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
        floodfill((2, 5), scale=8, origin='center'),
        floodfill((4, 8), scale=8, origin='center')
    ],
    restart=[
        floodfill((7, 1), scale=8, origin='center'),
        floodfill((9, 1), scale=8, origin='center')
    ],
    back=[
        floodfill((11, 1), scale=8, origin='center'),
        floodfill((1, 9.5), scale=8, origin='center')
    ],
    gameovertext=floodfill((4, 11), scale=10, origin='center'),
    gameover=floodfill((9, 6), scale=10, origin='center'),
    fade=floodfill((10, 11), scale=999, origin='center'),
    bathead=floodfill((4.5, 3.5), origin='center')
)
