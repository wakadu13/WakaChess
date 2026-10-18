// search.hpp - Intelligence artificielle : évaluation et recherche
#pragma once

#include "board.hpp"

#include <atomic>
#include <cstddef>
#include <cstdint>
#include <functional>

constexpr int SCORE_MAT = 32000;
constexpr int SEUIL_MAT = SCORE_MAT - 256; // au-delà : score de mat
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
    uint64_t noeuds = 0;      // 0 = illimité
};

// Appelé à la fin de chaque itération : profondeur, score, nœuds, temps (ms), variante principale
using RappelInfo = std::function<void(int, int, uint64_t, int64_t, const Coup*, int)>;

int evaluerPlateau(const Position& pos); // du point de vue du camp au trait

// Approfondissement itératif ; s'arrête sur les limites ou quand `arret` passe à vrai
ResultatRecherche rechercher(Position& pos, const LimitesRecherche& limites,
                             const std::atomic<bool>& arret, const RappelInfo& info = nullptr);

// Profondeur fixe
ResultatRecherche trouverMeilleurCoup(Position& pos, int profondeur);

// Niveaux de difficulté 1 (débutant) à 5 (maximum), communs à la console et à l'interface graphique
constexpr int NIVEAU_MAX = 5;
LimitesRecherche limitesPourNiveau(int niveau);
const char* nomNiveau(int niveau);

// Table de transposition
void redimensionnerTT(std::size_t megaOctets);
void viderTT();
