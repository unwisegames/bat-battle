image('atlas2.xcf')
grid(12, 12)
scale(10)

def grave(X, Y):
    return floodfill((X, Y), scale=8, origin='center')

sprites(
    passed=floodfill((6, 2), scale=7, origin='center'),
        #failed=floodfill((6, 4), scale=7, origin='center'),
    target=floodfill((1, 6), origin='center'),
    clock=floodfill((2, 6), origin='center'),
    gamecenter=[
        floodfill((4, 6), scale=8, origin='center'),
        floodfill((6, 6), scale=8, origin='center')
    ],
    twitter=[
        floodfill((8, 6), scale=8, origin='center'),
        floodfill((10, 6), scale=8, origin='center')
    ],
    facebook=[
         floodfill((8, 3), scale=8, origin='center'),
         floodfill((10, 3), scale=8, origin='center')
         ],
    title=floodfill((6, 8.5), origin='center'),
        #grave=floodfill((11, 11), origin='center'),
    grave=[
          loop(
               (grave(1, 11), 0.05),
               (grave(3, 11), 0.05),
               (grave(5, 11), 0.05),
               (grave(6.5, 11), 0.05),
               (grave(8, 11), 0.05),
               (grave(10, 11), 0.05),
               (grave(11, 11), 0.05),
               ),
           loop((grave(11, 11), 1)),
    ],
    character1=floodfill((1, 4), scale=8, origin='center'),
    character2=floodfill((3, 4), scale=8, origin='center'),
    character3=floodfill((4.5, 4), scale=8, origin='center'),
    character4=floodfill((6.5, 4), scale=8, origin='center'),

)
