// web_api.cpp - Fonctions appelées depuis JavaScript (version web compilée en WebAssembly)
#include "board.hpp"
#include "search.hpp"

#include <atomic>
#include <string>
#include <utility>
#include <vector>

#ifdef __EMSCRIPTEN__
#include <emscripten/emscripten.h>
#define EXPORT extern "C" EMSCRIPTEN_KEEPALIVE
#else
#define EXPORT extern "C"
#endif

namespace {

Position pos;
std::vector<std::pair<Coup, Annulation>> partie;
std::string tampon; // les chaînes renvoyées à JavaScript restent valides jusqu'à l'appel suivant

const char* renvoyer(std::string s) {
    tampon = std::move(s);
    return tampon.c_str();
}

} // namespace

// Nouvelle partie depuis la position initiale
EXPORT void wc_nouvelle() {
    pos.positionInitiale();
    partie.clear();
}

// Nouvelle partie depuis une position FEN ; renvoie 0 si la FEN est invalide
EXPORT int wc_fen(const char* fen) {
    partie.clear();
    if (pos.chargerFEN(fen)) return 1;
    pos.positionInitiale();
    return 0;
}

// Joue un coup en notation UCI (e2e4, e7e8q) ; renvoie 0 s'il est illégal
EXPORT int wc_jouer(const char* uci) {
    Coup legaux[MAX_COUPS];
    const int n = pos.genererLegaux(legaux);
    for (int i = 0; i < n; ++i) {
        if (coupVersUCI(legaux[i]) == uci) {
            Annulation a;
            pos.jouer(legaux[i], a);
            partie.emplace_back(legaux[i], a);
            return 1;
        }
    }
    return 0;
}

EXPORT int wc_annuler() {
    if (partie.empty()) return 0;
    pos.annuler(partie.back().first, partie.back().second);
    partie.pop_back();
    return 1;
}

// Coups légaux séparés par des espaces
EXPORT const char* wc_legaux() {
    Coup legaux[MAX_COUPS];
    const int n = pos.genererLegaux(legaux);
    std::string s;
    for (int i = 0; i < n; ++i) {
        if (i) s += ' ';
        s += coupVersUCI(legaux[i]);
    }
    return renvoyer(s);
}

// 64 caractères de a1 à h8 : PNBRQK (Blancs), pnbrqk (Noirs), '.' (vide)
EXPORT const char* wc_plateau() {
    static const char lettres[] = ".PNBRQK";
    std::string s(64, '.');
    for (int c = 0; c < 64; ++c) {
        const int8_t p = pos.plateau[c];
        if (p > 0) s[c] = lettres[p];
        else if (p < 0) s[c] = static_cast<char>(lettres[-p] - 'A' + 'a');
    }
    return renvoyer(s);
}

EXPORT int wc_trait() { return pos.trait; }                 // 0 = Blancs, 1 = Noirs
EXPORT int wc_echec() { return pos.enEchec(pos.trait); }
EXPORT int wc_case_roi() { return pos.caseRoi[pos.trait]; }
EXPORT int wc_etat() { return etatPartie(pos); }             // voir EtatPartie dans board.hpp

// Meilleur coup au niveau donné (1 à 5) : "e2e4 <score>"
EXPORT const char* wc_chercher(int niveau) {
    const std::atomic<bool> jamais{false};
    const ResultatRecherche r = rechercher(pos, limitesPourNiveau(niveau), jamais);
    if (!r.trouve) return renvoyer("");
    return renvoyer(coupVersUCI(r.coup) + ' ' + std::to_string(r.score));
}

EXPORT void wc_vider_tt() { viderTT(); }
