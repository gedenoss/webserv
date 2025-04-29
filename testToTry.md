test for simultanious POST (formulaire)

for i in $(seq 1 100); do
  curl -s -X POST \
    -H "Content-Type: application/x-www-form-urlencoded" \
    -d "nom=TestNom$i&raison=JusteParceQue$i" \
    "http://localhost:8000/prop/testfile$i.txt" > /dev/null &
done

wait
echo "Toutes les requêtes POST ont été envoyées"

test for simultanious GET 

yes http://127.0.0.1:8080 | head -n100 | xargs -P10 -n1 -I{} curl -s -X POST {} > /dev/null


test for simultanious POST (img)

seq 1 50 | xargs -P10 -I{} curl -s -X POST -F "file=@images/gbouguer.jpg" http://localhost:8000/upload > /dev/null

 test for simultanious POST (2 servers)
 
{ yes http://127.0.0.1:8080/upload | head -n100; yes http://127.0.0.1:8000/upload | head -n100; } | \
xargs -P20 -n1 -I{} curl -s -X POST -F "file=@images/gbouguer.jpg" {} > /dev/null
