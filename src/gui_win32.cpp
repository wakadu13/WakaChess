// gui_win32.cpp - Interface graphique Windows : on joue entièrement à la souris
//   - cliquer une pièce puis sa case d'arrivée (ou la faire glisser)
//   - menus : nouvelle partie (Blancs / Noirs), annuler le coup, tourner l'échiquier, niveau
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>

#include "board.hpp"
#include "search.hpp"

#include <algorithm>
#include <atomic>
#include <cstdlib>
#include <memory>
#include <string>
#include <thread>
#include <utility>
#include <vector>

namespace {

enum : UINT {
    ID_BLANCS = 100, ID_NOIRS, ID_ANNULER, ID_TOURNER, ID_QUITTER,
    ID_NIVEAU = 200, // + niveau (1 à 5)
};
constexpr UINT WM_COUP_IA = WM_APP + 1;
constexpr int HAUTEUR_STATUT = 44;

const COLORREF COULEUR_CLAIRE = RGB(240, 217, 181);
const COLORREF COULEUR_SOMBRE = RGB(181, 136, 99);
const COLORREF COULEUR_DERNIER = RGB(205, 210, 106);
const COLORREF COULEUR_SELECTION = RGB(246, 246, 105);
const COLORREF COULEUR_ECHEC = RGB(235, 97, 80);
const COLORREF COULEUR_FOND = RGB(48, 46, 43);

struct Jeu {
    Position pos;
    std::vector<std::pair<Coup, Annulation>> partie;
    Coup legaux[MAX_COUPS];
    int nbLegaux = 0;
    int campHumain = BLANC;
    int niveau = 3;
    bool retourne = false;
    int selection = -1;
    bool glisse = false;
    bool finie = false;
    std::wstring resultat;

