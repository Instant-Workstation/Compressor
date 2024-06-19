#include <cassert>
#include <cstddef>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

enum Action
{
    Compress,
    Decompress
};

struct Command
{
    Action action = Action::Compress;
    std::string filePath = "enwik3";
};

Action GetAction(const std::vector<std::string>& cliArguments)
{
    std::size_t actionIndex = 1;
    std::string actionArgument = cliArguments.at(actionIndex);

    if (actionArgument == "-c" || actionArgument == "--compress")
    {
        return Action::Compress;
    }
    else if (actionArgument == "-d" || actionArgument == "--decompress")
    {
        return Action::Decompress;
    }
    else
    {
        throw std::runtime_error("Could not find command from command line arguments");
    }
}

std::string GetFilePath()
{
    return "";
}

Command GetCommand(const std::vector<std::string>& cliArguments)
{
    assert(GetAction({"Compressor", "-c", "enwik3"}) == Action::Compress);
    assert(GetAction({"Compressor", "-d", "enwik3"}) == Action::Decompress);
    assert(GetAction({"Compressor", "--compress", "enwik3"}) == Action::Compress);
    assert(GetAction({"Compressor", "--decompress", "enwik3"}) == Action::Decompress);

    Command command;

    command.action = GetAction(cliArguments);
    command.filePath = GetFilePath();

    return command;
}

void ExecuteCommand(Command command)
{
    return;
}

int main(int argc, char* argv[])
{
    try
    {
        std::vector<std::string> cliArguments;
        cliArguments.assign(argv, argv + argc);

        Command command = GetCommand(cliArguments);
        ExecuteCommand(command);

        return 0;
    }
    catch (std::exception exception)
    {
        std::cout << "Something went wrong: " << exception.what() << std::endl;
        return 1;
    }
}
