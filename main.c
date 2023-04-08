/**
 * @file main.c
 * @author Ferhat BEZTOUT
 * @brief
 * @version 1.0
 * @date 2023-04-02
 *
 * @copyright Copyright (c) 2023
 *
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include <pthread.h>
#include <stdint.h>
#include <semaphore.h>
#include <stdbool.h>
#include <unistd.h>

#include "main.h"

tournoi mon_tournoi;

sem_t *equipe1;
sem_t *equipe2;
int team_count;
int nbr_tours;
pthread_mutex_t my_mutex,wait_match = PTHREAD_MUTEX_INITIALIZER;
int *interrupteur;


/**
 * @brief lit les equipes dans un fichier donné et les sauvegarde dans une liste
 *
 * @param filename le chemin du fichier texte (une equipe par ligne)
 * @param listeEquipes liste des equipes du tournoi
 * @param team_count nombre d'equipes du tournoi
 */
void read_teams(char *filename, Equipe *listeEquipes, int *team_count)
{
    FILE *fp = fopen(filename, "r");
    if (fp == NULL)
    {
        perror("Erreur d'ouverture fichier");
        exit(EXIT_FAILURE);
    }

    char *buffer = malloc(MAX_LENGTH_TEAM * sizeof(char));
    if (buffer == NULL)
    {
        perror("Erreur allocation memoire");
        exit(EXIT_FAILURE);
    }

    *team_count = 0;
    while (fgets(buffer, MAX_LENGTH_TEAM, fp) != NULL)
    {
        buffer[strcspn(buffer, "\n")] = '\0';
        inserer_queue(listeEquipes, buffer, (*team_count) + 1);
        (*team_count)++;
    }

    fclose(fp);
    free(buffer);
}

/**
 * @brief Garder le nombre d'equipe comme une puissance de 2 (on supprime les equipes excedentes)
 *
 * @param tete le liste des equipes
 * @param team_count le nombre d'equipes
 */
void keep_power_of_two(Equipe *tete, int *team_count)
{
    // Vérification que la liste a au moins un élément
    if (*tete == NULL)
    {
        return;
    }

    int power = 1;
    int size = 2;
    while (size <= *team_count)
    {
        power++;
        size *= 2;
    }

    // Si le nombre d'éléments de la liste est déjà une puissance de 2, on ne fait rien
    if (size == *team_count)
    {
        return;
    }

    int elements_to_remove = *team_count - (size / 2);
    Equipe courant = *tete;
    while (courant->suivant != NULL && elements_to_remove > 0)
    {
        Equipe suivant = courant->suivant;
        if (suivant->suivant == NULL)
        {
            free(suivant);
            courant->suivant = NULL;
            *team_count -= 1;
        }
        else
        {
            while (suivant->suivant != NULL && suivant->suivant->suivant != NULL)
            {
                suivant = suivant->suivant;
            }
            free(suivant->suivant);
            suivant->suivant = NULL;
            *team_count -= 1;
        }
        elements_to_remove--;
    }
}

/**
 * @brief Fonction pour créer un nouveau nœud avec le nom d'équipe donné
 *
 * @param nom nom de l'equipe
 * @return Equipe
 */
Equipe nouvelle_equipe(char *nom, int id)
{
    Equipe equipe = (Equipe)malloc(sizeof(struct equipe));
    equipe->nom = (char *)malloc((strlen(nom) + 1) * sizeof(char)); // allouer suffisamment de mémoire pour le nom
    strcpy(equipe->nom, nom);                                       // copier le nom donné dans la nouvelle zone de mémoire allouée
    equipe->id = id;
    equipe->suivant = NULL;
    return equipe;
}




/**
 * @brief Insérer une equipe en tete de liste
 *
 * @param tete un pointeur vers la tête de la liste chaînée
 * @param equipe l'équipe à insérer en tête
 */
void inserer_tete(Equipe *tete, Equipe equipe)
{
    equipe->suivant = *tete;
    *tete = equipe;
}



/**
 * @brief insére une equipe à la fin de la liste
 *
 * @param tete tete de la liste
 * @param nom nom de l'equipe
 */
