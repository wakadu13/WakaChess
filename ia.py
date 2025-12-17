# ia.py
from geo import ensembleCoupsLegauxMin, ensembleCoupsLegauxMaj
import random

# --- 1. CONFIGURATION DES VALEURS ET TABLES (PST) ---

VALEURS_PIECES = {
    'p': -100, 'c': -320, 'f': -330, 't': -500, 'd': -900, 'r': -20000,
    'P': 100, 'C': 320, 'F': 330, 'T': 500, 'D': 900, 'R': 20000,
    '.': 0
}

# Bonus positionnel pour les Cavaliers (favorise le centre)
PST_CAVALIER = [
    [-50,-40,-30,-30,-30,-30,-40,-50],
    [-40,-20,  0,  0,  0,  0,-20,-40],
    [-30,  0, 10, 15, 15, 10,  0,-30],
    [-30,  5, 15, 20, 20, 15,  5,-30],
    [-30,  0, 15, 20, 20, 15,  0,-30],
    [-30,  5, 10, 15, 15, 10,  5,-30],
    [-40,-20,  0,  5,  5,  0,-20,-40],
    [-50,-40,-30,-30,-30,-30,-40,-50]
]

# Bonus pour les Pions (favorise l'avancée)
PST_PION = [
    [ 0,  0,  0,  0,  0,  0,  0,  0],
    [50, 50, 50, 50, 50, 50, 50, 50],
    [10, 10, 20, 30, 30, 20, 10, 10],
    [ 5,  5, 10, 25, 25, 10,  5,  5],
    [ 0,  0,  0, 20, 20,  0,  0,  0],
    [ 5, -5,-10,  0,  0,-10, -5,  5],
    [ 5, 10, 10,-20,-20, 10, 10,  5],
    [ 0,  0,  0,  0,  0,  0,  0,  0]
]

# --- 2. FONCTIONS UTILITAIRES D'ÉVALUATION ---

def evaluer_plateau(plateau, camp_ia_est_maj):
    """
    Calcule le score du plateau en combinant matériel et position.
    """
    score = 0
    for y in range(8):
        for x in range(8):
            piece = plateau[y][x]
            if piece == '.':
                continue
            
            # 1. Valeur Matérielle
            score += VALEURS_PIECES.get(piece, 0)
            
            # 2. Valeur Positionnelle (PST)
            # On inverse l'indice y pour les noirs si nécessaire selon votre logique de grille
            if piece == 'C': # Cavalier Blanc
                score += PST_CAVALIER[y][x]
            elif piece == 'c': # Cavalier Noir
                score -= PST_CAVALIER[7-y][x] # Miroir pour les noirs
            
            if piece == 'P': # Pion Blanc
                score += PST_PION[y][x]
            elif piece == 'p': # Pion Noir
                score -= PST_PION[7-y][x]

    return score if camp_ia_est_maj else -score

def trier_coups(plateau, coups):
    """
    Trie les coups pour l'élagage Alpha-Beta : les captures d'abord !
    C'est crucial pour la performance.
    """
    score_coups = []
    for posInit, posFut in coups:
        x_fut, y_fut = posFut
        piece_cible = plateau[y_fut][x_fut]
        
        # Si on mange une pièce, on donne une haute priorité (valeur absolue de la pièce)
        valeur_capture = 0
        if piece_cible != '.':
            valeur_capture = abs(VALEURS_PIECES.get(piece_cible, 0)) * 10
        
        score_coups.append(((posInit, posFut), valeur_capture))
    
    # On trie du plus fort au plus faible score
    score_coups.sort(key=lambda x: x[1], reverse=True)
    return [c[0] for c in score_coups]

# --- 3. MOTEUR MINIMAX OPTIMISÉ ---

