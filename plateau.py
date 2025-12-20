droits_roque = {
    'R': True, 'T_gauche': True, 'T_droite': True, # Majuscules (Blancs)
    'r': True, 't_gauche': True, 't_droite': True  # Minuscules (Noirs)
}
# Dans plateau.py
case_en_passant = None  # Stockera (x, y) de la case derrière le pion ayant sauté
def generationPlateau(): #Génération du plateau de jeu
    Pion = ['P' for i in range(8)]
    pion = ['p' for i in range(8)]
    Pieces = ["T","C","F","R","D","F","C","T"]
    pieces = ["t", "c", "f", "r", "d", "f", "c", "t"]

    plateau = []
    plateau.append(Pieces)
    plateau.append(Pion)
    for i in range(4):
        plateau.append(['.', '.', '.','.','.','.','.','.'])
    plateau.append(pion)
    plateau.append(pieces)

    return plateau
def deplacer_piece_physique(posInit, posFut, piece, plateau):
    """Effectue le mouvement SANS aucune vérification de légalité ou d'échec.
    Utilile pour la modularité du code."""
    plateau[posFut[1]][posFut[0]] = piece
    plateau[posInit[1]][posInit[0]] = "."


def affichagePlateau(plateau): #Affichage du plateau de jeu
    print("  ",end="")
    for i in range(8):
        print(" ",i,end="")
    print()
    i = 0
    for ligne in plateau:
        print(" ",end="")
        print(i,end="")
        i += 1
        for elmt in ligne:
            print(" ",elmt,end="")
        print()

def CopiePlateau(plateau):# Copie le plateau pour simuler des déplacements
    newPlateau = []
    for ligne in plateau:
        newLigne = []
        for elmt in ligne:
            newLigne.append(elmt)
        newPlateau.append(newLigne)
    return newPlateau

# Fichier: plateau.py

def faire_coup_rapide(plateau, posInit, posFut):
    x1, y1 = posInit
    x2, y2 = posFut
    piece_deplacee = plateau[y1][x1]
    piece_capturee = plateau[y2][x2]
    
    # --- LOGIQUE SPÉCIFIQUE AU ROQUE ---
    if piece_deplacee.upper() == 'R' and abs(x2 - x1) == 2:
        # C'est un roque : on déplace la tour manuellement
        ligne = y1
        if x2 == 6: # Petit roque (vers la droite)
            plateau[ligne][5] = plateau[ligne][7]
            plateau[ligne][7] = '.'
        elif x2 == 2: # Grand roque (vers la gauche)
            plateau[ligne][3] = plateau[ligne][0]
            plateau[ligne][0] = '.'

    # Déplacement normal du Roi (ou de n'importe quelle pièce)
    plateau[y2][x2] = piece_deplacee
    plateau[y1][x1] = '.'
    
    return piece_capturee

def defaire_coup_rapide(plateau, posInit, posFut, piece_capturee):
    x1, y1 = posInit
    x2, y2 = posFut
    piece_deplacee = plateau[y2][x2]
    
    # --- ANNULATION DU ROQUE ---
    if piece_deplacee.upper() == 'R' and abs(x2 - x1) == 2:
        ligne = y1
        if x2 == 6: # Annuler petit roque
            plateau[ligne][7] = plateau[ligne][5]
            plateau[ligne][5] = '.'
        elif x2 == 2: # Annuler grand roque
            plateau[ligne][0] = plateau[ligne][3]
            plateau[ligne][3] = '.'

    plateau[y1][x1] = piece_deplacee
    plateau[y2][x2] = piece_capturee

def executer_mouvement_complet(plateau, posInit, posFut, droits_roque):
    x_i, y_i = posInit
    x_f, y_f = posFut
    piece = plateau[y_i][x_i]

    # --- LOGIQUE SPÉCIFIQUE AU ROQUE ---
    if piece.upper() == 'R' and abs(x_f - x_i) == 2:
        # Petit Roque
        if x_f == 6:
            plateau[y_f][5] = plateau[y_f][7] # La tour bouge en f1/f8
            plateau[y_f][7] = "."
        # Grand Roque
        elif x_f == 2:
            plateau[y_f][3] = plateau[y_f][0] # La tour bouge en d1/d8
            plateau[y_f][0] = "."

    # --- MISE À JOUR DES DROITS AU ROQUE ---
    # Si le roi bouge
    if piece == 'R': droits_roque['R'] = False
    if piece == 'r': droits_roque['r'] = False
    
    # Si une tour bouge ou est capturée
    if (x_i == 0 and y_i == 0) or (x_f == 0 and y_f == 0): droits_roque['T_gauche'] = False
    if (x_i == 7 and y_i == 0) or (x_f == 7 and y_f == 0): droits_roque['T_droite'] = False
    if (x_i == 0 and y_i == 7) or (x_f == 0 and y_f == 7): droits_roque['t_gauche'] = False
    if (x_i == 7 and y_i == 7) or (x_f == 7 and y_f == 7): droits_roque['t_droite'] = False

    # Enfin, on déplace le roi normalement
    plateau[y_f][x_f] = piece
    plateau[y_i][x_i] = "."