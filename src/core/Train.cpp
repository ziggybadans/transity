// Train.cpp
#include "Train.h"
#include "Line.h"
#include <iostream>

Train::Train(Line* line)
    : line(line), speed(100.0f), progress(0.0f), forward(true),
    waitTime(2.0f), currentWaitTime(0.0f), isStopped(false)
{
    // Initialize the graphical representation of the train
    shape.setRadius(5.0f);
    shape.setFillColor(sf::Color::Green);
    shape.setOrigin(shape.getRadius(), shape.getRadius());
}

void Train::Update(float deltaTime)
{
    if (!line) return;

    if (isStopped)
    {
        currentWaitTime += deltaTime;
        if (currentWaitTime >= waitTime)
        {
            isStopped = false;
            currentWaitTime = 0.0f;
        }
        else
        {
            return; // Stay stopped until wait time is over
        }
    }

    // Move the train along the line
    float lineLength = line->GetLength();
    float distanceToTravel = speed * deltaTime;
    float progressChange = distanceToTravel / lineLength;

    if (forward)
    {
        progress += progressChange;
        if (progress >= 1.0f)
        {
            progress = 1.0f;
            forward = false;
            isStopped = true;
        }
    }
    else
    {
        progress -= progressChange;
        if (progress <= 0.0f)
        {
            progress = 0.0f;
            forward = true;
            isStopped = true;
        }
    }
}

void Train::Render(sf::RenderWindow& window, float zoomLevel) const
{
    if (!line) return;

    sf::Vector2f position = getPositionAlongLine();
    shape.setPosition(position);

    // Adjust size based on zoom level
    float baseRadius = 5.0f;
    float scaledRadius = baseRadius * zoomLevel;
    shape.setRadius(scaledRadius);
    shape.setOrigin(scaledRadius, scaledRadius);

    window.draw(shape);
}

sf::Vector2f Train::getPositionAlongLine() const
{
    if (!line) return sf::Vector2f();

    return line->GetPositionAlongLine(progress);
}
