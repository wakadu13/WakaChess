# WakaChess
♟️ WakaChess - Moteur d'Échecs en C++

**WakaChess** est un jeu d'échecs contre une Intelligence Artificielle, jouable sur **Android, iOS, Windows, macOS (Apple Silicon) et Linux**, au doigt ou à la souris. Le projet implémente toutes les règles des échecs (déplacements, roque, prise en passant, promotion, échec/mat, pat, nulles) et un moteur de réflexion d'environ **2460 Elo** (échelle des moteurs).

Le projet était à l'origine écrit en Python ; il a été entièrement réécrit en C++ pour la performance. À profondeur égale, l'IA réfléchit environ **2 500 fois plus vite** (profondeur 5 depuis la position initiale : ~60 s en Python, ~0,02 s en C++).

## 🚀 Fonctionnalités

### 🎮 Gameplay
- **Règles complètes** : déplacements de chaque pièce, double pas des pions, captures.
- **Le roque** : petit et grand roque, avec vérification que le roi et la tour n'ont pas bougé, que le roi n'est pas en échec et que les cases traversées ne sont pas attaquées.
- **La prise en passant**.
- **La promotion** : Dame automatiquement pour l'IA, choix interactif Dame/Tour/Cavalier/Fou pour le joueur (l'IA envisage toutes les promotions pendant sa recherche).
- **Saisie** par coordonnées (`4 1 4 3`) ou en notation `e2e4`.

