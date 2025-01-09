// HandleManager.h
#pragma once

#include <vector>
#include <SFML/Graphics.hpp>

struct Handle {
    int index;         // Index of the node in the points vector
    bool isSelected;   // Selection state

    Handle(int idx = 0, bool selected = false)
        : index(idx), isSelected(selected) {}
};

class HandleManager {
public:
    HandleManager() = default;

    void AddHandle(int index) {
        handles.emplace_back(index, false);
    }

    void InsertHandle(int position, int index) {
        if (position < 0 || position > handles.size())
            return;
        handles.emplace(handles.begin() + position, Handle(index, false));
        UpdateHandleIndices(position + 1);
    }

    void SelectHandle(int index) {
        DeselectAll();
        for (auto& handle : handles) {
            if (handle.index == index) {
                handle.isSelected = true;
                break;
            }
        }
    }

    void DeselectAll() {
        for (auto& handle : handles) {
            handle.isSelected = false;
        }
    }

    int GetSelectedHandleIndex() const {
        for (const auto& handle : handles) {
            if (handle.isSelected) {
                return handle.index;
            }
        }
        return -1; // No handle selected
    }

    const std::vector<Handle>& GetHandles() const {
        return handles;
    }

private:
    std::vector<Handle> handles;

    void UpdateHandleIndices(int start) {
        for (size_t i = start; i < handles.size(); ++i) {
            handles[i].index = static_cast<int>(i);
        }
    }
};
