# WakaChess
♟️ WakaChess - Moteur d'Échecs en Python

**WakaChess** est un jeu d'échecs développé en Python, jouable en console contre une Intelligence Artificielle. Le projet implémente les règles principales des échecs (déplacements, roque, prise en passant, promotion, échec/mat, pat, nulles) ainsi qu'un moteur de réflexion basé sur l'algorithme Negamax.

## 🚀 Fonctionnalités

### 🎮 Gameplay
- **Règles de base** : déplacements standards de chaque pièce, double pas des pions, captures.
- **Le roque** : petit et grand roque, avec vérification que le roi et la tour n'ont pas bougé et que les cases traversées ne sont pas attaquées.
- **La prise en passant**.
- **La promotion** : un pion qui atteint la dernière rangée est promu (Dame automatiquement pour l'IA, choix interactif Dame/Tour/Cavalier/Fou pour le joueur).
- **Système de tour** : alternance entre les Majuscules (Blancs) et les Minuscules (Noirs).

### 🤖 Intelligence artificielle
- **Algorithme Negamax** : variante du Minimax adaptée aux jeux à somme nulle à deux joueurs.
- **Élagage Alpha-Bêta** : réduit le nombre de branches explorées.
- **Quiescence Search** : prolonge la recherche sur les séquences de captures pour éviter l'effet d'horizon.
- **Piece-Square Tables (PST)** : bonus positionnel pour les pions et les cavaliers (ex. cavalier mieux valorisé au centre).
- **Tri des coups** : les captures sont évaluées en priorité pour améliorer l'efficacité de l'élagage.

### ⚖️ Arbitrage
- Détection de l'échec, de l'échec et mat, et du pat.
- Nulle par triple répétition de position.
- Nulle par la règle des 50 coups (sans capture ni mouvement de pion).

---

## 🧩 Fonctionnement

Le projet est découpé en 5 fichiers Python, sans dépendance externe :

| Fichier | Rôle |
|---|---|
| [main.py](main.py) | Point d'entrée, boucle de jeu, saisie des coups, application d'un coup (`changementPosition`), promotion, conditions de nulle. |
| [plateau.py](plateau.py) | Représentation du plateau et primitives de déplacement bas niveau : génération du plateau, affichage, copie, exécution/annulation rapide d'un coup (`faire_coup_rapide` / `defaire_coup_rapide`, utilisées par l'IA pour simuler), exécution complète d'un coup avec gestion du roque et des droits au roque (`executer_mouvement_complet`). |
| [geo.py](geo.py) | Génération des coups possibles pour chaque type de pièce, détection des cases attaquées, détection d'échec, calcul de l'ensemble des coups légaux, détection de fin de partie (mat/pat). |
| [ia.py](ia.py) | Évaluation de position (matériel + PST), Negamax avec élagage alpha-bêta, quiescence search, point d'entrée `trouver_meilleur_coup`. |
| [utilitaire.py](utilitaire.py) | Fonctions utilitaires : parsing d'une commande joueur, validation de coordonnées. |

### Représentation du plateau

Le plateau est une liste de 8 listes de 8 caractères, `plateau[y][x]`, avec `y=0` la rangée de départ des Blancs et `y=7` celle des Noirs. Une case vide est représentée par `"."`. Les pièces Blanches (Majuscules) sont `P T C F R D` (Pion, Tour, Cavalier, Fou, Roi, Dame) ; les pièces Noires (minuscules) sont `p t c f r d`.

Particularité de ce projet : la rangée de départ est `T C F R D F C T`, donc le Roi démarre en `x=3` et la Dame en `x=4` (inversé par rapport à la disposition standard où le roi est en `e1`/`x=4`).

### Boucle de jeu

