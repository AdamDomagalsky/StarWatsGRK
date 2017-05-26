//
// Created by domagalsky on 09.05.17.
//

#include <cstdlib>
#include <deque>
#include <iostream>
#include <thread>
#include <boost/asio.hpp>
#include "game_message.hpp"
#include "StarWatsMain.hpp"
using boost::asio::ip::tcp;

typedef std::deque<game_message> game_message_queue;

int main(int argc, char* argv[])
{
    try
    {
        if ( argc != 3)
        {
            std::cerr << "Usage: gameClient <host> <port>\n";
            return 1;
        }
		//Run
		GRKmain(argc, argv);
    }
    catch (std::exception& e)
    {
        std::cerr << "Exception: " << e.what() << "\n";
    }
	system("PAUSE");
    return 0;
}
