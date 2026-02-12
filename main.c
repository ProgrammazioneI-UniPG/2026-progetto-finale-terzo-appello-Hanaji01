#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "gamelib.h"

//funzione principale del gioco, mostra il menu e gestisce le scelte dell'utente
int main(void) {
    int scelta = 0;

    /* Inizializza il generatore di numeri casuali una sola volta */
    srand((unsigned int)time(NULL));

    /* Stampa il banner di benvenuto */
    printf("========================================\n");
    printf("       BENVENUTO IN COSESTRANE!\n");
    printf("========================================\n");
    printf("\nNella tranquilla cittadina di Occhinz,\n");
    printf("famosa per i Waffle Undici e per il numero\n");
    printf("inspiegabilmente alto di biciclette scomparse,\n");
    printf("cominciano ad aprirsi strani portali dimensionali...\n");
    printf("\nSei pronto a esplorare il Mondo Reale\n");
    printf("e il misterioso Soprasotto?\n");

    /* Ciclo principale del menu */
    do {
        /* Stampa il menu principale */
        printf("\n");
        printf("================================================================================\n");
        printf("                              COSESTRANE                                        \n");
        printf("================================================================================\n");
        printf("\n");
        printf("1) Imposta gioco\n");
        printf("2) Gioca\n");
        printf("3) Termina gioco\n");
        printf("4) Visualizza crediti\n");
        printf("\n");
        printf("Scegli un'opzione (1-4): ");

        /* Legge e valida l'input dell'utente */
        if (scanf("%d", &scelta) != 1) {
            /* Input non numerico: pulisce il buffer e richiede un nuovo input */
            while (getchar() != '\n');
            printf("\nErrore: devi inserire un numero intero tra 1 e 4!\n");
            scelta = 0;
            continue;
        }

        /* Pulisce il buffer dopo la lettura corretta */
        while (getchar() != '\n');

        /* Esegue l'azione corrispondente alla scelta */
        switch (scelta) {
            case 1:
                imposta_gioco();
                break;
            case 2:
                gioca();
                break;
            case 3:
                termina_gioco();
                break;
            case 4:
                crediti();
                break;
            default:
                printf("\nErrore: scelta non valida! Inserisci un numero da 1 a 4.\n");
                scelta = 0;
                break;
        }

    } while (scelta != 3);// Continua a mostrare il menu finché l'utente non sceglie di terminare il gioco

    return 0;
}
