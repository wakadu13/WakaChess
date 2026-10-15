// search.hpp - Intelligence artificielle (ex ia.py)
#pragma once

#include "board.hpp"

constexpr int SCORE_MAT = 100000;

struct ResultatRecherche {
    bool trouve = false;
    Coup coup;
    int score = 0;
    uint64_t noeuds = 0;
};

int evaluerPlateau(const Position& pos); // du point de vue du camp au trait
ResultatRecherche trouverMeilleurCoup(Position& pos, int profondeur);
