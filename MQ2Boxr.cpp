#include "boxr.h"
#include "boxr_type.h"

PLUGIN_VERSION(0.1);
PreSetup("MQ2Boxr");

std::vector<std::string> getArgVector(const std::string& argsString, const std::string& delimiter = " ") {
	auto args = std::vector<std::string>();
	size_t nextDelimiterStartPos;
	size_t nextTokenStartPos = 0;
	while ((nextDelimiterStartPos = argsString.find(delimiter, nextTokenStartPos)) != std::string::npos) {
		size_t tokenLength = nextDelimiterStartPos - nextTokenStartPos;
		args.push_back(argsString.substr(nextTokenStartPos, tokenLength));
		nextTokenStartPos = nextDelimiterStartPos + delimiter.length();
	}
	args.push_back(argsString.substr(nextTokenStartPos));
	return args;
}

bool iStrEquals(const char* arg1, const std::string& arg2) {
	return _strcmpi(arg1, arg2.c_str()) == 0;
}

bool parseBoolArg(std::string arg) {
	if (iStrEquals("on", arg) || iStrEquals("true", arg) || iStrEquals("yes", arg) || iStrEquals("y", arg) ||
		iStrEquals("1", arg)) {
		return true;
	}
	else if (iStrEquals("off", arg) || iStrEquals("false", arg) || iStrEquals("no", arg) || iStrEquals("n", arg) ||
		iStrEquals("0", arg)) {
		return false;
	}
	throw std::invalid_argument("Invalid argument. Valid values are: [on, off, true, false, yes, no, 1, 0]");
}

void printUsage() {
	LOGGER.info("Boxr commands:\n"
		"\a-t|\ax  \ay/boxr Pause\ax - Pauses the character\n"
		"\a-t|\ax  \ay/boxr Unpause\ax - Unpause the character\n"
		"\a-t|\ax  \ay/boxr Chase\ax - Sets navivation to chase assisted character, and stop camping\n"
		"\a-t|\ax  \ay/boxr Camp\ax - Sets navigation to camp at current position, and return to camp after combat\n"
		"\a-t|\ax  \ay/boxr Manual\ax - Sets manual navigation (don't chase, no camp)\n"
		"\a-t|\ax  \ay/boxr BurnNow\ax - Burn current target\n"
		"\a-t|\ax  \ay/boxr BurnOff\ax - Turn Burns Off\n"
		"\a-t|\ax  \ay/boxr BurnNamed\ax - Burn Named targets\n"
#if !defined(ROF2EMU) && !defined(UFEMU)
		"\a-t|\ax  \ay/boxr RaidAssistNum <1, 2, 3>\ax - Toggles which Raid MA to assist\n"
#endif
		"\a-t|\ax  \ay/boxr Debug \a-y[on|off]\ax\ax - Toggles MQ2Boxr debug logging\n"
		"\a-t|\ax  \ay/boxr Help\ax - Prints this help\n"
	);
}

void BoxrCommand(SPAWNINFO* pChar, char* szLine) {
	if (GetGameState() != GAMESTATE_INGAME) {
		LOGGER.error("Was asked to do '{}', but am not in game");
		return;
	}

	auto argVector = getArgVector(szLine);

	if (argVector.empty() || iStrEquals("help", argVector.front())) {
		printUsage();
	} else if (iStrEquals("pause", argVector.front())) {
		MasterBoxControl::getInstance().Pause();
	} else if (iStrEquals("unpause", argVector.front())) {
		MasterBoxControl::getInstance().Unpause();
	} else if (iStrEquals("chase", argVector.front())) {
		MasterBoxControl::getInstance().Chase();
	} else if (iStrEquals("camp", argVector.front())) {
		MasterBoxControl::getInstance().Camp();
	} else if (iStrEquals("manual", argVector.front())) {
		MasterBoxControl::getInstance().Manual();
	} else if (iStrEquals("burnnow", argVector.front())) {
		MasterBoxControl::getInstance().BurnNow();
	} else if (iStrEquals("burnoff", argVector.front())) {
		MasterBoxControl::getInstance().BurnOff();
	} else if (iStrEquals("burnnamed", argVector.front())) {
		MasterBoxControl::getInstance().BurnNamed();
	} else if (iStrEquals("raidassistnum", argVector.front())) {
		if (argVector.size() != 2) {
			LOGGER.error("/boxr RaidAssistNum: expected exactly one argument, but got {}", argVector.size() - 1);
			return;
		}
		try {
			int raidAssistNum = std::stoi(argVector.at(1));
			if (raidAssistNum < 1 || raidAssistNum > 3) {
				LOGGER.error("/boxr RaidAssistNum: RaidAssistNum must be either 1, 2, or 3.");
				return;
			}
#if defined(ROF2EMU) || defined(UFEMU)
			LOGGER.error("/boxr RaidAssistNum is not supported on EMU");
			return;
#else
			if (!GetCharInfo()->raidData.MainAssistNames[raidAssistNum - 1]) {
				LOGGER.error("/boxr RaidAssistNum: There is no main assist {}", raidAssistNum);
				return;
			}
#endif
			MasterBoxControl::getInstance().RaidAssistNum(raidAssistNum);
		} catch (std::invalid_argument& e) {
			UNREFERENCED_PARAMETER(e);
			LOGGER.error("/boxr raidassistnum: invalid argument - expected 1, 2, or 3, but got: {}", argVector.at(1).c_str());
		}
	} else if (iStrEquals("debug", argVector.front())) {
		try {
			if (argVector.size() == 1) {
				LOGGER.toggleDebugEnabled();
			} else {
				LOGGER.setDebugEnabled(parseBoolArg(argVector.at(1)));
			}
			LOGGER.info("Debug logging {}", LOGGER.isDebugEnabled() ? "enabled" : "disabled");
		} catch (std::invalid_argument& e) {
			LOGGER.error("/boxr debug: {}", e.what());
		}
	} else {
		LOGGER.info("Do not understand command: {}", szLine);
	}
}

// Called once, when the plugin is to initialize
PLUGIN_API VOID InitializePlugin(VOID) {
	DebugSpewAlways("Initializing MQ2Boxr");
	MQ2BoxrType::RegisterBoxrType();
	AddCommand("/boxr", BoxrCommand);
}

// Called once, when the plugin is to shutdown
PLUGIN_API VOID ShutdownPlugin(VOID) {
	DebugSpewAlways("Shutting down MQ2Boxr");
	RemoveCommand("/boxr");
	MQ2BoxrType::UnregisterBoxrType();
}
