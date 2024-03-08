//Uwaga! Co najmniej C++17!!!
//Project-> ... Properties->Configuration Properties->General->C++ Language Standard = ISO C++ 17 Standard (/std:c++17)

#include "SFML/Graphics.hpp"
#include <fstream>
#include <iostream>

enum class Field { VOID, FLOOR, WALL, BOX, PARK, PLAYER };




class Sokoban : public sf::Drawable
{
public:
	void LoadMapFromFile(std::string fileName);
	void draw(sf::RenderTarget& target, sf::RenderStates states) const;
	void SetDrawParameters(sf::Vector2u draw_area_size);
	void Move_Player_Left();
	void Move_Player_Right();
	void Move_Player_Up();
	void Move_Player_Down();
	bool Is_Victory() const;

	void SetSprites(); //wczytuje tekstury i ustawia sparajty 
	void SpritesResize();//ustawia wlasciwa wielkosc sprajtow
private:
	std::vector<std::vector<Field>> map;
	sf::Vector2f shift, tile_size;
	sf::Vector2i player_position;
	std::vector<sf::Vector2i> park_positions;

	sf::Vector2u base_size_of_texture;
	sf::Texture texture_void, texture_floor, texture_wall, texture_box, texture_park, texture_player;
	sf::Sprite sprite_void, sprite_floor, sprite_wall, sprite_box, sprite_park, sprite_player; //sparajty danych bloczkow

	void move_player(int dx, int dy);
};

void Sokoban::LoadMapFromFile(std::string fileName)
{
	std::string str;
	std::vector<std::string> vos;

	std::ifstream in(fileName.c_str());
	while (std::getline(in, str)) { vos.push_back(str); }
	in.close();

	map.clear();
	map.resize(vos.size(), std::vector<Field>(vos[0].size()));
	for (auto [row, row_end, y] = std::tuple{ vos.cbegin(), vos.cend(), 0 }; row != row_end; ++row, ++y)
		for (auto [element, end, x] = std::tuple{ row->begin(), row->end(), 0 }; element != end; ++element, ++x)
			switch (*element)
			{
			case 'X': map[y][x] = Field::WALL; break;
			case '*': map[y][x] = Field::VOID; break;
			case ' ': map[y][x] = Field::FLOOR; break;
			case 'B': map[y][x] = Field::BOX; break;
			case 'P': map[y][x] = Field::PARK; park_positions.push_back(sf::Vector2i(x, y));  break;
			case 'S': map[y][x] = Field::PLAYER; player_position = sf::Vector2i(x, y);  break;
			}
}

void Sokoban::draw(sf::RenderTarget& target, sf::RenderStates states) const
{
	// Tu niewątpliwie powinno coś być : -) Tu należy narysować wszystko. O tak jakoś :
	//target.draw(....);
	target.clear();
	//Przydatna może być pętla :
	for (unsigned int y = 0; y < map.size(); ++y)
		for (unsigned int x = 0; x < map[y].size(); ++x)
		{
			const sf::Vector2f position(x* tile_size.x + shift.x, y* tile_size.y + shift.y);
			sf::Sprite sprite;
			switch (map[y][x])
			{
			case Field::WALL: 
				sprite = sprite_wall;
				sprite.setPosition(position);
				target.draw(sprite, states);
				break;
			case Field::VOID: 
				sprite = sprite_void;
				sprite.setPosition(position);
				target.draw(sprite, states);
				break;
			case Field::FLOOR: 
				sprite = sprite_floor;
				sprite.setPosition(position);
				target.draw(sprite, states);
				break;
			case Field::BOX:
				sprite = sprite_box;
				sprite.setPosition(position);
				target.draw(sprite, states);
				break;
			case Field::PARK: 
				sprite = sprite_park;
				sprite.setPosition(position);
				target.draw(sprite, states);
				break;
			case Field::PLAYER: 
				sprite = sprite_player;
				sprite.setPosition(position);
				target.draw(sprite, states);
				break;
			}
			
			//Teraz map[y][x] mówi nam CO mamy narysować.
		}
}

void Sokoban::SetDrawParameters(sf::Vector2u draw_area_size)
{
	this->tile_size = sf::Vector2f(
		std::min(std::floor((float)draw_area_size.x / (float)map[0].size()), std::floor((float)draw_area_size.y / (float)map.size())),
		std::min(std::floor((float)draw_area_size.x / (float)map[0].size()), std::floor((float)draw_area_size.y / (float)map.size()))
	);
	this->shift = sf::Vector2f(
		((float)draw_area_size.x - this->tile_size.x * map[0].size()) / 2.0f,
		((float)draw_area_size.y - this->tile_size.y * map.size()) / 2.0f
	);
	SpritesResize();
}

