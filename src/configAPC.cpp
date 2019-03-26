#include "configAPC.h"

bool ConfigAPC::readFile(std::string configPath) {
  try {
    config.readFile(configPath.c_str());
    return (true);
  } catch (const libconfig::FileIOException& fioex) {
    std::cerr << "I/O error while reading file." << std::endl;
    return (false);
  } catch (const libconfig::ParseException& pex) {
    std::cerr << "Parse error at " << pex.getFile() << ":" << pex.getLine()
              << " - " << pex.getError() << std::endl;
    return (false);
  }
}

std::string ConfigAPC::getConfigName() {
  const libconfig::Setting& root = config.getRoot();
  try {
    std::string str;
    if (root.lookupValue("name", str)) {
      return str;
    }
  } catch (const libconfig::SettingNotFoundException& nfex) {
    // Ignored
  }
  return ("NO_NAME");
}

bool ConfigAPC::getBoolValue(const char* name1, const char* name2,
                             bool defVal) {
  const libconfig::Setting& root = config.getRoot();
  try {
    bool boolVal;
    if (root[name1].lookupValue(name2, boolVal)) {
      return boolVal;
    }
  } catch (const libconfig::SettingNotFoundException& nfex) {
    // Ignored
  }
  return (defVal);
}

int ConfigAPC::getIntValue(const char* name1, const char* name2, int defVal) {
  const libconfig::Setting& root = config.getRoot();
  try {
    int intVal;
    if (root[name1].lookupValue(name2, intVal)) {
      return intVal;
    }
  } catch (const libconfig::SettingNotFoundException& nfex) {
    // Ignored
  }
  return (defVal);
}

std::string ConfigAPC::getStringValue(const char* name1, const char* name2,
                                      std::string defVal) {
  const libconfig::Setting& root = config.getRoot();
  try {
    std::string str;
    if (root[name1].lookupValue(name2, str)) {
      return str;
    }
  } catch (const libconfig::SettingNotFoundException& nfex) {
    // Ignored
  }
  return (defVal);
}

std::vector<std::string> ConfigAPC::getHeuristics() {
  std::vector<std::string> heuristics;

  try {
    const libconfig::Setting& root = config.getRoot();
    const libconfig::Setting& rootSolver = root["solver"];
    const libconfig::Setting& hsettings = rootSolver.lookup("heuristics");
    for (int i = 0; i < hsettings.getLength(); ++i) {
      heuristics.push_back(hsettings[i].c_str());
    }
  } catch (const libconfig::SettingNotFoundException& nfex) {
    // Ignored
  }
  return (heuristics);
}

void ConfigAPC::toDimacs() {
  // TODO Add Config Name !
  std::cout << "c CONFIG_NAME " << getConfigName() << std::endl
            << "c SOLVER " << getSolverType() << std::endl
            << "c O_FUNC " << getObjectiveFunction() << std::endl
            << "c O_FLOW " << getWeightFlowtime() << std::endl
            << "c O_QUAL " << getWeightQualified() << std::endl;
}
