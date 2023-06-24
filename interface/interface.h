#pragma once

#include "ainterface.h"
#include "huffman.h"

class Interface final : virtual public AbstractInterface {
 public:
  Interface();
  explicit Interface(const Interface &other) = delete;
  explicit Interface(Interface &&other) = delete;
  Interface &operator=(const Interface &other) = delete;
  Interface &operator=(Interface &&other) = delete;
  ~Interface();

  virtual void Exec() final override;

 private:
  enum MenuFuncs : std::size_t {
    kMainFuncMenu = 0U,
    kEncode,
    kDecode,
    kUploadConfig,
    kSaveDirectory,
    kMenuFuncsAll
  };

  enum MenuItem : std::size_t {
    kIntroduction = 0U,
    kMainMenu,
    kLoadCodeMenu,
    kLoadConfigMenu,
    kSaveDirectoryMenu,
    kNotExistMenu,
    kMissingMenu,
    kCompletion
  };

  Huffman m_model;

  void InitFuncMenus();
  bool RunProcessFile(std::function<bool(const std::filesystem::path &)> func);
};