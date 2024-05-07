#ifndef PLANTMANIPULATOR_HPP
#define PLANTMANIPULATOR_HPP

#include <Stream.h>
#include <TimeOfFlightArray.hpp>
#include <Cart.hpp>

// Distances min et max entre lesquelles on considère qu'une plante est présente devant un ToF et est atteignable
#define PLANTPANIULATOR_MIN_DISTANCE_MM 100
#define PLANTPANIULATOR_MAX_DISTANCE_MM 200

/**
 * @brief Détection et manipulation des plantes
 * 
 */
class PlantManipulator {

    public:
        /**
         * @brief Construct a new Plant Manipulator object
         * 
         * @param serial Liaison série à utiliser pour écrire des résultats
         * @param tofArrayCount Nombre de segment de ToF, doit être positif strictement.
         * @param tofArrays Liste des segments de ToF supposés agencés les uns à la suite des autres. Doit contenir tofArrayCount éléments.
         * @param distanceByTof Tableau de correspondance entre l'index d'un ToF et sa distance corespondante sur le chariot. Doit contenir tofArrayCount éléments.
         */
        PlantManipulator(Stream* serial, Cart* cart, short tofArrayCount, TimeOfFlightArray** tofArrays, int* distanceByToF);

        /**
         * @brief Effectue une détection des plantes se trouvant devant la pince.
         * Tente ensuite de positionner le chariot en face de la plante la plus proche trouvée, de manière non bloquantes : un ordre de placment est envoyé au chariot.
         * 
         * @return true Si une plante est trouvé
         * @return false Si aucune plante n'est trouvée
         */
        bool faceNextClosestPlantAsync();

        /**
         * @brief Effetcue une acquisition sur tous les ToF disponibles et écrit le résultat sur la liaison série
         * 
         */
        void acquireAndPrintLine();

        /**
         * @brief Lit tous les ToF et renvoie la plus petite distance détectée
         * 
         * @param sampleCount Le nombre d'échantillons à réaliser par TOF. La valeur renvoyée sera la plus petite de toutes les ditances moyennées par ToF
         * @return int 
         */
        int getClosestObjectDistance(unsigned int sampleCount);

        /**
         * @brief Lit le ToF qui avait détecté l'objet le plus proche par getClosestObjectDistance() et renvoie la distance
         * Provoque un appel à getClosestObjectDistance() s'il n'a pas été appelé au moins une fois
         * 
         * @param sampleCount Nombre d'échantillons à réaliser par ToF.
         * @return int 
         */
        int getLastClosestObjectDistance(unsigned int sampleCount);

    private:
        Stream* serial;
        Cart* cart;
        short tofArrayCount;
        TimeOfFlightArray** tofArrays;
        int* distanceByToF;

        /**
         * @brief Index du ToF qui a détecté l'objet le plus proche lors du dernier getClosestObjectDistance()
         * 
         */
        int lastClosestObjectTofIndexInArray;
};

#endif