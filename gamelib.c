#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "gamelib.h"

/* ============================================================================
 * VARIABILI GLOBALI STATICHE
 * ============================================================================ */

/* Puntatori alle mappe dei due mondi */
static Zona_mondoreale* prima_zona_mondoreale = NULL;
static Zona_soprasotto* prima_zona_soprasotto = NULL;

/* Array dei giocatori attivi */
static Giocatore* giocatori[4] = {NULL, NULL, NULL, NULL};

/* Contatori e flag di stato */
static int num_giocatori = 0;
static int mappa_chiusa  = 0;
static int gioco_impostato = 0;

/* Statistiche di gioco */
static char ultimo_vincitore[3][NOME_MAX] = {"Nessuno", "Nessuno", "Nessuno"};
static int partite_giocate = 0;

/* ============================================================================
 * FUNZIONI DI UTILITA' GENERALI
 * ============================================================================ */

/**
 * Lancia un dado da 20 facce
 * @return Numero casuale tra 1 e 20 (inclusi)
 */
static int lancia_dado(void) {
    return (rand() % 20) + 1;
}

/**
 * Converte un tipo di zona in stringa leggibile
 * @param tipo Il tipo di zona da convertire
 * @return Stringa descrittiva del tipo di zona
 */
static const char* tipo_zona_to_string(Tipo_zona tipo) {
    switch (tipo) {
        case BOSCO:                return "Bosco";
        case SCUOLA:               return "Scuola";
        case LABORATORIO:          return "Laboratorio";
        case CAVERNA:              return "Caverna";
        case STRADA:               return "Strada";
        case GIARDINO:             return "Giardino";
        case SUPERMERCATO:         return "Supermercato";
        case CENTRALE_ELETTRICA:   return "Centrale Elettrica";
        case DEPOSITO_ABBANDONATO: return "Deposito Abbandonato";
        case STAZIONE_POLIZIA:     return "Stazione di Polizia";
        default:                   return "Sconosciuta";
    }
}

/**
 * Converte un tipo di nemico in stringa leggibile
 * @param tipo Il tipo di nemico da convertire
 * @return Stringa descrittiva del tipo di nemico
 */
static const char* tipo_nemico_to_string(Tipo_nemico tipo) {
    switch (tipo) {
        case NESSUN_NEMICO: return "Nessun nemico";
        case BILLI:         return "Billi";
        case DEMOCANE:      return "Democane";
        case DEMOTORZONE:   return "Demotorzone";
        default:            return "Sconosciuto";
    }
}

/**
 * Converte un tipo di oggetto in stringa leggibile
 * @param tipo Il tipo di oggetto da convertire
 * @return Stringa descrittiva del tipo di oggetto
 */
static const char* tipo_oggetto_to_string(Tipo_oggetto tipo) {
    switch (tipo) {
        case NESSUN_OGGETTO:         return "Nessun oggetto";
        case BICICLETTA:             return "Bicicletta";
        case MAGLIETTA_FUOCOINFERNO: return "Maglietta Fuocoinferno";
        case BUSSOLA:                return "Bussola";
        case SCHITARRATA_METALLICA:  return "Schitarrata Metallica";
        default:                     return "Sconosciuto";
    }
}

/* ============================================================================
 * FUNZIONI DI CONTEGGIO
 * ============================================================================ */

/**
 * Conta il numero totale di zone nella mappa del Mondo Reale
 * @return Numero di zone presenti
 */
static int conta_zone_mondoreale(void) {
    int count = 0;
    Zona_mondoreale* current = prima_zona_mondoreale;

    while (current != NULL) {
        count++;
        current = current->avanti;
    }

    return count;
}

/**
 * Conta il numero di Demotorzone presenti nella mappa del Soprasotto
 * @return Numero di Demotorzone trovati (deve essere esattamente 1)
 */
static int conta_demotorzone(void) {
    int count = 0;
    Zona_soprasotto* current = prima_zona_soprasotto;

    while (current != NULL) {
        if (current->nemico == DEMOTORZONE) {
            count++;
        }
        current = current->avanti;
    }

    return count;
}

/* ============================================================================
 * FUNZIONI DI GENERAZIONE CASUALE
 * ============================================================================ */

/**
 * Genera un tipo di nemico casuale per il Mondo Reale
 * Probabilita': 40% nessuno, 30% Democane, 30% Billi
 * @return Tipo di nemico generato
 */
static Tipo_nemico genera_nemico_mondoreale(void) {
    int prob = rand() % 100;

    if (prob < PROB_NESSUN_NEMICO_MR) {
        return NESSUN_NEMICO;
    } else if (prob < PROB_NESSUN_NEMICO_MR + PROB_DEMOCANE_MR) {
        return DEMOCANE;
    } else {
        return BILLI;
    }
}

/**
 * Genera un tipo di nemico casuale per il Soprasotto
 * Se deve_avere_demotorzone e' true, genera sempre un Demotorzone
 * Altrimenti: 50% nessuno, 50% Democane
 * @param deve_avere_demotorzone Flag per forzare la generazione del Demotorzone
 * @return Tipo di nemico generato
 */
static Tipo_nemico genera_nemico_soprasotto(int deve_avere_demotorzone) {
    int prob;

    if (deve_avere_demotorzone) {
        return DEMOTORZONE;
    }

    prob = rand() % 100;

    if (prob < PROB_NESSUN_NEMICO_SS) {
        return NESSUN_NEMICO;
    } else {
        return DEMOCANE;
    }
}

/**
 * Genera un tipo di oggetto casuale
 * Probabilita':
 * - 50% nessuno
 * - 15% Bicicletta
 * - 15% Maglietta Fuocoinferno
 * - 10% Bussola
 * - 10% Schitarrata Metallica
 * @return Tipo di oggetto generato
 */
static Tipo_oggetto genera_oggetto(void) {
    int prob = rand() % 100;

    if (prob < PROB_NESSUN_OGGETTO) {
        return NESSUN_OGGETTO;
    } else if (prob < PROB_NESSUN_OGGETTO + PROB_BICICLETTA) {
        return BICICLETTA;
    } else if (prob < PROB_NESSUN_OGGETTO + PROB_BICICLETTA + PROB_MAGLIETTA) {
        return MAGLIETTA_FUOCOINFERNO;
    } else if (prob < PROB_NESSUN_OGGETTO + PROB_BICICLETTA + PROB_MAGLIETTA + PROB_BUSSOLA) {
        return BUSSOLA;
    } else {
        return SCHITARRATA_METALLICA;
    }
}

/* ============================================================================
 * FUNZIONI DI GESTIONE MEMORIA
 * ============================================================================ */

/**
 * Libera tutta la memoria allocata per le mappe dei due mondi
 * Attraversa entrambe le liste e dealloca tutte le zone
 */
static void libera_mappe(void) {
    Zona_mondoreale* current_mr = prima_zona_mondoreale;
    while (current_mr != NULL) {
        Zona_mondoreale* temp = current_mr;
        current_mr = current_mr->avanti;
        free(temp);
    }
    prima_zona_mondoreale = NULL;

    Zona_soprasotto* current_ss = prima_zona_soprasotto;
    while (current_ss != NULL) {
        Zona_soprasotto* temp = current_ss;
        current_ss = current_ss->avanti;
        free(temp);
    }
    prima_zona_soprasotto = NULL;
}

/**
 * Libera tutta la memoria allocata per i giocatori
 * Resetta anche il contatore dei giocatori
 */
static void libera_giocatori(void) {
    int i;
    for (i = 0; i < 4; i++) {
        if (giocatori[i] != NULL) {
            free(giocatori[i]);
            giocatori[i] = NULL;
        }
    }
    num_giocatori = 0;
}

/* ============================================================================
 * FUNZIONI DI CREAZIONE E MODIFICA MAPPA
 * ============================================================================ */

// Genera una mappa casuale per entrambi i mondi
static void genera_mappa(void) {
    int i;
    int posizione_demotorzone;
    Zona_mondoreale* ultima_mr = NULL;
    Zona_soprasotto* ultima_ss = NULL;

    libera_mappe();

    posizione_demotorzone = rand() % ZONE_MINIME;

    for (i = 0; i < ZONE_MINIME; i++) {
        Zona_mondoreale* nuova_mr = (Zona_mondoreale*)malloc(sizeof(Zona_mondoreale));
        if (nuova_mr == NULL) {
            printf("Errore: memoria insufficiente durante la creazione della mappa!\n");
            libera_mappe();
            return;
        }

        Zona_soprasotto* nuova_ss = (Zona_soprasotto*)malloc(sizeof(Zona_soprasotto));
        if (nuova_ss == NULL) {
            printf("Errore: memoria insufficiente durante la creazione della mappa!\n");
            free(nuova_mr);
            libera_mappe();
            return;
        }

        nuova_mr->tipo     = (Tipo_zona)(rand() % 10);
        nuova_mr->nemico   = genera_nemico_mondoreale();
        nuova_mr->oggetto  = genera_oggetto();
        nuova_mr->avanti   = NULL;
        nuova_mr->indietro = NULL;
        nuova_mr->link_soprasotto = NULL;

        nuova_ss->tipo     = nuova_mr->tipo;
        nuova_ss->nemico   = genera_nemico_soprasotto(i == posizione_demotorzone);
        nuova_ss->avanti   = NULL;
        nuova_ss->indietro = NULL;
        nuova_ss->link_mondoreale = NULL;

        nuova_mr->link_soprasotto = nuova_ss;
        nuova_ss->link_mondoreale = nuova_mr;

        if (prima_zona_mondoreale == NULL) {
            prima_zona_mondoreale = nuova_mr;
            prima_zona_soprasotto = nuova_ss;
        } else {
            ultima_mr->avanti  = nuova_mr;
            nuova_mr->indietro = ultima_mr;

            ultima_ss->avanti  = nuova_ss;
            nuova_ss->indietro = ultima_ss;
        }

        ultima_mr = nuova_mr;
        ultima_ss = nuova_ss;
    }

    printf("\nMappa generata con successo! %d zone create per ciascun mondo.\n", ZONE_MINIME);
}

