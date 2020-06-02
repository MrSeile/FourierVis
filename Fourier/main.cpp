#include <iostream>
#include <complex>
#include <vector>
#include <functional>
#include <memory>
#include <array>
#include <vector>

#include <UITools/UITools.h>
#include <utility/utility.h>

#include "Fourier.h"


class SVGPath
{
private:
	std::vector<sf::Vector2f> points;

public:
	SVGPath(const std::string& path)
	{
		std::ifstream ifs(path);
		std::string str((std::istreambuf_iterator<char>(ifs)), (std::istreambuf_iterator<char>()));

		std::replace(str.begin(), str.end(), '\n', ' ');
		std::replace(str.begin(), str.end(), ',', ' ');


		int mDelim = str.find(std::string("M"));
		int cDelim = str.find(std::string("C"));
		int zDelim = str.find(std::string("Z"));

		std::string mStr = str.substr(mDelim + 1, cDelim - mDelim - 1);
		std::string cStr = str.substr(cDelim + 1, zDelim - cDelim - 1);


		// Split mStr
		std::stringstream mStrSS(mStr);
		std::vector<std::string> mData((std::istream_iterator<std::string>(mStrSS)), std::istream_iterator<std::string>());

		points.push_back({ std::stof(mData[0]), std::stof(mData[1]) });

		// Split cStr
		std::stringstream cStrSS(cStr);
		std::vector<std::string> cData = std::vector((std::istream_iterator<std::string>(cStrSS)), std::istream_iterator<std::string>());

		for (int i = 0; i < cData.size(); i += 2)
		{
			points.push_back({ std::stof(cData[i]), std::stof(cData[i + 1]) });
		}
	}

	sf::Vector2f get(const float& t) const
	{
		// 3 values per curve
		int nSegm = (points.size() - 1) / 3;

		int segment = std::min(std::floorf(nSegm * t), nSegm - 1.f);

		float x = map(t, segment / (float)nSegm, (segment + 1) / (float)nSegm, 0.f, 1.f);


		sf::Vector2f p0 = points[segment * 3];
		sf::Vector2f p1 = points[segment * 3 + 1];
		sf::Vector2f p2 = points[segment * 3 + 2];
		sf::Vector2f p3 = points[segment * 3 + 3];

		// p0 * (1 - t)^3 + p1 + 3 * t * (1 - t)^2 + p2 * 3 * t^2 * (1 - t) + p3 * t^3
		return powf(1 - x, 3) * p0 + 3 * powf(1 - x, 2) * x * p1 + 3 * (1 - x) * powf(x, 2) * p2 + powf(x, 3) * p3;
	}
};

enum class State
{
	Free,
	Chase
};


