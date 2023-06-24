#pragma once

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <map>
#include <memory>
#include <queue>
#include <unordered_map>

class Huffman {
 public:
  Huffman();
  Huffman(const Huffman &other) = delete;
  Huffman(Huffman &&other) = delete;
  Huffman &operator=(const Huffman &other) = delete;
  Huffman &operator=(Huffman &&other) = delete;
  ~Huffman() = default;

  bool Encode(const std::filesystem::path &path);
  bool Decode(const std::filesystem::path &path);
  bool UploadConfig(const std::filesystem::path &path);
  bool SetSaveDirectory(const std::filesystem::path &path);

 private:
  struct HuffmanNode {
    std::byte data{};
    std::size_t freq{0};
    std::shared_ptr<HuffmanNode> left{nullptr};
    std::shared_ptr<HuffmanNode> right{nullptr};

    HuffmanNode() = default;
    HuffmanNode(std::byte data, std::size_t freq);
    HuffmanNode(std::shared_ptr<HuffmanNode> left,
                std::shared_ptr<HuffmanNode> right);
    ~HuffmanNode() = default;

    bool Leaf() const;
  };

  std::filesystem::path m_conf_path;
  std::filesystem::path m_dirr_path;

  std::shared_ptr<HuffmanNode> ReadConfigFile();
  std::map<std::byte, std::size_t> CreateFrequencyMap(std::ifstream &ifs);
  void WriteConfigFile(
      const std::unordered_map<std::byte, std::vector<bool>> &codes);
  std::shared_ptr<HuffmanNode> CreateHuffmanTree(
      std::map<std::byte, std::size_t> &freq);
  void ReadCompressedData(std::ifstream &ifs, std::ofstream &ofs,
                          const std::shared_ptr<HuffmanNode> &root);
  std::unordered_map<std::byte, std::vector<bool>> CreateCodesMap(
      const std::shared_ptr<HuffmanNode> &root);
  void RestoreTree(std::shared_ptr<Huffman::HuffmanNode> node, std::byte byte,
                   const std::vector<bool> &bits);
  void WriteCompressedData(
      std::ifstream &ifs, std::ofstream &ofs,
      std::unordered_map<std::byte, std::vector<bool>> &codes);
  void TraverseHuffmanTree(
      const std::shared_ptr<HuffmanNode> &node, std::vector<bool> &bits,
      std::unordered_map<std::byte, std::vector<bool>> &codes);
};