// Inserisce una nuova zona in una posizione specifica della mappa
static void inserisci_zona(void) {
    int posizione;
    int tipo_input, nemico_input, oggetto_input;
    int i;
    int num_zone;
    Zona_mondoreale* nuova_mr;
    Zona_soprasotto* nuova_ss;

    if (mappa_chiusa) {
        printf("\nErrore: la mappa e' gia' stata chiusa!\n");
        printf("Non puoi piu' modificarla.\n");
        return;
    }

    num_zone = conta_zone_mondoreale();

    printf("\nInserisci la posizione (1-%d): ", num_zone + 1);
    if (scanf("%d", &posizione) != 1) {
        while (getchar() != '\n');
        printf("Errore: devi inserire un numero intero!\n");
        return;
    }
    while (getchar() != '\n');

    if (posizione < 1 || posizione > num_zone + 1) {
        printf("Errore: posizione non valida! Deve essere tra 1 e %d.\n", num_zone + 1);
        return;
    }

    printf("\nTipo di zona (0-9):\n");
    printf("0=Bosco, 1=Scuola, 2=Laboratorio, 3=Caverna, 4=Strada\n");
    printf("5=Giardino, 6=Supermercato, 7=Centrale Elettrica\n");
    printf("8=Deposito Abbandonato, 9=Stazione Polizia\n");
    printf("Scegli: ");
    if (scanf("%d", &tipo_input) != 1 || tipo_input < 0 || tipo_input > 9) {
        while (getchar() != '\n');
        printf("Errore: tipo non valido! Deve essere tra 0 e 9.\n");
        return;
    }
    while (getchar() != '\n');

    printf("\nNemico Mondo Reale (0=Nessuno, 1=Billi, 2=Democane): ");
    if (scanf("%d", &nemico_input) != 1 || nemico_input < 0 || nemico_input > 2) {
        while (getchar() != '\n');
        printf("Errore: nemico non valido! Deve essere 0, 1 o 2.\n");
        return;
    }
    while (getchar() != '\n');

    printf("\nOggetto (0=Nessuno, 1=Bicicletta, 2=Maglietta, 3=Bussola, 4=Schitarrata): ");
    if (scanf("%d", &oggetto_input) != 1 || oggetto_input < 0 || oggetto_input > 4) {
        while (getchar() != '\n');
        printf("Errore: oggetto non valido! Deve essere tra 0 e 4.\n");
        return;
    }
    while (getchar() != '\n');

    nuova_mr = (Zona_mondoreale*)malloc(sizeof(Zona_mondoreale));
    if (nuova_mr == NULL) {
        printf("Errore: memoria insufficiente!\n");
        return;
    }

    nuova_ss = (Zona_soprasotto*)malloc(sizeof(Zona_soprasotto));
    if (nuova_ss == NULL) {
        printf("Errore: memoria insufficiente!\n");
        free(nuova_mr);
        return;
    }

    nuova_mr->tipo     = (Tipo_zona)tipo_input;
    nuova_mr->nemico   = (Tipo_nemico)nemico_input;
    nuova_mr->oggetto  = (Tipo_oggetto)oggetto_input;
    nuova_mr->avanti   = NULL;
    nuova_mr->indietro = NULL;

    nuova_ss->tipo     = (Tipo_zona)tipo_input;
    nuova_ss->nemico   = genera_nemico_soprasotto(0); /* Il Demotorzone si inserisce solo via genera_mappa */
    nuova_ss->avanti   = NULL;
    nuova_ss->indietro = NULL;

    nuova_mr->link_soprasotto = nuova_ss;
    nuova_ss->link_mondoreale = nuova_mr;

    if (posizione == 1) { // Inserimento in testa
        nuova_mr->avanti = prima_zona_mondoreale;
        nuova_ss->avanti = prima_zona_soprasotto;

        if (prima_zona_mondoreale != NULL) { // Aggiorna il link indietro della vecchia prima zona del Mondo Reale
            prima_zona_mondoreale->indietro = nuova_mr;
        }
        if (prima_zona_soprasotto != NULL) {// Aggiorna il link indietro della vecchia prima zona del Soprasotto
            prima_zona_soprasotto->indietro = nuova_ss;
        }

        prima_zona_mondoreale = nuova_mr;
        prima_zona_soprasotto = nuova_ss;
    } else { // Inserimento in posizione intermedia o in coda
        Zona_mondoreale* current_mr = prima_zona_mondoreale;
        Zona_soprasotto* current_ss = prima_zona_soprasotto;

        for (i = 1; i < posizione - 1 && current_mr != NULL; i++) { // Posizione - 1 perché vogliamo fermarci alla zona precedente a quella di inserimento
            current_mr = current_mr->avanti;
            current_ss = current_ss->avanti;
        }

        if (current_mr != NULL) {// Inserisce la nuova zona dopo current_mr e current_ss
            nuova_mr->avanti   = current_mr->avanti;
            nuova_mr->indietro = current_mr;

            if (current_mr->avanti != NULL) { // Aggiorna il link indietro della zona successiva del Mondo Reale
                current_mr->avanti->indietro = nuova_mr;
            }
            current_mr->avanti = nuova_mr;

            nuova_ss->avanti   = current_ss->avanti;
            nuova_ss->indietro = current_ss;

            if (current_ss->avanti != NULL) { // Aggiorna il link indietro della zona successiva del Soprasotto
                current_ss->avanti->indietro = nuova_ss;
            }
            current_ss->avanti = nuova_ss;
        }
    }

    printf("\nZona inserita con successo in posizione %d!\n", posizione);
}

// Cancella una zona in una posizione specifica della mappa
static void cancella_zona(void) {
    int posizione;
    int i;
    int num_zone;
    Zona_mondoreale* current_mr;
    Zona_soprasotto* current_ss;

    if (mappa_chiusa) {
        printf("\nErrore: la mappa e' gia' stata chiusa!\n");
        printf("Non puoi piu' modificarla.\n");
        return;
    }

    if (prima_zona_mondoreale == NULL) {
        printf("\nErrore: non ci sono zone da cancellare!\n");
        return;
    }

    num_zone = conta_zone_mondoreale();

    printf("\nInserisci la posizione da cancellare (1-%d): ", num_zone);
    if (scanf("%d", &posizione) != 1) {
        while (getchar() != '\n');
        printf("Errore: devi inserire un numero intero!\n");
        return;
    }
    while (getchar() != '\n');

    if (posizione < 1 || posizione > num_zone) {
        printf("Errore: posizione non valida! Deve essere tra 1 e %d.\n", num_zone);
        return;
    }

    current_mr = prima_zona_mondoreale;
    current_ss = prima_zona_soprasotto;

    for (i = 1; i < posizione && current_mr != NULL; i++) {
        current_mr = current_mr->avanti;
        current_ss = current_ss->avanti;
    }

    if (current_mr == NULL) {
        printf("Errore: posizione non trovata!\n");
        return;
    }

    if (current_mr->indietro != NULL) {
        current_mr->indietro->avanti = current_mr->avanti;
    } else {
        prima_zona_mondoreale = current_mr->avanti;
    }

    if (current_mr->avanti != NULL) {
        current_mr->avanti->indietro = current_mr->indietro;
    }

    if (current_ss->indietro != NULL) {
        current_ss->indietro->avanti = current_ss->avanti;
    } else {
        prima_zona_soprasotto = current_ss->avanti;
    }

    if (current_ss->avanti != NULL) {
        current_ss->avanti->indietro = current_ss->indietro;
    }

    free(current_mr);
    free(current_ss);

    printf("\nZona cancellata con successo!\n");
}

/* ============================================================================
 * FUNZIONI DI VISUALIZZAZIONE MAPPA
 * ============================================================================ */

//
static void stampa_mappa(void) {
    int scelta;
    int count = 1;

    printf("\nQuale mappa vuoi visualizzare?\n");
    printf("1) Mondo Reale\n");
    printf("2) Soprasotto\n");
    printf("Scegli: ");

    if (scanf("%d", &scelta) != 1) {
        while (getchar() != '\n');
        printf("Errore: devi inserire 1 o 2!\n");
        return;
    }
    while (getchar() != '\n');

    if (scelta == 1) { // Visualizza la mappa del Mondo Reale
        Zona_mondoreale* current = prima_zona_mondoreale;

        printf("\n=== MAPPA MONDO REALE ===\n\n");

        if (current == NULL) {
            printf("La mappa e' vuota.\n");
            return;
        }

        while (current != NULL) {
            printf("Zona %d:\n", count);
            printf("  Tipo: %s\n",    tipo_zona_to_string(current->tipo));
            printf("  Nemico: %s\n",  tipo_nemico_to_string(current->nemico));
            printf("  Oggetto: %s\n", tipo_oggetto_to_string(current->oggetto));
            printf("\n");
            current = current->avanti;
            count++;
        }
    } else if (scelta == 2) { // Visualizza la mappa del Soprasotto
        Zona_soprasotto* current = prima_zona_soprasotto;

        printf("\n=== MAPPA SOPRASOTTO ===\n\n");

        if (current == NULL) {
            printf("La mappa e' vuota.\n");
            return;
        }

        while (current != NULL) {
            printf("Zona %d:\n", count);
            printf("  Tipo: %s\n",   tipo_zona_to_string(current->tipo));
            printf("  Nemico: %s\n", tipo_nemico_to_string(current->nemico));
            printf("\n");
            current = current->avanti;
            count++;
        }
    } else {
        printf("Errore: scelta non valida! Inserisci 1 o 2.\n");
    }
}

