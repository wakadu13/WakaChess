// search.cpp - Évaluation et recherche (Negamax alpha-bêta, PVS, table de transposition, élagages)
#include "search.hpp"

#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstdlib>
#include <cstring>
#include <memory>
#include <vector>

namespace {

constexpr int INFINI = 32500;
constexpr int MAX_PLY = 128;

// ---------------------------------------------------------------------------------------------
// Évaluation : tables PeSTO (matériel + position), interpolées entre milieu et fin de partie
// Tables écrites du point de vue des Blancs, rangée 8 en haut (index 0 = a8)
// ---------------------------------------------------------------------------------------------

// Indexées par type de pièce : VIDE, PION, CAVALIER, FOU, TOUR, DAME, ROI
constexpr int VALEUR_MG[7] = {0, 82, 337, 365, 477, 1025, 0};
constexpr int VALEUR_EG[7] = {0, 94, 281, 297, 512, 936, 0};
constexpr int PHASE[7] = {0, 0, 1, 1, 2, 4, 0};

constexpr int PST_MG[7][64] = {
    {},
    { // Pion
      0,   0,   0,   0,   0,   0,  0,   0,
     98, 134,  61,  95,  68, 126, 34, -11,
     -6,   7,  26,  31,  65,  56, 25, -20,
    -14,  13,   6,  21,  23,  12, 17, -23,
    -27,  -2,  -5,  12,  17,   6, 10, -25,
    -26,  -4,  -4, -10,   3,   3, 33, -12,
    -35,  -1, -20, -23, -15,  24, 38, -22,
      0,   0,   0,   0,   0,   0,  0,   0},
    { // Cavalier
    -167, -89, -34, -49,  61, -97, -15, -107,
     -73, -41,  72,  36,  23,  62,   7,  -17,
     -47,  60,  37,  65,  84, 129,  73,   44,
      -9,  17,  19,  53,  37,  69,  18,   22,
     -13,   4,  16,  13,  28,  19,  21,   -8,
     -23,  -9,  12,  10,  19,  17,  25,  -16,
     -29, -53, -12,  -3,  -1,  18, -14,  -19,
    -105, -21, -58, -33, -17, -28, -19,  -23},
    { // Fou
    -29,   4, -82, -37, -25, -42,   7,  -8,
    -26,  16, -18, -13,  30,  59,  18, -47,
    -16,  37,  43,  40,  35,  50,  37,  -2,
     -4,   5,  19,  50,  37,  37,   7,  -2,
     -6,  13,  13,  26,  34,  12,  10,   4,
      0,  15,  15,  15,  14,  27,  18,  10,
      4,  15,  16,   0,   7,  21,  33,   1,
    -33,  -3, -14, -21, -13, -12, -39, -21},
    { // Tour
     32,  42,  32,  51, 63,  9,  31,  43,
     27,  32,  58,  62, 80, 67,  26,  44,
     -5,  19,  26,  36, 17, 45,  61,  16,
    -24, -11,   7,  26, 24, 35,  -8, -20,
    -36, -26, -12,  -1,  9, -7,   6, -23,
    -45, -25, -16, -17,  3,  0,  -5, -33,
    -44, -16, -20,  -9, -1, 11,  -6, -71,
    -19, -13,   1,  17, 16,  7, -37, -26},
    { // Dame
    -28,   0,  29,  12,  59,  44,  43,  45,
    -24, -39,  -5,   1, -16,  57,  28,  54,
    -13, -17,   7,   8,  29,  56,  47,  57,
    -27, -27, -16, -16,  -1,  17,  -2,   1,
     -9, -26,  -9, -10,  -2,  -4,   3,  -3,
    -14,   2, -11,  -2,  -5,   2,  14,   5,
    -35,  -8,  11,   2,   8,  15,  -3,   1,
     -1, -18,  -9,  10, -15, -25, -31, -50},
    { // Roi
    -65,  23,  16, -15, -56, -34,   2,  13,
     29,  -1, -20,  -7,  -8,  -4, -38, -29,
     -9,  24,   2, -16, -20,   6,  22, -22,
    -17, -20, -12, -27, -30, -25, -14, -36,
    -49,  -1, -27, -39, -46, -44, -33, -51,
    -14, -14, -22, -46, -44, -30, -15, -27,
      1,   7,  -8, -64, -43, -16,   9,   8,
    -15,  36,  12, -54,   8, -28,  24,  14},
};

constexpr int PST_EG[7][64] = {
    {},
    { // Pion
      0,   0,   0,   0,   0,   0,   0,   0,
    178, 173, 158, 134, 147, 132, 165, 187,
     94, 100,  85,  67,  56,  53,  82,  84,
     32,  24,  13,   5,  -2,   4,  17,  17,
     13,   9,  -3,  -7,  -7,  -8,   3,  -1,
      4,   7,  -6,   1,   0,  -5,  -1,  -8,
     13,   8,   8,  10,  13,   0,   2,  -7,
      0,   0,   0,   0,   0,   0,   0,   0},
    { // Cavalier
    -58, -38, -13, -28, -31, -27, -63, -99,
    -25,  -8, -25,  -2,  -9, -25, -24, -52,
    -24, -20,  10,   9,  -1,  -9, -19, -41,
    -17,   3,  22,  22,  22,  11,   8, -18,
    -18,  -6,  16,  25,  16,  17,   4, -18,
    -23,  -3,  -1,  15,  10,  -3, -20, -22,
    -42, -20, -10,  -5,  -2, -20, -23, -44,
    -29, -51, -23, -15, -22, -18, -50, -64},
    { // Fou
    -14, -21, -11,  -8, -7,  -9, -17, -24,
     -8,  -4,   7, -12, -3, -13,  -4, -14,
      2,  -8,   0,  -1, -2,   6,   0,   4,
     -3,   9,  12,   9, 14,  10,   3,   2,
     -6,   3,  13,  19,  7,  10,  -3,  -9,
    -12,  -3,   8,  10, 13,   3,  -7, -15,
    -14, -18,  -7,  -1,  4,  -9, -15, -27,
    -23,  -9, -23,  -5, -9, -16,  -5, -17},
    { // Tour
     13, 10, 18, 15, 12,  12,   8,   5,
     11, 13, 13, 11, -3,   3,   8,   3,
      7,  7,  7,  5,  4,  -3,  -5,  -3,
      4,  3, 13,  1,  2,   1,  -1,   2,
      3,  5,  8,  4, -5,  -6,  -8, -11,
     -4,  0, -5, -1, -7, -12,  -8, -16,
     -6, -6,  0,  2, -9,  -9, -11,  -3,
     -9,  2,  3, -1, -5, -13,   4, -20},
    { // Dame
     -9,  22,  22,  27,  27,  19,  10,  20,
    -17,  20,  32,  41,  58,  25,  30,   0,
    -20,   6,   9,  49,  47,  35,  19,   9,
      3,  22,  24,  45,  57,  40,  57,  36,
    -18,  28,  19,  47,  31,  34,  39,  23,
    -16, -27,  15,   6,   9,  17,  10,   5,
    -22, -23, -30, -16, -16, -23, -36, -32,
    -33, -28, -22, -43,  -5, -32, -20, -41},
    { // Roi
    -74, -35, -18, -18, -11,  15,   4, -17,
    -12,  17,  14,  17,  17,  38,  23,  11,
     10,  17,  23,  15,  20,  45,  44,  13,
     -8,  22,  24,  27,  26,  33,  26,   3,
    -18,  -4,  21,  24,  27,  23,   9, -11,
    -19,  -3,  11,  21,  23,  16,   7,  -9,
    -27, -11,   4,  13,  14,   4,  -5, -17,
    -53, -34, -21, -11, -28, -14, -24, -43},
};

// Tables combinées (matériel + position, signe inclus) indexées par [pièce + 6][case]
struct TablesEval {
    int mg[13][64], eg[13][64];
    TablesEval() {
        for (int type = 1; type <= 6; ++type) {
            for (int c = 0; c < 64; ++c) {
                const int x = colonne(c), y = rangee(c);
                const int iBlanc = (7 - y) * 8 + x; // la table a la rangée 8 en haut
                const int iNoir = y * 8 + x;        // miroir pour les Noirs
                mg[6 + type][c] = VALEUR_MG[type] + PST_MG[type][iBlanc];
                eg[6 + type][c] = VALEUR_EG[type] + PST_EG[type][iBlanc];
                mg[6 - type][c] = -(VALEUR_MG[type] + PST_MG[type][iNoir]);
                eg[6 - type][c] = -(VALEUR_EG[type] + PST_EG[type][iNoir]);
            }
        }
        for (int c = 0; c < 64; ++c) mg[6][c] = eg[6][c] = 0;
    }
};
const TablesEval TE;

// Valeurs simples pour le tri des captures (MVV-LVA) et l'élagage delta
constexpr int VALEUR_TRI[7] = {0, 100, 320, 330, 500, 900, 20000};

// ---------------------------------------------------------------------------------------------
// Table de transposition
// ---------------------------------------------------------------------------------------------

enum : uint8_t { TT_VIDE = 0, TT_EXACT = 1, TT_BAS = 2, TT_HAUT = 3 }; // BAS : score >= ; HAUT : score <=

struct EntreeTT {
    uint64_t cle;
    Coup coup;
    int16_t score;
    int8_t profondeur;
    uint8_t type;
};
static_assert(sizeof(EntreeTT) == 16, "entrée de 16 octets");

std::vector<EntreeTT> tableTT(std::size_t(1) << 20); // 16 Mo par défaut
std::size_t masqueTT = tableTT.size() - 1;

inline EntreeTT& entreeTT(uint64_t cle) { return tableTT[cle & masqueTT]; }

// Les scores de mat sont stockés relativement à la position, pas à la racine
inline int versTT(int score, int ply) {
    return score >= SEUIL_MAT ? score + ply : score <= -SEUIL_MAT ? score - ply : score;
}
inline int depuisTT(int score, int ply) {
    return score >= SEUIL_MAT ? score - ply : score <= -SEUIL_MAT ? score + ply : score;
}

// ---------------------------------------------------------------------------------------------
// Recherche
// ---------------------------------------------------------------------------------------------

struct TableReductions {
    int r[64][64];
    TableReductions() {
        for (int d = 0; d < 64; ++d)
            for (int n = 0; n < 64; ++n)
                r[d][n] = (d == 0 || n == 0) ? 0 : static_cast<int>(0.75 + std::log(d) * std::log(n) / 2.25);
    }
};
const TableReductions LMR;

inline bool estTactique(const Coup& m) { return (m.flags & F_CAPTURE) || m.promo; }

using Horloge = std::chrono::steady_clock;

struct Recherche {
    Position& pos;
    const std::atomic<bool>& arretExterne;
    int64_t tempsDurMs;
    uint64_t limiteNoeuds;
    Horloge::time_point debut = Horloge::now();
    uint64_t noeuds = 0;
    bool arret = false;

