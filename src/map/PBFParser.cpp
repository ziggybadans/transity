#include "PBFParser.h"
#include "OSMDataStore.h"
#include "ProgressTracker.h"
#include "../Debug.h"

PBFParser::PBFParser(std::shared_ptr<OSMDataStore> dataStore)
    : m_dataStore(std::move(dataStore))
    , m_cancelled(false)
    , m_processedBlocks(0)
{
    DEBUG_INFO("PBF Parser initialized");
}

void PBFParser::ProcessBlock(const std::vector<uint8_t>& blockData, const ParserConfig& config)
{
    if (m_cancelled) {
        DEBUG_INFO("Block processing cancelled");
        return;
    }

    try {
        // Validate block size
        if (blockData.empty()) {
            throw std::runtime_error("Empty block data");
        }
        if (blockData.size() > config.blockSize) {
            throw std::runtime_error("Block size exceeds configured maximum");
        }

        // Track processing time
        auto startTime = std::chrono::high_resolution_clock::now();

        // Read block header (first byte indicates content type)
        if (blockData.size() < sizeof(uint8_t)) {
            throw std::runtime_error("Invalid block header");
        }
        uint8_t contentType = blockData[0];
        size_t dataOffset = sizeof(uint8_t);

        // Read compression flag if present (second byte)
        bool isCompressed = false;
        if (blockData.size() > dataOffset) {
            isCompressed = (blockData[dataOffset] != 0);
            dataOffset += sizeof(uint8_t);
        }

        // Prepare data for parsing
        std::vector<uint8_t> processData;
        if (isCompressed) {
            // Note: In a real implementation, you would decompress the data here
            // using zlib or similar. For this example, we'll throw an error
            throw std::runtime_error("Compressed blocks not yet implemented");
        } else {
            processData.assign(blockData.begin() + dataOffset, blockData.end());
        }

        // Process based on content type and config
        switch (contentType) {
            case 0x01: // Node block
                if (config.parseNodes) {
                    DEBUG_DEBUG("Processing node block of size ", processData.size());
                    ParseNodes(processData);
                }
                break;

            case 0x02: // Way block
                if (config.parseWays) {
                    DEBUG_DEBUG("Processing way block of size ", processData.size());
                    ParseWays(processData);
                }
                break;

            case 0x03: // Relation block
                if (config.parseRelations) {
                    DEBUG_DEBUG("Processing relation block of size ", processData.size());
                    ParseRelations(processData);
                }
                break;

            default:
                throw std::runtime_error("Unknown block type: " + std::to_string(contentType));
        }

        // Increment processed blocks counter
        m_processedBlocks++;

        // Update progress if tracker is available
        if (m_progressTracker) {
            // Assuming linear progress through blocks
            float progress = static_cast<float>(m_processedBytes) / static_cast<float>(m_totalFileSize);
            m_progressTracker->UpdateProgress(progress);
            
            std::string status = "Processed block " + std::to_string(m_processedBlocks);
            switch (contentType) {
                case 0x01: status += " (Nodes)"; break;
                case 0x02: status += " (Ways)"; break;
                case 0x03: status += " (Relations)"; break;
            }
            m_progressTracker->SetStatus(status);
        }

        // Calculate and log performance metrics
        auto endTime = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime);

        DEBUG_INFO("Processed block ", m_processedBlocks, 
                  " (", processData.size(), " bytes) in ", 
                  duration.count(), "ms");

    } catch (const std::exception& e) {
        m_lastError = "Error processing block: " + std::string(e.what());
        DEBUG_ERROR(m_lastError);
        throw; // Re-throw to be handled by caller
    }
}

