#include "SpatialIndex.h"
#include "../Debug.h"
#include <algorithm>
#include <queue>
#include <unordered_set>
template<typename T>
void SpatialIndex<T>::InsertInternal(const T& element, const BoundingBox& bounds) {
    auto startTime = std::chrono::high_resolution_clock::now();

    try {
        // If root doesn't exist, create it and add element directly
        if (!m_root) {
            m_root = std::make_unique<RTreeNode<T>>();
            m_root->m_entries.emplace_back(bounds, element);
            m_root->m_bounds = bounds;
            
            DEBUG_DEBUG("Created root node and inserted first element");
            return;
        }

        // Choose the best leaf node for insertion
        RTreeNode<T>* leafNode = ChooseSubtree(bounds);
        
        // Add element to chosen leaf
        leafNode->m_entries.emplace_back(bounds, element);

        // Update bounds of the leaf node
        std::vector<BoundingBox> leafBoxes;
        for (const auto& entry : leafNode->m_entries) {
            leafBoxes.push_back(entry.first);
        }
        leafNode->m_bounds = CalculateBounds(leafBoxes);

        // Check if node needs splitting
        if (leafNode->m_entries.size() > RTreeNode<T>::MAX_ENTRIES) {
            DEBUG_DEBUG("Node overflow, performing split");
            SplitNode(leafNode);
            
            // Update bounds of all affected nodes
            RTreeNode<T>* current = leafNode;
            while (current != m_root.get()) {
                // Find parent node
                RTreeNode<T>* parent = nullptr;
                std::function<void(RTreeNode<T>*)> findParent = [&](RTreeNode<T>* node) {
                    if (!node) return;
                    for (const auto& child : node->m_children) {
                        if (child.get() == current) {
                            parent = node;
                            return;
                        }
                        if (!child->IsLeaf()) {
                            findParent(child.get());
                        }
                    }
                };
                findParent(m_root.get());
                
                if (!parent) break;

                // Update parent bounds
                std::vector<BoundingBox> childBoxes;
                for (const auto& child : parent->m_children) {
                    if (child) {
                        childBoxes.push_back(child->GetBounds());
                    }
                }
                parent->m_bounds = CalculateBounds(childBoxes);
                
                current = parent;
            }
        }

        // Calculate and log performance metrics
        auto endTime = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime);
        
        DEBUG_DEBUG("Inserted element in ", duration.count() / 1000.0, "ms");
    }
    catch (const std::exception& e) {
        DEBUG_ERROR("Failed to insert element: ", e.what());
        throw; // Re-throw after logging
    }
}

