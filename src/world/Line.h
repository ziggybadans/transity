#pragma once
#include <string>
#include <SFML/Graphics.hpp>

#include "City.h"

struct BezierSegment {
	sf::Vector2f start;
	sf::Vector2f startControl;
	sf::Vector2f endControl;
	sf::Vector2f end;
};

class Line {
public:
	enum class Handle{ None, Start, End };

	Line(City* startCity, const std::string& lineName, const sf::Color& lineColor = sf::Color::Blue, float lineThickness = 4.0f)
		: name(lineName), color(lineColor), thickness(lineThickness), selected(false) {
		cities.push_back(startCity);
	}

    void AddCityToStart(City* city);
	void AddCityToEnd(City* city);
	const std::vector<City*>& GetCities() const { return cities; }

	void SetThickness(float newThickness) { thickness = newThickness; }
    void SetSelected(bool value) { selected = value; }
	std::string GetName() { return name; }
	sf::Color GetColor() const { return color; }
	float GetThickness() const { return thickness; }
    bool IsSelected() const { return selected; }

    std::vector<sf::Vector2f> GetPathPoints(int pointsPerSegment, std::vector<int>& cityIndices) const;
	const std::vector<BezierSegment>& GetBezierSegments() const { return bezierSegments; }
	sf::Vector2f GetStartPosition() const;
	sf::Vector2f GetEndPosition() const;
	void SetSelectedHandle(Handle handle) { selectedHandle = handle; }
	Handle GetSelectedHandle() const { return selectedHandle; }

private:
	std::vector<City*> cities;
	std::vector<BezierSegment> bezierSegments;
	std::string name;
	sf::Color color;
	float thickness;
    bool selected;
	Handle selectedHandle;

    std::vector<sf::Vector2f> ComputeCubicBezier(const BezierSegment& segment, int numPoints) const;
    std::vector<float> CalculateCumulativeArcLength(const std::vector<sf::Vector2f>& points) const;
    std::vector<sf::Vector2f> ResampleEquallySpaced(const std::vector<sf::Vector2f>& points, int numSamples) const;
	void UpdateBezierSegments();

    sf::Vector2f Normalize(const sf::Vector2f& vec) const {
        float length = std::sqrt(vec.x * vec.x + vec.y * vec.y);
        if (length != 0)
            return vec / length;
        else
            return sf::Vector2f(0.f, 0.f);
    }
};