void inserer_queue(Equipe *tete, char *nom, int id)
{
    Equipe equipe = nouvelle_equipe(nom, id);
    if (*tete == NULL)
    {
        // Si la liste est vide, le nouveau nœud devient la tête et la queue
        *tete = equipe;
    }
    else
    {
        // Sinon, on parcourt la liste jusqu'à trouver le dernier nœud
        Equipe courant = *tete;
        while (courant->suivant != NULL)
        {

            courant = courant->suivant;
        }
        // On insère le nouveau nœud en queue
        courant->suivant = equipe;
    }
}

/**
 * @brief Supprime et récupére la tête (equipe) de la liste
 *
 * @param tete la liste des equipes
 * @return Equipe
 */
Equipe pop_tete(Equipe *tete)
{
    if (*tete == NULL)
    {
        printf("La liste chaînée est vide.\n");
        return NULL;
    }

    Equipe equipe_supprimee = *tete;
    *tete = (*tete)->suivant;
    equipe_supprimee->suivant = NULL;
    return equipe_supprimee;
}

/**
 * @brief Fonction pour afficher la liste chaînée d'équipes
 *
 * @param tete la liste des equipes
 */
void afficher_equipes(Equipe tete)
{
    Equipe courant = tete;
    while (courant != NULL)
    {
        printf("\tid:%d, nom: %s\n", courant->id, courant->nom);
        courant = courant->suivant;
    }
}

/**
 * @brief Fonction pour libérer la mémoire allouée pour chaque nœud de la liste chaînée
 *
 * @param tete la liste des equipes
 */
void liberer_equipes(Equipe tete)
{
    Equipe courant = tete;
    while (courant != NULL)
    {
        Equipe suivant = courant->suivant;
        free(courant->nom); // libérer la mémoire allouée pour le nom d'équipe
        free(courant);      // libérer la mémoire allouée pour le nœud
        courant = suivant;
    }
}

/**
 * @brief Crée un tableau de liste d'equipe (Chaque indice du tableau correspond à un tour du tournoi)
 *
 * @param nbrTour Nombre de tour du tournoi
 * @return t un tournoi
 */
tournoi nouveau_tournoi(int nbrTour)
{
    tournoi t;
    t.nbrTour = nbrTour;
    t.tour = malloc(nbrTour * sizeof(Equipe));
    for (int i = 0; i < nbrTour; i++)
    {
        t.tour[i] = NULL;
    }
    return t;
}

/**
 * @brief Récupérer la liste des equipes d'un tour i
 *
 * @param t structure contenant les tours (tableau de liste d'equipe)
 * @param index tour desiré
 * @return Equipe
 */
Equipe get_equipe_tournoi(tournoi t, int index_tour)
{
    if (index_tour < 0 || index_tour >= t.nbrTour)
    {
        printf("Index hors limite.\n");
        return NULL;
    }
    return t.tour[index_tour];
}

/**
 * @brief Insérer une equipe dans un tour i
 *
 * @param t structure contenant les tours (tableau de liste d'equipe)
 * @param index_tour tour désiré
 * @param equipe equipe à inserer
 */
void inserer_equipe_tournoi(tournoi t, int index_tour, Equipe equipe)
{
    if (index_tour < 0 || index_tour >= t.nbrTour)
    {
        printf("insert : Index %d hors limite.\n",index_tour);
        
        
    } else {
        inserer_tete(&t.tour[index_tour], equipe);
    }
   
}

/**
 * @brief Affiche toutes les équipes de chaque tour
 *
 * @param t structure contenant les tours (tableau de liste d'equipe)
 */
void afficher_equipe_tournoi(tournoi t)
{
    printf("=======Tournoi=======\n");
    for (int i = 0; i < t.nbrTour; i++)
    {
        printf("-Equipes du tour %d :\n", i);
        afficher_equipes(t.tour[i]);
    }
}

/**
 * @brief Libérer le tableau des tours (fin du tournoi)
 *
 * @param t structure contenant les tours (tableau de liste d'equipe)
 */
