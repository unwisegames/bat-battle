image('atlas2.xcf')
grid(12, 12)
scale(10)

sprites(
    passed=floodfill((6, 2), scale=7, origin='center'),
    failed=floodfill((6, 4), scale=7, origin='center'),
    target=floodfill((1, 6), origin='center'),
    clock=floodfill((2, 6), origin='center')
)
