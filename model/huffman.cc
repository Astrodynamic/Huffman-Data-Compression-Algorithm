#include "huffman.h"

Huffman::HuffmanNode::HuffmanNode(std::byte data, std::size_t freq)
    : data{data}, freq{freq} {}

Huffman::HuffmanNode::HuffmanNode(std::shared_ptr<HuffmanNode> left,
                                  std::shared_ptr<HuffmanNode> right)
    : left{left}, right{right} {
  this->freq = left->freq + right->freq;
}

bool Huffman::HuffmanNode::Leaf() const { return (!left && !right); }

Huffman::Huffman()
    : m_conf_path(std::filesystem::current_path() / "huffman.cfg"),
      m_dirr_path(std::filesystem::current_path()) {}

bool Huffman::Encode(const std::filesystem::path &path) {
  bool flag{std::filesystem::exists(path)};
  if (flag = (flag && std::filesystem::is_regular_file(path)), flag) {
    std::ifstream ifs(path, std::ios::binary);

    std::string file = path.stem().string();
    file += std::string("_encoded");
    file += path.extension().string();
    std::ofstream ofs(m_dirr_path / file, std::ios::binary);

    std::map<std::byte, std::size_t> freq = CreateFrequencyMap(ifs);
    std::shared_ptr<HuffmanNode> root = CreateHuffmanTree(freq);
    std::unordered_map<std::byte, std::vector<bool>> codes =
        CreateCodesMap(root);
    WriteConfigFile(codes);
    ifs.clear();
    ifs.seekg(0, std::ios::beg);
    WriteCompressedData(ifs, ofs, codes);
    ifs.close();
    ofs.close();
  }
  return flag;
}

bool Huffman::Decode(const std::filesystem::path &path) {
  bool flag{std::filesystem::exists(path)};
  if (flag = (flag && std::filesystem::exists(m_conf_path) &&
              std::filesystem::exists(m_conf_path)),
      flag) {
    std::ifstream ifs(path, std::ios::binary);

    std::string file = path.stem().string();
    file += std::string("_decoded");
    file += path.extension().string();
    std::ofstream ofs(m_dirr_path / file, std::ios::binary);

    std::shared_ptr<HuffmanNode> root = ReadConfigFile();
    ReadCompressedData(ifs, ofs, root);
    ifs.close();
    ofs.close();
  }
  return flag;
}

bool Huffman::UploadConfig(const std::filesystem::path &path) {
  bool flag{std::filesystem::exists(path)};
  if (flag = (flag && std::filesystem::is_regular_file(path) &&
              path.extension() == ".conf"),
      flag) {
    m_conf_path = path;
  }
  return flag;
}

bool Huffman::SetSaveDirectory(const std::filesystem::path &path) {
  bool flag{std::filesystem::exists(path)};
  if (flag = (flag && std::filesystem::is_directory(path)), flag) {
    m_dirr_path = path;
  }
  return flag;
}

std::map<std::byte, std::size_t> Huffman::CreateFrequencyMap(
    std::ifstream &ifs) {
  std::map<std::byte, std::size_t> freq;
  for (std::byte byte; ifs.read(reinterpret_cast<char *>(&byte), sizeof(byte));
       ++freq[byte])
    ;
  return freq;
}

std::shared_ptr<Huffman::HuffmanNode> Huffman::CreateHuffmanTree(
    std::map<std::byte, std::size_t> &freq) {
  struct CompareHuffmanNodes {
    bool operator()(const std::shared_ptr<HuffmanNode> &lhs,
                    const std::shared_ptr<HuffmanNode> &rhs) const {
      return lhs->freq > rhs->freq;
    }
  };
  std::priority_queue<std::shared_ptr<HuffmanNode>,
                      std::vector<std::shared_ptr<HuffmanNode>>,
                      CompareHuffmanNodes>
      pq;
  for (auto &[byte, frequency] : freq) {
    pq.push(std::make_shared<HuffmanNode>(byte, frequency));
  }
  while (pq.size() > 1) {
    auto lhs = pq.top();
    pq.pop();
    auto rhs = pq.top();
    pq.pop();
    pq.push(std::make_shared<HuffmanNode>(lhs, rhs));
  }
  return pq.top();
}

std::unordered_map<std::byte, std::vector<bool>> Huffman::CreateCodesMap(
    const std::shared_ptr<HuffmanNode> &root) {
  std::unordered_map<std::byte, std::vector<bool>> codes;
  std::vector<bool> bits;
  TraverseHuffmanTree(root, bits, codes);
  return codes;
}

