image('bats.xcf')
grid(12, 12)
scale(12)

def ffl(points, interval, **kwargs):
    return loop(*[(floodfill(p, **kwargs), interval) for p in points])

def grey(points):
    return [
        ffl(points, 0.15),       # side
        #ffl(points, 0.15, X=(-1, 0)),       # side
        ffl([(1.23, 0.65), (3.9, 0.65), (6.05, 0.42), (3.9, 0.65)], 0.15), # front
        ffl([(1.23, 3.7), (3.82, 3.7), (6.2, 3.55), (3.82, 3.7)], 0.15), # rear
        ffl([(10.8, 0.9), (10.8, 2.45), (10.7, 4), (11.1, 5.55)], 0.15), # dying
        ffl(((x, y) for y in (5.5, 7.5) for x in (1, 3, 5, 7, 9)), 0.1, scale=20) # puff
    ]

def yellow(points):
    return [
        ffl(points, 0.15),       # side
        #ffl(points, 0.15, X=(-1, 0)),       # side
        ffl([(1.33, 10.27), (4.13, 10.27), (6.42, 10.02), (4.13 , 10.27)], 0.15), # front
        ffl([(1.35, 11.7), (4.1, 11.7), (6.38, 11.47), (4.1, 11.7)], 0.15), # rear
        ffl([(11.09, 7.26), (11.15, 8.77), (11.15, 10.28), (11.18, 11.84)], 0.15), # dying
        ffl(((x, y) for y in (5.5, 7.5) for x in (1, 3, 5, 7, 9)), 0.1, scale=20) # puff
    ]

sprites(
    #bat = floodfill((4, 0.5)),  # unused
    #wings=floodfill((0.5, 1), scale=20),  # broken (and unused)
    bats=[
        grey([(1.5 ,  2.27), (3.98,  2.21), (6.32,  1.86), (8.86,  2.21)]),
        yellow([(1.6, 8.85), (4.3, 8.85), (6.63, 8.61), (4.3 , 8.85)]),
    ],
)