template<typename T>
void SpatialIndex<T>::BulkLoadInternal(const std::vector<std::pair<T, BoundingBox>>& elements) {
    auto startTime = std::chrono::high_resolution_clock::now();

    try {
        if (elements.empty()) {
            DEBUG_WARNING("Attempting to bulk load empty element set");
            return;
        }

        DEBUG_DEBUG("Starting bulk load of ", elements.size(), " elements");

        // Sort elements by their center latitude (could be improved with Hilbert curve)
        std::vector<std::pair<T, BoundingBox>> sortedElements = elements;
        std::sort(sortedElements.begin(), sortedElements.end(),
            [](const auto& a, const auto& b) {
                double centerA = (a.second.GetMinLat() + a.second.GetMaxLat()) / 2.0;
                double centerB = (b.second.GetMinLat() + b.second.GetMaxLat()) / 2.0;
                return centerA < centerB;
            });

        // Calculate optimal number of leaf nodes
        size_t totalElements = sortedElements.size();
        size_t elementsPerNode = RTreeNode<T>::MAX_ENTRIES;
        size_t numLeafNodes = (totalElements + elementsPerNode - 1) / elementsPerNode;

        DEBUG_DEBUG("Creating ", numLeafNodes, " leaf nodes");

        // Create leaf nodes
        std::vector<std::unique_ptr<RTreeNode<T>>> leafNodes;
        leafNodes.reserve(numLeafNodes);

        size_t elementIndex = 0;
        while (elementIndex < totalElements) {
            auto leafNode = std::make_unique<RTreeNode<T>>();
            leafNode->m_isLeaf = true;

            // Fill node with elements
            size_t count = std::min(elementsPerNode, totalElements - elementIndex);
            for (size_t i = 0; i < count; ++i) {
                leafNode->m_entries.push_back(sortedElements[elementIndex + i]);
            }

            // Calculate bounds for the leaf node
            std::vector<BoundingBox> leafBoxes;
            for (const auto& entry : leafNode->m_entries) {
                leafBoxes.push_back(entry.first);
            }
            leafNode->m_bounds = CalculateBounds(leafBoxes);

            leafNodes.push_back(std::move(leafNode));
            elementIndex += count;
        }

        // Build non-leaf levels bottom-up
        std::vector<std::unique_ptr<RTreeNode<T>>> currentLevel = std::move(leafNodes);
        
        while (currentLevel.size() > 1) {
            std::vector<std::unique_ptr<RTreeNode<T>>> nextLevel;
            size_t nodesPerParent = RTreeNode<T>::MAX_ENTRIES;
            size_t numParentNodes = (currentLevel.size() + nodesPerParent - 1) / nodesPerParent;
            nextLevel.reserve(numParentNodes);

            DEBUG_DEBUG("Creating ", numParentNodes, " internal nodes");

            size_t nodeIndex = 0;
            while (nodeIndex < currentLevel.size()) {
                auto parentNode = std::make_unique<RTreeNode<T>>();
                parentNode->m_isLeaf = false;

                // Add child nodes to parent
                size_t count = std::min(nodesPerParent, currentLevel.size() - nodeIndex);
                for (size_t i = 0; i < count; ++i) {
                    size_t childIndex = nodeIndex + i;
                    parentNode->m_children[i] = std::move(currentLevel[childIndex]);
                }

                // Calculate bounds for the parent node
                std::vector<BoundingBox> parentBoxes;
                for (const auto& child : parentNode->m_children) {
                    if (child) {
                        parentBoxes.push_back(child->GetBounds());
                    }
                }
                parentNode->m_bounds = CalculateBounds(parentBoxes);

                nextLevel.push_back(std::move(parentNode));
                nodeIndex += count;
            }

            currentLevel = std::move(nextLevel);
        }

        // Set the root
        m_root = std::move(currentLevel[0]);

        // Calculate and log performance metrics
        auto endTime = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime);

        DEBUG_INFO("Bulk load completed: ",
                  elements.size(), " elements, ",
                  numLeafNodes, " leaf nodes, ",
                  "in ", duration.count() / 1000.0, "ms");
    }
    catch (const std::exception& e) {
        DEBUG_ERROR("Bulk load failed: ", e.what());
        throw; // Re-throw after logging
    }
}

template<typename T>
std::vector<T> SpatialIndex<T>::QueryRangeInternal(const BoundingBox& bounds) const {
    auto startTime = std::chrono::high_resolution_clock::now();
    std::vector<T> results;

    try {
        // Return empty result if root is null
        if (!m_root) {
            DEBUG_DEBUG("Query on empty index");
            return results;
        }

        // Stack for iterative traversal (more efficient than recursion)
        std::vector<const RTreeNode<T>*> stack;
        stack.push_back(m_root.get());

        while (!stack.empty()) {
            const RTreeNode<T>* current = stack.back();
            stack.pop_back();

            // Skip if node's bounds don't intersect query bounds
            if (!current->GetBounds().Intersects(bounds)) {
                continue;
            }

            if (current->IsLeaf()) {
                // For leaf nodes, check each entry
                for (const auto& entry : current->m_entries) {
                    if (entry.first.Intersects(bounds)) {
                        results.push_back(entry.second);
                    }
                }
            } else {
                // For internal nodes, add all non-null children to stack
                for (const auto& child : current->m_children) {
                    if (child) {
                        stack.push_back(child.get());
                    }
                }
            }
        }

        // Calculate and log performance metrics
        auto endTime = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime);

        DEBUG_DEBUG("Range query completed: ",
                   "found ", results.size(), " elements ",
                   "in ", duration.count() / 1000.0, "ms ",
                   "bounds: (", bounds.GetMinLat(), ",", bounds.GetMinLon(), ") to (",
                   bounds.GetMaxLat(), ",", bounds.GetMaxLon(), ")");

        return results;
    }
    catch (const std::exception& e) {
        DEBUG_ERROR("Range query failed: ", e.what());
        throw; // Re-throw after logging
    }
}

