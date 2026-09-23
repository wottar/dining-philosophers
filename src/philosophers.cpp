#include <iostream>
#include <thread>
#include <ncurses.h>
#include <mutex>
#include <condition_variable>
#include <chrono>
#include <random>
#include <vector>
#include <memory>
#include <atomic>
#include <csignal>

using std::cout;
using std::cin;
using std::endl;

std::mutex terminal;
std::vector<std::unique_ptr<std::mutex>> w;
std::vector<bool> ma_widelec;
std::vector<std::unique_ptr<std::condition_variable>> cv;
std::atomic<bool> running{true};

void sig_handler(int) {
    running = false;
}

void Wyswietlanie(size_t filozofowie) {
    //INTERFACE OF NON-CHANGEABLE INFORMATION SUCH AS NAME OF COLLUMNS
    WINDOW *legenda = newwin(filozofowie * 3, COLS, 0, 0);
    wattron(legenda, A_BOLD);
    mvwprintw(legenda, 0, 0, "PHILOSOPHERS:");
    mvwprintw(legenda, 0, COLS / 3 - 2, "STATUS:");
    mvwprintw(legenda, 0, 2 * COLS / 3 - 2, "FORKS:");
    mvwprintw(legenda, filozofowie + 2, 0, "LEGEND:");
    wattroff(legenda, A_BOLD);

    wattron(legenda, COLOR_PAIR(1));
    mvwprintw(legenda, filozofowie + 3, 0, "T - THINKS");
    wattroff(legenda, COLOR_PAIR(1));

    wattron(legenda, COLOR_PAIR(2));
    mvwprintw(legenda, filozofowie + 4, 0, "W - WAITS");
    wattroff(legenda, COLOR_PAIR(2));

    wattron(legenda, COLOR_PAIR(3));
    mvwprintw(legenda, filozofowie + 5, 0, "E - EATS");
    wattroff(legenda, COLOR_PAIR(3));

    wrefresh(legenda);
    delwin(legenda);
}

void Filozof(size_t id, size_t filozofowie) {

    WINDOW *filozof_win = newwin(1, COLS / 3, id + 1, 0);
    WINDOW *status_win  = newwin(1, COLS / 3, id + 1, COLS / 3);
    WINDOW *widelce_win = newwin(1, COLS / 3, id + 1, 2 * COLS / 3);

    while (running) {
        // PSEUDO-RANDOM NUMBERS GENERATOR
        std::random_device rd;
        std::mt19937 mt(rd()); //mersenne twister generator lcizb losowych do losowego czasu jedzenia przez filozofa
        std::uniform_int_distribution<int> dist(1000, 2000);
        int random_time = dist(mt);

        // -----THINKS-----

        std::unique_lock<std::mutex> terminal_lock(terminal);

        mvwprintw(filozof_win, 0, 0, "Philosopher %zu", id);
        wrefresh(filozof_win);
        wattron(status_win, COLOR_PAIR(1));
        mvwprintw(status_win, 0, 0, "T");
        wattroff(status_win, COLOR_PAIR(1));
        wrefresh(status_win);
        terminal_lock.unlock();

        //SOLUTION FOR DEADLOCK AND STARVATION
        size_t lewy_widelec_id = id;
        size_t prawy_widelec_id = (id + 1) % filozofowie;

        if (id % 2 == 1) {
            std::swap(lewy_widelec_id, prawy_widelec_id);
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(1200));
        if (!running) break;

        // -----TAKE LEFT FORK-----
        std::unique_lock<std::mutex> lewy_widelec_lock(*w[lewy_widelec_id]);
        cv[lewy_widelec_id]->wait(lewy_widelec_lock, [lewy_widelec_id] { return !ma_widelec[lewy_widelec_id] || !running; });
        if (!running) break;
        ma_widelec[lewy_widelec_id] = true;

        terminal_lock.lock();
        wattron(status_win, COLOR_PAIR(2));
        mvwprintw(status_win, 0, 0, "W");
        wattroff(status_win, COLOR_PAIR(2));
        wrefresh(status_win);
        mvwprintw(widelce_win, 0, 0, "%zu", lewy_widelec_id);
        wrefresh(widelce_win);
        terminal_lock.unlock();

        // -----TAKE RIGHT FORK-----
        std::unique_lock<std::mutex> prawy_widelec_lock(*w[prawy_widelec_id]);
        cv[prawy_widelec_id]->wait(prawy_widelec_lock, [prawy_widelec_id] { return !ma_widelec[prawy_widelec_id] || !running; });
        if (!running) {
            ma_widelec[lewy_widelec_id] = false;
            lewy_widelec_lock.unlock();
            cv[lewy_widelec_id]->notify_one();
            break;
        }
        ma_widelec[prawy_widelec_id] = true;

        terminal_lock.lock();
        wattron(status_win, COLOR_PAIR(2));
        mvwprintw(status_win, 0, 0, "W");
        wattroff(status_win, COLOR_PAIR(2));
        wrefresh(status_win);
        mvwprintw(widelce_win, 0, 0, "  %zu", prawy_widelec_id);
        wrefresh(widelce_win);
        terminal_lock.unlock();

        // -----EAT-----
        terminal_lock.lock();
        wattron(status_win, COLOR_PAIR(3));
        mvwprintw(status_win, 0, 0, "E");
        wattroff(status_win, COLOR_PAIR(3));
        wrefresh(status_win);
        mvwprintw(widelce_win, 0, 0, "%zu %zu", id, (id + 1) % filozofowie);
        wrefresh(widelce_win);
        terminal_lock.unlock();

        std::this_thread::sleep_for(std::chrono::milliseconds(random_time));

        // -----PUT OFF LEFT FORK-----
        terminal_lock.lock();
        ma_widelec[lewy_widelec_id] = false;
        lewy_widelec_lock.unlock();
        cv[lewy_widelec_id]->notify_one();
        mvwprintw(widelce_win, 0, 0, " ");
        wrefresh(widelce_win);
        terminal_lock.unlock();

        // -----PUT OFF RIGHT FORK-----
        terminal_lock.lock();
        ma_widelec[prawy_widelec_id] = false;
        prawy_widelec_lock.unlock();
        cv[prawy_widelec_id]->notify_one();
        mvwprintw(widelce_win, 0, 0, "   ");
        wrefresh(widelce_win);
        terminal_lock.unlock();
    }

    delwin(filozof_win);
    delwin(status_win);
    delwin(widelce_win);
}

int main(int argc, char* argv[]) {
    if (argc < 2 || atoi(argv[1]) < 2) return 1; 

    std::signal(SIGINT, sig_handler);

    initscr();
    cbreak();
    noecho();
    curs_set(0);

    start_color();
    init_pair(1, COLOR_GREEN, COLOR_BLACK);
    init_pair(2, COLOR_YELLOW, COLOR_BLACK);
    init_pair(3, COLOR_RED, COLOR_BLACK);

    size_t filozofowie = static_cast<size_t>(atoi(argv[1]));

    ma_widelec.resize(filozofowie, false);
    for (size_t i = 0; i < filozofowie; i++) 
    {
        w.push_back(std::make_unique<std::mutex>());
        cv.push_back(std::make_unique<std::condition_variable>());
    }

    Wyswietlanie(filozofowie);

    std::vector<std::thread> f(filozofowie);
    
    for (size_t i = 0; i < filozofowie; i++) 
    {
        f[i] = std::thread(Filozof, i, filozofowie);
    }

    for (size_t i = 0; i < filozofowie; i++) 
    {
        f[i].join();
    }

    endwin();
    return 0;
}