void PBFParser::ParseNodes(const std::vector<uint8_t>& nodeData)
{
    if (m_cancelled) {
        DEBUG_INFO("Node parsing cancelled");
        return;
    }

    try {
        // Track parsing progress
        size_t processedNodes = 0;
        auto startTime = std::chrono::high_resolution_clock::now();

        // Parse dense nodes format
        // Note: This is a simplified structure - actual PBF format uses delta encoding
        size_t offset = 0;
        while (offset + sizeof(int64_t) + 2 * sizeof(double) <= nodeData.size()) {
            // Check for cancellation periodically
            if (processedNodes % 10000 == 0 && m_cancelled) {
                DEBUG_INFO("Node parsing cancelled after ", processedNodes, " nodes");
                return;
            }

            // Read node data from buffer
            int64_t id;
            double lat, lon;
            std::memcpy(&id, nodeData.data() + offset, sizeof(int64_t));
            offset += sizeof(int64_t);
            std::memcpy(&lat, nodeData.data() + offset, sizeof(double));
            offset += sizeof(double);
            std::memcpy(&lon, nodeData.data() + offset, sizeof(double));
            offset += sizeof(double);

            // Validate coordinates
            if (lat < -90.0 || lat > 90.0 || lon < -180.0 || lon > 180.0) {
                DEBUG_WARNING("Invalid coordinates for node ", id, 
                            ": (", lat, ", ", lon, ")");
                continue;
            }

            // Create node object
            OSMNode node(id, lat, lon);

            // Parse tags if present
            while (offset + 2 * sizeof(uint32_t) <= nodeData.size()) {
                uint32_t keyLen, valueLen;
                std::memcpy(&keyLen, nodeData.data() + offset, sizeof(uint32_t));
                offset += sizeof(uint32_t);

                // Check for end of tags marker
                if (keyLen == 0) break;

                std::memcpy(&valueLen, nodeData.data() + offset, sizeof(uint32_t));
                offset += sizeof(uint32_t);

                // Read key and value strings
                if (offset + keyLen + valueLen <= nodeData.size()) {
                    std::string key(reinterpret_cast<const char*>(nodeData.data() + offset), keyLen);
                    offset += keyLen;
                    std::string value(reinterpret_cast<const char*>(nodeData.data() + offset), valueLen);
                    offset += valueLen;

                    node.AddTag(key, value);
                } else {
                    DEBUG_ERROR("Invalid tag data for node ", id);
                    break;
                }
            }

            // Add node to data store
            m_dataStore->AddNode(std::move(node));
            processedNodes++;

            // Update progress periodically
            if (m_progressTracker && processedNodes % 10000 == 0) {
                float progress = static_cast<float>(offset) / static_cast<float>(nodeData.size());
                m_progressTracker->UpdateProgress(progress);
                m_progressTracker->SetStatus("Processed " + std::to_string(processedNodes) + " nodes...");
            }
        }

        // Calculate and log performance metrics
        auto endTime = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime);

        DEBUG_INFO("Parsed ", processedNodes, " nodes in ", 
                  duration.count(), "ms (", 
                  processedNodes * 1000.0 / duration.count(), " nodes/sec)");

    } catch (const std::exception& e) {
        m_lastError = "Error parsing nodes: " + std::string(e.what());
        DEBUG_ERROR(m_lastError);
        throw; // Re-throw to be handled by ProcessBlock
    }
}

