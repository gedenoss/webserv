# Parsing d'une requête GET en HTTP

## 1. Structure d'une requête GET
Une requête GET en HTTP suit la structure suivante :

```
GET /chemin/ressource HTTP/1.1
Host: www.example.com
[Autres en-têtes optionnelles]
```

Elle est composée de trois parties principales :
1. **Ligne de requête** :
   - Méthode : `GET`
   - URI : `/chemin/ressource`
   - Version : `HTTP/1.1`
2. **En-têtes (Headers)** : Métadonnées sur la requête
3. **Corps (Body)** : Normalement absent dans une requête GET

---

## 2. Headers obligatoires et facultatifs

### **Headers obligatoires**
📌 **`Host`** → Obligatoire en HTTP/1.1, définit le serveur cible.

Exemple :
```
Host: www.example.com
```

Si ce header est absent en HTTP/1.1 → **Erreur 400 Bad Request**.

### **Headers facultatifs courants**

#### **Identification et préférences**
- **`User-Agent`** → Identifie le client (
ex. navigateur, bot)
- **`Accept`** → Types MIME acceptés (`text/html, application/json, */*`)
- **`Accept-Encoding`** → Compression acceptée (`gzip, deflate`)
- **`Accept-Language`** → Langues préférées (`fr-FR, en-US`)

#### **Gestion du cache**
- **`Cache-Control`** → Directive de cache (`no-cache`, `max-age=3600`)
- **`If-Modified-Since` / `If-None-Match`** → Validation de cache

#### **Connexion et authentification**
- **`Connection`** → Gère la connexion (`keep-alive` ou `close`)
- **`Authorization`** → Contient les informations d'authentification

---

## 3. Exemple de requête GET bien formée
```
GET /index.html HTTP/1.1
Host: www.example.com
User-Agent: Mozilla/5.0
Accept: text/html,application/xhtml+xml
Accept-Language: fr-FR,fr;q=0.9
Connection: keep-alive
```

---

## 4. Parsing dans Webserv : Points clés
✅ **Lire la ligne de requête** et vérifier son format (`GET URI HTTP/1.1`)
✅ **Vérifier la présence de `Host`** en HTTP/1.1
✅ **Gérer les headers `Accept` et `Content-Type`**
✅ **Ne pas interpréter de `body`** dans une requête GET
✅ **Renvoyer une erreur `400 Bad Request` si la requête est malformée**

---

## 5. Comportement si `Accept` est absent ou incorrect
- **Pas de header `Accept`** → Accepter tous les types MIME disponibles.
- **Header `Accept` présent** → S'assurer que le `Content-Type` de la réponse est compatible.
- **Exemple d'analyse du header `Accept`** :
  - `Accept: image/*` → Accepter `image/png`, `image/jpeg`, etc.
  - `Accept: text/html,application/json` → N'envoyer que l'un de ces formats.
  - `Accept: */*` → Tout type est accepté.

---

## 6. Erreurs possibles
- **400 Bad Request** → Requête mal formée (ex. ligne incorrecte, `Host` absent en HTTP/1.1).
- **404 Not Found** → Fichier demandé introuvable.
- **403 Forbidden** → Permissions insuffisantes.
- **406 Not Acceptable** → Aucun `Content-Type` disponible pour le `Accept` fourni.

---

✅ **Objectif dans Webserv** : Implémenter un parsing robuste qui valide la syntaxe, vérifie les headers, et gère correctement les erreurs ! 🚀

