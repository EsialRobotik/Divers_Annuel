# Pince à plantes
Contient le sketch Arduino permettant de piloter la pince à plante de Mamma Princess 2024
Communication au travers d'une liaison série :

## Utilisation
Envoyer en une fois "\<commande>\<argument>\n" :
| Commande | Argument | Description |
|--|--|--|
| z | *aucun* | Effectue un zéro du chariot : le place en butée côté moteur et reset la distance courante à 0mm |
| Z | *aucun* | Place le chariot à l'extrêmité côté poulie pour détemriner la distance max disponible |
| g | distance en mm | Place le chariot à la position demandée et renvoie 'ok' ou 'err' |
| a | *aucun* | Renvoie la position courante du chariot en mm sur la liaison série |
| A | *aucun* | Renvoie la distance  max que le chariot peut parcourir si elle a été déteminée par la commande Z, sinon -1 |
| h | *aucun* | Arrêt d'urgence du chariot : désactive la puissance dans le moteurs jusqu'au prochain 'z' ou 'g' |
| l | *aucun* | Lit la ligne de ToF et renvoie toutes les distances sur la liaison série |
| f | *aucun* | Cherche une plante et place le chariot devant elle. Ne déplace pas le chariot et renvoie 'ko' si aucune plante trouvée ou 'ok' une fois le chariot placé (il peut donc y avoir un délais de quelques secondes avant un 'ok') |

## Bugs connus
1. Lors d'un reset de du chariot, un bug empêche d'accéder à la valeur "à jour" de la valeur des ticks de l'odométrie. Une solution de contournement consiste à écrire des espaces sur la liaison série avec Serial.print(" ") juste avant l'accès à la variable. Cela oblige à ignorer les espaces renvoyés par cette commande du côté du programme qui interagit avec le chariot.