void PBFParser::ParseWays(const std::vector<uint8_t>& wayData)
{
    if (m_cancelled) {
        DEBUG_INFO("Way parsing cancelled");
        return;
    }

    try {
        // Track parsing progress
        size_t processedWays = 0;
        auto startTime = std::chrono::high_resolution_clock::now();

        // Parse ways
        size_t offset = 0;
        while (offset + sizeof(int64_t) <= wayData.size()) {
            // Check for cancellation periodically
            if (processedWays % 1000 == 0 && m_cancelled) {
                DEBUG_INFO("Way parsing cancelled after ", processedWays, " ways");
                return;
            }

            // Read way ID
            int64_t id;
            std::memcpy(&id, wayData.data() + offset, sizeof(int64_t));
            offset += sizeof(int64_t);

            // Create way object
            OSMWay way(id);

            // Read node references count
            uint32_t nodeRefsCount;
            if (offset + sizeof(uint32_t) > wayData.size()) break;
            std::memcpy(&nodeRefsCount, wayData.data() + offset, sizeof(uint32_t));
            offset += sizeof(uint32_t);

            // Read node references
            for (uint32_t i = 0; i < nodeRefsCount; ++i) {
                if (offset + sizeof(int64_t) > wayData.size()) {
                    DEBUG_ERROR("Invalid node reference data for way ", id);
                    break;
                }
                int64_t nodeRef;
                std::memcpy(&nodeRef, wayData.data() + offset, sizeof(int64_t));
                offset += sizeof(int64_t);
                way.AddNodeRef(nodeRef);
            }

            // Parse tags
            while (offset + 2 * sizeof(uint32_t) <= wayData.size()) {
                uint32_t keyLen, valueLen;
                std::memcpy(&keyLen, wayData.data() + offset, sizeof(uint32_t));
                offset += sizeof(uint32_t);

                // Check for end of tags marker
                if (keyLen == 0) break;

                std::memcpy(&valueLen, wayData.data() + offset, sizeof(uint32_t));
                offset += sizeof(uint32_t);

                // Read key and value strings
                if (offset + keyLen + valueLen <= wayData.size()) {
                    std::string key(reinterpret_cast<const char*>(wayData.data() + offset), keyLen);
                    offset += keyLen;
                    std::string value(reinterpret_cast<const char*>(wayData.data() + offset), valueLen);
                    offset += valueLen;

                    way.AddTag(key, value);
                } else {
                    DEBUG_ERROR("Invalid tag data for way ", id);
                    break;
                }
            }

            // Add way to data store
            m_dataStore->AddWay(std::move(way));
            processedWays++;

            // Update progress periodically
            if (m_progressTracker && processedWays % 1000 == 0) {
                float progress = static_cast<float>(offset) / static_cast<float>(wayData.size());
                m_progressTracker->UpdateProgress(progress);
                m_progressTracker->SetStatus("Processed " + std::to_string(processedWays) + " ways...");
            }
        }

        // Calculate and log performance metrics
        auto endTime = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime);

        DEBUG_INFO("Parsed ", processedWays, " ways in ", 
                  duration.count(), "ms (", 
                  processedWays * 1000.0 / duration.count(), " ways/sec)");

        // Log additional statistics about way types
        if (processedWays > 0) {
            size_t highways = 0, buildings = 0, waterways = 0, areas = 0;
            {
                const auto& ways = m_dataStore->GetWays();
                for (const auto& way : ways) {
                    if (way.HasTag("highway")) highways++;
                    if (way.HasTag("building")) buildings++;
                    if (way.HasTag("waterway")) waterways++;
                    if (way.IsArea()) areas++;
                }
            }
            
            DEBUG_INFO("Way statistics: ",
                      highways, " highways, ",
                      buildings, " buildings, ",
                      waterways, " waterways, ",
                      areas, " areas");
        }

    } catch (const std::exception& e) {
        m_lastError = "Error parsing ways: " + std::string(e.what());
        DEBUG_ERROR(m_lastError);
        throw; // Re-throw to be handled by ProcessBlock
    }
}

