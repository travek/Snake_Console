#include <iostream>
#include <ctime>
#include <atomic>
#include <thread>
#include <conio.h>
#include <windows.h>
#include <vector>
#include <deque>
#include <algorithm>  
#include "easylogging++.h"

INITIALIZE_EASYLOGGINGPP

using namespace std;
typedef std::pair<int, int> pos;


class functional_keys
{
public:
	std::atomic<int> key_pressed;
	static const int game_size_x = 20;
	static const int game_size_y = 20;
	std::atomic<char> direction;
	functional_keys();
	std::deque<pos> snake;
	std::atomic<int> exit;

};


functional_keys::functional_keys()
{
	key_pressed.store(0);
	direction.store('N');
	exit.store(0);
}

int make_field(functional_keys& kp)
{
	for (int i = 0; i < kp.game_size_x + 2; i++)
	{
		if (i == 0 || i == (kp.game_size_x +1))
		{
			cout << "*";
			continue;
		}

		cout << "=";
	}
	cout << endl;

	for (int i = 1; i < kp.game_size_y +1 ; i++)
	{
		for (int j = 0; j < kp.game_size_x +2; j++)
		{
			if (j==0)
			{
				cout << "|";
				continue;
			}
			if (j == kp.game_size_x +1 )
			{
				cout << "|\n";
				continue;
			}
			cout << " ";
		}
	}
	for (int i = 0; i < kp.game_size_x +2; i++)
	{
		if (i == 0 || i == (kp.game_size_x +1))
		{
			cout << "*";
			continue;
		}

		cout << "=";
	}
	cout << endl;

	cout << "\n";
	return 0;
}

int adapt_x(const int x)
{
	return x + 1;
}
int adapt_y(const int y)
{
	return y + 1;
}


int read_key(functional_keys &kp)
{
	int key=0;

	while (true)
	{
			if (_kbhit())
		{
			key = _getch();
			
			if (key == 75 || key == 80|| key == 77 || key == 72)  //72 - up , 77- right, 75 - left, 80 - down
				kp.key_pressed.store(key);

			if (key == 'b')
			{
				kp.exit.store(1);
				break;
			}
		}

	}

	LOG(INFO) << "leaving 2nd thread";
	return 0;
}

int clear_screen_at(const HANDLE hConsole, const int x, const int y)
{
	COORD pos = { adapt_x(x), adapt_y(y) };
	SetConsoleCursorPosition(hConsole, pos);
	WriteConsole(hConsole, " ", 1, NULL, NULL);

	return 0;
}

int generate_fruit(functional_keys& fk, pos& fr)
{

	if (fr.first == -1 && fr.second == -1)
	{
		while (1 == 1)
		{
			fr.first = std::rand() % fk.game_size_x;
			fr.second = std::rand() % fk.game_size_y;

			if (std::all_of(fk.snake.begin(), fk.snake.end(), [fr](pos p){return p.first != fr.first || p.second != fr.second;}) )
				break;
		}
	}

	return 0;
}

int paste_fruit(const HANDLE hConsole, const pos fruit, functional_keys& fk)
{

	if ((fruit.first != -1 && fruit.second != -1) && (std::all_of(fk.snake.begin(), fk.snake.end(), [fruit](pos p) {return p.first != fruit.first || p.second != fruit.second; }))  )
	{
		COORD cpos = { adapt_x(fruit.first), adapt_y(fruit.second) };
		SetConsoleCursorPosition(hConsole, cpos);
		WriteConsole(hConsole, "$", 1, NULL, NULL);
	}


	return 0;
}

int check_collision_fruit(const pos head, pos& fruit)
{

	if (head.first == fruit.first && head.second == fruit.second)
	{
		fruit.first = -1;
		fruit.second = -1;
		return 1;
	}

	return 0;
}

