#Voici le fichier principal main.py qui gère le déroulement du jeu d'échecs
#Ce jeu permet de jouer contre une IA utilisant l'algorithme Minimax avec élagage Alpha-Beta
#On peut choisir son camp (majuscules ou minuscules) et la difficulté (profondeur de recherche de l'IA)
#Le jeu affiche le plateau après chaque coup et vérifie la légalité des mouvements
#Le jeu se termine lorsqu'un camp n'a plus de coups légaux (échec et mat ou pat)
#Il manque certaines fonctionnalités comme le roque
#Ainsi j'ai utilisé l'ia pour trouver certaines fonctionnalités et surtout pour 
#tester le code car je ne peux pas anticiper tous les cas de figure
#Et par conséquent l'ia est une aide pour trouver les bueug éventuels ou les bueug


#Le code pourrait être amélioré en l'optimisant davantage car l'agorithme Minimax peut être lent pour des profondeurs élevées
#On pourrait aussi ajouter une interface graphique pour rendre le jeu plus convivial
#Enfin, il serait intéressant d'ajouter des fonctionnalités supplémentaires comme le roque
#

#Enfin bon courage pour la lecture du code car certains noms de variables peuvent être obscure.
from plateau import *
from utilitaire import *
from geo import *
from ia import *


def changementPosition(posInit, posFut, piece, plateau): #Fonction qui change la position d'une piece si le coup est legal
    if CoupLegal(plateau, posInit, posFut) == False:
        return False
    deplacer_piece_physique(posInit, posFut, piece, plateau)
    yFut = posFut[1]
    if piece == "P" and yFut == 7:
        c = input("Placer un pion de votre choix entre D T F C : ")
        plateau[yFut][posFut[0]] = c
    elif piece == "p" and yFut == 0:
        c = input("Placer un pion de votre choix entre d t f c : ")
        plateau[yFut][posFut[0]] = c
    return True


    
def main():
    plateau = generationPlateau()#Initialisation du plateau
    nbTour = 0#Compteur de tours
     # Boucle principale du jeu
     # Le jeu continue tant que la variable play est True
    play = True
    Nombre_Profondeur = 3#Profondeur par défaut
    IA_camp_est_maj = True  # L'IA joue les Majuscules (Blancs)
    print("Bienvenue dans ce jeu d'échecs")
    campChoisi = input("Choisissez votre camp (M pour Majuscules/Blancs, m pour Minuscules/Noirs) : ")
    if campChoisi == 'M':#On définit le camp de l'IA en fonction du choix du joueur
        IA_camp_est_maj = False
        print("Les majuscules commencent")
    else:
        IA_camp_est_maj = True
        print("Les minuscules commencent")
    #Choix de la difficulté
    difficulty = input("Choisissez la difficulté (1-10) : ")
    try:
        #La difficulté détermine la profondeur de recherche de l'IA
        difficulty = int(difficulty)
        if 1 <= difficulty <= 10:#La profondeur de 10 ralenti enormement le jeu
            Nombre_Profondeur = difficulty + 1  # Ajuste la profondeur en fonction de la difficulté
        else:
            print("Difficulté invalide, niveau par défaut (3) sélectionné.")
    except ValueError:
        print("Difficulté invalide, niveau par défaut (3) sélectionné.")
    while play:
        print("TOUR NUMERO : ",nbTour)
        affichagePlateau(plateau)#Affichage du plateau
         # Tour de l'IA
         # L'IA joue si c'est son tour en fonction du camp choisi
        if (nbTour % 2 == 0 and IA_camp_est_maj) or (nbTour % 2 == 1 and not IA_camp_est_maj):
            print("Le bot réfléchit...")
            coup, score = trouver_meilleur_coup(plateau, Nombre_Profondeur, IA_camp_est_maj)
            if coup is None:
                print("Le bot ne peut pas jouer. Fin de la partie.")
                break
            posInit, posFut = coup
            piece = plateau[posInit[1]][posInit[0]]
            changementPosition(posInit, posFut, piece, plateau)
            print(f"Le bot a joué de {posInit} à {posFut} avec un score de {score}.")
            nbTour += 1
            if finDePartie(plateau, nbTour):
                play = False
                break
            continue
        else:
            commande = input("Commande pour le jeu d'echecs : ")#On demande la commande au joueur
            if commande == "help":
                print("Pour jouer, entrez les coordonnées de la pièce à déplacer suivies des coordonnées de la destination, séparées par des espaces. Par exemple : '1 1 1 3' déplace la pièce de (1,1) à (1,3).")
                print("Entrez 'stop' pour quitter le jeu.")
                print("Entrez 'help' pour afficher cette aide.")
            elif commande == "stop":#On arrête le jeu si le joueur entre "stop"
                play = False
                break
            posInit, posFut = parse_commande(commande)#On parse la commande pour obtenir les positions initiale et future
             # On vérifie que les positions sont valides et que le coup est légal
            if posInit and posFut:
                piece = plateau[posInit[1]][posInit[0]]
                if not joueur_peut_jouer(piece, nbTour):#On vérifie que le joueur joue bien son camp
                    print("Ce n'est pas à votre tour de jouer cette pièce")
                    continue
                if not(dans_plateau(posInit[0], posInit[1])) or not(dans_plateau(posFut[0], posFut[1])):
                    #On vérifie que les positions sont dans le plateau
                    print("Position hors plateau")
                    continue
                if piece == ".":
                    print("Ceci n'est pas une piece")
                else:
                    if changementPosition(posInit, posFut, piece, plateau):
                        print("Coup effectué")
                        nbTour += 1
                    else:  
                        print("Coup illégal")
            else:
                print("Commande invalide")
            if finDePartie(plateau, nbTour):#On vérifie si la partie est terminée
                play = False
                break


        
        
    
    


if __name__ == "__main__":
    main()