template<typename T>
BoundingBox SpatialIndex<T>::CalculateBounds(const std::vector<BoundingBox>& boxes) {
    if (boxes.empty()) {
        DEBUG_WARNING("Calculating bounds for empty box set");
        return BoundingBox(0, 0, 0, 0);
    }

    try {
        // Initialize bounds with first box
        double minLat = boxes[0].GetMinLat();
        double maxLat = boxes[0].GetMaxLat();
        double minLon = boxes[0].GetMinLon();
        double maxLon = boxes[0].GetMaxLon();

        // Expand bounds to include all other boxes
        for (size_t i = 1; i < boxes.size(); ++i) {
            const auto& box = boxes[i];
            
            // Update minimum bounds
            minLat = std::min(minLat, box.GetMinLat());
            minLon = std::min(minLon, box.GetMinLon());
            
            // Update maximum bounds
            maxLat = std::max(maxLat, box.GetMaxLat());
            maxLon = std::max(maxLon, box.GetMaxLon());
        }

        // Validate final bounds
        if (minLat > maxLat || minLon > maxLon) {
            DEBUG_ERROR("Invalid bounds calculated: ",
                       "Lat(", minLat, ", ", maxLat, "), ",
                       "Lon(", minLon, ", ", maxLon, ")");
            throw std::runtime_error("Invalid bounds calculated");
        }

        DEBUG_DEBUG("Calculated bounds for ", boxes.size(), " boxes: ",
                   "Lat(", minLat, ", ", maxLat, "), ",
                   "Lon(", minLon, ", ", maxLon, ")");

        return BoundingBox(minLat, maxLat, minLon, maxLon);
    }
    catch (const std::exception& e) {
        DEBUG_ERROR("Error calculating bounds: ", e.what());
        throw; // Re-throw after logging
    }
}