    Coup tueurs[MAX_PLY][2] = {};  // coups calmes ayant provoqué une coupure (killer moves)
    int historique[2][64][64] = {}; // succès passés des coups calmes
    Coup pv[MAX_PLY][MAX_PLY] = {};
    int pvLongueur[MAX_PLY] = {};
    bool racineAmelioree = false;

    Recherche(Position& p, const std::atomic<bool>& a, int64_t t, uint64_t n)
        : pos(p), arretExterne(a), tempsDurMs(t), limiteNoeuds(n) {}

    int64_t ecouleMs() const {
        return std::chrono::duration_cast<std::chrono::milliseconds>(Horloge::now() - debut).count();
    }

    bool doitArreter() {
        if (arret) return true;
        if (limiteNoeuds && noeuds >= limiteNoeuds) arret = true;
        else if ((noeuds & 2047) == 0 &&
                 (arretExterne.load(std::memory_order_relaxed) || (tempsDurMs >= 0 && ecouleMs() >= tempsDurMs)))
            arret = true;
        return arret;
    }

    // Donne une note à chaque coup pour les explorer du plus prometteur au moins prometteur
    void noterCoups(const Coup* coups, int* notes, int n, const Coup& coupTT, int ply) const {
        for (int i = 0; i < n; ++i) {
            const Coup& m = coups[i];
            if (m == coupTT) notes[i] = 1 << 30;
            else if (estTactique(m)) {
                const int victime = (m.flags & F_EN_PASSANT) ? PION : std::abs(pos.plateau[m.vers]);
                notes[i] = (1 << 24) + VALEUR_TRI[victime] * 8 - std::abs(pos.plateau[m.de]) +
                           (m.promo == DAME ? VALEUR_TRI[DAME] * 8 : 0);
            } else if (m == tueurs[ply][0]) notes[i] = (1 << 22) + 1;
            else if (m == tueurs[ply][1]) notes[i] = 1 << 22;
            else notes[i] = historique[pos.trait][m.de][m.vers];
        }
    }

