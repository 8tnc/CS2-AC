#include "localization.h"

#include "KeyValues.h"
#include "filesystem.h"

#include <cctype>

namespace
{
	std::unordered_map<std::string, std::string> phrases;
	std::string currentLanguage {"en"};
	bool fallbackWarningPrinted;

	void WarnFallback()
	{
		if (!fallbackWarningPrinted)
		{
			Msg("[CS2AC] Some phrases for language '%s' are missing or invalid. English will be used for them.\n", currentLanguage.c_str());
			fallbackWarningPrinted = true;
		}
	}

	std::string NormalizeLanguage(const char *language)
	{
		std::string result = language ? language : "";
		if (result.empty() || result.size() > 16)
		{
			return {};
		}
		for (char &character : result)
		{
			if (character == '_')
			{
				character = '-';
			}
			else if (std::isalnum(static_cast<unsigned char>(character)))
			{
				character = static_cast<char>(std::tolower(static_cast<unsigned char>(character)));
			}
			else if (character != '-')
			{
				return {};
			}
		}
		return result;
	}

	std::set<std::string> Placeholders(std::string_view text)
	{
		std::set<std::string> result;
		for (std::size_t start = text.find('{'); start != std::string_view::npos; start = text.find('{', start + 1))
		{
			const std::size_t end = text.find('}', start + 1);
			if (end == std::string_view::npos)
			{
				break;
			}
			const std::string_view name = text.substr(start + 1, end - start - 1);
			if (!name.empty()
				&& std::all_of(name.begin(), name.end(),
							   [](unsigned char character) { return std::isalnum(character) || character == '_' || character == '.'; }))
			{
				result.emplace(name);
			}
		}
		return result;
	}

	std::string Apply(std::string_view text, localization::Arguments arguments)
	{
		std::string result;
		result.reserve(text.size() + 32);
		for (std::size_t index = 0; index < text.size();)
		{
			if (text[index] != '{')
			{
				result += text[index++];
				continue;
			}
			const std::size_t end = text.find('}', index + 1);
			if (end == std::string_view::npos)
			{
				result.append(text.substr(index));
				break;
			}
			const std::string_view name = text.substr(index + 1, end - index - 1);
			const auto found =
				std::find_if(arguments.begin(), arguments.end(), [name](const localization::Argument &argument) { return argument.name == name; });
			if (found == arguments.end())
			{
				result.append(text.substr(index, end - index + 1));
			}
			else
			{
				result += found->value;
			}
			index = end + 1;
		}
		return result;
	}
} // namespace

void localization::Reload(const char *language)
{
	phrases.clear();
	fallbackWarningPrinted = false;
	currentLanguage = NormalizeLanguage(language);
	if (currentLanguage.empty())
	{
		currentLanguage = "en";
		WarnFallback();
		return;
	}
	if (!g_pFullFileSystem)
	{
		WarnFallback();
		return;
	}

	const std::string path = "addons/cs2ac/translations/" + currentLanguage + ".txt";
	KeyValues file("Phrases");
	if (!file.LoadFromFile(g_pFullFileSystem, path.c_str(), nullptr))
	{
		WarnFallback();
		return;
	}
	for (KeyValues *phrase = file.GetFirstValue(); phrase; phrase = phrase->GetNextValue())
	{
		const char *value = phrase->GetString(nullptr);
		if (value && *value)
		{
			phrases[phrase->GetName()] = value;
		}
	}
}

void localization::Shutdown()
{
	phrases.clear();
	currentLanguage = "en";
	fallbackWarningPrinted = false;
}

const char *localization::CurrentLanguage()
{
	return currentLanguage.c_str();
}

std::string localization::Get(const char *key, const char *english)
{
	const std::string fallback = english ? english : "";
	const auto found = key ? phrases.find(key) : phrases.end();
	if (found == phrases.end() || Placeholders(found->second) != Placeholders(fallback))
	{
		if (found == phrases.end() || found->second != fallback)
		{
			WarnFallback();
		}
		return fallback;
	}
	return found->second;
}

localization::Text localization::Format(const char *key, const char *english, Arguments arguments)
{
	return {Apply(english ? english : "", arguments), Apply(Get(key, english), arguments)};
}
