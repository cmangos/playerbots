class PlayerbotLLMInterface
{
public:
    PlayerbotLLMInterface() {}
    static std::string Generate(const std::string& prompt);

    static std::vector<std::string> ParseResponse(const std::string& response, const std::string& startPattern, const std::string& endPattern, const std::string& splitPattern);

    static void LimitContext(std::string& context, uint32 currentLength);
};