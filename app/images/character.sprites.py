image('character.xcf')
grid(12, 12)
scale(8)

sprites(
    character=[
               loop((floodfill((0.5, 0.5),  origin='center'), 1)), #biggrin
               loop((floodfill((2, 0.5),    origin='center'), 1)), #confused
               loop((floodfill((3, 0.5),    origin='center'), 1)), #dead
               loop((floodfill((4.5, 0.5),  origin='center'), 1)), #determined
               loop((floodfill((5.5, 0.5),  origin='center'), 1)), #exclaim
               loop((floodfill((7, 0.5),    origin='center'), 1)), #hurt
               loop((floodfill((8, 0.5),    origin='center'), 1)), #shrug
               loop((floodfill((10, 0.5),   origin='center'), 1)), #sad
               loop((floodfill((11, 0.5),   origin='center'), 1)), #smile
               loop((floodfill((7, 2),      origin='center'), 1)), #smug
               loop((floodfill((8, 2),      origin='center'), 1)), #yell
               loop((floodfill((1, 4),      origin='center'), 1)), #mag
               loop((floodfill((1, 6)), 1)),                       #aim
               loop((floodfill((10, 2.5)), 3), (floodfill((10, 4.5)), 0.2)), #ready
               loop(
                    (floodfill((0.5, 2), origin='center'), 0.2),
                    (floodfill((2, 2), origin='center'), 0.2),
                    (floodfill((3, 2), origin='center'), 0.2),
                    (floodfill((4.5, 2), origin='center'), 0.2),
                    (floodfill((5.5, 2), origin='center'), 0.2),
               ),
               loop(
                    (floodfill((3, 6)), 0.1),
                    (floodfill((6, 6)), 0.1),
                    (floodfill((9, 6)), 0.1),
                    (floodfill((1, 8)), 0.1),
                    (floodfill((3, 8)), 0.1),
                    (floodfill((5, 8)), 0.1),
                    (floodfill((7, 8)), 0.1),
                    (floodfill((9, 8)), 0.1),
                    (floodfill((11, 8)), 0.1),
                    (floodfill((2, 10)), 0.1),
                    (floodfill((5, 10)), 0.1),
                    (floodfill((7, 10)), 0.1),
                    (floodfill((9, 10)), 0.1),
                    (floodfill((11, 10)), 0.1),
                    (floodfill((5, 4)), 0.1),
                    (floodfill((7, 4)), 0.1),
                    (floodfill((3, 6)), 99),
                ),

    ],
)