// Visualizza i dettagli di una zona specifica in entrambe le mappe
static void stampa_zona(void) {
    int posizione;
    int i;
    int num_zone;
    Zona_mondoreale* current_mr;

    num_zone = conta_zone_mondoreale();

    if (num_zone == 0) {
        printf("\nLa mappa e' vuota! Non ci sono zone da visualizzare.\n");
        return;
    }

    printf("\nInserisci la posizione della zona (1-%d): ", num_zone);
    if (scanf("%d", &posizione) != 1) {
        while (getchar() != '\n');
        printf("Errore: devi inserire un numero intero!\n");
        return;
    }
    while (getchar() != '\n');

    if (posizione < 1 || posizione > num_zone) {
        printf("Errore: posizione non valida! Deve essere tra 1 e %d.\n", num_zone);
        return;
    }

    current_mr = prima_zona_mondoreale;

    for (i = 1; i < posizione && current_mr != NULL; i++) {
        current_mr = current_mr->avanti;
    }

    if (current_mr == NULL) {
        printf("Errore: zona non trovata!\n");
        return;
    }

    printf("\n=== ZONA %d - MONDO REALE ===\n", posizione);
    printf("Tipo: %s\n",    tipo_zona_to_string(current_mr->tipo));
    printf("Nemico: %s\n",  tipo_nemico_to_string(current_mr->nemico));
    printf("Oggetto: %s\n", tipo_oggetto_to_string(current_mr->oggetto));

    printf("\n=== ZONA %d - SOPRASOTTO ===\n", posizione);
    printf("Tipo: %s\n",   tipo_zona_to_string(current_mr->link_soprasotto->tipo));
    printf("Nemico: %s\n", tipo_nemico_to_string(current_mr->link_soprasotto->nemico));
    printf("\n");
}

// Valida la mappa e la chiude per le modifiche, rendendola pronta per il gioco
static void chiudi_mappa(void) {
    int num_zone        = conta_zone_mondoreale();
    int num_demotorzone = conta_demotorzone();

    if (num_zone < ZONE_MINIME) {
        printf("\nErrore: la mappa deve avere almeno %d zone!\n", ZONE_MINIME);
        printf("Attualmente ne hai %d. Aggiungine altre %d.\n",
               num_zone, ZONE_MINIME - num_zone);
        return;
    }

    if (num_demotorzone != 1) {
        printf("\nErrore: la mappa deve avere esattamente 1 Demotorzone nel Soprasotto!\n");
        if (num_demotorzone == 0) {
            printf("Attualmente non ce ne sono. Devi generare nuovamente la mappa\n");
            printf("o inserire manualmente una zona con Demotorzone nel Soprasotto.\n");
        } else {
            printf("Attualmente ne hai %d. Devi generare nuovamente la mappa.\n", num_demotorzone);
        }
        return;
    }

    mappa_chiusa = 1;
    printf("\n");
    printf("================================================================================\n");
    printf("                     MAPPA VALIDATA E CHIUSA                                    \n");
    printf("================================================================================\n");
    printf("\nLa mappa e' pronta! Hai creato %d zone.\n", num_zone);
    printf("Il Demotorzone ti aspetta nel Soprasotto...\n");
    printf("Il gioco e' pronto per iniziare!\n");
    printf("================================================================================\n");
}

/* ============================================================================
 * FUNZIONE PUBBLICA: IMPOSTA_GIOCO
 * ============================================================================ */

// Permette di configurare il gioco, impostare i giocatori e preparare la mappa
void imposta_gioco(void) {
    int i;
    int scelta_menu;
    int num_input;
    int scelta_abilita;
    int undici_disponibile = 1;

    printf("\n");
    printf("================================================================================\n");
    printf("                         IMPOSTAZIONE GIOCO                                     \n");
    printf("================================================================================\n");

    libera_giocatori();
    libera_mappe();
    mappa_chiusa    = 0;
    gioco_impostato = 0;
    num_giocatori   = 0;

    /* ========================================================================
     * FASE 1: CONFIGURAZIONE GIOCATORI
     * ======================================================================== */

    do {
        printf("\nInserisci il numero di giocatori (1-4): ");
        if (scanf("%d", &num_input) != 1) {
            while (getchar() != '\n');
            printf("Errore: devi inserire un numero intero!\n");
            continue;
        }
        while (getchar() != '\n');

        if (num_input < 1 || num_input > 4) {
            printf("Errore: il numero di giocatori deve essere tra 1 e 4!\n");
        }
    } while (num_input < 1 || num_input > 4);

    num_giocatori = num_input;

    for (i = 0; i < num_giocatori; i++) {
        giocatori[i] = (Giocatore*)malloc(sizeof(Giocatore));
        if (giocatori[i] == NULL) {
            printf("Errore: memoria insufficiente per creare i giocatori!\n");
            libera_giocatori();
            return;
        }

        printf("\n--- Giocatore %d ---\n", i + 1);
        printf("Inserisci il nome (max %d caratteri): ", NOME_MAX - 1);
        if (fgets(giocatori[i]->nome, NOME_MAX, stdin) != NULL) { 
            size_t len = strlen(giocatori[i]->nome);
            if (len > 0 && giocatori[i]->nome[len - 1] == '\n') {// Rimuove il newline se presente
                giocatori[i]->nome[len - 1] = '\0';
            }
        }

        giocatori[i]->attacco_psichico = lancia_dado();
        giocatori[i]->difesa_psichica  = lancia_dado();
        giocatori[i]->fortuna          = lancia_dado();
        giocatori[i]->punti_vita       = PV_INIZIALI;

        printf("\nAbilita' iniziali (lancio dado da 20):\n");
        printf("  Attacco Psichico: %d\n", giocatori[i]->attacco_psichico);
        printf("  Difesa Psichica:  %d\n", giocatori[i]->difesa_psichica);
        printf("  Fortuna:          %d\n", giocatori[i]->fortuna);
        printf("  Punti Vita:       %d\n", giocatori[i]->punti_vita);

        printf("\nVuoi modificare le tue abilita'?\n");
        printf("1) +%d Attacco, -%d Difesa\n", MODIFICA_ATTACCO_DIFESA, MODIFICA_ATTACCO_DIFESA);
        printf("2) +%d Difesa, -%d Attacco\n", MODIFICA_ATTACCO_DIFESA, MODIFICA_ATTACCO_DIFESA);
        if (undici_disponibile) {
            printf("3) Diventa UndiciVirgolaCinque (+%d Attacco, +%d Difesa, -%d Fortuna)\n",
                   BONUS_UNDICI_ATTACCO, BONUS_UNDICI_DIFESA, MALUS_UNDICI_FORTUNA);
        }
        printf("4) Nessuna modifica\n");
        printf("Scegli: ");

        if (scanf("%d", &scelta_abilita) != 1) {
            while (getchar() != '\n');
            scelta_abilita = 4;
        }
        while (getchar() != '\n');

        switch (scelta_abilita) {
            case 1:
                giocatori[i]->attacco_psichico += MODIFICA_ATTACCO_DIFESA;
                giocatori[i]->difesa_psichica  -= MODIFICA_ATTACCO_DIFESA;
                if (giocatori[i]->difesa_psichica < 1) {
                    giocatori[i]->difesa_psichica = 1;
                }
                printf("Abilita' modificate! Sei ora piu' offensivo.\n");
                break;
            case 2:
                giocatori[i]->difesa_psichica  += MODIFICA_ATTACCO_DIFESA;
                giocatori[i]->attacco_psichico -= MODIFICA_ATTACCO_DIFESA;
                if (giocatori[i]->attacco_psichico < 1) {
                    giocatori[i]->attacco_psichico = 1;
                }
                printf("Abilita' modificate! Sei ora piu' difensivo.\n");
                break;
            case 3:
                if (undici_disponibile) {
                    giocatori[i]->attacco_psichico += BONUS_UNDICI_ATTACCO;
                    giocatori[i]->difesa_psichica  += BONUS_UNDICI_DIFESA;
                    giocatori[i]->fortuna          -= MALUS_UNDICI_FORTUNA;
                    if (giocatori[i]->fortuna < 1) {
                        giocatori[i]->fortuna = 1;
                    }
                    strncpy(giocatori[i]->nome, "UndiciVirgolaCinque", NOME_MAX - 1);
                    giocatori[i]->nome[NOME_MAX - 1] = '\0';
                    undici_disponibile = 0;
                    printf("\n*** SEI DIVENTATO UNDICIVIRGOLACINQUE! ***\n");
                    printf("Poteri aumentati, ma la fortuna ti ha abbandonato!\n");
                } else {
                    printf("UndiciVirgolaCinque non e' piu' disponibile!\n");
                    printf("Un altro giocatore ha gia' preso questo ruolo.\n");
                }
                break;
            default:
                printf("Nessuna modifica applicata.\n");
                break;
        }
        // Imposta la posizione iniziale del giocatore nel Mondo Reale (prima zona)
        giocatori[i]->mondo          = MONDO_REALE;
        giocatori[i]->pos_mondoreale = NULL;
        giocatori[i]->pos_soprasotto = NULL;

        {
            int j;
            for (j = 0; j < ZAINO_MAX; j++) {
                giocatori[i]->zaino[j] = NESSUN_OGGETTO;
            }
        }

        printf("\nGiocatore %d configurato con successo!\n", i + 1);
        printf("Abilita' finali:\n");
        printf("  Attacco: %d | Difesa: %d | Fortuna: %d\n",
               giocatori[i]->attacco_psichico,
               giocatori[i]->difesa_psichica,
               giocatori[i]->fortuna);
    }

    /* ========================================================================
     * FASE 2: CREAZIONE MAPPA
     * ======================================================================== */

    printf("\n");
    printf("================================================================================\n");
    printf("                         CREAZIONE MAPPA                                        \n");
    printf("================================================================================\n");
    printf("\nOra devi creare la mappa del gioco.\n");
    printf("Puoi generare una mappa casuale o costruirla manualmente.\n");
    printf("Ricorda: servono almeno %d zone e 1 Demotorzone!\n", ZONE_MINIME);

    do {
        printf("\n--- Menu Creazione Mappa ---\n");
        printf("1) Genera mappa casuale (%d zone)\n", ZONE_MINIME);
        printf("2) Inserisci zona manualmente\n");
        printf("3) Cancella zona\n");
        printf("4) Visualizza mappa completa\n");
        printf("5) Visualizza singola zona\n");
        printf("6) Chiudi mappa e termina impostazione\n");
        printf("Scegli: ");

        if (scanf("%d", &scelta_menu) != 1) {
            while (getchar() != '\n');
            printf("Errore: devi inserire un numero intero!\n");
            continue;
        }
        while (getchar() != '\n');

        switch (scelta_menu) {
            case 1: genera_mappa();    break;
            case 2: inserisci_zona();  break;
            case 3: cancella_zona();   break;
            case 4: stampa_mappa();    break;
            case 5: stampa_zona();     break;
            case 6:
                chiudi_mappa();
                if (mappa_chiusa) {
                    gioco_impostato = 1;
                }
                break;
            default:
                printf("Scelta non valida! Inserisci un numero da 1 a 6.\n");
                break;
        }
    } while (!mappa_chiusa);

    printf("\n");
    printf("================================================================================\n");
    printf("                   IMPOSTAZIONE COMPLETATA!                                     \n");
    printf("================================================================================\n");
    printf("\nIl gioco e' pronto. Torna al menu principale e scegli \"Gioca\"!\n");
}

