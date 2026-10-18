package fr.wakachess;

import android.app.Activity;
import android.graphics.Color;
import android.os.Bundle;
import android.webkit.WebResourceRequest;
import android.webkit.WebResourceResponse;
import android.webkit.WebSettings;
import android.webkit.WebView;
import android.webkit.WebViewClient;

import androidx.webkit.WebViewAssetLoader;

// Affiche la version web de WakaChess (moteur C++ compilé en WebAssembly), embarquée dans l'application :
// tout fonctionne hors ligne.
public class MainActivity extends Activity {
    private WebView vue;

    @Override
    protected void onCreate(Bundle etat) {
        super.onCreate(etat);
        getWindow().setStatusBarColor(Color.parseColor("#302e2b"));
        getWindow().setNavigationBarColor(Color.parseColor("#302e2b"));

        // Sert les fichiers de assets/ sous https://appassets.androidplatform.net/ (nécessaire pour le Web Worker)
        final WebViewAssetLoader chargeur = new WebViewAssetLoader.Builder()
                .addPathHandler("/", new WebViewAssetLoader.AssetsPathHandler(this))
                .build();

        vue = new WebView(this);
        vue.setBackgroundColor(Color.parseColor("#302e2b"));
        WebSettings reglages = vue.getSettings();
        reglages.setJavaScriptEnabled(true);
        reglages.setDomStorageEnabled(true); // sauvegarde de la partie en cours
        vue.setWebViewClient(new WebViewClient() {
            @Override
            public WebResourceResponse shouldInterceptRequest(WebView v, WebResourceRequest requete) {
                return chargeur.shouldInterceptRequest(requete.getUrl());
            }
        });
        setContentView(vue);

        if (etat != null) vue.restoreState(etat);
        else vue.loadUrl("https://appassets.androidplatform.net/www/index.html");
    }

    @Override
    protected void onSaveInstanceState(Bundle sortie) {
        super.onSaveInstanceState(sortie);
        vue.saveState(sortie);
    }
}
