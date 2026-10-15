# WakaChess
♟️ WakaChess - Moteur d'Échecs en C++

**WakaChess** est un jeu d'échecs en console contre une Intelligence Artificielle. Le projet implémente toutes les règles des échecs (déplacements, roque, prise en passant, promotion, échec/mat, pat, nulles) et un moteur de réflexion basé sur l'algorithme Negamax.

Le projet était à l'origine écrit en Python ; il a été entièrement réécrit en C++ pour la performance. À profondeur égale, l'IA réfléchit environ **2 500 fois plus vite** (profondeur 5 depuis la position initiale : ~60 s en Python, ~0,02 s en C++).

## 🚀 Fonctionnalités

### 🎮 Gameplay
- **Règles complètes** : déplacements de chaque pièce, double pas des pions, captures.
- **Le roque** : petit et grand roque, avec vérification que le roi et la tour n'ont pas bougé, que le roi n'est pas en échec et que les cases traversées ne sont pas attaquées.
- **La prise en passant**.
- **La promotion** : Dame automatiquement pour l'IA, choix interactif Dame/Tour/Cavalier/Fou pour le joueur (l'IA envisage toutes les promotions pendant sa recherche).
- **Saisie** par coordonnées (`4 1 4 3`) ou en notation `e2e4`.

### 🤖 Intelligence artificielle
- **Negamax** avec **élagage Alpha-Bêta**.
- **Quiescence Search** : prolonge la recherche sur les captures pour éviter l'effet d'horizon.
- **Piece-Square Tables (PST)** pour les pions et les cavaliers, bonus pour le roi roqué.
- **Tri des coups** : captures et promotions explorées en priorité.
- **Détection des mats** dans la recherche (le mat le plus rapide est préféré) et du pat.

### ⚖️ Arbitrage
- Échec, échec et mat, pat.
- Nulle par triple répétition (hachage Zobrist des positions).
- Nulle par la règle des 50 coups.

---

## 🛠️ Compilation et lancement

**Prérequis** : un compilateur C++17 (g++ ou clang++). Aucune dépendance externe.

```bash
git clone https://github.com/wakadu13/WakaChess.git
cd WakaChess
```

Avec CMake :
```bash
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build
./build/wakachess
```

Ou avec Make :
```bash
make
./wakachess
```

`-march=native` est activé par défaut ; pour un binaire portable : `cmake -B build -DWAKACHESS_NATIVE=OFF`.

### Options
| Commande | Rôle |
|---|---|
| `./wakachess` | Lance une partie contre l'IA. |
| `./wakachess --perft N [FEN]` | Compte les positions jusqu'à la profondeur N (validation du générateur de coups). |
| `./wakachess --bench P` | Mesure la réflexion de l'IA à la profondeur P depuis la position initiale. |

---

## 🧩 Fonctionnement

| Fichier | Rôle |
|---|---|
| [src/board.hpp](src/board.hpp) / [src/board.cpp](src/board.cpp) | Plateau, état de la partie (trait, droits au roque, case en passant, compteur 50 coups), génération des coups, cases attaquées, jouer/annuler un coup, perft. |
| [src/search.hpp](src/search.hpp) / [src/search.cpp](src/search.cpp) | Évaluation (matériel + PST), tri des coups, Negamax alpha-bêta, quiescence, `trouverMeilleurCoup`. |
| [src/main.cpp](src/main.cpp) | Boucle de jeu, saisie, promotion, fin de partie et nulles. |

### Représentation du plateau
- Le plateau est un tableau de 64 cases `plateau[y*8 + x]`, `y = 0` étant la rangée de départ des Blancs (rangée 1) et `x = 0` la colonne a. Disposition standard : Roi en e1 (`x=4`), Dame en d1 (`x=3`).
- Les pièces sont des entiers (positifs pour les Blancs, négatifs pour les Noirs). À l'affichage : Blancs (Majuscules) `P C F T D R`, Noirs (minuscules) `p c f t d r` (Pion, Cavalier, Fou, Tour, Dame, Roi), `.` pour une case vide.

### Pourquoi c'est rapide
- Aucune allocation pendant la recherche : les coups sont générés dans des tableaux fixes sur la pile.
- Les coups sont joués puis annulés sur place (`jouer` / `annuler`) au lieu de copier le plateau.
- Les déplacements du cavalier, du roi et les rayons des pièces glissantes sont précalculés.
- « Cette case est-elle attaquée ? » se teste en partant de la case elle-même (rayons, sauts de cavalier, pions, roi), au lieu de générer tous les coups adverses.
- La quiescence ne génère que les captures.

### Validation
Le générateur de coups donne les valeurs perft de référence, par exemple :

| Position | Profondeur | Nœuds |
|---|---|---|
| Initiale | 5 | 4 865 609 |
| Kiwipete | 4 | 4 085 603 |

---

## 🐞 Corrections par rapport à la version Python
- **Prise en passant** : la case en passant était une variable globale copiée par `from plateau import *`, jamais mise à jour dans `geo.py` ; la prise en passant n'était donc jamais proposée.
- **Mats invisibles pour l'IA** : quand il n'y avait plus de coup légal, la recherche renvoyait l'évaluation matérielle au lieu d'un score de mat ou de nulle.
- **Table des pions inversée** : la PST était lue à l'envers, ce qui récompensait les pions restés sur leur case de départ.
- **Recherche** : le roque, la promotion et les droits au roque sont maintenant gérés correctement pendant la simulation des coups.

---

## 🗺️ Pistes d'amélioration
- **Recherche à temps limité** (*iterative deepening*).
- **Table de transposition** (le hachage Zobrist existe déjà).
- **Bitboards** pour une génération de coups encore plus rapide.
- **Protocole UCI** pour brancher le moteur sur une interface graphique.
- **Sauvegarde / chargement de partie** (PGN) et annulation du dernier coup.