/* ============================================================================
 * FUNZIONI DI GIOCO - VISUALIZZAZIONE
 * ============================================================================ */

// Stampa le informazioni dettagliate di un giocatore, inclusi nome, mondo, statistiche e inventario
static void stampa_giocatore_info(Giocatore* g) {
    int i;

    if (g == NULL) {
        printf("Errore: giocatore non valido!\n");
        return;
    }

    printf("\n");
    printf("================================================================================\n");
    printf("                     SCHEDA GIOCATORE                                           \n");
    printf("================================================================================\n");
    printf("\n");
    printf("Nome: %s\n", g->nome);
    printf("Mondo attuale: %s\n", g->mondo == MONDO_REALE ? "Mondo Reale" : "Soprasotto");
    printf("\n--- Statistiche ---\n");
    printf("Punti Vita:       %d/%d\n", g->punti_vita, PV_INIZIALI);
    printf("Attacco Psichico: %d\n",    g->attacco_psichico);
    printf("Difesa Psichica:  %d\n",    g->difesa_psichica);
    printf("Fortuna:          %d\n",    g->fortuna);
    printf("\n--- Inventario ---\n");

    for (i = 0; i < ZAINO_MAX; i++) {
        printf("  Slot %d: %s\n", i + 1, tipo_oggetto_to_string(g->zaino[i]));
    }
    printf("\n");
    printf("================================================================================\n");
}

// Stampa le informazioni dettagliate della zona in cui si trova il giocatore, inclusi tipo di zona, nemici presenti e oggetti disponibili
static void stampa_zona_corrente(Giocatore* g) {
    if (g == NULL) {
        printf("Errore: giocatore non valido!\n");
        return;
    }

    printf("\n");
    printf("================================================================================\n");
    printf("                     DOVE TI TROVI                                              \n");
    printf("================================================================================\n");

    if (g->mondo == MONDO_REALE) {
        if (g->pos_mondoreale != NULL) {
            printf("\nSei nel MONDO REALE\n");
            printf("Zona: %s\n", tipo_zona_to_string(g->pos_mondoreale->tipo));
            printf("\n");

            if (g->pos_mondoreale->nemico != NESSUN_NEMICO) {// C'e' un nemico nella zona
                printf("*** ATTENZIONE: PRESENZA NEMICA! ***\n\n");
                switch (g->pos_mondoreale->nemico) {
                    case BILLI:
                        printf("Una presenza inquietante si muove nell'ombra...\n");
                        printf("E' Billi! Un ragazzo ribelle e violento che e' stato posseduto.\n");
                        printf("Ti blocca il passaggio con uno sguardo vuoto e minaccioso!\n");
                        break;
                    case DEMOCANE:
                        printf("Un ringhio sordo risuona nell'aria gelida...\n");
                        printf("Un Democane emerge dall'oscurita', mostrando i denti!\n");
                        printf("Le sue zanne brillano nella penombra.\n");
                        break;
                    case DEMOTORZONE:
                        printf("L'aria diventa elettrica, i capelli si rizzano...\n");
                        printf("IL DEMOTORZONE! La creatura piu' temibile di tutte!\n");
                        printf("Ma aspetta... non dovrebbe essere qui!\n");
                        break;
                    default:
                        break;
                }
            } else { // Nessun nemico nella zona
                printf("L'area sembra tranquilla... per ora.\n");
                printf("Nessuna minaccia immediata, ma resta vigile.\n");
            }

            if (g->pos_mondoreale->oggetto != NESSUN_OGGETTO) { // C'e' un oggetto nella zona
                printf("\n");
                printf(">>> Noti qualcosa che luccica a terra <<<\n");
                printf("E' %s!\n", tipo_oggetto_to_string(g->pos_mondoreale->oggetto));
                if (g->pos_mondoreale->nemico == NESSUN_NEMICO) {
                    printf("Potresti raccoglierlo se vuoi.\n");
                } else {
                    printf("Ma prima devi liberarti del nemico!\n");
                }
            }
        } else {
            printf("Errore: posizione non valida!\n");
        }
    } else {
        if (g->pos_soprasotto != NULL) { // Soprasotto
            printf("\nSei nel SOPRASOTTO - La Dimensione Oscura\n");
            printf("Zona: %s (versione distorta e inquietante)\n",
                   tipo_zona_to_string(g->pos_soprasotto->tipo));
            printf("\n");
            printf("L'aria e' gelida e spettrale.\n");
            printf("Tutto sembra sbagliato qui. Le ombre si muovono da sole.\n");
            printf("Il silenzio e' assordante.\n");

            if (g->pos_soprasotto->nemico != NESSUN_NEMICO) { // C'e' un nemico nel Soprasotto
                printf("\n*** PERICOLO IMMINENTE! ***\n\n");
                switch (g->pos_soprasotto->nemico) {
                    case DEMOCANE:
                        printf("Un ululato spettrale echeggia nelle tenebre...\n");
                        printf("Un Democane del Soprasotto ti ha trovato!\n");
                        printf("E' ancora piu' mostruoso della sua controparte reale!\n");
                        break;
                    case DEMOTORZONE:
                        printf("****************************************************\n");
                        printf("*                                                  *\n");
                        printf("*   Una forza elettrica riempie l'aria!            *\n");
                        printf("*   IL DEMOTORZONE SI ERGE DAVANTI A TE!           *\n");
                        printf("*   Questa e' la tua unica possibilita' di         *\n");
                        printf("*   salvare Occhinz!                               *\n");
                        printf("*                                                  *\n");
                        printf("****************************************************\n");
                        break;
                    default:
                        break;
                }
            } else {
                printf("\nPer ora non vedi minacce...\n");
                printf("Ma non abbassare la guardia. Qualcosa potrebbe essere in agguato.\n");
            }
        } else {
            printf("Errore: posizione non valida!\n");
        }
    }

    printf("\n");
    printf("================================================================================\n");
}

/* ============================================================================
 * FUNZIONI DI GIOCO - AZIONI GIOCATORE
 * ============================================================================ */

// Permette al giocatore di raccogliere un oggetto presente nella zona del Mondo Reale, se non ci sono nemici
static void raccogli_oggetto(Giocatore* g) {
    int i;
    int spazio_trovato = 0;

    if (g == NULL) {//
        printf("Errore: giocatore non valido!\n");
        return;
    }

    if (g->mondo != MONDO_REALE) {// Il giocatore e' nel Soprasotto, non puo' raccogliere oggetti
        printf("\nNel Soprasotto non ci sono oggetti da raccogliere...\n");
        printf("Solo oscurita' e pericolo ti circondano.\n");
        return;
    }

    if (g->pos_mondoreale == NULL) {// Non dovrebbe mai succedere, ma meglio controllare
        printf("Errore: posizione non valida!\n");
        return;
    }

    if (g->pos_mondoreale->nemico != NESSUN_NEMICO) {// C'e' un nemico nella zona, non si puo' raccogliere
        printf("\n*** IMPOSSIBILE RACCOGLIERE! ***\n");
        printf("C'e' %s qui!\n", tipo_nemico_to_string(g->pos_mondoreale->nemico));
        printf("E' troppo pericoloso raccogliere oggetti ora!\n");
        printf("Sconfiggilo prima di frugare in giro!\n");
        return;
    }

    if (g->pos_mondoreale->oggetto == NESSUN_OGGETTO) {// Non c'e' nessun oggetto nella zona
        printf("\nGuardi attentamente in giro ma non trovi nulla di utile.\n");
        printf("La zona e' vuota.\n");
        return;
    }

    for (i = 0; i < ZAINO_MAX; i++) {// Cerca uno slot vuoto nello zaino
        if (g->zaino[i] == NESSUN_OGGETTO) {// Slot vuoto trovato
            printf("\nTi avvicini cautamente all'oggetto...\n");
            printf("\n>>> RACCOLTO: %s! <<<\n", tipo_oggetto_to_string(g->pos_mondoreale->oggetto));
            printf("Lo infili nello zaino (slot %d).\n", i + 1);
            printf("Potrebbe tornare molto utile!\n");

            g->zaino[i] = g->pos_mondoreale->oggetto;
            g->pos_mondoreale->oggetto = NESSUN_OGGETTO;
            spazio_trovato = 1;
            break;
        }
    }

    if (!spazio_trovato) {// Nessuno slot vuoto trovato, lo zaino e' pieno
        printf("\n*** ZAINO PIENO! ***\n");
        printf("Il tuo zaino e' pieno zeppo!\n");
        printf("Devi usare qualcosa prima di raccogliere altro.\n");
        printf("Apri l'inventario e utilizza un oggetto per fare spazio.\n");
    }
}

