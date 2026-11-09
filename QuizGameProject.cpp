#include <iostream>
#include <string>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <limits>
using namespace std;

// ============================================================
//  Enums (enum class => no name conflicts, e.g. two "Mix" values)
// ============================================================
enum class enQuestionsLevel { Easy = 1, Med = 2, Hard = 3, Mix = 4 };
enum class enOperationType { Add = 1, Sub = 2, Mul = 3, Div = 4, Mix = 5 };

// ============================================================
//  Structures
// ============================================================

// One question with everything related to it
struct stQuestion
{
    int Number1 = 0;
    int Number2 = 0;
    enOperationType OperationType = enOperationType::Add;
    enQuestionsLevel QuestionLevel = enQuestionsLevel::Easy;
    int CorrectAnswer = 0;
    int PlayerAnswer = 0;
    bool AnswerResult = false;
};
// The whole quiz: settings + all questions + final result.
// vector grows to any size, so there is no fixed limit like [100].
struct stQuizz
{
    vector<stQuestion> QuestionList;
    int NumberOfQuestions = 0;
    enQuestionsLevel QuestionsLevel = enQuestionsLevel::Easy;
    enOperationType OpType = enOperationType::Add;
    int NumberOfRightAnswers = 0;
    int NumberOfWrongAnswers = 0;
    bool isPass = false;
};

// ============================================================
//  General helpers
// ============================================================

int RandomNumber(int From, int To)
{
    return rand() % (To - From + 1) + From;
}
// Reads an integer safely: if the user types letters, it does not hang.
int ReadInt(const string& Message)
{
    int Number = 0;
    while (true)
    {
        cout << Message;
        if (cin >> Number)
            return Number;

        cin.clear();                                              // reset the error state
        cin.ignore(numeric_limits<streamsize>::max(), '\n');      // throw away the bad input
        cout << "Invalid input, please enter a number.\n";
    }
}
void SetScreenColor(bool Right)
{
#ifdef _WIN32
    if (Right)
        system("color 2F");   // green
    else
        system("color 4F");   // red
#endif
    if (!Right)
        cout << "\a";         // alert sound
}
void ResetScreen()
{
#ifdef _WIN32
    system("cls");
    system("color 0F");
#else
    system("clear");
#endif
}
// ============================================================
//  Text helpers
// ============================================================
string GetOpTypeSymbol(enOperationType OpType)
{
    switch (OpType)
    {
    case enOperationType::Add: return "+";
    case enOperationType::Sub: return "-";
    case enOperationType::Mul: return "x";
    case enOperationType::Div: return "/";
    default:                   return "Mix";
    }
}

string GetQuestionLevelText(enQuestionsLevel Level)
{
    string Text[4] = { "Easy", "Medium", "Hard", "Mixed" };
    return Text[(int)Level - 1];
}

string GetOpTypeText(enOperationType OpType)
{
    string Text[5] = { "Add", "Sub", "Mul", "Div", "Mixed" };
    return Text[(int)OpType - 1];
}

// ============================================================
//  Reading the quiz settings from the player
// ============================================================

int ReadHowManyQuestions()
{
    int Number = 0;
    do
    {
        Number = ReadInt("How many questions do you want to answer ? ");
    } while (Number <= 0);
    return Number;
}

enQuestionsLevel ReadQuestionsLevel()
{
    int Choice = 0;
    do
    {
        Choice = ReadInt("\nEnter Questions Level [1] Easy, [2] Med, [3] Hard, [4] Mix ? ");
    } while (Choice < 1 || Choice > 4);
    return (enQuestionsLevel)Choice;
}

enOperationType ReadOpType()
{
    int Choice = 0;
    do
    {
        Choice = ReadInt("\nEnter Operation Type [1] Add, [2] Sub, [3] Mul, [4] Div, [5] Mix ? ");
    } while (Choice < 1 || Choice > 5);
    return (enOperationType)Choice;
}

// ============================================================
//  Question generation
// ============================================================

int SimpleCalculator(int Number1, int Number2, enOperationType OpType)
{
    switch (OpType)
    {
    case enOperationType::Add: return Number1 + Number2;
    case enOperationType::Sub: return Number1 - Number2;
    case enOperationType::Mul: return Number1 * Number2;
    case enOperationType::Div: return (Number2 != 0) ? (Number1 / Number2) : 0;
    default:                   return Number1 + Number2;
    }
}

enOperationType GetRandomOperationType()
{
    return (enOperationType)RandomNumber(1, 4);
}