void Sokoban::Move_Player_Left()
{
	move_player(-1, 0);
}

void Sokoban::Move_Player_Right()
{
	move_player(1, 0);
}

void Sokoban::Move_Player_Up()
{
	move_player(0, -1);
}

void Sokoban::Move_Player_Down()
{
	move_player(0, 1);
}

void Sokoban::move_player(int dx, int dy)
{
	bool allow_move = false; // Pesymistyczne załóżmy, że gracz nie może się poruszyć.

	if (player_position.x + dx >= 0 && player_position.y + dy >= 0)
	{
		sf::Vector2i new_pp(player_position.x + dx, player_position.y + dy); //Potencjalna nowa pozycja gracza.
		Field fts = map[new_pp.y][new_pp.x]; //Element na miejscu na które gracz zamierza przejść.

		//Gracz może się poruszyć jeśli pole na którym ma stanąć to podłoga lub miejsce na skrzynki.
		if (fts == Field::FLOOR || fts == Field::PARK) allow_move = true;


		if (new_pp.y + dy >= 0 && new_pp.x + dx >= 0)
		{
			Field ftsa = map[new_pp.y + dy][new_pp.x + dx]; //Element na miejscu ZA miejscem na które gracz zamierza przejść. :-D

			//Jeśli pole na które chce się poruszyć gracz zawiera skrzynkę to może się on poruszyć jedynie jeśli kolejne pole jest puste lub zawiera miejsce na skrzynkę  - bo wtedy może przepchnąć skrzynkę.
			if (fts == Field::BOX && (ftsa == Field::FLOOR || ftsa == Field::PARK))
			{
				allow_move = true;
				//Przepychamy skrzynkę.
				map[new_pp.y + dy][new_pp.x + dx] = Field::BOX;
				//Oczywiście pole na którym stała skrzynka staje się teraz podłogą.
				map[new_pp.y][new_pp.x] = Field::FLOOR;
			}
		}

		if (allow_move)
		{
			//Przesuwamy gracza.
			map[player_position.y][player_position.x] = Field::FLOOR;
			player_position = new_pp;
			map[player_position.y][player_position.x] = Field::PLAYER;
		}
	}	

	//Niestety w czasie ruchu mogły „ucierpieć” miejsca na skrzynkę. ;-(
	for (auto park_position : park_positions) if (map[park_position.y][park_position.x] == Field::FLOOR) map[park_position.y][park_position.x] = Field::PARK;
}

bool Sokoban::Is_Victory() const
{
	//Tym razem dla odmiany optymistycznie zakładamy, że gracz wygrał.
	//No ale jeśli na którymkolwiek miejscu na skrzynki nie ma skrzynki to chyba założenie było zbyt optymistyczne... : -/
	for (auto park_position : park_positions) if (map[park_position.y][park_position.x] != Field::BOX) return false;
	return true;
}

void Sokoban::SetSprites()
{
	
	texture_void.loadFromFile("void.png");
	texture_floor.loadFromFile("floor.png");
	texture_wall.loadFromFile("wall.png");
	texture_box.loadFromFile("box.png");
	texture_park.loadFromFile("park.png");
	texture_player.loadFromFile("player.png");
	sprite_void.setTexture(texture_void);
	sprite_floor.setTexture(texture_floor);
	sprite_wall.setTexture(texture_wall);
	sprite_box.setTexture(texture_box);
	sprite_park.setTexture(texture_park);
	sprite_player.setTexture(texture_player);
	base_size_of_texture = texture_floor.getSize();
}

void Sokoban::SpritesResize()
{
	sf::Vector2f scale = sf::Vector2f((tile_size.x / base_size_of_texture.x) , (tile_size.y / base_size_of_texture.y) );
	sprite_void.setScale(scale);
	sprite_floor.setScale(scale);
	sprite_wall.setScale(scale);
	sprite_box.setScale(scale);
	sprite_park.setScale(scale);
	sprite_player.setScale(scale);	
}