void liberer_equipe_tournoi(tournoi t)
{
    for (int i = 0; i < t.nbrTour; i++)
    {
        liberer_equipes(t.tour[i]);
    }
    free(t.tour);
}



/**
 * @brief Supprimer et récupérer une equipe d'un tour i
 *
 * @param t structure contenant les tours (tableau de liste d'equipe)
 * @param index_tour tour désiré
 * @return Equipe
 */
Equipe pop_equipe_at_tour(tournoi *t, int index_tour)
{
    return pop_tete(&(t->tour[index_tour]));
}



/**
 * @brief genere un nom aleatoire en majuscule de taille 3
 * 
 * @return char* le nom généré
 */
char *generer_nom_equipe()
{
    static char name[4];

    name[0] = 'A' + rand() % 26; // 1er caractere
    name[1] = 'A' + rand() % 26; // 2er caractere
    name[2] = 'A' + rand() % 26; // 3er caractere
    name[3] = '\0';              // fin de chaine

    return name;
}




/**
 * @brief Verifie si un entier est une puissance de 2
 *
 * @param n le nombre entier
 * @return int
 */
int is_power_two(int n)
{
    if (n <= 0)
    {
        return 0;
    }
    return (n & (n - 1)) == 0;
}

/**
 * @brief Renvoie la puissanec de 2 la plus proche inférieur à n
 *
 * @param n le nombre entier
 * @return int
 */
int nearest_power_two(int n)
{
    if (n <= 0)
    {
        return 0;
    }
    int i = 0;
    while ((1 << i) <= n)
    {
        i++;
    }
    return (1 << (i - 1));
}

/**
 * @brief débute le tournoi en plaçant toutes les equipe au tour 0
 *
 * @param t un tournoi donné
 * @param e une liste d'equipe
 */
void start_tournoi(tournoi *t, Equipe e)
{
    (*t).tour[0] = e;
}



Equipe simuler_match(Equipe e1, Equipe e2, int tour)
{

    int temps = 0;
    int score_e1 = 0;
    int score_e2 = 0;
    // Simuler le match
    printf("\033[0;33m[Tour %d]\033[0m Coup d'envoi entre %s et %s\n", tour, e1->nom, e2->nom);
    while (temps < DUREE_MATCH)
    {
        // Simuler une action
        if (rand() % 5 == 0)
        { // 1 chance sur 10 de marquer un but
            if (rand() % 2 == 0)
            {               // si l'équipe 1 marque
                score_e1++; // incrémenter le score de l'équipe 1
                printf("\033[0;33m[Tour %d]\033[0m %s a marqué !\n\t %s %d - %d %s\n", tour, e1->nom, e1->nom, score_e1, score_e2, e2->nom);
            }
            else
            {               // si l'équipe 2 marque
                score_e2++; // incrémenter le score de l'équipe 2
                printf("\033[0;33m[Tour %d]\033[0m %s a marqué !\n\t %s %d - %d %s\n", tour, e2->nom, e1->nom, score_e1, score_e2, e2->nom);
            }
        }
        temps++;
        usleep(rand() % MAX_DUREE_ACTION); // attendre 100 ms avant de simuler l'action suivante
    }

    printf("\033[0;33m[Tour %d]\033[0m Score final : %s %d - %d %s\n", tour, e1->nom, score_e1, score_e2, e2->nom);
    // Déterminer l'équipe gagnante
    if (score_e1 > score_e2)
    {
        printf("\033[0;33m[Tour %d] \033[0m\033[0;32mEquipe gagnante : %s\033[0m\n", tour, e1->nom);
        return e1; // l'équipe 1 est la gagnante
    }
    else if (score_e2 > score_e1)
    {
        printf("\033[0;33m[Tour %d] \033[0m\033[0;32mEquipe gagnante : %s\033[0m\n", tour, e2->nom);
        return e2; // l'équipe 2 est la gagnante
    }
    else
    {
        if (rand() % 2 == 0)
        {
            printf("\033[0;33m[Tour %d]\033[0m \033[0;32mScore nul, l'equipe %s gagne grâce aux penalties\033[0m\n", tour, e1->nom);
            return e1;
        }
        else
        {
            printf("\033[0;33m[Tour %d]\033[0m \033[0;32mScore nul, l'equipe %s gagne grâce aux penalties\033[0m\n", tour, e2->nom);
            return e2;
        }
    }
}



