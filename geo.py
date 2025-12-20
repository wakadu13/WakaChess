# Geo.py - Gestion de la géométrie et des coups légaux
from plateau import *

# --- FONCTIONS DE DÉPLACEMENT ---

def positionPossiblePion(x, y, plateau):
    if plateau[y][x].isupper():
        estEnnemie = str.islower
        decallage = 1
        posDepart = 1
    elif plateau[y][x].islower():
        estEnnemie = str.isupper
        decallage = -1
        posDepart = 6
    else: return []

    possibilite = []
    # Double déplacement
    if (0 <= y + 2*decallage <= 7) and y == posDepart and plateau[y + 2*decallage][x] == "." and plateau[y + decallage][x] == ".":
        possibilite.append((x, y+2*decallage))
    # Simple déplacement
    if (0 <= y + decallage <= 7) and plateau[y + decallage][x] == ".":
        possibilite.append((x, y+decallage))
    # Prises en diagonale
    for dx in [-1, 1]:
        if (0 <= x + dx <= 7) and (0 <= y + decallage <= 7) and estEnnemie(plateau[y + decallage][x + dx]):
            possibilite.append((x + dx, y + decallage))
    return possibilite

def positionPossibleTour(x, y, plateau):
    if plateau[y][x].isupper():
        estAllie, estEnnemie = str.isupper, str.islower
    else:
        estAllie, estEnnemie = str.islower, str.isupper
    
    possibilite = []
    for dx, dy in [(1,0), (-1,0), (0,1), (0,-1)]:
        for i in range(1, 8):
            nx, ny = x + dx*i, y + dy*i
            if not (0 <= nx <= 7 and 0 <= ny <= 7) or estAllie(plateau[ny][nx]): break
            possibilite.append((nx, ny))
            if estEnnemie(plateau[ny][nx]): break
    return possibilite

def positionPossibleCavalier(x, y, plateau):
    poss = [(x-1,y-2),(x+1,y-2),(x+2,y-1),(x+2,y+1),(x+1,y+2),(x-1,y+2),(x-2,y+1),(x-2,y-1)]
    estAllie = str.islower if plateau[y][x].islower() else str.isupper
    res = []
    for u, v in poss:
        if 0 <= u <= 7 and 0 <= v <= 7 and not estAllie(plateau[v][u]):
            res.append((u, v))
    return res

def positionPossibleFou(x, y, plateau):
    if plateau[y][x].islower():
        estAllie, estEnnemie = str.islower, str.isupper
    else:
        estAllie, estEnnemie = str.isupper, str.islower
    possibilite = []
    for dx, dy in [(-1,-1), (1,-1), (-1,1), (1,1)]:
        for i in range(1, 8):
            nx, ny = x + dx*i, y + dy*i
            if not (0 <= nx <= 7 and 0 <= ny <= 7) or estAllie(plateau[ny][nx]): break
            possibilite.append((nx, ny))
            if estEnnemie(plateau[ny][nx]): break
    return possibilite

def positionPossibleReine(x, y, plateau):
    return positionPossibleFou(x, y, plateau) + positionPossibleTour(x, y, plateau)

# --- GESTION DU ROI ET DU ROQUE ---

def est_case_attaquee(plateau, pos, est_roi_maj):
    # Important : on utilise les attaques simplifiées pour éviter la récursion infinie
    attaques = possibiliteDeplacementMin(plateau) if est_roi_maj else possibiliteDeplacementMaj(plateau)
    for zone in attaques:
        if pos in zone: return True
    return False

def positionPossibleRoi(x, y, plateau):
    possibilite = []
    # 1. Déplacements standards
    for dx in [-1, 0, 1]:
        for dy in [-1, 0, 1]:
            if dx == 0 and dy == 0: continue
            nx, ny = x + dx, y + dy
            if 0 <= nx <= 7 and 0 <= ny <= 7:
                possibilite.append((nx, ny))
    
    piece = plateau[y][x]
    est_maj = piece.isupper()
    estAllie = str.isupper if est_maj else str.islower
    vpossibilite = []
    
    # Filtrage des cases (alliées ou échecs)
    for u, v in possibilite:
        if not estAllie(plateau[v][u]):
            # On vérifie si la case destination est attaquée (le roi ne peut se mettre en échec)
            if not est_case_attaquee(plateau, (u, v), est_maj):
                vpossibilite.append((u, v))

    # --- 2. LE ROQUE (ROI EN X=3) ---
    ligne = 0 if est_maj else 7
    roi_carac = 'R' if est_maj else 'r'
    tour_carac = 'T' if est_maj else 't'

    if y == ligne and x == 3 and droits_roque[roi_carac] and not estEchec(plateau, roi_carac):
        # Petit Roque (Vers la gauche, x=0)
        if droits_roque[tour_carac + '_gauche'] and plateau[ligne][1] == "." and plateau[ligne][2] == ".":
            if not est_case_attaquee(plateau, (2, ligne), est_maj):
                vpossibilite.append((1, ligne))
        
        # Grand Roque (Vers la droite, x=7)
        if droits_roque[tour_carac + '_droite'] and plateau[ligne][4] == "." and plateau[ligne][5] == "." and plateau[ligne][6] == ".":
            if not est_case_attaquee(plateau, (4, ligne), est_maj):
                vpossibilite.append((5, ligne))
                
    return vpossibilite

