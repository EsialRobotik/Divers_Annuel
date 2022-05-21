# Ascenseur Sonde
Contient le sketch Arduino permettant de piloter l'ascenseur et la sonde des carrés de fouille du robot principal.
Communication au travers d'une liaison série :
## Utilisation
Envoyer en une fois "\<commande>\<argument>\n" :
| Commande | Argument | Description |
|--|--|--|
| z | *aucun* | Place l'ascenseur tout en bas de son axe et reset à 0 la hauteur courante |
| g | hauteur en mm | Place l'ascenseur à la position demandée et renvoie 'ok' ou 'err' |
| a | *aucun* | Renvoie la position courante de l'ascenseur en mm sur la liaison série |
| h | *aucun* | Arrêt d'urgence de l'ascenseur : désactive la puissance dans le moteurs jusqu'au prochain 'z' ou 'g' |
| m | *aucun* | Renvoie la hauteur max de l'ascenseur |
| s | *aucun* | Renvoie la valeur de la sonde du carré de fouille

## Bugs connus
1. Lors d'un reset de l'ascenseur, un bug empêche d'accéder à la valeur "à jour" de la valeur des ticks de l'odométrie. Une solution de contournement consiste à écrire des espaces sur la liaison série avec Serial.print(" ") juste avant l'accès à la variable. Cela oblige à ignorer les espaces renvoyés par cette commande du côté du programme qui interagit avec l'ascenseur.
