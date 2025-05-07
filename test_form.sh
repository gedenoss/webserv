#!/bin/bash

# Nombre de requêtes à envoyer
NUM_REQUESTS=10

# URLs des serveurs (un sur le port 8000 et un sur le port 8080)
URL_8000="http://localhost:8000/test2"
URL_8080="http://localhost:8080/test2"

# Corps du message (tu peux personnaliser ici)
BODY_TEMPLATE="message number"

# Lancer les requêtes en parallèle pour les deux serveurs
for i in $(seq 1 $NUM_REQUESTS); do
  curl -X POST "$URL_8000" \
       -H "Content-Type: application/x-www-form-urlencoded" \
       -d "content=${BODY_TEMPLATE} $i from port 8000" \
       --silent --output /dev/null &

  curl -X POST "$URL_8080" \
       -H "Content-Type: application/x-www-form-urlencoded" \
       -d "content=${BODY_TEMPLATE} $i from port 8080" \
       --silent --output /dev/null &
done

# Attendre que toutes les requêtes soient terminées
wait

echo "$NUM_REQUESTS requêtes POST envoyées en parallèle sur les ports 8000 et 8080."
