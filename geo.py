#Geo.py permet de gérer la géométrie du jeu d'échecs c'est à dire les déplacements des pièces et la vérification des coups légaux

from plateau import *
  
def positionPossiblePion(x, y, plateau): #Determine les positions possibles pour un pion
    if plateau[y][x].isupper():#Détermine si la piece est une majuscule ou une minuscule
        estEnnemie = str.islower#Aide de l'ia pour trouver la fonctionnalité
        decallage = 1
        posDepart = 1
    elif plateau[y][x].islower():
        estEnnemie = str.isupper
        decallage = -1
        posDepart = 6
    possibilite = []
    #Fonctionnalités du double déplacement au premier coup
    if (0 <= y + 2*decallage <= 7) and y == posDepart and plateau[y + 2*decallage][x] == "." and  plateau[y + decallage][x] == ".":
        possibilite.append((x, y+2*decallage))
    #Déplacement simple
    if ((0 <= y + decallage <= 7)) and plateau[y + decallage][x] == ".":
        possibilite.append((x, y+decallage))
    #Prise en diagonale
    if ((0 <= x + 1 <= 7)) and ((0 <= y + decallage <= 7)) and estEnnemie(plateau[y + decallage][x + 1]):
        possibilite.append((x + 1, y + decallage))
    #Prise en diagonale
    if ((0 <= x - 1 <= 7)) and ((0 <= y + decallage <= 7)) and estEnnemie(plateau[y + decallage][x - 1]):
        possibilite.append((x - 1, y + decallage))
    return possibilite
    
def positionPossibleTour(x, y, plateau): #Permet de trouver les positions possibles pour une tour
    if plateau[y][x].isupper():
        estAllie = str.isupper #Aide de l'ia pour trouver la fonctionnalité
        estEnnemie = str.islower
    elif plateau[y][x].islower():
        estAllie = str.islower
        estEnnemie = str.isupper
    possibilite = []
    for i in range(x+1, 8): #Droite
        if estAllie(plateau[y][i]):
            break
        possibilite.append((i, y))
        if estEnnemie(plateau[y][i]):
            break
    for i in range(x-1, -1, -1): #Gauche
        if estAllie(plateau[y][i]):
            break
        possibilite.append((i, y))
        if estEnnemie(plateau[y][i]):
            break
    for i in range(y+1, 8):#Bas
        if estAllie(plateau[i][x]):
            break
        possibilite.append((x, i))
        if estEnnemie(plateau[i][x]):
            break
    for i in range(y-1, -1, -1):#Haut
        if estAllie(plateau[i][x]):
            break
        possibilite.append((x, i))
        if estEnnemie(plateau[i][x]):
            break
    return possibilite

def positionPossibleCavalier(x, y, plateau):#Permet de trouver les positions possibles pour un cavalier
    #On définit toutes les positions possibles du cavalier
    possibilite = [(x-1, y-2), (x+1, y-2),(x+2, y-1), (x+2, y+1), (x+1, y+2), (x-1, y+2), (x-2, y+1), (x-2, y-1)]
    #On vérifie que les positions sont valides (dans le plateau et pas alliées)
    vpossibilite = []
    if plateau[y][x].islower():
        estAllie = str.islower
    else:
        estAllie = str.isupper
    for i in range(len(possibilite)): #On parcourt les positions possibles
        u, v = possibilite[i][0], possibilite[i][1]
        if not(0 <= v <= 7 and 0 <= u <= 7):#On vérifie que la position est dans le plateau
            continue
        if estAllie(plateau[v][u]):#On vérifie que la position n'est pas alliée
            continue
        vpossibilite.append(possibilite[i])#On ajoute la position valide à la liste
    return vpossibilite

def positionPossibleFou(x, y, plateau):#Permet de trouver les positions possibles pour un fou
    if plateau[y][x].islower():#Détermine si la piece est une majuscule ou une minuscule
        estAllie = str.islower
        estEnnemie = str.isupper
    else:
        estAllie = str.isupper
        estEnnemie = str.islower
    possibilite = []#Liste des positions possibles
    direction = [(-1, -1), (1, -1), (-1, 1), (1, 1)]#Directions possibles pour le fou
    for dx, dy in direction:#On parcourt chaque direction
        for i in range(1, 8):
            posX, posY = x+dx*i, y+dy*i#Calcul de la nouvelle position
            if not(0 <= posX <= 7 and 0 <= posY <= 7):
                break
            if estAllie(plateau[posY][posX]):
                break
            if estEnnemie(plateau[posY][posX]):
                possibilite.append((posX, posY)) 
                break
            else:
                possibilite.append((posX, posY))
    return possibilite
