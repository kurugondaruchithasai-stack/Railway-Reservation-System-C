#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <ctype.h>

#define MAX_TRAINS 20
#define TOTAL_SEATS 60
#define MAX_NAME 50
#define MAX_TEXT 50
#define MAX_DATE 12

#define BOOKING_FILE "bookings.txt"
#define TRAIN_FILE "trains.txt"
#define WAIT_FILE "waitlist.txt"

typedef struct Passenger
{
    char name[MAX_NAME];
    char gen[10];
    int age;

    int pnr;
    int seat;
    int trainNo;

    char cla[20];
    double fare;

    char source[MAX_TEXT];
    char destination[MAX_TEXT];
    char travelDate[MAX_DATE];

    struct Passenger *link;
} Passenger;

typedef struct
{
    int id;
    char name[50];
    char station[50];
    int hour;
    int minute;
    double acFare;
    double sleeperFare;
    int active;
} Train;

typedef struct WaitNode
{
    char name[MAX_NAME];
    char gen[10];
    int age;

    int pnr;
    int trainNo;

    char cla[20];
    double fare;

    char source[MAX_TEXT];
    char destination[MAX_TEXT];
    char travelDate[MAX_DATE];

    struct WaitNode *link;
} WaitNode;

Passenger *start = NULL;
WaitNode *waitStart = NULL;
Train trains[MAX_TRAINS];
int trainCount = 0;
int cancelledCount = 0;

/* ---------------- INPUT FUNCTIONS ---------------- */
void trim_newline(char *s);
void read_string(const char *prompt, char *s, int size);
int read_int(const char *prompt);
double read_double(const char *prompt);
int valid_date(const char *date);
void read_date(char *date, int size);

/* ---------------- TRAIN FUNCTIONS ---------------- */
void initialize_trains(void);
void load_trains(void);
void save_trains(void);
void display_trains(void);
void add_train(void);
void remove_train(void);
void train_management(void);

/* ---------------- BOOKING FUNCTIONS ---------------- */
void add_node(Passenger *p);
int pnr_exists(int pnr);
int generate_pnr(void);
int allocate_seat(int trainNo, const char *travelDate);
int seat_is_booked(int trainNo, int seat, const char *travelDate);
void collect_passenger(Passenger *p, int pnr, int trainNo,
                       const char *className, double fare,
                       const char *source, const char *destination,
                       const char *travelDate);
void book_ticket(void);
void display_ticket(Passenger *p);
void search_ticket(void);
void cancel_ticket(void);
void modify_passenger(void);
void display_all_bookings(void);
void reports(void);

/* ---------------- WAITING LIST ---------------- */
void add_waiting(Passenger *p);
void display_waiting(void);
void process_waiting_for_train(int trainNo);

/* ---------------- FILE HANDLING ---------------- */
void load_bookings(void);
void save_bookings(void);
void load_waitlist(void);
void save_waitlist(void);

/* ---------------- ADMIN ---------------- */
int admin_login(void);
void admin_dashboard(void);
void admin_menu(void);

/* ---------------- PASSENGER MENU ---------------- */
void user_menu(void);

/* ---------------- MEMORY ---------------- */
void free_bookings(void);
void free_waiting(void);

/* =========================================================
                        MAIN FUNCTION
   ========================================================= */
int main(void)
{
    int choice;

    srand((unsigned int)time(NULL));

    initialize_trains();
    load_trains();
    load_bookings();
    load_waitlist();

    printf("\n===============================================\n");
    printf("       RAILWAY RESERVATION SYSTEM\n");
    printf("===============================================\n");

    while (1)
    {
        printf("\n1. Passenger Menu\n");
        printf("2. Admin Menu\n");
        printf("3. Exit\n");

        choice = read_int("Enter your choice: ");

        if (choice == 1)
            user_menu();
        else if (choice == 2)
            admin_menu();
        else if (choice == 3)
            break;
        else
            printf("Invalid choice.\n");
    }

    save_bookings();
    save_waitlist();
    save_trains();

    free_bookings();
    free_waiting();

    printf("\nThank you for using Railway Reservation System.\n");
    return 0;
}

