// Train.cpp
#include "Train.h"
#include "Line.h"
#include <iostream>

/**
<summary>
Train class represents a train that moves along a specified line in the game world. It handles the train's movement,
direction changes, and wait times at the end points of the line.
</summary>
*/
Train::Train(Line* line)
    : line(line), 
    speed(10.0f), waitTime(2.0f), 
    progress(0.0f), currentWaitTime(0.0f), forward(true), isStopped(false),
    logTimer(0.0f)
{
    // Initialize the graphical representation of the train
    shape.setRadius(10.0f);
    shape.setFillColor(sf::Color::Black);
    shape.setOrigin(shape.getRadius(), shape.getRadius());
}

/**
<summary>
Updates the train's position along the line, changing direction and stopping at the endpoints if necessary.
</summary>
<param name="deltaTime">Time elapsed since the last frame.</param>
*/
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
            return;
        }
    }

    float lineLength = line->GetLength();
    float distanceToTravel = speed * deltaTime;
    float progressChange = distanceToTravel / lineLength;

    logTimer += deltaTime;
    if (logTimer >= 1.0f) {
        std::cout << "Progress towards current station: " << line->GetClosestStationProgress(progress) << std::endl;
        logTimer -= 1.0f;
    }

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

/**
<summary>
Renders the train on the screen at its current position along the line.
</summary>
<param name="window">Reference to the SFML RenderWindow where the train will be drawn.</param>
<param name="zoomLevel">Current zoom level of the camera, used to adjust the size of the train for better visibility.</param>
*/
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

/**
<summary>
Calculates the current position of the train along the line based on its progress.
</summary>
<returns>The current position of the train as a vector.</returns>
*/
sf::Vector2f Train::getPositionAlongLine() const
{
    if (!line) return sf::Vector2f();

    return line->GetPositionAlongLine(progress);
}
