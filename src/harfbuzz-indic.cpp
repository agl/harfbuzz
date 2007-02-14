/*******************************************************************
 *
 *  Copyright 2007  Trolltech ASA
 *
 *  This is part of HarfBuzz, an OpenType Layout engine library.
 *
 *  See the file name COPYING for licensing information.
 *
 ******************************************************************/

#include "harfbuzz-shaper.h"
#include "harfbuzz-shaper-private.h"

#include <assert.h>

enum Form {
    Invalid = 0x0,
    UnknownForm = Invalid,
    Consonant,
    Nukta,
    Halant,
    Matra,
    VowelMark,
    StressMark,
    IndependentVowel,
    LengthMark,
    Control,
    Other
};

static const unsigned char indicForms[0xe00-0x900] = {
    // Devangari
    Invalid, VowelMark, VowelMark, VowelMark,
    Invalid, IndependentVowel, IndependentVowel, IndependentVowel,
    IndependentVowel, IndependentVowel, IndependentVowel, IndependentVowel,
    IndependentVowel, IndependentVowel, IndependentVowel, IndependentVowel,

    IndependentVowel, IndependentVowel, IndependentVowel, IndependentVowel,
    IndependentVowel, Consonant, Consonant, Consonant,
    Consonant, Consonant, Consonant, Consonant,
    Consonant, Consonant, Consonant, Consonant,

    Consonant, Consonant, Consonant, Consonant,
    Consonant, Consonant, Consonant, Consonant,
    Consonant, Consonant, Consonant, Consonant,
    Consonant, Consonant, Consonant, Consonant,

    Consonant, Consonant, Consonant, Consonant,
    Consonant, Consonant, Consonant, Consonant,
    Consonant, Consonant, UnknownForm, UnknownForm,
    Nukta, Other, Matra, Matra,

    Matra, Matra, Matra, Matra,
    Matra, Matra, Matra, Matra,
    Matra, Matra, Matra, Matra,
    Matra, Halant, UnknownForm, UnknownForm,

    Other, StressMark, StressMark, StressMark,
    StressMark, UnknownForm, UnknownForm, UnknownForm,
    Consonant, Consonant, Consonant, Consonant,
    Consonant, Consonant, Consonant, Consonant,

    IndependentVowel, IndependentVowel, VowelMark, VowelMark,
    Other, Other, Other, Other,
    Other, Other, Other, Other,
    Other, Other, Other, Other,

    Other, Other, Other, Other,
    Other, Other, Other, Other,
    Other, Other, Other, Other,
    Other, Other, Other, Other,

    // Bengali
    Invalid, VowelMark, VowelMark, VowelMark,
    Invalid, IndependentVowel, IndependentVowel, IndependentVowel,
    IndependentVowel, IndependentVowel, IndependentVowel, IndependentVowel,
    IndependentVowel, Invalid, Invalid, IndependentVowel,

    IndependentVowel, Invalid, Invalid, IndependentVowel,
    IndependentVowel, Consonant, Consonant, Consonant,
    Consonant, Consonant, Consonant, Consonant,
    Consonant, Consonant, Consonant, Consonant,

    Consonant, Consonant, Consonant, Consonant,
    Consonant, Consonant, Consonant, Consonant,
    Consonant, Invalid, Consonant, Consonant,
    Consonant, Consonant, Consonant, Consonant,

    Consonant, Invalid, Consonant, Invalid,
    Invalid, Invalid, Consonant, Consonant,
    Consonant, Consonant, UnknownForm, UnknownForm,
    Nukta, Other, Matra, Matra,

    Matra, Matra, Matra, Matra,
    Matra, Invalid, Invalid, Matra,
    Matra, Invalid, Invalid, Matra,
    Matra, Halant, Consonant, UnknownForm,

    Invalid, Invalid, Invalid, Invalid,
    Invalid, Invalid, Invalid, VowelMark,
    Invalid, Invalid, Invalid, Invalid,
    Consonant, Consonant, Invalid, Consonant,

    IndependentVowel, IndependentVowel, VowelMark, VowelMark,
    Other, Other, Other, Other,
    Other, Other, Other, Other,
    Other, Other, Other, Other,

    Consonant, Consonant, Other, Other,
    Other, Other, Other, Other,
    Other, Other, Other, Other,
    Other, Other, Other, Other,

    // Gurmukhi
    Invalid, Invalid, VowelMark, Invalid,
    Invalid, IndependentVowel, IndependentVowel, IndependentVowel,
    IndependentVowel, IndependentVowel, IndependentVowel, Invalid,
    Invalid, Invalid, Invalid, IndependentVowel,

    IndependentVowel, Invalid, Invalid, IndependentVowel,
    IndependentVowel, Consonant, Consonant, Consonant,
    Consonant, Consonant, Consonant, Consonant,
    Consonant, Consonant, Consonant, Consonant,

    Consonant, Consonant, Consonant, Consonant,
    Consonant, Consonant, Consonant, Consonant,
    Consonant, Invalid, Consonant, Consonant,
    Consonant, Consonant, Consonant, Consonant,

    Consonant, Invalid, Consonant, Consonant,
    Invalid, Consonant, Consonant, Invalid,
    Consonant, Consonant, UnknownForm, UnknownForm,
    Nukta, Other, Matra, Matra,

    Matra, Matra, Matra, Invalid,
    Invalid, Invalid, Invalid, Matra,
    Matra, Invalid, Invalid, Matra,
    Matra, Halant, UnknownForm, UnknownForm,

    Invalid, Invalid, Invalid, Invalid,
    Invalid, UnknownForm, UnknownForm, UnknownForm,
    Invalid, Consonant, Consonant, Consonant,
    Consonant, Invalid, Consonant, Invalid,

    Other, Other, Invalid, Invalid,
    Other, Other, Other, Other,
    Other, Other, Other, Other,
    Other, Other, Other, Other,

    StressMark, StressMark, Other, Other,
    Other, Other, Other, Other,
    Other, Other, Other, Other,
    Other, Other, Other, Other,

    // Gujarati
    Invalid, VowelMark, VowelMark, VowelMark,
    Invalid, IndependentVowel, IndependentVowel, IndependentVowel,
    IndependentVowel, IndependentVowel, IndependentVowel, IndependentVowel,
    Invalid, IndependentVowel, Invalid, IndependentVowel,

    IndependentVowel, IndependentVowel, Invalid, IndependentVowel,
    IndependentVowel, Consonant, Consonant, Consonant,
    Consonant, Consonant, Consonant, Consonant,
    Consonant, Consonant, Consonant, Consonant,

    Consonant, Consonant, Consonant, Consonant,
    Consonant, Consonant, Consonant, Consonant,
    Consonant, Invalid, Consonant, Consonant,
    Consonant, Consonant, Consonant, Consonant,

    Consonant, Invalid, Consonant, Consonant,
    Invalid, Consonant, Consonant, Consonant,
    Consonant, Consonant, UnknownForm, UnknownForm,
    Nukta, Other, Matra, Matra,

    Matra, Matra, Matra, Matra,
    Matra, Matra, Invalid, Matra,
    Matra, Matra, Invalid, Matra,
    Matra, Halant, UnknownForm, UnknownForm,

    Other, UnknownForm, UnknownForm, UnknownForm,
    UnknownForm, UnknownForm, UnknownForm, UnknownForm,
    UnknownForm, UnknownForm, UnknownForm, UnknownForm,
    UnknownForm, UnknownForm, UnknownForm, UnknownForm,

    Other, Other, Other, Other,
    Other, Other, Other, Other,
    Other, Other, Other, Other,
    Other, Other, Other, Other,

    Other, Other, Other, Other,
    Other, Other, Other, Other,
    Other, Other, Other, Other,
    Other, Other, Other, Other,

    // Oriya
    Invalid, VowelMark, VowelMark, VowelMark,
    Invalid, IndependentVowel, IndependentVowel, IndependentVowel,
    IndependentVowel, IndependentVowel, IndependentVowel, IndependentVowel,
    IndependentVowel, Invalid, Invalid, IndependentVowel,

    IndependentVowel, Invalid, Invalid, IndependentVowel,
    IndependentVowel, Consonant, Consonant, Consonant,
    Consonant, Consonant, Consonant, Consonant,
    Consonant, Consonant, Consonant, Consonant,

    Consonant, Consonant, Consonant, Consonant,
    Consonant, Consonant, Consonant, Consonant,
    Consonant, Invalid, Consonant, Consonant,
    Consonant, Consonant, Consonant, Consonant,

    Consonant, Invalid, Consonant, Consonant,
    Invalid, Invalid, Consonant, Consonant,
    Consonant, Consonant, UnknownForm, UnknownForm,
    Nukta, Other, Matra, Matra,

    Matra, Matra, Matra, Matra,
    Invalid, Invalid, Invalid, Matra,
    Matra, Invalid, Invalid, Matra,
    Matra, Halant, UnknownForm, UnknownForm,

    Other, Invalid, Invalid, Invalid,
    Invalid, UnknownForm, LengthMark, LengthMark,
    Invalid, Invalid, Invalid, Invalid,
    Consonant, Consonant, Invalid, Consonant,

    IndependentVowel, IndependentVowel, Invalid, Invalid,
    Invalid, Invalid, Other, Other,
    Other, Other, Other, Other,
    Other, Other, Other, Other,

    Other, Other, Other, Other,
    Other, Other, Other, Other,
    Other, Other, Other, Other,
    Other, Other, Other, Other,

    //Tamil
    Invalid, Invalid, VowelMark, Other,
    Invalid, IndependentVowel, IndependentVowel, IndependentVowel,
    IndependentVowel, IndependentVowel, IndependentVowel, Invalid,
    Invalid, Invalid, IndependentVowel, IndependentVowel,

    IndependentVowel, Invalid, IndependentVowel, IndependentVowel,
    IndependentVowel, Consonant, Invalid, Invalid,
    Invalid, Consonant, Consonant, Invalid,
    Consonant, Invalid, Consonant, Consonant,

    Invalid, Invalid, Invalid, Consonant,
    Consonant, Invalid, Invalid, Invalid,
    Consonant, Consonant, Consonant, Invalid,
    Invalid, Invalid, Consonant, Consonant,

    Consonant, Consonant, Consonant, Consonant,
    Consonant, Consonant, Invalid, Consonant,
    Consonant, Consonant, UnknownForm, UnknownForm,
    Invalid, Invalid, Matra, Matra,

    Matra, Matra, Matra, Invalid,
    Invalid, Invalid, Matra, Matra,
    Matra, Invalid, Matra, Matra,
    Matra, Halant, Invalid, Invalid,

    Invalid, Invalid, Invalid, Invalid,
    Invalid, Invalid, Invalid, LengthMark,
    Invalid, Invalid, Invalid, Invalid,
    Invalid, Invalid, Invalid, Invalid,

    Invalid, Invalid, Invalid, Invalid,
    Invalid, Invalid, Other, Other,
    Other, Other, Other, Other,
    Other, Other, Other, Other,

    Other, Other, Other, Other,
    Other, Other, Other, Other,
    Other, Other, Other, Other,
    Other, Other, Other, Other,

    // Telugu
    Invalid, VowelMark, VowelMark, VowelMark,
    Invalid, IndependentVowel, IndependentVowel, IndependentVowel,
    IndependentVowel, IndependentVowel, IndependentVowel, IndependentVowel,
    IndependentVowel, Invalid, IndependentVowel, IndependentVowel,

    IndependentVowel, Invalid, IndependentVowel, IndependentVowel,
    IndependentVowel, Consonant, Consonant, Consonant,
    Consonant, Consonant, Consonant, Consonant,
    Consonant, Consonant, Consonant, Consonant,

    Consonant, Consonant, Consonant, Consonant,
    Consonant, Consonant, Consonant, Consonant,
    Consonant, Invalid, Consonant, Consonant,
    Consonant, Consonant, Consonant, Consonant,

    Consonant, Consonant, Consonant, Consonant,
    Invalid, Consonant, Consonant, Consonant,
    Consonant, Consonant, UnknownForm, UnknownForm,
    Invalid, Invalid, Matra, Matra,

    Matra, Matra, Matra, Matra,
    Matra, Invalid, Matra, Matra,
    Matra, Invalid, Matra, Matra,
    Matra, Halant, Invalid, Invalid,

    Invalid, Invalid, Invalid, Invalid,
    Invalid, LengthMark, Matra, Invalid,
    Invalid, Invalid, Invalid, Invalid,
    Invalid, Invalid, Invalid, Invalid,

    IndependentVowel, IndependentVowel, Invalid, Invalid,
    Invalid, Invalid, Other, Other,
    Other, Other, Other, Other,
    Other, Other, Other, Other,

    Other, Other, Other, Other,
    Other, Other, Other, Other,
    Other, Other, Other, Other,
    Other, Other, Other, Other,

    // Kannada
    Invalid, Invalid, VowelMark, VowelMark,
    Invalid, IndependentVowel, IndependentVowel, IndependentVowel,
    IndependentVowel, IndependentVowel, IndependentVowel, IndependentVowel,
    IndependentVowel, Invalid, IndependentVowel, IndependentVowel,

    IndependentVowel, Invalid, IndependentVowel, IndependentVowel,
    IndependentVowel, Consonant, Consonant, Consonant,
    Consonant, Consonant, Consonant, Consonant,
    Consonant, Consonant, Consonant, Consonant,

    Consonant, Consonant, Consonant, Consonant,
    Consonant, Consonant, Consonant, Consonant,
    Consonant, Invalid, Consonant, Consonant,
    Consonant, Consonant, Consonant, Consonant,

    Consonant, Consonant, Consonant, Consonant,
    Invalid, Consonant, Consonant, Consonant,
    Consonant, Consonant, UnknownForm, UnknownForm,
    Invalid, Invalid, Matra, Matra,

    Matra, Matra, Matra, Matra,
    Matra, Invalid, Matra, Matra,
    Matra, Invalid, Matra, Matra,
    Matra, Halant, Invalid, Invalid,

    Invalid, Invalid, Invalid, Invalid,
    Invalid, LengthMark, LengthMark, Invalid,
    Invalid, Invalid, Invalid, Invalid,
    Invalid, Invalid, Consonant, Invalid,

    IndependentVowel, IndependentVowel, Invalid, Invalid,
    Invalid, Invalid, Other, Other,
    Other, Other, Other, Other,
    Other, Other, Other, Other,

    Other, Other, Other, Other,
    Other, Other, Other, Other,
    Other, Other, Other, Other,
    Other, Other, Other, Other,

    // Malayalam
    Invalid, Invalid, VowelMark, VowelMark,
    Invalid, IndependentVowel, IndependentVowel, IndependentVowel,
    IndependentVowel, IndependentVowel, IndependentVowel, IndependentVowel,
    IndependentVowel, Invalid, IndependentVowel, IndependentVowel,

    IndependentVowel, Invalid, IndependentVowel, IndependentVowel,
    IndependentVowel, Consonant, Consonant, Consonant,
    Consonant, Consonant, Consonant, Consonant,
    Consonant, Consonant, Consonant, Consonant,

    Consonant, Consonant, Consonant, Consonant,
    Consonant, Consonant, Consonant, Consonant,
    Consonant, Invalid, Consonant, Consonant,
    Consonant, Consonant, Consonant, Consonant,

    Consonant, Consonant, Consonant, Consonant,
    Consonant, Consonant, Consonant, Consonant,
    Consonant, Consonant, UnknownForm, UnknownForm,
    Invalid, Invalid, Matra, Matra,

    Matra, Matra, Matra, Matra,
    Invalid, Invalid, Matra, Matra,
    Matra, Invalid, Matra, Matra,
    Matra, Halant, Invalid, Invalid,

    Invalid, Invalid, Invalid, Invalid,
    Invalid, Invalid, Invalid, LengthMark,
    Invalid, Invalid, Invalid, Invalid,
    Invalid, Invalid, Invalid, Invalid,

    IndependentVowel, IndependentVowel, Invalid, Invalid,
    Invalid, Invalid, Other, Other,
    Other, Other, Other, Other,
    Other, Other, Other, Other,

    Other, Other, Other, Other,
    Other, Other, Other, Other,
    Other, Other, Other, Other,
    Other, Other, Other, Other,

    // Sinhala
    Invalid, Invalid, VowelMark, VowelMark,
    Invalid, IndependentVowel, IndependentVowel, IndependentVowel,
    IndependentVowel, IndependentVowel, IndependentVowel, IndependentVowel,
    IndependentVowel, IndependentVowel, IndependentVowel, IndependentVowel,

    IndependentVowel, IndependentVowel, IndependentVowel, IndependentVowel,
    IndependentVowel, IndependentVowel, IndependentVowel, Invalid,
    Invalid, Invalid, Consonant, Consonant,
    Consonant, Consonant, Consonant, Consonant,

    Consonant, Consonant, Consonant, Consonant,
    Consonant, Consonant, Consonant, Consonant,
    Consonant, Consonant, Consonant, Consonant,
    Consonant, Consonant, Consonant, Consonant,

    Consonant, Consonant, Invalid, Consonant,
    Consonant, Consonant, Consonant, Consonant,
    Consonant, Consonant, Consonant, Consonant,
    Invalid, Consonant, Invalid, Invalid,

    Consonant, Consonant, Consonant, Consonant,
    Consonant, Consonant, Consonant, Invalid,
    Invalid, Invalid, Halant, Invalid,
    Invalid, Invalid, Invalid, Matra,

    Matra, Matra, Matra, Matra,
    Matra, Invalid, Matra, Invalid,
    Matra, Matra, Matra, Matra,
    Matra, Matra, Matra, Matra,

    Invalid, Invalid, Invalid, Invalid,
    Invalid, Invalid, Invalid, Invalid,
    Invalid, Invalid, Invalid, Invalid,
    Invalid, Invalid, Invalid, Invalid,

    Invalid, Invalid, Matra, Matra,
    Other, Other, Other, Other,
    Other, Other, Other, Other,
    Other, Other, Other, Other,
};