1. Au lancement, `main()` génère le plateau, demande au joueur son camp (`M` = Majuscules/Blancs, `m` = Minuscules/Noirs) et le niveau de difficulté (1 à 5, converti en profondeur de recherche pour l'IA).
2. À chaque tour, le camp dont c'est le tour est déterminé par la parité de `nbTour` (pair = Majuscules, impair = Minuscules).
3. Si c'est à l'IA de jouer, `trouver_meilleur_coup` calcule le meilleur coup ; sinon le joueur saisit une commande au format `x_depart y_depart x_arrivee y_arrivee` (coordonnées entre 0 et 7), ou `stop` pour quitter.
4. Le coup est appliqué via `changementPosition`, qui vérifie sa légalité, gère la prise en passant, le compteur des 50 coups, la promotion, puis délègue l'exécution à `executer_mouvement_complet`.
5. Après chaque coup, `finDePartie` vérifie l'échec et mat / le pat, et le programme vérifie la triple répétition et la règle des 50 coups.

---

## 🛠️ Installation et lancement

**Prérequis** : Python ≥ 3.14 (voir [pyproject.toml](pyproject.toml)). Le projet n'a aucune dépendance externe.

```bash
git clone https://github.com/wakadu13/WakaChess.git
cd WakaChess
```

Avec [uv](https://docs.astral.sh/uv/) (recommandé, gère l'environnement automatiquement) :
```bash
uv run python main.py
```

Ou directement avec Python :
```bash
python main.py
```

---

## 🐞 Bugs corrigés

Le projet ne pouvait pas être lancé en l'état avant les corrections suivantes :
- **Plantage immédiat au lancement** : `ia.py` contenait deux définitions de `trouver_meilleur_coup`, la seconde (une ébauche inachevée) écrasant la première et appelant une fonction inexistante. `main.py` l'appelait en plus avec un nombre d'arguments incorrect dès le premier tour. → suppression de l'ébauche cassée et de l'appel mort.
- **Roque inopérant** : le roi part de `x=3` sur ce plateau (et non `x=4`), mais le déplacement de la tour lors du roque était codé pour un roi en `x=4`. La tour ne bougeait donc jamais. → coordonnées de roque corrigées dans `plateau.py`.
- **Choix du camp inversé** : la comparaison du choix du joueur utilisait `.lower()`, rendant `M` et `m` indistinguables ; l'IA jouait donc toujours les Blancs quel que soit le choix saisi. → comparaison corrigée pour respecter la casse.
- **Promotion absente** : alors qu'elle était annoncée, aucun code ne promouvait les pions arrivés en dernière rangée. → promotion implémentée (automatique en Dame pour l'IA, choix interactif pour le joueur).
- **Droits au roque trop permissifs** : ils étaient révoqués sur la seule base des coordonnées de départ, sans vérifier que la pièce déplacée était bien une tour. → vérification du type de pièce ajoutée.

---

## 🗺️ Pistes d'amélioration

- **Tests automatisés** : aucun test n'existe actuellement ; un jeu de tests (pytest) sur la génération de coups, l'échec/mat, le roque, la prise en passant et la promotion sécuriserait les évolutions futures.
- **Nettoyage du code mort** : `verifier_nulle()` dans `main.py` n'est jamais appelée (la logique de répétition est dupliquée en ligne), et `historique_positions` est défini à la fois dans `plateau.py` et `main.py` sans lien entre les deux.
- **Notation algébrique** : remplacer la saisie par coordonnées brutes (`3 6 3 4`) par une notation type `e2e4`, plus naturelle pour un joueur d'échecs.
- **Interface graphique** : une interface (ex. `pygame` ou une interface web) rendrait le jeu plus accessible que le mode console actuel.
- **Recherche à temps limité** : implémenter un véritable *iterative deepening* borné dans le temps (une tentative inachevée existait déjà dans `ia.py`), plutôt qu'une profondeur fixe choisie au démarrage.
- **Table de transposition** : mémoriser les positions déjà évaluées accélérerait significativement la recherche de l'IA.
- **Sauvegarde / chargement de partie** (export PGN) et fonction d'annulation du dernier coup pour le joueur.
- **Validation des entrées** plus robuste (actuellement, une saisie malformée lève une exception interceptée de façon générique).
