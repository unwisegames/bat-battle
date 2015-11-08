image('blast.xcf')
grid(12, 12)
scale(30)

def blast_frame(X, Y):
    return floodfill((X, Y), origin='centroid')

def char_blast_frame(X, Y):
    return floodfill((X, Y), scale=10, origin='centroid')

sprites(
    blast=[
          loop(
               (blast_frame(1, 1.2),    0.1),
               (blast_frame(3, 1),      0.1),
               (blast_frame(6, 1),      0.1),
               (blast_frame(8, 1),      0.1),
               (blast_frame(11, 1),     0.1),
               (blast_frame(1, 3),      0.1),
               (blast_frame(3, 3),      0.1),
               (blast_frame(6, 3),      0.1),
               (blast_frame(8, 3),      0.1),
               (blast_frame(10, 3),     0.1),
               (blast_frame(1, 5),      99),
               ),
    ],
    characterblast=[
           loop(
                (char_blast_frame(1, 1.2),    0.1),
                (char_blast_frame(3, 1),      0.1),
                (char_blast_frame(6, 1),      0.1),
                (char_blast_frame(8, 1),      0.1),
                (char_blast_frame(11, 1),     0.1),
                (char_blast_frame(1, 3),      0.1),
                (char_blast_frame(3, 3),      0.1),
                (char_blast_frame(6, 3),      0.1),
                (char_blast_frame(8, 3),      0.1),
                (char_blast_frame(10, 3),     0.1),
                (char_blast_frame(1, 5),      99),
                ),
           ],
    blackbat=floodfill((7, 9), scale=8, origin='center'),
    blackbat2=floodfill((6, 6), scale=10, origin='center'),
    blackbat3=floodfill((10, 6), scale=10, origin='center'),

)