enum Position {
    None,
    Pre,
    Above,
    Below,
    Post,
    Split,
    Base,
    Reph,
    Vattu,
    Inherit
};

static const unsigned char indicPosition[0xe00-0x900] = {
    // Devanagari
    None, Above, Above, Post,
    None, None, None, None,
    None, None, None, None,
    None, None, None, None,

    None, None, None, None,
    None, None, None, None,
    None, None, None, None,
    None, None, None, None,

    None, None, None, None,
    None, None, None, None,
    None, None, None, None,
    None, None, None, None,

    Below, None, None, None,
    None, None, None, None,
    None, None, None, None,
    None, None, Post, Pre,

    Post, Below, Below, Below,
    Below, Above, Above, Above,
    Above, Post, Post, Post,
    Post, None, None, None,

    None, Above, Below, Above,
    Above, None, None, None,
    None, None, None, None,
    None, None, None, None,

    None, None, Below, Below,
    None, None, None, None,
    None, None, None, None,
    None, None, None, None,

    None, None, None, None,
    None, None, None, None,
    None, None, None, None,
    None, None, None, None,

    // Bengali
    None, Above, Post, Post,
    None, None, None, None,
    None, None, None, None,
    None, None, None, None,

    None, None, None, None,
    None, None, None, None,
    None, None, None, None,
    None, None, None, None,

    None, None, None, None,
    None, None, None, None,
    None, None, None, None,
    Below, None, None, Post,

    Below, None, None, None,
    None, None, None, None,
    None, None, None, None,
    Below, None, Post, Pre,

    Post, Below, Below, Below,
    Below, None, None, Pre,
    Pre, None, None, Split,
    Split, Below, None, None,

    None, None, None, None,
    None, None, None, Post,
    None, None, None, None,
    None, None, None, None,

    None, None, Below, Below,
    None, None, None, None,
    None, None, None, None,
    None, None, None, None,

    None, None, None, None,
    None, None, None, None,
    None, None, None, None,
    None, None, None, None,

    // Gurmukhi
    None, None, Above, None,
    None, None, None, None,
    None, None, None, None,
    None, None, None, None,

    None, None, None, None,
    None, None, None, None,
    None, None, None, None,
    None, None, None, None,

    None, None, None, None,
    None, None, None, None,
    None, None, None, None,
    None, None, None, Post,

    Below, None, None, None,
    None, Below, None, None,
    None, Below, None, None,
    Below, None, Post, Pre,

    Post, Below, Below, None,
    None, None, None, Above,
    Above, None, None, Above,
    Above, None, None, None,

    None, None, None, None,
    None, None, None, None,
    None, None, None, None,
    None, None, None, None,

    None, None, None, None,
    None, None, None, None,
    None, None, None, None,
    None, None, None, None,

    Above, Above, None, None,
    None, None, None, None,
    None, None, None, None,
    None, None, None, None,

    // Gujarati
    None, Above, Above, Post,
    None, None, None, None,
    None, None, None, None,
    None, None, None, None,

    None, None, None, None,
    None, None, None, None,
    None, None, None, None,
    None, None, None, None,

    None, None, None, None,
    None, None, None, None,
    None, None, None, None,
    None, None, None, None,

    Below, None, None, None,
    None, None, None, None,
    None, None, None, None,
    None, None, Post, Pre,

    Post, Below, Below, Below,
    Below, Above, None, Above,
    Above, Post, None, Post,
    Post, None, None, None,

    None, None, None, None,
    None, None, None, None,
    None, None, None, None,
    None, None, None, None,

    None, None, None, None,
    None, None, None, None,
    None, None, None, None,
    None, None, None, None,

    None, None, None, None,
    None, None, None, None,
    None, None, None, None,
    None, None, None, None,

    // Oriya
    None, Above, Post, Post,
    None, None, None, None,
    None, None, None, None,
    None, None, None, None,

    None, None, None, None,
    None, None, None, None,
    None, None, None, None,
    None, None, None, None,

    None, None, None, None,
    Below, None, None, None,
    Below, None, None, None,
    Below, Below, Below, Post,

    Below, None, Below, Below,
    None, None, None, None,
    None, None, None, None,
    None, None, Post, Above,

    Post, Below, Below, Below,
    None, None, None, Pre,
    Split, None, None, Split,
    Split, None, None, None,

    None, None, None, None,
    None, None, Above, Post,
    None, None, None, None,
    None, None, None, None,

    None, None, None, None,
    None, None, None, None,
    None, None, None, None,
    None, None, None, None,

    None, None, None, None,
    None, None, None, None,
    None, None, None, None,
    None, None, None, None,

    // Tamil
    None, None, Above, None,
    None, None, None, None,
    None, None, None, None,
    None, None, None, None,

    None, None, None, None,
    None, None, None, None,
    None, None, None, None,
    None, None, None, None,

    None, None, None, None,
    None, None, None, None,
    None, None, None, None,
    None, None, None, None,

    None, None, None, None,
    None, None, None, None,
    None, None, None, None,
    None, None, Post, Post,

    Above, Below, Below, None,
    None, None, Pre, Pre,
    Pre, None, Split, Split,
    Split, Halant, None, None,

    None, None, None, None,
    None, None, None, Post,
    None, None, None, None,
    None, None, None, None,

    None, None, None, None,
    None, None, None, None,
    None, None, None, None,
    None, None, None, None,

    None, None, None, None,
    None, None, None, None,
    None, None, None, None,
    None, None, None, None,

    // Telugu
    None, Post, Post, Post,
    None, None, None, None,
    None, None, None, None,
    None, None, None, None,

    None, None, None, None,
    None, Below, Below, Below,
    Below, Below, Below, Below,
    Below, Below, Below, Below,

    Below, Below, Below, Below,
    Below, Below, Below, Below,
    Below, None, Below, Below,
    Below, Below, Below, Below,

    Below, None, Below, Below,
    None, Below, Below, Below,
    Below, Below, None, None,
    None, None, Post, Above,

    Above, Post, Post, Post,
    Post, None, Above, Above,
    Split, None, Post, Above,
    Above, Halant, None, None,

    None, None, None, None,
    None, Above, Below, None,
    None, None, None, None,
    None, None, None, None,

    None, None, None, None,
    None, None, None, None,
    None, None, None, None,
    None, None, None, None,

    None, None, None, None,
    None, None, None, None,
    None, None, None, None,
    None, None, None, None,

    // Kannada
    None, None, Post, Post,
    None, None, None, None,
    None, None, None, None,
    None, None, None, None,

    None, None, None, None,
    None, Below, Below, Below,
    Below, Below, Below, Below,
    Below, Below, Below, Below,

    Below, Below, Below, Below,
    Below, Below, Below, Below,
    Below, Below, Below, Below,
    Below, Below, Below, Below,

    Below, None, Below, Below,
    None, Below, Below, Below,
    Below, Below, None, None,
    None, None, Post, Above,

    Split, Post, Post, Post,
    Post, None, Above, Split,
    Split, None, Split, Split,
    Above, Halant, None, None,

    None, None, None, None,
    None, Post, Post, None,
    None, None, None, None,
    None, None, Below, None,

    None, None, None, None,
    None, None, None, None,
    None, None, None, None,
    None, None, None, None,

    None, None, None, None,
    None, None, None, None,
    None, None, None, None,
    None, None, None, None,

    // Malayalam
    None, None, Post, Post,
    None, None, None, None,
    None, None, None, None,
    None, None, None, None,

    None, None, None, None,
    None, None, None, None,
    None, None, None, None,
    None, None, None, None,

    None, None, None, None,
    None, None, None, None,
    None, None, None, None,
    None, None, None, Post,

    Post, None, Below, None,
    None, Post, None, None,
    None, None, None, None,
    None, None, Post, Post,

    Post, Post, Post, Post,
    None, None, Pre, Pre,
    Pre, None, Split, Split,
    Split, Halant, None, None,

    None, None, None, None,
    None, None, None, Post,
    None, None, None, None,
    None, None, None, None,

    None, None, None, None,
    None, None, None, None,
    None, None, None, None,
    None, None, None, None,

    None, None, None, None,
    None, None, None, None,
    None, None, None, None,
    None, None, None, None,

    // Sinhala
    None, None, Post, Post,
    None, None, None, None,
    None, None, None, None,
    None, None, None, None,

    None, None, None, None,
    None, None, None, None,
    None, None, None, None,
    None, None, None, None,

    None, None, None, None,
    None, None, None, None,
    None, None, None, None,
    None, None, None, None,

    None, None, None, None,
    None, None, None, None,
    None, None, None, None,
    None, None, None, None,

    None, None, None, None,
    None, None, None, None,
    None, None, None, None,
    None, None, None, Post,

    Post, Post, Above, Above,
    Below, None, Below, None,
    Post, Pre, Split, Pre,
    Split, Split, Split, Post,

    None, None, None, None,
    None, None, None, None,
    None, None, None, None,
    None, None, None, None,

    None, None, Post, Post,
    None, None, None, None,
    None, None, None, None,
    None, None, None, None
};

