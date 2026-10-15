"""Estime l'Elo de WakaChess à partir d'un PGN de gauntlet contre des adversaires « SF<elo> »."""
import math
import re
import sys

texte = open(sys.argv[1], encoding="utf-8").read()
parties = re.findall(r'\[White "([^"]+)"\]\s*\[Black "([^"]+)"\]\s*\[Result "([^"]+)"\]', texte)

scores = {}  # elo adverse -> [points, parties]
for blanc, noir, res in parties:
    if res not in ("1-0", "0-1", "1/2-1/2"):
        continue
    adversaire = noir if blanc == "WakaChess" else blanc
    points = {"1-0": 1, "0-1": 0, "1/2-1/2": 0.5}[res]
    if blanc != "WakaChess":
        points = 1 - points
    s = scores.setdefault(int(re.sub(r"\D", "", adversaire)), [0.0, 0])
    s[0] += points
    s[1] += 1

def attendu(elo, adversaire):
    return 1 / (1 + 10 ** ((adversaire - elo) / 400))

def vraisemblance(elo):
    return sum(p * math.log(attendu(elo, a)) + (n - p) * math.log(1 - attendu(elo, a))
               for a, (p, n) in scores.items())

for adversaire, (p, n) in sorted(scores.items()):
    print(f"SF{adversaire} : {p:g}/{n} ({p / n:.0%})")
candidats = range(0, 3600)
meilleur = max(candidats, key=vraisemblance)
seuil = vraisemblance(meilleur) - 1.92  # intervalle de confiance à 95 %
intervalle = [e for e in candidats if vraisemblance(e) >= seuil]
print(f"Elo estimé : {meilleur} (95 % : {intervalle[0]} - {intervalle[-1]})")