void PBFParser::ParseRelations(const std::vector<uint8_t>& relationData)
{
    if (m_cancelled) {
        DEBUG_INFO("Relation parsing cancelled");
        return;
    }

    try {
        // Track parsing progress
        size_t processedRelations = 0;
        auto startTime = std::chrono::high_resolution_clock::now();

        // Parse relations
        size_t offset = 0;
        while (offset + sizeof(int64_t) <= relationData.size()) {
            // Check for cancellation periodically
            if (processedRelations % 1000 == 0 && m_cancelled) {
                DEBUG_INFO("Relation parsing cancelled after ", processedRelations, " relations");
                return;
            }

            // Read relation ID
            int64_t id;
            std::memcpy(&id, relationData.data() + offset, sizeof(int64_t));
            offset += sizeof(int64_t);

            // Create relation object
            OSMRelation relation(id);

            // Read member count
            uint32_t memberCount;
            if (offset + sizeof(uint32_t) > relationData.size()) break;
            std::memcpy(&memberCount, relationData.data() + offset, sizeof(uint32_t));
            offset += sizeof(uint32_t);

            // Read members
            for (uint32_t i = 0; i < memberCount; ++i) {
                if (offset + sizeof(uint8_t) + sizeof(int64_t) > relationData.size()) {
                    DEBUG_ERROR("Invalid member data for relation ", id);
                    break;
                }

                // Read member type
                uint8_t typeValue;
                std::memcpy(&typeValue, relationData.data() + offset, sizeof(uint8_t));
                offset += sizeof(uint8_t);

                // Read member ID
                int64_t memberId;
                std::memcpy(&memberId, relationData.data() + offset, sizeof(int64_t));
                offset += sizeof(int64_t);

                // Read role string length
                uint32_t roleLen;
                if (offset + sizeof(uint32_t) > relationData.size()) break;
                std::memcpy(&roleLen, relationData.data() + offset, sizeof(uint32_t));
                offset += sizeof(uint32_t);

                // Read role string
                if (offset + roleLen > relationData.size()) {
                    DEBUG_ERROR("Invalid role string for relation ", id);
                    break;
                }
                std::string role(reinterpret_cast<const char*>(relationData.data() + offset), roleLen);
                offset += roleLen;

                // Convert type value to enum
                OSMRelation::MemberType memberType;
                switch (typeValue) {
                    case 0: memberType = OSMRelation::MemberType::Node; break;
                    case 1: memberType = OSMRelation::MemberType::Way; break;
                    case 2: memberType = OSMRelation::MemberType::Relation; break;
                    default:
                        DEBUG_WARNING("Invalid member type ", typeValue, " for relation ", id);
                        continue;
                }

                // Add member to relation
                relation.AddMember(memberType, memberId, role);
            }

            // Parse tags
            while (offset + 2 * sizeof(uint32_t) <= relationData.size()) {
                uint32_t keyLen, valueLen;
                std::memcpy(&keyLen, relationData.data() + offset, sizeof(uint32_t));
                offset += sizeof(uint32_t);

                // Check for end of tags marker
                if (keyLen == 0) break;

                std::memcpy(&valueLen, relationData.data() + offset, sizeof(uint32_t));
                offset += sizeof(uint32_t);

                // Read key and value strings
                if (offset + keyLen + valueLen <= relationData.size()) {
                    std::string key(reinterpret_cast<const char*>(relationData.data() + offset), keyLen);
                    offset += keyLen;
                    std::string value(reinterpret_cast<const char*>(relationData.data() + offset), valueLen);
                    offset += valueLen;

                    relation.AddTag(key, value);
                } else {
                    DEBUG_ERROR("Invalid tag data for relation ", id);
                    break;
                }
            }

            // Add relation to data store
            m_dataStore->AddRelation(std::move(relation));
            processedRelations++;

            // Update progress periodically
            if (m_progressTracker && processedRelations % 1000 == 0) {
                float progress = static_cast<float>(offset) / static_cast<float>(relationData.size());
                m_progressTracker->UpdateProgress(progress);
                m_progressTracker->SetStatus("Processed " + std::to_string(processedRelations) + " relations...");
            }
        }

        // Calculate and log performance metrics
        auto endTime = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime);

        DEBUG_INFO("Parsed ", processedRelations, " relations in ", 
                  duration.count(), "ms (", 
                  processedRelations * 1000.0 / duration.count(), " relations/sec)");

        // Log additional statistics about relation types
        if (processedRelations > 0) {
            size_t multipolygons = 0, routes = 0, boundaries = 0;
            size_t nodeMembers = 0, wayMembers = 0, relationMembers = 0;
            {
                const auto& relations = m_dataStore->GetRelations();
                for (const auto& rel : relations) {
                    if (rel.IsMultipolygon()) multipolygons++;
                    if (rel.IsRoute()) routes++;
                    if (rel.IsBoundary()) boundaries++;

                    // Count member types
                    for (const auto& member : rel.GetMembers()) {
                        switch (member.type) {
                            case OSMRelation::MemberType::Node: nodeMembers++; break;
                            case OSMRelation::MemberType::Way: wayMembers++; break;
                            case OSMRelation::MemberType::Relation: relationMembers++; break;
                        }
                    }
                }
            }
            
            DEBUG_INFO("Relation statistics: ",
                      multipolygons, " multipolygons, ",
                      routes, " routes, ",
                      boundaries, " boundaries");
            DEBUG_INFO("Member statistics: ",
                      nodeMembers, " nodes, ",
                      wayMembers, " ways, ",
                      relationMembers, " relations");
        }

    } catch (const std::exception& e) {
        m_lastError = "Error parsing relations: " + std::string(e.what());
        DEBUG_ERROR(m_lastError);
        throw; // Re-throw to be handled by ProcessBlock
    }
}

