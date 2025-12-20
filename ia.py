# ia.py
from geo import *
from plateau import *

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
            if piece == 'C': # Cavalier Blanc
                score += PST_CAVALIER[y][x]
            elif piece == 'c': # Cavalier Noir
                score -= PST_CAVALIER[7-y][x] # Miroir pour les noirs
            
            if piece == 'P': # Pion Blanc
                score += PST_PION[y][x]
            elif piece == 'p': # Pion Noir
                score -= PST_PION[7-y][x]
            # Bonus pour le roque (encourager l'IA à mettre son roi à l'abri)
    # On vérifie si le roi n'est plus sur sa case de départ (souvent signe de roque)
    if plateau[0][6] == 'R' or plateau[0][2] == 'R': score += 40
    if plateau[7][6] == 'r' or plateau[7][2] == 'r': score -= 40

    return score if camp_ia_est_maj else -score

def trier_coups(plateau, coups):
    """
    Trie les coups pour l'élagage Alpha-Beta : les captures d'abord !
    """
    score_coups = []
    for posInit, posFut in coups:
        x_fut, y_fut = posFut
        piece_cible = plateau[y_fut][x_fut]
        
        # Si on mange une pièce, on donne une haute priorité
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

    coups_legaux = trier_coups(plateau, coups_bruts)

    if not coups_legaux:
        return None, 0

    meilleur_score = float('-inf')
    meilleur_coup = coups_legaux[0]
    alpha = float('-inf')
    beta = float('inf')

    for posInit, posFut in coups_legaux:
        # --- DO (Faire le coup) ---
        # On utilise la version importée "rapide"
        piece_capturee = faire_coup_rapide(plateau, posInit, posFut)
        
        # Appel récursif
        score = -negamax(plateau, profondeur - 1, -beta, -alpha, not camp_ia_est_maj)
        
        # --- UNDO (Défaire le coup) ---
        defaire_coup_rapide(plateau, posInit, posFut, piece_capturee)

        if score > meilleur_score:
            meilleur_score = score
            meilleur_coup = (posInit, posFut)
        
        alpha = max(alpha, score)
        if alpha >= beta:
            break

    return meilleur_coup, meilleur_score

def negamax(plateau, profondeur, alpha, beta, est_camp_maj):
    """
    Algorithme Negamax avec Alpha-Beta et Recherche de Repos.
    """
    if profondeur == 0:
        return recherche_repos(plateau, alpha, beta, est_camp_maj)

    if est_camp_maj:
        coups = ensembleCoupsLegauxMaj(plateau)
    else:
        coups = ensembleCoupsLegauxMin(plateau)

    if not coups:
        return evaluer_plateau(plateau, est_camp_maj)

    coups = trier_coups(plateau, coups)
    
    max_eval = float('-inf')

    for posInit, posFut in coups:
        # DO
        piece_capturee = faire_coup_rapide(plateau, posInit, posFut)
        
        # RECURSION
        score = -negamax(plateau, profondeur - 1, -beta, -alpha, not est_camp_maj)
        
        # UNDO
        defaire_coup_rapide(plateau, posInit, posFut, piece_capturee)

        max_eval = max(max_eval, score)
        alpha = max(alpha, score)
        if alpha >= beta:
            break
            
    return max_eval

def recherche_repos(plateau, alpha, beta, est_camp_maj):
    """
    Quiescence Search.
    """
    stand_pat = evaluer_plateau(plateau, est_camp_maj)
    
    if stand_pat >= beta:
        return beta
    if alpha < stand_pat:
        alpha = stand_pat

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
        piece_capturee = faire_coup_rapide(plateau, posInit, posFut)
        
        # RECURSION
        score = -recherche_repos(plateau, -beta, -alpha, not est_camp_maj)
        
        # UNDO
        defaire_coup_rapide(plateau, posInit, posFut, piece_capturee)

        if score >= beta:
            return beta
        if score > alpha:
            alpha = score
            
    return alpha