    // Sélection paresseuse : amène le meilleur coup restant en position i
    static void choisirSuivant(Coup* coups, int* notes, int n, int i) {
        int meilleur = i;
        for (int j = i + 1; j < n; ++j)
            if (notes[j] > notes[meilleur]) meilleur = j;
        std::swap(coups[i], coups[meilleur]);
        std::swap(notes[i], notes[meilleur]);
    }

    void majHistorique(int camp, const Coup& m, int bonus) {
        int& h = historique[camp][m.de][m.vers];
        h += bonus - h * std::abs(bonus) / 16384; // reste borné dans [-16384, 16384]
    }

    // Recherche de repos : on prolonge sur les captures pour éviter l'effet d'horizon
    int quiescence(int alpha, int beta, int ply) {
        ++noeuds;
        if (doitArreter()) return 0;
        const int standPat = evaluerPlateau(pos);
        if (ply >= MAX_PLY - 1) return standPat;

        const bool pvNoeud = beta - alpha > 1;
        const EntreeTT& e = entreeTT(pos.cle);
        if (!pvNoeud && e.cle == pos.cle) {
            const int s = depuisTT(e.score, ply);
            if (e.type == TT_EXACT || (e.type == TT_BAS && s >= beta) || (e.type == TT_HAUT && s <= alpha))
                return s;
        }

        if (standPat >= beta) return standPat;
        if (standPat > alpha) alpha = standPat;

        Coup coups[MAX_COUPS];
        int notes[MAX_COUPS];
        const int n = pos.genererPseudo(coups, true);
        noterCoups(coups, notes, n, Coup{}, ply);
        const int camp = pos.trait;
        int meilleur = standPat;
        for (int i = 0; i < n; ++i) {
            choisirSuivant(coups, notes, n, i);
            const Coup& m = coups[i];
            // Élagage delta : même en gagnant la pièce, on ne remonterait pas jusqu'à alpha
            const int victime = (m.flags & F_EN_PASSANT) ? PION : std::abs(pos.plateau[m.vers]);
            if (!m.promo && standPat + VALEUR_TRI[victime] + 200 < alpha) continue;

            Annulation a;
            pos.jouer(m, a);
            if (pos.enEchec(camp)) { pos.annuler(m, a); continue; } // coup illégal
            const int score = -quiescence(-beta, -alpha, ply + 1);
            pos.annuler(m, a);
            if (arret) return 0;
            if (score > meilleur) {
                meilleur = score;
                if (score > alpha) alpha = score;
                if (score >= beta) break;
            }
        }
        return meilleur;
    }

