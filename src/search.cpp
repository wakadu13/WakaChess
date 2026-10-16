// search.cpp - Évaluation, Negamax alpha-bêta et quiescence (ex ia.py)
#include "search.hpp"

#include <algorithm>
#include <chrono>
#include <cstdlib>

namespace {

constexpr int INFINI = 1000000;

// Indexées par type de pièce : VIDE, PION, CAVALIER, FOU, TOUR, DAME, ROI
constexpr int VALEURS[7] = {0, 100, 320, 330, 500, 900, 20000};

// Tables écrites du point de vue des Blancs, rangée 8 en haut (ligne 0)
constexpr int PST_CAVALIER[8][8] = {
    {-50,-40,-30,-30,-30,-30,-40,-50},
    {-40,-20,  0,  0,  0,  0,-20,-40},
    {-30,  0, 10, 15, 15, 10,  0,-30},
    {-30,  5, 15, 20, 20, 15,  5,-30},
    {-30,  0, 15, 20, 20, 15,  0,-30},
    {-30,  5, 10, 15, 15, 10,  5,-30},
    {-40,-20,  0,  5,  5,  0,-20,-40},
    {-50,-40,-30,-30,-30,-30,-40,-50},
};

constexpr int PST_PION[8][8] = {
    { 0,  0,  0,  0,  0,  0,  0,  0},
    {50, 50, 50, 50, 50, 50, 50, 50},
    {10, 10, 20, 30, 30, 20, 10, 10},
    { 5,  5, 10, 25, 25, 10,  5,  5},
    { 0,  0,  0, 20, 20,  0,  0,  0},
    { 5, -5,-10,  0,  0,-10, -5,  5},
    { 5, 10, 10,-20,-20, 10, 10,  5},
    { 0,  0,  0,  0,  0,  0,  0,  0},
};

// Tri des coups pour l'élagage : captures (et promotions) d'abord, tri stable par insertion
void trierCoups(const Position& pos, Coup* coups, int n) {
    int score[MAX_COUPS];
    for (int i = 0; i < n; ++i) {
        const Coup& m = coups[i];
        int s = 0;
        if (m.flags & F_EN_PASSANT) s = VALEURS[PION] * 10;
        else if (m.flags & F_CAPTURE) s = VALEURS[std::abs(pos.plateau[m.vers])] * 10;
        if (m.promo) s += VALEURS[m.promo] * 10;
        score[i] = s;
    }
    for (int i = 1; i < n; ++i) {
        const Coup m = coups[i];
        const int s = score[i];
        int j = i - 1;
        while (j >= 0 && score[j] < s) { coups[j + 1] = coups[j]; score[j + 1] = score[j]; --j; }
        coups[j + 1] = m; score[j + 1] = s;
    }
}

using Horloge = std::chrono::steady_clock;

struct Recherche {
    Position& pos;
    const std::atomic<bool>& arretExterne;
    int64_t tempsDurMs;
    Horloge::time_point debut = Horloge::now();
    uint64_t noeuds = 0;
    bool arret = false;

    int64_t ecouleMs() const {
        return std::chrono::duration_cast<std::chrono::milliseconds>(Horloge::now() - debut).count();
    }

    // Vérifie régulièrement (tous les 2048 nœuds) le temps et la demande d'arrêt
    bool doitArreter() {
        if (arret) return true;
        if ((noeuds & 2047) == 0 &&
            (arretExterne.load(std::memory_order_relaxed) || (tempsDurMs >= 0 && ecouleMs() >= tempsDurMs)))
            arret = true;
        return arret;
    }

    // Recherche de repos : on prolonge sur les captures pour éviter l'effet d'horizon
    int quiescence(int alpha, int beta) {
        ++noeuds;
        if (doitArreter()) return 0;
        const int standPat = evaluerPlateau(pos);
        if (standPat >= beta) return beta;
        if (alpha < standPat) alpha = standPat;

        Coup coups[MAX_COUPS];
        const int n = pos.genererPseudo(coups, true);
        trierCoups(pos, coups, n);
        const int camp = pos.trait;
        for (int i = 0; i < n; ++i) {
            Annulation a;
            pos.jouer(coups[i], a);
            if (pos.enEchec(camp)) { pos.annuler(coups[i], a); continue; } // coup illégal
            const int score = -quiescence(-beta, -alpha);
            pos.annuler(coups[i], a);
            if (arret) return 0;
            if (score >= beta) return beta;
            if (score > alpha) alpha = score;
        }
        return alpha;
    }