template<typename T>
void SpatialIndex<T>::SplitNode(RTreeNode<T>* node) {
    DEBUG_DEBUG("Splitting node with ", node->m_entries.size(), " entries");
    auto startTime = std::chrono::high_resolution_clock::now();

    try {
        // Create new node for the split
        auto newNode = std::make_unique<RTreeNode<T>>();
        newNode->m_isLeaf = node->m_isLeaf;

        // Find the two entries that would waste the most area if put together
        size_t seed1 = 0, seed2 = 0;
        double maxWaste = -1;

        for (size_t i = 0; i < node->m_entries.size(); ++i) {
            for (size_t j = i + 1; j < node->m_entries.size(); ++j) {
                std::vector<BoundingBox> seedBoxes = {
                    node->m_entries[i].first,
                    node->m_entries[j].first
                };
                BoundingBox combined = CalculateBounds(seedBoxes);
                
                // Calculate wasted area
                double area1 = (node->m_entries[i].first.GetMaxLat() - node->m_entries[i].first.GetMinLat()) *
                             (node->m_entries[i].first.GetMaxLon() - node->m_entries[i].first.GetMinLon());
                double area2 = (node->m_entries[j].first.GetMaxLat() - node->m_entries[j].first.GetMinLat()) *
                             (node->m_entries[j].first.GetMaxLon() - node->m_entries[j].first.GetMinLon());
                double combinedArea = (combined.GetMaxLat() - combined.GetMinLat()) *
                                    (combined.GetMaxLon() - combined.GetMinLon());
                double waste = combinedArea - (area1 + area2);

                if (waste > maxWaste) {
                    maxWaste = waste;
                    seed1 = i;
                    seed2 = j;
                }
            }
        }

        // Initialize the two groups with the seeds
        std::vector<std::pair<BoundingBox, T>> group1, group2;
        group1.push_back(node->m_entries[seed1]);
        group2.push_back(node->m_entries[seed2]);

        // Create a list of entries to distribute (excluding seeds)
        std::vector<std::pair<BoundingBox, T>> remaining;
        for (size_t i = 0; i < node->m_entries.size(); ++i) {
            if (i != seed1 && i != seed2) {
                remaining.push_back(node->m_entries[i]);
            }
        }

        // Distribute remaining entries
        while (!remaining.empty()) {
            // Check if we need to put all remaining entries in one group
            if (group1.size() + remaining.size() <= RTreeNode<T>::MIN_ENTRIES) {
                // Put all remaining in group1
                group1.insert(group1.end(), remaining.begin(), remaining.end());
                break;
            }
            if (group2.size() + remaining.size() <= RTreeNode<T>::MIN_ENTRIES) {
                // Put all remaining in group2
                group2.insert(group2.end(), remaining.begin(), remaining.end());
                break;
            }

            // Find entry with maximum preference for one group
            size_t bestIndex = 0;
            double maxDiff = -1;
            bool assignToGroup1 = true;

            for (size_t i = 0; i < remaining.size(); ++i) {
                // Calculate area enlargement for both groups
                std::vector<BoundingBox> boxes1 = { CalculateBounds(GetBoundingBoxes(group1)), remaining[i].first };
                std::vector<BoundingBox> boxes2 = { CalculateBounds(GetBoundingBoxes(group2)), remaining[i].first };
                
                BoundingBox enlarged1 = CalculateBounds(boxes1);
                BoundingBox enlarged2 = CalculateBounds(boxes2);

                double area1 = (enlarged1.GetMaxLat() - enlarged1.GetMinLat()) *
                             (enlarged1.GetMaxLon() - enlarged1.GetMinLon());
                double area2 = (enlarged2.GetMaxLat() - enlarged2.GetMinLat()) *
                             (enlarged2.GetMaxLon() - enlarged2.GetMinLon());
                
                double diff = std::abs(area1 - area2);
                if (diff > maxDiff) {
                    maxDiff = diff;
                    bestIndex = i;
                    assignToGroup1 = area1 < area2;
                }
            }

            // Assign the chosen entry to the preferred group
            if (assignToGroup1) {
                group1.push_back(remaining[bestIndex]);
            } else {
                group2.push_back(remaining[bestIndex]);
            }

            // Remove the entry from remaining
            remaining.erase(remaining.begin() + bestIndex);
        }

        // Update the original node with group1
        node->m_entries = std::move(group1);
        node->m_bounds = CalculateBounds(GetBoundingBoxes(node->m_entries));

        // Set up the new node with group2
        newNode->m_entries = std::move(group2);
        newNode->m_bounds = CalculateBounds(GetBoundingBoxes(newNode->m_entries));

        // Add the new node to the parent
        // Find first empty slot in parent's children array
        bool slotFound = false;
        for (auto& child : node->m_children) {
            if (!child) {
                child = std::move(newNode);
                slotFound = true;
                break;
            }
        }

        if (!slotFound) {
            DEBUG_ERROR("No empty slot found in parent node for split");
            throw std::runtime_error("Node split failed: no empty slot in parent");
        }

        // Log performance metrics
        auto endTime = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime);
        
        DEBUG_DEBUG("Node split completed in ", duration.count() / 1000.0, "ms. ",
                   "Group1: ", node->m_entries.size(), " entries, ",
                   "Group2: ", newNode->m_entries.size(), " entries");
    }
    catch (const std::exception& e) {
        DEBUG_ERROR("Node split failed: ", e.what());
        throw; // Re-throw after logging
    }
}

// Helper method to extract bounding boxes from entries
template<typename T>
std::vector<BoundingBox> GetBoundingBoxes(const std::vector<std::pair<BoundingBox, T>>& entries) {
    std::vector<BoundingBox> boxes;
    boxes.reserve(entries.size());
    for (const auto& entry : entries) {
        boxes.push_back(entry.first);
    }
    return boxes;
}

