#include <iostream>
#include <complex>
#include <vector>
#include <functional>
#include <memory>
#include <array>
#include <vector>

#include <sstream>

#include <UITools/UITools.h>
#include <utility/utility.h>
#include <Instrumentor/Instrumentor.h>

#include "Fourier.h"

complex f(const float& x)
{
	if (x < 0.5f)
		return 0.f;
	if (x == 0.5f)
		return 0.5;
	if (x > 0.5f)
		return 1.f;

	//return x - std::floor(x);

	//if (x < 0.25)
	//	return 4.f * x - 0.5f - 0.5i;
	//if (x < 0.5)
	//	return 0.5f + (x - 0.25f) * 4i - 0.5i;
	//if (x < 0.75)
	//	return 0.5f - (4.f * (x - 0.5f)) + 0.5i;
	//if (x <= 1)
	//	return -0.5f + (1i - (x - 0.75f) * 4i) - 0.5i;

	//if (x < 0.25)
	//	return 4.f * x;
	//if (x < 0.5)
	//	return 1.f + (x - 0.25f) * 4i;
	//if (x < 0.75)
	//	return 1.f - (4.f * (x - 0.5f)) + 1i;
	//if (x <= 1)
	//	return (1i - (x - 0.75f) * 4i);

	//return complex
	//(
	//	sinf(x * 2 * PI) * ( expf(cosf(x * 2 * PI)) - 2 * cosf(4.f * x * 2 * PI) - powf(sinf((x * 2 * PI) / 12.f), 5) ),
	//	cosf(x * 2 * PI) * ( expf(cosf(x * 2 * PI)) - 2 * cosf(4.f * x * 2 * PI) - powf(sinf((x * 2 * PI) / 12.f), 5) )
	//);
}

std::vector<complex> GetDataFromSVG(std::string path, float step)
{
	std::string cmd = "python loadSVG.py \"" + path + "\" " + std::to_string(step);

	std::string output;
	std::array<char, 128> buffer;

	std::unique_ptr<FILE, decltype(&_pclose)> pipe(_popen(cmd.c_str(), "r"), _pclose);

	while (fgets(buffer.data(), (int)buffer.size(), pipe.get()) != nullptr)
	{
		output += buffer.data();
	}

	std::stringstream ss(output);
	std::string line;

	std::vector<float> lines;
	lines.reserve((int)(2.f / step));
	while (std::getline(ss, line))
	{
		lines.push_back(std::stof(line));
	}

	std::vector<complex> func;
	func.reserve((int)(1.f / step));
	for (int i = 0; i < lines.size(); i += 2)
	{
		func.push_back(lines[i] + lines[(__int64)i + 1] * 1_i);
	}

	return func;
}

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

	std::cout << "Creating UI: ";
	t.Restart();
	sf::ContextSettings settings;
	settings.antialiasingLevel = 8;

	sf::Vector2f windowSize = { 600, 600 };

	sf::RenderWindow window({ (uint)windowSize.x, (uint)windowSize.y }, "Fourier", sf::Style::Default, settings);
	window.setFramerateLimit(60);


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

	// Load function from svg
	std::cout << "Loading svg: ";
	t.Restart();
	std::vector<complex> points = GetDataFromSVG("rsc/svgs/s.svg", 0.00001f);
	std::cout << t.GetElapsedTime<Timer::milliseconds>() << std::endl;

	// Create the fourier object
	std::cout << "Creating Fourier: ";
	t.Restart();
	Fourier four(points, 500);
	std::cout << t.GetElapsedTime<Timer::milliseconds>() << std::endl;

	std::cout << "Getting fourier points: ";
	t.Restart();
	// Add fourier function
	std::vector<sf::Vector2f> data;
	data.reserve(1.f / 0.0001f);
	for (float x = 0; x <= 1; x += 0.0001f)
	{
		complex y = four.get(x);
		//complex y = f(x);

		data.push_back({ y.real(), y.imag() });
		//data.push_back({ x, y.real() });
	}
	std::cout << t.GetElapsedTime<Timer::milliseconds>() << std::endl;

	std::cout << "Getting original points: ";
	t.Restart();
	// Add original function
	std::vector<sf::Vector2f> oData;
	oData.reserve((int)(points.size() / 100));
	for (int i = 0; i < points.size(); i++)
	{
		if (i % 100 == 0)
		{
			oData.push_back({ points[i].real(), points[i].imag() });
		}
	}
	oData.push_back({ points[0].real(), points[0].imag() });
	std::cout << t.GetElapsedTime<Timer::milliseconds>() << std::endl;
	std::cout << std::endl;

	// Draw functions
	//graph.Plot(oData, { 2, sf::Color::Black });
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