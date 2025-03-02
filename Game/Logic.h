#pragma once
#include <random>
#include <vector>

#include "../Models/Move.h"
#include "Board.h"
#include "Config.h"

const int INF = 1e9;

class Logic
{
  public:
    Logic(Board *board, Config *config) : board(board), config(config)
    {

        optimization = (*config)("Bot", "Optimization");
    }

    vector<move_pos> find_best_turns(const bool color) {
        //возвращает вектор ходов 
        next_move.clear(); //чистим векторы, содержит ходы
        next_best_state.clear(); //чистим векторы, содержит номер состояния
        //вызываем функцию и находим первый наилучших ход, заполняем ранее очищенные вектора
        find_first_best_turn(board->get_board(), color, -1, -1, 0);

        vector<move_pos> res; //вектор ответа
        int state = 0;
        do {
            res.push_back(next_move[state]); //следующий ход из предыдущего состояния
            state = next_best_state[state];
        } while (state == -1 || next_move[state].x == -1);
        return res;
    }
private:
    vector<vector<POS_T>> make_turn(vector<vector<POS_T>> mtx, move_pos turn) const//производит ход turn на матрице и возвращаем копию матриц. Полезна для передачи новой матрицы в рекурсию. Сама функция пересчитывает матрицу в зависимости от хода 
    {
        if (turn.xb != -1)
            mtx[turn.xb][turn.yb] = 0;
        if ((mtx[turn.x][turn.y] == 1 && turn.x2 == 0) || (mtx[turn.x][turn.y] == 2 && turn.x2 == 7))
            mtx[turn.x][turn.y] += 2;
        mtx[turn.x2][turn.y2] = mtx[turn.x][turn.y];
        mtx[turn.x][turn.y] = 0;
        return mtx;
    }

    double calc_score(const vector<vector<POS_T>> &mtx, const bool first_bot_color) const //подсчитывает количество белых пешек, белых королев, чёрных пешек, чёрных королев
    {
        // color - who is max player
        double w = 0, wq = 0, b = 0, bq = 0;
        for (POS_T i = 0; i < 8; ++i)
        {
            for (POS_T j = 0; j < 8; ++j)
            {
                w += (mtx[i][j] == 1); //подсчёт белых пешек
                wq += (mtx[i][j] == 3);//подсчёт белых королев
                b += (mtx[i][j] == 2);//подсчёт чёрных пешек
                bq += (mtx[i][j] == 4);//подсчёт чёрных королев
                if (scoring_mode == "NumberAndPotential")
                {
                    w += 0.05 * (mtx[i][j] == 1) * (7 - i);
                    b += 0.05 * (mtx[i][j] == 2) * (i);
                }
            }
        }
        if (!first_bot_color)//кто наш бот: белый или чёрный? кто ходит первым
        {
            swap(b, w);
            swap(bq, wq);
        }
        if (w + wq == 0)
            return INF;//если белых нет - выводим бесконечность
        if (b + bq == 0)
            return 0;//если чёрных нет - возвращаем 0
        int q_coef = 4;
        if (scoring_mode == "NumberAndPotential")
        {
            q_coef = 5;//настраиваемый параметр, чему равен вес королевы (насколько она важна), т.е. королева важна как 5 пешек 
        }
        return (b + bq * q_coef) / (w + wq * q_coef);//иначе делим количество чёрных на количество белых , умноженное на q_coef
    }

