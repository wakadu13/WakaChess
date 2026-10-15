// board.hpp - Représentation du plateau, génération des coups et règles
#pragma once

#include <cstdint>
#include <string>

// Pièces : positives = Blancs (Majuscules), négatives = Noirs (minuscules), 0 = case vide
enum Piece : int8_t { VIDE = 0, PION = 1, CAVALIER = 2, FOU = 3, TOUR = 4, DAME = 5, ROI = 6 };
enum Camp : int { BLANC = 0, NOIR = 1 };

// Droits au roque
enum : uint8_t { ROQUE_BR = 1, ROQUE_BD = 2, ROQUE_NR = 4, ROQUE_ND = 8 }; // R = côté roi, D = côté dame

// Drapeaux d'un coup
enum : uint8_t { F_CAPTURE = 1, F_DOUBLE = 2, F_EN_PASSANT = 4, F_ROQUE = 8 };

constexpr int MAX_COUPS = 256;

// Case = y*8 + x, avec y = 0 la rangée de départ des Blancs (rangée 1) et x = 0 la colonne a
inline int caseDe(int x, int y) { return y * 8 + x; }
inline int colonne(int c) { return c & 7; }
inline int rangee(int c) { return c >> 3; }

struct Coup {
    uint8_t de = 0, vers = 0;
    uint8_t promo = VIDE; // type de pièce de promotion (DAME, TOUR, FOU, CAVALIER) ou VIDE
    uint8_t flags = 0;
};

inline bool operator==(const Coup& a, const Coup& b) {
    return a.de == b.de && a.vers == b.vers && a.promo == b.promo;
}

// Ce qu'il faut mémoriser pour annuler un coup
struct Annulation {
    int8_t capturee;
    uint8_t roque;
    int8_t enPassant;
    int compteur50;
};

struct Position {
    int8_t plateau[64];
    int trait = BLANC;      // camp qui doit jouer
    uint8_t roque = 0;      // droits au roque
    int8_t enPassant = -1;  // case de prise en passant, -1 si aucune
    int compteur50 = 0;     // demi-coups depuis la dernière capture ou le dernier coup de pion
    int caseRoi[2] = {4, 60};

    void positionInitiale();
    bool chargerFEN(const std::string& fen);

    bool estAttaquee(int c, int parCamp) const;
    bool enEchec(int camp) const { return estAttaquee(caseRoi[camp], camp ^ 1); }

    // Coups pseudo-légaux (le roi peut rester en échec) ; capturesSeules pour la quiescence
    int genererPseudo(Coup* liste, bool capturesSeules = false) const;
    // Coups légaux uniquement
    int genererLegaux(Coup* liste);

    void jouer(const Coup& m, Annulation& a);
    void annuler(const Coup& m, const Annulation& a);

    uint64_t hash() const; // Pour la triple répétition
    void afficher() const;
};

char lettrePiece(int8_t p);
std::string coupVersTexte(const Coup& m); // ex: "e2e4", "e7e8D"
uint64_t perft(Position& pos, int profondeur);
