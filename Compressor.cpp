#include <cassert>
#include <cmath>
#include <cstddef>
#include <fstream>
#include <iostream>
#include <iterator>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <vector>

using HistoryEntry = std::unordered_map<std::string, std::size_t>;

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

struct RelativePosition
{
    std::size_t inputPosition = 0;
    std::size_t level = 1;

    RelativePosition() = default;

    RelativePosition(std::size_t inputPosition, std::size_t level)
    {
        this->inputPosition = inputPosition;
        this->level = level;
    }
};

struct CombinationData
{
    std::size_t level = 1;
    std::size_t value = 0;

    CombinationData() = default;

    CombinationData(std::size_t level, std::size_t value)
    {
        this->level = level;
        this->value = value;
    }
};

bool CheckDoubles(double first, double second)
{
    return std::fabs(first - second) < 0.01;
}

struct VoteWeight
{
    double confidence = 0.0;
    double performance = 0.0;

    bool operator==(const VoteWeight& otherWeight) const
    {
        return CheckDoubles(confidence, otherWeight.confidence) && CheckDoubles(performance, otherWeight.performance);
    }
};

struct Vote
{
    unsigned char bit = 0;
    VoteWeight voteWeight;

    Vote() = default;

    Vote(unsigned char bit, double confidence, double performance)
    {
        this->bit = bit;
        this->voteWeight.confidence = confidence;
        this->voteWeight.performance = performance;
    }

    bool operator==(const Vote& otherVote) const
    {
        return bit == otherVote.bit && voteWeight == otherVote.voteWeight;
    }
};

struct Guess
{
    unsigned char bit = 1;
    double confidence = 0.5;
};

struct Performance
{
    std::size_t correct = 0;
    std::size_t incorrect = 1;

    Performance() = default;

    Performance(std::size_t correct, std::size_t incorrect)
    {
        this->correct = correct;
        this->incorrect = incorrect;
    }
};

struct History
{
    std::unordered_map<std::string, HistoryEntry> historicData;
    std::unordered_map<std::size_t, Performance> performance;
};

struct PredictionModel
{
    Model model = Model::Statistics;
    std::size_t levels = 1;
    History history;

    PredictionModel() = default;
    PredictionModel(Model model, std::size_t levels = 1)
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