static inline Form form(unsigned short uc) {
    if (uc < 0x900 || uc > 0xdff) {
        if (uc == 0x25cc)
            return Consonant;
        if (uc == 0x200c || uc == 0x200d)
            return Control;
        return Other;
    }
    return (Form)indicForms[uc-0x900];
}

static inline Position indic_position(unsigned short uc) {
    if (uc < 0x900 || uc > 0xdff)
        return None;
    return (Position) indicPosition[uc-0x900];
}


enum IndicScriptProperties {
    HasReph = 0x01,
    HasSplit = 0x02
};

const uint8_t scriptProperties[10] = {
    // Devanagari,
    HasReph,
    // Bengali,
    HasReph|HasSplit,
    // Gurmukhi,
    0,
    // Gujarati,
    HasReph,
    // Oriya,
    HasReph|HasSplit,
    // Tamil,
    HasSplit,
    // Telugu,
    HasSplit,
    // Kannada,
    HasSplit|HasReph,
    // Malayalam,
    HasSplit,
    // Sinhala,
    HasSplit
};

struct IndicOrdering {
    Form form;
    Position position;
};

static const IndicOrdering devanagari_order [] = {
    { Consonant, Below },
    { Matra, Below },
    { VowelMark, Below },
    { StressMark, Below },
    { Matra, Above },
    { Matra, Post },
    { Consonant, Reph },
    { VowelMark, Above },
    { StressMark, Above },
    { VowelMark, Post },
    { (Form)0, None }
};

