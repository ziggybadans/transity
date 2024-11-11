// Train.cpp
#include "Train.h"
#include "Line.h"
#include <iostream>

/**
<summary>
Train class represents a train that moves along a specified line in the game world. It handles the train's movement,
direction changes, and wait times at the stations along its line.
</summary>
*/
Train::Train(Line* line)
    : line(line),
    waitTime(2.0f),
    progress(0.0f), currentWaitTime(0.0f), forward(true), isStopped(false)
{
    // Initialize the graphical representation of the train
    shape.setRadius(10.0f);
    shape.setFillColor(sf::Color::Black);
    shape.setOrigin(shape.getRadius(), shape.getRadius());

    if (line) {
        stationProgressValues = line->GetStationProgressValues();

        if (!stationProgressValues.empty()) {
            progress = stationProgressValues[0]; // Start at first station
            currentStationIndex = 1; // Next station to go to
            forward = true;
        }
        else {
            // No stations on the line
            progress = 0.0f;
            currentStationIndex = -1;
            forward = true;
        }
    }
}

/**
<summary>
Updates the train's position along the line, stopping at each station in sequence.
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

            // Update currentStationIndex
            if (forward)
            {
                currentStationIndex++;
                if (currentStationIndex >= stationProgressValues.size())
                {
                    // Reverse direction
                    forward = false;
                    currentStationIndex = stationProgressValues.size() - 2; // Second last station
                }
            }
            else
            {
                currentStationIndex--;
                if (currentStationIndex < 0)
                {
                    // Reverse direction
                    forward = true;
                    currentStationIndex = 1; // Second station
                }
            }
        }
        else
        {
            return;
        }
    }

    if (currentStationIndex < 0 || currentStationIndex >= stationProgressValues.size())
    {
        return; // No valid station to go to
    }

    float targetProgress = stationProgressValues[currentStationIndex];

    // Move towards targetProgress
    float lineLength = line->GetLength();

    // Calculate speed in units per second
    float speedInUnitsPerSecond = line->GetSpeed() / (Constants::kmPerUnit * 3600.0f);

    float distanceToTravel = speedInUnitsPerSecond * deltaTime;
    float progressChange = distanceToTravel / lineLength;

    if (forward)
    {
        if (progress + progressChange >= targetProgress)
        {
            progress = targetProgress;
            isStopped = true;
        }
        else
        {
            progress += progressChange;
        }
    }
    else
    {
        if (progress - progressChange <= targetProgress)
        {
            progress = targetProgress;
            isStopped = true;
        }
        else
        {
            progress -= progressChange;
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
