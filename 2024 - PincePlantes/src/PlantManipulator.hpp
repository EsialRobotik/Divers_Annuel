#ifndef PLANTMANIPULATOR_HPP
#define PLANTMANIPULATOR_HPP

#include <Stream.h>
#include <TimeOfFlightArray.hpp>
#include <Cart.hpp>

// Distances min et max entre lesquelles on considère qu'une plante est présente devant un ToF et est atteignable
#define PLANTPANIULATOR_MIN_DISTANCE_MM 70
#define PLANTPANIULATOR_MAX_DISTANCE_MM 230

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
         * Tente ensuite de positionner le chariot en face de la première plante toruvée de manière non bloquantes.
         * Si une plante a été détectée, le chariot est envoyé à la position et "ok" est renvoyé sur la liaison série lorsqu'il s'immobilise
         * 
         * @return true Si une plante est trouvé
         * @return false Si aucune plante n'est trouvée
         */
        bool faceNextPlantAsync();

        /**
         * @brief Effetcue une acquisition sur tous les ToF disponibles et écrit le résultat sur la liaison série
         * 
         */
        void acquireAndPrintLine();

    private:
        Stream* serial;
        Cart* cart;
        short tofArrayCount;
        TimeOfFlightArray** tofArrays;
        int* distanceByToF;
        bool movingToPlant;
};

#endif