static const IndicOrdering bengali_order [] = {
    { Consonant, Below },
    { Matra, Below },
    { Matra, Above },
    { Consonant, Reph },
    { VowelMark, Above },
    { Consonant, Post },
    { Matra, Post },
    { VowelMark, Post },
    { (Form)0, None }
};

static const IndicOrdering gurmukhi_order [] = {
    { Consonant, Below },
    { Matra, Below },
    { Matra, Above },
    { Consonant, Post },
    { Matra, Post },
    { VowelMark, Above },
    { (Form)0, None }
};

static const IndicOrdering tamil_order [] = {
    { Matra, Above },
    { Matra, Post },
    { VowelMark, Post },
    { (Form)0, None }
};

static const IndicOrdering telugu_order [] = {
    { Matra, Above },
    { Matra, Below },
    { Matra, Post },
    { Consonant, Below },
    { Consonant, Post },
    { VowelMark, Post },
    { (Form)0, None }
};

static const IndicOrdering kannada_order [] = {
    { Matra, Above },
    { Matra, Post },
    { Consonant, Below },
    { Consonant, Post },
    { LengthMark, Post },
    { Consonant, Reph },
    { VowelMark, Post },
    { (Form)0, None }
};

static const IndicOrdering malayalam_order [] = {
    { Consonant, Below },
    { Matra, Below },
    { Consonant, Reph },
    { Consonant, Post },
    { Matra, Post },
    { VowelMark, Post },
    { (Form)0, None }
};

