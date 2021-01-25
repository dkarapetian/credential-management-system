/******************************************************************************
Password Management System

Students:   Danny Kalnian - 98059328
            Derek Karapetian - 98050418
            Andre Pearce -
            Shaun Thomas - 97119073
            Georgios Statiris 12403616

credentialmanager.c

Manage credentials
******************************************************************************/
/******************************************************************************
macros
******************************************************************************/
#ifdef _WIN32
    #define CURRENT_OS "windows"
#else
    #define CURRENT_OS "unix"
#endif


/******************************************************************************
Header files
******************************************************************************/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>


/******************************************************************************
Defines
******************************************************************************/
#define RESET "\x1b[0m"
#define RED "\x1b[31m"
#define GREEN "\x1b[32m"
#define GREEN_BOLD "\x1b[32;1m"

#define CHOICE_LENGTH 3
#define NAME_LENGTH 31
#define HASH_CIPHER_LENGTH 257
#define MAX_CREDENTIALS 100
#define URL_LENGTH 201
#define DETAILS_LENGTH 401
#define MAX_ACCOUNTS 20
#define QUESTION_LENGTH 50
#define ANSWER_LENGTH 30
#define MAX_QUESTIONS 3
#define DATE_LENGTH 15
#define DATE_LENGTH_FORMATTED 20
#define DATABASE_NAME "database"

/* TODO define key for cipher and hash */


/******************************************************************************
Typedefs
******************************************************************************/
typedef enum {
    false = (1 == 0),
    true = (!false)
} bool;

/* TODO look into dynamically setting size of strings, not sure how this will
play with our database when using sizeof function though*/

/* TODO define struct for encrypted data type, one property of int or char ? */

typedef struct securityQuestion {
    char question[QUESTION_LENGTH];
    char answer[ANSWER_LENGTH];
} securityQuestion_t;

typedef struct profile {
    char username[NAME_LENGTH];
    char firstname[NAME_LENGTH];
    char lastname[NAME_LENGTH];
    char passwordhash[HASH_CIPHER_LENGTH];
    char dateregistered[DATE_LENGTH];
    char lastloggedin[DATE_LENGTH];
    securityQuestion_t securityQuestions[MAX_QUESTIONS];
} profile_t;

typedef struct credential {
    char name[NAME_LENGTH];
    char username[NAME_LENGTH];
    char encryptedpassword[HASH_CIPHER_LENGTH];
    char url[URL_LENGTH];
    char details[DETAILS_LENGTH];
    char datecreated[DATE_LENGTH];
    char lastmodified[DATE_LENGTH];
} credential_t;

typedef struct account {
    profile_t profile;
    credential_t credentials[MAX_CREDENTIALS];
    unsigned int credentialcount;
} account_t;


/******************************************************************************
Function Prototypes
******************************************************************************/
bool userLoop(int);
void startMenu(char[]);
bool login(void);
void forgotPassword(void);
void signUp(void);
void getProfileValue(char[], const char[], int);
void mainMenu(char[], char[]);
void addCredential(int);
void viewCredentials(int);
void editCredential(int);
void deleteCredential(int);
void exportCredentials(void);
void importCredentials(void);
unsigned int databaseRead(account_t[]);
void databaseWrite(account_t[], unsigned int);
void clearBuffer(void);
void delay(int);
void loadingProgress(int, int);
void getUserInput(char[], int);
void clearLine(void);
int systemCall(size_t, char*[]);
bool isIntInArray(int, int[], int);
void formatStringToDate(char[], char[]);


/******************************************************************************
Main
******************************************************************************/
int main(void) {
    char* args[] = {"clear"};
    systemCall((sizeof(args) / sizeof(args[0])), args);

    int i;
    for (i = 0; i < 5; i++) {
        loadingProgress(i, 3);
    }
    clearLine();

    bool running = true;
    char choice[CHOICE_LENGTH];
    while (running) {
      startMenu(choice);
      if (strlen(choice) == 1) {
          switch (choice[0]) {
              case '1':
                  if (!login()) {
                      printf(RED"Login Failed.\n"RESET);
                  }
                  break;
              case '2':
                  signUp();
                  break;
              case '3':
                  forgotPassword();
                  break;
              case '4':
                  running = false;
                  break;
              default:
                  printf(RED"Invalid selection.\n"RESET);
          }
      } else {
          printf(RED"Invalid selection.\n"RESET);
      }
    }
    if (running) {
        printf(RED"Program quit unexpectedly.\n"RESET);
    }
    return running;
}


