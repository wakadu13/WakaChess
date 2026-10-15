// search.hpp - Intelligence artificielle (ex ia.py)
#pragma once

#include "board.hpp"

#include <atomic>
#include <cstdint>
#include <functional>

constexpr int SCORE_MAT = 100000;
constexpr int PROFONDEUR_MAX = 64;

struct ResultatRecherche {
    bool trouve = false;
    Coup coup;
    int score = 0;
    int profondeur = 0;
    uint64_t noeuds = 0;
};

struct LimitesRecherche {
    int profondeur = PROFONDEUR_MAX;
    int64_t tempsDurMs = -1;  // arrêt immédiat au-delà (-1 = illimité)
    int64_t tempsDouxMs = -1; // pas de nouvelle itération au-delà (-1 = illimité)
};

// Appelé à la fin de chaque itération : profondeur, score, nœuds, temps (ms), meilleur coup
using RappelInfo = std::function<void(int, int, uint64_t, int64_t, const Coup&)>;

int evaluerPlateau(const Position& pos); // du point de vue du camp au trait

// Approfondissement itératif ; s'arrête sur les limites ou quand `arret` passe à vrai
ResultatRecherche rechercher(Position& pos, const LimitesRecherche& limites,
                             const std::atomic<bool>& arret, const RappelInfo& info = nullptr);

// Profondeur fixe (mode console)
ResultatRecherche trouverMeilleurCoup(Position& pos, int profondeur);
