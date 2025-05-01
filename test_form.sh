#!/bin/bash

# Nombre de requêtes à envoyer
NUM_REQUESTS=10

# URL du formulaire (à adapter à ton serveur)
URL="http://localhost:8000/scrpts/test2"

# Corps du message (tu peux personnaliser ici)
BODY_TEMPLATE="message number"

# Lancer les requêtes en parallèle
for i in $(seq 1 $NUM_REQUESTS); do
  curl -X POST "$URL" \
       -H "Content-Type: application/x-www-form-urlencoded" \
       -d "content=${BODY_TEMPLATE} $i" \
       --silent --output /dev/null &
done

# Attendre que toutes les requêtes soient terminées
wait

echo "$NUM_REQUESTS requêtes POST envoyées en parallèle."
