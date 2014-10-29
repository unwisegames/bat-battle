image('atlas2.xcf')
grid(12, 12)
scale(10)

sprites(
    passed=floodfill((6, 2), scale=7, origin='center'),
    failed=floodfill((6, 4), scale=7, origin='center'),
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
    title=floodfill((6, 8.5), origin='center'),

)