void toggle(int* state) {
  *state ^= 1;
}


void print_val_of_sem_tour(sem_t *sem,char* nom_sem)
{
    int value,i;
    printf("%s[",nom_sem);
    for (i=0;i<nbr_tours;i++) {
        sem_getvalue(&sem[i], &value);
        printf("%d ", value);
    }
     printf("]\n");
}


int *init_interrupteurs(int nbr_tours) {
    int* interrupteur = malloc(nbr_tours * sizeof(int));
    for(int i=0;i<nbr_tours;i++) {
        interrupteur[i]=0;
    }
    return interrupteur;
}




/**
 * @brief simule un tour du tournoi
 *
 * @param arg numéro du tour
 * @return void*
 */
void *simuler_tour(void *arg)
{
    int tour = (int)(intptr_t)arg;
     
    int nbr_match = (team_count / (tour+1))/2;
    int value;
    
    match *m = malloc(nbr_match * sizeof(match));
    pthread_t *threads_match = malloc(nbr_match * sizeof(pthread_t));
    // Lancer les matchs parallélement
    
    for (int i = 0; i < nbr_match; i++)
    {
        m[i].num_match = i;
        m[i].num_tour = tour;
         printf("==============je suis tour %d , m.tour %d , m.match %d\n",tour,m[i].num_tour, m[i].num_match);
        if (pthread_create(&threads_match[i], NULL, thread_function_match, &m[i]) != 0)
        {
            perror("Erreur lors de creation thread tour");
            return (void *)1; // Erreur creation thread tournoi
        }
        
        // puts("");
    }
    
    Equipe eg = (Equipe)malloc(nbr_match * sizeof(struct equipe));
    int status;
    for (int i = 0; i < nbr_match; i++)
    {
        if (pthread_join(threads_match[i], (void**)&status) != 0)
        {
            perror("Erreur lors de join thread tour");
            return (void *)1; // Erreur lors du join des threads tour
        }
        printf("j'ai fini tour %d thread_match %d with status %d\n",tour,  i, status);
        
    }

    free(threads_match);

    printf("j'ai fini simuler tour %d\n",tour);
    return NULL;
}





void *thread_function_match(void *arg)
{

    match m = *(match*)arg;
    Equipe eg = malloc(sizeof(struct equipe));
   
    int tour = m.num_tour;
    int num_match = m.num_match;
    sem_wait(&equipe1[tour]);
    print_val_of_sem_tour(equipe1,"sem_equipe1");
    printf("tour %d match %d passed equipe1\n",tour,num_match);
    sem_wait(&equipe2[tour]);
    print_val_of_sem_tour(equipe2,"sem_equipe2");
    printf("tour %d match %d passed equipe2\n",tour,num_match);

    pthread_mutex_lock(&my_mutex);
    // debut_SC
        Equipe e1 = pop_equipe_at_tour(&mon_tournoi, tour);
        Equipe e2 = pop_equipe_at_tour(&mon_tournoi, tour);
    // fin_SC
    pthread_mutex_unlock(&my_mutex);

    /* Simuler le match */
     eg = simuler_match(e1, e2, tour);
    //usleep(rand()%100000);
    printf("---- fin tour %d match %d\n",tour,num_match);
    pthread_mutex_lock(&my_mutex);
        if (tour<nbr_tours) {
            printf("__tour %d < nbr_tour %d\n",tour,nbr_tours);
            toggle(&interrupteur[tour]);
            if (interrupteur[tour]){
                
                sem_post(&equipe1[tour+1]);
            } else {
                sem_post(&equipe2[tour+1]);
            }
             inserer_equipe_tournoi(mon_tournoi, tour + 1, eg);
             afficher_equipe_tournoi(mon_tournoi);
        }
        
    pthread_mutex_unlock(&my_mutex);

   
    /* Retourner l'equipe gagnante */
     
    return NULL;
}