// Permette al giocatore di utilizzare un oggetto presente nel suo zaino, applicando i bonus corrispondenti e consumando l'oggetto
static void utilizza_oggetto(Giocatore* g) {
    int scelta;
    int i;

    if (g == NULL) {// Controllo di sicurezza
        printf("Errore: giocatore non valido!\n");
        return;
    }

    printf("\n");
    printf("================================================================================\n");
    printf("                        IL TUO ZAINO                                            \n");
    printf("================================================================================\n");
    printf("\n");
    for (i = 0; i < ZAINO_MAX; i++) {// Elenca gli oggetti presenti nello zaino
        printf("%d) %s\n", i + 1, tipo_oggetto_to_string(g->zaino[i]));
    }
    printf("%d) Annulla\n", ZAINO_MAX + 1);
    printf("\n");
    printf("Quale oggetto vuoi usare? ");

    if (scanf("%d", &scelta) != 1) {
        while (getchar() != '\n');
        printf("Errore: devi inserire un numero!\n");
        return;
    }
    while (getchar() != '\n');

    if (scelta < 1 || scelta > ZAINO_MAX + 1) {
        printf("Scelta non valida!\n");
        return;
    }

    if (scelta == ZAINO_MAX + 1) {// Il giocatore ha scelto di annullare
        printf("Operazione annullata.\n");
        return;
    }

    if (g->zaino[scelta - 1] == NESSUN_OGGETTO) {// Lo slot scelto e' vuoto, non c'e' niente da usare
        printf("\nNessun oggetto in questa posizione dello zaino!\n");
        return;
    }

    printf("\n");
    printf("================================================================================\n");
    printf("                    UTILIZZO OGGETTO                                            \n");
    printf("================================================================================\n");
    printf("\n");

    switch (g->zaino[scelta - 1]) {
        case BICICLETTA: // BONUS: +2 Fortuna (PERMANENTE)
            printf("Usi la Bicicletta!\n");
            printf("Pedalare ti fa sentire piu' fortunato e fiducioso.\n");
            printf("Fortuna +%d (PERMANENTE)\n", BONUS_BICICLETTA_FORTUNA);
            g->fortuna += BONUS_BICICLETTA_FORTUNA;
            break;

        case MAGLIETTA_FUOCOINFERNO: // BONUS: +3 Attacco Psichico (PERMANENTE)
            printf("Indossi la Maglietta Fuocoinferno!\n");
            printf("Senti il potere del fuoco scorrere in te!\n");
            printf("Attacco Psichico +%d (PERMANENTE)\n", BONUS_MAGLIETTA_ATTACCO);
            g->attacco_psichico += BONUS_MAGLIETTA_ATTACCO;
            break;

        case BUSSOLA:
            printf("Usi la Bussola!\n"); // BONUS: +2 Fortuna (PERMANENTE)
            printf("Ti orienti meglio, trovando la via giusta.\n");
            printf("La tua intuizione migliora.\n");
            printf("Fortuna +%d (PERMANENTE)\n", BONUS_BUSSOLA_FORTUNA);
            g->fortuna += BONUS_BUSSOLA_FORTUNA;
            break;

        case SCHITARRATA_METALLICA:// BONUS: +2 Attacco Psichico, +1 Difesa Psichica (PERMANENTE)
            printf("Suoni una Schitarrata Metallica!\n");
            printf("La musica ti da' forza e coraggio!\n");
            printf("Attacco +%d, Difesa +%d (PERMANENTE)\n",
                   BONUS_SCHITARRATA_ATTACCO, BONUS_SCHITARRATA_DIFESA);
            g->attacco_psichico += BONUS_SCHITARRATA_ATTACCO;
            g->difesa_psichica  += BONUS_SCHITARRATA_DIFESA;
            break;

        default:
            printf("Oggetto sconosciuto!\n");
            return;
    }

    g->zaino[scelta - 1] = NESSUN_OGGETTO; // Consuma l'oggetto, lo slot torna vuoto

    printf("\nOggetto utilizzato e consumato.\n");
    printf("Lo slot %d del tuo zaino e' ora vuoto.\n", scelta);
    printf("\n");
    printf("================================================================================\n");
}

// Controlla se c'e' un nemico nella zona in cui si trova il giocatore, restituendo 1 se c'e' un nemico e 0 altrimenti
static int ha_nemico_zona(Giocatore* g) {
    if (g == NULL) return 0; // Controllo di sicurezza

    if (g->mondo == MONDO_REALE) {// Controlla il Mondo Reale
        if (g->pos_mondoreale != NULL && g->pos_mondoreale->nemico != NESSUN_NEMICO) {
            return 1;
        }
    } else {// Controlla il Soprasotto
        if (g->pos_soprasotto != NULL && g->pos_soprasotto->nemico != NESSUN_NEMICO) {
            return 1;
        }
    }

    return 0;
}

// Permette al giocatore di avanzare alla zona successiva, se non ci sono nemici che bloccano il passaggio
static void avanza(Giocatore* g) {
    if (g == NULL) {
        printf("Errore: giocatore non valido!\n");
        return;
    }

 
    if (ha_nemico_zona(g)) { // C'e' un nemico nella zona, non si puo' avanzare
        printf("\n*** IMPOSSIBILE AVANZARE! ***\n");
        printf("C'e' un nemico che ti blocca il passaggio!\n");
        printf("Devi sconfiggerlo prima di procedere!\n");
        return;
    }

    if (g->mondo == MONDO_REALE) {// Avanza nel Mondo Reale
        if (g->pos_mondoreale != NULL) {
            if (g->pos_mondoreale->avanti != NULL) {
                g->pos_mondoreale = g->pos_mondoreale->avanti;
                printf("\n>>> Ti fai strada verso la zona successiva... <<<\n");
                stampa_zona_corrente(g);
            } else { // Non c'e' una zona successiva, sei alla fine del percorso
                printf("\n*** FINE DEL PERCORSO ***\n");
                printf("Davanti a te c'e' solo il vuoto.\n");
                printf("Non puoi andare oltre.\n");
            }
        }
    } else {// Avanza nel Soprasotto
        if (g->pos_soprasotto != NULL) { // Controllo di sicurezza
            if (g->pos_soprasotto->avanti != NULL) {// C'e' una zona successiva, puoi avanzare
                g->pos_soprasotto = g->pos_soprasotto->avanti;
                printf("\n>>> Avanzi cautamente nell'oscurita' del Soprasotto... <<<\n");
                stampa_zona_corrente(g);
            } else {// Non c'e' una zona successiva, sei alla fine del percorso
                printf("\n*** FINE DEL PERCORSO ***\n");
                printf("Davanti a te solo tenebra impenetrabile.\n");
                printf("Non puoi proseguire oltre.\n");
            }
        }
    }
}

// Permette al giocatore di tornare alla zona precedente, se non ci sono nemici che bloccano il passaggio
static void indietreggia(Giocatore* g) {
    if (g == NULL) {// Controllo di sicurezza
        printf("Errore: giocatore non valido!\n");
        return;
    }

    
    if (ha_nemico_zona(g)) {// C'e' un nemico nella zona, non si puo' indietreggiare
        printf("\n*** IMPOSSIBILE INDIETREGGIARE! ***\n");
        printf("C'e' un nemico che ti blocca!\n");
        printf("Devi sconfiggerlo prima di muoverti!\n");
        return;
    }

    if (g->mondo == MONDO_REALE) {// Indietreggia nel Mondo Reale
        if (g->pos_mondoreale != NULL) {
            if (g->pos_mondoreale->indietro != NULL) {// C'e' una zona precedente, puoi indietreggiare
                g->pos_mondoreale = g->pos_mondoreale->indietro;
                printf("\n>>> Torni sui tuoi passi, verso la zona precedente... <<<\n");
                stampa_zona_corrente(g);
            } else {
                printf("\n*** INIZIO DEL PERCORSO ***\n");
                printf("Sei gia' all'inizio.\n");
                printf("Non puoi tornare piu' indietro.\n");
            }
        }
    } else {// Indietreggia nel Soprasotto
        if (g->pos_soprasotto != NULL) {
            if (g->pos_soprasotto->indietro != NULL) {
                g->pos_soprasotto = g->pos_soprasotto->indietro;
                printf("\n>>> Indietreggi nell'oscurita'... <<<\n");
                stampa_zona_corrente(g);
            } else {
                printf("\n*** INIZIO DEL PERCORSO ***\n");
                printf("Non puoi tornare piu' indietro.\n");
                printf("Sei vicino al portale di entrata.\n");
            }
        }
    }
}