    std::thread fil;
    std::atomic<bool> arret{false};
    bool iaReflechit = false;
    int generation = 0; // pour ignorer le résultat d'une réflexion annulée
};

Jeu J;
HWND fenetre = nullptr;
std::wstring policeSymboles = L"Segoe UI Symbol";

// Première police installée qui contient vraiment les symboles d'échecs (♔ à ♟)
void choisirPoliceSymboles() {
    const wchar_t* candidates[] = {L"Segoe UI Symbol", L"Segoe UI Emoji", L"Arial Unicode MS",
                                   L"DejaVu Sans", L"FreeSerif", L"Noto Sans Symbols2", L"Symbola"};
    HDC dc = GetDC(nullptr);
    for (const wchar_t* nom : candidates) {
        HFONT police = CreateFontW(-32, 0, 0, 0, FW_NORMAL, 0, 0, 0, DEFAULT_CHARSET, 0, 0, 0, 0, nom);
        HGDIOBJ ancienne = SelectObject(dc, police);
        wchar_t nomReel[LF_FACESIZE] = {};
        GetTextFaceW(dc, LF_FACESIZE, nomReel);
        WORD glyphes[12];
        const wchar_t symboles[12] = {0x2654, 0x2655, 0x2656, 0x2657, 0x2658, 0x2659,
                                      0x265A, 0x265B, 0x265C, 0x265D, 0x265E, 0x265F};
        bool complet = GetGlyphIndicesW(dc, symboles, 12, glyphes, GGI_MARK_NONEXISTING_GLYPHS) == 12;
        for (WORD g : glyphes) complet = complet && g != 0xFFFF;
        SelectObject(dc, ancienne);
        DeleteObject(police);
        if (complet && lstrcmpiW(nomReel, nom) == 0) { policeSymboles = nom; break; }
    }
    ReleaseDC(nullptr, dc);
}

std::wstring versLarge(const std::string& s) {
    const int n = MultiByteToWideChar(CP_UTF8, 0, s.c_str(), -1, nullptr, 0);
    std::wstring w(n > 0 ? n - 1 : 0, L'\0');
    if (n > 1) MultiByteToWideChar(CP_UTF8, 0, s.c_str(), -1, &w[0], n);
    return w;
}

void redessiner() {
    const std::wstring titre = L"WakaChess  —  Niveau : " + versLarge(nomNiveau(J.niveau));
    SetWindowTextW(fenetre, titre.c_str());
    InvalidateRect(fenetre, nullptr, FALSE);
}

void arreterIA() {
    J.arret = true;
    if (J.fil.joinable()) J.fil.join();
    J.iaReflechit = false;
}

void lancerIA() {
    arreterIA();
    J.arret = false;
    J.iaReflechit = true;
    const int generation = ++J.generation;
    auto copie = std::make_shared<Position>(J.pos);
    const LimitesRecherche limites = limitesPourNiveau(J.niveau);
    J.fil = std::thread([copie, limites, generation] {
        auto* res = new ResultatRecherche(rechercher(*copie, limites, J.arret));
        PostMessageW(fenetre, WM_COUP_IA, static_cast<WPARAM>(generation), reinterpret_cast<LPARAM>(res));
    });
    redessiner();
}

void verifierFin() {
    switch (etatPartie(J.pos)) {
        case MAT: J.resultat = J.pos.trait == BLANC ? L"Échec et mat : les Noirs gagnent !" : L"Échec et mat : les Blancs gagnent !"; break;
        case PAT: J.resultat = L"Pat : partie nulle."; break;
        case NULLE_REPETITION: J.resultat = L"Partie nulle par triple répétition."; break;
        case NULLE_50_COUPS: J.resultat = L"Partie nulle (règle des 50 coups)."; break;
        case NULLE_MATERIEL: J.resultat = L"Partie nulle (matériel insuffisant)."; break;
        case EN_COURS: return;
    }
    J.finie = true;
    redessiner();
    UpdateWindow(fenetre);
    MessageBoxW(fenetre, J.resultat.c_str(), L"WakaChess", MB_OK | MB_ICONINFORMATION);
}

void jouerCoup(const Coup& m) {
    Annulation a;
    J.pos.jouer(m, a);
    J.partie.emplace_back(m, a);
    J.selection = -1;
    J.nbLegaux = J.pos.genererLegaux(J.legaux);
    redessiner();
    verifierFin();
    if (!J.finie && J.pos.trait != J.campHumain) lancerIA();
}

void nouvellePartie(int campHumain) {
    arreterIA();
    J.pos.positionInitiale();
    J.partie.clear();
    J.campHumain = campHumain;
    J.retourne = (campHumain == NOIR);
    J.selection = -1;
    J.finie = false;
    J.resultat.clear();
    J.nbLegaux = J.pos.genererLegaux(J.legaux);
    viderTT();
    redessiner();
    if (J.pos.trait != J.campHumain) lancerIA();
}

void annulerCoup() {
    arreterIA();
    if (J.partie.empty()) return;
    do {
        J.pos.annuler(J.partie.back().first, J.partie.back().second);
        J.partie.pop_back();
    } while (!J.partie.empty() && J.pos.trait != J.campHumain);
    J.selection = -1;
    J.finie = false;
    J.resultat.clear();
    J.nbLegaux = J.pos.genererLegaux(J.legaux);
    redessiner();
    if (J.pos.trait != J.campHumain) lancerIA();
}

// --- Géométrie de l'échiquier ---

int tailleCase() {
    RECT rc;
    GetClientRect(fenetre, &rc);
    return std::max(8, static_cast<int>(std::min(rc.right, rc.bottom - HAUTEUR_STATUT) / 8));
}

POINT origine() {
    RECT rc;
    GetClientRect(fenetre, &rc);
    const int t = tailleCase();
    return POINT{(rc.right - 8 * t) / 2, std::max(0L, (rc.bottom - HAUTEUR_STATUT - 8 * t) / 2)};
}

RECT rectCase(int c) {
    const int t = tailleCase();
    const POINT o = origine();
    int col = colonne(c), lig = 7 - rangee(c);
    if (J.retourne) { col = 7 - col; lig = 7 - lig; }
    return RECT{o.x + col * t, o.y + lig * t, o.x + (col + 1) * t, o.y + (lig + 1) * t};
}

int caseSous(int x, int y) {
    const int t = tailleCase();
    const POINT o = origine();
    if (x < o.x || y < o.y || x >= o.x + 8 * t || y >= o.y + 8 * t) return -1;
    int col = (x - o.x) / t, lig = (y - o.y) / t;
    if (J.retourne) { col = 7 - col; lig = 7 - lig; }
    return caseDe(col, 7 - lig);
}

// --- Souris ---

bool estAMoi(int c) {
    const int8_t p = J.pos.plateau[c];
    return p != VIDE && (p > 0) == (J.campHumain == BLANC);
}

bool essayerCoup(int de, int vers) {
    const Coup* trouve = nullptr;
    for (int i = 0; i < J.nbLegaux; ++i)
        if (J.legaux[i].de == de && J.legaux[i].vers == vers) { trouve = &J.legaux[i]; break; }
    if (!trouve) return false;

    Coup m = *trouve;
    if (m.promo) {
        HMENU menu = CreatePopupMenu();
        AppendMenuW(menu, MF_STRING, DAME, L"Dame");
        AppendMenuW(menu, MF_STRING, TOUR, L"Tour");
        AppendMenuW(menu, MF_STRING, FOU, L"Fou");
        AppendMenuW(menu, MF_STRING, CAVALIER, L"Cavalier");
        POINT p;
        GetCursorPos(&p);
        const int choix = TrackPopupMenu(menu, TPM_RETURNCMD | TPM_NONOTIFY, p.x, p.y, 0, fenetre, nullptr);
        DestroyMenu(menu);
        if (choix == 0) return true; // menu fermé sans choix
        m.promo = static_cast<uint8_t>(choix);
    }
    jouerCoup(m);
    return true;
}

void clicEnfonce(int x, int y) {
    if (J.finie || J.iaReflechit || J.pos.trait != J.campHumain) return;
    const int c = caseSous(x, y);
    if (c < 0) return;
    if (J.selection >= 0 && J.selection != c && essayerCoup(J.selection, c)) return;
    if (estAMoi(c)) { J.selection = c; J.glisse = true; SetCapture(fenetre); }
    else J.selection = -1;
    redessiner();
}

void clicRelache(int x, int y) {
    if (!J.glisse) return;
    J.glisse = false;
    ReleaseCapture();
    const int c = caseSous(x, y);
    if (J.selection >= 0 && c >= 0 && c != J.selection) essayerCoup(J.selection, c); // glisser-déposer
}

// --- Dessin ---

void remplir(HDC dc, const RECT& r, COLORREF couleur) {
    HBRUSH b = CreateSolidBrush(couleur);
    FillRect(dc, &r, b);
    DeleteObject(b);
}

COLORREF melanger(COLORREF a, COLORREF b) {
    return RGB((GetRValue(a) + GetRValue(b)) / 2, (GetGValue(a) + GetGValue(b)) / 2, (GetBValue(a) + GetBValue(b)) / 2);
}

void dessinerPiece(HDC dc, RECT r, int8_t p) {
    // Symboles Unicode des échecs : contour (♔ U+2654...) et plein (♚ U+265A...)
    static const int decalage[7] = {0, 5, 4, 3, 2, 1, 0}; // PION..ROI -> ordre Roi, Dame, Tour, Fou, Cavalier, Pion
    const wchar_t plein = static_cast<wchar_t>(0x265A + decalage[std::abs(p)]);
    const wchar_t contour = static_cast<wchar_t>(0x2654 + decalage[std::abs(p)]);
    const UINT format = DT_CENTER | DT_VCENTER | DT_SINGLELINE | DT_NOPREFIX;
    if (p > 0) {
        SetTextColor(dc, RGB(255, 255, 255));
        DrawTextW(dc, &plein, 1, &r, format);
        SetTextColor(dc, RGB(0, 0, 0));
        DrawTextW(dc, &contour, 1, &r, format);
    } else {
        SetTextColor(dc, RGB(0, 0, 0));
        DrawTextW(dc, &plein, 1, &r, format);
    }
}

std::wstring texteStatut() {
    if (J.finie) return J.resultat;
    std::wstring s;
    if (J.iaReflechit) s = L"L'IA réfléchit…";
    else s = J.campHumain == BLANC ? L"À vous de jouer (Blancs)" : L"À vous de jouer (Noirs)";
    if (J.pos.enEchec(J.pos.trait)) s += L"   —   Échec !";
    if (!J.partie.empty()) s += L"   —   Dernier coup : " + versLarge(coupVersTexte(J.partie.back().first));
    return s;
}

void peindre(HDC ecran) {
    RECT rc;
    GetClientRect(fenetre, &rc);
    HDC dc = CreateCompatibleDC(ecran);
    HBITMAP bmp = CreateCompatibleBitmap(ecran, rc.right, rc.bottom);
    HGDIOBJ ancienBmp = SelectObject(dc, bmp);
    remplir(dc, rc, COULEUR_FOND);
    SetBkMode(dc, TRANSPARENT);

    const int t = tailleCase();
    const Coup* dernier = J.partie.empty() ? nullptr : &J.partie.back().first;
    const int caseRoiEchec = J.pos.enEchec(J.pos.trait) ? J.pos.caseRoi[J.pos.trait] : -1;

    HFONT policeCoord = CreateFontW(-std::max(9, t / 6), 0, 0, 0, FW_BOLD, 0, 0, 0, DEFAULT_CHARSET, 0, 0,
                                    CLEARTYPE_QUALITY, 0, L"Segoe UI");
    HFONT policePiece = CreateFontW(-t * 4 / 5, 0, 0, 0, FW_NORMAL, 0, 0, 0, DEFAULT_CHARSET, 0, 0,
                                    CLEARTYPE_QUALITY, 0, policeSymboles.c_str());

    for (int c = 0; c < 64; ++c) {
        const RECT r = rectCase(c);
        const bool claire = (colonne(c) + rangee(c)) % 2 == 1;
        COLORREF couleur = claire ? COULEUR_CLAIRE : COULEUR_SOMBRE;
        if (dernier && (c == dernier->de || c == dernier->vers)) couleur = melanger(couleur, COULEUR_DERNIER);
        if (c == J.selection) couleur = melanger(couleur, COULEUR_SELECTION);
        if (c == caseRoiEchec) couleur = COULEUR_ECHEC;
        remplir(dc, r, couleur);

        // Coordonnées sur les bords
        SelectObject(dc, policeCoord);
        SetTextColor(dc, claire ? COULEUR_SOMBRE : COULEUR_CLAIRE);
        const int colAffichee = J.retourne ? 7 - colonne(c) : colonne(c);
        const int ligAffichee = J.retourne ? rangee(c) : 7 - rangee(c);
        if (ligAffichee == 7) {
            const wchar_t lettre = static_cast<wchar_t>(L'a' + colonne(c));
            RECT rr = r; rr.right -= 3; rr.bottom -= 1;
            DrawTextW(dc, &lettre, 1, &rr, DT_RIGHT | DT_BOTTOM | DT_SINGLELINE);
        }
        if (colAffichee == 0) {
            const wchar_t chiffre = static_cast<wchar_t>(L'1' + rangee(c));
            RECT rr = r; rr.left += 3; rr.top += 1;
            DrawTextW(dc, &chiffre, 1, &rr, DT_LEFT | DT_TOP | DT_SINGLELINE);
        }
    }

    // Pièces
    SelectObject(dc, policePiece);
    for (int c = 0; c < 64; ++c)
        if (J.pos.plateau[c] != VIDE) dessinerPiece(dc, rectCase(c), J.pos.plateau[c]);

    // Cases d'arrivée possibles de la pièce sélectionnée
    if (J.selection >= 0) {
        HBRUSH pinceau = CreateSolidBrush(RGB(90, 90, 90));
        HGDIOBJ ancienPinceau = SelectObject(dc, pinceau);
        HGDIOBJ ancienStylo = SelectObject(dc, GetStockObject(NULL_PEN));
        for (int i = 0; i < J.nbLegaux; ++i) {
            if (J.legaux[i].de != J.selection) continue;
            const RECT r = rectCase(J.legaux[i].vers);
            const int cx = (r.left + r.right) / 2, cy = (r.top + r.bottom) / 2;
            const int rayon = (J.legaux[i].flags & F_CAPTURE) ? t / 2 - 2 : t / 7;
            if (J.legaux[i].flags & F_CAPTURE) { // anneau autour de la pièce à prendre
                HGDIOBJ s = SelectObject(dc, CreatePen(PS_SOLID, std::max(2, t / 14), RGB(90, 90, 90)));
                HGDIOBJ p = SelectObject(dc, GetStockObject(NULL_BRUSH));
                Ellipse(dc, cx - rayon, cy - rayon, cx + rayon, cy + rayon);
                SelectObject(dc, p);
                DeleteObject(SelectObject(dc, s));
            } else {
                Ellipse(dc, cx - rayon, cy - rayon, cx + rayon, cy + rayon);
            }
        }
        SelectObject(dc, ancienStylo);
        SelectObject(dc, ancienPinceau);
        DeleteObject(pinceau);
    }

    // Barre d'état
    HFONT policeStatut = CreateFontW(-18, 0, 0, 0, FW_SEMIBOLD, 0, 0, 0, DEFAULT_CHARSET, 0, 0,
                                     CLEARTYPE_QUALITY, 0, L"Segoe UI");
    SelectObject(dc, policeStatut);
    SetTextColor(dc, J.finie ? RGB(255, 210, 90) : RGB(235, 235, 235));
    RECT rs{12, rc.bottom - HAUTEUR_STATUT, rc.right - 12, rc.bottom};
    const std::wstring statut = texteStatut();
    DrawTextW(dc, statut.c_str(), static_cast<int>(statut.size()), &rs, DT_LEFT | DT_VCENTER | DT_SINGLELINE);

    BitBlt(ecran, 0, 0, rc.right, rc.bottom, dc, 0, 0, SRCCOPY);
    SelectObject(dc, ancienBmp);
    DeleteObject(bmp);
    DeleteObject(policeCoord);
    DeleteObject(policePiece);
    DeleteObject(policeStatut);
    DeleteDC(dc);
}

HMENU creerMenu() {
    HMENU partie = CreatePopupMenu();
    AppendMenuW(partie, MF_STRING, ID_BLANCS, L"Nouvelle partie avec les &Blancs");
    AppendMenuW(partie, MF_STRING, ID_NOIRS, L"Nouvelle partie avec les &Noirs");
    AppendMenuW(partie, MF_SEPARATOR, 0, nullptr);
    AppendMenuW(partie, MF_STRING, ID_ANNULER, L"&Annuler le dernier coup");
    AppendMenuW(partie, MF_STRING, ID_TOURNER, L"&Tourner l'échiquier");
    AppendMenuW(partie, MF_SEPARATOR, 0, nullptr);
    AppendMenuW(partie, MF_STRING, ID_QUITTER, L"&Quitter");

    HMENU niveaux = CreatePopupMenu();
    for (int n = 1; n <= NIVEAU_MAX; ++n) {
        const std::wstring nom = std::to_wstring(n) + L" - " + versLarge(nomNiveau(n));
        AppendMenuW(niveaux, MF_STRING, ID_NIVEAU + n, nom.c_str());
    }
    CheckMenuRadioItem(niveaux, ID_NIVEAU + 1, ID_NIVEAU + NIVEAU_MAX, ID_NIVEAU + J.niveau, MF_BYCOMMAND);

    HMENU barre = CreateMenu();
    AppendMenuW(barre, MF_POPUP, reinterpret_cast<UINT_PTR>(partie), L"&Partie");
    AppendMenuW(barre, MF_POPUP, reinterpret_cast<UINT_PTR>(niveaux), L"&Niveau");
    return barre;
}

LRESULT CALLBACK procedure(HWND h, UINT msg, WPARAM wp, LPARAM lp) {
    switch (msg) {
    case WM_PAINT: {
        PAINTSTRUCT ps;
        HDC dc = BeginPaint(h, &ps);
        peindre(dc);
        EndPaint(h, &ps);
        return 0;
    }
    case WM_ERASEBKGND: return 1;
    case WM_SIZE: redessiner(); return 0;
    case WM_GETMINMAXINFO:
        reinterpret_cast<MINMAXINFO*>(lp)->ptMinTrackSize = POINT{360, 440};
        return 0;
    case WM_LBUTTONDOWN: clicEnfonce(static_cast<short>(LOWORD(lp)), static_cast<short>(HIWORD(lp))); return 0;
    case WM_LBUTTONUP: clicRelache(static_cast<short>(LOWORD(lp)), static_cast<short>(HIWORD(lp))); return 0;
    case WM_COUP_IA: {
        std::unique_ptr<ResultatRecherche> res(reinterpret_cast<ResultatRecherche*>(lp));
        if (static_cast<int>(wp) != J.generation || !J.iaReflechit) return 0; // réflexion annulée entre-temps
        if (J.fil.joinable()) J.fil.join();
        J.iaReflechit = false;
        if (res->trouve) jouerCoup(res->coup);
        return 0;
    }
    case WM_COMMAND: {
        const UINT id = LOWORD(wp);
        if (id == ID_BLANCS) nouvellePartie(BLANC);
        else if (id == ID_NOIRS) nouvellePartie(NOIR);
        else if (id == ID_ANNULER) annulerCoup();
        else if (id == ID_TOURNER) { J.retourne = !J.retourne; redessiner(); }
        else if (id == ID_QUITTER) DestroyWindow(h);
        else if (id > ID_NIVEAU && id <= ID_NIVEAU + NIVEAU_MAX) {
            J.niveau = static_cast<int>(id - ID_NIVEAU);
            HMENU niveaux = GetSubMenu(GetMenu(h), 1);
            CheckMenuRadioItem(niveaux, ID_NIVEAU + 1, ID_NIVEAU + NIVEAU_MAX, id, MF_BYCOMMAND);
            redessiner();
        }
        return 0;
    }
    case WM_DESTROY:
        arreterIA();
        PostQuitMessage(0);
        return 0;
    }
    return DefWindowProcW(h, msg, wp, lp);
}

} // namespace

