// moteur.js - L'IA tourne ici, dans un Web Worker, pour que l'écran reste fluide pendant qu'elle réfléchit
'use strict';
importScripts('wakachess.js');

const pret = WakaChessModule().then((M) => ({
  nouvelle: M.cwrap('wc_nouvelle', null, []),
  fen: M.cwrap('wc_fen', 'number', ['string']),
  jouer: M.cwrap('wc_jouer', 'number', ['string']),
  chercher: M.cwrap('wc_chercher', 'string', ['number']),
  viderTT: M.cwrap('wc_vider_tt', null, []),
}));

onmessage = async (e) => {
  const api = await pret;
  if (e.data.nouvelle) { api.viderTT(); return; }
  const { id, fen, coups, niveau, debut } = e.data;
  if (!fen || !api.fen(fen)) api.nouvelle();
  for (const m of coups) api.jouer(m);
  postMessage({ id, debut, resultat: api.chercher(niveau) });
};