    double find_first_best_turn(vector<vector<POS_T>> mtx, const bool color, const POS_T x, const POS_T y, size_t state,
        double alpha = -1) { //первая последовательность
        next_move.emplace_back(-1, -1, -1, -1); //изначально ход пустой
        next_best_state.push_back(-1);
        //проверка является ли state 0. Если state = 0, то всё уже посчитано в game, если нет, то считаем сейчас (если не 0, значит мы кого-то бьём)
        if (state != 0) {
            find_turns(x, y, mtx);
            //делаем копию turns, что бы он не затирался
            auto now_turns = turns;
            //делаем копию beats
            auto now_have_beats = have_beats;
            //рассматриваем 2 случая: 1 - если мы кого-то бьём
            if (!now_have_beats && state != 0) {
                return find_best_turns_rec(mtx, 1 - color, 0, alpha); 
            }
            double best_score = -1;
            // если мы не можем никого побить
            for (auto turn : now_turns) {
                size_t new_state = next_move.size(); //создали новый тип
                double score; // что мы получили в результате хода
                if (now_have_beats) {
                    score = find_first_best_turn(make_turn(mtx, turn), color, turn.x2, turn.y2, new state, best_score);
                }
                //если мы никого не бьём, то мы переходим в рекурсию 
                else {
                    //ходит следующий игрок
                    score = find_best_turns_rec(make_turn(mtx, turn), 1 - color, 0, best_score);
                }
                //проверяем наилучший результат, точно ли он не хуже, чем тот что мы получили в результате хода
                if (score > best_score) { //значит мы нашли новую оптиму и надо обновить векторы
                    best_score = score;
                    next_move[state] = turn;
                    next_best_state[state] = (now_have_beats ? new_state : -1); //мы нашли наилучший ход (создаём новое состояние только в случае побития)
                }
            }
        
            return best_score;

    }
    double find_best_turns_rec(vector<vector<POS_T>> mtx, const bool color, const size_t depth, double alpha = -1,//функция просчёта 
        double beta = INF + 1, const POS_T x = -1, const POS_T y = -1) {
        //здесь уже будуть альфа бета отсечения
        //проверяем глубину, условия выхода из рекурсии
        if (depth == Max_deph) {
            //подсчёт score 
            return calc_score(mtx, (depth % 2 == color));
        }
        // если серия побитий, то начинаем искать от x y по матрице
        if (x != -1) {
            find_turns(x, y, mtx);
        }
        //иначе от цвета ищем все возможные ходы
        else {
            find_turns(color, mtx);
        }
        //делаем копию turns, что бы он не затирался
        auto now_turns = turns;
        //делаем копию beats
        auto now_have_beats = have_beats;
        //если сейчас нечего бить и у нас была серия побитий, то мы ищем наилучшее в рекурсии
        if (!now_have_beats && x != -1) {
            return find_best_turns_rec(mtx, 1 - color, depth + 1, alpha, beta)
        }

        //если ходов нет, то это значит что кто-то проиграл
        if (turns.empty()) {
            //кто програл?
            return (depth % 2 ? 0 : INF);
        }
        //поддерживаем  минимум и максимум
        double min_score = INF + 1;
        double max_score = - 1;
        // просчёт лучших ходов
        for (auto turn : now_turns) {
            double score; //создаём score
            //если есть побитие
            if (now_have_beats) {
                score = find_best_turns_rec(make_turn(mtx, turn), color, depth, alpha, beta, turn.x2, turn.y2);
            }
            //если нет побития
            else {
                score = find_best_turns_rec(make_turn(mtx, turn), 1 - color, depth + 1, alpha, beta);
            }
            //обновление минимума и максимума 
            min_score = min(min_score, score);
            max_score = max(max_score, score);
            // альфа и бета отсечения (ускоряем бота)
            //если ходим мы, то мы максимизатор
            if (depth % 2) {
                alpha = max(alpha, max_score);
            }
            // если ходит бот, то он минимизатор
            else {
                beta = min(beta, min_score);
            }
            //оптимизация, ускоряем
            if (optimization != "O0" && alpha > beta) {
                break;
            }
            if (optimization == "O2" && alpha == beta) {
                return (depth % 2 ? max_score + 1 : min_score - 1); //делаем результат похуже, что бы он не был выбран 
            }
        }
        //возвращаем резульат в зависимости от глубины
        return (depth % 2 ? max_score : min_score);
    }

public:
    void find_turns(const bool color) //функция, ищет все доступные ходы, взависимости от входных параметров
    {
        find_turns(color, board->get_board());//берёт текущую доску или ходы. той доски, которую мы ей передали
    }

    void find_turns(const POS_T x, const POS_T y)//так же может брать координаты, которые мы задали
    {
        find_turns(x, y, board->get_board());
    }

private:
    void find_turns(const bool color, const vector<vector<POS_T>> &mtx) //приватная функция, принимает цвет (кто ходит) и матрицу (нашего состояния). Передаём вектор векторов POS_T и цвет
    {
        vector<move_pos> res_turns;
        bool have_beats_before = false;//проходимся по всем клеткам
        for (POS_T i = 0; i < 8; ++i)
        {
            for (POS_T j = 0; j < 8; ++j)
            {
                if (mtx[i][j] && mtx[i][j] % 2 != color)
                {
                    find_turns(i, j, mtx);//если клетка совпадает с цветом, то мы выполняем find_turns(пытаемся найти все возможные ходы), только уже от данной клетки 
                    if (have_beats && !have_beats_before)
                    {
                        have_beats_before = true;
                        res_turns.clear();
                    }
                    if ((have_beats_before && have_beats) || !have_beats_before)
                    {
                        res_turns.insert(res_turns.end(), turns.begin(), turns.end());
                    }
                }
            }
        }
        turns = res_turns;
        shuffle(turns.begin(), turns.end(), rand_eng);
        have_beats = have_beats_before;
    }

    void find_turns(const POS_T x, const POS_T y, const vector<vector<POS_T>> &mtx)//финальная конечная функция, которая ищет все возможные ходы от определённой клетки, рассматривает кучу случаев 
    {
        turns.clear();
        have_beats = false;
        POS_T type = mtx[x][y];
        // check beats - логика побитий
        switch (type)
        {
        case 1://является ли фигура пек+шкой белой? 
        case 2://является ли фигура пек+шкой чёрной? 
            // check pieces - если да, то такая логика
            for (POS_T i = x - 2; i <= x + 2; i += 4)
            {
                for (POS_T j = y - 2; j <= y + 2; j += 4)
                {
                    if (i < 0 || i > 7 || j < 0 || j > 7)
                        continue;
                    POS_T xb = (x + i) / 2, yb = (y + j) / 2;
                    if (mtx[i][j] || !mtx[xb][yb] || mtx[xb][yb] % 2 == type % 2)
                        continue;
                    turns.emplace_back(x, y, i, j, xb, yb);
                }
            }
            break;
        default://если нет, то значит у нас королева с другим списком вариантов ходов
            // check queens
            for (POS_T i = -1; i <= 1; i += 2)
            {
                for (POS_T j = -1; j <= 1; j += 2)
                {
                    POS_T xb = -1, yb = -1;
                    for (POS_T i2 = x + i, j2 = y + j; i2 != 8 && j2 != 8 && i2 != -1 && j2 != -1; i2 += i, j2 += j)
                    {
                        if (mtx[i2][j2])
                        {
                            if (mtx[i2][j2] % 2 == type % 2 || (mtx[i2][j2] % 2 != type % 2 && xb != -1))
                            {
                                break;
                            }
                            xb = i2;
                            yb = j2;
                        }
                        if (xb != -1 && xb != i2)
                        {
                            turns.emplace_back(x, y, i2, j2, xb, yb);
                        }
                    }
                }
            }
            break;
        }
        // check other turns - все возможные варианты просто походить 
        if (!turns.empty())
        {
            have_beats = true;
            return;
        }
        switch (type)
        {
        case 1:
        case 2:
            // check pieces
            {
                POS_T i = ((type % 2) ? x - 1 : x + 1);
                for (POS_T j = y - 1; j <= y + 1; j += 2)
                {
                    if (i < 0 || i > 7 || j < 0 || j > 7 || mtx[i][j])
                        continue;
                    turns.emplace_back(x, y, i, j);
                }
                break;
            }
        default:
            // check queens
            for (POS_T i = -1; i <= 1; i += 2)
            {
                for (POS_T j = -1; j <= 1; j += 2)
                {
                    for (POS_T i2 = x + i, j2 = y + j; i2 != 8 && j2 != 8 && i2 != -1 && j2 != -1; i2 += i, j2 += j)
                    {
                        if (mtx[i2][j2])
                            break;
                        turns.emplace_back(x, y, i2, j2);
                    }
                }
            }
            break;
        }
    }

  public:
    vector<move_pos> turns;//все ходы. которые находятся с помощью функции find_turns
    bool have_beats;//флажок, отвечает за то являются ли наши ходы побитиями (если есть хоть одно побитие, то все возможные ходы - это побития) 
    int Max_depth;//максимальная глубина, задаётся в самой игре и берётся из файла конфигурации

  private:
   // string scoring_mode;//отвечает за оценку calc_score, т.е. как у нас оценивается поле (насколько потенциально может стать королевой та или иная шашка) 
    string optimization;//оптимизация, из конструктора 
    vector<move_pos> next_move;//вектор ходов (нужны для восстановления последовательности ходов) /следующий ход, который приведёт к лучшему состоянию 
    vector<int> next_best_state;//вектор интов (нужны для восстановления последовательности ходов) / следующее лучшее состояние 
    Board *board;//указатель на доску (указатель на оригинал, нужен что бы менять что-то в нужном нам месте)
    Config *config;//указатель на кофиг (указатель на оригинал, нужен что бы менять что-то в нужном нам месте)
};