int main(int argc, char *argv[])
{
    char *filename;
    int i;
    int fin_tournoi = 0;
    int tour = 0;
    Equipe mes_equipes;
    team_count = 0;

    srand(time(NULL)); // initialiser generateur aleatoire

    // Récupération du nom de fichier à partir des arguments de la ligne de commande (ou utilisation par défaut)
    if (argc < 2)
    {
        puts("Entrez le nombre d'equipe");
        int nbr_equipe = 0;
        scanf("%d", &nbr_equipe);
        if (!is_power_two(nbr_equipe))
        {
            nbr_equipe = nearest_power_two(nbr_equipe);
            printf("Le nombre n'est pas une puissance de 2 et a été moifié, nouvelle valeur : %d", nbr_equipe);
        }
        puts("Generation des equipes...");
        char name[4];
        for (i = 0; i < nbr_equipe; i++)
        {
            strcpy(name, generer_nom_equipe());
            inserer_queue(&mes_equipes, name, i + 1);
            team_count++;
        }
    }
    else
    {
        filename = argv[1];
        // Lecture des équipes à partir du fichier
        read_teams(filename, &mes_equipes, &team_count);

        // Garder qu'un nombre puissance de 2 des equipes
        keep_power_of_two(&mes_equipes, &team_count);
    }

    // Affichage des équipes lues à partir du fichier

    printf("Nombre d'equipe %d\n", team_count);

    nbr_tours = (int)log2((double)team_count);

    mon_tournoi = nouveau_tournoi(nbr_tours);

    start_tournoi(&mon_tournoi, mes_equipes);
    afficher_equipe_tournoi(mon_tournoi);
    // Création d'interrupteurs pour usage avec les semaphores
    interrupteur = init_interrupteurs(nbr_tours);

    /* Tableau de semaphores pour le nombre d'equipes Home(1) et Away(2) pour chaque tour */
    equipe1 = malloc(nbr_tours * sizeof(sem_t));
    equipe2 = malloc(nbr_tours * sizeof(sem_t));

    /* Initialisation des semaphores */
        if (sem_init(&equipe1[0], 0, team_count/2) == -1)
    {
        perror("Erreur initialisation semaphore");
        return 3; // Erreur création semaphore
    }

        if (sem_init(&equipe2[0], 0, team_count/2) == -1)
    {
        perror("Erreur initialisation semaphore");
        return 3; // Erreur création semaphore
    }

    for (i = 1; i < nbr_tours; i++)
    {
        if (sem_init(&equipe1[i], 0, 0) == -1)
        {
            perror("Erreur initialisation semaphore");
            return 3; // Erreur création semaphore
        }

        if (sem_init(&equipe2[i], 0, 0) == -1)
        {
            perror("Erreur initialisation semaphore");
            return 3; // Erreur création semaphore
        }
        
    }

    print_val_of_sem_tour(equipe1,"sem_equipe1");
    

    
    pthread_t *threads_tour = malloc(nbr_tours * sizeof(pthread_t));


    // Lancer les tours parallélement
    for (i = 0; i < nbr_tours; i++)
    {
        if (pthread_create(&threads_tour[i], NULL, simuler_tour, (void *)(intptr_t)i) != 0)
        {
            perror("Erreur lors de creation thread tour");
            return 1; // Erreur creation thread tournoi
        };
    }

    int  status;
    for (int i = 0; i < nbr_tours; i++)
    {
        
        if (pthread_join(threads_tour[i], (void**)&status) != 0)
        {
            perror("Erreur lors de join thread tour");
            return 1; // Erreur lors du join des threads tour
        }
        printf("Thread_tour %d exit status %d\n",i,status);
        
    }

    

    // Libération mémoire

    free(threads_tour);

    for (i = 0; i < nbr_tours; i++)
    {
        sem_destroy(&equipe1[i]);
        sem_destroy(&equipe2[i]);
    }
    pthread_mutex_destroy(&my_mutex);

     printf("j'ai fini main\n");
    return 0;
}