// Permette al giocatore di attraversare il portale tra il Mondo Reale e il Soprasotto, se si trovano nelle zone corrette e superano eventuali ostacoli
static int cambia_mondo(Giocatore* g) {
    int dado;

    if (g == NULL) {
        printf("Errore: giocatore non valido!\n");
        return 0;
    }

    if (g->mondo == MONDO_REALE) {// Dal Mondo Reale al Soprasotto: attraversamento automatico
        if (g->pos_mondoreale != NULL) {
            printf("\n");
            printf("================================================================================\n");
            printf("                    ATTRAVERSAMENTO PORTALE                                     \n");
            printf("================================================================================\n");
            printf("\n");
            printf("Davanti a te si apre un portale dimensionale...\n");
            printf("L'aria trema, la realta' si distorce.\n");
            printf("Colori impossibili danzano ai bordi del portale.\n");
            printf("\n");
            printf("Fai un respiro profondo ed entri.\n");
            printf("\n");
            printf("*** VIENI CATAPULTATO NEL SOPRASOTTO! ***\n");
            printf("\n");
            printf("Tutto e' distorto, oscuro, sbagliato...\n");
            printf("Il freddo ti penetra nelle ossa.\n");
            printf("================================================================================\n");

            g->pos_soprasotto = g->pos_mondoreale->link_soprasotto;
            g->pos_mondoreale = NULL; /* FIX: pulisce il riferimento al mondo precedente */
            g->mondo = SOPRASOTTO;
            stampa_zona_corrente(g);
            return 1;
        }
    } else {
        /* Dal Soprasotto al Mondo Reale: tiro di fortuna */
        printf("\n");
        printf("================================================================================\n");
        printf("                    TENTATIVO DI FUGA                                           \n");
        printf("================================================================================\n");
        printf("\n");
        printf("Cerchi disperatamente un portale per tornare a casa...\n");
        printf("La paura ti attanaglia, ma devi provare!\n");
        printf("Chiudi gli occhi e ti concentri sulla realta'...\n");
        printf("Visualizzi il Mondo Reale, i colori veri, la luce...\n");
        printf("\n");

        dado = lancia_dado();// Tiro di fortuna contro la fortuna del giocatore

        printf("[Tiro di Fortuna: %d VS Tua Fortuna: %d]\n\n", dado, g->fortuna);

        if (dado < g->fortuna) {// Successo: il giocatore riesce a tornare al Mondo Reale
            printf("================================================================================\n");
            printf("                         *** CE L'HAI FATTA! ***                                \n");
            printf("================================================================================\n");
            printf("\n");
            printf("Un portale luminoso si apre davanti a te!\n");
            printf("Ti tuffi attraverso con tutte le tue forze e...\n");
            printf("\n");
            printf("...TORNI AL MONDO REALE!\n");
            printf("\n");
            printf("L'aria fresca, i colori normali, il calore del sole...\n");
            printf("Sei salvo! Almeno per ora.\n");
            printf("================================================================================\n");

            if (g->pos_soprasotto != NULL) {// Controllo di sicurezza
                g->pos_mondoreale = g->pos_soprasotto->link_mondoreale;
                g->pos_soprasotto = NULL; /* FIX: pulisce il riferimento al mondo precedente */
                g->mondo = MONDO_REALE;
                stampa_zona_corrente(g);
                return 1;
            }
        } else {// Fallimento: il giocatore non riesce a tornare al Mondo Reale
            printf("================================================================================\n");
            printf("                        *** NON CE LA FAI! ***                                  \n");
            printf("================================================================================\n");
            printf("\n");
            printf("Il portale sfarfalla per un momento...\n");
            printf("Quasi... quasi riesci a vederlo...\n");
            printf("Ma poi scompare!\n");
            printf("\n");
            printf("Sei ancora intrappolato nel Soprasotto!\n");
            printf("L'oscurita' ti circonda.\n");
            printf("Dovrai riprovare piu' tardi.\n");
            printf("================================================================================\n");
            return 0;
        }
    }

    return 0;
}

/* ============================================================================
 * SISTEMA DI COMBATTIMENTO
 * ============================================================================ */

// Inizializza le statistiche di un nemico in base al suo tipo, restituendo HP, attacco e difesa tramite parametri di output
static void inizializza_statistiche_nemico(Tipo_nemico nemico, int* hp, int* attacco, int* difesa) {
    switch (nemico) {
        case BILLI:
            *hp      = HP_BILLI;
            *attacco = ATTACCO_BILLI;
            *difesa  = DIFESA_BILLI;
            break;
        case DEMOCANE:
            *hp      = HP_DEMOCANE;
            *attacco = ATTACCO_DEMOCANE;
            *difesa  = DIFESA_DEMOCANE;
            break;
        case DEMOTORZONE:
            *hp      = HP_DEMOTORZONE;
            *attacco = ATTACCO_DEMOTORZONE;
            *difesa  = DIFESA_DEMOTORZONE;
            break;
        default:
            *hp      = 0;
            *attacco = 0;
            *difesa  = 0;
            break;
    }
}

// Gestisce il combattimento tra il giocatore e un nemico presente nella zona, restituendo 1 se il giocatore vince, 0 se perde o non c'e' nessun nemico
static int combatti_nemico(Giocatore* g) {
    Tipo_nemico nemico;
    int hp_nemico;
    int attacco_nemico;
    int difesa_nemico;
    int scelta;
    int danno;
    int dado_giocatore, dado_nemico;
    int difesa_temporanea_attiva; // Flag per indicare se la difesa temporanea e' attiva (bonus di +3 difesa per un turno)

    if (g == NULL) {
        return 0;
    }

    if (g->mondo == MONDO_REALE) {// Controlla se c'e' un nemico nel Mondo Reale
        if (g->pos_mondoreale == NULL || g->pos_mondoreale->nemico == NESSUN_NEMICO) {
            printf("\nNon c'e' nessun nemico da combattere qui!\n");
            return 0;
        }
        nemico = g->pos_mondoreale->nemico;
    } else {// Controlla se c'e' un nemico nel Soprasotto
        if (g->pos_soprasotto == NULL || g->pos_soprasotto->nemico == NESSUN_NEMICO) {
            printf("\nNon c'e' nessun nemico da combattere qui!\n");
            return 0;
        }
        nemico = g->pos_soprasotto->nemico;
    }

    inizializza_statistiche_nemico(nemico, &hp_nemico, &attacco_nemico, &difesa_nemico);

    printf("\n");
    printf("================================================================================\n");
    printf("                       !!! COMBATTIMENTO !!!                                    \n");
    printf("================================================================================\n");
    printf("\n");

    switch (nemico) {
        case BILLI:
            printf("Billi ti fissa con occhi vuoti e minacciosi.\n");
            printf("Le sue mani tremano, posseduto da una forza oscura.\n");
            printf("Non hai scelta. Devi combattere per sopravvivere!\n");
            break;
        case DEMOCANE:
            printf("Il Democane ringhia e si prepara ad attaccare!\n");
            printf("Le sue fauci sbavano. La tensione e' palpabile.\n");
            printf("Preparati a difenderti!\n");
            break;
        case DEMOTORZONE:
            printf("****************************************************\n");
            printf("*                                                  *\n");
            printf("*        IL DEMOTORZONE TI HA TROVATO!             *\n");
            printf("*                                                  *\n");
            printf("*   Scintille elettriche crepitano nell'aria!      *\n");
            printf("*   Questa e' la battaglia finale!                 *\n");
            printf("*   SCONFIGGILO PER SALVARE OCCHINZ!               *\n");
            printf("*                                                  *\n");
            printf("****************************************************\n");
            break;
        default:
            printf("Un nemico ti sbarra la strada!\n");
            break;
    }

    printf("\n");
    printf("Statistiche nemico:\n");
    printf("  HP: %d | Attacco: %d | Difesa: %d\n", hp_nemico, attacco_nemico, difesa_nemico);
    printf("\n");
    printf("================================================================================\n");

    /* Loop di combattimento */
    while (hp_nemico > 0 && g->punti_vita > 0) {

        difesa_temporanea_attiva = 0; 

        printf("\n");
        printf("--- Turno di %s ---\n", g->nome);
        printf("Tuoi PV: %d/%d\n", g->punti_vita, PV_INIZIALI);
        printf("PV Nemico: %d\n", hp_nemico);
        printf("\n");
        printf("Azioni disponibili:\n");
        printf("1) Attacco base (danno normale)\n");
        printf("2) Attacco potenziato (-%d PV, danno x%.1f)\n",
               COSTO_ATTACCO_POTENZIATO, MOLTIPLICATORE_POTENZIATO);
        printf("3) Difesa (+%d difesa per questo turno)\n", BONUS_DIFESA_TEMPORANEO);
        printf("4) Utilizza oggetto dallo zaino\n");
        printf("Scegli azione: ");

        if (scanf("%d", &scelta) != 1) {
            while (getchar() != '\n');
            printf("Errore: devi inserire un numero!\n");
            continue;
        }
        while (getchar() != '\n');

        switch (scelta) {
            case 1:
                /* Attacco base */
                dado_giocatore = lancia_dado();
                dado_nemico    = lancia_dado();
                danno = (g->attacco_psichico + dado_giocatore) - (difesa_nemico + dado_nemico);

                if (danno < 0) danno = 0;
                hp_nemico -= danno;

                printf("\n>>> ATTACCO BASE! <<<\n");
                printf("Danno inflitto: %d\n", danno);
                printf("(Tuo attacco: %d + dado %d = %d VS Difesa nemica: %d + dado %d = %d)\n",
                       g->attacco_psichico, dado_giocatore, g->attacco_psichico + dado_giocatore,
                       difesa_nemico,       dado_nemico,    difesa_nemico + dado_nemico);
                break;

            case 2:
                /* Attacco potenziato */
                if (g->punti_vita <= COSTO_ATTACCO_POTENZIATO) {// Il giocatore non ha abbastanza PV per usare l'attacco potenziato
                    printf("\n*** NON HAI ABBASTANZA PV! ***\n");
                    printf("Hai solo %d PV, l'attacco ne costa %d.\n",
                           g->punti_vita, COSTO_ATTACCO_POTENZIATO);
                    printf("Usa l'attacco base o difenditi!\n");
                    continue;
                }

                g->punti_vita -= COSTO_ATTACCO_POTENZIATO;
                dado_giocatore = lancia_dado();
                dado_nemico    = lancia_dado();
                danno = ((int)((double)g->attacco_psichico * MOLTIPLICATORE_POTENZIATO) + dado_giocatore)
                        - (difesa_nemico + dado_nemico);

                if (danno < 0) danno = 0;
                hp_nemico -= danno;

                printf("\n>>> ATTACCO POTENZIATO! <<<\n");
                printf("Sacrifichi %d PV per un attacco devastante!\n", COSTO_ATTACCO_POTENZIATO);
                printf("Danno inflitto: %d\n", danno);
                break;

            case 3:
                /* Difesa temporanea */
                printf("\n>>> POSIZIONE DIFENSIVA! <<<\n");
                printf("Ti metti in guardia!\n");
                printf("Difesa +%d per questo turno.\n", BONUS_DIFESA_TEMPORANEO);
                g->difesa_psichica += BONUS_DIFESA_TEMPORANEO;
                difesa_temporanea_attiva = 1; 

            case 4:
                /* Usa oggetto: non consuma il turno di combattimento */
                utilizza_oggetto(g);
                continue;

            default:
                printf("Azione non valida!\n");
                continue;
        }

        /* Controlla se il nemico e' morto */
        if (hp_nemico <= 0) {
            if (difesa_temporanea_attiva) {
                g->difesa_psichica -= BONUS_DIFESA_TEMPORANEO;
            }

            printf("\n");
            printf("================================================================================\n");

            if (nemico == DEMOTORZONE) {
                printf("                    *** VITTORIA EPICA! ***                                     \n");
                printf("================================================================================\n");
                printf("\n");
                printf("Il Demotorzone emette un ultimo urlo elettrico!\n");
                printf("Scintille esplodono ovunque mentre crolla a terra!\n");
                printf("\n");
                printf("****************************************************\n");
                printf("*   CE L'HAI FATTA! HAI SCONFITTO IL DEMOTORZONE!  *\n");
                printf("*   HAI SALVATO OCCHINZ!                           *\n");
                printf("****************************************************\n");
            } else {
                printf("                    *** NEMICO SCONFITTO! ***                                   \n");
                printf("================================================================================\n");
                printf("\n");
                printf("%s cade a terra, sconfitto!\n", tipo_nemico_to_string(nemico));
                printf("Hai vinto il combattimento!\n");
            }

            /* Possibilita' che il nemico sparisca dalla zona */
            if (rand() % 2 == 0) {
                printf("\nIl corpo del nemico si dissolve nell'aria...\n");
                printf("La zona e' ora sicura.\n");
                if (g->mondo == MONDO_REALE) {
                    g->pos_mondoreale->nemico = NESSUN_NEMICO;
                } else {
                    g->pos_soprasotto->nemico = NESSUN_NEMICO;
                }
            } else {// Il nemico rimane a terra, ma potrebbe essere ancora pericoloso
                printf("\nIl nemico giace a terra, ma potrebbe non essere finita...\n");
                printf("Potrebbe ancora essere qui se qualcun altro passa.\n");
            }

            printf("\n");
            printf("================================================================================\n");

            if (nemico == DEMOTORZONE) {
                return 2;
            }
            return 1;
        }

        /* ====================================================================
         * TURNO DEL NEMICO
         * ==================================================================== */
        printf("\n");
        printf("--- Il nemico contrattacca! ---\n");
        dado_nemico    = lancia_dado();
        dado_giocatore = lancia_dado();
        danno = (attacco_nemico + dado_nemico) - (g->difesa_psichica + dado_giocatore);

        if (danno < 0) danno = 0;

        if (danno == 0) {
            printf("\n>>> HAI PARATO L'ATTACCO! <<<\n");
            printf("Nessun danno subito!\n");
        } else {
            g->punti_vita -= danno;

            if (danno < 5) {
                printf("\n%s ti graffia leggermente.\n", tipo_nemico_to_string(nemico));
                printf("Danni subiti: %d\n", danno);
            } else if (danno < 10) {
                printf("\n%s ti colpisce duramente!\n", tipo_nemico_to_string(nemico));
                printf("Danni subiti: %d\n", danno);
                printf("Senti il dolore penetrarti!\n");
            } else {
                printf("\n*** COLPO DEVASTANTE! ***\n");
                printf("%s sferra un attacco micidiale!\n", tipo_nemico_to_string(nemico));
                printf("Danni subiti: %d\n", danno);
                printf("Barcollando, riesci a rimanere in piedi!\n");
            }
        }

        printf("(Attacco nemico: %d + dado %d = %d VS Tua difesa: %d + dado %d = %d)\n",
               attacco_nemico,      dado_nemico,    attacco_nemico + dado_nemico,
               g->difesa_psichica,  dado_giocatore, g->difesa_psichica + dado_giocatore);

        
        if (difesa_temporanea_attiva) {// Rimuove il bonus di difesa temporanea alla fine del turno del nemico
            g->difesa_psichica -= BONUS_DIFESA_TEMPORANEO;
            difesa_temporanea_attiva = 0;
        }

        /* Controlla se il giocatore e' morto */
        if (g->punti_vita <= 0) {
            printf("\n");
            printf("================================================================================\n");
            printf("                    *** SEI STATO SCONFITTO ***                                 \n");
            printf("================================================================================\n");
            printf("\n");
            printf("Le tue forze ti abbandonano...\n");
            printf("Le ginocchia cedono...\n");
            printf("Tutto diventa nero...\n");
            printf("\n");
            printf(">>> %s e' morto. <<<\n", g->nome);
            printf("\n");
            printf("================================================================================\n");
            return -1;
        }
    }

    return 0;
}

