// uci.cpp - Protocole UCI : https://www.shredderchess.com/chess-features/uci-universal-chess-interface.html
#include "uci.hpp"

#include "board.hpp"
#include "search.hpp"

#include <algorithm>
#include <atomic>
#include <iostream>
#include <mutex>
#include <sstream>
#include <string>
#include <thread>

namespace {

std::mutex sortieMutex;

void envoyer(const std::string& ligne) {
    std::lock_guard<std::mutex> verrou(sortieMutex);
    std::cout << ligne << std::endl;
}

bool appliquerCoupUCI(Position& pos, const std::string& texte) {
    Coup legaux[MAX_COUPS];
    const int n = pos.genererLegaux(legaux);
    for (int i = 0; i < n; ++i) {
        if (coupVersUCI(legaux[i]) == texte) {
            Annulation a;
            pos.jouer(legaux[i], a);
            return true;
        }
    }
    return false;
}

std::string scoreVersUCI(int score) {
    if (std::abs(score) >= SEUIL_MAT) {
        const int plis = SCORE_MAT - std::abs(score);
        const int coups = (plis + 1) / 2;
        return "mate " + std::to_string(score > 0 ? coups : -coups);
    }
    return "cp " + std::to_string(score);
}

class Moteur {
public:
    ~Moteur() { arreter(); }

    void position(std::istringstream& in) {
        arreter();
        std::string mot;
        in >> mot;
        if (mot == "startpos") {
            pos_.positionInitiale();
            in >> mot; // "moves" éventuel
        } else if (mot == "fen") {
            std::string fen;
            while (in >> mot && mot != "moves") fen += mot + ' ';
            if (!pos_.chargerFEN(fen)) { envoyer("info string FEN invalide"); pos_.positionInitiale(); }
        } else {
            return;
        }
        while (in >> mot)
            if (!appliquerCoupUCI(pos_, mot)) { envoyer("info string coup invalide : " + mot); break; }
    }

    void go(std::istringstream& in) {
        arreter();
        LimitesRecherche limites;
        int64_t temps[2] = {-1, -1}, increment[2] = {0, 0};
        int movestogo = 0;
        int64_t movetime = -1;
        bool infini = false;
        std::string mot;
        while (in >> mot) {
            if (mot == "wtime") in >> temps[BLANC];
            else if (mot == "btime") in >> temps[NOIR];
            else if (mot == "winc") in >> increment[BLANC];
            else if (mot == "binc") in >> increment[NOIR];
            else if (mot == "movestogo") in >> movestogo;
            else if (mot == "movetime") in >> movetime;
            else if (mot == "depth") in >> limites.profondeur;
            else if (mot == "nodes") in >> limites.noeuds;
            else if (mot == "infinite") infini = true;
        }

        const int64_t restant = temps[pos_.trait];
        if (movetime >= 0) {
            limites.tempsDurMs = limites.tempsDouxMs = std::max<int64_t>(1, movetime - 10);
        } else if (restant >= 0 && !infini) {
            // Budget : une fraction du temps restant + la majeure partie de l'incrément
            const int64_t marge = std::min<int64_t>(50, restant / 10);
            const int64_t inc = increment[pos_.trait];
            const int64_t budget = restant / (movestogo > 0 ? movestogo + 1 : 25) + inc * 3 / 4;
            limites.tempsDurMs = std::max<int64_t>(1, std::min(budget * 2, restant - marge));
            limites.tempsDouxMs = std::max<int64_t>(1, std::min(budget / 2, restant - marge));
        }

        arret_ = false;
        Position copie = pos_;
        fil_ = std::thread([this, copie, limites, infini]() mutable {
            const ResultatRecherche r = rechercher(copie, limites, arret_,
                [](int profondeur, int score, uint64_t noeuds, int64_t ms, const Coup* pv, int longueur) {
                    std::ostringstream o;
                    o << "info depth " << profondeur << " score " << scoreVersUCI(score) << " nodes " << noeuds
                      << " time " << ms << " nps " << (ms > 0 ? noeuds * 1000 / ms : noeuds) << " pv";
                    for (int i = 0; i < longueur; ++i) o << ' ' << coupVersUCI(pv[i]);
                    envoyer(o.str());
                });
            // En mode "infinite", le meilleur coup n'est envoyé qu'après "stop"
            while (infini && !arret_) std::this_thread::sleep_for(std::chrono::milliseconds(1));
            envoyer("bestmove " + (r.trouve ? coupVersUCI(r.coup) : std::string("0000")));
        });
    }

    void arreter() {
        arret_ = true;
        if (fil_.joinable()) fil_.join();
    }

private:
    Position pos_ = [] { Position p; p.positionInitiale(); return p; }();
    std::atomic<bool> arret_{false};
    std::thread fil_;
};

} // namespace

int boucleUCI(bool dejaRecuUci) {
    Moteur moteur;
    auto identifier = [] {
        envoyer("id name WakaChess");
        envoyer("id author wakadu13");
        envoyer("option name Hash type spin default 16 min 1 max 1024");
        envoyer("uciok");
    };
    if (dejaRecuUci) identifier();

    std::string ligne;
    while (std::getline(std::cin, ligne)) {
        std::istringstream in(ligne);
        std::string commande;
        in >> commande;
        if (commande == "uci") identifier();
        else if (commande == "isready") envoyer("readyok");
        else if (commande == "ucinewgame") { moteur.arreter(); viderTT(); }
        else if (commande == "setoption") {
            std::string mot, nom, valeur;
            while (in >> mot && mot != "value") if (mot != "name") nom += (nom.empty() ? "" : " ") + mot;
            in >> valeur;
            if (nom == "Hash") {
                moteur.arreter();
                try { redimensionnerTT(static_cast<std::size_t>(std::clamp(std::stoi(valeur), 1, 1024))); }
                catch (...) {}
            }
        }
        else if (commande == "position") moteur.position(in);
        else if (commande == "go") moteur.go(in);
        else if (commande == "stop") moteur.arreter();
        else if (commande == "quit") break;
    }
    moteur.arreter();
    return 0;
}
