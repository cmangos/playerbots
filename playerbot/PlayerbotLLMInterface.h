namespace ai
{
    class PlayerbotLLMInterface
    {
    public:
        PlayerbotLLMInterface() {}
        static std::string Generate(const std::string& prompt);
    private:

    };
}