#!/usr/bin/env bash
# Mesure le niveau de WakaChess contre Stockfish bridé (UCI_Elo, calibré sur la liste CCRL).
# Prérequis : fastchess (https://github.com/Disservin/fastchess), stockfish, un livre d'ouvertures PGN
# (ex. 8moves_v3.pgn de https://github.com/official-stockfish/books).
# Usage : outils/mesurer_elo.sh <moteur> <livre.pgn> [parties_par_niveau] [cadence]
set -euo pipefail

MOTEUR=${1:?moteur WakaChess}
LIVRE=${2:?livre d\'ouvertures PGN}
PARTIES=${3:-40}
CADENCE=${4:-10+0.1}
FASTCHESS=${FASTCHESS:-fastchess}
STOCKFISH=${STOCKFISH:-stockfish}
SORTIE=${SORTIE:-gauntlet.pgn}

SF="cmd=$STOCKFISH option.UCI_LimitStrength=true option.Threads=1 option.Hash=16"
"$FASTCHESS" -engine cmd="$MOTEUR" args=--uci name=WakaChess \
    -engine $SF option.UCI_Elo=1320 name=SF1320 -engine $SF option.UCI_Elo=1600 name=SF1600 \
    -engine $SF option.UCI_Elo=1900 name=SF1900 -engine $SF option.UCI_Elo=2200 name=SF2200 \
    -each tc="$CADENCE" -openings file="$LIVRE" format=pgn order=random \
    -rounds $((PARTIES / 2)) -games 2 -repeat -concurrency "$(nproc)" \
    -tournament gauntlet -seeds 1 -recover -pgnout file="$SORTIE"

python3 "$(dirname "$0")/elo.py" "$SORTIE"