void Victory(sf::RenderWindow & window, sf::Time elapsed, sf::Font font, Sokoban& sokoban)
{
	float rec_width = window.getSize().x * 0.8f;
	float rec_high = window.getSize().y * 0.1f;
	sf::RectangleShape rectangle;
	rectangle.setSize(sf::Vector2f(rec_width, rec_high));
	rectangle.setFillColor(sf::Color::Black);
	rectangle.setOutlineColor(sf::Color::White);
	rectangle.setOutlineThickness(1.0f);
	rectangle.setPosition(window.getSize().x*0.5f- (rec_width*0.5), window.getSize().y * 0.5f- (rec_high*0.5));

	sf::Text vin_1;
	vin_1.setCharacterSize( rec_high/2 );
	vin_1.setFillColor(sf::Color::White);
	vin_1.setFont(font);
	vin_1.setString(L"Brawo, twój czas to:      ");
	float text_length = vin_1.findCharacterPos(26).x - vin_1.findCharacterPos(0).x;
	vin_1.setPosition(window.getSize().x * 0.5f - (text_length * 0.5), window.getSize().y * 0.5f - (rec_high * 0.3));
	window.draw(sokoban);
	window.draw(rectangle);
	window.draw(vin_1);

	sf::Text vin_2= vin_1;
	std::string vin_time = std::to_string((int)elapsed.asSeconds());
	vin_time += " s!";
	vin_2.setString(vin_time);
	vin_2.setPosition(vin_1.findCharacterPos(21));
	window.draw(vin_2);
	window.display();

	sf::Time time_to_read = sf::seconds(2.0);
	sf::sleep(time_to_read);
	window.close();
}

void Loss(sf::RenderWindow& window, sf::Font font, Sokoban& sokoban)
{
	float rec_width = window.getSize().x * 0.8f;
	float rec_high = window.getSize().y * 0.1f;
	sf::RectangleShape rectangle;
	rectangle.setSize(sf::Vector2f(rec_width, rec_high));
	rectangle.setFillColor(sf::Color::Black);
	rectangle.setOutlineColor(sf::Color::White);
	rectangle.setOutlineThickness(1.0f);
	rectangle.setPosition(window.getSize().x * 0.5f - (rec_width * 0.5), window.getSize().y * 0.5f - (rec_high * 0.5));

	sf::Text loss_text;
	loss_text.setCharacterSize(rec_high / 2);
	loss_text.setFillColor(sf::Color::White);
	loss_text.setFont(font);
	loss_text.setString(L"Tak łatwo się poddajesz?");
	float text_length = loss_text.findCharacterPos(24).x - loss_text.findCharacterPos(0).x;
	loss_text.setPosition(window.getSize().x * 0.5f - (text_length *0.5), window.getSize().y * 0.5f - (rec_high * 0.3));
	window.draw(sokoban);
	window.draw(rectangle);
	window.draw(loss_text);
	window.display();

	sf::Time time_to_read = sf::seconds(2.0);
	sf::sleep(time_to_read);
	window.close();
}

int main()
{
	sf::RenderWindow window(sf::VideoMode(800, 600), "GFK Lab 01", sf::Style::Titlebar | sf::Style::Close | sf::Style::Resize);
	Sokoban sokoban;

	sokoban.LoadMapFromFile("plansza.txt");
	sokoban.SetSprites();
	sokoban.SetDrawParameters(window.getSize());

	sf::Font font;
	font.loadFromFile("comic.ttf");

	sf::Text time_passed;
	time_passed.setFillColor(sf::Color::Red);
	time_passed.setCharacterSize(40);
	time_passed.setFont(font);
	
	sf::Clock clock;
	while (window.isOpen())
	{
		

		sf::Event event;

		while (window.pollEvent(event))
		{
			if (event.type == sf::Event::Resized)
			{
				float width = static_cast<float>(event.size.width);
				float height = static_cast<float>(event.size.height);
				window.setView(sf::View(sf::FloatRect(0, 0, width, height)));
				sokoban.SetDrawParameters(window.getSize());
			}
			if (event.type == sf::Event::Closed)
			{
				window.close();
			}
			if (event.type == sf::Event::KeyPressed)
			{
				switch (event.key.code)
				{
				case sf::Keyboard::Down:
					sokoban.Move_Player_Down();
					break;
				case sf::Keyboard::Up:
					sokoban.Move_Player_Up();
					break;
				case sf::Keyboard::Left:
					sokoban.Move_Player_Left();
					break;
				case sf::Keyboard::Right:
					sokoban.Move_Player_Right();
					break;
				case sf::Keyboard::Escape:
					Loss(window, font, sokoban);
					break;
				default:
					break;
				}
			}
			//Oczywiście tu powinny zostać jakoś obsłużone inne zdarzenia.Na przykład jak gracz naciśnie klawisz w lewo powinno pojawić się wywołanie metody :
			//sokoban.Move_Player_Left();
			//W dowolnym momencie mogą Państwo sprawdzić czy gracz wygrał:
			
		}
		window.draw(sokoban);
		
		time_passed.setString(std::to_string((int)clock.getElapsedTime().asSeconds()));
		window.draw(time_passed);

		window.display();


		if (sokoban.Is_Victory())
		{
			Victory(window, clock.getElapsedTime(), font, sokoban);
		}
	}

	return 0;
}
