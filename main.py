# main.py - Fichier principal du jeu WakaChess
from plateau import *
from utilitaire import *
from geo import *
from ia import *
import time



def changementPosition(posInit, posFut, piece, plateau, est_ia=False):
    global case_en_passant, compteur_50_coups
    
    if not CoupLegal(plateau, posInit, posFut): return False
    
    # Détection capture en passant (si le roi va sur la case vide de l'en-passant)
    if piece.upper() == 'P' and posFut == case_en_passant:
        # On supprime le pion adverse qui est juste derrière
        plateau[posInit[1]][posFut[0]] = "."

    # Mise à jour du compteur 50 coups
    # Si pion bouge ou capture, on remet à zéro, sinon +1
    if piece.upper() == 'P' or plateau[posFut[1]][posFut[0]] != ".":
        compteur_50_coups = 0
    else:
        compteur_50_coups += 1

    # Définir la case en passant pour le PROCHAIN tour
    if piece.upper() == 'P' and abs(posFut[1] - posInit[1]) == 2:
        case_en_passant = (posInit[0], (posInit[1] + posFut[1]) // 2)
    else:
        case_en_passant = None

    # Exécution du mouvement
    executer_mouvement_complet(plateau, posInit, posFut, droits_roque)
    
    # ... (reste du code pour promotion) ...
    return True
historique_positions = []

def verifier_nulle(plateau):
    # Convertir le plateau (liste de listes) en chaîne de caractères pour le comparer
    etat_actuel = str(plateau)
    historique_positions.append(etat_actuel)
    
    # Répétition 3 fois
    if historique_positions.count(etat_actuel) >= 3:
        print("Match nul par répétition !")
        return True
    return False
def main():
    # Initialisation du jeu
    plateau = generationPlateau()
    nbTour = 0
    play = True
    Nombre_Profondeur = 3
    
    print("=== Bienvenue dans WakaChess ===")
    campChoisi = input("Choisissez votre camp (M pour Majuscules/Blancs, m pour Minuscules/Noirs) : ")
    
    # Si le joueur choisit m, l'IA joue les Blancs (Majuscules)
    IA_camp_est_maj = (campChoisi.lower() == 'm')
    
    if IA_camp_est_maj:
        print("L'IA joue les Majuscules et commence la partie.")
    else:
        print("Vous jouez les Majuscules et commencez la partie.")

    # Choix de la difficulté
    difficulty = input("Choisissez la difficulté (1-5) : ")
    try:
        difficulty = int(difficulty)
        Nombre_Profondeur = max(1, min(difficulty + 1, 6))
    except ValueError:
        print("Niveau par défaut (3) sélectionné.")

    while play:
        # Dans ta boucle while play :
        debut_reflexion = time.time()
        # On réduit la profondeur si l'IA a mis trop de temps au tour précédent
        temps_ia = 10.0 # limite de 10 secondes par coup
        coup, score = trouver_meilleur_coup(plateau, Nombre_Profondeur, IA_camp_est_maj, debut_reflexion, temps_ia)
        print("\n" + "="*20)
        print(f"TOUR NUMERO : {nbTour}")
        affichagePlateau(plateau)

        # Déterminer si c'est au tour de l'IA ou du Joueur
        # Tour pair (0, 2, 4...) : Majuscules jouent
        # Tour impair (1, 3, 5...) : Minuscules jouent
        est_tour_majuscules = (nbTour % 2 == 0)
        ia_doit_jouer = (est_tour_majuscules and IA_camp_est_maj) or (not est_tour_majuscules and not IA_camp_est_maj)

        if ia_doit_jouer:
            print("Le bot réfléchit...")
            coup, score = trouver_meilleur_coup(plateau, Nombre_Profondeur, IA_camp_est_maj if est_tour_majuscules else not IA_camp_est_maj)
            
            if coup is None:
                print("Le bot est bloqué. Fin de la partie.")
                break
            
            posInit, posFut = coup
            piece = plateau[posInit[1]][posInit[0]]
            
            if changementPosition(posInit, posFut, piece, plateau, est_ia=True):
                print(f"Le bot a joué : {posInit} -> {posFut} (Score: {score})")
                nbTour += 1
            else:
                print("Erreur critique : l'IA a tenté un coup illégal.")
                break
        else:
            # Tour du Joueur
            commande = input("Votre coup (ex: 3 6 3 4) ou 'stop' : ")
            
            if commande.lower() == "stop":
                play = False
                break
            elif commande.lower() == "help":
                print("Entrez : x_depart y_depart x_arrivee y_arrivee")
                continue

            try:
                posInit, posFut = parse_commande(commande)
                if posInit and posFut:
                    piece = plateau[posInit[1]][posInit[0]]
                    
                    # Vérifier si le joueur touche bien ses propres pièces
                    if (est_tour_majuscules and not piece.isupper()) or (not est_tour_majuscules and not piece.islower()):
                        print("Ce n'est pas à votre tour de jouer cette pièce !")
                        continue
                    
                    if changementPosition(posInit, posFut, piece, plateau, est_ia=False):
                        print("Coup effectué.")
                        nbTour += 1
                    else:
                        print("Coup illégal ! Vérifiez les règles.")
                else:
                    print("Format invalide. Exemple: 3 6 3 4")
        
            except Exception as e:
                print(f"Erreur de saisie : {e}")


        # Vérification de fin de partie
        if finDePartie(plateau, nbTour):
            affichagePlateau(plateau)
            play = False
        # Dans main.py, après nbTour += 1
        etat_plateau = str(plateau)
        historique_positions.append(etat_plateau)

        if historique_positions.count(etat_plateau) >= 3:
            print("Nulle par répétition !")
            play = False
        elif compteur_50_coups >= 100: # 50 coups par camp = 100 demi-coups
            print("Nulle (règle des 50 coups) !")
            play = False

    print("Merci d'avoir joué à WakaChess !")


if __name__ == "__main__":
    main()