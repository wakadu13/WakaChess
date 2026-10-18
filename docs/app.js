// app.js - Interface de WakaChess (web, Android, iOS, ordinateurs) : doigt ou souris
'use strict';

const NIVEAUX = ['Débutant', 'Facile', 'Moyen (0,2 s)', 'Fort (1 s)', 'Maximum (3 s)'];
const ETATS = ['', 'Échec et mat', 'Pat : partie nulle.', 'Partie nulle par triple répétition.',
  'Partie nulle (règle des 50 coups).', 'Partie nulle (matériel insuffisant).'];
// Symboles pleins (♚♛♜♝♞♟) colorés en CSS ; U+FE0E impose le style texte (pas d'emoji)
const GLYPHES = { k: '♚', q: '♛', r: '♜', b: '♝', n: '♞', p: '♟' };
const TEXTE = '︎';

const etat = {
  fen: '', coups: [], campHumain: 0, retourne: false, niveau: 3,
  selection: -1, legaux: [], finie: false, reflechit: false, demande: 0,
};
let R = null;      // moteur des règles (WebAssembly, thread principal)
let moteur = null; // IA (WebAssembly dans un Web Worker, pour ne pas figer l'écran)

const $ = (id) => document.getElementById(id);
const plateauEl = $('plateau');
const cases = [];

const nomCase = (c) => 'abcdefgh'[c % 8] + (Math.floor(c / 8) + 1);
const indexCase = (s) => (s.charCodeAt(0) - 97) + 8 * (s.charCodeAt(1) - 49);
const estBlanc = (p) => p >= 'A' && p <= 'Z';

// --- Sauvegarde de la partie en cours (facultative : peut être indisponible) ---

function sauver() {
  try {
    localStorage.setItem('wakachess', JSON.stringify({
      fen: etat.fen, coups: etat.coups, campHumain: etat.campHumain, retourne: etat.retourne, niveau: etat.niveau,
    }));
  } catch (e) { /* stockage indisponible */ }
}

function relire() {
  try { return JSON.parse(localStorage.getItem('wakachess')); } catch (e) { return null; }
}

// --- IA dans un Web Worker ---

function creerMoteur() {
  if (moteur) moteur.terminate();
  etat.reflechit = false;
  moteur = new Worker('moteur.js');
  moteur.onmessage = (e) => {
    if (e.data.id !== etat.demande || !etat.reflechit) return; // réponse périmée
    const coup = e.data.resultat.split(' ')[0];
    const attente = Math.max(0, 250 - (performance.now() - e.data.debut)); // pas de réponse instantanée déroutante
    setTimeout(() => {
      if (e.data.id !== etat.demande || !etat.reflechit) return;
      etat.reflechit = false;
      if (coup) jouer(coup);
    }, attente);
  };
}

function lancerIA() {
  etat.reflechit = true;
  etat.demande += 1;
  moteur.postMessage({ id: etat.demande, fen: etat.fen, coups: etat.coups.slice(), niveau: etat.niveau, debut: performance.now() });
  dessiner();
}

function arreterIA() {
  if (etat.reflechit) creerMoteur(); // on interrompt la réflexion en relançant le worker
  etat.demande += 1;
}

// --- Déroulement de la partie ---

function majLegaux() { etat.legaux = R.legaux() ? R.legaux().split(' ') : []; }

function jouer(uci) {
  if (!R.jouer(uci)) return;
  etat.coups.push(uci);
  etat.selection = -1;
  majLegaux();
  sauver();
  dessiner();
  verifierFin();
  if (!etat.finie && R.trait() !== etat.campHumain) lancerIA();
}

function verifierFin() {
  const e = R.etat();
  if (e === 0) { etat.finie = false; return; }
  etat.finie = true;
  let texte = ETATS[e];
  if (e === 1) texte += R.trait() === 0 ? ' : les Noirs gagnent !' : ' : les Blancs gagnent !';
  dessiner();
  setTimeout(() => fenetre(texte, [
    ['Rejouer avec les Blancs', () => nouvellePartie(0)],
    ['Rejouer avec les Noirs', () => nouvellePartie(1)],
    ['Voir l\'échiquier', null],
  ]), 150);
}

