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
    ],
)