/******************************************************************************
userLoop
******************************************************************************/
bool userLoop(int userIndex) {
    char* argsforclear[] = {"clear"};
    systemCall((sizeof(argsforclear) / sizeof(argsforclear[0])), argsforclear);

    account_t accounts[MAX_ACCOUNTS];
    unsigned int numaccounts = databaseRead(accounts);

    char dateFormatted[DATE_LENGTH_FORMATTED];
    formatStringToDate(dateFormatted, accounts[userIndex].profile.lastloggedin);
    printf("Last logged in: %s\n", dateFormatted);

    char date[DATE_LENGTH];
    char* argsfordate[] = {"getdate", date};
    systemCall((sizeof(argsfordate) / sizeof(argsfordate[0])), argsfordate);
    strcpy(accounts[userIndex].profile.lastloggedin, date);
    /* TODO not sure if write at correct time (does order of accounts in array chage?) */
    databaseWrite(accounts, numaccounts);

    bool loggedIn = true;
    char choice[CHOICE_LENGTH];
    while (loggedIn) {
        mainMenu(choice, accounts[userIndex].profile.firstname);
        if (strlen(choice) == 1) {
            switch (choice[0]) {
                case '1':
                    addCredential(userIndex);
                    break;
                case '2':
                    editCredential(userIndex);
                    break;
                case '3':
                    viewCredentials(userIndex);
                    break;
                case '4':
                    deleteCredential(userIndex);
                    break;
                case '5':
                    break;
                case '6':
                    break;
                case '7':
                    loggedIn = false;
                    break;
                default:
                    printf(RED"Invalid selection.\n"RESET);
            }
        } else {
            printf(RED"Invalid selection.\n"RESET);
        }
    }

    return false;
}


/******************************************************************************
startMenu
******************************************************************************/
void startMenu (char choice[]) {
    printf("\n\n"
           GREEN_BOLD"Credential Management System\n\n"RESET
           "1. Login\n"
           "2. Register\n"
           "3. Forgot Password\n\n"
           "4. Exit\n"
           "Select an option from above> "
    );

    getUserInput(choice, CHOICE_LENGTH);

    char* args[] = {"clear"};
    systemCall((sizeof(args) / sizeof(args[0])), args);
}


/******************************************************************************
mainMenu
******************************************************************************/
void mainMenu (char choice[], char name[NAME_LENGTH]) {
    char date[DATE_LENGTH];
    char dateFormatted[DATE_LENGTH_FORMATTED];
    char* argsfordate[] = {"getdate", date};
    systemCall((sizeof(argsfordate) / sizeof(argsfordate[0])), argsfordate);
    formatStringToDate(dateFormatted, date);

    printf("\n\n"
           GREEN_BOLD"Welcome %s\n"RESET
           "%s\n\n"
           "1. Add credential\n"
           "2. Edit credentials\n"
           "3. View credentials\n"
           "4. Delete credential\n"
           "5. Export credentials\n"
           "6. Import credentials\n\n"
           "7. Log out\n"
           "Select an option from above> ",
           name,
           dateFormatted
    );

    getUserInput(choice, CHOICE_LENGTH);

    char* argsforclear[] = {"clear"};
    systemCall((sizeof(argsforclear) / sizeof(argsforclear[0])), argsforclear);
}


/******************************************************************************
login
******************************************************************************/
bool login(void) {
    account_t accounts[MAX_ACCOUNTS];
    unsigned int numaccounts = databaseRead(accounts);

    char username[NAME_LENGTH];
    /* TODO change password to be hash not raw */
    char password[HASH_CIPHER_LENGTH];

    printf("Please enter login credentials...\n\n");
    printf("Enter Username: ");
    getUserInput(username, NAME_LENGTH);
    printf ("Enter Password: ");
    getUserInput(password, HASH_CIPHER_LENGTH);

    unsigned int currentAccount;
    for (currentAccount = 0; currentAccount < numaccounts; currentAccount ++) {
        if (!strcmp(accounts[currentAccount].profile.username, username) && !strcmp(accounts[currentAccount].profile.passwordhash, password)) {
            if (userLoop(currentAccount)) {
                printf(RED"Logged out unexpectedly.\n"RESET);
            }
            return true;
        }
    }
    char* args[] = {"clear"};
    systemCall((sizeof(args) / sizeof(args[0])), args);

    return false;
}


