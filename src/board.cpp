// board.cpp - Plateau, génération des coups, exécution / annulation (ex plateau.py + geo.py)
#include "board.hpp"

#include <algorithm>
#include <cstdlib>
#include <iostream>
#include <sstream>

namespace {

// --- Tables précalculées (cavalier, roi, rayons des pièces glissantes) ---
struct Tables {
    int8_t cavalier[64][8]; int nbCavalier[64];
    int8_t roi[64][8];      int nbRoi[64];
    // 8 directions : 0-3 orthogonales (tour), 4-7 diagonales (fou)
    int8_t rayon[64][8][7]; int longueur[64][8];
    uint8_t masqueRoque[64]; // droits conservés quand une case est touchée (départ ou arrivée)
    uint64_t zobrist[13][64], zTrait, zRoque[16], zEnPassant[8];

    Tables() {
        const int dc[8][2] = {{1,2},{2,1},{2,-1},{1,-2},{-1,-2},{-2,-1},{-2,1},{-1,2}};
        const int dr[8][2] = {{0,1},{0,-1},{1,0},{-1,0},{1,1},{1,-1},{-1,1},{-1,-1}};
        for (int c = 0; c < 64; ++c) {
            int x = colonne(c), y = rangee(c);
            nbCavalier[c] = nbRoi[c] = 0;
            for (auto& d : dc) {
                int nx = x + d[0], ny = y + d[1];
                if (nx >= 0 && nx < 8 && ny >= 0 && ny < 8) cavalier[c][nbCavalier[c]++] = caseDe(nx, ny);
            }
            for (int d = 0; d < 8; ++d) {
                int nx = x + dr[d][0], ny = y + dr[d][1];
                if (nx >= 0 && nx < 8 && ny >= 0 && ny < 8) roi[c][nbRoi[c]++] = caseDe(nx, ny);
                longueur[c][d] = 0;
                while (nx >= 0 && nx < 8 && ny >= 0 && ny < 8) {
                    rayon[c][d][longueur[c][d]++] = caseDe(nx, ny);
                    nx += dr[d][0]; ny += dr[d][1];
                }
            }
            masqueRoque[c] = 0xF;
        }
        masqueRoque[4]  &= ~(ROQUE_BR | ROQUE_BD);
        masqueRoque[7]  &= ~ROQUE_BR;
        masqueRoque[0]  &= ~ROQUE_BD;
        masqueRoque[60] &= ~(ROQUE_NR | ROQUE_ND);
        masqueRoque[63] &= ~ROQUE_NR;
        masqueRoque[56] &= ~ROQUE_ND;

        uint64_t graine = 0x9E3779B97F4A7C15ULL; // xorshift64*
        auto aleatoire = [&graine] {
            graine ^= graine >> 12; graine ^= graine << 25; graine ^= graine >> 27;
            return graine * 2685821657736338717ULL;
        };
        for (auto& p : zobrist) for (auto& v : p) v = aleatoire();
        zTrait = aleatoire();
        for (auto& v : zRoque) v = aleatoire();
        for (auto& v : zEnPassant) v = aleatoire();
    }
};

const Tables T;

inline int signe(int camp) { return camp == BLANC ? 1 : -1; }
inline bool estAuCamp(int8_t p, int camp) { return camp == BLANC ? p > 0 : p < 0; }

inline void ajouter(Coup* liste, int& n, int de, int vers, uint8_t flags, uint8_t promo = VIDE) {
    Coup& m = liste[n++];
    m.de = de; m.vers = vers; m.flags = flags; m.promo = promo;
}

inline void ajouterPion(Coup* liste, int& n, int de, int vers, uint8_t flags, bool promotion) {
    if (promotion) {
        for (uint8_t p : {DAME, TOUR, FOU, CAVALIER}) ajouter(liste, n, de, vers, flags, p);
    } else {
        ajouter(liste, n, de, vers, flags);
    }
}

} // namespace

char lettrePiece(int8_t p) {
    static const char lettres[] = ".PCFTDR"; // Pion, Cavalier, Fou, Tour, Dame, Roi
    char l = lettres[std::abs(p)];
    return p < 0 ? static_cast<char>(l - 'A' + 'a') : l;
}

std::string coupVersTexte(const Coup& m) {
    std::string s;
    s += static_cast<char>('a' + colonne(m.de)); s += static_cast<char>('1' + rangee(m.de));
    s += static_cast<char>('a' + colonne(m.vers)); s += static_cast<char>('1' + rangee(m.vers));
    if (m.promo) s += lettrePiece(m.promo);
    return s;
}