// Facultatif : une position FEN en argument pour commencer à partir de celle-ci
int WINAPI WinMain(HINSTANCE instance, HINSTANCE, LPSTR ligneCommande, int affichage) {
    SetProcessDPIAware();
    choisirPoliceSymboles();

    WNDCLASSW wc{};
    wc.lpfnWndProc = procedure;
    wc.hInstance = instance;
    wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wc.hIcon = LoadIcon(nullptr, IDI_APPLICATION);
    wc.lpszClassName = L"WakaChess";
    RegisterClassW(&wc);

    const DWORD style = WS_OVERLAPPEDWINDOW;
    RECT r{0, 0, 8 * 80, 8 * 80 + HAUTEUR_STATUT};
    AdjustWindowRect(&r, style, TRUE);
    fenetre = CreateWindowExW(0, L"WakaChess", L"WakaChess", style, CW_USEDEFAULT, CW_USEDEFAULT,
                              r.right - r.left, r.bottom - r.top, nullptr, creerMenu(), instance, nullptr);
    if (!fenetre) return 1;

    J.pos.positionInitiale();
    if (ligneCommande && *ligneCommande) {
        std::string fen(ligneCommande);
        fen.erase(std::remove(fen.begin(), fen.end(), '"'), fen.end());
        if (!J.pos.chargerFEN(fen)) J.pos.positionInitiale();
        J.campHumain = J.pos.trait;
        J.retourne = (J.campHumain == NOIR);
    }
    J.nbLegaux = J.pos.genererLegaux(J.legaux);
    ShowWindow(fenetre, affichage);
    UpdateWindow(fenetre);

    MSG msg;
    while (GetMessageW(&msg, nullptr, 0, 0) > 0) {
        TranslateMessage(&msg);
        DispatchMessageW(&msg);
    }
    return 0;
}
