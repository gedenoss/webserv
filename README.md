### 🚀 **Répartition du projet Webserv en 3 groupes**  

L'idée est de bien séparer les responsabilités tout en assurant une bonne communication entre les parties.  

---

## **🟢 Groupe 1 : Gestion du Réseau et des Connexions**  
👉 **Ce groupe est responsable de la gestion du serveur et des connexions clients.**  

### **Ce qu'il fait :**  
1. **Initialisation du serveur**  
   - Lire la configuration (port, IP, etc.).  
   - Créer les sockets (`socket()`, `bind()`, `listen()`).  

2. **Gestion des connexions clients**  
   - Attendre les connexions entrantes avec `poll()` (ou `select()`, `epoll()`, `kqueue()`).  
   - Accepter les nouvelles connexions (`accept()`).  

3. **Gestion des échanges avec les clients**  
   - Lire les requêtes HTTP (`recv()`).  
   - Envoyer les réponses HTTP (`send()`).  

4. **Surveiller les événements réseau**  
   - Assurer que le serveur ne bloque jamais.  
   - Fermer les connexions inactives.  

---

## **🟡 Groupe 2 : Parsing et Configuration**  
👉 **Ce groupe est chargé du fichier de configuration et du parsing des requêtes.**  

### **Ce qu'il fait :**  
1. **Lire et interpréter le fichier de configuration**  
   - Définir les ports, serveurs, routes, pages d’erreur, limites, etc.  
   - Construire des structures de données claires pour le Groupe 1 et 3.  

2. **Parser les requêtes HTTP**  
   - Lire les headers et le body.  
   - Vérifier la validité de la requête (méthode, URL, version HTTP, etc.).  
   - Extraire les paramètres GET et POST.  

3. **Fournir les bonnes règles au Groupe 3**  
   - Associer la requête à la bonne route et ses règles.  
   - Définir si la requête est valide ou doit être rejetée (ex: méthode non autorisée).  

---

## **🔴 Groupe 3 : Gestion des Requêtes et CGI**  
👉 **Ce groupe traite les requêtes et génère les réponses.**  

### **Ce qu'il fait :**  
1. **Gérer les requêtes statiques**  
   - Vérifier si l’URL demandée correspond à un fichier ou un dossier.  
   - Lire et envoyer le fichier demandé (HTML, CSS, JS, images...).  

2. **Gérer les CGI (PHP, Python, etc.)**  
   - Exécuter un script CGI et récupérer sa sortie (`fork()`, `execve()`).  
   - Vérifier si un `Content-Length` est envoyé ou utiliser EOF.  

3. **Gérer les erreurs HTTP**  
   - Vérifier si le fichier demandé existe (`404 Not Found`).  
   - Vérifier les permissions (`403 Forbidden`).  
   - Vérifier la taille des requêtes (`413 Payload Too Large`).  

4. **Générer et envoyer les réponses HTTP**  
   - Construire les headers (`Content-Type`, `Content-Length`, `Connection`, etc.).  
   - Envoyer le body de la réponse.  

---

## **⚙️ Comment les groupes sont liés ?**  

### **1️⃣ Groupe 1 → Groupe 2**  
📩 **Le serveur reçoit une requête brute (HTTP).**  
- Il la transmet au Groupe 2 pour parsing.  

### **2️⃣ Groupe 2 → Groupe 3**  
🗂 **Le parsing de la requête est terminé.**  
- Le Groupe 2 dit au Groupe 3 quoi faire (servir un fichier, exécuter un CGI, renvoyer une erreur...).  

### **3️⃣ Groupe 3 → Groupe 1**  
📤 **Une réponse est générée.**  
- Le Groupe 3 envoie la réponse HTTP au Groupe 1, qui l’envoie au client avec `send()`.  

---

## **💡 Exemple de cycle complet :**
1. Un client envoie `GET /index.html`  
2. Le **Groupe 1** reçoit la requête et la transmet au **Groupe 2**.  
3. Le **Groupe 2** parse la requête et détermine qu’on doit servir un fichier statique.  
4. Le **Groupe 3** lit le fichier et construit une réponse HTTP avec `200 OK`.  
5. Le **Groupe 1** envoie la réponse au client.  

---

## **🔥 Qui fait quoi dans l’équipe ?**
- **Personne 1 (Groupe 1 - Réseau)** : Fait fonctionner le serveur et les sockets.  
- **Personne 2 (Groupe 2 - Parsing)** : Gère la configuration et l'analyse des requêtes HTTP.  
- **Personne 3 (Groupe 3 - Requêtes & CGI)** : Gère la logique des réponses et le traitement des fichiers.  

🎯 **Chacun a un rôle précis, mais tout doit bien communiquer !**