/******************************************************************************
forgotPassword
******************************************************************************/
void forgotPassword(void) {
    account_t accounts[MAX_ACCOUNTS];
    unsigned int numaccounts = databaseRead(accounts);

    printf("Please enter credentials...\n\n");
    char username[NAME_LENGTH];
    printf("Username: ");
    getUserInput(username, NAME_LENGTH);

    int accountIndex;
    bool validUser = false;
    for (accountIndex = 0; accountIndex < numaccounts && !validUser; accountIndex++) {
        if (!strcmp(accounts[accountIndex].profile.username, username)) {
            validUser = true;
        }
    }
    accountIndex--;

    if (validUser) {
        char answer[ANSWER_LENGTH];
        int results[MAX_QUESTIONS + 1];

        unsigned int questionCounter;
        for (questionCounter = 0; questionCounter < MAX_QUESTIONS; questionCounter++) {
            printf("%s? ", accounts[accountIndex].profile.securityQuestions[questionCounter].question);
            getUserInput(answer, ANSWER_LENGTH);
            results[questionCounter] = !(strcmp(accounts[accountIndex].profile.securityQuestions[questionCounter].answer, answer));
        }

        if (!(isIntInArray(0, results, MAX_QUESTIONS))) {
            getProfileValue(accounts[accountIndex].profile.passwordhash, "password", HASH_CIPHER_LENGTH);
            databaseWrite(accounts, numaccounts);
            char* args[] = {"clear"};
            systemCall((sizeof(args) / sizeof(args[0])), args);
            printf(GREEN"Password changed.\n"RESET);
        } else {
            char* args[] = {"clear"};
            systemCall((sizeof(args) / sizeof(args[0])), args);
            printf(RED"Go away.\n"RESET);
        }

    } else {
        char* args[] = {"clear"};
        systemCall((sizeof(args) / sizeof(args[0])), args);
        printf(RED"User doesn't exits.\n"RESET);
    }
}


/******************************************************************************
signup
******************************************************************************/
void signUp(void) {
    account_t accounts[MAX_ACCOUNTS];
    unsigned int numaccounts = databaseRead(accounts);
    /* TODO remove this print statement */
    printf("Number of accounts currently stored: %d\n", numaccounts);
    printf("Please enter user information...\n\n");
    if (numaccounts < MAX_ACCOUNTS) {
        profile_t profile;
        bool unique = false;
        while (!unique) {
            getProfileValue(profile.username, "username", NAME_LENGTH);
            if (strlen(profile.username) > 0) {
                unique = true;
                int i;
                for (i = 0; i < numaccounts; i++) {
                    if (!strcmp(profile.username, accounts[i].profile.username)) {
                        unique = false;
                        printf(RED"The entered username is already taken, try another.\n"RESET);
                    }
                }
            } else {
                printf(RED"User name can not be blank.\n"RESET);
            }
        }
        getProfileValue(profile.firstname, "first name", NAME_LENGTH);
        getProfileValue(profile.lastname, "last name", NAME_LENGTH);
        getProfileValue(profile.passwordhash, "password", HASH_CIPHER_LENGTH);

        char date[DATE_LENGTH];
        char* argsfordate[] = {"getdate", date};
        systemCall((sizeof(argsfordate) / sizeof(argsfordate[0])), argsfordate);
        strcpy(profile.dateregistered, date);

        printf("\nPlease enter %d security questions...\n", MAX_QUESTIONS);
        int questionCounter;
        for (questionCounter = 0; questionCounter < MAX_QUESTIONS; questionCounter++) {
            getProfileValue(profile.securityQuestions[questionCounter].question, "Security Question", QUESTION_LENGTH);
            getProfileValue(profile.securityQuestions[questionCounter].answer, "Answer", ANSWER_LENGTH);
            printf("\n");
        }

        account_t account;
        account.profile = profile;
        account.credentialcount = 0;

        accounts[numaccounts] = account;
        numaccounts++;

        databaseWrite(accounts, numaccounts);

        char* argsforclear[] = {"clear"};
        systemCall((sizeof(argsforclear) / sizeof(argsforclear[0])), argsforclear);
        printf(GREEN"Account created.\n"RESET);
    } else {
        char* argsforclear[] = {"clear"};
        systemCall((sizeof(argsforclear) / sizeof(argsforclear[0])), argsforclear);
        printf(RED"Maximum number of accounts created.\n"RESET);
    }
}


