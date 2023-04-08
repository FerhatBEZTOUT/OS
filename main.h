/* main.h */

/* Définitions des constantes */
#define MAX_LENGTH_TEAM 50  // nombre carac max du nom d'equipe
#define MAX_DUREE_ACTION 500000   // en micro seconde
#define DUREE_MATCH 5

/* Structures de données */


typedef struct equipe {
    int id;
    char *nom;
    struct equipe *suivant;
} *Equipe;

typedef struct {
    int nbrTour;
    Equipe *tour;
} tournoi;


typedef struct {
    int num_match;
    int num_tour;
} match;



/* ============================ Prototypes ============================ */
// FLit les équipes ligne par ligne depuis un fichier texte et les sauvegarder dans un tableau
void read_teams(char* filename, Equipe* listeEquipes, int* team_count);

// Garde le nombre d'equipe comme une puissance de 2 (on supprime les equipes excedentes)
void keep_power_of_two(Equipe* tete, int* team_count);

// crée une nouvelle equipe
Equipe nouvelle_equipe(char* nom, int id);

// insére une equipe en tete de liste
void inserer_tete(Equipe* tete, Equipe equipe);

// insérer une equipe en queue de liste
void inserer_queue(Equipe* tete, char* nom, int id);

// supprime la tête de la liste chaînée et renvoyer l'équipe supprimée
Equipe pop_tete(Equipe* tete);

//  affiche la liste chaînée d'équipes
void afficher_equipes(Equipe tete);

// libére la mémoire allouée pour chaque nœud de la liste chaînée
void liberer_equipes(Equipe tete);

// Crée un tableau de liste d'equipe (Chaque indice du tableau correspond à un tour du tournoi)
tournoi nouveau_tournoi(int nbrTour);

// Récupére la liste des equipes d'un tour i
Equipe get_equipe_tournoi(tournoi t, int index_tour);

// Insére une equipe dans un tour i
void inserer_equipe_tournoi(tournoi t, int index_tour, Equipe equipe);

// Affiche toutes les équipes de chaque tour
void afficher_equipe_tournoi(tournoi t);

// Libérer le tableau des tours (fin du tournoi)
void liberer_equipe_tournoi(tournoi t);

// Supprimer et récupérer une equipe d'un tour i
Equipe pop_equipe_at_tour(tournoi *t, int tour);

// Génére un nom aleatoire de 3 caractére 
char* generer_nom_equipe();

// Verifie si un entier une puissance de 2
int is_power_two(int n);

// Renvoie la puissance de 2 inférieur à n
int nearest_power_two(int n);

// Place toutes les equipes au tour 0
void start_tournoi(tournoi *t, Equipe e);

void *simuler_tournoi(void *arg);

Equipe simuler_match(Equipe e1, Equipe e2,int tour);

void* thread_function_match(void* arg);

void print_val_of_sem_tour(sem_t *sem, char* nom);

void toggle(int* state);