static const IndicOrdering sinhala_order [] = {
    { Matra, Below },
    { Matra, Above },
    { Matra, Post },
    { VowelMark, Post },
    { (Form)0, None }
};

static const IndicOrdering * const indic_order[] = {
    devanagari_order, // Devanagari
    bengali_order, // Bengali
    gurmukhi_order, // Gurmukhi
    devanagari_order, // Gujarati
    bengali_order, // Oriya
    tamil_order, // Tamil
    telugu_order, // Telugu
    kannada_order, // Kannada
    malayalam_order, // Malayalam
    sinhala_order // Sinhala
};



// vowel matras that have to be split into two parts.
static const unsigned short split_matras[]  = {
    //  matra, split1, split2

    // bengalis
    0x9cb, 0x9c7, 0x9be,
    0x9cc, 0x9c7, 0x9d7,
    // oriya
    0xb48, 0xb47, 0xb56,
    0xb4b, 0xb47, 0xb3e,
    0xb4c, 0xb47, 0xb57,
    // tamil
    0xbca, 0xbc6, 0xbbe,
    0xbcb, 0xbc7, 0xbbe,
    0xbcc, 0xbc6, 0xbd7,
    // telugu
    0xc48, 0xc46, 0xc56,
    // kannada
    0xcc0, 0xcbf, 0xcd5,
    0xcc7, 0xcc6, 0xcd5,
    0xcc8, 0xcc6, 0xcd6,
    0xcca, 0xcc6, 0xcc2,
    0xccb, 0xcca, 0xcd5,
    // malayalam
    0xd4a, 0xd46, 0xd3e,
    0xd4b, 0xd47, 0xd3e,
    0xd4c, 0xd46, 0xd57,
    // sinhala
    0xdda, 0xdd9, 0xdca,
    0xddc, 0xdd9, 0xdcf,
    0xddd, 0xddc, 0xdca,
    0xdde, 0xdd9, 0xddf,
    0xffff
};

static inline void splitMatra(unsigned short *reordered, int matra, int &len, int &base)
{
    unsigned short matra_uc = reordered[matra];
    //qDebug("matra=%d, reordered[matra]=%x", matra, reordered[matra]);

    const unsigned short *split = split_matras;
    while (split[0] < matra_uc)
        split += 3;

    assert(*split == matra_uc);
    ++split;

    if (indic_position(*split) == Pre) {
        reordered[matra] = split[1];
        memmove(reordered + 1, reordered, len*sizeof(unsigned short));
        reordered[0] = split[0];
        base++;
    } else {
        memmove(reordered + matra + 1, reordered + matra, (len-matra)*sizeof(unsigned short));
        reordered[matra] = split[0];
        reordered[matra+1] = split[1];
    }
    len++;
}

#if 0
#ifndef QT_NO_OPENTYPE
static const QOpenType::Features indic_features[] = {
    { FT_MAKE_TAG('c', 'c', 'm', 'p'), CcmpProperty },
    { FT_MAKE_TAG('i', 'n', 'i', 't'), InitProperty },
    { FT_MAKE_TAG('n', 'u', 'k', 't'), NuktaProperty },
    { FT_MAKE_TAG('a', 'k', 'h', 'n'), AkhantProperty },
    { FT_MAKE_TAG('r', 'p', 'h', 'f'), RephProperty },
    { FT_MAKE_TAG('b', 'l', 'w', 'f'), BelowFormProperty },
    { FT_MAKE_TAG('h', 'a', 'l', 'f'), HalfFormProperty },
    { FT_MAKE_TAG('p', 's', 't', 'f'), PostFormProperty },
    { FT_MAKE_TAG('v', 'a', 't', 'u'), VattuProperty },
    { FT_MAKE_TAG('p', 'r', 'e', 's'), PreSubstProperty },
    { FT_MAKE_TAG('b', 'l', 'w', 's'), BelowSubstProperty },
    { FT_MAKE_TAG('a', 'b', 'v', 's'), AboveSubstProperty },
    { FT_MAKE_TAG('p', 's', 't', 's'), PostSubstProperty },
    { FT_MAKE_TAG('h', 'a', 'l', 'n'), HalantProperty },
    { 0, 0 }
};
#endif
#endif

// #define INDIC_DEBUG
#ifdef INDIC_DEBUG
#define IDEBUG qDebug
#else
#define IDEBUG if(0) printf
#endif

#if 0
#ifdef INDIC_DEBUG
static QString propertiesToString(int properties)
{
    QString res;
    properties = ~properties;
    if (properties & CcmpProperty)
        res += "Ccmp ";
    if (properties & InitProperty)
        res += "Init ";
    if (properties & NuktaProperty)
        res += "Nukta ";
    if (properties & AkhantProperty)
        res += "Akhant ";
    if (properties & RephProperty)
        res += "Reph ";
    if (properties & PreFormProperty)
        res += "PreForm ";
    if (properties & BelowFormProperty)
        res += "BelowForm ";
    if (properties & AboveFormProperty)
        res += "AboveForm ";
    if (properties & HalfFormProperty)
        res += "HalfForm ";
    if (properties & PostFormProperty)
        res += "PostForm ";
    if (properties & VattuProperty)
        res += "Vattu ";
    if (properties & PreSubstProperty)
        res += "PreSubst ";
    if (properties & BelowSubstProperty)
        res += "BelowSubst ";
    if (properties & AboveSubstProperty)
        res += "AboveSubst ";
    if (properties & PostSubstProperty)
        res += "PostSubst ";
    if (properties & HalantProperty)
        res += "Halant ";
    if (properties & CligProperty)
        res += "Clig ";
    return res;
}
#endif