def trouver_meilleur_coup(plateau, profondeur, camp_ia_est_maj):
    """Point d'entrée de l'IA."""
    if camp_ia_est_maj:
        coups_bruts = ensembleCoupsLegauxMaj(plateau)
    else:
        coups_bruts = ensembleCoupsLegauxMin(plateau)

    # Optimisation : Trier les coups pour regarder les captures en premier
    coups_legaux = trier_coups(plateau, coups_bruts)

    if not coups_legaux:
        return None, 0

    meilleur_score = float('-inf')
    meilleur_coup = coups_legaux[0] # Par défaut le premier coup trié
    alpha = float('-inf')
    beta = float('inf')

    for posInit, posFut in coups_legaux:
        # --- DO (Faire le coup) ---
        piece_capturee = faire_coup_virtuel(plateau, posInit, posFut)
        
        # Appel récursif
        score = -negamax(plateau, profondeur - 1, -beta, -alpha, not camp_ia_est_maj)
        
        # --- UNDO (Défaire le coup) ---
        defaire_coup_virtuel(plateau, posInit, posFut, piece_capturee)

        if score > meilleur_score:
            meilleur_score = score
            meilleur_coup = (posInit, posFut)
        
        alpha = max(alpha, score)
        if alpha >= beta:
            break

    return meilleur_coup, meilleur_score

def negamax(plateau, profondeur, alpha, beta, est_camp_maj):
    """
    Algorithme Negamax (variante simplifiée de Minimax) avec Alpha-Beta
    et Recherche de Repos (Quiescence).
    """
    # Si profondeur 0, on lance la recherche de repos pour éviter l'effet d'horizon
    if profondeur == 0:
        return recherche_repos(plateau, alpha, beta, est_camp_maj)

    # Génération des coups
    if est_camp_maj:
        coups = ensembleCoupsLegauxMaj(plateau)
    else:
        coups = ensembleCoupsLegauxMin(plateau)

    if not coups:
        # Vérifier échec et mat ici si possible, sinon retour eval simple
        return evaluer_plateau(plateau, est_camp_maj)

    coups = trier_coups(plateau, coups)
    
    max_eval = float('-inf')

    for posInit, posFut in coups:
        # DO
        piece_capturee = faire_coup_virtuel(plateau, posInit, posFut)
        
        # RECURSION (Note le signe moins et l'inversion alpha/beta)
        score = -negamax(plateau, profondeur - 1, -beta, -alpha, not est_camp_maj)
        
        # UNDO
        defaire_coup_virtuel(plateau, posInit, posFut, piece_capturee)

        max_eval = max(max_eval, score)
        alpha = max(alpha, score)
        if alpha >= beta:
            break # Élagage
            
    return max_eval

def recherche_repos(plateau, alpha, beta, est_camp_maj):
    """
    Quiescence Search : continue de chercher tant qu'il y a des captures.
    Évite que l'IA s'arrête au milieu d'un échange de pièces.
    """
    # 1. Évaluation "Stand-pat" (si on ne fait rien, quel est le score ?)
    stand_pat = evaluer_plateau(plateau, est_camp_maj)
    
    if stand_pat >= beta:
        return beta
    if alpha < stand_pat:
        alpha = stand_pat

    # 2. On ne regarde QUE les coups de capture
    if est_camp_maj:
        coups = ensembleCoupsLegauxMaj(plateau)
    else:
        coups = ensembleCoupsLegauxMin(plateau)
        
    for posInit, posFut in coups:
        x_fut, y_fut = posFut
        # Si ce n'est pas une capture, on ignore
        if plateau[y_fut][x_fut] == '.':
            continue
            
        # DO
        piece_capturee = faire_coup_virtuel(plateau, posInit, posFut)
        
        # RECURSION QUIESCENCE
        score = -recherche_repos(plateau, -beta, -alpha, not est_camp_maj)
        
        # UNDO
        defaire_coup_virtuel(plateau, posInit, posFut, piece_capturee)

        if score >= beta:
            return beta
        if score > alpha:
            alpha = score
            
    return alpha

# --- 4. GESTION PHYSIQUE DU PLATEAU (DO/UNDO) ---

def faire_coup_virtuel(plateau, posInit, posFut):
    """Applique un coup directement sur le tableau et retourne la pièce mangée."""
    x1, y1 = posInit
    x2, y2 = posFut
    piece = plateau[y1][x1]
    capture = plateau[y2][x2]
    
    plateau[y2][x2] = piece
    plateau[y1][x1] = '.'
    return capture

def defaire_coup_virtuel(plateau, posInit, posFut, capture):
    """Annule le coup et remet la pièce capturée."""
    x1, y1 = posInit
    x2, y2 = posFut
    piece = plateau[y2][x2]
    
    plateau[y1][x1] = piece
    plateau[y2][x2] = capture