    int negamax(int alpha, int beta, int profondeur, int ply, bool nulAutorise) {
        const bool pvNoeud = beta - alpha > 1;
        const bool racine = ply == 0;
        pvLongueur[ply] = ply;

        // Nulle par répétition ou par la règle des 50 coups
        if (!racine && (pos.compteur50 >= 100 || pos.repetitions() > 0)) return 0;

        const bool enEchec = pos.enEchec(pos.trait);
        if (enEchec) ++profondeur; // extension d'échec
        if (profondeur <= 0) return quiescence(alpha, beta, ply);

        ++noeuds;
        if (doitArreter()) return 0;
        if (ply >= MAX_PLY - 1) return evaluerPlateau(pos);

        // Table de transposition
        Coup coupTT{};
        {
            const EntreeTT& e = entreeTT(pos.cle);
            if (e.cle == pos.cle) {
                coupTT = e.coup;
                if (!pvNoeud && e.profondeur >= profondeur) {
                    const int s = depuisTT(e.score, ply);
                    if (e.type == TT_EXACT || (e.type == TT_BAS && s >= beta) || (e.type == TT_HAUT && s <= alpha))
                        return s;
                }
            }
        }

        const int evalStatique = enEchec ? -INFINI : evaluerPlateau(pos);

        if (!pvNoeud && !enEchec) {
            // Élagage statique : la position est déjà tellement bonne qu'une marge ne suffit pas à la ruiner
            if (profondeur <= 6 && std::abs(beta) < SEUIL_MAT && evalStatique - 80 * profondeur >= beta)
                return evalStatique;

            // Élagage du coup nul : même en passant son tour, on dépasse beta
            if (nulAutorise && profondeur >= 3 && evalStatique >= beta && pos.aDesPiecesLourdes(pos.trait)) {
                const int R = 3 + profondeur / 4 + std::min(3, (evalStatique - beta) / 200);
                Annulation a;
                pos.jouerNul(a);
                const int s = -negamax(-beta, -beta + 1, profondeur - 1 - R, ply + 1, false);
                pos.annulerNul(a);
                if (arret) return 0;
                if (s >= beta) return s >= SEUIL_MAT ? beta : s;
            }
        }

        Coup coups[MAX_COUPS];
        int notes[MAX_COUPS];
        const int n = pos.genererPseudo(coups);
        noterCoups(coups, notes, n, coupTT, ply);

        Coup calmesEssayes[MAX_COUPS];
        int nbCalmes = 0;
        const int camp = pos.trait;
        const int alphaDepart = alpha;
        int meilleur = -INFINI;
        Coup meilleurCoup{};
        int legaux = 0;

        for (int i = 0; i < n; ++i) {
            choisirSuivant(coups, notes, n, i);
            const Coup m = coups[i];
            const bool calme = !estTactique(m);

            // Élagage des coups tardifs : à faible profondeur, les derniers coups calmes sont ignorés
            if (!racine && !pvNoeud && !enEchec && calme && meilleur > -SEUIL_MAT && profondeur <= 4 &&
                legaux >= 4 + profondeur * profondeur)
                continue;

            Annulation a;
            pos.jouer(m, a);
            if (pos.enEchec(camp)) { pos.annuler(m, a); continue; } // coup illégal
            ++legaux;
            const bool donneEchec = pos.enEchec(pos.trait);

            int score;
            if (legaux == 1) {
                score = -negamax(-beta, -alpha, profondeur - 1, ply + 1, true);
            } else {
                // Réduction des coups tardifs (LMR) : recherche moins profonde, refaite si le coup surprend
                int r = 0;
                if (profondeur >= 3 && calme && !enEchec && !donneEchec) {
                    r = LMR.r[std::min(profondeur, 63)][std::min(legaux, 63)];
                    if (pvNoeud) --r;
                    if (m == tueurs[ply][0] || m == tueurs[ply][1]) --r;
                    r = std::clamp(r, 0, profondeur - 2);
                }
                // Recherche à fenêtre nulle (PVS) : on vérifie seulement que le coup ne fait pas mieux
                score = -negamax(-alpha - 1, -alpha, profondeur - 1 - r, ply + 1, true);
                if (score > alpha && r > 0)
                    score = -negamax(-alpha - 1, -alpha, profondeur - 1, ply + 1, true);
                if (score > alpha && score < beta)
                    score = -negamax(-beta, -alpha, profondeur - 1, ply + 1, true);
            }
            pos.annuler(m, a);
            if (arret) return 0;

            if (score > meilleur) {
                meilleur = score;
                meilleurCoup = m;
                if (score > alpha) {
                    alpha = score;
                    // Variante principale
                    pv[ply][ply] = m;
                    for (int j = ply + 1; j < pvLongueur[ply + 1]; ++j) pv[ply][j] = pv[ply + 1][j];
                    pvLongueur[ply] = std::max(pvLongueur[ply + 1], ply + 1);
                    if (racine) racineAmelioree = true;

                    if (score >= beta) {
                        if (calme) {
                            if (!(m == tueurs[ply][0])) { tueurs[ply][1] = tueurs[ply][0]; tueurs[ply][0] = m; }
                            const int bonus = std::min(profondeur * profondeur, 1200);
                            majHistorique(camp, m, bonus);
                            for (int j = 0; j < nbCalmes; ++j) majHistorique(camp, calmesEssayes[j], -bonus);
                        }
                        break;
                    }
                }
            }
            if (calme) calmesEssayes[nbCalmes++] = m;
        }

        if (legaux == 0) return enEchec ? -SCORE_MAT + ply : 0; // mat ou pat

        const uint8_t type = meilleur >= beta ? TT_BAS : meilleur > alphaDepart ? TT_EXACT : TT_HAUT;
        EntreeTT& e = entreeTT(pos.cle);
        if (e.cle != pos.cle || profondeur + 2 >= e.profondeur || type == TT_EXACT) {
            e.cle = pos.cle;
            e.coup = meilleurCoup;
            e.score = static_cast<int16_t>(versTT(meilleur, ply));
            e.profondeur = static_cast<int8_t>(std::min(profondeur, 127));
            e.type = type;
        }
        return meilleur;
    }
};

} // namespace

