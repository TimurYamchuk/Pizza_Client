#define WIN32_LEAN_AND_MEAN

#include <winsock2.h>
#include <windows.h>
#include <iostream>
#include <vector>
#include <string>
#include <sstream>
using namespace std;

#define MAX_CLIENTS 30
#define DEFAULT_BUFLEN 1000
#define CLIENT_MONEY 60

#pragma comment(lib, "ws2_32.lib")
#pragma warning(disable:4996)

SOCKET server_socket;

enum PizzaType {
    MARGHERITA,
    PEPPERONI,
    HAWAIIAN,
    VEGGIE
};

struct myClients {
    SOCKET Client_socket;
    int Client_money;
    myClients(SOCKET socket, int money) : Client_socket(socket), Client_money(money) {}
};

vector<myClients> CLIENTS;

void lowerCase(char* client_message) {
    for (int i = 0; client_message[i] != '\0'; ++i) {
        client_message[i] = tolower(client_message[i]);
    }
}

void orderInTheStore(SOCKET* client_sockets, int i, const char* client_message, myClients& myClients) {
    int Pizza_prices[] = { 10, 12, 14, 11 };
    int Pizza_times[] = { 5, 6, 7, 6 };

    int total_time = 0, total_price = 0;

    int Pizza_counts[4] = { 0 };
    int Client_money = myClients.Client_money;

    bool First_entry;

    if (Client_money == CLIENT_MONEY) {
        First_entry = true;
    }
    else {
        First_entry = false;
    }

    if (First_entry) {
        string message = "Your money: " + to_string(Client_money) + "\n";
        message += "margherita: " + to_string(Pizza_prices[MARGHERITA]) + "\n";
        message += "pepperoni: " + to_string(Pizza_prices[PEPPERONI]) + "\n";
        message += "hawaiian: " + to_string(Pizza_prices[HAWAIIAN]) + "\n";
        message += "veggie: " + to_string(Pizza_prices[VEGGIE]) + "\n";
        send(client_sockets[i], message.c_str(), message.size(), 0);
    }

    stringstream stream(client_message);
    string One_word;
    while (stream >> One_word) {
        if (One_word == "margherita") {
            Pizza_counts[MARGHERITA]++;
            total_time += Pizza_times[MARGHERITA];
            total_price += Pizza_prices[MARGHERITA];
        }
        else if (One_word == "pepperoni") {
            Pizza_counts[PEPPERONI]++;
            total_time += Pizza_times[PEPPERONI];
            total_price += Pizza_prices[PEPPERONI];
        }
        else if (One_word == "hawaiian") {
            Pizza_counts[HAWAIIAN]++;
            total_time += Pizza_times[HAWAIIAN];
            total_price += Pizza_prices[HAWAIIAN];
        }
        else if (One_word == "veggie") {
            Pizza_counts[VEGGIE]++;
            total_time += Pizza_times[VEGGIE];
            total_price += Pizza_prices[VEGGIE];
        }
    }

    if (Pizza_counts[MARGHERITA] != 0 || Pizza_counts[PEPPERONI] != 0 || Pizza_counts[HAWAIIAN] != 0 || Pizza_counts[VEGGIE] != 0) {
        if (Client_money - total_price >= 0) {
            string text = "Your order:";
            if (Pizza_counts[MARGHERITA] > 0) {
                text += " Margherita: " + to_string(Pizza_counts[MARGHERITA]);
            }
            if (Pizza_counts[PEPPERONI] > 0) {
                text += " Pepperoni: " + to_string(Pizza_counts[PEPPERONI]);
            }
            if (Pizza_counts[HAWAIIAN] > 0) {
                text += " Hawaiian: " + to_string(Pizza_counts[HAWAIIAN]);
            }
            if (Pizza_counts[VEGGIE] > 0) {
                text += " Veggie: " + to_string(Pizza_counts[VEGGIE]);
            }
            text += "\nYour order is ready in " + to_string(total_time) + " seconds";

            send(client_sockets[i], text.c_str(), text.size(), 0);

            if (total_time > 0) {
                Sleep(total_time * 1000);
                text = "Your order is ready";
                send(client_sockets[i], text.c_str(), text.size(), 0);
            }
            myClients.Client_money = Client_money - total_price;
        }
        else {
            string text = "You don't have the money\n";
            send(client_sockets[i], text.c_str(), text.size(), 0);
        }
    }
    else {
        string text = "You haven't ordered anything\n";
        send(client_sockets[i], text.c_str(), text.size(), 0);
    }
    string text = "Your money: " + to_string(myClients.Client_money) + "\n";
    send(client_sockets[i], text.c_str(), text.size(), 0);
}

int main() {
    WSADATA wsa;
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
        printf("Failed. Error Code: %d", WSAGetLastError());
        return 1;
    }
    if ((server_socket = socket(AF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET) {
        printf("Could not create socket: %d", WSAGetLastError());
        return 2;
    }
    sockaddr_in server;
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons(10000);

    if (bind(server_socket, (sockaddr*)&server, sizeof(server)) == SOCKET_ERROR) {
        printf("Bind failed with error code: %d", WSAGetLastError());
        return 3;
    }
    listen(server_socket, MAX_CLIENTS);

    cout << "Start Server\n";

    fd_set readfds;
    SOCKET client_sockets[MAX_CLIENTS] = {};

    while (true) {
        FD_ZERO(&readfds);
        FD_SET(server_socket, &readfds);
        for (int i = 0; i < MAX_CLIENTS; i++) {
            SOCKET s = client_sockets[i];
            if (s > 0) {
                FD_SET(s, &readfds);
            }
        }

        if (select(0, &readfds, NULL, NULL, NULL) == SOCKET_ERROR) {
            printf("select function call failed with error code : %d", WSAGetLastError());
            return 4;
        }

        SOCKET new_socket;
        sockaddr_in address;
        int addrlen = sizeof(sockaddr_in);
        if (FD_ISSET(server_socket, &readfds)) {
            if ((new_socket = accept(server_socket, (sockaddr*)&address, &addrlen)) < 0) {
                perror("accept function error");
                return 5;
            }

            printf("Client, socket fd is %d, ip is: %s, port: %d\n", new_socket, inet_ntoa(address.sin_addr), ntohs(address.sin_port));

            for (int i = 0; i < MAX_CLIENTS; i++) {
                if (client_sockets[i] == 0) {
                    client_sockets[i] = new_socket;
                    CLIENTS.push_back(myClients(new_socket, CLIENT_MONEY));
                    printf("Adding to list of sockets at index %d\n", i);
                    break;
                }
            }
        }

        for (int i = 0; i < MAX_CLIENTS; i++) {
            SOCKET s = client_sockets[i];
            if (FD_ISSET(s, &readfds)) {
                sockaddr_in address;
                int addrlen = sizeof(sockaddr_in);

                char client_message[DEFAULT_BUFLEN];

                int client_message_length = recv(s, client_message, DEFAULT_BUFLEN, 0);
                client_message[client_message_length] = '\0';

                lowerCase(client_message);

                string check_exit = client_message;
                if (check_exit == "off") {
                    cout << "Client #" << i << " is off\n";
                    client_sockets[i] = 0;
                }
                else {
                    for (int j = 0; j < CLIENTS.size(); ++j) {
                        if (CLIENTS[j].Client_socket == s) {
                            orderInTheStore(client_sockets, i, client_message, CLIENTS[j]);
                            break;
                        }
                    }
                }
            }
        }
    }

    WSACleanup();
}