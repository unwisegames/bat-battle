image('bats.xcf')
grid(12, 12)
scale(10)

def bat(*points):
    frames = [(floodfill(p), 0.15) for p in points]
    return [loop(*frames)]

sprites(
    bats=[
        bat((1.5, 2.27), (3.98, 2.21), (6.32, 1.86), (8.86, 2.21)),
        bat((1.33, 7.27), (4.13, 7.27), (6.42, 7.02), (8.9 , 7.27)),
    ],
)