template<typename T>
RTreeNode<T>* SpatialIndex<T>::ChooseSubtree(const BoundingBox& bounds) {
    if (!m_root) {
        // Create root if it doesn't exist
        m_root = std::make_unique<RTreeNode<T>>();
        return m_root.get();
    }

    RTreeNode<T>* current = m_root.get();

    // Traverse until we reach a leaf node
    while (!current->IsLeaf()) {
        RTreeNode<T>* bestChild = nullptr;
        double minEnlargement = std::numeric_limits<double>::max();
        double minArea = std::numeric_limits<double>::max();

        // Examine all children to find the best one
        for (const auto& child : current->m_children) {
            if (!child) continue;

            // Calculate area enlargement needed
            std::vector<BoundingBox> boxes = {child->GetBounds(), bounds};
            BoundingBox combinedBounds = CalculateBounds(boxes);
            
            // Calculate metrics
            double currentArea = (child->GetBounds().GetMaxLat() - child->GetBounds().GetMinLat()) *
                               (child->GetBounds().GetMaxLon() - child->GetBounds().GetMinLon());
            double enlargedArea = (combinedBounds.GetMaxLat() - combinedBounds.GetMinLat()) *
                                (combinedBounds.GetMaxLon() - combinedBounds.GetMinLon());
            double enlargement = enlargedArea - currentArea;

            // Update best child if this one is better
            if (enlargement < minEnlargement || 
                (enlargement == minEnlargement && currentArea < minArea)) {
                minEnlargement = enlargement;
                minArea = currentArea;
                bestChild = child.get();
            }
        }

        if (!bestChild) {
            // If no valid child found, create a new one
            auto newChild = std::make_unique<RTreeNode<T>>();
            bestChild = newChild.get();
            
            // Find first empty slot in children array
            for (auto& child : current->m_children) {
                if (!child) {
                    child = std::move(newChild);
                    break;
                }
            }
        }

        DEBUG_DEBUG("Chose subtree with enlargement ", minEnlargement,
                   " and area ", minArea);

        current = bestChild;
    }

    return current;
}

// SpatialIndexManager implementation
void SpatialIndexManager::AddNode(const OSMNode& node) {
    // Create a point bounding box for the node
    // For nodes, the bounding box is effectively a point (min = max)
    BoundingBox nodeBounds(
        node.GetLatitude(),
        node.GetLatitude(),
        node.GetLongitude(),
        node.GetLongitude()
    );

    try {
        // Insert the node into the spatial index
        m_nodeIndex.Insert(node, nodeBounds);

        DEBUG_DEBUG("Added node ", node.GetId(), 
                   " at (", node.GetLatitude(), ", ", node.GetLongitude(), ")");

        // If this is a significant node (has tags), log additional info
        if (!node.GetTags().empty()) {
            DEBUG_DEBUG("Node ", node.GetId(), " has ", 
                       node.GetTags().size(), " tags");
        }
    }
    catch (const std::exception& e) {
        DEBUG_ERROR("Failed to add node ", node.GetId(), 
                   " to spatial index: ", e.what());
        throw; // Re-throw the exception after logging
    }
}