void PBFParser::SetProgressTracker(std::shared_ptr<ProgressTracker> tracker)
{
    // Store progress tracker
    m_progressTracker = std::move(tracker);
    
    // If we have a valid tracker and blocks have been processed, update the progress
    if (m_progressTracker) {
        // Set initial status
        m_progressTracker->SetStatus("Processing PBF data...");
        
        // If blocks have been processed, calculate and update progress
        if (m_processedBlocks > 0) {
            // Update the progress based on processed blocks
            float progress = static_cast<float>(m_processedBytes) / static_cast<float>(m_totalFileSize);
            m_progressTracker->UpdateProgress(progress);
            
            // Update the status with block count
            m_progressTracker->SetStatus("Processed " + 
                                       std::to_string(m_processedBlocks) + 
                                       " blocks...");
        } else {
            // No blocks processed yet, set initial progress
            m_progressTracker->UpdateProgress(0.0f);
        }
    }
    
    DEBUG_INFO("Progress tracker " + 
               std::string(tracker ? "attached to" : "detached from") + 
               " PBF Parser");
}

void PBFParser::SetCancelled(bool cancelled)
{
    // Atomically set cancelled flag
    m_cancelled = cancelled;
    
    // Log cancellation status
    if (cancelled) {
        DEBUG_INFO("PBF Parser: Cancellation requested");
        m_lastError = "Operation cancelled by user";
    } else {
        DEBUG_INFO("PBF Parser: Cancellation flag cleared");
        // Only clear the error if it was a cancellation error
        if (m_lastError == "Operation cancelled by user") {
            m_lastError.clear();
        }
    }

    // If we have a progress tracker, update its status
    if (m_progressTracker) {
        m_progressTracker->SetStatus(cancelled ? "Cancelling..." : "Processing...");
    }
}

size_t PBFParser::GetProcessedBlocks() const
{
    return m_processedBlocks;
}

bool PBFParser::HasError() const
{
    return !m_lastError.empty();
}

std::string PBFParser::GetLastError() const
{
    return m_lastError;
}

std::vector<uint8_t> PBFParser::DecompressBlock(const std::vector<uint8_t>& compressedData) {
    if (m_decompressionBuffer.size() < COMPRESSION_BUFFER_SIZE) {
        m_decompressionBuffer.resize(COMPRESSION_BUFFER_SIZE);
    }
    
    z_stream stream{};
    stream.next_in = const_cast<Bytef*>(compressedData.data());
    stream.avail_in = static_cast<uInt>(compressedData.size());
    stream.next_out = m_decompressionBuffer.data();
    stream.avail_out = static_cast<uInt>(m_decompressionBuffer.size());

    if (inflateInit(&stream) != Z_OK) {
        throw std::runtime_error("Failed to initialize decompression");
    }

    std::vector<uint8_t> decompressedData;
    do {
        int result = inflate(&stream, Z_NO_FLUSH);
        if (result == Z_STREAM_END) {
            break;
        }
        if (result != Z_OK) {
            inflateEnd(&stream);
            throw std::runtime_error("Decompression failed");
        }
        
        size_t bytesDecompressed = m_decompressionBuffer.size() - stream.avail_out;
        decompressedData.insert(decompressedData.end(),
                              m_decompressionBuffer.begin(),
                              m_decompressionBuffer.begin() + bytesDecompressed);
                              
        stream.next_out = m_decompressionBuffer.data();
        stream.avail_out = static_cast<uInt>(m_decompressionBuffer.size());
    } while (stream.avail_in > 0);

    inflateEnd(&stream);
    return decompressedData;
}