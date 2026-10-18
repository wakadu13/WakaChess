// sw.js - Met l'application en cache pour qu'elle fonctionne hors ligne
const CACHE = 'wakachess-v1';
const FICHIERS = [
  './', 'index.html', 'style.css', 'app.js', 'moteur.js', 'wakachess.js',
  'manifest.webmanifest', 'icone-192.png', 'icone-512.png', 'apple-touch-icon.png',
];

self.addEventListener('install', (e) => {
  e.waitUntil(caches.open(CACHE).then((c) => c.addAll(FICHIERS)));
  self.skipWaiting();
});

self.addEventListener('activate', (e) => {
  e.waitUntil(caches.keys().then((cles) => Promise.all(cles.filter((k) => k !== CACHE).map((k) => caches.delete(k)))));
  self.clients.claim();
});

// Réponse immédiate depuis le cache, mise à jour en arrière-plan quand le réseau est disponible
self.addEventListener('fetch', (e) => {
  if (e.request.method !== 'GET') return;
  e.respondWith(caches.open(CACHE).then(async (cache) => {
    const enCache = await cache.match(e.request);
    const reseau = fetch(e.request).then((r) => {
      if (r.ok) cache.put(e.request, r.clone());
      return r;
    }).catch(() => enCache);
    return enCache || reseau;
  }));
});
