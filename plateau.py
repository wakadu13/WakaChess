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
