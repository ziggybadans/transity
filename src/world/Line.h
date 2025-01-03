#pragma once
#include <string>
#include <list>
#include <SFML/Graphics.hpp>

#include "City.h"

class Train;

struct BezierSegment {
	sf::Vector2f start;
	sf::Vector2f startControl;
	sf::Vector2f endControl;
	sf::Vector2f end;
};

struct LinePoint {
	bool isCity;             // True if it represents a city, false if it's a node
	sf::Vector2f position;   // The world position of the city or node
	City* city;              // Pointer to a City if isCity == true, otherwise nullptr
};

struct Handle {
	int index; // Index of the node in the points vector
	bool isSelected;

	Handle(int idx) : index(idx), isSelected(false) {}
};

class Line {
public:
	Line(City* startCity, const std::string& lineName,
		const sf::Color& lineColor = sf::Color::Blue, float lineThickness = 4.0f)
		: name(lineName), color(lineColor), thickness(lineThickness), selected(false) {

		LinePoint p;
		p.isCity = true;
		p.position = startCity->position;
		p.city = startCity;
		points.push_back(p);

		handles.emplace_back(0);
	}

    void AddCityToStart(City* city);
	void AddCityToEnd(City* city);
	void InsertCityAfter(int index, City* city);
	const std::vector<City*> GetCities() const;
	const std::vector<LinePoint>& GetPoints() const { return points; }
	void AddNode(sf::Vector2f pos);

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
	sf::Vector2f GetPointPosition(int index) const;

	void SelectHandle(int index);
	void DeselectHandles();
	int GetSelectedHandleIndex() const;
	std::vector<Handle> GetHandles() const { return handles; }
	void MoveHandle(int index, sf::Vector2f newPos);

	void AddTrain(Train* train) { trains.emplace_back(train); }
	bool HasTrains() { return trains.empty(); }

private:
	std::vector<LinePoint> points;
	std::vector<BezierSegment> bezierSegments;
	std::vector<Handle> handles;
	std::vector<Train*> trains;
	std::string name;
	sf::Color color;
	float thickness;
    bool selected;

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