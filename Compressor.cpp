#include <cassert>
#include <cstddef>
#include <fstream>
#include <iostream>
#include <iterator>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <vector>

enum Model
{
    Statistics,
    HistoricDictionary,
    FutureDictionary,
    Distance
};

enum Action
{
    Compress,
    Decompress
};

struct History
{
    std::unordered_map<std::size_t, std::size_t> historicData;
    std::unordered_map<std::size_t, std::size_t> performance;
};

struct PredictionModel
{
    Model model = Model::Statistics;
    History history;

    PredictionModel() = default;
    PredictionModel(Model model)
    {
        this->model = model;
    }
};

struct GuessResult
{
    bool correct = true;
    std::vector<unsigned char> guessedBits{1};
};

struct Command
{
    Action action = Action::Compress;
    std::string target = "enwik1";

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

struct Operation
{
    Command command = Command(Action::Compress, "enwik1");
    std::vector<unsigned char> inputBytes{'<', 'm', 'e', 'd', 'i', 'a', 'w', 'i', 'k', 'i'};
};

struct Predictor
{
    std::size_t inputPosition = 0;
    Operation operation;
};

GuessResult MakeGuess(const std::vector<PredictionModel>& predictionModels)
{
    return GuessResult();
}

void RecordGuess()
{
    return;
}

void RecordHistory()
{
    return;
}

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

    if (inputFile.fail())
    {
        throw std::runtime_error("Could not read from file " + target);
    }

    std::vector<unsigned char> inputBytes(
        (std::istreambuf_iterator<char>(inputFile)),
        (std::istreambuf_iterator<char>()));

    return inputBytes;
}

void ProcessTarget(const Operation& operation)
{
    const std::size_t numberBits = 8;

    std::size_t correctBits = 0;
    std::vector<PredictionModel> predictionModels;
    std::vector<unsigned char> outputBytes;

    predictionModels.push_back(PredictionModel(Model::Statistics));
    predictionModels.push_back(PredictionModel(Model::HistoricDictionary));
    predictionModels.push_back(PredictionModel(Model::FutureDictionary));
    predictionModels.push_back(PredictionModel(Model::Distance));

    while (correctBits != operation.inputBytes.size() * numberBits)
    {
        GuessResult guessResult = MakeGuess(predictionModels);
        RecordGuess();
        RecordHistory();

        if (guessResult.correct)
        {
            correctBits += guessResult.guessedBits.size();
        }
    }

    return;
}

void ValidateArguments(const std::vector<std::string>& cliArguments) {
    const std::size_t expectedArguments = 3;

    if (cliArguments.size() != expectedArguments) {
        throw std::runtime_error("Invalid number of command line arguments\n\n" + GetUsage());
    }
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
    assert(ReadTarget("enwik") == (std::vector<unsigned char>{}));
    assert(ReadTarget("enwik1") == (std::vector<unsigned char>{'<', 'm', 'e', 'd', 'i', 'a', 'w', 'i', 'k', 'i'}));
    assert(ReadTarget("enwik2").size() == 100);
    assert(ReadTarget("enwik3").size() == 1000);

    Operation operation;

    operation.command = command;
    operation.inputBytes = ReadTarget(command.target);

    ProcessTarget(operation);

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
