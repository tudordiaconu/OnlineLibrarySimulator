#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include "helpers.h"
#include "requests.h"

int main() {
    int sockfd;
    char* message;
    char* response;
    char* access_token = calloc(300, sizeof(char));

    char *id = calloc(10, sizeof(char));
    char *url = calloc(100, sizeof(char));

    char* payload = calloc(300, sizeof(char));
    char* http_number_string = calloc(3, sizeof(char));

    char *username = calloc(30, sizeof(char));
    char *password = calloc(30, sizeof(char));

    char *title = calloc(50, sizeof(char));
    char *author = calloc(100, sizeof(char));
    char *genre = calloc(50, sizeof(char));
    char *publisher = calloc(50, sizeof(char));
    char *page_count = calloc(5, sizeof(char));

    char** body_data;
    char** cookies;
    char* authentification = calloc(100, sizeof(char));
    char* list_of_books = calloc(3000, sizeof(char));

    int logged_in = 0;
    
    body_data = calloc(10, sizeof(char *));
    cookies = calloc(10, sizeof(char *));

    for (int i = 0; i < 10; ++i) {
        body_data[i] = calloc(100, sizeof(char));
        cookies[i] = calloc(1000, sizeof(char));
    }

    // variable where we read our command
    char* command = calloc(15, sizeof(char));

    // program runs constantly until the "exit" command
    while(1) {
        // reading the command
        fgets(command, 15, stdin);
        command[strlen(command) - 1] = '\0';
        if (!strcmp(command, "register")) {
            // registering a new user

            // opening a new connection
            sockfd = open_connection("34.241.4.235", 8080, AF_INET, SOCK_STREAM, 0);
            
            // introducing the username and password
            // after the required prompts
            int valid = 1;            
            printf("username=");
            fgets(username, 30, stdin);
            username[strlen(username) - 1] = '\0';
            printf("password=");
            fgets(password, 30, stdin);
            password[strlen(password) - 1] = '\0';

            // checking if the username and password don't have any spaces
            if (strchr(username, ' ') || strchr(password, ' ') || strlen(username) == 0 || strlen(password) == 0) {
                valid = 0;
            }

             if (valid == 0) {
                printf("Username/Password cannot have spaces!\n");
            } else {
                // creating a JSON format message with the username and
                // password for the POST request
                strcpy(body_data[0], "{\"username\":\"");
                strcat(body_data[0], username);
                strcat(body_data[0], "\",");

                strcpy(body_data[1], "\"password\":\"");
                strcat(body_data[1], password);
                strcat(body_data[1], "\"}");

                // computing the POST request, sending it to the server
                // and receiving the server's response
                message = compute_post_request("34.241.4.235",
                "/api/v1/tema/auth/register", "application/json", body_data, 2, NULL, 0, NULL);
                send_to_server(sockfd, message);
                response = receive_from_server(sockfd);

                // extracting the number of the HTTP response
                memcpy(http_number_string, response + 9, 3);
                int http_number = atoi(http_number_string);

                // checking if the error of the username already being used
                // appears
                payload = strstr(response, "{\"error\":\"The username");


                if (http_number == 201) {
                    // HTTP success
                    printf("User %s was successfully created!\n", username);
                } else if (payload) {
                    // username already taken
                    printf("Username %s already exists!\n", username);  
                } else if (http_number == 429) {
                    // too many requests in a short time
                    printf("Too many requests! Calm down\n");
                } else if (http_number / 100 == 5) {
                    printf("Internal server error! Try again later!\n");
                }
            }

            // closing the connection to the server
            close_connection(sockfd);
        } else if (!strcmp(command, "login")) {
            // logging in as an existing user

            if (logged_in == 0) {
                // checking if we're not already logged in

                // opening a new connection to the server
                sockfd = open_connection("34.241.4.235", 8080, AF_INET, SOCK_STREAM, 0);

                // reading the username and password and checking for spaces
                char *username = calloc(30, sizeof(char));
                char *password = calloc(30, sizeof(char));

                int valid = 1;            
                printf("username=");
                fgets(username, 30, stdin);
                username[strlen(username) - 1] = '\0';
                
                printf("password=");
                fgets(password, 30, stdin);
                password[strlen(password) - 1] = '\0';

                if (strchr(username, ' ') || strchr(password, ' ') || strlen(username) == 0 || strlen(password) == 0) {
                    valid = 0;
                }

                if (valid == 0) {
                    printf("Username/Password cannot have spaces!\n");
                } else {
                    // computing the JSON format message with the username and
                    // password
                    strcpy(body_data[0], "{\"username\":\"");
                    strcat(body_data[0], username);
                    strcat(body_data[0], "\",");

                    strcpy(body_data[1], "\"password\":\"");
                    strcat(body_data[1], password);
                    strcat(body_data[1], "\"}");

                    // sending a POST request to the server to log in and getting its response
                    message = compute_post_request("34.241.4.235", "/api/v1/tema/auth/login",
                    "application/json", body_data, 2, NULL, 0, NULL);
                    send_to_server(sockfd, message);
                    response = receive_from_server(sockfd);
                    
                    // extracting the number of the HTTP response
                    memcpy(http_number_string, response + 9, 3);
                    int http_number = atoi(http_number_string);

                    // in case it's an error, we get its type
                    if (http_number == 400) {
                        char *error = strstr(response, "error");
                        char *credentials = strstr(error, "Credentials");
                        char *no_account = strstr(error, "No account");

                        if (credentials) {
                            printf("Wrong password for user %s! Try again!\n", username);
                        } else if (no_account) {
                            printf("There is no account with username %s!\n", username);
                        }

                    } else if (http_number == 429) {
                        printf("Too many requests! Calm down\n");
                    } else if (http_number == 200) {
                        // in case of success, we also the sessions's id
                        logged_in = 1;
                        printf("User %s logged in successfully!\n", username);
                        char *cookie = strstr(response, "connect.sid");
                        authentification = strtok(cookie, ";");
                    } else if (http_number / 100 == 5) {
                        printf("Internal server error! Try again later!\n");
                    }
                }

                // saving the session's id as a cookie
                strcpy(cookies[0], authentification);

                // closing the connection
                close_connection(sockfd);
            } else {
                printf("Already logged in!\n");
            }
        } else if (!strcmp(command, "enter_library")) {
            // the command to gain access to the library

            // opening a new connection to the server
            sockfd = open_connection("34.241.4.235", 8080, AF_INET, SOCK_STREAM, 0);

            // computing a GET request with the current session's id as a cookie and sending it to the server
            message = compute_get_request("34.241.4.235", "/api/v1/tema/library/access", NULL, cookies, 1, NULL);
            send_to_server(sockfd, message);
            response = receive_from_server(sockfd);

            // getting the HTTP response number
            memcpy(http_number_string, response + 9, 3);
            int http_number = atoi(http_number_string);

            if (http_number == 200) {
                // in case of success, we extract the JWT token
                printf("Access to the library obtained successfully!\n");
                access_token = strstr(response, "{\"token\":");
                access_token = access_token + 10;
                access_token[strlen(access_token) - 2] = '\0';
            } else if (http_number == 401) {
                // we get error 401, it means we are not logged in as we don't have the session cookie
                printf("You need to be logged in order to access the library!\n");
            } else if (http_number == 429) {
                printf("Too many requests! Calm down\n");
            }

            // closing the connection
            close_connection(sockfd);
        } else if (!strcmp(command, "get_books")) {
            // the command to get the list of all the books

            // opening a new connection to the server
            sockfd = open_connection("34.241.4.235", 8080, AF_INET, SOCK_STREAM, 0);

            // computing the GET request in order to get the list of books 
            message = compute_get_request("34.241.4.235", "/api/v1/tema/library/books", NULL, cookies, 1, access_token);
            send_to_server(sockfd, message);
            response = receive_from_server(sockfd);
            
            // getting the HTTP response number
            memcpy(http_number_string, response + 9, 3);
            int http_number = atoi(http_number_string);

            if (http_number == 200) {
                // in case of success, we print the list of books
                printf("Here is the list of books:\n");
                list_of_books = strstr(response, "[");
                puts(list_of_books);
            } else if (http_number == 500) {
                // we get 500 when we haven't received the JWT token yet
                printf("You do not have access to the library! Use command enter_library!\n");
            } else if (http_number == 429) {
                printf("Too many requests! Calm down\n");
            }

            // closing the connection
            close_connection(sockfd);
        } else if (!strcmp(command, "get_book")) {
            // the command to get a book with a specific id

            // opening a new connection to the server
            sockfd = open_connection("34.241.4.235", 8080, AF_INET, SOCK_STREAM, 0);

            // reading the wanted id and computing the url
            printf("id=");
            fgets(id, 10, stdin);
            id[strlen(id) - 1] = '\0';
            strcpy(url, "/api/v1/tema/library/books/");
            strcat(url, id);
            
            // sending the GET request in order to get the wanted book
            message = compute_get_request("34.241.4.235", url, NULL, cookies, 1, access_token);
            send_to_server(sockfd, message);
            response = receive_from_server(sockfd);

            // getting the HTTP response number
            memcpy(http_number_string, response + 9, 3);
            int http_number = atoi(http_number_string);

            if (http_number == 500) {
                // we get 500 when we haven't received the JWT token yet
                printf("You do not have access to the library! Use command enter_library!\n");
            } else if (http_number == 429) {
                printf("Too many requests! Calm down\n");
            } else if (http_number == 200) {
                // in case of success, we print the details about the book
                printf("Here are details of the book with id %s:\n", id);
                payload = strstr(response, "[{\"title");
                puts(payload);
            } else if (http_number == 404) {
                printf("There is no book with id %s!\n", id);
            }

            // closing the connection
            close_connection(sockfd);
        } else if (!strcmp(command, "add_book")) {
            // command to add a new book to the database

            // opening a new connection to the server
            sockfd = open_connection("34.241.4.235", 8080, AF_INET, SOCK_STREAM, 0);

            // entering the details of the book after each prompt
            printf("title=");
            fgets(title, 50, stdin);
            title[strlen(title) - 1] = '\0';
            printf("author=");
            fgets(author, 100, stdin);
            author[strlen(author) - 1] = '\0';
            printf("genre=");
            fgets(genre, 50, stdin);
            genre[strlen(genre) - 1] = '\0';
            printf("publisher=");
            fgets(publisher, 50, stdin);
            publisher[strlen(publisher) - 1] = '\0';
            printf("page_count=");
            fgets(page_count, 5, stdin);
            page_count[strlen(page_count) - 1] = '\0';

            // creating the JSON formatted message in order to include it in the
            // POST request
            strcpy(body_data[0], "{\"title\":\"");
            strcat(body_data[0], title);
            strcat(body_data[0], "\",");

            strcpy(body_data[1], "\"author\":\"");
            strcat(body_data[1], author);
            strcat(body_data[1], "\",");

            strcpy(body_data[2], "\"genre\":\"");
            strcat(body_data[2], genre);
            strcat(body_data[2], "\",");

            strcpy(body_data[3], "\"page_count\":\"");
            strcat(body_data[3], page_count);
            strcat(body_data[3], "\",");

            strcpy(body_data[4], "\"publisher\":\"");
            strcat(body_data[4], publisher);
            strcat(body_data[4], "\"}");

            // computing a POST request and sending it
            message = compute_post_request("34.241.4.235", "/api/v1/tema/library/books", "application/json", body_data, 5, cookies, 1, access_token);
            send_to_server(sockfd, message);
            response = receive_from_server(sockfd);

            // getting the HTTP response number
            memcpy(http_number_string, response + 9, 3);
            int http_number = atoi(http_number_string);

            if (http_number == 200) {
                printf("Book added succesfully!\n");
            } else if (http_number == 429) {
                printf("Too many requests! Calm down\n");
            } else if (http_number == 500) {
                // extracting the type of error we get
                char *error = strstr(response, "error");
                char *something = strstr(error, "Something");
                char *no_token = strstr(error, "tokenn");

                if (something) {
                    printf("Wrong format of details of the book! Please keep in mind that the title, author, publisher and genre are strings and the page count is a number!\n");
                } else if(no_token) {
                    printf("You do not have access to the library! Use command enter_library!\n");
                }
            }
            
            // closing the connection
            close_connection(sockfd);
        } else if (!strcmp(command, "delete_book")) {
            // command to delete a book

            // opening a new connection to the server
            sockfd = open_connection("34.241.4.235", 8080, AF_INET, SOCK_STREAM, 0);

            printf("id=");
            scanf("%s", id);
            strcpy(url, "/api/v1/tema/library/books/");
            strcat(url, id);

            // computing a DELETE request with the personalized url and sending it
            message = compute_delete_request("34.241.4.235", url, NULL, cookies, 1, access_token);
            send_to_server(sockfd, message);
            response = receive_from_server(sockfd);

            // extracting the HTTP respone number
            memcpy(http_number_string, response + 9, 3);
            int http_number = atoi(http_number_string);

            // treating each case
            if (http_number == 200) {
                printf("Book deleted succesfully!\n");
            } else if (http_number == 404) {
                printf("There is no book with id %s!\n", id); 
            } else if (http_number == 429) {
                printf("Too many requests! Calm down\n");
            } else if (http_number == 500) {
                printf("You do not have access to the library! Use command enter_library!\n");
            }

            // closing the connection
            close_connection(sockfd);
        } else if (!strcmp(command, "logout")) {
            // command to log out
            if (logged_in == 1) {
                // opening a new connection to the server
                sockfd = open_connection("34.241.4.235", 8080, AF_INET, SOCK_STREAM, 0);
                
                // computing a GET request to logout and sending it to the server
                message = compute_get_request("34.241.4.235", "/api/v1/tema/auth/logout", NULL, cookies, 1, NULL);
                send_to_server(sockfd, message);
                response = receive_from_server(sockfd);

                // getting the HTTP response number
                memcpy(http_number_string, response + 9, 3);
                int http_number = atoi(http_number_string);

                // treating the cases
                if (http_number == 200) {
                    logged_in = 0;
                    printf("Logged out!\n");
                    free(access_token);
                    free(authentification);
                    free(cookies[0]);
                } else if (http_number == 429) {
                    printf("Too many requests! Calm down\n");
                } else if (http_number == 400) {
                    printf("You are not logged in!\n");
                }

                // closing the connection
                close_connection(sockfd);
            } else {
                printf("You are not logged in!\n");
            }
        } else if (!strcmp(command, "exit")) {
            break;
        } else {
            printf("Wrong usage! Working commands are:\n\tregister\n\tlogin\n\tenter_library\n\tget_books\n\tget_book\n\tadd_book\n\tdelete_book\n\tlogout\n\texit\n");
        }
    }

    for (int i = 0; i < 10; ++i) {
        free(body_data[i]);
    }

    if (logged_in == 1) {
        free(cookies[0]);
    }

    for (int i = 1; i < 10; ++i) {
        free(cookies[i]);
    }

    free(http_number_string);
    free(payload);
    free(id);
    free(url);
    free(username);
    free(password);
    free(title);
    free(author);
    free(genre);
    free(publisher);
    free(list_of_books);
    free(command);
    free(cookies);
    free(body_data);
}