/* ============================================================================
 * FUNZIONE PUBBLICA: GIOCA
 * ============================================================================ */

// Gestisce il flusso principale del gioco, alternando i turni dei giocatori, gestendo le azioni e i combattimenti, e controllando le condizioni di vittoria o sconfitta
void gioca(void) {
    int i;
    int scelta;
    int turno = 0;
    int giocatori_vivi;
    int num_vivi_round = 0; /* FIX: fisso per tutto il round */
    int ordine_turno[4];
    int idx_turno = 0;
    int risultato_combattimento;
    int giocatore_corrente;
    int nemico_presente;
    int mossa_effettuata;
    int appena_mosso_con_nemico;
    int turno_finito;

    printf("\n");
    printf("================================================================================\n");
    printf("                     L'AVVENTURA HA INIZIO                                      \n");
    printf("================================================================================\n");

    if (!gioco_impostato) {// Controllo se il gioco e' stato impostato, altrimenti mostra un messaggio di errore e torna al menu principale
        printf("\n*** ERRORE ***\n");
        printf("Devi prima impostare il gioco dal menu principale!\n");
        printf("Seleziona l'opzione 1) Imposta gioco.\n");
        return;
    }

    /* Posiziona tutti i giocatori nella prima zona del Mondo Reale */
    for (i = 0; i < num_giocatori; i++) {
        if (giocatori[i] != NULL) {
            giocatori[i]->pos_mondoreale = prima_zona_mondoreale;
            giocatori[i]->pos_soprasotto = NULL;
            giocatori[i]->mondo = MONDO_REALE;
        }
    }

    printf("\n");
    printf("Ti trovi a Occhinz, la tranquilla cittadina ora infestata da portali.\n");
    printf("Davanti a te si estende il percorso verso il Soprasotto...\n");
    printf("Ricorda: solo sconfiggendo il Demotorzone salverai la citta'!\n");
    printf("\nTutti i giocatori partono dalla prima zona del Mondo Reale.\n");
    printf("Che l'avventura abbia inizio!\n");
    printf("================================================================================\n");

    /* ========================================================================
     * LOOP PRINCIPALE DI GIOCO
     * ======================================================================== */
    while (1) {

        // Ricalcola giocatori vivi all'inizio di ogni iterazione
        giocatori_vivi = 0;
        for (i = 0; i < num_giocatori; i++) {
            if (giocatori[i] != NULL) giocatori_vivi++;
        }

        // Condizione di sconfitta 
        if (giocatori_vivi == 0) {
            printf("\n");
            printf("================================================================================\n");
            printf("                         GAME OVER                                              \n");
            printf("================================================================================\n");
            printf("\n");
            printf("Tutti i giocatori sono caduti in battaglia...\n");
            printf("Il Demotorzone continua a terrorizzare Occhinz.\n");
            printf("La citta' e' perduta.\n");
            printf("\n");
            printf("Forse la prossima volta...\n");
            printf("================================================================================\n");
            gioco_impostato = 0;
            break;
        }

        // Inizio nuovo round: genera ordine casuale e congela num_vivi_round 
        if (idx_turno == 0) {
            int temp_idx[4];

            num_vivi_round = 0; 

            for (i = 0; i < num_giocatori; i++) {
                if (giocatori[i] != NULL) {
                    temp_idx[num_vivi_round] = i;
                    num_vivi_round++;
                }
            }

        
            for (i = 0; i < num_vivi_round; i++) {// Fisher-Yates shuffle per mescolare l'ordine dei giocatori
                int r   = rand() % num_vivi_round;
                int tmp = temp_idx[i];
                temp_idx[i] = temp_idx[r];
                temp_idx[r] = tmp;
            }

            for (i = 0; i < num_vivi_round; i++) {
                ordine_turno[i] = temp_idx[i];
            }

            turno++;
            printf("\n");
            printf("================================================================================\n");
            printf("                           ROUND %d                                             \n", turno);
            printf("================================================================================\n");
        }

        
        if (idx_turno >= num_vivi_round) {//confronto con num_vivi_round, non con num_giocatori
            idx_turno = 0;
            continue;
        }

        giocatore_corrente = ordine_turno[idx_turno];

        // Salta se il giocatore e' morto durante questo round
        if (giocatori[giocatore_corrente] == NULL) {
            idx_turno++;
            if (idx_turno >= num_vivi_round) { // Se abbiamo finito i giocatori vivi per questo round, resetta l'ordine e inizia un nuovo round
                idx_turno = 0;
            }
            continue;
        }

        printf("\n");
        printf("================================================================================\n");
        printf("                    Turno di: %s                                   \n",
               giocatori[giocatore_corrente]->nome);
        printf("================================================================================\n");

        stampa_zona_corrente(giocatori[giocatore_corrente]);

        nemico_presente         = ha_nemico_zona(giocatori[giocatore_corrente]);
        mossa_effettuata        = 0;
        appena_mosso_con_nemico = 0;
        turno_finito            = 0;

        /* ====================================================================
         * LOOP AZIONI DEL TURNO
         * ==================================================================== */
        while (!turno_finito) {
            printf("\n");
            printf("--- Azioni Disponibili ---\n");
            printf("1) Avanza alla zona successiva\n");
            printf("2) Indietreggia alla zona precedente\n");
            printf("3) Cambia mondo (attraversa portale)\n");
            printf("4) Combatti nemico\n");
            printf("5) Visualizza valori giocatore\n");
            printf("6) Visualizza zona corrente\n");
            printf("7) Raccogli oggetto\n");
            printf("8) Utilizza oggetto dallo zaino\n");
            printf("9) Passa il turno\n");
            printf("\n");
            printf("Scegli azione: ");

            if (scanf("%d", &scelta) != 1) {
                while (getchar() != '\n');
                printf("Errore: devi inserire un numero!\n");
                continue;
            }
            while (getchar() != '\n');

            switch (scelta) {
                case 1:
                    /* Avanza */
                    if (nemico_presente) {
                        printf("\n*** IMPOSSIBILE AVANZARE! ***\n");
                        printf("C'e' un nemico che ti blocca il passaggio!\n");
                        printf("Devi sconfiggerlo prima di procedere!\n");
                    } else if (mossa_effettuata) {
                        printf("\n*** HAI GIA' FATTO UNA MOSSA! ***\n");
                        printf("Puoi fare una sola mossa di movimento per turno.\n");
                    } else {
                        avanza(giocatori[giocatore_corrente]);
                        mossa_effettuata = 1;

                        if (ha_nemico_zona(giocatori[giocatore_corrente])) {
                            printf("\n>>> ATTENZIONE: NEMICO RILEVATO! <<<\n");
                            printf("Turno terminato.\n");
                            printf("Dovrai affrontarlo nel prossimo turno.\n");
                            appena_mosso_con_nemico = 1;
                        }
                    }
                    break;

                case 2:
                    /* Indietreggia */
                    if (nemico_presente) {
                        printf("\n*** IMPOSSIBILE INDIETREGGIARE! ***\n");
                        printf("C'e' un nemico che ti blocca!\n");
                        printf("Devi sconfiggerlo prima di muoverti!\n");
                    } else if (mossa_effettuata) {
                        printf("\n*** HAI GIA' FATTO UNA MOSSA! ***\n");
                        printf("Puoi fare una sola mossa di movimento per turno.\n");
                    } else {
                        indietreggia(giocatori[giocatore_corrente]);
                        mossa_effettuata = 1;

                        if (ha_nemico_zona(giocatori[giocatore_corrente])) {
                            printf("\n>>> ATTENZIONE: NEMICO RILEVATO! <<<\n");
                            printf("Turno terminato.\n");
                            printf("Dovrai affrontarlo nel prossimo turno.\n");
                            appena_mosso_con_nemico = 1;
                        }
                    }
                    break;

                case 3:
                    /* Cambia mondo */
                    if (nemico_presente && giocatori[giocatore_corrente]->mondo == MONDO_REALE) { // Il giocatore non puo' attraversare il portale se c'e' un nemico nel Mondo Reale, ma puo' farlo se e' nel Soprasotto (tentativo di fuga disperato)
                        printf("\n*** IMPOSSIBILE CAMBIARE MONDO! ***\n");
                        printf("C'e' un nemico che ti blocca!\n");
                        printf("Devi sconfiggerlo prima di attraversare il portale!\n");
                    } else if (mossa_effettuata) {
                        printf("\n*** HAI GIA' FATTO UNA MOSSA! ***\n");
                        printf("Puoi fare una sola mossa per turno.\n");
                    } else {
                        if (nemico_presente && giocatori[giocatore_corrente]->mondo == SOPRASOTTO) {
                            printf("\n>>> TENTATIVO DI FUGA DAL NEMICO! <<<\n");
                        }
                        if (cambia_mondo(giocatori[giocatore_corrente])) { // Se il cambio mondo e' riuscito, controlla se c'e' un nemico nella nuova zona
                            mossa_effettuata = 1;
                            nemico_presente  = ha_nemico_zona(giocatori[giocatore_corrente]);

                            if (nemico_presente) {
                                printf("\n>>> ATTENZIONE: NEMICO RILEVATO! <<<\n");
                                printf("Turno terminato.\n");
                                appena_mosso_con_nemico = 1;
                            }
                        } else if (giocatori[giocatore_corrente]->mondo == SOPRASOTTO) {
                            printf("\nSei ancora nel Soprasotto con il nemico!\n");
                        }
                    }
                    break;

                case 4:
                    /* Combatti */
                    if (appena_mosso_con_nemico) {
                        printf("\n*** NON PUOI COMBATTERE ORA! ***\n");
                        printf("Hai appena fatto una mossa in questo turno!\n");
                        printf("Dovrai aspettare il prossimo turno.\n");
                    } else { // Combatti il nemico presente nella zona
                        risultato_combattimento =
                            combatti_nemico(giocatori[giocatore_corrente]);

                        if (risultato_combattimento == -1) {
                            /* Giocatore morto */
                            printf("\n");
                            printf(">>> %s e' caduto in battaglia... <<<\n",
                                   giocatori[giocatore_corrente]->nome);
                            printf("Il suo nome sara' ricordato negli annali di Occhinz.\n");
                            free(giocatori[giocatore_corrente]);
                            giocatori[giocatore_corrente] = NULL;
                            turno_finito = 1;

                        } else if (risultato_combattimento == 2) { 
                            // Vittoria finale: Demotorzone sconfitto
                            printf("\n");
                            printf("================================================================================\n");
                            printf("                    *** VITTORIA FINALE ***                                    \n");
                            printf("================================================================================\n");
                            printf("\n");
                            printf("Con il Demotorzone sconfitto, i portali cominciano a chiudersi!\n");
                            printf("Il Soprasotto si dissolve, la realta' torna normale!\n");
                            printf("\n");
                            printf(">>> %s ha salvato Occhinz! <<<\n",
                                   giocatori[giocatore_corrente]->nome);
                            printf("\n");
                            printf("La citta' e' salva. Le biciclette scomparse riappaiono misteriosamente.\n");
                            printf("I Waffle Undici non sono mai stati cosi' buoni.\n");
                            printf("Sei un eroe!\n");
                            printf("================================================================================\n");
                            // Aggiorna la classifica degli ultimi vincitori
                            strncpy(ultimo_vincitore[2], ultimo_vincitore[1], NOME_MAX);
                            strncpy(ultimo_vincitore[1], ultimo_vincitore[0], NOME_MAX);
                            strncpy(ultimo_vincitore[0],
                                    giocatori[giocatore_corrente]->nome, NOME_MAX);
                            partite_giocate++;

                            gioco_impostato = 0;
                            return;

                        } else if (risultato_combattimento == 1) {
                            /* Vittoria normale */
                            nemico_presente = 0;
                            printf("\nOra puoi muoverti liberamente!\n");
                        }
                    }
                    break;

                case 5:
                    stampa_giocatore_info(giocatori[giocatore_corrente]);
                    break;

                case 6:
                    stampa_zona_corrente(giocatori[giocatore_corrente]);
                    break;

                case 7:
                    raccogli_oggetto(giocatori[giocatore_corrente]);
                    break;

                case 8:
                    utilizza_oggetto(giocatori[giocatore_corrente]);
                    break;

                case 9:
                    if (nemico_presente && !appena_mosso_con_nemico) { // Il giocatore non puo' passare il turno se c'e' un nemico presente e non ha appena fatto una mossa (per evitare di passare dopo essere entrati in una zona con nemico)
                        printf("\nNon puoi passare il turno! C'e' un nemico!\n");
                        printf("Devi combatterlo!\n");
                    } else {
                        printf("\nPassi il turno.\n");
                        turno_finito = 1;
                    }
                    break;

                default:
                    printf("Scelta non valida!\n");
                    continue;
            }

            /* Termina il turno se il giocatore e' morto, ha passato,
             * o si e' mosso in una zona con nemico */
            if (turno_finito || appena_mosso_con_nemico) {
                break;
            }
        }
        // Avanza al prossimo giocatore vivo nell'ordine del turno 
        idx_turno++;
        if (idx_turno >= num_vivi_round) { 
            idx_turno = 0;
        }
    }
}