### 🖱️ Interface graphique (Windows, web, Android, iOS)
Tout se fait au doigt ou à la souris, avec les mêmes fonctions sur toutes les plateformes :
- **Jouer un coup** : cliquer sur une pièce puis sur sa case d'arrivée, ou la faire glisser. Les cases possibles sont marquées d'un point (d'un anneau pour une prise).
- **Promotion** : un petit menu propose Dame, Tour, Fou ou Cavalier.
- **Menu Partie** : nouvelle partie avec les Blancs ou les Noirs, annuler le dernier coup, tourner l'échiquier.
- **Menu Niveau** : 5 niveaux, de Débutant à Maximum (3 s de réflexion par coup).
- Le dernier coup est surligné, le roi en échec apparaît en rouge, et la fin de partie (mat, pat, répétition, 50 coups, matériel insuffisant) est annoncée.
- On peut aussi démarrer d'une position donnée : `WakaChess.exe "<FEN>"` sous Windows, `?fen=<FEN>` dans l'adresse de la version web.

### 🤖 Intelligence artificielle
- **Negamax alpha-bêta** avec **recherche à fenêtre nulle (PVS)** et **approfondissement itératif** avec fenêtres d'aspiration.
- **Table de transposition** (hachage Zobrist incrémental) : les positions déjà analysées ne sont pas recalculées.
- **Élagages** : coup nul (*null move pruning*), élagage statique (*reverse futility pruning*), élagage des coups tardifs (*late move pruning*).
- **Réductions des coups tardifs (LMR)** et **extension d'échec**.
- **Tri des coups** : coup de la table de transposition, captures (MVV-LVA), coups « tueurs » (*killer moves*), historique des coups calmes.
- **Quiescence** : prolonge la recherche sur les captures et promotions, avec élagage delta.
- **Évaluation PeSTO** : matériel et position de chaque pièce, interpolés entre milieu et fin de partie.
- **Nulles détectées pendant la recherche** (répétition, règle des 50 coups) : l'IA ne gâche plus une position gagnante en répétant les coups, et cherche la nulle quand elle est perdue.

### 🔌 Protocole UCI
WakaChess parle le protocole **UCI** : on peut le charger dans une interface graphique (Arena, Cute Chess, BanksiaGUI…) ou le faire jouer contre d'autres moteurs. Il suffit d'indiquer `wakachess` (ou `WakaChess-console.exe` sous Windows) comme moteur UCI : il passe automatiquement en mode UCI quand l'interface envoie la commande `uci`.

En mode UCI, l'IA gère sa pendule : elle approfondit sa recherche petit à petit (profondeur 1, 2, 3…) et s'arrête quand le temps alloué au coup est écoulé. Option disponible : `Hash` (taille de la table de transposition en Mo, 16 par défaut).

### ⚖️ Arbitrage
- Échec, échec et mat, pat.
- Nulle par triple répétition (hachage Zobrist des positions).
- Nulle par la règle des 50 coups.

---

## 📲 Installer WakaChess

| Plateforme | Comment jouer |
|---|---|
| **Toutes** (Android, iOS, Windows, macOS, Linux) | Ouvrir la **version web** dans le navigateur, puis l'installer comme une application (voir ci-dessous). Elle fonctionne ensuite **hors ligne**. |
| **Android** | Version web installée, ou fichier **`WakaChess.apk`** (voir « Fichiers compilés »). |
| **iOS / iPadOS** | Version web dans Safari → bouton **Partager** → **« Sur l'écran d'accueil »**. |
| **Windows** | [bin/WakaChess.exe](bin/WakaChess.exe) : interface graphique, double-clic et c'est parti. |
| **macOS (Apple Silicon)** | Version web (Safari → Fichier → **« Ajouter au Dock »**), ou moteur console/UCI `wakachess-macos-arm64`. |
| **Linux** | Version web, ou moteur console/UCI : [bin/linux-x64/wakachess](bin/linux-x64/wakachess), [bin/linux-arm64/wakachess](bin/linux-arm64/wakachess) (Raspberry Pi…). |

### Version web (toutes plateformes)
Le moteur C++ est compilé en **WebAssembly** : c'est exactement la même IA que les versions natives, qui tourne dans le navigateur, sans serveur. Les fichiers sont dans [docs/](docs).

**Mise en ligne (une seule fois)** : sur GitHub, *Settings → Pages → Build and deployment* : *Source* = **Deploy from a branch**, *Branch* = **main** et dossier **/docs**. Le jeu est alors disponible à l'adresse `https://wakadu13.github.io/WakaChess/`.

**Installation comme une application** :
- **Android** (Chrome) : menu ⋮ → **« Installer l'application »** (ou « Ajouter à l'écran d'accueil »).
- **iPhone / iPad** (Safari) : bouton **Partager** → **« Sur l'écran d'accueil »**.
- **Windows / Linux** (Chrome, Edge) : icône d'installation dans la barre d'adresse.
- **Mac** (Safari) : menu **Fichier → « Ajouter au Dock »**.

Pour essayer en local : `python3 -m http.server -d docs 8000`, puis ouvrir `http://localhost:8000`. Pour partir d'une position : `…/index.html?fen=<FEN>`.

### Fichiers compilés (Android, Mac, Linux, Windows)
Le dépôt compile automatiquement toutes les versions avec **GitHub Actions** ([.github/workflows/compilation.yml](.github/workflows/compilation.yml)) :
- onglet **Actions** → dernière exécution de « Compilation » → section **Artifacts** : `wakachess-android` (APK), `wakachess-macos-arm64`, `wakachess-linux`, `wakachess-windows`, `wakachess-web` ;
- en poussant une étiquette (ex. `git tag v2.0 && git push origin v2.0`), une **Release** GitHub est créée avec tous ces fichiers en téléchargement direct.

**Installer l'APK Android** : le télécharger sur le téléphone, l'ouvrir et autoriser l'installation d'applications de cette source. L'application embarque la version web et fonctionne hors ligne.

**Mac** : après téléchargement, `chmod +x wakachess-macos-arm64`, puis au premier lancement clic droit → Ouvrir (le fichier n'est pas signé par Apple).

### Windows (sans rien installer)
Deux exécutables autonomes (64 bits, aucune DLL ni installation nécessaire) :

| Fichier | Rôle |
|---|---|
| [bin/WakaChess.exe](bin/WakaChess.exe) | **Interface graphique** : double-clique dessus et joue à la souris. |
| [bin/WakaChess-console.exe](bin/WakaChess-console.exe) | Version console, et moteur **UCI** à charger dans Arena, Cute Chess… |

### Compiler soi-même
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
make            # moteur console/UCI pour cette machine
make linux      # exécutables Linux statiques x64 et ARM64 (bin/linux-*)
make windows    # exécutables Windows (MinGW-w64)
make web        # version WebAssembly dans docs/ (Emscripten)
./wakachess
```

L'application Android se compile avec Gradle 8.9 et le SDK Android : `gradle -p android assembleRelease`.

`-march=native` est activé par défaut ; pour un binaire portable : `cmake -B build -DWAKACHESS_NATIVE=OFF`.

### Options
| Commande | Rôle |
|---|---|
| `./wakachess` | Lance une partie contre l'IA. |
| `./wakachess --uci` | Mode UCI, pour une interface graphique (Arena, Cute Chess, BanksiaGUI…) ou des matchs entre moteurs. |
| `./wakachess --perft N [FEN]` | Compte les positions jusqu'à la profondeur N (validation du générateur de coups). |
| `./wakachess --bench P` | Mesure la réflexion de l'IA à la profondeur P depuis la position initiale. |

---

## 🧩 Fonctionnement

| Fichier | Rôle |
|---|---|
| [src/board.hpp](src/board.hpp) / [src/board.cpp](src/board.cpp) | Plateau, état de la partie (trait, droits au roque, case en passant, compteur 50 coups), génération des coups, cases attaquées, jouer/annuler un coup, perft. |
| [src/search.hpp](src/search.hpp) / [src/search.cpp](src/search.cpp) | Évaluation PeSTO, table de transposition, recherche (PVS, élagages, réductions, quiescence), niveaux de difficulté. |
| [src/main.cpp](src/main.cpp) | Boucle de jeu, saisie, promotion, fin de partie et nulles. |
| [src/uci.hpp](src/uci.hpp) / [src/uci.cpp](src/uci.cpp) | Protocole UCI : position, go (pendule, profondeur, temps fixe, nœuds), stop, option Hash. |
| [src/gui_win32.cpp](src/gui_win32.cpp) | Interface graphique Windows (Win32/GDI, sans bibliothèque externe). |
| [src/web_api.cpp](src/web_api.cpp) | Fonctions exposées à JavaScript pour la version WebAssembly. |
| [docs/](docs) | Version web / PWA : interface tactile et souris (`app.js`), IA dans un Web Worker (`moteur.js`), fonctionnement hors ligne (`sw.js`). |
| [android/](android) | Application Android (WebView qui embarque la version web). |
| [outils/](outils) | Scripts de mesure de l'Elo (matchs avec fastchess contre Stockfish bridé). |

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

## 📈 Niveau

Mesuré à la cadence 10 s + 0,1 s par coup contre Stockfish 16 bridé (mode `UCI_Elo`, calibré sur la liste CCRL des moteurs), 40 parties par niveau, chaque ouverture jouée avec les deux couleurs :

| Version | Adversaires et scores | Elo estimé |
|---|---|---|
| Traduction du Python (étape 1) | SF1320 90 % · SF1600 51 % · SF1900 45 % · SF2200 21 % | **~1780** (1700 – 1850) |
| Recherche moderne + PeSTO (étape 2) | SF2000 85 % · SF2300 62 % · SF2600 41 % · SF2900 14 % | **~2460** (2390 – 2530) |

Ces Elo sont sur l'échelle des moteurs, pas sur celle des joueurs humains FIDE. Les intervalles sont à 95 %.

Pour refaire la mesure : `outils/mesurer_elo.sh ./wakachess 8moves_v3.pgn` (nécessite [fastchess](https://github.com/Disservin/fastchess), Stockfish et un [livre d'ouvertures](https://github.com/official-stockfish/books)).

---

## 🐞 Corrections par rapport à la version Python
- **Prise en passant** : la case en passant était une variable globale copiée par `from plateau import *`, jamais mise à jour dans `geo.py` ; la prise en passant n'était donc jamais proposée.
- **Mats invisibles pour l'IA** : quand il n'y avait plus de coup légal, la recherche renvoyait l'évaluation matérielle au lieu d'un score de mat ou de nulle.
- **Table des pions inversée** : la PST était lue à l'envers, ce qui récompensait les pions restés sur leur case de départ.
- **Recherche** : le roque, la promotion et les droits au roque sont maintenant gérés correctement pendant la simulation des coups.

---

## 🗺️ Pistes d'amélioration
- **Réseau de neurones NNUE** entraîné sur des parties du moteur contre lui-même (étape 3, objectif 3000).
- **Bitboards** pour une génération de coups plus rapide.
- **SEE** (évaluation statique des échanges) pour mieux trier et élaguer les captures.
- **Sauvegarde / chargement de partie** (PGN).