/******************************************************************************
addCredential
******************************************************************************/
void addCredential(int user) {
    account_t accounts[MAX_ACCOUNTS];
    unsigned int numaccounts = databaseRead(accounts);

    if (accounts[user].credentialcount < MAX_CREDENTIALS) {
        credential_t newCredential;

        bool unique = false;
        while (!unique) {
            getProfileValue(newCredential.name, "name", NAME_LENGTH);
            if (strlen(newCredential.name) > 0) {
                unique = true;
                int i;
                for (i = 0; i < numaccounts; i++) {
                    if (!strcmp(newCredential.name, accounts[user].credentials[i].name)) {
                        unique = false;
                        printf(RED"The entered credential name already exists, try another.\n"RESET);
                    }
                }
            } else {
                printf(RED"Credential name can not be blank.\n"RESET);
            }
        }

        getProfileValue(newCredential.username, "username", NAME_LENGTH);
        getProfileValue(newCredential.encryptedpassword, "password", HASH_CIPHER_LENGTH);
        getProfileValue(newCredential.url, "url", URL_LENGTH);
        getProfileValue(newCredential.details, "details", DETAILS_LENGTH);

        char date[DATE_LENGTH];
        char* args[] = {"getdate", date};
        systemCall((sizeof(args) / sizeof(args[0])), args);
        strcpy(newCredential.datecreated, date);

        accounts[user].credentials[accounts[user].credentialcount] = newCredential;
        accounts[user].credentialcount = accounts[user].credentialcount + 1;

        databaseWrite(accounts, numaccounts);
        char* argsforclear[] = {"clear"};
        systemCall((sizeof(argsforclear) / sizeof(argsforclear[0])), argsforclear);
        printf(GREEN"Credential added successfully.\n"RESET);


    } else {
        char* argsforclear[] = {"clear"};
        systemCall((sizeof(argsforclear) / sizeof(argsforclear[0])), argsforclear);
        printf(RED"Limit reached.\n"RESET);
    }
}


/******************************************************************************
viewCredential
******************************************************************************/
void viewCredentials(int user) {
    account_t accounts[MAX_ACCOUNTS];
    databaseRead(accounts);

    int credential;
    bool found;
    bool viewing = true;
    char toView[NAME_LENGTH];
    while (viewing) {
        printf("Current credentials...\n\n");
        for (credential = 0; credential < accounts[user].credentialcount; credential++) {
            printf("%s\n", accounts[user].credentials[credential].name);
        }

        printf("\nWhich credential would you like to view? (Blank to return)\nCredential name> ");
        getUserInput(toView, NAME_LENGTH);
        char* argsforclear[] = {"clear"};
        systemCall((sizeof(argsforclear) / sizeof(argsforclear[0])), argsforclear);

        if (strlen(toView) < 1) {
            viewing = false;
        } else {
            found = false;
            for (credential = 0; credential < accounts[user].credentialcount && !found; credential++) {
                if (!strcmp(accounts[user].credentials[credential].name, toView)) {
                    char dateCreatedFormatted[DATE_LENGTH_FORMATTED];
                    char dateModifiedFormatted[DATE_LENGTH_FORMATTED];
                    formatStringToDate(dateCreatedFormatted, accounts[user].credentials[credential].datecreated);
                    formatStringToDate(dateModifiedFormatted, accounts[user].credentials[credential].lastmodified);

                    printf("Credential Name: %s\n", accounts[user].credentials[credential].name);
                    printf("User Name:       %s\n", accounts[user].credentials[credential].username);
                    printf("Password:        %s\n", accounts[user].credentials[credential].encryptedpassword);
                    printf("URL:             %s\n", accounts[user].credentials[credential].url);
                    printf("Date Created:    %s\n", dateCreatedFormatted);
                    printf("Last Modified:   %s\n", dateModifiedFormatted);
                    printf("Details:         %s\n", accounts[user].credentials[credential].details);
                    found = true;

                    printf("\n[ENTER]");
                    clearBuffer();
                    char* argsforclear[] = {"clear"};
                    systemCall((sizeof(argsforclear) / sizeof(argsforclear[0])), argsforclear);
                }
            }
            if (!found) {
                char* argsforclear[] = {"clear"};
                systemCall((sizeof(argsforclear) / sizeof(argsforclear[0])), argsforclear);
                printf(RED"No such credential.\n"RESET);
            }
        }
    }
}