int main()
{
	Timer t;

	State state = State::Free;
	bool animation = true;
	float x = 0.f;
	float vel = 0.01f;

	sf::ContextSettings settings;
	settings.antialiasingLevel = 8;

	sf::Vector2f windowSize = { 600, 600 };

	sf::RenderWindow window({ (uint)windowSize.x, (uint)windowSize.y }, "Fourier", sf::Style::Default, settings);
	window.setFramerateLimit(60);


	std::cout << "Creating UI: ";
	t.Restart();

	// Create graph
	ui::Graph graph = ui::Graph("g");
	graph.SetSize(windowSize);
	graph.SetPosition({ 0, 0 });

	sf::Font font;
	font.loadFromFile("rsc/fonts/font.ttf");

	// Create widget
	ui::Widget ui;

	// State toggle button
	ui::ToggleButton stateButton("state");
	stateButton.shape.setPosition({ 20, 20 });
	stateButton.shape.setSize({ 120, 40 });
	stateButton.shape.setFillColor(sf::Color::White);

	stateButton.text.setFont(font);
	stateButton.text.setCharacterSize(15);

	stateButton.SetClickFunction([&](ui::UIObject* obj, bool pressed)
	{
		auto self = dynamic_cast<ui::ToggleButton*>(obj);

		self->text.setString(pressed ? "Chase cam" : "Free cam");

		state = pressed ? State::Free : State::Chase;
	});
	stateButton.SetPressed(false);

	ui.AddObject(&stateButton);

	// Animation toggle button
	ui::ToggleButton animationButton("animation");
	animationButton.shape.setPosition({ 20, 80 });
	animationButton.shape.setSize({ 120, 40 });
	animationButton.shape.setFillColor(sf::Color::White);

	animationButton.text.setFont(font);
	animationButton.text.setCharacterSize(15);

	animationButton.SetClickFunction([&](ui::UIObject* obj, bool pressed)
	{
		auto self = dynamic_cast<ui::ToggleButton*>(obj);

		self->text.setString(pressed ? "No animation" : "Animation");

		animation = pressed;
	});
	animationButton.SetPressed(true);

	ui.AddObject(&animationButton);

	// Speed slider
	ui::Text speedText("speedText");
	speedText.setFont(font);

	ui::Slider speedSlider("speed", font);
	speedSlider.SetPosition({ 20, 140 });
	speedSlider.SetSize({ 400, 20 });
	speedSlider.SetStep(0.0001f);
	speedSlider.ShowValue(true);
	speedSlider.SetRange({ 0, 1 });
	speedSlider.GetText().setFillColor(sf::Color::Black);
	speedSlider.SetValue(vel * 50.f);

	speedSlider.SetUpdateFunction([&](ui::UIObject* obj)
	{
		auto self = dynamic_cast<ui::Slider*>(obj);

		vel = self->GetValue() / 50.f;
	});

	ui.AddObject(&speedSlider);

	// Position slider
	ui::Slider posSlider("pos", font);
	posSlider.SetPosition({ 20, 200 });
	posSlider.SetSize({ 400, 20 });
	posSlider.SetStep(0.0001f);
	posSlider.ShowValue(true);
	posSlider.SetRange({ 0, 1 });
	posSlider.GetText().setFillColor(sf::Color::Black);
	posSlider.SetValue(0);

	posSlider.SetUpdateFunction([&](ui::UIObject* obj)
	{
		auto self = dynamic_cast<ui::Slider*>(obj);

		if (animation)
			self->SetValue(x);

		else
			x = self->GetValue();
	});

	ui.AddObject(&posSlider);
	std::cout << t.GetElapsedTime<Timer::milliseconds>() << std::endl;


	std::cout << "Loadingn svg: ";
	t.Restart();
	SVGPath svg("rsc/e.dat");

	auto f = [&](const float& t) -> complex
	{
		sf::Vector2f out = svg.get(t);
		return complex
		{
			out.x, out.y
		};
	};
	std::cout << t.GetElapsedTime<Timer::milliseconds>() << std::endl;

	// Create the fourier object
	std::cout << "Creating Fourier: ";
	t.Restart();
	Fourier four(f, 200);
	std::cout << t.GetElapsedTime<Timer::milliseconds>() << std::endl;

	std::cout << "Getting fourier points: ";
	t.Restart();
	// Add fourier function
	std::vector<sf::Vector2f> data;
	data.reserve((int)(1.f / 0.0001f));
	for (float x = 0; x <= 1; x += 0.0001f)
	{
		complex y = four.get(x);
		//complex y = f(x);

		data.push_back({ y.real(), y.imag() });
		//data.push_back({ x, y.real() });
	}
	std::cout << t.GetElapsedTime<Timer::milliseconds>() << std::endl;

	graph.Plot(data, { 2, { 150, 150, 150 } });

	// Main loop
	sf::Vector2f startPos;

	float zoom = 2.f;
	sf::Vector2f center(0, 0);
	sf::Vector2f view(-zoom, zoom);
	bool mousePressed = false;
	float aspect = windowSize.x / windowSize.y;
	
	while (window.isOpen())
	{
		ui::Event e;
		while (window.pollEvent(e))
		{
			ui.CheckInput(window, e);

			if (e.type == sf::Event::Closed)
				window.close();

			if (e.type == sf::Event::Resized)
			{
				windowSize = (sf::Vector2f)window.getSize();

				sf::View view = window.getView();
				view.setSize(windowSize);
				view.setCenter(windowSize / 2.f);
				window.setView(view);

				aspect = windowSize.x / windowSize.y;
				graph.SetSize(windowSize);
			}

			if (e.type == sf::Event::MouseWheelMoved)
			{
				switch (e.mouseWheel.delta)
				{
				case 1:
					zoom /= 1.2f;
					break;

				case -1:
					zoom *= 1.2f;
					break;

				default:
					break;
				}

				view = { -zoom, zoom };
			}

			if (e.type == sf::Event::MouseButtonPressed && e.key.code == sf::Mouse::Left && !e.handled)
			{
				mousePressed = true;
				startPos = (sf::Vector2f)sf::Mouse::getPosition(window);
			}

			if (e.type == sf::Event::MouseButtonReleased && e.key.code == sf::Mouse::Left)
			{
				mousePressed = false;
			}
		}

		window.clear();
		
		graph.ClearArrows();

		// Draw arrows
		complex pos = 0;
		for (const auto& [n, c] : four.GetCoefs())
		{
			complex vec = c * std::exp(n * 2.f * (float)PI * 1_i * x);

			graph.Arrow({ pos.real(), pos.imag() }, { vec.real(), vec.imag() }, { 2.5, { 70, 70, 200 } });
			
			pos += vec;
		}

		// Manage camera
		switch (state)
		{
		case State::Free:
			if (mousePressed)
			{
				sf::Vector2f mousePos = graph.MapCoordsToPos((sf::Vector2f)sf::Mouse::getPosition(window));
				sf::Vector2f delta = graph.MapCoordsToPos(startPos) - mousePos;
				center += delta;

				startPos = (sf::Vector2f)sf::Mouse::getPosition(window);
			}
			break;

		case State::Chase:
			center = { pos.real(), pos.imag() };
			break;
		}

		graph.SetRange(sf::Vector2f(center.x, center.x) + view * aspect, sf::Vector2f(center.y, center.y) + view);
		
		ui.Update(window);

		// Draw
		graph.Draw(window);
		ui.Draw(window);

		window.display();

		if (animation)
		{
			x += vel * (1 / 60.f);
			if (x > 1)
				x = 0;
		}
	}
}

