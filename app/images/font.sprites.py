# -*- coding: utf-8 -*-

image('font.xcf')
scale(10)

sprites(glyphs=font(ur'''
    01234567
    89ABCDE%
    FGHIJKL:/
    MNOPQRS
    TUVWXYZ.
    ''',
    baselines=guides()['horizontal']
))
