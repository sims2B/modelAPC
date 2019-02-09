#include "utils.h"


void testFilename(bool withExtension) {
  std::cout << getFilename("/foo/bar.txt", withExtension) << '\n'
            << getFilename("/foo/.bar", withExtension) << '\n'
            << getFilename("/foo/bar/", withExtension) << '\n'
            << getFilename("/foo/.", withExtension) << '\n'
            << getFilename("/foo/..", withExtension) << '\n'
            << getFilename(".", withExtension) << '\n'
            << getFilename("..", withExtension) << '\n'
            << getFilename("/", withExtension) << '\n'
            << getFilename("//host", withExtension) << '\n';
}

int main()
{
  testFilename(true);
  testFilename(false);
}