int game_logic(functional_keys& kp, const HANDLE hConsole)
{
	int key=0;
	pos fr; // fruit coords
	fr.first = -1;
	fr.second = -1;
	pos last_pos;
	pos head_pos;

	while (true)
	{
		if (kp.exit.load() == 1)
			break;
		key = kp.key_pressed.load();
		std::this_thread::sleep_for(std::chrono::milliseconds(250));
		last_pos = kp.snake.at(kp.snake.size()-1);
		head_pos = kp.snake.at(0);

		if (key != 0)
		{
			//cout << "Key pressed: "<<key<<" \n";
			kp.key_pressed.store(0);
		}
		if (key == 72 and !(kp.direction.load() =='D' && kp.snake.size() >= 2 ))    // up
		{
			int sposition_x = head_pos.first;
			int sposition_y = head_pos.second-1;
			if (sposition_y < 0)
				sposition_y = kp.game_size_y-1;
			COORD cpos = { adapt_x(sposition_x), adapt_y(sposition_y) };
			SetConsoleCursorPosition(hConsole, cpos);
			WriteConsole(hConsole, "#", 1, NULL, NULL);
			kp.snake.push_front(std::make_pair(sposition_x, sposition_y));
			head_pos = kp.snake.at(0);
			kp.direction.store('U');
			if (check_collision_fruit(head_pos, fr) != 1)
			{
				clear_screen_at(hConsole, last_pos.first, last_pos.second);
				kp.snake.pop_back();
			}

			SetConsoleCursorPosition(hConsole, { 1, 25 });
			continue;
		}

		if (key == 80 and !(kp.direction.load() == 'U' && kp.snake.size() >= 2))  // down 
		{
			int sposition_x = head_pos.first;
			int sposition_y = head_pos.second + 1;
			if (sposition_y > kp.game_size_y - 1)
				sposition_y = 0;
			COORD cpos = { adapt_x(sposition_x), adapt_y(sposition_y) };
			SetConsoleCursorPosition(hConsole, cpos);
			WriteConsole(hConsole, "#", 1, NULL, NULL);
			kp.snake.push_front(std::make_pair(sposition_x, sposition_y));
			head_pos = kp.snake.at(0);
			kp.direction.store('D');
			if (check_collision_fruit(head_pos, fr) != 1)
			{
				clear_screen_at(hConsole, last_pos.first, last_pos.second);
				kp.snake.pop_back();
			}

			SetConsoleCursorPosition(hConsole, { 1, 25 });
			continue;
		}

		if (key == 77 and !(kp.direction.load() == 'L' && kp.snake.size() >= 2)) // right
		{

			int sposition_x = head_pos.first+1;
			int sposition_y = head_pos.second;
			if (sposition_x > kp.game_size_x-1)
				sposition_x = 0;
			COORD cpos = { adapt_x(sposition_x), adapt_y(sposition_y) };
			SetConsoleCursorPosition(hConsole, cpos);
			WriteConsole(hConsole, "#", 1, NULL, NULL);
			kp.snake.push_front(std::make_pair(sposition_x, sposition_y));
			head_pos = kp.snake.at(0);
			kp.direction.store('R');
			if (check_collision_fruit(head_pos, fr) != 1)
			{
				clear_screen_at(hConsole, last_pos.first, last_pos.second);
				kp.snake.pop_back();
			}

			SetConsoleCursorPosition(hConsole, { 1, 25 });
			continue;
		}

		if (key == 75 and !(kp.direction.load() == 'R' && kp.snake.size() >= 2)) // left
		{
			int sposition_x = head_pos.first - 1;
			int sposition_y = head_pos.second;
			if (sposition_x < 0)
				sposition_x = kp.game_size_x - 1;
			COORD cpos = { adapt_x(sposition_x), adapt_y(sposition_y) };
			SetConsoleCursorPosition(hConsole, cpos);
			WriteConsole(hConsole, "#", 1, NULL, NULL);
			kp.snake.push_front(std::make_pair(sposition_x, sposition_y));
			head_pos = kp.snake.at(0);
			kp.direction.store('L');
			if (check_collision_fruit(head_pos, fr) != 1)
			{
				clear_screen_at(hConsole, last_pos.first, last_pos.second);
				kp.snake.pop_back();
			}

			SetConsoleCursorPosition(hConsole, { 1, 25 });
			continue;
		}
		
		generate_fruit(kp, fr);
		paste_fruit(hConsole, fr, kp);

		if (kp.direction.load()=='U')  // up
		{
			int sposition_x = head_pos.first;
			int sposition_y = head_pos.second - 1;
			if (sposition_y < 0)
				sposition_y = kp.game_size_y - 1;
			COORD cpos = { adapt_x(sposition_x), adapt_y(sposition_y) };
			SetConsoleCursorPosition(hConsole, cpos);
			WriteConsole(hConsole, "#", 1, NULL, NULL);
			kp.snake.push_front(std::make_pair(sposition_x, sposition_y));
			head_pos = kp.snake.at(0);
			kp.direction.store('U');
			if (check_collision_fruit(head_pos, fr) != 1)
			{
				clear_screen_at(hConsole, last_pos.first, last_pos.second);
				kp.snake.pop_back();
			}

			SetConsoleCursorPosition(hConsole, { 1, 25 });
			continue;
		}

		if (kp.direction.load() == 'D')  // down 
		{
			int sposition_x = head_pos.first;
			int sposition_y = head_pos.second + 1;
			if (sposition_y > kp.game_size_y - 1)
				sposition_y = 0;
			COORD cpos = { adapt_x(sposition_x), adapt_y(sposition_y) };
			SetConsoleCursorPosition(hConsole, cpos);
			WriteConsole(hConsole, "#", 1, NULL, NULL);
			kp.snake.push_front(std::make_pair(sposition_x, sposition_y));
			head_pos = kp.snake.at(0);
			kp.direction.store('D');
			if (check_collision_fruit(head_pos, fr) != 1)
			{
				clear_screen_at(hConsole, last_pos.first, last_pos.second);
				kp.snake.pop_back();
			}

			SetConsoleCursorPosition(hConsole, { 1, 25 });
			continue;
		}

		if (kp.direction.load() == 'R') // right
		{
			int sposition_x = head_pos.first + 1;
			int sposition_y = head_pos.second;
			if (sposition_x > kp.game_size_x-1)
				sposition_x = 0;
			COORD cpos = { adapt_x(sposition_x), adapt_y(sposition_y) };
			SetConsoleCursorPosition(hConsole, cpos);
			WriteConsole(hConsole, "#", 1, NULL, NULL);
			kp.snake.push_front(std::make_pair(sposition_x, sposition_y));
			head_pos = kp.snake.at(0);
			kp.direction.store('R');
			if (check_collision_fruit(head_pos, fr) != 1)
			{
				clear_screen_at(hConsole, last_pos.first, last_pos.second);
				kp.snake.pop_back();
			}

			SetConsoleCursorPosition(hConsole, { 1, 25 });
			continue;
		}

		if (kp.direction.load() == 'L') // left
		{
			int sposition_x = head_pos.first - 1;
			int sposition_y = head_pos.second;
			if (sposition_x < 0)
				sposition_x = kp.game_size_x - 1;
			COORD cpos = { adapt_x(sposition_x), adapt_y(sposition_y) };
			SetConsoleCursorPosition(hConsole, cpos);
			WriteConsole(hConsole, "#", 1, NULL, NULL);
			kp.snake.push_front(std::make_pair(sposition_x, sposition_y));
			head_pos = kp.snake.at(0);
			kp.direction.store('L');
			if (check_collision_fruit(head_pos, fr) != 1)
			{
				clear_screen_at(hConsole, last_pos.first, last_pos.second);
				kp.snake.pop_back();
			}

			SetConsoleCursorPosition(hConsole, { 1, 25 });
			continue;
		}
		
	}
	LOG(INFO) << "leaving 2nd thread";

	return 0;
}