# --- DICTIONNAIRE ET ANALYSE GLOBALE ---

index_fonction = {
    'P': positionPossiblePion, 'p': positionPossiblePion,
    'T': positionPossibleTour, 't': positionPossibleTour,
    'C': positionPossibleCavalier, 'c': positionPossibleCavalier,
    'F': positionPossibleFou, 'f': positionPossibleFou,
    'D': positionPossibleReine, 'd': positionPossibleReine,
    'R': positionPossibleRoi, 'r': positionPossibleRoi
}

def possibiliteDeplacementMaj(plateau):
    res = []
    for j in range(8):
        for i in range(8):
            p = plateau[j][i]
            if p.isupper():
                if p == 'R': res.append(attaquesRoi(i, j, plateau))
                elif p == 'P': res.append(attaquesPion(i, j, plateau))
                else: res.append(index_fonction[p](i, j, plateau))
            else: res.append([])
    return res

def possibiliteDeplacementMin(plateau):
    res = []
    for j in range(8):
        for i in range(8):
            p = plateau[j][i]
            if p.islower():
                if p == 'r': res.append(attaquesRoi(i, j, plateau))
                elif p == 'p': res.append(attaquesPion(i, j, plateau))
                else: res.append(index_fonction[p](i, j, plateau))
            else: res.append([])
    return res

# Fonctions d'attaques simplifiées pour éviter la récursion infinie dans estEchec
def attaquesPion(x, y, plateau):
    dy = 1 if plateau[y][x] == 'P' else -1
    res = []
    for dx in [-1, 1]:
        if 0 <= x+dx <= 7 and 0 <= y+dy <= 7: res.append((x+dx, y+dy))
    return res

def attaquesRoi(x, y, plateau):
    res = []
    for dx in [-1,0,1]:
        for dy in [-1,0,1]:
            if dx == 0 and dy == 0: continue
            if 0 <= x+dx <= 7 and 0 <= y+dy <= 7: res.append((x+dx, y+dy))
    return res

def trouverRoi(plateau, pieceRoi):
    for j in range(8):
        for i in range(8):
            if plateau[j][i] == pieceRoi: return (i, j)
    return None

def estEchec(plateau, pieceRoi):
    posRoi = trouverRoi(plateau, pieceRoi)
    if not posRoi: return False
    return est_case_attaquee(plateau, posRoi, pieceRoi.isupper())

def CoupLegal(plateau, posInit, posFut):
    piece = plateau[posInit[1]][posInit[0]]
    if piece == ".": return False
    if posFut not in index_fonction[piece](posInit[0], posInit[1], plateau): return False
    
    # Simulation
    piece_cap = faire_coup_rapide(plateau, posInit, posFut)
    roi = 'R' if piece.isupper() else 'r'
    echec = estEchec(plateau, roi)
    defaire_coup_rapide(plateau, posInit, posFut, piece_cap)
    return not echec

def ensembleCoupsLegauxMin(plateau):
    coups = []
    for j in range(8):
        for i in range(8):
            if plateau[j][i].islower():
                for pf in index_fonction[plateau[j][i]](i, j, plateau):
                    if CoupLegal(plateau, (i,j), pf): coups.append(((i,j), pf))
    return coups

def ensembleCoupsLegauxMaj(plateau):
    coups = []
    for j in range(8):
        for i in range(8):
            if plateau[j][i].isupper():
                for pf in index_fonction[plateau[j][i]](i, j, plateau):
                    if CoupLegal(plateau, (i,j), pf): coups.append(((i,j), pf))
    return coups

def finDePartie(plateau, nbTour):
    est_tour_min = (nbTour % 2 == 1) # Car tu as dit que les noirs commencent au tour 0 ou 1 ? 
    # Attention : ajuste selon ta boucle main. Si tour 0 = blancs, alors pair = maj, impair = min.
    coups = ensembleCoupsLegauxMin(plateau) if nbTour % 2 != 0 else ensembleCoupsLegauxMaj(plateau)
    roi = 'r' if nbTour % 2 != 0 else 'R'
    
    if not coups:
        if estEchec(plateau, roi):
            print("Echec et mat !")
        else:
            print("Pat !")
        return True
    return False