#ifndef TIMEOFFLIGHTARRAY_HPP
#define TIMEOFFLIGHTARRAY_HPP

#include <Wire.h>
#include <VL53L1X.h>

#define TOF_DEFAULT_ADDRESS 0x29
#define TOF_MAX_COUNT 8

/**
 * @brief Représente un tableau de TimeOfFlight reliés à un pcf8574 pour gérer leur paramétrage initial
 * 
 */
class TimeOfFlightArray {

    public:
        /**
         * @brief Construct a new Time Of Flight Array object
         * 
         * @param pcf8574Address Adresse I2C du pcf8574
         * @param firstTimeOfFlightAddress Adresse du prmeier ToF Détecté, le suivant aura l'adresse timeOfFlightAddressOffset + 1, puis timeOfFlightAddressOffset + 2, etc.
         */
        TimeOfFlightArray(unsigned char pcf8574Address, unsigned char firstTimeOfFlightAddress = TOF_DEFAULT_ADDRESS);

        /**
         * @brief Effectue la séquence de détection et de paramétrage des TimeOFFlught
         * 
         * @return true initilisation ok
         * @return false initialisation échouée
         */
        bool init();

        /**
         * @brief Renvoie le nombre de Time Of Flight détectés après l'appel au init()
         * 
         * @return unsigned short 
         */
        unsigned short getConnectedTofCount();

        /**
         * @brief Vérifie si un ToF est présent à l'id indiqué
         * 
         * @param id 
         * @return true 
         * @return false 
         */
        bool tofExists(unsigned short id);

        /**
         * @brief Vérifie la présence du pcf8574 sur la ligne I2C
         * 
         * @return true 
         * @return false 
         */
        bool checkIfPcf8574IsPresent();

        /**
         * @brief récupère la distance détectée par le Time Of Flight #id
         * Une valeur de 0xFFFF indique un problème de mesure : le ToF n'est pas joignable ou une erreur de mesure est arrivée 
         * 
         * @param id 
         * @return uint16_t 
         */
        uint16_t getDistance(unsigned short id);

        /**
         * @brief Déclenche une mesure bloquante de tous les ToF présents
         * Bloque jusqu'à ce que tous les ToF aient répondu
         */
        void triggerMeasuresBlocking();

        /**
         * @brief Récolte une des mesures continues délcenchées par startContinuous()
         * 
         */
        void triggerMeasuresNonBlocking();

        /**
         * @brief Démarre l'acquisiiton en continu des ToF
         * 
         * @param period_ms 
         */
        void startContinuous(uint32_t period_ms);

    private:
        /**
         * @brief états des 8 sorties du pcf8574
         * 
         */
        char pcf8574_state;

        /**
         * @brief Références vers les Time Of Flight 
         * 
         */
        VL53L1X* tofs[TOF_MAX_COUNT];

        /**
         * @brief Trace des dernières mesures effectuées
         * 
         */
        uint16_t distances[TOF_MAX_COUNT];

        /**
         * @brief Retiens si le pcf8574 a été détecté
         * 
         */
        bool pcf8574IsPresent;

        /**
         * @brief Adresse du pcf8574
         * 
         */
        unsigned char pcf8574Address;

        /**
         * @brief Adresse du 1er Time Of Flight trouvé
         * 
         */
        unsigned char firstTimeOfFlightAddress;

        /**
         * @brief Allume ou éteint tous les slots timeofflight
         * 
         * @param enable 
         */
        void enableAllPcfSlots(bool enable);

        /**
         * @brief Allume ou éteint un slot précis timeofflight
         * 
         * @param slot slot concerné de 0 à 7 
         * @param enable 
         */
        void enableSinglePcfSlot(char slot, bool enable);

        /**
         * @brief Teste la présence du TimeOfLight sur le Bus I2C en activant le slot concerné avant le test
         * 
         * @param slot 
         * @return VL53L1X* 
         */
        VL53L1X* instanciateTofIfPresentInSlot(int slot);
};

#endif