void SpatialIndexManager::AddWay(const OSMWay& way) {
    try {
        // Get the way's bounds
        double minLat, maxLat, minLon, maxLon;
        way.CalculateBounds(minLat, maxLat, minLon, maxLon);
        
        // Create bounding box for the way
        BoundingBox wayBounds(minLat, maxLat, minLon, maxLon);

        // Insert the way into the spatial index
        m_wayIndex.Insert(way, wayBounds);

        // Log basic way information
        DEBUG_DEBUG("Added way ", way.GetId(), 
                   " with ", way.GetNodeRefs().size(), " nodes",
                   " (", minLat, ",", minLon, ") to (", 
                   maxLat, ",", maxLon, ")");

        // Log additional information for significant ways
        if (!way.GetTags().empty()) {
            std::string wayType = way.IsArea() ? "area" : "line";
            DEBUG_DEBUG("Way ", way.GetId(), 
                       " is ", wayType,
                       ", has ", way.GetTags().size(), " tags");
            
            // Log specific important tags
            static const std::vector<std::string> importantTags = {
                "highway", "railway", "waterway", "building", "landuse"
            };
            
            for (const auto& tag : importantTags) {
                if (way.HasTag(tag)) {
                    DEBUG_DEBUG("Way ", way.GetId(), 
                               " has ", tag, "=", way.GetTag(tag));
                }
            }
        }
    }
    catch (const std::exception& e) {
        DEBUG_ERROR("Failed to add way ", way.GetId(), 
                   " to spatial index: ", e.what());
        throw; // Re-throw the exception after logging
    }
}

std::vector<OSMNode> SpatialIndexManager::GetNodesInBounds(const BoundingBox& bounds) const {
    DEBUG_DEBUG("Querying nodes in bounds: ",
                bounds.GetMinLat(), ", ", bounds.GetMaxLat(), ", ",
                bounds.GetMinLon(), ", ", bounds.GetMaxLon());

    auto startTime = std::chrono::high_resolution_clock::now();
    
    // Query the node index
    auto nodes = m_nodeIndex.QueryRange(bounds);
    
    // Calculate and log performance metrics
    auto endTime = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime);
    
    DEBUG_DEBUG("Found ", nodes.size(), " nodes in ", 
                duration.count() / 1000.0, "ms");
    
    return nodes;
}

std::vector<OSMWay> SpatialIndexManager::GetWaysInBounds(const BoundingBox& bounds) const {
    DEBUG_DEBUG("Querying ways in bounds: ",
                bounds.GetMinLat(), ", ", bounds.GetMaxLat(), ", ",
                bounds.GetMinLon(), ", ", bounds.GetMaxLon());

    auto startTime = std::chrono::high_resolution_clock::now();
    
    // Query the way index
    auto ways = m_wayIndex.QueryRange(bounds);
    
    // Post-process ways to ensure they actually intersect the bounds
    // (R-tree might return ways whose bounding box intersects but the way itself doesn't)
    ways.erase(
        std::remove_if(ways.begin(), ways.end(),
            [&bounds](const OSMWay& way) {
                double minLat, maxLat, minLon, maxLon;
                way.CalculateBounds(minLat, maxLat, minLon, maxLon);
                BoundingBox wayBounds(minLat, maxLat, minLon, maxLon);
                return !wayBounds.Intersects(bounds);
            }
        ),
        ways.end()
    );
    
    // Calculate and log performance metrics
    auto endTime = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime);
    
    DEBUG_DEBUG("Found ", ways.size(), " ways in ", 
                duration.count() / 1000.0, "ms");
    
    return ways;
}

void SpatialIndexManager::Clear() {
    auto startTime = std::chrono::high_resolution_clock::now();

    try {
        // Store counts before clearing for logging
        size_t nodeCount = m_nodeIndex.QueryRange(BoundingBox(-90, 90, -180, 180)).size();
        size_t wayCount = m_wayIndex.QueryRange(BoundingBox(-90, 90, -180, 180)).size();
        size_t relationCount = m_relationIndex.QueryRange(BoundingBox(-90, 90, -180, 180)).size();

        // Clear all indices
        m_nodeIndex.Clear();
        m_wayIndex.Clear();
        m_relationIndex.Clear();

        // Calculate and log performance metrics
        auto endTime = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime);

        DEBUG_INFO("Cleared spatial indices: ",
                  nodeCount, " nodes, ",
                  wayCount, " ways, ",
                  relationCount, " relations ",
                  "in ", duration.count() / 1000.0, "ms");
    }
    catch (const std::exception& e) {
        DEBUG_ERROR("Failed to clear spatial indices: ", e.what());
        throw; // Re-throw the exception after logging
    }
}

