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
    """
    Applique le coup directement sur la liste 'plateau'.
    Retourne la pièce qui a été mangée (ou '.' si vide).
    """
    x1, y1 = posInit
    x2, y2 = posFut
    
    # 1. On sauvegarde la pièce qui va se faire écraser (pour pouvoir annuler après)
    piece_capturee = plateau[y2][x2]
    
    # 2. On déplace la pièce
    piece_deplacee = plateau[y1][x1]
    plateau[y2][x2] = piece_deplacee
    
    # 3. On vide la case de départ
    plateau[y1][x1] = '.'
    
    # (Optionnel : Gestion basique de la promotion du pion)
    # Si c'est un pion blanc (P) qui arrive en ligne 0 ou noir (p) en ligne 7
    # Tu pourrais ajouter ici la transformation en Reine.
    
    return piece_capturee

def defaire_coup_rapide(plateau, posInit, posFut, piece_capturee):
    """
    Annule le coup précédent pour remettre le plateau dans son état original.
    """
    x1, y1 = posInit
    x2, y2 = posFut
    
    # 1. On récupère la pièce qui a bougé (elle est actuellement en posFut)
    piece_deplacee = plateau[y2][x2]
    
    # 2. On la remet à sa position de départ
    plateau[y1][x1] = piece_deplacee
    
    # 3. On remet la pièce capturée (ou le point '.') à sa place
    plateau[y2][x2] = piece_capturee