def positionPossibleReine(x, y, plateau):#Permet de trouver les positions possibles pour une reine
    p1 = positionPossibleFou(x, y, plateau)
    p2 = positionPossibleTour(x, y, plateau)
    return p1 + p2
def est_case_attaquee(plateau, pos, est_roi_maj):
    # On regarde si la case 'pos' (x, y) est dans les zones de contrôle ennemies
    attaques_ennemies = possibiliteDeplacementMin(plateau) if est_roi_maj else possibiliteDeplacementMaj(plateau)
    for zone in attaques_ennemies:
        if pos in zone:
            return True
    return False

def positionPossibleRoi(x, y, plateau):
    possibilite = []
    # 1. Déplacements classiques (1 case autour)
    for i in range(-1, 2):
        for j in range(-1, 2):
            if (i,j) != (0, 0):
                possibilite.append((x+i,y+j))
    
    vpossibilite = []
    piece = plateau[y][x]
    est_maj = piece.isupper()
    
    # Déterminer qui est l'ennemi pour vérifier les échecs
    if est_maj:
        estAllie = str.isupper
        posEnnemie = possibiliteDeplacementMin(plateau)
        roi_carac = 'R'
        tour_carac = 'T'
        ligne = 0
    else:
        estAllie = str.islower
        posEnnemie = possibiliteDeplacementMaj(plateau)
        roi_carac = 'r'
        tour_carac = 't'
        ligne = 7

    # 2. Filtrer les cases hors plateau, alliées ou attaquées
    for u, v in possibilite:
        if not(0 <= v <= 7 and 0 <= u <= 7): continue
        if estAllie(plateau[v][u]): continue
        
        # Vérification si la case est attaquée
        attaque = False
        for elmt in posEnnemie:
            if (u,v) in elmt:
                attaque = True
                break
        if not attaque:
            vpossibilite.append((u, v))

    # --- 3. LOGIQUE DU ROQUE (ADAPTÉE ROI EN X=3) ---
    # Le Roi est sur sa case de départ et n'est pas en échec
    if y == ligne and x == 3 and droits_roque[roi_carac] and not estEchec(plateau, roi_carac):
        
        # PETIT ROQUE (Vers la Tour en 0,0 ou 0,7)
        # On vérifie les cases vides entre la tour (0) et le roi (3) : colonnes 1 et 2
        if droits_roque[tour_carac + '_gauche'] and plateau[ligne][1] == "." and plateau[ligne][2] == ".":
            # Le roi ne doit pas passer par une case attaquée (ici la case 2)
            if not est_case_attaquee(plateau, (2, ligne), est_maj):
                vpossibilite.append((1, ligne))

        # GRAND ROQUE (Vers la Tour en 7,0 ou 7,7)
        # On vérifie les cases vides entre le roi (3) et la tour (7) : colonnes 4, 5 et 6
        if droits_roque[tour_carac + '_droite'] and plateau[ligne][4] == "." and plateau[ligne][5] == "." and plateau[ligne][6] == ".":
            # Le roi ne doit pas passer par une case attaquée (ici la case 4)
            if not est_case_attaquee(plateau, (4, ligne), est_maj):
                vpossibilite.append((5, ligne))
                
    return vpossibilite

index_fonction = { #gestion des déplacements possibles
    'P': positionPossiblePion,#Aide de l'ia pour trouver la fonctionnalité
    'p': positionPossiblePion,
    'T': positionPossibleTour,
    't': positionPossibleTour,
    'C': positionPossibleCavalier,
    'c': positionPossibleCavalier,
    'F': positionPossibleFou,
    'f': positionPossibleFou,
    'D': positionPossibleReine,
    'd': positionPossibleReine,
    'R': positionPossibleRoi,
    'r': positionPossibleRoi
}

def possibiliteDeplacementMaj(plateau):#Permet de trouver les positions possibles pour les majuscules
    positionPossible = []
    for j in range(8):
        for i in range(8):
            piece = plateau[j][i]
            if piece == "." or piece.islower() :
                positionPossible.append([])
            elif piece == 'R':
                positionPossible.append(attaquesRoi(i,j,plateau))
            else:
                if piece == "P":
                    positionPossible.append(attaquesPion(i,j,plateau))
                else:
                    positionPossible.append(index_fonction[piece](i, j, plateau))
    return positionPossible