    int negamax(int profondeur, int alpha, int beta, int ply) {
        if (profondeur == 0) return quiescence(alpha, beta);
        ++noeuds;
        if (doitArreter()) return 0;

        Coup coups[MAX_COUPS];
        const int n = pos.genererLegaux(coups);
        if (n == 0) // Mat (le plus rapide est préféré) ou pat
            return pos.enEchec(pos.trait) ? -SCORE_MAT + ply : 0;
        trierCoups(pos, coups, n);

        int meilleur = -INFINI;
        for (int i = 0; i < n; ++i) {
            Annulation a;
            pos.jouer(coups[i], a);
            const int score = -negamax(profondeur - 1, -beta, -alpha, ply + 1);
            pos.annuler(coups[i], a);
            if (arret) return 0;
            if (score > meilleur) meilleur = score;
            if (score > alpha) alpha = score;
            if (alpha >= beta) break;
        }
        return meilleur;
    }
};

} // namespace

int evaluerPlateau(const Position& pos) {
    int score = 0;
    for (int c = 0; c < 64; ++c) {
        const int8_t p = pos.plateau[c];
        if (p == VIDE) continue;
        const int x = colonne(c), y = rangee(c);
        if (p > 0) {
            score += VALEURS[p];
            if (p == CAVALIER) score += PST_CAVALIER[7 - y][x];
            else if (p == PION) score += PST_PION[7 - y][x];
        } else {
            score -= VALEURS[-p];
            if (p == -CAVALIER) score -= PST_CAVALIER[y][x]; // miroir pour les Noirs
            else if (p == -PION) score -= PST_PION[y][x];
        }
    }
    // Bonus pour le roque : roi en g1/c1 (g8/c8)
    if (pos.plateau[6] == ROI || pos.plateau[2] == ROI) score += 40;
    if (pos.plateau[62] == -ROI || pos.plateau[58] == -ROI) score -= 40;

    return pos.trait == BLANC ? score : -score;
}

ResultatRecherche rechercher(Position& pos, const LimitesRecherche& limites,
                             const std::atomic<bool>& arret, const RappelInfo& info) {
    ResultatRecherche res;
    Coup coups[MAX_COUPS];
    const int n = pos.genererLegaux(coups);
    if (n == 0) return res;
    trierCoups(pos, coups, n);
    res.trouve = true;
    res.coup = coups[0];

    Recherche r{pos, arret, limites.tempsDurMs};
    const int profondeurMax = std::min(limites.profondeur, PROFONDEUR_MAX);

    for (int profondeur = 1; profondeur <= profondeurMax; ++profondeur) {
        int alpha = -INFINI;
        const int beta = INFINI;
        int meilleurScore = -INFINI;
        int meilleur = -1;

        for (int i = 0; i < n; ++i) {
            Annulation a;
            pos.jouer(coups[i], a);
            const int score = -r.negamax(profondeur - 1, -beta, -alpha, 1);
            pos.annuler(coups[i], a);
            if (r.arret) break;
            if (score > meilleurScore) { meilleurScore = score; meilleur = i; }
            if (score > alpha) alpha = score;
        }
        // Itération interrompue : on garde son meilleur coup seulement s'il a été entièrement évalué
        if (meilleur < 0) break;

        // Le meilleur coup est exploré en premier à l'itération suivante
        const Coup m = coups[meilleur];
        for (int i = meilleur; i > 0; --i) coups[i] = coups[i - 1];
        coups[0] = m;

        res.coup = m;
        res.score = meilleurScore;
        res.profondeur = profondeur;
        res.noeuds = r.noeuds;
        if (r.arret) break;
        if (info) info(profondeur, meilleurScore, r.noeuds, r.ecouleMs(), m);

        if (limites.tempsDouxMs >= 0 && r.ecouleMs() >= limites.tempsDouxMs) break;
        if (std::abs(meilleurScore) >= SCORE_MAT - PROFONDEUR_MAX && profondeur >= SCORE_MAT - std::abs(meilleurScore))
            break; // mat trouvé et prouvé à cette profondeur
    }
    res.noeuds = r.noeuds;
    return res;
}

ResultatRecherche trouverMeilleurCoup(Position& pos, int profondeur) {
    const std::atomic<bool> jamais{false};
    LimitesRecherche limites;
    limites.profondeur = profondeur;
    return rechercher(pos, limites, jamais);
}