void Position::positionInitiale() {
    chargerFEN("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
}

bool Position::chargerFEN(const std::string& fen) {
    std::istringstream in(fen);
    std::string place, camp = "w", roques = "-", ep = "-";
    int c50 = 0;
    if (!(in >> place)) return false;
    in >> camp >> roques >> ep >> c50;

    for (auto& p : plateau) p = VIDE;
    int x = 0, y = 7;
    for (char ch : place) {
        if (ch == '/') { --y; x = 0; continue; }
        if (ch >= '1' && ch <= '8') { x += ch - '0'; continue; }
        int8_t type;
        switch (ch | 0x20) { // en minuscule
            case 'p': type = PION; break;     case 'n': type = CAVALIER; break;
            case 'b': type = FOU; break;      case 'r': type = TOUR; break;
            case 'q': type = DAME; break;     case 'k': type = ROI; break;
            default: return false;
        }
        if (x > 7 || y < 0) return false;
        int8_t p = (ch >= 'a') ? static_cast<int8_t>(-type) : type;
        plateau[caseDe(x, y)] = p;
        if (type == ROI) caseRoi[p > 0 ? BLANC : NOIR] = caseDe(x, y);
        ++x;
    }
    trait = (camp == "b") ? NOIR : BLANC;
    roque = 0;
    for (char ch : roques) {
        if (ch == 'K') roque |= ROQUE_BR; else if (ch == 'Q') roque |= ROQUE_BD;
        else if (ch == 'k') roque |= ROQUE_NR; else if (ch == 'q') roque |= ROQUE_ND;
    }
    enPassant = (ep.size() == 2) ? static_cast<int8_t>(caseDe(ep[0] - 'a', ep[1] - '1')) : -1;
    compteur50 = c50;
    cle = calculerCle();
    nbHistorique = 0;
    return true;
}

bool Position::estAttaquee(int c, int parCamp) const {
    const int s = signe(parCamp);
    const int x = colonne(c), y = rangee(c);

    // Pions : un pion blanc attaque vers le haut, il se trouve donc une rangée en dessous
    int yp = y - s;
    if (yp >= 0 && yp < 8) {
        if (x > 0 && plateau[caseDe(x - 1, yp)] == s * PION) return true;
        if (x < 7 && plateau[caseDe(x + 1, yp)] == s * PION) return true;
    }
    for (int i = 0; i < T.nbCavalier[c]; ++i)
        if (plateau[T.cavalier[c][i]] == s * CAVALIER) return true;
    for (int i = 0; i < T.nbRoi[c]; ++i)
        if (plateau[T.roi[c][i]] == s * ROI) return true;

    // Pièces glissantes : on remonte chaque rayon jusqu'à la première pièce
    for (int d = 0; d < 8; ++d) {
        const int8_t lent = (d < 4) ? TOUR : FOU;
        for (int i = 0; i < T.longueur[c][d]; ++i) {
            int8_t p = plateau[T.rayon[c][d][i]];
            if (p == VIDE) continue;
            if (p == s * lent || p == s * DAME) return true;
            break;
        }
    }
    return false;
}

int Position::genererPseudo(Coup* liste, bool capturesSeules) const {
    int n = 0;
    const int s = signe(trait);
    const int eux = trait ^ 1;

    for (int c = 0; c < 64; ++c) {
        const int8_t p = plateau[c];
        if (!estAuCamp(p, trait)) continue;

        switch (std::abs(p)) {
        case PION: {
            const int x = colonne(c), y = rangee(c);
            const int ny = y + s;
            const bool promotion = (ny == 7 || ny == 0);
            // Avance simple et double
            if (capturesSeules && promotion && plateau[caseDe(x, ny)] == VIDE)
                ajouter(liste, n, c, caseDe(x, ny), 0, DAME); // la promotion en dame est gardée
            if (!capturesSeules && plateau[caseDe(x, ny)] == VIDE) {
                ajouterPion(liste, n, c, caseDe(x, ny), 0, promotion);
                const int depart = (trait == BLANC) ? 1 : 6;
                if (y == depart && plateau[caseDe(x, y + 2 * s)] == VIDE)
                    ajouter(liste, n, c, caseDe(x, y + 2 * s), F_DOUBLE);
            }
            // Prises en diagonale (et en passant)
            for (int dx : {-1, 1}) {
                const int nx = x + dx;
                if (nx < 0 || nx > 7) continue;
                const int cible = caseDe(nx, ny);
                if (estAuCamp(plateau[cible], eux)) ajouterPion(liste, n, c, cible, F_CAPTURE, promotion);
                else if (cible == enPassant) ajouter(liste, n, c, cible, F_CAPTURE | F_EN_PASSANT);
            }
            break;
        }
        case CAVALIER:
            for (int i = 0; i < T.nbCavalier[c]; ++i) {
                const int cible = T.cavalier[c][i];
                const int8_t q = plateau[cible];
                if (q == VIDE) { if (!capturesSeules) ajouter(liste, n, c, cible, 0); }
                else if (estAuCamp(q, eux)) ajouter(liste, n, c, cible, F_CAPTURE);
            }
            break;
        case ROI: {
            for (int i = 0; i < T.nbRoi[c]; ++i) {
                const int cible = T.roi[c][i];
                const int8_t q = plateau[cible];
                if (q == VIDE) { if (!capturesSeules) ajouter(liste, n, c, cible, 0); }
                else if (estAuCamp(q, eux)) ajouter(liste, n, c, cible, F_CAPTURE);
            }
            if (capturesSeules) break;
            // Roque : cases vides entre le roi et la tour, roi non en échec, cases traversées non attaquées
            const int base = (trait == BLANC) ? 0 : 56;
            const uint8_t cRoi = (trait == BLANC) ? ROQUE_BR : ROQUE_NR;
            const uint8_t cDame = (trait == BLANC) ? ROQUE_BD : ROQUE_ND;
            if ((roque & (cRoi | cDame)) && c == base + 4 && !estAttaquee(c, eux)) {
                if ((roque & cRoi) && plateau[base + 5] == VIDE && plateau[base + 6] == VIDE &&
                    !estAttaquee(base + 5, eux) && !estAttaquee(base + 6, eux))
                    ajouter(liste, n, c, base + 6, F_ROQUE);
                if ((roque & cDame) && plateau[base + 3] == VIDE && plateau[base + 2] == VIDE &&
                    plateau[base + 1] == VIDE && !estAttaquee(base + 3, eux) && !estAttaquee(base + 2, eux))
                    ajouter(liste, n, c, base + 2, F_ROQUE);
            }
            break;
        }
        default: { // Fou, Tour, Dame
            const int type = std::abs(p);
            const int dDebut = (type == FOU) ? 4 : 0;
            const int dFin = (type == TOUR) ? 4 : 8;
            for (int d = dDebut; d < dFin; ++d) {
                for (int i = 0; i < T.longueur[c][d]; ++i) {
                    const int cible = T.rayon[c][d][i];
                    const int8_t q = plateau[cible];
                    if (q == VIDE) { if (!capturesSeules) ajouter(liste, n, c, cible, 0); continue; }
                    if (estAuCamp(q, eux)) ajouter(liste, n, c, cible, F_CAPTURE);
                    break;
                }
            }
            break;
        }
        }
    }
    return n;
}

int Position::genererLegaux(Coup* liste) {
    Coup pseudo[MAX_COUPS];
    const int nb = genererPseudo(pseudo);
    const int camp = trait;
    int n = 0;
    for (int i = 0; i < nb; ++i) {
        Annulation a;
        jouer(pseudo[i], a);
        if (!enEchec(camp)) liste[n++] = pseudo[i];
        annuler(pseudo[i], a);
    }
    return n;
}

void Position::jouer(const Coup& m, Annulation& a) {
    const int8_t piece = plateau[m.de];
    const int s = signe(trait);

    a.roque = roque; a.enPassant = enPassant; a.compteur50 = compteur50; a.cle = cle;
    a.capturee = plateau[m.vers];
    if (nbHistorique < MAX_HISTORIQUE) historique[nbHistorique] = cle;
    ++nbHistorique;

    uint64_t k = cle;
    if (m.flags & F_EN_PASSANT) {
        const int casePrise = m.vers - 8 * s; // le pion adverse est juste derrière la case d'arrivée
        a.capturee = plateau[casePrise];
        plateau[casePrise] = VIDE;
        k ^= T.zobrist[a.capturee + 6][casePrise];
    } else if (a.capturee != VIDE) {
        k ^= T.zobrist[a.capturee + 6][m.vers];
    }

    const int8_t arrivee = m.promo ? static_cast<int8_t>(s * m.promo) : piece;
    plateau[m.vers] = arrivee;
    plateau[m.de] = VIDE;
    k ^= T.zobrist[piece + 6][m.de] ^ T.zobrist[arrivee + 6][m.vers];

    if (m.flags & F_ROQUE) {
        const int base = m.vers & 56;
        const int tDe = colonne(m.vers) == 6 ? base + 7 : base + 0;
        const int tVers = colonne(m.vers) == 6 ? base + 5 : base + 3;
        plateau[tVers] = plateau[tDe]; plateau[tDe] = VIDE;
        k ^= T.zobrist[s * TOUR + 6][tDe] ^ T.zobrist[s * TOUR + 6][tVers];
    }
    if (piece == s * ROI) caseRoi[trait] = m.vers;

    compteur50 = (std::abs(piece) == PION || a.capturee != VIDE) ? 0 : compteur50 + 1;
    k ^= T.zRoque[roque];
    roque &= T.masqueRoque[m.de] & T.masqueRoque[m.vers];
    k ^= T.zRoque[roque];
    if (enPassant >= 0) k ^= T.zEnPassant[colonne(enPassant)];
    enPassant = (m.flags & F_DOUBLE) ? static_cast<int8_t>((m.de + m.vers) / 2) : -1;
    if (enPassant >= 0) k ^= T.zEnPassant[colonne(enPassant)];
    trait ^= 1;
    cle = k ^ T.zTrait;
}

void Position::jouerNul(Annulation& a) {
    a.roque = roque; a.enPassant = enPassant; a.compteur50 = compteur50; a.cle = cle; a.capturee = VIDE;
    if (nbHistorique < MAX_HISTORIQUE) historique[nbHistorique] = cle;
    ++nbHistorique;
    if (enPassant >= 0) cle ^= T.zEnPassant[colonne(enPassant)];
    enPassant = -1;
    ++compteur50;
    trait ^= 1;
    cle ^= T.zTrait;
}

void Position::annulerNul(const Annulation& a) {
    trait ^= 1;
    --nbHistorique;
    enPassant = a.enPassant; compteur50 = a.compteur50; cle = a.cle;
}

int Position::repetitions() const {
    // Seules les positions depuis la dernière capture ou le dernier coup de pion peuvent se répéter
    int n = 0;
    const int fin = std::max(0, nbHistorique - compteur50);
    for (int i = std::min(nbHistorique, MAX_HISTORIQUE) - 2; i >= fin; i -= 2)
        if (historique[i] == cle) ++n;
    return n;
}

bool Position::aDesPiecesLourdes(int camp) const {
    for (int c = 0; c < 64; ++c) {
        const int8_t p = plateau[c];
        if (estAuCamp(p, camp) && std::abs(p) != PION && std::abs(p) != ROI) return true;
    }
    return false;
}

void Position::annuler(const Coup& m, const Annulation& a) {
    trait ^= 1;
    const int s = signe(trait);
    const int8_t piece = m.promo ? static_cast<int8_t>(s * PION) : plateau[m.vers];

    plateau[m.de] = piece;
    if (m.flags & F_EN_PASSANT) {
        plateau[m.vers] = VIDE;
        plateau[m.vers - 8 * s] = a.capturee;
    } else {
        plateau[m.vers] = a.capturee;
    }
    if (m.flags & F_ROQUE) {
        const int base = m.vers & 56;
        if (colonne(m.vers) == 6) { plateau[base + 7] = plateau[base + 5]; plateau[base + 5] = VIDE; }
        else                      { plateau[base + 0] = plateau[base + 3]; plateau[base + 3] = VIDE; }
    }
    if (piece == s * ROI) caseRoi[trait] = m.de;

    roque = a.roque; enPassant = a.enPassant; compteur50 = a.compteur50; cle = a.cle;
    --nbHistorique;
}

uint64_t Position::calculerCle() const {
    uint64_t h = 0;
    for (int c = 0; c < 64; ++c)
        if (plateau[c] != VIDE) h ^= T.zobrist[plateau[c] + 6][c];
    if (trait == NOIR) h ^= T.zTrait;
    h ^= T.zRoque[roque];
    if (enPassant >= 0) h ^= T.zEnPassant[colonne(enPassant)];
    return h;
}

void Position::afficher() const {
    // Même orientation que la version Python : rangée y = 0 (Blancs) en haut
    std::cout << "    ";
    for (int x = 0; x < 8; ++x) std::cout << ' ' << x << ' ';
    std::cout << '\n';
    for (int y = 0; y < 8; ++y) {
        std::cout << "  " << y << ' ';
        for (int x = 0; x < 8; ++x) std::cout << ' ' << lettrePiece(plateau[caseDe(x, y)]) << ' ';
        std::cout << "  " << (y + 1) << '\n';
    }
    std::cout << "    ";
    for (int x = 0; x < 8; ++x) std::cout << ' ' << static_cast<char>('a' + x) << ' ';
    std::cout << '\n';
}

uint64_t perft(Position& pos, int profondeur) {
    Coup liste[MAX_COUPS];
    const int n = pos.genererLegaux(liste);
    if (profondeur <= 1) return profondeur == 1 ? n : 1;
    uint64_t total = 0;
    for (int i = 0; i < n; ++i) {
        Annulation a;
        pos.jouer(liste[i], a);
        total += perft(pos, profondeur - 1);
        pos.annuler(liste[i], a);
    }
    return total;
}
