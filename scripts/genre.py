#!/usr/bin/env python3

import cgi
import cgitb
cgitb.enable()

form = cgi.FieldStorage()
genre = form.getfirst("genre", "").lower()

print("Content-Type: text/html\r\n")
print()

# Détection du fond et du message selon le genre
if genre == "garcon":
    bg_color = "#cce6ff"  # bleu clair
    message = '<h1 class="king">GO KING 👑🔥💪</h1>'
elif genre == "fille":
    bg_color = "#ffd6e7"  # rose clair
    message = '<h1 class="queen">GO QUEEN 👑💅✨</h1>'
elif genre == "autre":
    bg_color = "#fff9cc"  # jaune clair
    message = '<h1 class="nb">SLAYYY 👑🌈✨</h1>'
else:
    bg_color = "#f0f0f0"
    message = """
        <h1>Tu es un roi, une reine ou autre ?</h1>
        <form method="get" action="/scripts/genre.py">
            <label for="genre">Choisis ton camp :</label><br><br>
            <select name="genre" id="genre">
                <option value="garcon">Garçon</option>
                <option value="fille">Fille</option>
                <option value="autre">Autre</option>
            </select><br><br>
            <button type="submit">Let's go !</button>
        </form>
    """

# HTML complet avec fond dynamique
print(f"""
<!DOCTYPE html>
<html lang="fr">
<head>
    <meta charset="UTF-8">
    <title>Qui es-tu ?</title>
    <style>
        body {{
            font-family: Arial, sans-serif;
            text-align: center;
            padding-top: 50px;
            background-color: {bg_color};
        }}
        .box {{
            display: inline-block;
            padding: 30px;
            border-radius: 15px;
            box-shadow: 0 0 15px rgba(0,0,0,0.1);
            background-color: white;
        }}
        h1 {{
            font-size: 3em;
        }}
        .king {{
            color: #0077cc;
        }}
        .queen {{
            color: #e91e63;
        }}
        .nb {{
            color: #d4af37;
        }}
        form {{
            margin-top: 30px;
        }}
        select, button {{
            font-size: 1.2em;
            padding: 10px 15px;
            border-radius: 8px;
            border: 1px solid #ccc;
        }}
    </style>
</head>
<body>
    <div class="box">
        {message}
    </div>
</body>
</html>
""")
