<?php
// Définir les headers HTTP valides pour une réponse CGI
header('Content-Type: text/html; charset=UTF-8');

// Afficher QUERY_STRING directement pour déboguer
echo "<!-- QUERY_STRING: " . $_SERVER['QUERY_STRING'] . " -->\n";

// Décomposer la chaîne de requête QUERY_STRING dans $_GET
parse_str($_SERVER['QUERY_STRING'], $_GET);

// Commencer la sortie HTML
echo "<!DOCTYPE html>\n";
echo "<html lang=\"fr\">\n";
echo "<head>\n";
echo "    <meta charset=\"UTF-8\">\n";
echo "    <title>CGI PHP Test</title>\n";
echo "</head>\n";
echo "<body>\n";

// Vérifier si le formulaire a été soumis
if ($_SERVER['REQUEST_METHOD'] === 'GET' && isset($_GET['name'])) {
    $name = htmlspecialchars($_GET['name']);
    echo "<h1>Bonjour, " . $name . "!</h1>\n";
} else {
    echo "<h1>Bienvenue sur le script CGI PHP</h1>\n";
    echo "<form method='get' action=''>\n";
    echo "    <label for=\"name\">Entrez votre nom:</label>\n";
    echo "    <input type='text' name='name' id='name' />\n";
    echo "    <input type='submit' value='Envoyer' />\n";
    echo "</form>\n";
}

echo "</body>\n";
echo "</html>\n";
?>
