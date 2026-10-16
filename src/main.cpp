// main.cpp - Point d'entrée et boucle de jeu de WakaChess (ex main.py + utilitaire.py)
#include "board.hpp"
#include "search.hpp"
#include "uci.hpp"

#include <algorithm>
#include <atomic>
#include <cctype>
#include <chrono>
#include <cstdlib>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#ifdef _WIN32
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#endif

namespace {

using Horloge = std::chrono::steady_clock;

double secondesDepuis(Horloge::time_point debut) {
    return std::chrono::duration<double>(Horloge::now() - debut).count();
}

bool lireLigne(const std::string& invite, std::string& ligne) {
    std::cout << invite << std::flush;
    if (!std::getline(std::cin, ligne)) return false;
    // Suppression des espaces en début et fin
    const auto debut = ligne.find_first_not_of(" \t\r");
    const auto fin = ligne.find_last_not_of(" \t\r");
    ligne = (debut == std::string::npos) ? "" : ligne.substr(debut, fin - debut + 1);
    return true;
}

std::string enMinuscules(std::string s) {
    for (auto& ch : s) ch = static_cast<char>(std::tolower(static_cast<unsigned char>(ch)));
    return s;
}

// Accepte "x_depart y_depart x_arrivee y_arrivee" (0-7) ou la notation "e2e4"
bool lireCommande(const std::string& commande, int& de, int& vers) {
    std::istringstream in(commande);
    std::vector<int> v;
    int k;
    while (in >> k) v.push_back(k);
    if (in.eof() && v.size() == 4) {
        for (int c : v) if (c < 0 || c > 7) return false;
        de = caseDe(v[0], v[1]);
        vers = caseDe(v[2], v[3]);
        return true;
    }
    const std::string s = enMinuscules(commande);
    if (s.size() == 4 && s[0] >= 'a' && s[0] <= 'h' && s[1] >= '1' && s[1] <= '8' &&
        s[2] >= 'a' && s[2] <= 'h' && s[3] >= '1' && s[3] <= '8') {
        de = caseDe(s[0] - 'a', s[1] - '1');
        vers = caseDe(s[2] - 'a', s[3] - '1');
        return true;
    }
    return false;
}

std::string decrireCoup(const Coup& m) {
    std::ostringstream o;
    o << '(' << colonne(m.de) << ", " << rangee(m.de) << ") -> (" << colonne(m.vers) << ", "
      << rangee(m.vers) << ") [" << coupVersTexte(m) << ']';
    return o.str();
}

int lancerPerft(int profondeurMax, const std::string& fen) {
    Position pos;
    if (fen.empty()) pos.positionInitiale();
    else if (!pos.chargerFEN(fen)) { std::cerr << "FEN invalide\n"; return 1; }
    for (int d = 1; d <= profondeurMax; ++d) {
        const auto debut = Horloge::now();
        const uint64_t n = perft(pos, d);
        const double t = secondesDepuis(debut);
        std::cout << "perft " << d << " : " << n << "  (" << t << " s, "
                  << static_cast<uint64_t>(n / std::max(t, 1e-9)) << " noeuds/s)\n";
    }
    return 0;
}

int lancerBench(int profondeur) {
    Position pos;
    pos.positionInitiale();
    const auto debut = Horloge::now();
    const ResultatRecherche r = trouverMeilleurCoup(pos, profondeur);
    std::cout << "profondeur " << profondeur << " : " << decrireCoup(r.coup) << " score " << r.score
              << ", " << r.noeuds << " noeuds, " << secondesDepuis(debut) << " s\n";
    return 0;
}

int jouerPartie() {
    Position pos;
    pos.positionInitiale();
    std::vector<uint64_t> historique{pos.hash()};
    int nbTour = 0;
    int niveau = 3;
    std::string ligne;

    std::cout << "=== Bienvenue dans WakaChess ===\n";
    if (!lireLigne("Choisissez votre camp (M pour Majuscules/Blancs, m pour Minuscules/Noirs) : ", ligne))
        return 0;
    if (ligne == "uci") return boucleUCI(true); // lancé par une interface graphique
    // Si le joueur choisit m (minuscule), il joue les Noirs : l'IA joue alors les Blancs
    const int campIA = (ligne == "m") ? BLANC : NOIR;
    std::cout << (campIA == BLANC ? "L'IA joue les Majuscules et commence la partie.\n"
                                  : "Vous jouez les Majuscules et commencez la partie.\n");

    std::cout << "Niveaux :";
    for (int n = 1; n <= NIVEAU_MAX; ++n) std::cout << "  " << n << " = " << nomNiveau(n);
    std::cout << '\n';
    if (!lireLigne("Choisissez la difficulté (1-5) : ", ligne)) return 0;
    try {
        niveau = std::clamp(std::stoi(ligne), 1, NIVEAU_MAX);
    } catch (...) {
        std::cout << "Niveau par défaut (3) sélectionné.\n";
    }
    const std::atomic<bool> jamais{false};

    while (true) {
        std::cout << "\n" << std::string(20, '=') << "\nTOUR NUMERO : " << nbTour << '\n';
        pos.afficher();
        if (pos.enEchec(pos.trait)) std::cout << "Échec !\n";

        Coup legaux[MAX_COUPS];
        const int nbLegaux = pos.genererLegaux(legaux);

        Coup choisi;
        if (pos.trait == campIA) {
            std::cout << "Le bot réfléchit...\n";
            const auto debut = Horloge::now();
            const ResultatRecherche r = rechercher(pos, limitesPourNiveau(niveau), jamais);
            if (!r.trouve) { std::cout << "Le bot est bloqué. Fin de la partie.\n"; break; }
            choisi = r.coup;
            std::cout << "Le bot a joué : " << decrireCoup(choisi) << " (Score: " << r.score << ", "
                      << r.noeuds << " noeuds, " << secondesDepuis(debut) << " s)\n";
        } else {
            if (!lireLigne("Votre coup (ex: 4 1 4 3 ou e2e4) ou 'stop' : ", ligne)) break;
            const std::string cmd = enMinuscules(ligne);
            if (cmd == "stop") break;
            if (cmd == "help") {
                std::cout << "Entrez : x_depart y_depart x_arrivee y_arrivee (0-7), ou la notation e2e4\n";
                continue;
            }
            int de, vers;
            if (!lireCommande(ligne, de, vers)) { std::cout << "Format invalide. Exemple: 4 1 4 3\n"; continue; }

            const int8_t piece = pos.plateau[de];
            if (piece == VIDE || (piece > 0) != (pos.trait == BLANC)) {
                std::cout << "Ce n'est pas à votre tour de jouer cette pièce !\n";
                continue;
            }

            const Coup* trouve = nullptr;
            for (int i = 0; i < nbLegaux; ++i)
                if (legaux[i].de == de && legaux[i].vers == vers) { trouve = &legaux[i]; break; }
            if (!trouve) { std::cout << "Coup illégal ! Vérifiez les règles.\n"; continue; }
            choisi = *trouve;

            if (choisi.promo) {
                if (!lireLigne("Promotion ! Choisissez la pièce (D=Dame, T=Tour, C=Cavalier, F=Fou) : ", ligne))
                    break;
                const char c = ligne.empty() ? 'D' : static_cast<char>(std::toupper(static_cast<unsigned char>(ligne[0])));
                choisi.promo = (c == 'T') ? TOUR : (c == 'C') ? CAVALIER : (c == 'F') ? FOU : DAME;
            }
            std::cout << "Coup effectué.\n";
        }

        Annulation a;
        pos.jouer(choisi, a);
        ++nbTour;

        // Vérification de fin de partie
        Coup reponses[MAX_COUPS];
        if (pos.genererLegaux(reponses) == 0) {
            pos.afficher();
            std::cout << (pos.enEchec(pos.trait) ? "Echec et mat !\n" : "Pat !\n");
            break;
        }
        const uint64_t h = pos.hash();
        historique.push_back(h);
        if (std::count(historique.begin(), historique.end(), h) >= 3) {
            pos.afficher();
            std::cout << "Nulle par répétition !\n";
            break;
        }
        if (pos.compteur50 >= 100) { // 50 coups par camp = 100 demi-coups
            pos.afficher();
            std::cout << "Nulle (règle des 50 coups) !\n";
            break;
        }
    }

    std::cout << "Merci d'avoir joué à WakaChess !\n";
#ifdef _WIN32
    // Lancé par double-clic, la console se fermerait aussitôt
    lireLigne("Appuyez sur Entrée pour quitter...", ligne);
#endif
    return 0;
}

} // namespace

int main(int argc, char** argv) {
    std::ios::sync_with_stdio(false);
#ifdef _WIN32
    SetConsoleOutputCP(CP_UTF8); // pour afficher correctement les accents
    SetConsoleCP(CP_UTF8);
#endif
    if (argc >= 3 && std::string(argv[1]) == "--perft")
        return lancerPerft(std::atoi(argv[2]), argc >= 4 ? argv[3] : "");
    if (argc >= 3 && std::string(argv[1]) == "--bench")
        return lancerBench(std::atoi(argv[2]));
    if (argc >= 2 && std::string(argv[1]) == "--uci")
        return boucleUCI(false);
    if (argc >= 2) {
        std::cout << "Usage : " << argv[0] << " [--uci] [--perft N [FEN]] [--bench PROFONDEUR]\n";
        return 1;
    }
    return jouerPartie();
}
