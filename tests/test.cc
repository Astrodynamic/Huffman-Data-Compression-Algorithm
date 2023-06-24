#include <gtest/gtest.h>

#include "huffman.h"

TEST(HUFFMAN, Huffman_test_1) {
  std::filesystem::path path_0{"file"};
  std::filesystem::path path_1{"file_encoded"};
  std::filesystem::path path_2{"file_encoded_decoded"};

  Huffman huffman;

  ASSERT_NO_THROW(huffman.Encode(path_0));
  ASSERT_NO_THROW(huffman.Decode(path_1));

  std::ifstream f1(path_0), f2(path_2);
  std::string content1((std::istreambuf_iterator<char>(f1)),
                       (std::istreambuf_iterator<char>())),
      content2((std::istreambuf_iterator<char>(f2)),
               (std::istreambuf_iterator<char>()));
  ASSERT_EQ(content1, content2);
}