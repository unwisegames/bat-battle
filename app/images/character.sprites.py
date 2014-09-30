image('character.xcf')
grid(12, 12)
scale(15)

def frame(X, Y, duration):
    return (floodfill((X, Y), origin='center'), duration)

sprites(
    character=[
               loop(frame(10.5, 1.5,    1)),    #biggrin
               loop(frame(10.5, 1.5,    1)),    #rescued
               loop(frame(2, 2.5,       1)),    #confused
               loop(frame(8, 2.5,       1)),    #dead
               loop(
                    frame(9, 2.5,       3),     #determined
                    frame(10.5, 2.5,    0.2)
               ),
               loop(frame(0.5, 3.5,     1)),    #exclaim
               loop(frame(2, 3.5,       1)),    #hurt
               loop(frame(9, 3.5,       1)),    #shrug
               loop(frame(8, 3.5,       1)),    #sad
               loop(frame(10.5, 3.5,    1)),    #smile
               loop(frame(0.5, 4.5,     1)),    #smug
               loop(frame(2, 4.5,       1)),    #yell
               loop(frame(2, 4.5,       1)),    #startled
               loop(frame(3, 3.5,       1)),    #mag
               loop(frame(9, 1.5,       1)),    #aim
               loop(
                    frame(4.5, 3.5,     0.1),   #ready
                    frame(5.5, 3.5,     0.1),
                    #frame(5, 8.5,  0.2)
               ),
               loop(
                    frame(9, 1.5,       0.2),   #shooting
                    frame(9, 1.5,       0.2),
                    ),
               loop(
                    frame(6.7, 2.5,     0.4),   #crying
                    frame(3, 2.5,       0.4),
                    frame(4.3, 2.5,     0.4),
                    frame(5.5, 2.5,     0.4),
                    frame(0.5, 2.5,     0.4),
               ),
               loop(
                    frame(0.5, 0.5,     0.1),   #reloading
                    frame(10.5, 0.5,    0.1),
                    frame(0.5, 1.5,     0.1),
                    frame(2, 1.5,       0.1),
                    frame(3, 1.5,       0.1),
                    frame(4.5, 1.5,     0.1),
                    frame(5.5, 1.5,     0.1),
                    frame(7, 1.5,       0.1),
                    frame(8, 1.5,       0.1),
                    frame(2, 0.5,       0.1),
                    frame(3, 0.5,       0.1),
                    frame(4.3, 0.5,     0.1),
                    frame(5.5, 0.5,     0.1),
                    frame(6.7, 0.5,     0.1),
                    frame(8, 0.5,       0.1),
                    frame(9, 0.5,       0.1),
                    frame(0.5, 0.5,     0.1),
                    #                    frame(1, 1,     99),
               ),

    ],
)
