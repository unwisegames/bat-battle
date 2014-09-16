image('characters.xcf')
grid(12, 12)
scale(10)

def character(X, Y):
    return [floodfill((3 * X + x + 0.5, 3 * Y + y + 0.5), n=20)
            for (x, y) in [(0, 0), (1, 0), (2, 0), (1, 1), (2, 2)]]

sprites(
    characters=[
        character(0, 0),
        character(1, 0),
        character(2, 0),
        character(3, 0),
        character(0, 2),
    ],
    dart=floodfill((4.5, 6.5), scale=5),
)
