/* ----------------------------------------------------------------------------
   libconfig - A library for processing structured configuration files
   Copyright (C) 2005-2010  Mark A Lindner

   This file is part of libconfig.

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public License
   as published by the Free Software Foundation; either version 2.1 of
   the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful, but
   WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Library General Public
   License along with this library; if not, see
   <http://www.gnu.org/licenses/>.
   ----------------------------------------------------------------------------
*/

#include <iostream>
#include <cstdlib>
#include "solverAPC.h"

using namespace std;
using namespace libconfig;

// This example reads the configuration file 'example.cfg' and displays
// some of its contents.

int main(int argc, char **argv)
{
  ConfigAPC cfg;
  cfg.readFile("example.cfg");
  cfg.toDimacs();
  for(auto& h : cfg.getHeuristics()) {
    std::cout << h << std::endl;
  }
  return(EXIT_SUCCESS);
}

// eof
