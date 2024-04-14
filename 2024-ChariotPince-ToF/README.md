# 2024 - Pince sur courroie + ToF
Contient le sketch Arduino permettant de piloter la pince sur courroie du robot principal. Il permet également 
d'intéragir avec un array de capteurs Time Of Flight afin de détecter la position d'un objet cible devant le robot.
Communication au travers d'une liaison série :

## Utilisation
Envoyer en une fois "\<commande>\<argument>\n" :
| Commande | Argument | Description |
|--|--|--|
| z | *aucun* | Place l'ascenseur tout en bas de son axe et reset la hauteur courante à 0mm |
| Z | *aucun* | Place l'ascenseur tout en haut de son axe pour détemriner la hauteur max de l'ascenseur |
| g | hauteur en mm | Place l'ascenseur à la position demandée et renvoie 'ok' ou 'err' |
| a | *aucun* | Renvoie la position courante de l'ascenseur en mm sur la liaison série |
| A | *aucun* | Renvoie la hauteur max de l'ascenseur si elle a été déteminée par la commande Z, sinon -1 |
| h | *aucun* | Arrêt d'urgence de l'ascenseur : désactive la puissance dans le moteurs jusqu'au prochain 'z' ou 'g' |
| d | numéro du ToF | Renvoie la distance (en mm) perçue par le ToF demandé |
| D | *aucun* | Renvoie les distances (en mm) perçues par les différents ToF |
| f | *aucun* | Renvoie la distance (en mm) au premier obstacle perçu par l'array de ToF |


## Bugs connus
1. Lors d'un reset de l'ascenseur, un bug empêche d'accéder à la valeur "à jour" de la valeur des ticks de l'odométrie. Une solution de contournement consiste à écrire des espaces sur la liaison série avec Serial.print(" ") juste avant l'accès à la variable. Cela oblige à ignorer les espaces renvoyés par cette commande du côté du programme qui interagit avec l'ascenseur.