def possibiliteDeplacementMin(plateau):#Permet de trouver les positions possibles pour les minuscules
    positionPossible = []
    for j in range(8):
        for i in range(8):
            piece = plateau[j][i]
            if piece == "." or piece.isupper():
                positionPossible.append([])
            elif piece == 'r':
                positionPossible.append(attaquesRoi(i,j,plateau))
            else:
                if piece == "p":
                    positionPossible.append(attaquesPion(i,j,plateau))
                else:
                    positionPossible.append(index_fonction[piece](i, j, plateau))
    return positionPossible

def attaquesPion(x, y, plateau): #Permet de trouver les positions d'attaque pour un pion
    piece = plateau[y][x]
    if piece == "P":
        dy = 1
    else:  # "p"
        dy = -1
    attaques = []
    for dx in (-1, 1):
        nx, ny = x + dx, y + dy
        if 0 <= nx <= 7 and 0 <= ny <= 7:
            attaques.append((nx, ny))
    return attaques

def attaquesRoi(x, y, plateau):#Permet de trouver les positions d'attaque pour un roi
    piece = plateau[y][x]
    estAllie = str.islower if piece.islower() else str.isupper
    res = []
    for dy in (-1, 0, 1):
        for dx in (-1, 0, 1):
            if dx == 0 and dy == 0:
                continue
            nx, ny = x + dx, y + dy
            if not (0 <= nx <= 7 and 0 <= ny <= 7):
                continue
            if estAllie(plateau[ny][nx]):
                continue
            res.append((nx, ny))
    return res


def trouverRoi(plateau, pieceRoi):#Permet de trouver la position du roi
    for j in range(8):
        for i in range(8):
            if plateau[j][i] == pieceRoi:
                return (i, j)
    return None


def estEchec(plateau, pieceRoi):#Permet de savoir si le roi est en échec
    posRoi = trouverRoi(plateau, pieceRoi)
    if posRoi is None:
        return False
    posEnnemie = possibiliteDeplacementMin(plateau) if pieceRoi.isupper() else possibiliteDeplacementMaj(plateau)
    for elmt in posEnnemie:
        for pos in elmt:
            if pos == posRoi:
                return True
    return False


def CoupLegal(plateau, posInit, posFut): #Permet de vérifier si un coup est légal
    piece = plateau[posInit[1]][posInit[0]]
    if piece == ".":
        return False
    positionPossible = index_fonction[piece](posInit[0], posInit[1], plateau)
    if posFut not in positionPossible:
        return False
    piece_capturee = faire_coup_rapide(plateau, posInit, posFut)#Simule le déplacement
    pieceRoi = 'R' if piece.isupper() else 'r'
    if estEchec(plateau, pieceRoi):#Permet de vérifier si le roi est en échec après le déplacement
        defaire_coup_rapide(plateau, posInit, posFut, piece_capturee)#Annule le déplacement
        return False
    defaire_coup_rapide(plateau, posInit, posFut, piece_capturee)
    return True

def ensembleCoupsLegauxMin(plateau):#Permet de trouver tous les coups légaux pour les minuscules
    coupsLegaux = []
    for j in range(8):
        for i in range(8):
            piece = plateau[j][i]
            if piece.islower():
                positionPossible = index_fonction[piece](i, j, plateau)
                for posFut in positionPossible:
                    if CoupLegal(plateau, (i,j), posFut):
                        coupsLegaux.append( ((i,j), posFut) )
    return coupsLegaux


def ensembleCoupsLegauxMaj(plateau):#Permet de trouver tous les coups légaux pour les majuscules
    coupsLegaux = []
    for j in range(8):
        for i in range(8):
            piece = plateau[j][i]
            if piece.isupper():
                positionPossible = index_fonction[piece](i, j, plateau)
                for posFut in positionPossible:
                    if CoupLegal(plateau, (i,j), posFut):
                        coupsLegaux.append( ((i,j), posFut) )
    return coupsLegaux

def finDePartie(plateau, nbTour):#Permet de vérifier si la partie est terminée
    if nbTour % 2 == 0:
        coupsLegaux = ensembleCoupsLegauxMin(plateau)
        pieceRoi = 'r'
    else:
        coupsLegaux = ensembleCoupsLegauxMaj(plateau)
        pieceRoi = 'R'
    if len(coupsLegaux) == 0:
        if estEchec(plateau, pieceRoi):
            print("Echec et mat ! Le camp ", "Majuscules" if pieceRoi == 'r' else "Minuscules", " a gagné.")
        else:
            print("Pat ! Match nul.")
        return True
    return False