function nouvellePartie(camp) {
  arreterIA();
  R.nouvelle();
  etat.fen = '';
  moteur.postMessage({ nouvelle: true });
  etat.coups = [];
  etat.campHumain = camp;
  etat.retourne = camp === 1;
  etat.selection = -1;
  etat.finie = false;
  majLegaux();
  sauver();
  construirePlateau();
  dessiner();
  if (R.trait() !== etat.campHumain) lancerIA();
}

function annulerCoup() {
  arreterIA();
  if (etat.coups.length === 0) return;
  do {
    R.annuler();
    etat.coups.pop();
  } while (etat.coups.length > 0 && R.trait() !== etat.campHumain);
  etat.selection = -1;
  etat.finie = false;
  majLegaux();
  sauver();
  dessiner();
  if (R.trait() !== etat.campHumain) lancerIA();
}

// --- Dessin ---

function construirePlateau() {
  plateauEl.textContent = '';
  for (let ligne = 0; ligne < 8; ligne++) {
    for (let col = 0; col < 8; col++) {
      const c = etat.retourne ? ligne * 8 + (7 - col) : (7 - ligne) * 8 + col;
      const el = document.createElement('div');
      el.className = 'case ' + ((c % 8 + Math.floor(c / 8)) % 2 ? 'claire' : 'sombre');
      el.setAttribute('role', 'gridcell');
      if (ligne === 7) el.insertAdjacentHTML('beforeend', `<span class="coord lettre">${'abcdefgh'[c % 8]}</span>`);
      if (col === 0) el.insertAdjacentHTML('beforeend', `<span class="coord chiffre">${Math.floor(c / 8) + 1}</span>`);
      const piece = document.createElement('span');
      piece.className = 'piece';
      el.appendChild(piece);
      plateauEl.appendChild(el);
      cases[c] = { el, piece };
    }
  }
}

function dessiner() {
  const plateau = R.plateau();
  const dernier = etat.coups.length ? etat.coups[etat.coups.length - 1] : null;
  const de = dernier ? indexCase(dernier.slice(0, 2)) : -1;
  const vers = dernier ? indexCase(dernier.slice(2, 4)) : -1;
  const roiEchec = R.echec() ? R.caseRoi() : -1;
  const cibles = new Map();
  if (etat.selection >= 0) {
    const origine = nomCase(etat.selection);
    for (const m of etat.legaux) if (m.startsWith(origine)) cibles.set(indexCase(m.slice(2, 4)), true);
  }

  for (let c = 0; c < 64; c++) {
    const { el, piece } = cases[c];
    const p = plateau[c];
    el.classList.toggle('dernier', c === de || c === vers);
    el.classList.toggle('selection', c === etat.selection);
    el.classList.toggle('echec', c === roiEchec);
    el.classList.toggle('cible', cibles.has(c));
    el.classList.toggle('prise', cibles.has(c) && p !== '.');
    if (p === '.') {
      piece.textContent = '';
      piece.className = 'piece';
    } else {
      piece.textContent = GLYPHES[p.toLowerCase()] + TEXTE;
      piece.className = 'piece ' + (estBlanc(p) ? 'blanc' : 'noir');
    }
  }

  const statut = $('statut');
  statut.classList.toggle('fin', etat.finie);
  if (etat.finie) {
    const e = R.etat();
    statut.textContent = ETATS[e] + (e === 1 ? (R.trait() === 0 ? ' : les Noirs gagnent !' : ' : les Blancs gagnent !') : '');
  } else {
    let s = etat.reflechit ? 'L\'IA réfléchit…' : (etat.campHumain === 0 ? 'À vous de jouer (Blancs)' : 'À vous de jouer (Noirs)');
    if (R.echec()) s += ' — Échec !';
    if (dernier) s += ' — Dernier coup : ' + dernier;
    statut.textContent = s;
  }
  $('btn-annuler').disabled = etat.coups.length === 0;
}