#if defined(Q_WS_X11) || defined(Q_WS_QWS)
static bool indic_shape_syllable(QOpenType *openType, QShaperItem *item, bool invalid)
{
    Q_UNUSED(openType)
    int script = item->script;
    Q_ASSERT(script >= QUnicodeTables::Devanagari && script <= QUnicodeTables::Sinhala);
    const unsigned short script_base = 0x0900 + 0x80*(script-QUnicodeTables::Devanagari);
    const unsigned short ra = script_base + 0x30;
    const unsigned short halant = script_base + 0x4d;
    const unsigned short nukta = script_base + 0x3c;

    int len = item->length;
    IDEBUG(">>>>> indic shape: from=%d, len=%d invalid=%d", item->from, item->length, invalid);

    if (item->num_glyphs < len+4) {
        item->num_glyphs = len+4;
        return false;
    }

    QVarLengthArray<unsigned short> reordered(len+4);
    QVarLengthArray<unsigned char> position(len+4);

    unsigned char properties = scriptProperties[script-QUnicodeTables::Devanagari];

    if (invalid) {
        *reordered.data() = 0x25cc;
        memcpy(reordered.data()+1, item->string->unicode() + item->from, len*sizeof(QChar));
        len++;
    } else {
        memcpy(reordered.data(), item->string->unicode() + item->from, len*sizeof(QChar));
    }
    if (reordered[len-1] == 0x200c) // zero width non joiner
        len--;

    int i;
    int base = 0;
    int reph = -1;

#ifdef INDIC_DEBUG
    IDEBUG("original:");
    for (i = 0; i < len; i++) {
        IDEBUG("    %d: %4x", i, reordered[i]);
    }
#endif

    if (len != 1) {
        unsigned short *uc = reordered.data();
        bool beginsWithRa = false;

        // Rule 1: find base consonant
        //
        // The shaping engine finds the base consonant of the
        // syllable, using the following algorithm: starting from the
        // end of the syllable, move backwards until a consonant is
        // found that does not have a below-base or post-base form
        // (post-base forms have to follow below-base forms), or
        // arrive at the first consonant. The consonant stopped at
        // will be the base.
        //
        //  * If the syllable starts with Ra + H (in a script that has
        //    'Reph'), Ra is excluded from candidates for base
        //    consonants.
        //
        // * In Kannada and Telugu, the base consonant cannot be
        //   farther than 3 consonants from the end of the syllable.
        // #### replace the HasReph property by testing if the feature exists in the font!
        if (form(*uc) == Consonant || (script == QUnicodeTables::Bengali && form(*uc) == IndependentVowel)) {
            beginsWithRa = (properties & HasReph) && ((len > 2) && *uc == ra && *(uc+1) == halant);

            if (beginsWithRa && form(*(uc+2)) == Control)
                beginsWithRa = false;

            base = (beginsWithRa ? 2 : 0);
            IDEBUG("    length = %d, beginsWithRa = %d, base=%d", len, beginsWithRa, base);

            int lastConsonant = 0;
            int matra = -1;
            // we remember:
            // * the last consonant since we need it for rule 2
            // * the matras position for rule 3 and 4

            // figure out possible base glyphs
            memset(position.data(), 0, len);
            if (script == QUnicodeTables::Devanagari || script == QUnicodeTables::Gujarati) {
                bool vattu = false;
                for (i = base; i < len; ++i) {
                    position[i] = form(uc[i]);
                    if (position[i] == Consonant) {
                        lastConsonant = i;
                        vattu = (!vattu && uc[i] == ra);
                        if (vattu) {
                            IDEBUG("excluding vattu glyph at %d from base candidates", i);
                            position[i] = Vattu;
                        }
                    } else if (position[i] == Matra) {
                        matra = i;
                    }
                }
            } else {
                for (i = base; i < len; ++i) {
                    position[i] = form(uc[i]);
                    if (position[i] == Consonant)
                        lastConsonant = i;
                    else if (matra < 0 && position[i] == Matra)
                        matra = i;
                }
            }
            int skipped = 0;
            Position pos = Post;
            for (i = len-1; i > base; i--) {
                if (position[i] != Consonant && (position[i] != Control || script == QUnicodeTables::Kannada))
                    continue;

                Position charPosition = indic_position(uc[i]);
                if (pos == Post && charPosition == Post) {
                    pos = Post;
                } else if ((pos == Post || pos == Below) && charPosition == Below) {
                    if (script == QUnicodeTables::Devanagari || script == QUnicodeTables::Gujarati)
                        base = i;
                    pos = Below;
                } else {
                    base = i;
                    break;
                }
                if (skipped == 2 && (script == QUnicodeTables::Kannada || script == QUnicodeTables::Telugu)) {
                    base = i;
                    break;
                }
                ++skipped;
            }

            IDEBUG("    base consonant at %d skipped=%d, lastConsonant=%d", base, skipped, lastConsonant);

            // Rule 2:
            //
            // If the base consonant is not the last one, Uniscribe
            // moves the halant from the base consonant to the last
            // one.
            if (lastConsonant > base) {
                int halantPos = 0;
                if (uc[base+1] == halant)
                    halantPos = base + 1;
                else if (uc[base+1] == nukta && uc[base+2] == halant)
                    halantPos = base + 2;
                if (halantPos > 0) {
                    IDEBUG("    moving halant from %d to %d!", base+1, lastConsonant);
                    for (i = halantPos; i < lastConsonant; i++)
                        uc[i] = uc[i+1];
                    uc[lastConsonant] = halant;
                }
            }

            // Rule 3:
            //
            // If the syllable starts with Ra + H, Uniscribe moves
            // this combination so that it follows either:

            // * the post-base 'matra' (if any) or the base consonant
            //   (in scripts that show similarity to Devanagari, i.e.,
            //   Devanagari, Gujarati, Bengali)
            // * the base consonant (other scripts)
            // * the end of the syllable (Kannada)

            Position matra_position = None;
            if (matra > 0)
                matra_position = indic_position(uc[matra]);
            IDEBUG("    matra at %d with form %d, base=%d", matra, matra_position, base);

            if (beginsWithRa && base != 0) {
                int toPos = base+1;
                if (toPos < len && uc[toPos] == nukta)
                    toPos++;
                if (toPos < len && uc[toPos] == halant)
                    toPos++;
                if (toPos < len && uc[toPos] == 0x200d)
                    toPos++;
                if (toPos < len-1 && uc[toPos] == ra && uc[toPos+1] == halant)
                    toPos += 2;
                if (script == QUnicodeTables::Devanagari || script == QUnicodeTables::Gujarati || script == QUnicodeTables::Bengali) {
                    if (matra_position == Post || matra_position == Split) {
                        toPos = matra+1;
                        matra -= 2;
                    }
                } else if (script == QUnicodeTables::Kannada) {
                    toPos = len;
                    matra -= 2;
                }

                IDEBUG("moving leading ra+halant to position %d", toPos);
                for (i = 2; i < toPos; i++)
                    uc[i-2] = uc[i];
                uc[toPos-2] = ra;
                uc[toPos-1] = halant;
                base -= 2;
                if (properties & HasReph)
                    reph = toPos-2;
            }

            // Rule 4:

            // Uniscribe splits two- or three-part matras into their
            // parts. This splitting is a character-to-character
            // operation).
            //
            //      Uniscribe describes some moving operations for these
            //      matras here. For shaping however all pre matras need
            //      to be at the beginning of the syllable, so we just move
            //      them there now.
            if (matra_position == Split) {
                splitMatra(uc, matra, len, base);
                // Handle three-part matras (0xccb in Kannada)
                matra_position = indic_position(uc[matra]);
                if (matra_position == Split)
                    splitMatra(uc, matra, len, base);
            } else if (matra_position == Pre) {
                unsigned short m = uc[matra];
                while (matra--)
                    uc[matra+1] = uc[matra];
                uc[0] = m;
                base++;
            }
        }

        // Rule 5:
        //
        // Uniscribe classifies consonants and 'matra' parts as
        // pre-base, above-base (Reph), below-base or post-base. This
        // classification exists on the character code level and is
        // language-dependent, not font-dependent.
        for (i = 0; i < base; ++i)
            position[i] = Pre;
        position[base] = Base;
        for (i = base+1; i < len; ++i) {
            position[i] = indic_position(uc[i]);
            // #### replace by adjusting table
            if (uc[i] == nukta || uc[i] == halant)
                position[i] = Inherit;
        }
        if (reph > 0) {
            // recalculate reph, it might have changed.
            for (i = base+1; i < len; ++i)
                if (uc[i] == ra)
                    reph = i;
            position[reph] = Reph;
            position[reph+1] = Inherit;
        }

        // all reordering happens now to the chars after the base
        int fixed = base+1;
        if (fixed < len && uc[fixed] == nukta)
            fixed++;
        if (fixed < len && uc[fixed] == halant)
            fixed++;
        if (fixed < len && uc[fixed] == 0x200d)
            fixed++;

#ifdef INDIC_DEBUG
        for (i = fixed; i < len; ++i)
            IDEBUG("position[%d] = %d, form=%d uc=%x", i, position[i], form(uc[i]), uc[i]);
#endif
        // we continuosly position the matras and vowel marks and increase the fixed
        // until we reached the end.
        const IndicOrdering *finalOrder = indic_order[script-QUnicodeTables::Devanagari];

        IDEBUG("    reordering pass:");
        IDEBUG("        base=%d fixed=%d", base, fixed);
        int toMove = 0;
        while (finalOrder[toMove].form && fixed < len-1) {
            IDEBUG("        fixed = %d, toMove=%d, moving form %d with pos %d", fixed, toMove, finalOrder[toMove].form, finalOrder[toMove].position);
            for (i = fixed; i < len; i++) {
                IDEBUG() << "           i=" << i << "uc=" << hex << uc[i] << "form=" << form(uc[i])
                         << "position=" << position[i];
                if (form(uc[i]) == finalOrder[toMove].form &&
                     position[i] == finalOrder[toMove].position) {
                    // need to move this glyph
                    int to = fixed;
                    if (i < len-1 && position[i+1] == Inherit) {
                        IDEBUG("         moving two chars from %d to %d", i, to);
                        unsigned short ch = uc[i];
                        unsigned short ch2 = uc[i+1];
                        unsigned char pos = position[i];
                        for (int j = i+1; j > to+1; j--) {
                            uc[j] = uc[j-2];
                            position[j] = position[j-2];
                        }
                        uc[to] = ch;
                        uc[to+1] = ch2;
                        position[to] = pos;
                        position[to+1] = pos;
                        fixed += 2;
                    } else {
                        IDEBUG("         moving one char from %d to %d", i, to);
                        unsigned short ch = uc[i];
                        unsigned char pos = position[i];
                        for (int j = i; j > to; j--) {
                            uc[j] = uc[j-1];
                            position[j] = position[j-1];
                        }
                        uc[to] = ch;
                        position[to] = pos;
                        fixed++;
                    }
                }
            }
            toMove++;
        }

    }

    if (reph > 0) {
        // recalculate reph, it might have changed.
        for (i = base+1; i < len; ++i)
            if (reordered[i] == ra)
                reph = i;
    }

#ifndef QT_NO_OPENTYPE
    const int availableGlyphs = item->num_glyphs;
#endif
    if (!item->font->stringToCMap((const QChar *)reordered.data(), len, item->glyphs, &item->num_glyphs, QFlag(item->flags)))
        return false;


    IDEBUG("  base=%d, reph=%d", base, reph);
    IDEBUG("reordered:");
    for (i = 0; i < len; i++) {
        item->glyphs[i].attributes.mark = false;
        item->glyphs[i].attributes.clusterStart = false;
        item->glyphs[i].attributes.justification = 0;
        item->glyphs[i].attributes.zeroWidth = false;
        IDEBUG("    %d: %4x", i, reordered[i]);
    }

    // now we have the syllable in the right order, and can start running it through open type.

    bool control = false;
    for (i = 0; i < len; ++i)
        control |= (form(reordered[i]) == Control);

#ifndef QT_NO_OPENTYPE
    if (openType) {

        // we need to keep track of where the base glyph is for some
        // scripts and use the cluster feature for this.  This
        // also means we have to correct the logCluster output from
        // the open type engine manually afterwards.  for indic this
        // is rather simple, as all chars just point to the first
        // glyph in the syllable.
        QVarLengthArray<unsigned short> clusters(len);
        QVarLengthArray<unsigned int> properties(len);

        for (i = 0; i < len; ++i)
            clusters[i] = i;

        // features we should always apply
        for (i = 0; i < len; ++i)
            properties[i] = ~(CcmpProperty
                              | NuktaProperty
                              | VattuProperty
                              | PreSubstProperty
                              | BelowSubstProperty
                              | AboveSubstProperty
                              | HalantProperty
                              | PositioningProperties);

        // Ccmp always applies
        // Init
        if (item->from == 0
            || !(item->string->unicode()[item->from-1].isLetter() ||  item->string->unicode()[item->from-1].isMark()))
            properties[0] &= ~InitProperty;

        // Nukta always applies
        // Akhant
        for (i = 0; i <= base; ++i)
            properties[i] &= ~AkhantProperty;
        // Reph
        if (reph >= 0) {
            properties[reph] &= ~RephProperty;
            properties[reph+1] &= ~RephProperty;
        }
        // BelowForm
        for (i = base+1; i < len; ++i)
            properties[i] &= ~BelowFormProperty;

        if (script == QUnicodeTables::Devanagari || script == QUnicodeTables::Gujarati) {
            // vattu glyphs need this aswell
            bool vattu = false;
            for (i = base-2; i > 1; --i) {
                if (form(reordered[i]) == Consonant) {
                    vattu = (!vattu && reordered[i] == ra);
                    if (vattu) {
                        IDEBUG("forming vattu ligature at %d", i);
                        properties[i] &= ~BelowFormProperty;
                        properties[i+1] &= ~BelowFormProperty;
                    }
                }
            }
        }
        // HalfFormProperty
        for (i = 0; i < base; ++i)
            properties[i] &= ~HalfFormProperty;
        if (control) {
            for (i = 2; i < len; ++i) {
                if (reordered[i] == 0x200d /* ZWJ */) {
                    properties[i-1] &= ~HalfFormProperty;
                    properties[i-2] &= ~HalfFormProperty;
                } else if (reordered[i] == 0x200c /* ZWNJ */) {
                    properties[i-1] &= ~HalfFormProperty;
                    properties[i-2] &= ~HalfFormProperty;
                }
            }
        }
        // PostFormProperty
        for (i = base+1; i < len; ++i)
            properties[i] &= ~PostFormProperty;
        // vattu always applies
        // pres always applies
        // blws always applies
        // abvs always applies

        // psts
        // ### this looks slightly different from before, but I believe it's correct
        if (reordered[len-1] != halant || base != len-2)
            properties[base] &= ~PostSubstProperty;
        for (i = base+1; i < len; ++i)
            properties[i] &= ~PostSubstProperty;

        // halant always applies

#ifdef INDIC_DEBUG
        {
            IDEBUG("OT properties:");
            for (int i = 0; i < len; ++i)
                qDebug("    i: %s", ::propertiesToString(properties[i]).toLatin1().data());
        }
#endif

        // initialize
        item->log_clusters = clusters.data();
        openType->shape(item, properties.data());

        int newLen = openType->len();
        HB_GlyphItem otl_glyphs = openType->glyphs();

        // move the left matra back to its correct position in malayalam and tamil
        if ((script == QUnicodeTables::Malayalam || script == QUnicodeTables::Tamil) && (form(reordered[0]) == Matra)) {
//             qDebug("reordering matra, len=%d", newLen);
            // need to find the base in the shaped string and move the matra there
            int basePos = 0;
            while (basePos < newLen && (int)otl_glyphs[basePos].cluster <= base)
                basePos++;
            --basePos;
            if (basePos < newLen && basePos > 1) {
//                 qDebug("moving prebase matra to position %d in syllable newlen=%d", basePos, newLen);
                HB_GlyphItemRec m = otl_glyphs[0];
                --basePos;
                for (i = 0; i < basePos; ++i)
                    otl_glyphs[i] = otl_glyphs[i+1];
                otl_glyphs[basePos] = m;
            }
        }

        if (!openType->positionAndAdd(item, availableGlyphs, false))
            return false;

        if (control) {
            IDEBUG("found a control char in the syllable");
            int i = 0, j = 0;
            while (i < item->num_glyphs) {
                if (form(reordered[otl_glyphs[i].cluster]) == Control) {
                    ++i;
                    if (i >= item->num_glyphs)
                        break;
                }
                item->glyphs[j] = item->glyphs[i];
                ++i;
                ++j;
            }
            item->num_glyphs = j;
        }

    }
#endif // QT_NO_OPENTYPE
    item->glyphs[0].attributes.clusterStart = true;

    IDEBUG("<<<<<<");
    return true;
}
#endif // Q_WS_X11 || Q_WS_QWS

