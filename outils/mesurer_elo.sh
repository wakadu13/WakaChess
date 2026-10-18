#!/usr/bin/env bash
# Mesure le niveau de WakaChess contre Stockfish bridé (UCI_Elo, calibré sur la liste CCRL).
# Prérequis : fastchess (https://github.com/Disservin/fastchess), stockfish, un livre d'ouvertures PGN
# (ex. 8moves_v3.pgn de https://github.com/official-stockfish/books).
# Usage : outils/mesurer_elo.sh <moteur> <livre.pgn> [parties_par_niveau] [cadence]
# Variables : NIVEAUX ("2000 2300 2600 2900"), FASTCHESS, STOCKFISH, SORTIE
set -euo pipefail

MOTEUR=${1:?moteur WakaChess}
LIVRE=${2:?livre d\'ouvertures PGN}
PARTIES=${3:-40}
CADENCE=${4:-10+0.1}
FASTCHESS=${FASTCHESS:-fastchess}
STOCKFISH=${STOCKFISH:-stockfish}
SORTIE=${SORTIE:-gauntlet.pgn}

NIVEAUX=${NIVEAUX:-"2000 2300 2600 2900"} # niveaux de Stockfish (UCI_Elo, de 1320 à 3190)

SF="cmd=$STOCKFISH option.UCI_LimitStrength=true option.Threads=1 option.Hash=16"
ADVERSAIRES=()
for elo in $NIVEAUX; do ADVERSAIRES+=(-engine $SF option.UCI_Elo="$elo" name="SF$elo"); done

"$FASTCHESS" -engine cmd="$MOTEUR" args=--uci name=WakaChess "${ADVERSAIRES[@]}" \
    -each tc="$CADENCE" -openings file="$LIVRE" format=pgn order=random \
    -rounds $((PARTIES / 2)) -games 2 -repeat -concurrency "$(nproc)" \
    -tournament gauntlet -seeds 1 -recover -pgnout file="$SORTIE"

python3 "$(dirname "$0")/elo.py" "$SORTIE"