int start_game(const HANDLE hConsole, functional_keys& fk)
{
	int sposition_x = std::rand() % fk.game_size_x;
	int sposition_y = std::rand() % fk.game_size_y;

	fk.snake.push_front(std::make_pair(sposition_x, sposition_y));

	cout << sposition_x << ";" << sposition_y << endl;
	COORD pos = { adapt_x(sposition_x), adapt_y(sposition_y) };
	SetConsoleCursorPosition(hConsole, pos);
	WriteConsole(hConsole, "#", 1, NULL, NULL);

	return 0;
}

int main()
{
	int size_h = 20;
	int size_v = 10;
	
	functional_keys kp;
	HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);

	// Load configuration from file
	el::Configurations conf("log.conf");
	// Reconfigure single logger
	el::Loggers::reconfigureLogger("default", conf);
	// Actually reconfigure all loggers instead
	el::Loggers::reconfigureAllLoggers(conf);
	// Now all the loggers will use configuration from file

	std::srand(std::time(nullptr));

	make_field(kp);

	cout << "Game size horizontal: " << size_h << "and vertical: " << size_v << endl;

	start_game(hConsole, kp);		

	thread t(read_key, std::ref(kp));
	thread t2(game_logic, std::ref(kp), hConsole);
	t.join();
	t2.join();	

	cout << "fin!";
	cin.get();
	return 0;
}