/* =========================================================
                     INPUT FUNCTIONS
   ========================================================= */
void trim_newline(char *s)
{
    size_t len = strlen(s);

    if (len > 0 && s[len - 1] == '\n')
        s[len - 1] = '\0';
}

void read_string(const char *prompt, char *s, int size)
{
    int ch;
    int truncated;

    do
    {
        printf("%s", prompt);

        if (fgets(s, size, stdin) == NULL)
        {
            s[0] = '\0';
            return;
        }

        truncated = (strchr(s, '\n') == NULL);

        if (truncated)
        {
            while ((ch = getchar()) != '\n' && ch != EOF)
                ;
        }

        trim_newline(s);

        if (strlen(s) == 0)
            printf("Input cannot be empty.\n");

    } while (strlen(s) == 0);
}

int read_int(const char *prompt)
{
    char buffer[100];
    char *end;
    long value;

    while (1)
    {
        printf("%s", prompt);

        if (fgets(buffer, sizeof(buffer), stdin) == NULL)
            return 0;

        value = strtol(buffer, &end, 10);

        if (end != buffer)
        {
            while (isspace((unsigned char)*end))
                end++;

            if (*end == '\0')
                return (int)value;
        }

        printf("Please enter a valid integer.\n");
    }
}

double read_double(const char *prompt)
{
    char buffer[100];
    char *end;
    double value;

    while (1)
    {
        printf("%s", prompt);

        if (fgets(buffer, sizeof(buffer), stdin) == NULL)
            return 0.0;

        value = strtod(buffer, &end);

        if (end != buffer)
        {
            while (isspace((unsigned char)*end))
                end++;

            if (*end == '\0')
                return value;
        }

        printf("Please enter a valid number.\n");
    }
}

