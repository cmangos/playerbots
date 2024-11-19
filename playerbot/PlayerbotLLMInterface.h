class PlayerbotLLMInterface
{
public:
    PlayerbotLLMInterface() {}
    static std::string Generate(const std::string& prompt);

    static std::vector<std::string> ParseResponse(const std::string& response, std::string startPattern, std::string endPattern);
};