int evaluerPlateau(const Position& pos) {
    int mg = 0, eg = 0, phase = 0;
    for (int c = 0; c < 64; ++c) {
        const int8_t p = pos.plateau[c];
        if (p == VIDE) continue;
        mg += TE.mg[p + 6][c];
        eg += TE.eg[p + 6][c];
        phase += PHASE[std::abs(p)];
    }
    phase = std::min(phase, 24);
    const int score = (mg * phase + eg * (24 - phase)) / 24;
    return pos.trait == BLANC ? score : -score;
}

ResultatRecherche rechercher(Position& pos, const LimitesRecherche& limites,
                             const std::atomic<bool>& arret, const RappelInfo& info) {
    ResultatRecherche res;
    Coup legaux[MAX_COUPS];
    if (pos.genererLegaux(legaux) == 0) return res;
    res.trouve = true;
    res.coup = legaux[0];

    auto r = std::make_unique<Recherche>(pos, arret, limites.tempsDurMs, limites.noeuds);
    const int profondeurMax = std::min(limites.profondeur, PROFONDEUR_MAX);
    int score = 0;

    for (int profondeur = 1; profondeur <= profondeurMax; ++profondeur) {
        r->racineAmelioree = false;
        // Fenêtre d'aspiration : on cherche d'abord autour du score précédent
        int delta = 25;
        int alpha = -INFINI, beta = INFINI;
        if (profondeur >= 5) { alpha = std::max(score - delta, -INFINI); beta = std::min(score + delta, INFINI); }
        int s;
        while (true) {
            s = r->negamax(alpha, beta, profondeur, 0, false);
            if (r->arret) break;
            if (s <= alpha) { beta = (alpha + beta) / 2; alpha = std::max(s - delta, -INFINI); }
            else if (s >= beta) { beta = std::min(s + delta, INFINI); }
            else break;
            delta += delta / 2;
        }

        if (r->arret) {
            // Itération interrompue : on garde le coup qui a déjà amélioré le score de la racine
            if (r->racineAmelioree && r->pvLongueur[0] > 0) res.coup = r->pv[0][0];
            break;
        }

        score = s;
        res.coup = r->pv[0][0];
        res.score = s;
        res.profondeur = profondeur;
        if (info) info(profondeur, s, r->noeuds, r->ecouleMs(), r->pv[0], r->pvLongueur[0]);

        if (limites.tempsDouxMs >= 0 && r->ecouleMs() >= limites.tempsDouxMs) break;
        if (std::abs(s) >= SEUIL_MAT && profondeur >= SCORE_MAT - std::abs(s)) break; // mat prouvé
    }
    res.noeuds = r->noeuds;
    return res;
}

