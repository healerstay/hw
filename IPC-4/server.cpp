#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <vector>

#define PORT 8080

struct Client {
    int sock;
    std::string name;
};

std::vector<Client*> clients;
pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;

void add_client(Client *cl) {
    pthread_mutex_lock(&lock);
    clients.push_back(cl);
    pthread_mutex_unlock(&lock);
}

void remove_client(int sock) {
    pthread_mutex_lock(&lock);
    for(auto it = clients.begin(); it != clients.end(); ++it) {
        if((*it)->sock == sock) {
            delete *it;
            clients.erase(it);
            break;
        }
    }
    pthread_mutex_unlock(&lock);
}

void broadcast(const char *msg, int sender) {
    pthread_mutex_lock(&lock);
    for(auto cl : clients) {
        if(cl->sock != sender) {
            send(cl->sock, msg, strlen(msg), 0);
        }
    }
    pthread_mutex_unlock(&lock);
}

void *handle(void *arg) {
    char buf[2048];
    std::string msg;
    Client *cl = (Client*)arg;
    
    memset(buf, 0, sizeof(buf));
    recv(cl->sock, buf, sizeof(buf), 0);
    for(int i = 0; i < sizeof(buf); i++) {
        if(buf[i] == '\n' || buf[i] == '\r') {
            buf[i] = 0;
            break;
        }
    }
    cl->name = std::string(buf);
    
    printf("%s joined\n", cl->name.c_str());
    msg = "[" + cl->name + " joined]\n";
    broadcast(msg.c_str(), cl->sock);
    
    while(recv(cl->sock, buf, sizeof(buf), 0) > 0) {
        for(int i = 0; i < sizeof(buf); i++) {
            if(buf[i] == '\n') {
                buf[i] = 0;
                break;
            }
        }
        
        if(strcmp(buf, "/exit") == 0) break;
        
        if(strcmp(buf, "/list") == 0) {
            msg = "Users: ";
            pthread_mutex_lock(&lock);
            for(auto c : clients) {
                msg += c->name + " ";
            }
            pthread_mutex_unlock(&lock);
            msg += "\n";
            send(cl->sock, msg.c_str(), msg.length(), 0);
        } else {
            msg = "[" + cl->name + "] " + std::string(buf) + "\n";
            broadcast(msg.c_str(), cl->sock);
            send(cl->sock, msg.c_str(), msg.length(), 0);
        }
        memset(buf, 0, sizeof(buf));
    }
    
    msg = "[" + cl->name + " left]\n";
    printf("%s left\n", cl->name.c_str());
    broadcast(msg.c_str(), cl->sock);
    close(cl->sock);
    remove_client(cl->sock);
    
    return NULL;
}

int main() {
    int server_fd, client_fd;
    struct sockaddr_in addr;
    pthread_t tid;
    
    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(PORT);
    
    bind(server_fd, (struct sockaddr*)&addr, sizeof(addr));
    listen(server_fd, 10);
    
    printf("Server started on port %d\n", PORT);
    
    while(1) {
        client_fd = accept(server_fd, NULL, NULL);
        
        Client *cl = new Client();
        cl->sock = client_fd;
        
        add_client(cl);
        pthread_create(&tid, NULL, handle, cl);
        pthread_detach(tid);
    }
    
    return 0;
}