function ajusterTaille() {
  // L'échiquier occupe la place disponible, en largeur comme en hauteur
  const reserve = document.querySelector('header').offsetHeight + document.querySelector('nav').offsetHeight + 90;
  const taille = Math.floor(Math.max(240, Math.min(window.innerWidth - 32, window.innerHeight - reserve, 760)) / 8) * 8;
  document.documentElement.style.setProperty('--taille', taille + 'px');
}

// --- Fenêtres (fin de partie, nouvelle partie, promotion) ---

function fenetre(texte, boutons, classe) {
  return new Promise((resoudre) => {
    $('fenetre-texte').textContent = texte;
    const zone = $('fenetre-boutons');
    zone.textContent = '';
    for (const [libelle, action, valeur] of boutons) {
      const b = document.createElement('button');
      b.type = 'button';
      b.textContent = libelle;
      if (classe) b.className = classe;
      b.addEventListener('click', () => {
        $('fenetre').hidden = true;
        if (action) action();
        resoudre(valeur);
      });
      zone.appendChild(b);
    }
    $('fenetre').hidden = false;
  });
}

function choisirPromotion() {
  const blanc = etat.campHumain === 0;
  const choix = [['q', 'Dame'], ['r', 'Tour'], ['b', 'Fou'], ['n', 'Cavalier']];
  const boutons = choix.map(([l]) => [GLYPHES[l] + TEXTE, null, l]);
  const promesse = fenetre('Promotion : choisissez la pièce', boutons, 'piece-promo ' + (blanc ? 'blanc' : 'noir'));
  [...$('fenetre-boutons').children].forEach((b, i) => b.setAttribute('aria-label', choix[i][1]));
  return promesse;
}

// --- Doigt / souris ---

function peutJouer() { return R && !etat.finie && !etat.reflechit && R.trait() === etat.campHumain; }

function estAMoi(c) {
  const p = R.plateau()[c];
  return p !== '.' && estBlanc(p) === (etat.campHumain === 0);
}

function caseSous(x, y) {
  const r = plateauEl.getBoundingClientRect();
  const col = Math.floor((x - r.left) / (r.width / 8));
  const ligne = Math.floor((y - r.top) / (r.height / 8));
  if (col < 0 || col > 7 || ligne < 0 || ligne > 7) return -1;
  return etat.retourne ? ligne * 8 + (7 - col) : (7 - ligne) * 8 + col;
}

function essayer(de, vers) {
  const debut = nomCase(de) + nomCase(vers);
  const coups = etat.legaux.filter((m) => m.startsWith(debut));
  if (coups.length === 0) return false;
  if (coups.length === 1) { jouer(coups[0]); return true; }
  choisirPromotion().then((piece) => { if (piece) jouer(debut + piece); });
  return true;
}

let glisse = null; // { c, x, y, fantome }

plateauEl.addEventListener('pointerdown', (e) => {
  if (!peutJouer()) return;
  const c = caseSous(e.clientX, e.clientY);
  if (c < 0) return;
  if (etat.selection >= 0 && etat.selection !== c && essayer(etat.selection, c)) return;
  if (estAMoi(c)) {
    etat.selection = c;
    glisse = { c, x: e.clientX, y: e.clientY, fantome: null };
    plateauEl.setPointerCapture(e.pointerId);
  } else {
    etat.selection = -1;
  }
  dessiner();
});

