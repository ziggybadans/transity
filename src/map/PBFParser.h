#pragma once

#include <vector>
#include <memory>
#include <cstdint>
#include <string>
#include <mutex>
#include <atomic>
#include <chrono>

/* Forward declarations */
class OSMDataStore;
class ProgressTracker;

/**
 * @brief Handles the parsing of PBF format OSM data
 */
class PBFParser {
public:
    struct ParserConfig {
        bool parseNodes{true};
        bool parseWays{true};
        bool parseRelations{true};
        size_t blockSize{32 * 1024};
        bool enableCompression{true};
        bool enableParallelProcessing{true};
        size_t threadCount{std::thread::hardware_concurrency()};
        bool buildSpatialIndex{true};
    };

    struct ParserStatistics {
        size_t totalNodes{0};
        size_t totalWays{0};
        size_t totalRelations{0};
        size_t processedBytes{0};
        std::chrono::milliseconds processingTime{0};
        float nodesPerSecond{0.0f};
        float waysPerSecond{0.0f};
        float relationsPerSecond{0.0f};
    };

    explicit PBFParser(std::shared_ptr<OSMDataStore> dataStore);
    ~PBFParser() = default;
    
    void ProcessBlock(const std::vector<uint8_t>& blockData, const ParserConfig& config);
    void SetProgressTracker(std::shared_ptr<ProgressTracker> tracker);
    void SetCancelled(bool cancelled);
    
    size_t GetProcessedBlocks() const;
    bool HasError() const;
    std::string GetLastError() const;

private:
    void ParseNodes(const std::vector<uint8_t>& nodeData);
    void ParseWays(const std::vector<uint8_t>& wayData);
    void ParseRelations(const std::vector<uint8_t>& relationData);

    std::shared_ptr<OSMDataStore> m_dataStore;
    std::shared_ptr<ProgressTracker> m_progressTracker;
    std::atomic<bool> m_cancelled{false};
    size_t m_processedBlocks{0};
    std::string m_lastError;

    static constexpr size_t COMPRESSION_BUFFER_SIZE = 1024 * 1024; // 1MB buffer
    std::vector<uint8_t> m_decompressionBuffer;
    
    std::vector<uint8_t> DecompressBlock(const std::vector<uint8_t>& compressedData);

    mutable std::mutex m_mutex;

    std::unique_ptr<MemoryMappedFile> m_mappedFile;

    size_t m_totalFileSize{0};
    size_t m_processedBytes{0};

    static constexpr size_t BATCH_SIZE = 10000;
    ThreadPool m_threadPool;

    std::shared_ptr<SpatialIndex<OSMNode>> m_nodeIndex;
    std::shared_ptr<SpatialIndex<OSMWay>> m_wayIndex;
    std::shared_ptr<SpatialIndex<OSMRelation>> m_relationIndex;
}; 