#if 1

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
	struct CurveData
	{
		ui::Vec2f p0, p1, p2, p3;
		float length;
	};
	
	std::vector<CurveData> m_curves;

	int m_nSegments;
	float m_length = 0.f;

private:
	ui::Vec2f GetCurve(float x, const CurveData& curve) const
	{
		// p0 * (1 - t)^3 + p1 + 3 * t * (1 - t)^2 + p2 * 3 * t^2 * (1 - t) + p3 * t^3
		return	curve.p0 * powf(1 - x, 3) +
				curve.p1 * 3.f * powf(1 - x, 2) * x +
				curve.p2 * 3.f * (1 - x) * powf(x, 2) +
				curve.p3 * powf(x, 3);
	}

	float CurveLength(const CurveData& curve, int iterations = 5)
	{
		float length = 0;

		float delta = 1.f / (float)iterations;
		for (int i = 1; i < iterations; i++)
		{
			ui::Vec2f iPos = GetCurve((i - 1) * delta, curve);
			ui::Vec2f fPos = GetCurve(i * delta, curve);

			length += (fPos - iPos).Length();
		}

		return length;
	}

public:
	SVGPath(const std::string& path)
	{
		std::ifstream ifs(path);
		std::string str((std::istreambuf_iterator<char>(ifs)), (std::istreambuf_iterator<char>()));

		std::replace(str.begin(), str.end(), '\n', ' ');
		std::replace(str.begin(), str.end(), ',', ' ');


		int mDelim = (int)str.find(std::string("M"));
		int cDelim = (int)str.find(std::string("C"));
		int zDelim = (int)str.find(std::string("Z"));

		std::string mStr = str.substr(mDelim + 1, cDelim - mDelim - 1);
		std::string cStr = str.substr(cDelim + 1, zDelim - cDelim - 1);

		std::vector<ui::Vec2f> points;

		// Split mStr (Start pos)
		std::stringstream mStrSS(mStr);
		std::vector<std::string> mData((std::istream_iterator<std::string>(mStrSS)), std::istream_iterator<std::string>());

		points.push_back({ std::stof(mData[0]), std::stof(mData[1]) });

		// Split cStr (Path data)
		std::stringstream cStrSS(cStr);
		std::vector<std::string> cData = std::vector((std::istream_iterator<std::string>(cStrSS)), std::istream_iterator<std::string>());

		for (int i = 0; i < cData.size(); i += 2)
		{
			points.push_back({ std::stof(cData[i]), std::stof(cData[i + 1]) });
		}

		m_nSegments = (int)(points.size() - 1) / 3;

		for (int i = 0; i < m_nSegments; i++)
		{
			m_curves.push_back({
				points[i * 3 + 0],
				points[i * 3 + 1],
				points[i * 3 + 2],
				points[i * 3 + 3]
			});
			m_curves.back().length = CurveLength(m_curves.back(), 20);
			m_length += m_curves.back().length;
		}
	}

	ui::Vec2f get(const float& t, bool constSpeed = true) const
	{
		float val = t;
		while (val > 1)
			val -= 1;


		float x = 0;
		int segment = 0;

		if (constSpeed)
		{
			float iPos = 0.f;
			float fPos = 0.f;
			
			for (segment = 0; segment < m_nSegments; segment++)
			{
				fPos += m_curves[segment].length / m_length;
				iPos = fPos - m_curves[segment].length / m_length;

				if (fPos > val)
					break;
			}

			x = map(val, iPos, fPos, 0.f, 1.f);
		}
		else
		{
			segment = std::min((int)std::floor(m_nSegments * val), m_nSegments - 1);
			x = map(val, segment / (float)m_nSegments, (segment + 1) / (float)m_nSegments, 0.f, 1.f);
		}
		

		return GetCurve(x, m_curves[segment]);
	}
};

enum class CameraState
{
	Free,
	Chase
};