ResultatRecherche trouverMeilleurCoup(Position& pos, int profondeur) {
    const std::atomic<bool> jamais{false};
    LimitesRecherche limites;
    limites.profondeur = profondeur;
    return rechercher(pos, limites, jamais);
}

LimitesRecherche limitesPourNiveau(int niveau) {
    LimitesRecherche l;
    switch (std::clamp(niveau, 1, NIVEAU_MAX)) {
        case 1: l.profondeur = 1; break;
        case 2: l.profondeur = 3; break;
        case 3: l.tempsDurMs = l.tempsDouxMs = 200; break;
        case 4: l.tempsDurMs = l.tempsDouxMs = 1000; break;
        default: l.tempsDurMs = l.tempsDouxMs = 3000; break;
    }
    return l;
}

const char* nomNiveau(int niveau) {
    static const char* const noms[] = {"Débutant", "Facile", "Moyen (0,2 s)", "Fort (1 s)", "Maximum (3 s)"};
    return noms[std::clamp(niveau, 1, NIVEAU_MAX) - 1];
}

void redimensionnerTT(std::size_t megaOctets) {
    std::size_t n = 1;
    while (n * 2 * sizeof(EntreeTT) <= std::max<std::size_t>(megaOctets, 1) << 20) n *= 2;
    tableTT.assign(n, EntreeTT{});
    masqueTT = n - 1;
}

void viderTT() { std::fill(tableTT.begin(), tableTT.end(), EntreeTT{}); }
