#Divers utilitaires pour le jeu d'échecs
def parse_commande(commande: str) -> tuple[tuple[int,int], tuple[int,int]] | None:
    if not isinstance(commande, str):
        return None
    if len(commande.split()) != 4:
        return None
    commande = commande.split()
    x,y,X,Y = commande
    return (int(x), int(y)), (int(X), int(Y))

def dans_plateau(x: int, y: int) -> bool: #On vérifie que les coordonnées sont dans le plateau
    """Retourne True si (x,y) est dans [0..7]."""
    if 0 <= x <= 7 and 0 <= y <= 7:
        return True
    return False

def joueur_peut_jouer(piece: str, nbTour: int) -> bool: #On vérifie que le joueur joue bien son camp
    """Vérifie si c’est au bon camp de jouer cette pièce."""
    if (nbTour % 2 == 1 and piece.islower()) or (nbTour % 2 == 0 and piece.isupper()):
        return True
    return False