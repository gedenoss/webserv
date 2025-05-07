#!/usr/bin/python3

import sys
import os

# Lire la taille des données
length = int(os.environ.get('CONTENT_LENGTH', 0))
post_data = sys.stdin.read(length)

# Répondre au client
print("Content-Type: text/plain\r\n")
print(f"MAUVAIS TEST\n"
