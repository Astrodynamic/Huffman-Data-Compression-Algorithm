#include "interface.h"

const std::vector<std::string> Interface::AbstractInterface::m_menus{
    " -------------------------------------------------------------- \n"
    "|                     Huffman algorithm 1.0                    |\n"
    " -------------------------------------------------------------- \n",
    " -------------------------------------------------------------- \n"
    "|                       Select menu item                       |\n"
    " -------------------------------------------------------------- \n"
    "| 0. Exit                                                      |\n"
    "| 1. Encoding information by the Huffman method                |\n"
    "| 2. Decoding information by the Huffman method                |\n"
    "| 3. Upload configuration file for decryption                  |\n"
    "| 4. Setting the default path to save files                    |\n"
    " -------------------------------------------------------------- \n"
    " > ",
    " -------------------------------------------------------------- \n"
    "|                       Select menu item                       |\n"
    " -------------------------------------------------------------- \n"
    "| 0. Exit                                                      |\n"
    "| 1. Enter the full path to the encoded(decoded) file ...      |\n"
    " -------------------------------------------------------------- \n"
    " > ",
    " -------------------------------------------------------------- \n"
    "|                       Select menu item                       |\n"
    " -------------------------------------------------------------- \n"
    "| 0. Exit                                                      |\n"
    "| 1. Enter the full path to the config file ...                |\n"
    " -------------------------------------------------------------- \n"
    " > ",
    " -------------------------------------------------------------- \n"
    "|                       Select menu item                       |\n"
    " -------------------------------------------------------------- \n"
    "| 0. Exit                                                      |\n"
    "| 1. Enter the full path to the save directory                 |\n"
    " -------------------------------------------------------------- \n"
    " > ",
    " -------------------------------------------------------------- \n"
    "|       A file(directory) with that name does not exist        |\n"
    " -------------------------------------------------------------- \n",
    " -------------------------------------------------------------- \n"
    "|      Missing configuration file or valid save directory      |\n"
    " -------------------------------------------------------------- \n",
    " -------------------------------------------------------------- \n"
    "|            Successful completion of the programme            |\n"
    " -------------------------------------------------------------- \n"};

Interface::Interface() {
  InitFuncMenus();
  ShowMenu(m_menus[MenuItem::kIntroduction]);
}

void Interface::InitFuncMenus() {
  m_funcs.resize(MenuFuncs::kMenuFuncsAll);

  m_funcs[MenuFuncs::kMainFuncMenu] = {
      std::bind(&Interface::Exit, this),
      std::bind(&Interface::RunMenu, this,
                std::ref(m_funcs[MenuFuncs::kEncode]), MenuItem::kLoadCodeMenu),
      std::bind(&Interface::RunMenu, this,
                std::ref(m_funcs[MenuFuncs::kDecode]), MenuItem::kLoadCodeMenu),
      std::bind(&Interface::RunMenu, this,
                std::ref(m_funcs[MenuFuncs::kUploadConfig]),
                MenuItem::kLoadConfigMenu),
      std::bind(&Interface::RunMenu, this,
                std::ref(m_funcs[MenuFuncs::kSaveDirectory]),
                MenuItem::kSaveDirectoryMenu)};

  m_funcs[MenuFuncs::kEncode] = {
      std::bind(&Interface::Exit, this), [this]() -> bool {
        return RunProcessFile(std::bind(&Huffman::Encode, std::ref(m_model),
                                        std::placeholders::_1));
      }};

  m_funcs[MenuFuncs::kDecode] = {
      std::bind(&Interface::Exit, this), [this]() -> bool {
        return RunProcessFile(std::bind(&Huffman::Decode, std::ref(m_model),
                                        std::placeholders::_1));
      }};

  m_funcs[MenuFuncs::kUploadConfig] = {
      std::bind(&Interface::Exit, this), [this]() -> bool {
        return RunProcessFile(std::bind(
            &Huffman::UploadConfig, std::ref(m_model), std::placeholders::_1));
      }};

  m_funcs[MenuFuncs::kSaveDirectory] = {
      std::bind(&Interface::Exit, this), [this]() -> bool {
        return RunProcessFile(std::bind(&Huffman::SetSaveDirectory,
                                        std::ref(m_model),
                                        std::placeholders::_1));
      }};
}

bool Interface::RunProcessFile(
    std::function<bool(const std::filesystem::path&)> func) {
  const auto [success, path]{CheckInputPathFile()};
  if (success) {
    if (!func(path)) {
      ShowMenu(m_menus[MenuItem::kMissingMenu]);
    }
  } else {
    ShowMenu(m_menus[MenuItem::kNotExistMenu]);
  }
  return !success;
}

Interface::~Interface() { ShowMenu(m_menus[MenuItem::kCompletion]); }

void Interface::Exec() {
  RunMenu(m_funcs[MenuFuncs::kMainFuncMenu], MenuItem::kMainMenu);
}