# WakaChess
♟️ WakaChess - Moteur d'Échecs en Python

**WakaChess** est un jeu d'échecs complet développé en Python, permettant de jouer contre une Intelligence Artificielle performante. Le projet intègre les règles officielles de la FIDE et un moteur de réflexion basé sur des algorithmes de théorie des jeux.

## 🚀 Fonctionnalités

### 🎮 Gameplay
- **Règles Complètes** : Déplacements standards, double pas des pions, et captures.
- **Le Roque** : Gestion du petit et du grand roque (vérification de l'immobilité des pièces et des cases attaquées).
- **Promotion** : Promotion automatique pour l'IA et choix interactif pour le joueur lorsqu'un pion atteint la dernière rangée.
- **Système de Tour** : Gestion alternée entre les Majuscules (Blancs) et les Minuscules (Noirs).

### 🤖 Intelligence Artificielle
- **Algorithme Negamax** : Une variante optimisée du Minimax.
- **Élagage Alpha-Beta** : Optimisation drastique de la vitesse de calcul en ignorant les branches de jeu futiles.
- **Quiescence Search** : Recherche de repos pour éviter l'effet d'horizon (ne s'arrête pas au milieu d'une capture).
- **PST (Piece Square Tables)** : L'IA évalue les pièces selon leur position (ex: les Cavaliers sont plus forts au centre).

### ⚖️ Arbitrage
- **Détection d'Échec et Mat / Pat**.
- **Nulle par répétition** : Détecte si la même position se produit 3 fois.
- **Règle des 50 coups** : Empêche les parties infinies sans capture ou mouvement de pion.

---

## 🛠️ Installation et Lancement

1. **Prérequis** : Avoir Python 3.8+ installé.
2. **Cloner le projet** :
   ```bash
   git clone [https://github.com/wakadu13/WakaChess.git](https://github.com/wakadu13/WakaChess.git)
   cd WakaChess