    bool operator==(const Command& otherCommand) const
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

struct Position
{
    std::size_t inputPosition = 0;
    std::size_t virtualPosition = 0;
};

struct OperationStatus
{
    Operation operation;
    Position position;
};

struct Predictor
{
    OperationStatus operationStatus;
    std::unordered_map<std::string, PredictionModel> predictionModels;
};

std::string GenerateKey(const CombinationData& combinationData)
{
    std::string combinationKey = "";

    for (std::size_t position = 1; position <= combinationData.level; position++)
    {
        if (combinationData.value & (1 << combinationData.level - position))
        {
            combinationKey += "1";
        }
        else
        {
            combinationKey += "0";
        }
    }

    return combinationKey;
}

std::size_t BitPosition(const RelativePosition& relativePosition)
{
    std::size_t bitPosition = relativePosition.level;
    while ((relativePosition.inputPosition - (relativePosition.level - bitPosition)) % bitPosition != 0)
    {
        bitPosition--;
    }

    return relativePosition.level - bitPosition;
}

unsigned char GetBitFromInput(const std::vector<unsigned char> inputBytes, std::size_t bitPosition)
{
    return (inputBytes.at(bitPosition / 8) & (128 >> (bitPosition % 8))) > 0 ? 1 : 0;
}

bool StillPossible(
    const Predictor& predictor,
    const std::vector<unsigned char> guessedBits,
    std::string combinationKey,
    std::size_t bitPosition)
{
    assert(GetBitFromInput({0}, 0) == 0);
    assert(GetBitFromInput({1}, 0) == 0);
    assert(GetBitFromInput({128}, 0) == 1);
    assert(GetBitFromInput({128, 0}, 0) == 1);
    assert(GetBitFromInput({128, 0}, 1) == 0);
    assert(GetBitFromInput({128, 0}, 9) == 0);
    assert(GetBitFromInput({255, 0}, 7) == 1);
    assert(GetBitFromInput({128, 255}, 9) == 1);
    assert(GetBitFromInput({128, 64, 2, 0}, 23) == 0);
    assert(GetBitFromInput({128, 64, 2, 0}, 22) == 1);

    const std::vector<unsigned char>& inputBytes = predictor.operationStatus.operation.inputBytes;
    const std::size_t inputPosition = predictor.operationStatus.position.inputPosition;

    for (std::size_t i = bitPosition; i > 0; i--)
    {
        if (guessedBits.size() >= ((bitPosition - i) + 1))
        {
            if (guessedBits.at(guessedBits.size() - ((bitPosition - i) + 1)) !=
                (combinationKey.at(bitPosition - ((bitPosition - i) + 1)) == '0' ? 0 : 1))
            {
                return false;
            }
        }
        else
        {
            if (GetBitFromInput(inputBytes, inputPosition - (((bitPosition - guessedBits.size()) - i) + 1)) !=
                (combinationKey.at(bitPosition - ((bitPosition - i) + 1)) == '0' ? 0 : 1))
            {
                return false;
            }
        }
    }

    return true;
}

std::string GenerateHistoricKey(const Predictor& predictor, std::vector<unsigned char> guessedBits, std::size_t level)
{
    const std::vector<unsigned char>& inputBytes = predictor.operationStatus.operation.inputBytes;
    const std::size_t inputPosition = predictor.operationStatus.position.inputPosition;

    std::string key;

    for (std::size_t i = level; i > 0; i--)
    {
        if (guessedBits.size() >= ((level - i) + 1))
        {
            key.insert(0, (guessedBits.at(guessedBits.size() - ((level - i) + 1)) == 0 ? "0" : "1"));
        }
        else
        {
            key.insert(0, (GetBitFromInput(inputBytes, inputPosition - (((level - guessedBits.size()) - i) + 1)) == 0 ? "0" : "1"));
        }
    }

    return key;
}

std::vector<Vote> StatisticsVotes(const Predictor& predictor, const std::vector<unsigned char> guessedBits)
{
    assert(GenerateKey(CombinationData(1, 0)) == "0");
    assert(GenerateKey(CombinationData(1, 1)) == "1");
    assert(GenerateKey(CombinationData(2, 0)) == "00");
    assert(GenerateKey(CombinationData(2, 2)) == "10");
    assert(GenerateKey(CombinationData(2, 3)) == "11");
    assert(GenerateKey(CombinationData(4, 5)) == "0101");
    assert(GenerateKey(CombinationData(8, 42)) == "00101010");
    assert(GenerateKey(CombinationData(16, 127)) == "0000000001111111");

    assert(BitPosition(RelativePosition(0, 1)) == 0);
    assert(BitPosition(RelativePosition(1, 1)) == 0);
    assert(BitPosition(RelativePosition(1, 2)) == 1);
    assert(BitPosition(RelativePosition(2, 1)) == 0);
    assert(BitPosition(RelativePosition(2, 2)) == 0);
    assert(BitPosition(RelativePosition(3, 1)) == 0);
    assert(BitPosition(RelativePosition(3, 2)) == 1);
    assert(BitPosition(RelativePosition(4, 4)) == 0);
    assert(BitPosition(RelativePosition(30, 4)) == 2);

    Predictor testPredictor;
    assert(StillPossible(testPredictor, {}, "0", 0) == true);
    assert(StillPossible(testPredictor, {1}, "01", 0) == true);
    assert(StillPossible(testPredictor, {1}, "01", 1) == false);
    assert(StillPossible(testPredictor, {1}, "11", 1) == true);

    testPredictor.operationStatus.operation.inputBytes = {128};
    testPredictor.operationStatus.position.inputPosition = 1;
    assert(StillPossible(testPredictor, {}, "11", 1) == true);

    testPredictor.operationStatus.operation.inputBytes = {128, 64, 0, 255};
    testPredictor.operationStatus.position.inputPosition = 31;
    assert(StillPossible(testPredictor, {1, 1, 1}, "111111", 5) == true);

    testPredictor.operationStatus.operation.inputBytes = {128, 64, 0, 0};
    testPredictor.operationStatus.position.inputPosition = 31;
    assert(StillPossible(testPredictor, {1, 1, 1}, "111111", 5) == false);

    testPredictor.operationStatus.operation.inputBytes = {128, 64, 1, 0};
    testPredictor.operationStatus.position.inputPosition = 25;
    assert(StillPossible(testPredictor, {0, 0, 0}, "00000", 5) == false);

    testPredictor.operationStatus.operation.inputBytes = {128, 64, 2, 0};
    testPredictor.operationStatus.position.inputPosition = 25;
    assert(StillPossible(testPredictor, {0, 0, 0}, "00000", 5) == true);

    std::vector<Vote> votes;
    const PredictionModel& statisticsModel = predictor.predictionModels.at("Statistics");
    const std::unordered_map<std::string, HistoryEntry> historicData = statisticsModel.history.historicData;
    std::size_t divisor = 1;

    for (std::size_t level = 1; level <= statisticsModel.levels; level++)
    {
        if (level % divisor != 0)
        {
            continue;
        }
        else
        {
            if (divisor < 8)
            {
                divisor *= 2;
            }
        }

        std::size_t votesZero = 0;
        std::size_t votesOne = 0;

        for (std::size_t combination = 0; combination < 1 << level; combination++)
        {
            CombinationData combinationData;
            combinationData.level = level;
            combinationData.value = combination;

            std::string combinationKey = GenerateKey(combinationData);

            if (statisticsModel.history.historicData.contains(combinationKey))
            {
                const HistoryEntry& combinationHistory = historicData.at(combinationKey);

                RelativePosition relativePosition;
                relativePosition.inputPosition = predictor.operationStatus.position.virtualPosition;
                relativePosition.level = level;

                std::size_t bitPosition = BitPosition(relativePosition);

                if (StillPossible(predictor, guessedBits, combinationKey, bitPosition))
                {
                    if (combinationKey.at(bitPosition) == '0')
                    {
                        votesZero += combinationHistory.at("");
                    }
                    else
                    {
                        votesOne += combinationHistory.at("");
                    }
                }
            }
        }

        Performance performance;

        if (statisticsModel.history.performance.contains(level))
        {
            performance = statisticsModel.history.performance.at(level);
        }

        double votesZeroDouble = static_cast<double>(votesZero);
        double votesOneDouble = static_cast<double>(votesOne);
        double totalVotes = votesZeroDouble + votesOneDouble;

        double correctDouble = static_cast<double>(performance.correct);
        double incorrectDouble = static_cast<double>(performance.incorrect);
        double totalGuesses = correctDouble + incorrectDouble;

        totalVotes = totalVotes == 0 ? 1 : totalVotes;

        VoteWeight voteWeight;
        voteWeight.confidence = votesZero >= votesOne ? votesZeroDouble / totalVotes : votesOneDouble / totalVotes;
        voteWeight.performance = correctDouble / (correctDouble + incorrectDouble);

        Vote vote;
        vote.bit = votesZero >= votesOne ? 0 : 1;
        vote.voteWeight = voteWeight;

        votes.push_back(vote);
    }

    return votes;
}

std::vector<Vote> HistoricVotes(const Predictor& predictor, const std::vector<unsigned char> guessedBits)
{
    Predictor testPredictor;
    testPredictor.operationStatus.position.inputPosition = 1;
    testPredictor.operationStatus.position.virtualPosition = 1;
    testPredictor.operationStatus.operation.inputBytes = {0};
    assert(GenerateHistoricKey(testPredictor, {}, 1) == "0");

    testPredictor.operationStatus.operation.inputBytes = {128};
    assert(GenerateHistoricKey(testPredictor, {}, 1) == "1");

    testPredictor.operationStatus.operation.inputBytes = {64};
    assert(GenerateHistoricKey(testPredictor, {}, 1) == "0");

    testPredictor.operationStatus.operation.inputBytes = {64};
    testPredictor.operationStatus.position.virtualPosition = 2;
    assert(GenerateHistoricKey(testPredictor, {1}, 1) == "1");

    assert(GenerateHistoricKey(testPredictor, {1}, 2) == "01");

    testPredictor.operationStatus.operation.inputBytes = {128};
    testPredictor.operationStatus.position.inputPosition = 2;
    testPredictor.operationStatus.position.virtualPosition = 4;
    assert(GenerateHistoricKey(testPredictor, {1, 0}, 4) == "1010");

    const std::size_t inputPosition = predictor.operationStatus.position.inputPosition;
    const std::size_t virtualPosition = predictor.operationStatus.position.virtualPosition;

    const PredictionModel& historicModel = predictor.predictionModels.at("HistoricDictionary");
    const std::unordered_map<std::string, HistoryEntry> historicData = historicModel.history.historicData;

    std::vector<Vote> votes;
    std::size_t divisor = 1;

    for (std::size_t level = 1; level <= historicModel.levels; level++)
    {
        if (virtualPosition < level)
        {
            break;
        }

        if (level % divisor != 0)
        {
            continue;
        }
        else
        {
            if (divisor < 8)
            {
                divisor *= 2;
            }
        }

        std::size_t votesZero = 0;
        std::size_t votesOne = 0;

        std::string key = GenerateHistoricKey(predictor, guessedBits, level);

        if (historicData.contains(key))
        {
            HistoryEntry historyEntry = historicData.at(key);

            for (auto& [combination, occurences]: historyEntry)
            {
                RelativePosition relativePosition;
                relativePosition.inputPosition = predictor.operationStatus.position.virtualPosition;
                relativePosition.level = level;

                std::size_t bitPosition = BitPosition(relativePosition);

                if (StillPossible(predictor, guessedBits, combination, bitPosition))
                {
                    if (combination.at(bitPosition) == '0')
                    {
                        votesZero += occurences;
                    }
                    else
                    {
                        votesOne += occurences;
                    }
                }
            }
        }

        Performance performance;

        if (historicModel.history.performance.contains(level))
        {
            performance = historicModel.history.performance.at(level);
        }

        double votesZeroDouble = static_cast<double>(votesZero);
        double votesOneDouble = static_cast<double>(votesOne);
        double totalVotes = votesZeroDouble + votesOneDouble;

        double correctDouble = static_cast<double>(performance.correct);
        double incorrectDouble = static_cast<double>(performance.incorrect);
        double totalGuesses = correctDouble + incorrectDouble;

        totalVotes = totalVotes == 0 ? 1 : totalVotes;

        VoteWeight voteWeight;
        voteWeight.confidence = votesZero >= votesOne ? votesZeroDouble / totalVotes : votesOneDouble / totalVotes;
        voteWeight.performance = correctDouble / (correctDouble + incorrectDouble);

        Vote vote;
        vote.bit = votesZero >= votesOne ? 0 : 1;
        vote.voteWeight = voteWeight;

        votes.push_back(vote);
    }

    return votes;
}

Guess GuessBit(const Predictor& predictor, std::vector<unsigned char>& guessedBits)
{
    Predictor testPredictor;
    testPredictor.predictionModels["Statistics"] = PredictionModel(Model::Statistics);
    testPredictor.predictionModels["HistoricDictionary"] = PredictionModel(Model::HistoricDictionary);
    PredictionModel& statisticsModel = testPredictor.predictionModels["Statistics"];
    PredictionModel& historicModel = testPredictor.predictionModels["HistoricDictionary"];

    std::unordered_map<std::string, HistoryEntry>& statisticsHistory = statisticsModel.history.historicData;
    std::unordered_map<std::size_t, Performance>& statisticsPerformance = statisticsModel.history.performance;
    std::unordered_map<std::string, HistoryEntry>& historicHistory = historicModel.history.historicData;
    std::unordered_map<std::size_t, Performance>& historicPerformance = historicModel.history.performance;

    statisticsHistory = std::unordered_map<std::string, HistoryEntry>();
    statisticsPerformance = std::unordered_map<std::size_t, Performance>();

    std::vector<unsigned char> testGuessedBits;

    assert(StatisticsVotes(testPredictor, testGuessedBits) == (std::vector<Vote>{Vote()}));
    assert(HistoricVotes(testPredictor, testGuessedBits) == (std::vector<Vote>{}));

    statisticsHistory["0"] = HistoryEntry{{"", 1}};
    assert(StatisticsVotes(testPredictor, testGuessedBits) == (std::vector<Vote>{Vote(0, 1.0, 0.0)}));

    statisticsHistory["1"] = HistoryEntry{{"", 2}};
    assert(StatisticsVotes(testPredictor, testGuessedBits) == (std::vector<Vote>{Vote(1, 0.67, 0.0)}));

    statisticsPerformance[1] = Performance(1, 1);
    assert(StatisticsVotes(testPredictor, testGuessedBits) == (std::vector<Vote>{Vote(1, 0.67, 0.5)}));

    statisticsPerformance[1] = Performance(5, 1);
    assert(StatisticsVotes(testPredictor, testGuessedBits) == (std::vector<Vote>{Vote(1, 0.67, 0.83)}));

    statisticsModel.levels = 2;
    assert(StatisticsVotes(testPredictor, testGuessedBits) == (std::vector<Vote>{Vote(1, 0.67, 0.83), Vote(0, 0.0, 0.0)}));
    assert(HistoricVotes(testPredictor, testGuessedBits) == (std::vector<Vote>{}));

    statisticsHistory["10"] = HistoryEntry{{"", 5}};
    assert(StatisticsVotes(testPredictor, testGuessedBits) == (std::vector<Vote>{Vote(1, 0.67, 0.83), Vote(1, 1.0, 0.0)}));

    testPredictor.operationStatus.position.virtualPosition = 1;
    testGuessedBits = { 1 };
    assert(StatisticsVotes(testPredictor, testGuessedBits) == (std::vector<Vote>{Vote(1, 0.67, 0.83), Vote(0, 1.0, 0.0)}));
    assert(HistoricVotes(testPredictor, testGuessedBits) == (std::vector<Vote>{Vote(0, 0.0, 0.0)}));

    testGuessedBits = { 0 };
    assert(StatisticsVotes(testPredictor, testGuessedBits) == (std::vector<Vote>{Vote(1, 0.67, 0.83), Vote(0, 0.0, 0.0)}));

    testPredictor.operationStatus.position.inputPosition = 1;
    testPredictor.operationStatus.operation.inputBytes = { 128 };
    testGuessedBits = {};
    historicHistory["0"] = HistoryEntry{{"1", 1}};
    assert(StatisticsVotes(testPredictor, testGuessedBits) == (std::vector<Vote>{Vote(1, 0.67, 0.83), Vote(0, 1.0, 0.0)}));
    assert(HistoricVotes(testPredictor, testGuessedBits) == (std::vector<Vote>{Vote(0, 0.0, 0.0)}));

    historicHistory["1"] = HistoryEntry{{"1", 1}};
    assert(HistoricVotes(testPredictor, testGuessedBits) == (std::vector<Vote>{Vote(1, 1.0, 0.0)}));

    historicHistory["1"] = HistoryEntry{{"0", 1 }, {"1", 4}};
    assert(HistoricVotes(testPredictor, testGuessedBits) == (std::vector<Vote>{Vote(1, 0.8, 0.0)}));

    testPredictor.operationStatus.operation.inputBytes = { 127 };
    assert(StatisticsVotes(testPredictor, testGuessedBits) == (std::vector<Vote>{Vote(1, 0.67, 0.83), Vote(0, 0.0, 0.0)}));

    statisticsModel.levels = 3;
    assert(StatisticsVotes(testPredictor, testGuessedBits) == (std::vector<Vote>{Vote(1, 0.67, 0.83), Vote(0, 0.0, 0.0)}));

    testPredictor.operationStatus.position.inputPosition = 1;
    testPredictor.operationStatus.position.virtualPosition = 2;
    testPredictor.operationStatus.operation.inputBytes = { 128 };
    testGuessedBits = { 0 };
    historicModel.levels = 2;
    historicHistory["10"] = HistoryEntry{{"0", 1 }, {"1", 4}};
    assert(HistoricVotes(testPredictor, testGuessedBits) == (std::vector<Vote>{Vote(1, 1.0, 0.0), Vote(1, 0.8, 0.0)}));

    std::vector<Vote> votes;
    std::vector<Vote> statisticsVotes;
    std::vector<Vote> historicVotes;

    for (auto& [modelKey, predictionModel]: predictor.predictionModels)
    {
        switch (predictionModel.model)
        {
            case Model::Statistics:
                statisticsVotes = StatisticsVotes(predictor, guessedBits);
                break;
            case Model::HistoricDictionary:
                historicVotes = HistoricVotes(predictor, guessedBits);
                break;
            case Model::FutureDictionary:
                break;
            case Model::Distance:
                break;
            default:
                throw std::runtime_error("Unknown prediction model");
        }
    }

    return Guess();
}

std::vector<unsigned char> GuessBits(const Predictor& predictor)
{
    double confidence = 1.0;
    std::vector<unsigned char> guessedBits;

    while (confidence > 0.5)
    {
        Guess guess = GuessBit(predictor, guessedBits);

        guessedBits.push_back(guess.bit);
        confidence *= guess.confidence;
    }

    return std::vector<unsigned char>{1};
}

bool CheckGuess()
{
    return true;
}

GuessResult MakeGuess(const Predictor& predictor)
{
    GuessResult guessResult;

    guessResult.guessedBits = GuessBits(predictor);
    guessResult.correct = CheckGuess();

    return guessResult;
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
    std::vector<unsigned char> outputBytes;

    OperationStatus operationStatus;
    operationStatus.operation = operation;

    std::unordered_map<std::string, PredictionModel> predictionModels;
    predictionModels["Statistics"] = PredictionModel(Model::Statistics);
    predictionModels["HistoricDictionary"] = PredictionModel(Model::HistoricDictionary);
    predictionModels["FutureDictionary"] = PredictionModel(Model::FutureDictionary);
    predictionModels["Distance"] = PredictionModel(Model::Distance);

    Predictor predictor;
    predictor.operationStatus = operationStatus;
    predictor.predictionModels = predictionModels;

    while (correctBits != operation.inputBytes.size() * numberBits)
    {
        GuessResult guessResult = MakeGuess(predictor);
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