#endif

/* syllables are of the form:

   (Consonant Nukta? Halant)* Consonant Matra? VowelMark? StressMark?
   (Consonant Nukta? Halant)* Consonant Halant
   IndependentVowel VowelMark? StressMark?

   We return syllable boundaries on invalid combinations aswell
*/
static int indic_nextSyllableBoundary(HB_Script script, const HB_UChar16 *s, int start, int end, bool *invalid)
{
    *invalid = false;
    IDEBUG("indic_nextSyllableBoundary: start=%d, end=%d", start, end);
    const HB_UChar16 *uc = s+start;

    int pos = 0;
    Form state = form(uc[pos]);
    IDEBUG("state[%d]=%d (uc=%4x)", pos, state, uc[pos]);
    pos++;

    if (state != Consonant && state != IndependentVowel) {
        if (state != Other)
            *invalid = true;
        goto finish;
    }

    while (pos < end - start) {
        Form newState = form(uc[pos]);
        IDEBUG("state[%d]=%d (uc=%4x)", pos, newState, uc[pos]);
        switch(newState) {
        case Control:
            newState = state;
 	    if (state == Halant && uc[pos] == 0x200d /* ZWJ */)
  		break;
            // the control character should be the last char in the item
            ++pos;
            goto finish;
        case Consonant:
	    if (state == Halant && (script != HB_Script_Sinhala || uc[pos-1] == 0x200d /* ZWJ */))
                break;
            goto finish;
        case Halant:
            if (state == Nukta || state == Consonant)
                break;
            // Bengali has a special exception allowing the combination Vowel_A/E + Halant + Ya
            if (script == HB_Script_Bengali && pos == 1 &&
                 (uc[0] == 0x0985 || uc[0] == 0x098f))
                break;
            goto finish;
        case Nukta:
            if (state == Consonant)
                break;
            goto finish;
        case StressMark:
            if (state == VowelMark)
                break;
            // fall through
        case VowelMark:
            if (state == Matra || state == IndependentVowel)
                break;
            // fall through
        case Matra:
            if (state == Consonant || state == Nukta)
                break;
            // ### not sure if this is correct. If it is, does it apply only to Bengali or should
            // it work for all Indic languages?
            // the combination Independent_A + Vowel Sign AA is allowed.
            if (script == HB_Script_Bengali && uc[pos] == 0x9be && uc[pos-1] == 0x985)
                break;
            if (script == HB_Script_Tamil && state == Matra) {
                if (uc[pos-1] == 0x0bc6 &&
                     (uc[pos] == 0xbbe || uc[pos] == 0xbd7))
                    break;
                if (uc[pos-1] == 0x0bc7 && uc[pos] == 0xbbe)
                    break;
            }
            goto finish;

        case LengthMark:
        case IndependentVowel:
        case Invalid:
        case Other:
            goto finish;
        }
        state = newState;
        pos++;
    }
 finish:
    return pos+start;
}

