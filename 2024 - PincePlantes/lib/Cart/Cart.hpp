#ifndef CART_HPP
#define CART_HPP

#include "ArduPID.h"

// Entrées/sorties
#define PIN_ODO_CODEUR_A 2
#define PIN_ODO_CODEUR_B 3
#define PIN_PID_MONTER 5
#define PIN_PID_DESCENDRE 6

// Constantes de l'odométrie
#define ODO_TICKS_PAR_MILLIMETRE 30         // Nombre de ticks de codeur par millimètre parcourus
#define ODO_TICKS_OFFSET 15000               // Marge pour éviter les under/overflow en butées
#define ODO_SEUIL_TICKS_DEPLACEMENT_NUL 5   // Seuil de variation du nombre de ticks en dessous duquel on considère le chariot immobile 
#define ODO_MARGE_COMMANDE 2                // Marge en dessous de laquelle on considère la consigne atteinte pour éviter les osciallations, plus la marge est élevée plus l'erreur de position est grande
#define ODO_ZERO_TIMEOUT_MS 6000            // Durée max en ms du zéro du chariot au delà de laquelle on considère la manoeuvre en échec

// Constantes du PID 
#define PID_MAX_MOTOR_POWER_ZERO 180  // Puissance max du moteur lors de la phase de zéro, ne pas mettre trop de jus car cherche volontairement à se caler contre la butée inférieure
#define PID_MAX_MOTOR_POWER 255       // Puissance max du moteur lors d'une commande de position
#define PID_P 10
#define PID_I 0.0
#define PID_D 0.10

// Gestion de la vitesse en approche de limites
#define ARRIVAL_BOTTOM_POWER_RATIO 0.7   // Ratio appliqué à la puissance du moteur en approche butée inférieur
#define ARRIVAL_BOTTOM_THRESHOLD_MM 10.  // Distance en millimètre en dessous de laquelle on réduit la puissance du moteur avec le coef ci dessus


class Cart {
    public:
        Cart();

        /**
         * @brief Réalise l'initialisation du chariot : réglage des E/S et reset des compteurs
         * 
         */
        void init();

        /**
         * Routine de comptage des ticks codeur incrément/décrément
         */
        static void odoComptageTicks();

        /**
         * Pilote le moteur du chariot
         * @param commande entre -255 et 255
         * Valeur négative fait reculer le chariot, une valeur positive le fait monter. Une valeur nulle l'arrête
         */
        void commander(double commande);

        /**
         * Calibre le chariot en le déplaçant à l'une ou l'autre de ses extrrêmités
         * Calibration du zéro : déplace le chariot tout en bas.
         * Calibration distance max : déplace le chariot le plus loin possible
         * Les déplacements de chariots sont réalisés à faible vitesse. La calibration s'arrête dès que ce dernier est bloqué (plus ticks codeur)
         * Renvoie "ok" en cas de succès ou "err" + la raison en cas d'erreur
         * @param altMax si true effectue une calibration de la hauteur max, sinon du zéro
         */
        void zero(bool altMax);

        /**
         * Vérifie la position demandée et l'applique au chariot
         * Bloque jusqu'à l'atteinte de la position et renvoie "ok" ou "err" sur la liaison série
         */
        void setPosition(int position);

        /**
         * @brief Effectue un arêt d'urgence du chariot : désactive la puissance envoyée dans le moteur
         * 
         */
        void emergencyStop();

        /**
         * Gère le PID du chariot
         * Renvoie true si la consigne est atteinte, false sinon
         */
        void handlePID();

        /**
         * @brief Renvoie la distance maximale que peut parcourir le chariot si elle a été déterminée avec zero(true), sinon -1
         * 
         * @return int 
         */
        double getDistanceMax();

        /**
         * @brief Renvoie la position courante du chariot
         * 
         * @return double 
         */
        double getPositionCourante();

        /**
         * @brief Indique si un zero du chariot a été fait
         * 
         * @return true 
         * @return false 
         */
        bool isHomeSet();

        /**
         * @brief A appeler périodiquement pour entretenir la boucle d'asservissement
         * 
         */
        void heartBeat();

        /**
         * @brief Indique si le chariot est en movement
         * 
         * @return true 
         * @return false 
         */
        bool isMoving();

    protected:
        // Variables de l'odométrie
        unsigned int odoTicks = ODO_TICKS_OFFSET;       // Compteur de distance du chariot en unité odométrique
        int hauteurMaxMm = -1;                          // Hauteur max en millimètres
        bool odoPrevA;
        bool odoPrevB;
        bool odoCurrA;
        bool odoCurrB;
        bool odoHomeSet = false;                        // Indique si le zéro du chariot a été fait

        // Variables du PID
        ArduPID chariotPID;
        double pidAltitudeCible = 0;
        double pidAltitudeCourante = 0;
        double pidCommandeMoteur = 0;
        double pidP = PID_P;
        double pidI = PID_I;
        double pidD = PID_D;
        bool pidEnabled = false;
        bool pidTargetReached = false;
        double lastCommand;

        /**
         * @brief Instance principale de la classe qui va recevoir les interruptions
         * 
         */
        static Cart* mainInstance;
};

#endif