void SpatialIndexManager::AddRelation(const OSMRelation& relation) {
    try {
        // Calculate bounds for the relation
        BoundingBox relationBounds = CalculateRelationBounds(relation);

        // Insert the relation into the spatial index
        m_relationIndex.Insert(relation, relationBounds);

        // Log basic relation information
        DEBUG_DEBUG("Added relation ", relation.GetId(),
                   " with ", relation.GetMembers().size(), " members",
                   " at (", relationBounds.GetMinLat(), ",", relationBounds.GetMinLon(),
                   ") to (", relationBounds.GetMaxLat(), ",", relationBounds.GetMaxLon(), ")");

        // Log additional information based on relation type
        std::string relationType;
        if (relation.IsMultipolygon()) {
            relationType = "multipolygon";
        } else if (relation.IsRoute()) {
            relationType = "route";
        } else if (relation.IsBoundary()) {
            relationType = "boundary";
        } else {
            relationType = "other";
        }

        DEBUG_DEBUG("Relation ", relation.GetId(),
                   " is type: ", relationType,
                   ", has ", relation.GetTags().size(), " tags");

        // Log member type distribution
        size_t nodeCount = 0, wayCount = 0, relationCount = 0;
        for (const auto& member : relation.GetMembers()) {
            switch (member.type) {
                case OSMRelation::MemberType::Node:
                    nodeCount++;
                    break;
                case OSMRelation::MemberType::Way:
                    wayCount++;
                    break;
                case OSMRelation::MemberType::Relation:
                    relationCount++;
                    break;
            }
        }

        DEBUG_DEBUG("Relation ", relation.GetId(),
                   " members: ", nodeCount, " nodes, ",
                   wayCount, " ways, ",
                   relationCount, " relations");

        // Log specific important tags
        static const std::vector<std::string> importantTags = {
            "type", "route", "boundary", "admin_level", "name"
        };

        for (const auto& tag : importantTags) {
            if (relation.HasTag(tag)) {
                DEBUG_DEBUG("Relation ", relation.GetId(),
                           " has ", tag, "=", relation.GetTag(tag));
            }
        }
    }
    catch (const std::exception& e) {
        DEBUG_ERROR("Failed to add relation ", relation.GetId(),
                   " to spatial index: ", e.what());
        throw; // Re-throw the exception after logging
    }
}

std::vector<OSMRelation> SpatialIndexManager::GetRelationsInBounds(const BoundingBox& bounds) const {
    DEBUG_DEBUG("Querying relations in bounds: ",
                bounds.GetMinLat(), ", ", bounds.GetMaxLat(), ", ",
                bounds.GetMinLon(), ", ", bounds.GetMaxLon());

    auto startTime = std::chrono::high_resolution_clock::now();
    
    // Query the relation index
    auto relations = m_relationIndex.QueryRange(bounds);
    
    // Post-process relations to handle special cases
    relations.erase(
        std::remove_if(relations.begin(), relations.end(),
            [this, &bounds](const OSMRelation& relation) {
                // Calculate actual bounds for the relation
                BoundingBox relationBounds = CalculateRelationBounds(relation);
                
                // Special handling for different relation types
                if (relation.IsMultipolygon() || relation.IsBoundary()) {
                    // For area-based relations, check for actual intersection
                    return !relationBounds.Intersects(bounds);
                }
                else if (relation.IsRoute()) {
                    // For routes, we might want to be more lenient
                    // to show complete routes that partially intersect
                    return false;
                }
                
                // For other relations, use standard bounds intersection
                return !relationBounds.Intersects(bounds);
            }
        ),
        relations.end()
    );
    
    // Calculate and log performance metrics
    auto endTime = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime);
    
    DEBUG_DEBUG("Found ", relations.size(), " relations in ", 
                duration.count() / 1000.0, "ms");
    
    return relations;
}