#if 0

static bool indic_shape(QShaperItem *item)
{
    Q_ASSERT(item->script >= QUnicodeTables::Devanagari && item->script <= QUnicodeTables::Sinhala);

#ifndef QT_NO_OPENTYPE
    QOpenType *openType = item->font->openType();
    if (openType)
        openType->selectScript(item, item->script, indic_features);
#else
    QOpenType *openType = 0;
#endif
    unsigned short *logClusters = item->log_clusters;

    QShaperItem syllable = *item;
    int first_glyph = 0;

    int sstart = item->from;
    int end = sstart + item->length;
    IDEBUG("indic_shape: from %d length %d", item->from, item->length);
    while (sstart < end) {
        bool invalid;
        int send = indic_nextSyllableBoundary(item->script, *item->string, sstart, end, &invalid);
        IDEBUG("syllable from %d, length %d, invalid=%s", sstart, send-sstart,
               invalid ? "true" : "false");
        syllable.from = sstart;
        syllable.length = send-sstart;
        syllable.glyphs = item->glyphs + first_glyph;
        syllable.num_glyphs = item->num_glyphs - first_glyph;
        if (!indic_shape_syllable(openType, &syllable, invalid)) {
            IDEBUG("syllable shaping failed, syllable requests %d glyphs", syllable.num_glyphs);
            item->num_glyphs += syllable.num_glyphs;
            return false;
        }
        // fix logcluster array
        IDEBUG("syllable:");
        for (int i = first_glyph; i < first_glyph + syllable.num_glyphs; ++i)
            IDEBUG("        %d -> glyph %x", i, item->glyphs[i].glyph);
        IDEBUG("    logclusters:");
        for (int i = sstart; i < send; ++i) {
            IDEBUG("        %d -> glyph %d", i, first_glyph);
            logClusters[i-item->from] = first_glyph;
        }
        sstart = send;
        first_glyph += syllable.num_glyphs;
    }
    item->num_glyphs = first_glyph;
    return true;
}
#endif

void HB_IndicAttributes(HB_Script script, const HB_UChar16 *text, uint32_t from, uint32_t len, HB_CharAttributes *attributes)
{
    int end = from + len;
    const HB_UChar16 *uc = text + from;
    attributes += from;
    int i = 0;
    while (i < len) {
        bool invalid;
        int boundary = indic_nextSyllableBoundary(script, text, from+i, end, &invalid) - from;
         attributes[i].charStop = true;

        if (boundary > len-1) boundary = len;
        i++;
        while (i < boundary) {
            attributes[i].charStop = false;
            ++uc;
            ++i;
        }
        assert(i == boundary);
    }


}