/* ============================================================================
 * FUNZIONI PUBBLICHE: TERMINA_GIOCO E CREDITI
 * ============================================================================ */

/**
 * Termina il gioco, libera la memoria e saluta il giocatore
 */
void termina_gioco(void) {
    printf("\n");
    printf("================================================================================\n");
    printf("                         ARRIVEDERCI!                                           \n");
    printf("================================================================================\n");
    printf("\nGrazie per aver giocato a Cosestrane!\n");
    printf("Alla prossima avventura!\n\n");

    libera_giocatori();
    libera_mappe();
}

/**
 * Mostra i crediti del gioco e le statistiche delle ultime partite
 */
void crediti(void) {
    int i;

    printf("\n");
    printf("================================================================================\n");
    printf("                            CREDITI                                             \n");
    printf("================================================================================\n");
    printf("\n");
    printf("Gioco: Cosestrane\n");
    printf("Creato da: Hanaji Tancre'\n");
    printf("Anno: 2025-2026\n");
    printf("Corso: Programmazione Procedurale\n");
    printf("\n");
    printf("Partite giocate: %d\n", partite_giocate);
    printf("\n");
    printf("Ultimi vincitori:\n");
    for (i = 0; i < 3; i++) {
        printf("  %d) %s\n", i + 1, ultimo_vincitore[i]);
    }
    printf("\n");
    printf("================================================================================\n");
    printf("\n");
}