BoundingBox SpatialIndexManager::CalculateRelationBounds(const OSMRelation& relation) const {
    // Add LRU cache for relation bounds
    static thread_local std::unordered_map<int64_t, std::pair<BoundingBox, std::chrono::steady_clock::time_point>> boundsCache;
    
    // Check cache first
    auto it = boundsCache.find(relation.GetId());
    if (it != boundsCache.end() && 
        (std::chrono::steady_clock::now() - it->second.second) < std::chrono::minutes(5)) {
        return it->second.first;
    }
    
    // Track processed relation IDs to prevent cycles
    static thread_local std::unordered_set<int64_t> processedRelations;
    
    // Initialize bounds to invalid state
    double minLat = std::numeric_limits<double>::max();
    double maxLat = -std::numeric_limits<double>::max();
    double minLon = std::numeric_limits<double>::max();
    double maxLon = -std::numeric_limits<double>::max();

    try {
        // Check for cycles
        if (!processedRelations.insert(relation.GetId()).second) {
            DEBUG_WARNING("Detected cycle in relation ", relation.GetId());
            return BoundingBox(0, 0, 0, 0); // Return empty bounds for cyclic relations
        }

        // Process each member
        for (const auto& member : relation.GetMembers()) {
            BoundingBox memberBounds;

            switch (member.type) {
                case OSMRelation::MemberType::Node: {
                    // Query node from index
                    auto nodes = m_nodeIndex.QueryRange(BoundingBox(-90, 90, -180, 180));
                    for (const auto& node : nodes) {
                        if (node.GetId() == member.ref) {
                            minLat = std::min(minLat, node.GetLatitude());
                            maxLat = std::max(maxLat, node.GetLatitude());
                            minLon = std::min(minLon, node.GetLongitude());
                            maxLon = std::max(maxLon, node.GetLongitude());
                            break;
                        }
                    }
                    break;
                }
                case OSMRelation::MemberType::Way: {
                    // Query way from index
                    auto ways = m_wayIndex.QueryRange(BoundingBox(-90, 90, -180, 180));
                    for (const auto& way : ways) {
                        if (way.GetId() == member.ref) {
                            double wayMinLat, wayMaxLat, wayMinLon, wayMaxLon;
                            way.CalculateBounds(wayMinLat, wayMaxLat, wayMinLon, wayMaxLon);
                            minLat = std::min(minLat, wayMinLat);
                            maxLat = std::max(maxLat, wayMaxLat);
                            minLon = std::min(minLon, wayMinLon);
                            maxLon = std::max(maxLon, wayMaxLon);
                            break;
                        }
                    }
                    break;
                }
                case OSMRelation::MemberType::Relation: {
                    // Recursively calculate bounds for sub-relations
                    auto relations = m_relationIndex.QueryRange(BoundingBox(-90, 90, -180, 180));
                    for (const auto& subRelation : relations) {
                        if (subRelation.GetId() == member.ref) {
                            BoundingBox subBounds = CalculateRelationBounds(subRelation);
                            minLat = std::min(minLat, subBounds.GetMinLat());
                            maxLat = std::max(maxLat, subBounds.GetMaxLat());
                            minLon = std::min(minLon, subBounds.GetMinLon());
                            maxLon = std::max(maxLon, subBounds.GetMaxLon());
                            break;
                        }
                    }
                    break;
                }
            }
        }

        // Remove relation from processed set after completion
        processedRelations.erase(relation.GetId());
    }
    catch (const std::exception& e) {
        // Clean up processed relations set in case of exception
        processedRelations.erase(relation.GetId());
        throw;
    }

    // If no valid bounds were found, return empty bounds
    if (minLat > maxLat || minLon > maxLon) {
        DEBUG_WARNING("No valid bounds found for relation ", relation.GetId());
        return BoundingBox(0, 0, 0, 0);
    }

    // Cache the result
    boundsCache[relation.GetId()] = { BoundingBox(minLat, maxLat, minLon, maxLon), std::chrono::steady_clock::now() };

    return BoundingBox(minLat, maxLat, minLon, maxLon);
}

// Explicit template instantiations
template class SpatialIndex<OSMNode>;
template class SpatialIndex<OSMWay>;
template class SpatialIndex<OSMRelation>; 