/******************************************************************************
edit credential
******************************************************************************/
void editCredential(int user) {
    account_t accounts[MAX_ACCOUNTS];
    unsigned int numaccounts = databaseRead(accounts);

    if (accounts[user].credentialcount > 0) {
        printf("Credential to edit...\n");
        char toEdit[NAME_LENGTH];
        printf("Credential name> ");
        getUserInput(toEdit, NAME_LENGTH);
        int credIndex;
        bool found = false;
        for (credIndex = 0; credIndex < accounts[user].credentialcount && !found; credIndex++) {
            if (!strcmp(accounts[user].credentials[credIndex].name, toEdit)) {
                found = true;
                char* originalData[4];
                originalData[0] = accounts[user].credentials[credIndex].username;
                originalData[1] = accounts[user].credentials[credIndex].encryptedpassword;
                originalData[2] = accounts[user].credentials[credIndex].url;
                originalData[3] = accounts[user].credentials[credIndex].details;

                char* newData[4];
                newData[0] = malloc(NAME_LENGTH);
                newData[1] = malloc(HASH_CIPHER_LENGTH);
                newData[2] = malloc(URL_LENGTH);
                newData[3] = malloc(DETAILS_LENGTH);

                printf("Username[%s]> ", originalData[0]);
                getUserInput(newData[0], NAME_LENGTH);

                printf("encryptedpassword[%s]> ", originalData[1]);
                getUserInput(newData[1], HASH_CIPHER_LENGTH);

                printf("url[%s]> ", originalData[2]);
                getUserInput(newData[2], URL_LENGTH);

                printf("details[%s]> ", originalData[3]);
                getUserInput(newData[3], DETAILS_LENGTH);

                int dataIndex;
                bool changeMade = false;
                for (dataIndex = 0; dataIndex < 4; dataIndex++) {
                    if (!changeMade) {
                        if (strlen(newData[dataIndex]) > 0) {
                            changeMade = true;
                            dataIndex = -1;
                        }
                    } else {
                        if (strlen(newData[dataIndex]) > 0) {
                            /* need to set change color */
                            printf(GREEN"%s => %s\n"RESET, originalData[dataIndex], newData[dataIndex]);
                        } else {
                            /* print normal color */
                            printf("%s => %s\n", originalData[dataIndex], originalData[dataIndex]);
                        }
                    }
                }
                if (changeMade) {
                    char confirmation[4];
                    printf("Are you sure you want to make these changes? (yes/no) ");
                    getUserInput(confirmation, 4);
                    if (!strcmp(confirmation, "yes")) {
                        if (strlen(newData[0]) > 0) {
                            strcpy(accounts[user].credentials[credIndex].username, newData[0]);
                        }
                        if (strlen(newData[1]) > 0) {
                            strcpy(accounts[user].credentials[credIndex].encryptedpassword, newData[1]);
                        }
                        if (strlen(newData[2]) > 0) {
                            strcpy(accounts[user].credentials[credIndex].url, newData[2]);
                        }
                        if (strlen(newData[3]) > 0) {
                            strcpy(accounts[user].credentials[credIndex].details, newData[3]);
                        }
                        char date[DATE_LENGTH];
                        char* argsfordate[] = {"getdate", date};
                        systemCall((sizeof(argsfordate) / sizeof(argsfordate[0])), argsfordate);
                        strcpy(accounts[user].credentials[credIndex].lastmodified, date);
                        databaseWrite(accounts, numaccounts);

                        char* argsforclear[] = {"clear"};
                        systemCall((sizeof(argsforclear) / sizeof(argsforclear[0])), argsforclear);
                        printf(GREEN"Credential edited.\n"RESET);
                    } else {
                        char* argsforclear[] = {"clear"};
                        systemCall((sizeof(argsforclear) / sizeof(argsforclear[0])), argsforclear);
                        printf("Credential unchanged.\n");
                    }
                } else {
                    char* argsforclear[] = {"clear"};
                    systemCall((sizeof(argsforclear) / sizeof(argsforclear[0])), argsforclear);
                    printf("Credential unchanged.\n");
                }
            }
        }
        if (!found) {
            char* argsforclear[] = {"clear"};
            systemCall((sizeof(argsforclear) / sizeof(argsforclear[0])), argsforclear);
            printf(RED"No such credential.\n"RESET);
        }
    } else {
        char* argsforclear[] = {"clear"};
        systemCall((sizeof(argsforclear) / sizeof(argsforclear[0])), argsforclear);
        printf(RED"Nothing to edit.\n"RESET);
    }
}

