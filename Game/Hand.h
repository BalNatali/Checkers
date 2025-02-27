#pragma once
#include <tuple>

#include "../Models/Move.h"
#include "../Models/Response.h"
#include "Board.h"

// methods for hands
class Hand//функционал нажатий
{
  public:
    Hand(Board *board) : board(board)
    {
    }
    tuple<Response, POS_T, POS_T> get_cell() const//POS_T - тип для хранения поля и координат
    {
        SDL_Event windowEvent;
        Response resp = Response::OK;
        int x = -1, y = -1;
        int xc = -1, yc = -1;
        while (true)//бесконечный цикл, который ожидает клик 
        {
            if (SDL_PollEvent(&windowEvent))
            {
                switch (windowEvent.type)
                {
                case SDL_QUIT:
                    resp = Response::QUIT;//выход
                    break;
                case SDL_MOUSEBUTTONDOWN://нажатие (логика при нажатии)
                    x = windowEvent.motion.x;
                    y = windowEvent.motion.y;
                    xc = int(y / (board->H / 10) - 1);
                    yc = int(x / (board->W / 10) - 1);
                    if (xc == -1 && yc == -1 && board->history_mtx.size() > 1)
                    {
                        resp = Response::BACK;//выход
                    }
                    else if (xc == -1 && yc == 8)
                    {
                        resp = Response::REPLAY;//запустить игру заново
                    }
                    else if (xc >= 0 && xc < 8 && yc >= 0 && yc < 8)
                    {
                        resp = Response::CELL;//нажатие по какой-либо клетке 
                    }
                    else
                    {
                        xc = -1;
                        yc = -1;
                    }
                    break;
                case SDL_WINDOWEVENT://изменение ширины экрана
                    if (windowEvent.window.event == SDL_WINDOWEVENT_SIZE_CHANGED)
                    {
                        board->reset_window_size();
                        break;
                    }
                }
                if (resp != Response::OK)//если нажатие было произведено, то выполняется return
                    break;
            }
        }
        return {resp, xc, yc};//возвращает Response и координату если они есть (иначе -1)
    }

    Response wait() const//выполняется в конце игры, когда кто-то выиграл или проиграл. Программа ожидает от нас действие: закрыть или переиграть
    {
        SDL_Event windowEvent;
        Response resp = Response::OK;
        while (true)
        {
            if (SDL_PollEvent(&windowEvent))
            {
                switch (windowEvent.type)
                {
                case SDL_QUIT:
                    resp = Response::QUIT;//закрыть 
                    break;
                case SDL_WINDOWEVENT_SIZE_CHANGED:
                    board->reset_window_size();
                    break;
                case SDL_MOUSEBUTTONDOWN: {
                    int x = windowEvent.motion.x;
                    int y = windowEvent.motion.y;
                    int xc = int(y / (board->H / 10) - 1);
                    int yc = int(x / (board->W / 10) - 1);
                    if (xc == -1 && yc == 8)
                        resp = Response::REPLAY;//переиграть
                }
                break;
                }
                if (resp != Response::OK)
                    break;
            }
        }
        return resp;
    }

  private:
    Board *board;
};