void Huffman::TraverseHuffmanTree(
    const std::shared_ptr<HuffmanNode> &node, std::vector<bool> &bits,
    std::unordered_map<std::byte, std::vector<bool>> &codes) {
  if (node->Leaf()) {
    codes[node->data] = bits;
    return;
  }
  bits.push_back(false);
  TraverseHuffmanTree(node->left, bits, codes);
  bits.pop_back();
  bits.push_back(true);
  TraverseHuffmanTree(node->right, bits, codes);
  bits.pop_back();
}

void Huffman::WriteConfigFile(
    const std::unordered_map<std::byte, std::vector<bool>> &codes) {
  std::ofstream config(m_conf_path, std::ios::binary);
  std::uint8_t buffer = 0;
  std::uint32_t buffer_size = 0;
  for (auto &[byte, bits] : codes) {
    config.write(reinterpret_cast<const char *>(&byte), sizeof(byte));
    std::uint32_t bits_size = static_cast<std::uint32_t>(bits.size());
    config.write(reinterpret_cast<const char *>(&bits_size), sizeof(bits_size));
    for (std::uint32_t i = 0; i < bits_size; ++i) {
      buffer |= (static_cast<int>(bits[i]) << (7 - buffer_size));
      if ((++buffer_size) == 8) {
        config.write(reinterpret_cast<const char *>(&buffer), sizeof(buffer));
        buffer = 0;
        buffer_size = 0;
      }
    }
    if (buffer_size) {
      config.write(reinterpret_cast<const char *>(&buffer), sizeof(buffer));
      buffer = 0;
      buffer_size = 0;
    }
  }
  config.close();
}

void Huffman::WriteCompressedData(
    std::ifstream &ifs, std::ofstream &ofs,
    std::unordered_map<std::byte, std::vector<bool>> &codes) {
  std::uint8_t buffer = 0;
  std::size_t buffer_size = 0;

  for (std::byte byte;
       ifs.read(reinterpret_cast<char *>(&byte), sizeof(byte));) {
    const auto &bits = codes[byte];
    for (bool bit : bits) {
      buffer |= (static_cast<std::uint8_t>(bit) << (7 - buffer_size));
      if ((++buffer_size) == 8) {
        ofs.write(reinterpret_cast<const char *>(&buffer), sizeof(buffer));
        buffer = 0;
        buffer_size = 0;
      }
    }
  }
  if (buffer_size) {
    ofs.write(reinterpret_cast<const char *>(&buffer), sizeof(buffer));
  }
}

std::shared_ptr<Huffman::HuffmanNode> Huffman::ReadConfigFile() {
  std::ifstream config(m_conf_path, std::ios::binary);
  std::shared_ptr<HuffmanNode> root = std::make_shared<HuffmanNode>();

  std::byte byte;
  std::vector<bool> bits;
  std::uint32_t bits_size;
  while (config.peek() != std::ifstream::traits_type::eof()) {
    config.read(reinterpret_cast<char *>(&byte), sizeof(byte));
    config.read(reinterpret_cast<char *>(&bits_size), sizeof(bits_size));

    bits.resize(bits_size);
    std::uint8_t buffer = 0;
    std::uint32_t buffer_size = 0;
    for (std::uint32_t i = 0; i < bits_size; ++i) {
      if (buffer_size == 0) {
        config.read(reinterpret_cast<char *>(&buffer), sizeof(buffer));
      }
      bits[i] = ((buffer >> (7 - buffer_size++)) & 1);
      buffer_size %= 8;
    }
    RestoreTree(root, byte, bits);
  }

  config.close();
  return root;
}

void Huffman::RestoreTree(std::shared_ptr<Huffman::HuffmanNode> node,
                          std::byte byte, const std::vector<bool> &bits) {
  for (bool bit : bits) {
    if (bit) {
      if (!node->right) {
        node->right = std::make_shared<Huffman::HuffmanNode>();
      }
      node = node->right;
    } else {
      if (!node->left) {
        node->left = std::make_shared<Huffman::HuffmanNode>();
      }
      node = node->left;
    }
  }
  node->data = byte;
}

void Huffman::ReadCompressedData(std::ifstream &ifs, std::ofstream &ofs,
                                 const std::shared_ptr<HuffmanNode> &root) {
  std::shared_ptr<HuffmanNode> node = root;
  for (std::uint8_t byte;
       ifs.read(reinterpret_cast<char *>(&byte), sizeof(byte));) {
    for (std::size_t i = 0; i < 8; ++i) {
      bool bit = ((byte >> (7 - i)) & 1);
      if (node->Leaf()) {
        ofs.write(reinterpret_cast<const char *>(&node->data),
                  sizeof(node->data));
        node = root;
      }
      node = bit ? node->right : node->left;
    }
  }
}