/******************************************************************************
deleteCredential
******************************************************************************/
void deleteCredential(int user) {
    account_t accounts[MAX_ACCOUNTS];
    unsigned int numaccounts = databaseRead(accounts);

    printf("Credential to delete...\n");

    if (accounts[user].credentialcount > 0) {
        char toDelete[NAME_LENGTH];
        printf("Credential name> ");
        getUserInput(toDelete, NAME_LENGTH);
        int i;
        bool found = false;
        for (i = 0; i < accounts[user].credentialcount && !found; i++) {
            if (!strcmp(accounts[user].credentials[i].name, toDelete)) {
                found = true;
                char confirmation[4];
                printf("Are you sure you want to delete? (yes/no) ");
                getUserInput(confirmation, 4);
                if (!strcmp(confirmation, "yes")) {
                    int j;
                    int k = 0;
                    for (j = 0; j < accounts[user].credentialcount; j++) {
                        if (strcmp(accounts[user].credentials[j].name, toDelete)) {
                            accounts[user].credentials[k] = accounts[user].credentials[j];
                            k++;
                            loadingProgress(j, 4);
                        }
                    }
                    clearLine();
                    accounts[user].credentialcount = accounts[user].credentialcount - 1;
                    databaseWrite(accounts, numaccounts);
                    char* argsforclear[] = {"clear"};
                    systemCall((sizeof(argsforclear) / sizeof(argsforclear[0])), argsforclear);
                    printf(GREEN"Credential deleted.\n"RESET);
                }
            }
        }
        if (!found) {
          char* argsforclear[] = {"clear"};
          systemCall((sizeof(argsforclear) / sizeof(argsforclear[0])), argsforclear);
          printf(RED"No such credential.\n"RESET);
        }
    } else {
        char* argsforclear[] = {"clear"};
        systemCall((sizeof(argsforclear) / sizeof(argsforclear[0])), argsforclear);
        printf(RED"Nothing to delete.\n"RESET);
    }
}


/******************************************************************************
getProfileValue
******************************************************************************/
void getProfileValue(char value[], const char type[], int maxLength) {
    printf("Enter %s> ", type);
    if (!strcmp(type, "password")) {
        /* TODO store the hash rarther than plain text */
        /* TODO work out what to do with password length */
        fgets(value, maxLength, stdin);
        /* TODO make function for this */
        if (strcspn(value, "\n") >= (maxLength - 1)) {
            clearBuffer();
            printf("INFO: The entered %s was trimmed to %d characters.\n", type, HASH_CIPHER_LENGTH - 1);
        } else {
            value[strcspn(value, "\n")] = '\0';
        }
    } else {
        fgets(value, maxLength, stdin);
        if (strcspn(value, "\n") >= (maxLength - 1)) {
            clearBuffer();
            printf("INFO: The entered %s was trimmed to %d characters.\n", type, maxLength - 1);
        } else {
            value[strcspn(value, "\n")] = '\0';
        }
    }
}


/******************************************************************************
databaseRead
******************************************************************************/
unsigned int databaseRead(account_t accounts[]) {
    FILE *database;
    database = fopen(DATABASE_NAME, "r");

    if (database != NULL) {
        /* TODO include encryption/decryption stuff */
        int i = 0;
        while ((!feof(database)) && i <= MAX_ACCOUNTS) {
            fread(&accounts[i], sizeof(account_t), 1, database);
            i++;
        }
        fclose(database);
        return i - 1;
    }

    return 0;

}