plateauEl.addEventListener('pointermove', (e) => {
  if (!glisse) return;
  if (!glisse.fantome) {
    if (Math.hypot(e.clientX - glisse.x, e.clientY - glisse.y) < 6) return;
    const f = document.createElement('span');
    f.id = 'fantome';
    f.textContent = cases[glisse.c].piece.textContent;
    f.className = cases[glisse.c].piece.classList.contains('blanc') ? 'blanc' : 'noir';
    document.body.appendChild(f);
    cases[glisse.c].piece.classList.add('cachee');
    glisse.fantome = f;
  }
  glisse.fantome.style.left = e.clientX + 'px';
  glisse.fantome.style.top = e.clientY + 'px';
});

function finGlisse(e, annule) {
  if (!glisse) return;
  const g = glisse;
  glisse = null;
  if (g.fantome) {
    g.fantome.remove();
    cases[g.c].piece.classList.remove('cachee');
    const c = caseSous(e.clientX, e.clientY);
    if (!annule && c >= 0 && c !== g.c && essayer(g.c, c)) return;
  }
  dessiner();
}
plateauEl.addEventListener('pointerup', (e) => finGlisse(e, false));
plateauEl.addEventListener('pointercancel', (e) => finGlisse(e, true));

// --- Boutons ---

$('btn-nouvelle').addEventListener('click', () => fenetre('Nouvelle partie', [
  ['Jouer les Blancs', () => nouvellePartie(0)],
  ['Jouer les Noirs', () => nouvellePartie(1)],
  ['Annuler', null],
]));
$('btn-annuler').addEventListener('click', annulerCoup);
$('btn-tourner').addEventListener('click', () => { etat.retourne = !etat.retourne; sauver(); construirePlateau(); dessiner(); });

const selectNiveau = $('niveau');
NIVEAUX.forEach((nom, i) => selectNiveau.add(new Option(`${i + 1} - ${nom}`, i + 1)));
selectNiveau.addEventListener('change', () => { etat.niveau = Number(selectNiveau.value); sauver(); });

window.addEventListener('resize', ajusterTaille);

// --- Démarrage ---

WakaChessModule().then((M) => {
  R = {
    nouvelle: M.cwrap('wc_nouvelle', null, []),
    fen: M.cwrap('wc_fen', 'number', ['string']),
    jouer: M.cwrap('wc_jouer', 'number', ['string']),
    annuler: M.cwrap('wc_annuler', 'number', []),
    legaux: M.cwrap('wc_legaux', 'string', []),
    plateau: M.cwrap('wc_plateau', 'string', []),
    trait: M.cwrap('wc_trait', 'number', []),
    echec: M.cwrap('wc_echec', 'number', []),
    caseRoi: M.cwrap('wc_case_roi', 'number', []),
    etat: M.cwrap('wc_etat', 'number', []),
  };
  creerMoteur();
  R.nouvelle();

  const s = relire();
  if (s) etat.niveau = Math.min(5, Math.max(1, Number(s.niveau) || 3));
  const fenAdresse = new URLSearchParams(location.search).get('fen');
  if (fenAdresse && R.fen(fenAdresse)) {
    // Position de départ donnée dans l'adresse : ?fen=...
    etat.fen = fenAdresse;
    etat.campHumain = R.trait();
    etat.retourne = etat.campHumain === 1;
    sauver();
  } else if (s && Array.isArray(s.coups)) {
    // Reprise de la partie précédente
    if (s.fen && R.fen(String(s.fen))) etat.fen = String(s.fen);
    etat.campHumain = s.campHumain === 1 ? 1 : 0;
    etat.retourne = !!s.retourne;
    for (const m of s.coups) { if (!R.jouer(String(m))) break; etat.coups.push(String(m)); }
  }
  selectNiveau.value = String(etat.niveau);
  majLegaux();
  ajusterTaille();
  construirePlateau();
  dessiner();
  verifierFin();
  if (!etat.finie && R.trait() !== etat.campHumain) lancerIA();
});

// Fonctionnement hors ligne (pas dans l'application Android, qui embarque déjà les fichiers)
if ('serviceWorker' in navigator && location.protocol === 'https:' && !location.hostname.startsWith('appassets')) {
  navigator.serviceWorker.register('sw.js').catch(() => {});
}