int main()
{
	Timer t;

	CameraState state = CameraState::Free;
	bool animation = true;

	// Current x
	float x = 0.f;

	// Current velocity
	float vel = 0.01f;

	// Initialize SFML window
	sf::ContextSettings settings;
	settings.antialiasingLevel = 8;

	ui::Vec2f windowSize = { 600, 600 };

	sf::RenderWindow window({ (uint)windowSize.x, (uint)windowSize.y }, "Fourier", sf::Style::Default, settings);
	window.setFramerateLimit(60);

	std::cout << "Loadingn svg: ";
	t.Restart();
	SVGPath svg("rsc/e2.dat");
	std::cout << t.GetElapsedTime<Timer::milliseconds>() << std::endl;

	// Create the fourier object
	std::cout << "Creating Fourier: ";
	t.Restart();
	auto f = [&](const float& t) -> complex
	{
		ui::Vec2f out = svg.get(t, true);
		return complex
		{
			out.x, out.y
		};
	};

	Fourier four(f, 500);
	std::cout << t.GetElapsedTime<Timer::milliseconds>() << std::endl;


	std::cout << "Getting fourier points: ";
	t.Restart();
	// Add fourier function
	std::vector<ui::Vec2f> data;
	data.reserve((int)(1.f / 0.0001f));
	for (float x = 0; x < 1; x += 0.0001f)
	{
		complex y = four.get(x);

		data.push_back({ y.real(), y.imag() });
	}
	std::cout << t.GetElapsedTime<Timer::milliseconds>() << std::endl;


	std::cout << "Getting original fourier points: ";
	t.Restart();
	// Add fourier function
	std::vector<ui::Vec2f> oData;
	data.reserve((int)(1.f / 0.0001f));
	for (float x = 0; x <= 1; x += 0.0001f)
	{
		complex y = f(x);

		oData.push_back({ y.real(), y.imag() });
	}
	std::cout << t.GetElapsedTime<Timer::milliseconds>() << std::endl;


	std::cout << "Creating UI: ";
	t.Restart();
	// Create graph
	ui::InteractiveGraph graph("g");
	graph.SetSize(windowSize);
	graph.SetPosition({ 0, 0 });

	graph.Plot(oData, { 1, { 50, 50, 50 }, true });
	graph.Plot(data, { 2, { 150, 150, 150 }, true });

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

		state = pressed ? CameraState::Free : CameraState::Chase;
	});
	stateButton.SetPressed(true);

	ui.AddObject(&stateButton);


	// Show original button
	ui::ToggleButton showOriginal("state");
	showOriginal.shape.setPosition({ 160, 20 });
	showOriginal.shape.setSize({ 140, 40 });
	showOriginal.shape.setFillColor(sf::Color::White);

	showOriginal.text.setFont(font);
	showOriginal.text.setCharacterSize(15);

	showOriginal.SetClickFunction([&](ui::UIObject* obj, bool pressed)
	{
		auto self = dynamic_cast<ui::ToggleButton*>(obj);

		self->text.setString(pressed ? "Hide Original" : "Show Original");

		graph.ClearPlots();

		graph.Plot(data, { 2, { 150, 150, 150 }, true });
		if (pressed)
			graph.Plot(oData, { 1, { 50, 50, 50 }, true });
	});
	showOriginal.SetPressed(false);

	ui.AddObject(&showOriginal);

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
	//ui::Text speedText("speedText");
	//speedText.setFont(font);

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
				windowSize = (ui::Vec2f)window.getSize();

				sf::View view = window.getView();
				view.setSize(windowSize);
				view.setCenter(windowSize / 2.f);
				window.setView(view);

				graph.SetSize(windowSize);
				graph.SetPosition({ 0, 0 });
			}

			graph.CheckInput(window, e);
		}

		window.clear({ 100, 100, 100 });

		graph.ClearArrows();

		// Draw arrows
		complex pos = 0;
		for (const auto& [n, c] : four.GetCoeffs())
		{
			complex vec = c * std::exp(n * 2.f * (float)PI * 1_i * x);

			graph.Arrow({ pos.real(), pos.imag() }, { vec.real(), vec.imag() }, { 2.5, { 70, 70, 200 } });

			pos += vec;
		}


		graph.Update(window);

		if (state == CameraState::Chase)
		{
			graph.SetCenter({ pos.real(), pos.imag() });
		}

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

#endif