// src/events/Event.h
#pragma once

// Enum to define different types of events that can occur in the game
enum class EventType {
    PlayerMoved,  // Event indicating the player has moved
    // Add other event types (e.g., EnemySpotted, ItemCollected, GameOver)
};

// Event structure that holds the type of event and any additional data associated with the event
struct Event {
    EventType type;  // The type of the event
    // Additional event data can be added here (e.g., position, entity ID)
};

// Summary:
// The Event class defines a structure for events that can occur in the game, along with an EventType enum
// that categorizes different types of events. This system can be used for handling game logic like movement,
// item collection, or state changes in a decoupled manner.