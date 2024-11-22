class PlayerbotLLMInterface
{
public:
    PlayerbotLLMInterface() {}

    static std::string Generate(const std::string& prompt, std::vector<std::string>& debugLines);

    static std::vector<std::string> ParseResponse(const std::string& response, const std::string& startPattern, const std::string& endPattern, const std::string& splitPattern, std::vector<std::string>& debugLines);

    static void LimitContext(std::string& context, uint32 currentLength);
private:
    std::atomic<uint32> generationCount;
};

#define sPlayerbotLLMInterface MaNGOS::Singleton<PlayerbotLLMInterface>::Instance()

