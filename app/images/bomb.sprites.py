image('bomb.xcf')
grid(10, 10)
scale(12)

def frame(X, Y, duration):
    return (floodfill((X, Y), scale=9, origin='center'), duration)

sprites(
    bomb=floodfill((1, 3), scale=9, origin='center'),
    blast=floodfill((3, 3), scale=30, origin='center'),
    bat=[
        loop(
            frame(1.25,     0.6, 0.15),     #flying
            frame(3.7,      0.6, 0.15),
            frame(6.15,     0.6, 0.15),
            frame(3.7,      0.6, 0.15),
        ),
        loop(
            frame(2, 4.5,   0.1),     #dying
            frame(5, 4.5,   0.1),
            frame(8, 4.5,   0.1),
            frame(2, 7,     0.1),
            frame(5, 7,     0.1),
            frame(8, 7,     99),
        ),
    ],
)
