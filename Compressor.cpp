#include <cassert>
#include <cstddef>
#include <fstream>
#include <iostream>
#include <iterator>
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
    std::string target = "enwik3";

    bool operator==(const Command& otherCommand)
    {
        return action == otherCommand.action && target == otherCommand.target;
    }

    Command() = default;
    Command(Action action, std::string target)
    {
        this->action = action;
        this->target = target;
    }
};

std::string GetUsage() {
    std::string usage = "Usage: ./Compressor <command> <target>\n\n";
    usage += "Commands:\n";
    usage += "  -c --compress   Compress target\n";
    usage += "  -d --decompress Decompress target\n\n";
    usage += "Examples:\n";
    usage += "  Compress the file enwik3:\n";
    usage += "    ./Compressor -c enwik3\n";
    usage += "  Decompress the file enwik3.iw:\n";
    usage += "    ./Compressor -d enwik3.iw";

    return usage;
}

void ValidateArguments(const std::vector<std::string>& cliArguments) {
    const std::size_t expectedArguments = 3;

    if (cliArguments.size() != expectedArguments) {
        throw std::runtime_error("Invalid number of command line arguments\n\n" + GetUsage());
    }
}

Action GetAction(const std::vector<std::string>& cliArguments)
{
    const std::size_t actionIndex = 1;
    const std::string actionArgument = cliArguments.at(actionIndex);

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
        throw std::runtime_error(actionArgument + " is not a valid command\n\n" + GetUsage());
    }
}

std::string GetTarget(const std::vector<std::string>& cliArguments)
{
    const std::size_t targetIndex = 2;
    return cliArguments.at(targetIndex);
}

std::vector<unsigned char> ReadTarget(const std::string& target)
{
    std::ifstream inputFile(target, std::ios::binary);

    std::vector<unsigned char> inputBytes(
        (std::istreambuf_iterator<char>(inputFile)),
        (std::istreambuf_iterator<char>()));

    return inputBytes;
}

void ProcessTarget()
{
    return;
}

Command GetCommand(const std::vector<std::string>& cliArguments)
{
    assert(GetAction({"Compressor", "-c", "enwik3"}) == Action::Compress);
    assert(GetAction({"Compressor", "-d", "enwik3"}) == Action::Decompress);
    assert(GetAction({"Compressor", "--compress", "enwik3"}) == Action::Compress);
    assert(GetAction({"Compressor", "--decompress", "enwik3"}) == Action::Decompress);

    assert(GetTarget({"Compressor", "-c", "enwik3"}) == "enwik3");
    assert(GetTarget({"Compressor", "-c", "enwik5"}) == "enwik5");
    assert(GetTarget({"Compressor", "-c", "enwik7"}) == "enwik7");
    assert(GetTarget({"Compressor", "-c", "enwik9"}) == "enwik9");

    Command command;

    command.action = GetAction(cliArguments);
    command.target = GetTarget(cliArguments);

    return command;
}

void ExecuteCommand(const Command& command)
{
    std::vector<unsigned char> inputBytes = ReadTarget(command.target);
    ProcessTarget();

    return;
}

int main(int argc, char* argv[])
{
    assert(GetCommand({"Compressor", "-c", "enwik3"}) == Command(Action::Compress, "enwik3"));
    assert(GetCommand({"Compressor", "-d", "enwik5"}) == Command(Action::Decompress, "enwik5"));
    assert(GetCommand({"Compressor", "--compress", "enwik7"}) == Command(Action::Compress, "enwik7"));
    assert(GetCommand({"Compressor", "--decompress", "enwik9"}) == Command(Action::Decompress, "enwik9"));

    try
    {
        std::vector<std::string> cliArguments;
        cliArguments.assign(argv, argv + argc);
        ValidateArguments(cliArguments);

        Command command = GetCommand(cliArguments);
        ExecuteCommand(command);

        return 0;
    }
    catch (const std::exception& exception)
    {
        std::cout << "Something went wrong: " << exception.what() << std::endl;
        return 1;
    }
}