/******************************************************************************
databaseWrite
******************************************************************************/
void databaseWrite(account_t accounts[], unsigned int numaccounts) {
    /* TODO look into compression techniques for files */
    /* TODO encrypt database using hashed portion of key */
    FILE *database;
    database = fopen(DATABASE_NAME, "w");
    fwrite(accounts, sizeof(account_t), numaccounts, database);
    fclose(database);
}

/******************************************************************************
clearBuffer
Clears input (stdin) stream buffer
program.
inputs:
- none
outputs:
- none
******************************************************************************/
void clearBuffer(void) {
    /* Get character in buffer */
    int buffer = getchar();
    /* While there are still characters in the buffer */
    while ((buffer != '\n') && (buffer != EOF)) {
        /* Get next character in buffer */
        buffer = getchar();
    }
}


/******************************************************************************
delay
******************************************************************************/
void delay(int milliseconds) {
    int counter1, counter2;
    milliseconds *= 18;

    for (counter1 = 0; counter1 <= milliseconds; counter1++) {
        for (counter2 = 0; counter2 <= milliseconds; counter2++) {
        }
    }
}


/******************************************************************************
loadingProgress

e.g. if states == 3:
progress (param)
0     1     2     3...
progress (local)
0 1 2 3 4 5 6 7 8 9...

inputs:
- int progress
- int states
outputs:
- none
******************************************************************************/
void loadingProgress(int progress, int states) {
    int i;
    progress *= states;
    char chars[5] = "\\|/-";
    for (i = 0; i < states; i++){
        printf("\r%c", chars[progress % 4]);
        progress++;
        fflush(stdout);

        delay(200);
    }
}


/******************************************************************************
getUserInput
Takes user input and stores string in array of chars passed as parameter.
inputs:
- char choice[]
outputs:
- none
******************************************************************************/
void getUserInput(char store[], int length) {
    fgets(store, length, stdin);
    if (strcspn(store, "\n") >= (length - 1)) {
        clearBuffer();
    } else {
        store[strcspn(store, "\n")] = '\0';
    }
}


/******************************************************************************
clearLine
******************************************************************************/
void clearLine(void) {
    printf("\33[2K\r");
}


/******************************************************************************
systemCall

Current calls:
    clear
    date '+%d%m%Y%H%M%S'

Potential to add more
******************************************************************************/
int systemCall(size_t numArgs, char* arguments[]) {
    if (!strcmp(arguments[0], "clear")) {
        if (!strcmp(CURRENT_OS, "windows")) {
            system("cls");
        } else if (!strcmp(CURRENT_OS, "unix")) {
            system("clear");
        }
        return 1;
    } else if (!strcmp(arguments[0], "getdate") && numArgs > 1) {
        if (!strcmp(CURRENT_OS, "windows")) {
            system("date /t");
            return 1;
        } else if (!strcmp(CURRENT_OS, "unix")) {
            FILE *shell;
            shell = popen("date '+%d%m%Y%H%M%S'", "r");
            if (shell != NULL) {
                fgets(arguments[1], DATE_LENGTH, shell);
                pclose(shell);
                return 1;
            }
        }
    }
    return 0;
}


/******************************************************************************
isIntInArray
******************************************************************************/
bool isIntInArray(int value, int array[], int arraySize) {
    int counter;
    for (counter = 0; counter < arraySize; counter++) {
        if (array[counter] == value) {
            return true;
        }
    }
    return false;
}


/******************************************************************************
formatStringToDate
ddmmyyyyhhmmss
to
dd/mm/yyyy hh:mm:ss
******************************************************************************/
void formatStringToDate(char dateOut[], char stringIn[]) {
    unsigned int index = 0;
    unsigned int counter;
    for (counter = 0; counter < DATE_LENGTH_FORMATTED; counter++) {
        if (counter == 2 || counter == 5) {
            dateOut[counter] = '/';
        } else if (counter == 10) {
            dateOut[counter] = ' ';
        } else if (counter == 13 || counter == 16) {
            dateOut[counter] = ':';
        } else {
            dateOut[counter] = stringIn[index];
            index++;
        }
    }
}
