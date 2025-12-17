# ia.py
#Ce fichier contient l'implémentation de l'IA utilisant l'algorithme Minimax avec élagage Alpha-Beta
#J'ai utilsé l'ia pour le générer
#L'IA évalue les positions en fonction de la valeur matérielle des pièces
#L'IA choisit le coup qui maximise son avantage tout en minimisant celui de l'adversaire
#L'IA peut jouer pour les deux camps (majuscules ou minuscules)
#L'IA utilise une profondeur de recherche ajustable pour équilibrer performance et temps de calcul
from geo import CoupLegal, ensembleCoupsLegauxMin, ensembleCoupsLegauxMaj
from plateau import CopiePlateau, deplacer_piece_physique
import random # Pour choisir un coup aléatoire au début si l'évaluation est nulle

VALEURS_PIECES = {
    'p': -100, 'c': -320, 'f': -330, 't': -500, 'd': -900, 'r': -20000,
    'P': 100, 'C': 320, 'F': 330, 'T': 500, 'D': 900, 'R': 20000,
    '.': 0
}

def evaluer_plateau(plateau, camp_ia_est_maj):
    """Calcule le score du plateau."""
    score = 0
    for ligne in plateau:
        for piece in ligne:
            score += VALEURS_PIECES.get(piece, 0)
            
    # Si l'IA est le camp Majuscule (Blancs), elle préfère un score positif.
    # Si l'IA est le camp Minuscule (Noirs), elle préfère un score négatif.
    return score if camp_ia_est_maj else -score


def trouver_meilleur_coup(plateau, profondeur, camp_ia_est_maj):
    """Lance l'algorithme Minimax pour choisir le meilleur coup."""
    
    # Choisit la fonction pour générer les coups légaux de l'IA
    if camp_ia_est_maj:
        coups_legaux = ensembleCoupsLegauxMaj(plateau)
    else:
        coups_legaux = ensembleCoupsLegauxMin(plateau)

    if not coups_legaux:
        return None, 0

    meilleur_score = float('-inf') if camp_ia_est_maj else float('inf')
    meilleur_coup = random.choice(coups_legaux) # Sécurité si tous les scores sont égaux

    # On explore chaque coup légal
    for posInit, posFut in coups_legaux:
        
        # 1. Simuler le mouvement
        temp_plateau = CopiePlateau(plateau)
        piece = temp_plateau[posInit[1]][posInit[0]]
        deplacer_piece_physique(posInit, posFut, piece, temp_plateau)
        
        # 2. Appeler Minimax (l'IA cherche l'opposé de ce que fera l'adversaire)
        # Note : On inverse 'camp_ia_est_maj' pour simuler le tour de l'adversaire
        score = minimax(temp_plateau, profondeur - 1, float('-inf'), float('inf'), False, camp_ia_est_maj) 
        
        # 3. Mise à jour du meilleur coup
        if score > meilleur_score:
            meilleur_score = score
            meilleur_coup = (posInit, posFut)

    return meilleur_coup, meilleur_score

def minimax(plateau, profondeur, alpha, beta, est_max, camp_ia_est_maj):
    """Algorithme Minimax avec élagage Alpha-Beta."""
    
    # 1. Condition d'arrêt : Profondeur atteinte ou fin de partie (échec/mat/pat)
    if profondeur == 0:
        return evaluer_plateau(plateau, camp_ia_est_maj)
    # (Implémenter ici la vérification d'échec et mat pour donner un score infini)
    
    # Le camp MAX est l'IA, le camp MIN est l'adversaire
    if est_max:
        # L'IA (MAX) cherche le coup qui maximise son score
        coups_legaux = ensembleCoupsLegauxMaj(plateau) if camp_ia_est_maj else ensembleCoupsLegauxMin(plateau)
        
        max_eval = float('-inf')
        for posInit, posFut in coups_legaux:
            temp_plateau = CopiePlateau(plateau)
            piece = temp_plateau[posInit[1]][posInit[0]]
            deplacer_piece_physique(posInit, posFut, piece, temp_plateau)
            
            evaluation = minimax(temp_plateau, profondeur - 1, alpha, beta, False, camp_ia_est_maj)
            max_eval = max(max_eval, evaluation)
            alpha = max(alpha, max_eval)
            if beta <= alpha:
                break # Élagage Beta
        return max_eval
    else:
        # L'adversaire (MIN) cherche le coup qui minimise le score de l'IA
        coups_legaux = ensembleCoupsLegauxMin(plateau) if camp_ia_est_maj else ensembleCoupsLegauxMaj(plateau)
        
        min_eval = float('inf')
        for posInit, posFut in coups_legaux:
            temp_plateau = CopiePlateau(plateau)
            piece = temp_plateau[posInit[1]][posInit[0]]
            deplacer_piece_physique(posInit, posFut, piece, temp_plateau)

            evaluation = minimax(temp_plateau, profondeur - 1, alpha, beta, True, camp_ia_est_maj)
            min_eval = min(min_eval, evaluation)
            beta = min(beta, min_eval)
            if beta <= alpha:
                break # Élagage Alpha
        return min_eval

