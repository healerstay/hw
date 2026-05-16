#include "shared.h"
#include <iostream>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <errno.h>
#include <thread>
#include <atomic>
#include <set>
#include <mutex>
#include <queue>
#include <condition_variable>

std::set<int> clients;
std::mutex clients_mutex; 
std::atomic<bool> server_running(true);
std::queue<std::pair<int,int>> task_queue;
std::mutex queue_mutex;
std::condition_variable queue_cv;

#define PORT 1234
#define MAX_EVENTS 10

void set_nonblocking(int sock);
void send_response(int client_fd, const std::string& response);
void process_command(int client_fd, const std::string& request);
void handle_client(int epoll_fd, int client_fd);

void worker_thread() {
    while (server_running) {
        int epoll_fd;
        int client_fd;
        {
            std::unique_lock<std::mutex> lock(queue_mutex);

            while (task_queue.empty() && server_running) {
                queue_cv.wait(lock);
            }

            if (!server_running) return;

            epoll_fd = task_queue.front().first;
            client_fd = task_queue.front().second;
            task_queue.pop();
        }
        handle_client(epoll_fd, client_fd);
    }
}

void reset_oneshot(int epoll_fd, int fd) {
    epoll_event ev{};
    ev.events = EPOLLIN | EPOLLET | EPOLLONESHOT;
    ev.data.fd = fd;
    epoll_ctl(epoll_fd, EPOLL_CTL_MOD, fd, &ev);
}

int main() {
    init_shared_memory();
    init_semaphores();
    init_aof();
    load_aof();

    std::vector<std::thread> pool;
    for (int i = 0; i < 4; i++) {
        pool.emplace_back(worker_thread);
    }

    std::thread flush_thread(aof_flush_thread);
    std::thread compact_thread(aof_compact_thread);
    std::thread ttl_thread(expiration_thread);

    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) { 
        std::cerr << "socket failed\n"; 
        exit(EXIT_FAILURE); 
    }

    set_nonblocking(server_fd);

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(PORT);

    if (bind(server_fd, (sockaddr*)&addr, sizeof(addr)) < 0) {
        std::cerr << "bind failed\n"; 
        exit(EXIT_FAILURE);
    }

    if (listen(server_fd, 4) < 0) {
        std::cerr << "listen failed\n"; 
        exit(EXIT_FAILURE);
    }

    std::cout << "Server started on port " << PORT << "\n";

    int epoll_fd = epoll_create1(0);
    if (epoll_fd < 0) { 
        std::cerr << "epoll_create failed\n"; 
        exit(EXIT_FAILURE); 
    }

    epoll_event ev{};
    ev.events = EPOLLIN | EPOLLET;
    ev.data.fd = server_fd;
    if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, server_fd, &ev) < 0) {
        std::cerr << "epoll_ctl failed\n"; exit(EXIT_FAILURE);
    }

    epoll_event events[MAX_EVENTS];

    while (server_running) {
        int n = epoll_wait(epoll_fd, events, MAX_EVENTS, 1000);
        if (n < 0) { 
            if (errno == EINTR) continue; 
            std::cerr << "epoll_wait failed\n"; 
            break;
        }

        for (int i = 0; i < n; i++) {
            if (events[i].data.fd == server_fd) {
                while (true) {
                    int client_fd = accept(server_fd, nullptr, nullptr);
                    if (client_fd < 0) { 
                        if (errno == EAGAIN) break; 
                        else {
                            std::cerr << "accept error\n";
                            break; 
                        }
                    }

                    set_nonblocking(client_fd);
                    {
                        std::lock_guard<std::mutex> lock(clients_mutex);
                        clients.insert(client_fd);
                    }
                    epoll_event cev{};
                    cev.events = EPOLLIN | EPOLLET | EPOLLONESHOT;
                    cev.data.fd = client_fd;

                    if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, client_fd, &cev) < 0) {
                        std::cerr << "epoll_ctl add client failed\n";
                        close(client_fd);
                    }
                }
            } else {
                //handle_client(events[i].data.fd);
                {
                    std::lock_guard<std::mutex> lock(queue_mutex);
                    task_queue.push({epoll_fd, events[i].data.fd});
                }
                queue_cv.notify_one();
            }
        }
    }

    server_running = false;
    running = false;

    flush_thread.join();
    compact_thread.join();
    ttl_thread.join();
    for (auto& t : pool) {
        t.join();
    }

    cleanup();
    close(server_fd);
    close(epoll_fd);

    std::cout << "Server stopped cleanly\n";
    return 0;
}
