#ifndef GAMELIB_H
#define GAMELIB_H

/* ============================================================================
 * COSTANTI DI GIOCO
 * ============================================================================ */

/* Parametri base giocatore */
#define PV_INIZIALI  80
#define ZONE_MINIME  15
#define NOME_MAX     50
#define ZAINO_MAX     3

/* Probabilità generazione nemici Mondo Reale (%) */
#define PROB_NESSUN_NEMICO_MR    40
#define PROB_DEMOCANE_MR         30  /* 40-70% = Democane */
/* 70-100% = Billi */

/* Probabilità generazione nemici Soprasotto (%) */
#define PROB_NESSUN_NEMICO_SS    50

/* Probabilità generazione oggetti (%) */
#define PROB_NESSUN_OGGETTO      50
#define PROB_BICICLETTA          15  /* 50-65% */
#define PROB_MAGLIETTA           15  /* 65-80% */
#define PROB_BUSSOLA             10  /* 80-90% */
/* 90-100% = Schitarrata */

/* Statistiche nemici */
#define HP_BILLI                 20
#define ATTACCO_BILLI             5
#define DIFESA_BILLI              3

#define HP_DEMOCANE              35
#define ATTACCO_DEMOCANE          8
#define DIFESA_DEMOCANE           5

#define HP_DEMOTORZONE           60
#define ATTACCO_DEMOTORZONE      12
#define DIFESA_DEMOTORZONE        7

/* Bonus oggetti */
#define BONUS_BICICLETTA_FORTUNA     3
#define BONUS_MAGLIETTA_ATTACCO      5
#define BONUS_BUSSOLA_FORTUNA        2
#define BONUS_SCHITARRATA_ATTACCO    3
#define BONUS_SCHITARRATA_DIFESA     3

/* Modifiche abilità */
#define MODIFICA_ATTACCO_DIFESA      3
#define BONUS_UNDICI_ATTACCO         4
#define BONUS_UNDICI_DIFESA          4
#define MALUS_UNDICI_FORTUNA         7

/* Combattimento */
#define COSTO_ATTACCO_POTENZIATO     3
#define MOLTIPLICATORE_POTENZIATO  1.5
#define BONUS_DIFESA_TEMPORANEO      5

/* ============================================================================
 * ENUMERAZIONI
 * ============================================================================ */

// Tipi di zona presenti nei due mondi
typedef enum {
    BOSCO,
    SCUOLA,
    LABORATORIO,
    CAVERNA,
    STRADA,
    GIARDINO,
    SUPERMERCATO,
    CENTRALE_ELETTRICA,
    DEPOSITO_ABBANDONATO,
    STAZIONE_POLIZIA
} Tipo_zona;

// Tipi di nemico presenti nei due mondi
typedef enum {
    NESSUN_NEMICO,
    BILLI,
    DEMOCANE,
    DEMOTORZONE
} Tipo_nemico;

// Tipi di oggetto che il giocatore può trovare
typedef enum {
    NESSUN_OGGETTO,
    BICICLETTA,              /* +3 Fortuna */
    MAGLIETTA_FUOCOINFERNO,  /* +5 Attacco */
    BUSSOLA,                 /* +2 Fortuna */
    SCHITARRATA_METALLICA    /* +3 Attacco, +3 Difesa */
} Tipo_oggetto;

// Tipi di mondo in cui il giocatore può trovarsi
typedef enum {
    MONDO_REALE,
    SOPRASOTTO
} Tipo_mondo;

/* ============================================================================
 * STRUTTURE DATI
 * ============================================================================ */


typedef struct Zona_soprasotto Zona_soprasotto;

// Struttura per una zona del Mondo Reale
typedef struct Zona_mondoreale {
    Tipo_zona tipo;                      /* Tipo di ambiente della zona */
    Tipo_nemico nemico;                  /* Nemico presente (se presente) */
    Tipo_oggetto oggetto;                /* Oggetto presente (se presente) */
    struct Zona_mondoreale* avanti;      /* Puntatore alla zona successiva */
    struct Zona_mondoreale* indietro;    /* Puntatore alla zona precedente */
    Zona_soprasotto* link_soprasotto;    /* Link alla zona parallela nel Soprasotto */
} Zona_mondoreale;

// Struttura per una zona del Soprasotto
struct Zona_soprasotto {
    Tipo_zona tipo;                      /* Tipo di ambiente (stesso del Mondo Reale) */
    Tipo_nemico nemico;                  /* Nemico presente (Democane o Demotorzone) */
    struct Zona_soprasotto* avanti;      /* Puntatore alla zona successiva */
    struct Zona_soprasotto* indietro;    /* Puntatore alla zona precedente */
    Zona_mondoreale* link_mondoreale;    /* Link alla zona parallela nel Mondo Reale */
};

// Struttura per rappresentare un giocatore
typedef struct Giocatore {
    char nome[NOME_MAX];                 /* Nome del giocatore */
    Tipo_mondo mondo;                    /* Mondo in cui si trova attualmente 0=MONDO_REALE, 1=SOPRASOTTO */
    Zona_mondoreale* pos_mondoreale;     /* Posizione nel Mondo Reale */
    Zona_soprasotto* pos_soprasotto;     /* Posizione nel Soprasotto */
    int attacco_psichico;                /* Statistica di attacco */
    int difesa_psichica;                 /* Statistica di difesa */
    int fortuna;                         /* Statistica di fortuna */
    int punti_vita;                      /* Punti vita correnti */
    Tipo_oggetto zaino[ZAINO_MAX];       /* Inventario oggetti */
} Giocatore;

/* ============================================================================
 * FUNZIONI PUBBLICHE
 * ============================================================================ */

//inizializza il gioco, creando mappe e giocatori
void imposta_gioco(void);

//funzione principale di gioco, gestisce il ciclo di gioco e le interazioni
void gioca(void);

//funzione per terminare il gioco, pulisce risorse e mostra messaggio finale
void termina_gioco(void);

//funzione per visualizzare i crediti del gioco
void crediti(void);

#endif 