stQuestion GenerateQuestion(enQuestionsLevel Level, enOperationType OpType)
{
    stQuestion Question;

    // Decide the REAL level and operation first, before building the question,
    // so the symbol we print always matches the operation we calculate.
    if (Level == enQuestionsLevel::Mix)
        Level = (enQuestionsLevel)RandomNumber(1, 3);

    if (OpType == enOperationType::Mix)
        OpType = GetRandomOperationType();

    Question.QuestionLevel = Level;
    Question.OperationType = OpType;

    switch (Level)
    {
    case enQuestionsLevel::Easy:
        Question.Number1 = RandomNumber(1, 10);
        Question.Number2 = RandomNumber(1, 10);
        break;
    case enQuestionsLevel::Med:
        Question.Number1 = RandomNumber(11, 50);
        Question.Number2 = RandomNumber(11, 50);
        break;
    case enQuestionsLevel::Hard:
        Question.Number1 = RandomNumber(51, 100);
        Question.Number2 = RandomNumber(51, 100);
        break;
    default:
        break;
    }

    // Division fix: build Number1 as a multiple of Number2,
    // so the result is always a whole number (e.g. 12 / 4 = 3, never 7 / 2).
    if (OpType == enOperationType::Div)
        Question.Number1 = Question.Number2 * RandomNumber(1, 10);

    Question.CorrectAnswer = SimpleCalculator(Question.Number1, Question.Number2, Question.OperationType);
    return Question;
}

void GenerateQuizzQuestions(stQuizz& Quizz)
{
    Quizz.QuestionList.clear();
    for (int i = 0; i < Quizz.NumberOfQuestions; i++)
    {
        Quizz.QuestionList.push_back(GenerateQuestion(Quizz.QuestionsLevel, Quizz.OpType));
    }
}

// ============================================================
//  Asking and correcting
// ============================================================

void PrintTheQuestion(const stQuestion& Q, int Number, int Total)
{
    cout << "\nQuestion [" << Number << "/" << Total << "]\n\n";
    cout << Q.Number1 << "\n";
    cout << Q.Number2 << " " << GetOpTypeSymbol(Q.OperationType) << "\n";
    cout << "_________\n";
}

void CorrectTheQuestionAnswer(stQuizz& Quizz, int Index)
{
    stQuestion& Q = Quizz.QuestionList[Index];   // reference: changes affect the real question

    Q.AnswerResult = (Q.PlayerAnswer == Q.CorrectAnswer);

    if (Q.AnswerResult)
        Quizz.NumberOfRightAnswers++;
    else
        Quizz.NumberOfWrongAnswers++;

    SetScreenColor(Q.AnswerResult);

    if (Q.AnswerResult)
    {
        cout << "Right Answer :-)\n";
    }
    else
    {
        cout << "Wrong Answer :-(\n";
        cout << "The right answer is: " << Q.CorrectAnswer << "\n";
    }
}

void AskAndCorrectQuestionListAnswers(stQuizz& Quizz)
{
    for (int i = 0; i < Quizz.NumberOfQuestions; i++)
    {
        PrintTheQuestion(Quizz.QuestionList[i], i + 1, Quizz.NumberOfQuestions);
        Quizz.QuestionList[i].PlayerAnswer = ReadInt("");   // negative numbers and 0 are allowed
        CorrectTheQuestionAnswer(Quizz, i);
    }

    Quizz.isPass = (Quizz.NumberOfRightAnswers >= Quizz.NumberOfWrongAnswers);
}

// ============================================================
//  Final results
// ============================================================

void PrintQuizzSummary(const stQuizz& Quizz)
{
    cout << "\nYour answers:\n";
    for (int i = 0; i < Quizz.NumberOfQuestions; i++)
    {
        const stQuestion& Q = Quizz.QuestionList[i];
        cout << "[" << i + 1 << "] "
            << Q.Number1 << " " << GetOpTypeSymbol(Q.OperationType) << " " << Q.Number2
            << " = " << Q.PlayerAnswer
            << (Q.AnswerResult ? "  (Right)" : "  (Wrong, correct: " + to_string(Q.CorrectAnswer) + ")")
            << "\n";
    }
}

void PrintQuizzResults(const stQuizz& Quizz)
{
    cout << "\n-----------------------------------------\n";
    cout << "  Final Result is " << (Quizz.isPass ? "PASS :-)" : "FAIL :-(") << "\n";
    cout << "-----------------------------------------\n";
    cout << "Number of Questions : " << Quizz.NumberOfQuestions << "\n";
    cout << "Questions Level     : " << GetQuestionLevelText(Quizz.QuestionsLevel) << "\n";
    cout << "Operation Type      : " << GetOpTypeText(Quizz.OpType) << "\n";
    cout << "Right Answers       : " << Quizz.NumberOfRightAnswers << "\n";
    cout << "Wrong Answers       : " << Quizz.NumberOfWrongAnswers << "\n";
    cout << "-----------------------------------------\n";

    SetScreenColor(Quizz.isPass);
}

//  Game flow
void PlayMathGame()
{
    stQuizz Quizz;

    Quizz.NumberOfQuestions = ReadHowManyQuestions();
    Quizz.QuestionsLevel = ReadQuestionsLevel();
    Quizz.OpType = ReadOpType();

    GenerateQuizzQuestions(Quizz);
    AskAndCorrectQuestionListAnswers(Quizz);

    PrintQuizzSummary(Quizz);
    PrintQuizzResults(Quizz);
}

int main()
{
    srand((unsigned)time(NULL));

    char PlayAgain = 'Y';
    do
    {
        ResetScreen();
        PlayMathGame();

        cout << "\nDo you want to play again? Y/N ? ";
        cin >> PlayAgain;
    } while (PlayAgain == 'Y' || PlayAgain == 'y');

    return 0;
}