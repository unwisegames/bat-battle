image('character.xcf')
grid(12, 12)
scale(10)

def frame(X, Y, duration):
    return (floodfill((X, Y), origin='center'), duration)

sprites(
    character=[
               loop(frame(11, 4,    1)),    #biggrin
               loop(frame(3, 5.5,   1)),    #confused
               loop(frame(1, 7,     1)),    #dead
               loop(
                    frame(3, 7,     3),     #determined
                    frame(5, 7,     0.2)
               ),
               loop(frame(7, 7,     1)),    #exclaim
               loop(frame(9, 7,     1)),    #hurt
               loop(frame(7, 8.5,   1)),    #shrug
               loop(frame(9, 8.5,   1)),    #sad
               loop(frame(11, 8.5,  1)),    #smile
               loop(frame(1, 10,    1)),    #smug
               loop(frame(3, 10,    1)),    #yell
               loop(frame(11, 7,    1)),    #mag
               loop(frame(9, 4,     1)),    #aim
               loop(
                    frame(1, 8.5,   0.1),   #ready
                    frame(3, 8.5,   0.1),
                    #frame(5, 8.5,  0.2)
               ),
               loop(
                    frame(11, 5.5,  0.4),   #crying
                    frame(5, 5.5,   0.4),
                    frame(7, 5.5,   0.4),
                    frame(9, 5.5,   0.4),
                    frame(1, 5.5,   0.4),
               ),
               loop(
                    frame(1, 1,     0.1),   #reloading
                    frame(5, 2.5,   0.1),
                    frame(7, 2.5,   0.1),
                    frame(9, 2.5,   0.1),
                    frame(11, 2.5,  0.1),
                    frame(1, 4,     0.1),
                    frame(3, 4,     0.1),
                    frame(5, 4,     0.1),
                    frame(7, 4,     0.1),
                    frame(3, 1,     0.1),
                    frame(5, 1,     0.1),
                    frame(7, 1,     0.1),
                    frame(9, 1,     0.1),
                    frame(11, 1,    0.1),
                    frame(1, 2.5,   0.1),
                    frame(3, 2.5,   0.1),
                    frame(1, 1,     0.1),
                    frame(1, 1,     99),
               ),

    ],
)