int valid_date(const char *date)
{
    int day, month, year;
    int daysInMonth[] = {0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    int leap;
    int i;

    if (strlen(date) != 10)
        return 0;

    if (date[2] != '-' || date[5] != '-')
        return 0;

    for (i = 0; i < 10; i++)
    {
        if (i == 2 || i == 5)
            continue;

        if (!isdigit((unsigned char)date[i]))
            return 0;
    }

    if (sscanf(date, "%2d-%2d-%4d", &day, &month, &year) != 3)
        return 0;

    if (year < 2026 || year > 2100 || month < 1 || month > 12)
        return 0;

    leap = (year % 400 == 0) || (year % 4 == 0 && year % 100 != 0);
    if (leap)
        daysInMonth[2] = 29;

    return day >= 1 && day <= daysInMonth[month];
}

void read_date(char *date, int size)
{
    while (1)
    {
        read_string("Travel date (DD-MM-YYYY): ", date, size);

        if (valid_date(date))
            return;

        printf("Invalid date. Use DD-MM-YYYY, for example 12-10-2026.\n");
    }
}

/* =========================================================
                       TRAIN FUNCTIONS
   ========================================================= */
void initialize_trains(void)
{
    trainCount = 5;

    trains[0] = (Train){1, "Rajdhani Express", "Sealdah Station", 10, 0, 2099, 1560, 1};
    trains[1] = (Train){2, "Satabdi Express", "Howrah Station", 5, 0, 1801, 981, 1};
    trains[2] = (Train){3, "Humsafar Express", "Kolkata Chitpur Station", 11, 0, 2199, 1780, 1};
    trains[3] = (Train){4, "Garib-Rath Express", "Sealdah Station", 5, 0, 1759, 1200, 1};
    trains[4] = (Train){5, "Duronto Express", "Santraganchi Station", 7, 0, 2205, 1905, 1};
}

void load_trains(void)
{
    FILE *fp = fopen(TRAIN_FILE, "r");

    if (!fp)
        return;

    trainCount = 0;

    while (trainCount < MAX_TRAINS &&
           fscanf(fp, "%d|%49[^|]|%49[^|]|%d|%d|%lf|%lf|%d\n",
                  &trains[trainCount].id,
                  trains[trainCount].name,
                  trains[trainCount].station,
                  &trains[trainCount].hour,
                  &trains[trainCount].minute,
                  &trains[trainCount].acFare,
                  &trains[trainCount].sleeperFare,
                  &trains[trainCount].active) == 8)
    {
        trainCount++;
    }

    fclose(fp);

    if (trainCount == 0)
        initialize_trains();
}

void save_trains(void)
{
    FILE *fp = fopen(TRAIN_FILE, "w");
    int i;

    if (!fp)
        return;

    for (i = 0; i < trainCount; i++)
    {
        fprintf(fp, "%d|%s|%s|%d|%d|%.2f|%.2f|%d\n",
                trains[i].id,
                trains[i].name,
                trains[i].station,
                trains[i].hour,
                trains[i].minute,
                trains[i].acFare,
                trains[i].sleeperFare,
                trains[i].active);
    }

    fclose(fp);
}

void display_trains(void)
{
    int i;

    printf("\n================ TRAIN LIST ================\n");

    for (i = 0; i < trainCount; i++)
    {
        if (!trains[i].active)
            continue;

        printf("%d. %-22s | %02d:%02d | %-25s | AC: %.2f | Sleeper: %.2f\n",
               trains[i].id,
               trains[i].name,
               trains[i].hour,
               trains[i].minute,
               trains[i].station,
               trains[i].acFare,
               trains[i].sleeperFare);
    }
}

void add_train(void)
{
    Train *t;

    if (trainCount >= MAX_TRAINS)
    {
        printf("Maximum train limit reached.\n");
        return;
    }

    t = &trains[trainCount];
    t->id = trainCount + 1;

    read_string("Train name: ", t->name, sizeof(t->name));
    read_string("Station: ", t->station, sizeof(t->station));

    do
    {
        t->hour = read_int("Departure hour (0-23): ");
    } while (t->hour < 0 || t->hour > 23);

    do
    {
        t->minute = read_int("Departure minute (0-59): ");
    } while (t->minute < 0 || t->minute > 59);

    do
    {
        t->acFare = read_double("AC fare: ");
    } while (t->acFare <= 0);

    do
    {
        t->sleeperFare = read_double("Sleeper fare: ");
    } while (t->sleeperFare <= 0);

    t->active = 1;
    trainCount++;

    save_trains();
    printf("Train added successfully with Train No. %d.\n", t->id);
}

void remove_train(void)
{
    int id = read_int("Enter train number to deactivate: ");

    if (id < 1 || id > trainCount)
    {
        printf("Invalid train number.\n");
        return;
    }

    trains[id - 1].active = 0;
    save_trains();

    printf("Train deactivated. Existing booking records are preserved.\n");
}

void train_management(void)
{
    int choice;

    while (1)
    {
        printf("\n=========== TRAIN MANAGEMENT ===========\n");
        printf("1. Display trains\n");
        printf("2. Add train\n");
        printf("3. Deactivate train\n");
        printf("4. Back\n");

        choice = read_int("Choice: ");

        if (choice == 1)
            display_trains();
        else if (choice == 2)
            add_train();
        else if (choice == 3)
            remove_train();
        else if (choice == 4)
            return;
        else
            printf("Invalid choice.\n");
    }
}

/* =========================================================
                    LINKED LIST FUNCTIONS
   ========================================================= */
void add_node(Passenger *p)
{
    Passenger *newNode = malloc(sizeof(Passenger));
    Passenger *ptr;

    if (!newNode)
    {
        printf("Memory allocation failed.\n");
        exit(EXIT_FAILURE);
    }

    *newNode = *p;
    newNode->link = NULL;

    if (start == NULL)
        start = newNode;
    else
    {
        ptr = start;
        while (ptr->link != NULL)
            ptr = ptr->link;
        ptr->link = newNode;
    }
}

/* =========================================================
                       PNR FUNCTIONS
   ========================================================= */
int pnr_exists(int pnr)
{
    Passenger *p = start;
    WaitNode *w = waitStart;

    while (p != NULL)
    {
        if (p->pnr == pnr)
            return 1;
        p = p->link;
    }

    while (w != NULL)
    {
        if (w->pnr == pnr)
            return 1;
        w = w->link;
    }

    return 0;
}

int generate_pnr(void)
{
    int pnr;

    do
    {
        pnr = 100000 + rand() % 900000;
    } while (pnr_exists(pnr));

    return pnr;
}

/* =========================================================
                       SEAT FUNCTIONS
   ========================================================= */
int seat_is_booked(int trainNo, int seat, const char *travelDate)
{
    Passenger *p = start;

    while (p != NULL)
    {
        if (p->trainNo == trainNo &&
            p->seat == seat &&
            strcmp(p->travelDate, travelDate) == 0)
        {
            return 1;
        }
        p = p->link;
    }

    return 0;
}

int allocate_seat(int trainNo, const char *travelDate)
{
    int seat;

    if (trainNo < 1 || trainNo > trainCount || !trains[trainNo - 1].active)
        return -1;

    for (seat = 1; seat <= TOTAL_SEATS; seat++)
    {
        if (!seat_is_booked(trainNo, seat, travelDate))
            return seat;
    }

    return -1;
}

/* =========================================================
                    FILE HANDLING
   ========================================================= */
void load_bookings(void)
{
    FILE *fp = fopen(BOOKING_FILE, "r");
    char line[500];
    Passenger p;
    char classWithTrain[20];
    char *separator;
    int fields;

    if (!fp)
        return;

    while (fgets(line, sizeof(line), fp) != NULL)
    {
        memset(&p, 0, sizeof(p));
        strcpy(p.travelDate, "N/A");

        fields = sscanf(line,
            "%d|%d|%49[^|]|%9[^|]|%d|%19[^|]|%lf|%49[^|]|%49[^|]|%10[^\n]",
            &p.pnr, &p.seat, p.name, p.gen, &p.age, classWithTrain,
            &p.fare, p.source, p.destination, p.travelDate);

        if (fields == 9)
        {
            fields = sscanf(line,
                "%d|%d|%49[^|]|%9[^|]|%d|%19[^|]|%lf|%49[^|]|%49[^\n]",
                &p.pnr, &p.seat, p.name, p.gen, &p.age, classWithTrain,
                &p.fare, p.source, p.destination);
        }

        if (fields < 9)
            continue;

        strcpy(p.cla, classWithTrain);
        separator = strrchr(p.cla, '~');

        if (separator)
        {
            p.trainNo = atoi(separator + 1);
            *separator = '\0';
        }
        else
            p.trainNo = 1;

        p.link = NULL;

        add_node(&p);
    }

    fclose(fp);
}

void save_bookings(void)
{
    FILE *fp = fopen(BOOKING_FILE, "w");
    Passenger *p = start;

    if (!fp)
        return;

    while (p != NULL)
    {
        fprintf(fp,
            "%d|%d|%s|%s|%d|%s~%d|%.2f|%s|%s|%s\n",
            p->pnr,
            p->seat,
            p->name,
            p->gen,
            p->age,
            p->cla,
            p->trainNo,
            p->fare,
            p->source,
            p->destination,
            p->travelDate);

        p = p->link;
    }

    fclose(fp);
}

void add_waiting(Passenger *p)
{
    WaitNode *newNode = malloc(sizeof(WaitNode));
    WaitNode *ptr;

    if (!newNode)
    {
        printf("Memory allocation failed.\n");
        return;
    }

    strcpy(newNode->name, p->name);
    strcpy(newNode->gen, p->gen);
    newNode->age = p->age;
    newNode->pnr = p->pnr;
    newNode->trainNo = p->trainNo;
    strcpy(newNode->cla, p->cla);
    newNode->fare = p->fare;
    strcpy(newNode->source, p->source);
    strcpy(newNode->destination, p->destination);
    strcpy(newNode->travelDate, p->travelDate);
    newNode->link = NULL;

    if (waitStart == NULL)
        waitStart = newNode;
    else
    {
        ptr = waitStart;
        while (ptr->link != NULL)
            ptr = ptr->link;
        ptr->link = newNode;
    }
}

void save_waitlist(void)
{
    FILE *fp = fopen(WAIT_FILE, "w");
    WaitNode *w = waitStart;

    if (!fp)
        return;

    while (w != NULL)
    {
        fprintf(fp,
            "%d|%s|%s|%d|%d|%s|%.2f|%s|%s|%s\n",
            w->pnr,
            w->name,
            w->gen,
            w->age,
            w->trainNo,
            w->cla,
            w->fare,
            w->source,
            w->destination,
            w->travelDate);

        w = w->link;
    }

    fclose(fp);
}

void load_waitlist(void)
{
    FILE *fp = fopen(WAIT_FILE, "r");
    char line[500];
    WaitNode w;
    WaitNode *newNode;
    WaitNode *ptr;
    int fields;

    if (!fp)
        return;

    while (fgets(line, sizeof(line), fp) != NULL)
    {
        memset(&w, 0, sizeof(w));
        strcpy(w.travelDate, "N/A");

        fields = sscanf(line,
            "%d|%49[^|]|%9[^|]|%d|%d|%19[^|]|%lf|%49[^|]|%49[^|]|%10[^\n]",
            &w.pnr, w.name, w.gen, &w.age, &w.trainNo, w.cla,
            &w.fare, w.source, w.destination, w.travelDate);

        if (fields == 9)
        {
            fields = sscanf(line,
                "%d|%49[^|]|%9[^|]|%d|%d|%19[^|]|%lf|%49[^|]|%49[^\n]",
                &w.pnr, w.name, w.gen, &w.age, &w.trainNo, w.cla,
                &w.fare, w.source, w.destination);
        }

        if (fields < 9)
            continue;

        newNode = malloc(sizeof(WaitNode));
        if (!newNode)
            break;

        *newNode = w;
        newNode->link = NULL;

        if (waitStart == NULL)
            waitStart = newNode;
        else
        {
            ptr = waitStart;
            while (ptr->link != NULL)
                ptr = ptr->link;
            ptr->link = newNode;
        }
    }

    fclose(fp);
}

/* =========================================================
                  PASSENGER DATA INPUT
   ========================================================= */
void collect_passenger(Passenger *p, int pnr, int trainNo,
                       const char *className, double fare,
                       const char *source, const char *destination,
                       const char *travelDate)
{
    p->pnr = pnr;
    p->trainNo = trainNo;
    p->fare = fare;

    strcpy(p->cla, className);
    strcpy(p->source, source);
    strcpy(p->destination, destination);
    strcpy(p->travelDate, travelDate);

    read_string("Passenger name: ", p->name, sizeof(p->name));
    read_string("Gender: ", p->gen, sizeof(p->gen));

    do
    {
        p->age = read_int("Age: ");

        if (p->age < 1 || p->age > 120)
            printf("Age must be between 1 and 120.\n");

    } while (p->age < 1 || p->age > 120);
}

/* =========================================================
                       BOOK TICKET
   ========================================================= */
void book_ticket(void)
{
    int count;
    int i;
    int trainNo;
    int classChoice;
    int pnr;
    int seat;

    char source[MAX_TEXT];
    char destination[MAX_TEXT];
    char travelDate[MAX_DATE];
    char className[20];

    double baseFare;
    double totalFare;
    Passenger p;

    count = read_int("Enter number of passengers: ");

    if (count < 1 || count > TOTAL_SEATS)
    {
        printf("Invalid number of passengers. Maximum is %d.\n", TOTAL_SEATS);
        return;
    }

    read_string("Enter source: ", source, sizeof(source));
    read_string("Enter destination: ", destination, sizeof(destination));
    read_date(travelDate, sizeof(travelDate));

    display_trains();

    trainNo = read_int("Select train number: ");

    if (trainNo < 1 || trainNo > trainCount || !trains[trainNo - 1].active)
    {
        printf("Invalid train. Please select a number shown in the train list.\n");
        return;
    }

    printf("\n1. AC\n");
    printf("2. Sleeper\n");

    classChoice = read_int("Select class: ");

    if (classChoice == 1)
    {
        strcpy(className, "AC");
        baseFare = trains[trainNo - 1].acFare;
    }
    else if (classChoice == 2)
    {
        strcpy(className, "Sleeper");
        baseFare = trains[trainNo - 1].sleeperFare;
    }
    else
    {
        printf("Invalid class.\n");
        return;
    }

    pnr = generate_pnr();
    totalFare = baseFare * 1.18;

    printf("\nGenerated PNR: %d\n", pnr);

    for (i = 0; i < count; i++)
    {
        seat = allocate_seat(trainNo, travelDate);
        memset(&p, 0, sizeof(p));

        collect_passenger(&p, pnr, trainNo, className, totalFare,
                          source, destination, travelDate);

        if (seat == -1)
        {
            p.seat = 0;
            add_waiting(&p);
            printf("Passenger %d added to waiting list.\n", i + 1);
        }
        else
        {
            p.seat = seat;
            add_node(&p);
            printf("Passenger %d booked successfully. Seat = %d\n", i + 1, seat);
        }
    }

    save_bookings();
    save_waitlist();

    printf("Booking completed. PNR: %d\n", pnr);
}

/* =========================================================
                       DISPLAY TICKET
   ========================================================= */
void display_ticket(Passenger *p)
{
    Train *t;

    if (p->trainNo < 1 || p->trainNo > trainCount)
        return;

    t = &trains[p->trainNo - 1];

    printf("\n--------------- TICKET ---------------\n");
    printf("PNR          : %d\n", p->pnr);
    printf("Passenger    : %s\n", p->name);
    printf("Gender       : %s\n", p->gen);
    printf("Age          : %d\n", p->age);
    printf("Train        : %s\n", t->name);
    printf("Train No.    : %d\n", p->trainNo);
    printf("Station      : %s\n", t->station);
    printf("Time         : %02d:%02d\n", t->hour, t->minute);
    printf("From         : %s\n", p->source);
    printf("To           : %s\n", p->destination);
    printf("Travel Date  : %s\n", p->travelDate);
    printf("Class        : %s\n", p->cla);
    printf("Seat         : %d\n", p->seat);
    printf("Fare + GST   : %.2f\n", p->fare);
    printf("---------------------------------------\n");
}

/* =========================================================
                    SEARCH BY PNR
   ========================================================= */
void search_ticket(void)
{
    int pnr = read_int("Enter PNR: ");
    Passenger *p = start;
    int found = 0;

    while (p != NULL)
    {
        if (p->pnr == pnr)
        {
            display_ticket(p);
            found = 1;
        }
        p = p->link;
    }

    if (!found)
        printf("No confirmed ticket found for PNR %d.\n", pnr);
}

/* =========================================================
                    WAITING LIST
   ========================================================= */
void display_waiting(void)
{
    WaitNode *w = waitStart;
    int position = 1;

    if (!w)
    {
        printf("Waiting list is empty.\n");
        return;
    }

    while (w != NULL)
    {
        printf("%d. PNR: %d | %s | Train: %d | Class: %s | Date: %s\n",
               position++, w->pnr, w->name, w->trainNo,
               w->cla, w->travelDate);
        w = w->link;
    }
}

void process_waiting_for_train(int trainNo)
{
    WaitNode *w = waitStart;
    WaitNode *prev = NULL;
    int seat;
    Passenger p;

    while (w != NULL)
    {
        if (w->trainNo == trainNo)
        {
            seat = allocate_seat(trainNo, w->travelDate);

            if (seat == -1)
                return;

            memset(&p, 0, sizeof(p));
            strcpy(p.name, w->name);
            strcpy(p.gen, w->gen);
            p.age = w->age;
            p.pnr = w->pnr;
            p.seat = seat;
            p.trainNo = w->trainNo;
            strcpy(p.cla, w->cla);
            p.fare = w->fare;
            strcpy(p.source, w->source);
            strcpy(p.destination, w->destination);
            strcpy(p.travelDate, w->travelDate);

            add_node(&p);

            printf("Waiting-list passenger %s got Seat %d.\n", p.name, seat);

            if (prev == NULL)
                waitStart = w->link;
            else
                prev->link = w->link;

            free(w);
            return;
        }

        prev = w;
        w = w->link;
    }
}

/* =========================================================
                     CANCEL TICKET
   ========================================================= */
void cancel_ticket(void)
{
    int pnr = read_int("Enter PNR: ");
    int seat = read_int("Enter seat number to cancel: ");
    Passenger *p = start;
    Passenger *prev = NULL;
    int trainNo;

    while (p != NULL && !(p->pnr == pnr && p->seat == seat))
    {
        prev = p;
        p = p->link;
    }

    if (!p)
    {
        printf("Ticket not found.\n");
        return;
    }

    trainNo = p->trainNo;
    if (prev == NULL)
        start = p->link;
    else
        prev->link = p->link;

    free(p);
    cancelledCount++;

    printf("Ticket cancelled successfully.\n");

    process_waiting_for_train(trainNo);

    save_bookings();
    save_waitlist();
}

/* =========================================================
                  MODIFY PASSENGER DETAILS
   ========================================================= */
Passenger *find_passenger_by_pnr_seat(int pnr, int seat)
{
    Passenger *p = start;

    while (p != NULL)
    {
        if (p->pnr == pnr && p->seat == seat)
            return p;

        p = p->link;
    }

    return NULL;
}

void modify_passenger(void)
{
    int pnr = read_int("Enter PNR: ");
    int seat = read_int("Enter current seat number: ");
    int newAge;
    Passenger *p = find_passenger_by_pnr_seat(pnr, seat);
    char newName[MAX_NAME];
    char newGender[10];

    if (!p)
    {
        printf("Passenger not found.\n");
        return;
    }

    read_string("Enter new name: ", newName, sizeof(newName));
    read_string("Enter new gender: ", newGender, sizeof(newGender));

    do
    {
        newAge = read_int("Enter new age: ");
    } while (newAge < 1 || newAge > 120);

    strcpy(p->name, newName);
    strcpy(p->gen, newGender);
    p->age = newAge;

    save_bookings();
    printf("Passenger details updated successfully.\n");
}

/* =========================================================
                   DISPLAY ALL BOOKINGS
   ========================================================= */
void display_all_bookings(void)
{
    Passenger *p = start;

    if (!p)
    {
        printf("No confirmed bookings.\n");
        return;
    }

    while (p != NULL)
    {
        display_ticket(p);
        p = p->link;
    }
}

/* =========================================================
                         REPORTS
   ========================================================= */
void reports(void)
{
    Passenger *p = start;
    WaitNode *w = waitStart;
    int count = 0;
    int waiting = 0;
    int i;
    int trainBooked;
    double revenue = 0.0;

    while (p != NULL)
    {
        count++;
        revenue += p->fare;
        p = p->link;
    }

    while (w != NULL)
    {
        waiting++;
        w = w->link;
    }

    printf("\n=============== REPORT ===============\n");
    printf("Confirmed passengers : %d\n", count);
    printf("Waiting passengers   : %d\n", waiting);
    printf("Cancelled tickets    : %d\n", cancelledCount);
    printf("Total revenue        : %.2f\n", revenue);

    printf("\nTrain-wise confirmed bookings (all travel dates):\n");
    for (i = 0; i < trainCount; i++)
    {
        trainBooked = 0;
        p = start;

        while (p != NULL)
        {
            if (p->trainNo == trains[i].id)
                trainBooked++;
            p = p->link;
        }

        printf("Train %d (%s): %d booking(s) | %s\n",
               trains[i].id, trains[i].name, trainBooked,
               trains[i].active ? "Active" : "Inactive");
    }
}

/* =========================================================
                    ADMIN LOGIN
   ========================================================= */
int admin_login(void)
{
    char username[30];
    char password[30];

    read_string("Admin username: ", username, sizeof(username));
    read_string("Admin password: ", password, sizeof(password));

    if (strcmp(username, "admin") == 0 &&
        strcmp(password, "railway123") == 0)
    {
        return 1;
    }

    printf("Invalid admin credentials.\n");
    return 0;
}

/* =========================================================
                    ADMIN DASHBOARD
   ========================================================= */
void admin_dashboard(void)
{
    Passenger *p = start;
    WaitNode *w = waitStart;
    int confirmed = 0;
    int waiting = 0;
    int activeTrains = 0;
    int i;
    int trainBooked;
    double revenue = 0.0;

    while (p != NULL)
    {
        confirmed++;
        revenue += p->fare;
        p = p->link;
    }

    while (w != NULL)
    {
        waiting++;
        w = w->link;
    }

    for (i = 0; i < trainCount; i++)
    {
        if (trains[i].active)
            activeTrains++;
    }

    printf("\n================================================\n");
    printf("                 ADMIN DASHBOARD\n");
    printf("================================================\n");
    printf("Confirmed bookings : %d\n", confirmed);
    printf("Waiting passengers : %d\n", waiting);
    printf("Cancelled this run : %d\n", cancelledCount);
    printf("Active trains      : %d\n", activeTrains);
    printf("Current revenue    : %.2f\n", revenue);
    printf("================================================\n");

    printf("\nTrain-wise occupancy (all travel dates):\n");

    for (i = 0; i < trainCount; i++)
    {
        trainBooked = 0;
        p = start;

        while (p != NULL)
        {
            if (p->trainNo == trains[i].id)
                trainBooked++;
            p = p->link;
        }

        printf("Train %d - %-22s : %d booking(s) | %s\n",
               trains[i].id, trains[i].name, trainBooked,
               trains[i].active ? "Active" : "Inactive");
    }
}

/* =========================================================
                       ADMIN MENU
   ========================================================= */
void admin_menu(void)
{
    int choice;

    if (!admin_login())
        return;

    while (1)
    {
        printf("\n=============== ADMIN MENU ===============\n");
        printf("1. Admin dashboard\n");
        printf("2. Train management\n");
        printf("3. View all bookings\n");
        printf("4. View waiting list\n");
        printf("5. Reports\n");
        printf("6. Back\n");

        choice = read_int("Choice: ");

        if (choice == 1)
            admin_dashboard();
        else if (choice == 2)
            train_management();
        else if (choice == 3)
            display_all_bookings();
        else if (choice == 4)
            display_waiting();
        else if (choice == 5)
            reports();
        else if (choice == 6)
            return;
        else
            printf("Invalid choice.\n");
    }
}

/* =========================================================
                    PASSENGER MENU
   ========================================================= */
void user_menu(void)
{
    int choice;

    while (1)
    {
        printf("\n============== PASSENGER MENU ==============\n");
        printf("1. Book ticket\n");
        printf("2. Search ticket by PNR\n");
        printf("3. Cancel ticket\n");
        printf("4. Modify passenger details\n");
        printf("5. View waiting list\n");
        printf("6. Back\n");

        choice = read_int("Choice: ");

        if (choice == 1)
            book_ticket();
        else if (choice == 2)
            search_ticket();
        else if (choice == 3)
            cancel_ticket();
        else if (choice == 4)
            modify_passenger();
        else if (choice == 5)
            display_waiting();
        else if (choice == 6)
            return;
        else
            printf("Invalid choice.\n");
    }
}

/* =========================================================
                       FREE MEMORY
   ========================================================= */
void free_bookings(void)
{
    Passenger *p = start;

    while (p != NULL)
    {
        Passenger *next = p->link;
        free(p);
        p = next;
    }

    start = NULL;
}

void free_waiting(void)
{
    WaitNode *w = waitStart;

    while (w != NULL)
    {
        WaitNode *next = w->link;
        free(w);
        w = next;
    }

    waitStart = NULL;
}
