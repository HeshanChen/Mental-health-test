#include <SFML/Graphics.hpp>
#include <iostream>
#include <fstream>
#include <vector>

sf::Font font;
sf::RenderWindow window(sf::VideoMode(800, 400), "SFML works!");
sf::Text text;
sf::Text timeText;
sf::RectangleShape timeBorder;

struct Question
{
    int qNum;
    std::string qustions[4];
};
std::vector<Question> qs;

/* Init the Menu */
void initMenu(std::string str)
{
    text.setString(str);
    text.setFont(font);
    text.setCharacterSize(30);
    text.setFillColor(sf::Color::Cyan);

    // center text
    sf::FloatRect textRect = text.getLocalBounds();
    text.setOrigin(textRect.left + textRect.width / 2.0f,
                   textRect.top + textRect.height / 2.0f);
    sf::Vector2f screenCenter = window.getView().getCenter();
    text.setPosition(screenCenter.x, 150);
}

/* Init a text without border*/
void initText(sf::Text &text, const std::string &str, int x, int y, int size = 24, sf::Color textColor = sf::Color::White)
{
    text.setString(str);
    text.setFont(font);
    text.setFillColor(textColor);
    text.setCharacterSize(size);
    text.setPosition(x, y);
}

/* Init a text */
void initTextWithBorder(sf::Text &text, sf::RectangleShape &border, const std::string &str, int x, int y, int size = 30, sf::Color textColor = sf::Color::White,
                        sf::Color borderColor = sf::Color::White)
{
    text.setString(str);
    text.setFont(font);
    text.setFillColor(textColor);
    text.setCharacterSize(size);
    text.setPosition(x, y);

    // set border of the button
    // get the offset of the text position
    sf::FloatRect rect = text.getLocalBounds();
    // define the margin between the border and the text
    float margin = 5;
    border = sf::RectangleShape(sf::Vector2f(rect.width + margin * 2, rect.height + margin * 2));
    border.setFillColor(sf::Color::Black);
    border.setOutlineColor(borderColor);
    border.setOutlineThickness(2);

    // the border's top left is the text's position plus the text's offset then minors the margin
    border.setPosition(rect.left + text.getPosition().x - margin, rect.top + text.getPosition().y - margin);
}

void showTime()
{
    // get the time
    time_t now;
    time(&now);
    char timestr[128];
    strftime(timestr, 128, "%H:%M:%S", localtime(&now));

    // display the time text
    timeText.setCharacterSize(30);
    initTextWithBorder(timeText, timeBorder, timestr, 50, 300, 30, sf::Color::Yellow, sf::Color::White);
    window.draw(timeBorder);
    window.draw(timeText);
}

// Counting down clock
class ClockCountingDown
{
private:
    sf::Text timeText;
    sf::RectangleShape timeBorder;
    sf::Clock clock;

public:
    int showCountingDown(int totalTime)
    {
        // the remain time is the total time minors the elapsed time since the clock created or restart
        int remainTime = totalTime - clock.getElapsedTime().asSeconds() + 1;

        // remain time should not below 0
        if (remainTime < 0)
            remainTime = 0;

        // init the time text and border(before showing it)
        initTextWithBorder(timeText, timeBorder, "Remain time: " + std::to_string(remainTime) + "s", 50, 300, 30, sf::Color::Yellow, sf::Color::White);
        // show the time text and border
        window.draw(timeBorder);
        window.draw(timeText);
        return remainTime;
    }

    sf::Time restart()
    {
        return clock.restart();
    }
};

// load questions from a file in the format of questions.txt
void loadQuestionsFromFile(std::string filename)
{
    std::ifstream fin(filename);
    if (!fin.is_open())
    {
        std::cerr << "file open failed!" << std::endl;
        return;
    }

    while (!fin.eof())
    {
        Question q;
        // read question num
        fin >> q.qNum;
        std::string skip;
        std::getline(fin, skip);
        for (int i = 0; i < 4; i++)
        {
            // read choice num
            int cNum;
            fin >> cNum;
            // read question body
            std::getline(fin, q.qustions[i]);
        }
        qs.push_back(q);
    }

    fin.close();
    
}

class TextWithoutBorder
{
protected:
    sf::Text text;

public:
    TextWithoutBorder() {}
    TextWithoutBorder(std::string str, int x, int y, int size, sf::Color textColor = sf::Color::White)
    {
        init(str, x, y, size, textColor);
    }

    void init(std::string str, int x, int y, int size, sf::Color textColor = sf::Color::White)
    {
        initText(text, str, x, y, size, textColor);
    }

    void show()
    {
        window.draw(text);
    }
    bool contains(sf::Vector2i pos)
    {
        return text.getGlobalBounds().contains(pos.x, pos.y);
    }
};

// Constructs a text with a border
class TextWithBorder: public TextWithoutBorder
{
protected:
    sf::RectangleShape textBorder;

public:
    TextWithBorder() {}
    TextWithBorder(std::string str, int x, int y, sf::Color textColor = sf::Color::White,
                   sf::Color borderColor = sf::Color::White)
    {
        init(str, x, y, textColor, borderColor);
    }

    void init(std::string str, int x, int y, sf::Color textColor = sf::Color::White,
              sf::Color borderColor = sf::Color::White)
    {
        initTextWithBorder(text, textBorder, str, x, y);
    }

    void show()
    {
        window.draw(textBorder);
        TextWithoutBorder::show();
    }
};

TextWithBorder buttonContinue;

// show the menu in the window
void showMenu()
{
    window.draw(text);
}

// init a question frame
class QuestionFrame
{
private:
    sf::Text textQustions[4];
    sf::Text title;
    int questionSelected;

public:
    void init(int qId)
    {
        // qId is the question index(start from 0)
        Question &q = qs[qId];

        // show title
        title.setString(std::to_string(q.qNum) + ". Please pick one of the four answers" + "   current score:");
        title.setPosition(10, 10);
        title.setFont(font);
        title.setCharacterSize(24);
        title.setFillColor(sf::Color::Yellow);


        for (int i = 0; i < 4; i++)
        {
            // show question body
            std::string qStr = q.qustions[i];

            // offset between each options
            int offset = 60;

            // if the qustion is too long, add a new line charactor in it
            if (qStr.size() >= 60)
            {
                for (int i = 59; i < qStr.size(); i++)
                {
                    if (qStr[i] == ' ')
                    {
                        qStr[i] = '\n';
                        break;
                    }
                }
            }

            // init the question text
            textQustions[i].setString(std::string(1, 'A' + i) + ". " + qStr);
            textQustions[i].setFont(font);
            textQustions[i].setCharacterSize(24);
            textQustions[i].setFillColor(sf::Color::Green);
            textQustions[i].setPosition(50, 50 + offset * i);
        }

        // now no question is selected
        questionSelected = -1;
    }

    // show the question frame
    void show()
    {
        window.draw(title);
        for (int i = 0; i < 4; i++)
            window.draw(textQustions[i]);
    }

    // check if the question text is clicked, if so, select it
    void clicked(sf::Vector2i pos)
    {
        for (int i = 0; i < 4; i++)
        {
            if (textQustions[i].getGlobalBounds().contains(pos.x, pos.y))
                selectQuestion(i);
        }
    }

    // select a qustion by its index and set its color
    void selectQuestion(int id)
    {
        for (int j = 0; j < 4; j++)
        {
            textQustions[j].setFillColor(sf::Color::Green);
        }
        textQustions[id].setFillColor(sf::Color::Cyan);
        questionSelected = id;
    }

    // get the selected question's id
    int getQuestionSelected()
    {
        return questionSelected;
    }
};

class TimeExceedFrame
{
private:
    TextWithBorder quit;
    TextWithBorder extendTime;

public:
    void init()
    {
        initMenu("Your time for answering the question\n\n\t\t\t\thas exceeded");
        quit.init(" Quit ", 100, 300);
        extendTime.init(" +10s ", 600, 300);
    }
    void show()
    {
        showMenu();
        quit.show();
        extendTime.show();
    }

    int clicked(sf::Vector2i pos)
    {
        if (quit.contains(pos))
        {
            return 0;
        }
        else if (extendTime.contains(pos))
        {
            return 1;
        }
        else
        {
            return -1;
        }
    }
};

#define between(x, a, b) (x >= a && x <= b)
std::string getSummaryString(int score)
{
    std::string summaries[] = {
        "You are very healthy \n Enjoy your life!!!!!!! \n Total score " +  std::to_string(score),
        "These ups and downs are considered normal \n Have a good day!!!!!!!! \n Total score " +  std::to_string(score),
        "Mild mood disturbance \nYou can do some exercise\nto adjust your mood! \n Total score " +  std::to_string(score),
        "Borderline clinical depression\nIf you have any problem\nplease register on the patient connect\nwe are ready to help you!! \n Total score " +  std::to_string(score),
        "Moderate depression \nIf you have any problem\nplease register on the patient connect\nwe are ready to help you!! \n Total score " +  std::to_string(score),
        "Severe depression \nIf you have any problem\nplease register on the patient connect\nor call 617-353-3569\nwe are ready to help you!! \n Total score " +  std::to_string(score),
        "Extreme depression  \nIf you have any problem\nplease register on the patient connect\nor call 617-353-3569\nwe are ready to help you!!\n Total score " +  std::to_string(score)};

    if (score == 0)
    {
        return summaries[0];
    }
    else if (between(score, 1, 10))
    {
        return summaries[1];
    }
    else if (between(score, 11, 16))
    {
        return summaries[2];
    }
    else if (between(score, 17, 20))
    {
        return summaries[3];
    }
    else if (between(score, 21, 30))
    {
        return summaries[4];
    }
    else if (between(score, 31, 40))
    {
        return summaries[5];
    }
    else if (score > 40)
    {
        return summaries[6];
    }
    else
    {
        return "Illegal score";
    }
}

int curPage = 0;
bool pageChanged = true;
QuestionFrame qFrame;
TimeExceedFrame tFrame;
ClockCountingDown clk;
enum STATUS
{
    NOTSTART,
    NORMAL,
    EXCEED,
    FAILED,
    EXTENDED
};
int status = NOTSTART;
int totalScore = 0;


int main()
{
    if (!font.loadFromFile("./arial.ttf"))
    {
        std::cerr << "Error loading font file!" << std::endl;
        return -1;
    }

    loadQuestionsFromFile("questions.txt");

    while (window.isOpen())
    {
        sf::Event event;

        while (window.pollEvent(event))
        {
            if (event.type == sf::Event::Closed)
            {
                window.close();
            }

            else if (event.type == sf::Event::KeyReleased)
            {
                if (between(event.key.code, 0, 4))
                {
                    qFrame.selectQuestion(event.key.code);
                }
                else if (event.key.code == sf::Keyboard::Enter || event.key.code == sf::Keyboard::Return)
                {
                    switch (status)
                    {
                    case NOTSTART:
                        curPage++;
                        pageChanged = true;
                        break;
                    case NORMAL:
                    case EXTENDED:
                    {
                        int questionSelected = qFrame.getQuestionSelected();

                        // haven't select a qustion, shouldn't proceed to the next page
                        if (questionSelected < 0)
                            break;

                        totalScore += questionSelected;
                        curPage++;
                        pageChanged = true;
                    }
                    break;
                    case EXCEED:
                        status = EXTENDED;
                        break;
                    default:
                        break;
                    }
                }
                else if (event.key.code == sf::Keyboard::Escape)
                {
                    if (status == EXCEED)
                    {
                        window.close();
                        break;
                    }
                }
            }

            else if (event.type == sf::Event::MouseButtonReleased)
            {
                if (event.mouseButton.button == sf::Mouse::Left)
                {
                    sf::Vector2i position = sf::Mouse::getPosition(window);

                    switch (status)
                    {
                    case NOTSTART:
                        if (buttonContinue.contains(position))
                        {
                            curPage++;
                            pageChanged = true;
                        }
                        break;
                    case NORMAL:
                    case EXTENDED:
                        qFrame.clicked(position);
                        if (buttonContinue.contains(position))
                        {
                            int questionSelected = qFrame.getQuestionSelected();

                            // haven't select a qustion, shouldn't proceed to the next page
                            if (questionSelected < 0)
                                break;

                            totalScore += questionSelected;
                            curPage++;
                            pageChanged = true;
                        }
                        break;
                    case EXCEED:
                        switch (tFrame.clicked(position))
                        {
                        // quit clicked
                        case 0:
                            window.close();
                            break;
                        // +10s clicked
                        case 1:
                            status = EXTENDED;
                            break;
                        default:
                            break;
                        }
                        break;
                    default:
                        break;
                    }
                }
            }
        }

        window.clear();
        switch (curPage)
        {
        case 0:
            if (pageChanged)
            {
                initMenu("Boston University Student Health Service \n\nWelcome to the Beck's Depression Inventory \n\n\n Please answer all the questions in the given time");
                buttonContinue.init("Continue", 600, 300);
                pageChanged = false;
            }
            showMenu();
            buttonContinue.show();
            break;
        case 1:
            if (pageChanged)
            {
                initMenu("Please answer every question\n\n\n\t\twithin 30 seconds");
                buttonContinue.init("Continue", 600, 300);
                pageChanged = false;
            }
            showMenu();
            buttonContinue.show();
            showTime();
            break;

        default:
            if (curPage >= 2 && curPage < 2 + qs.size())
            {
                if (status == EXCEED)
                {
                    tFrame.show();
                    break;
                }

                if (pageChanged)
                {
                    pageChanged = false;
                    status = NORMAL;
                    buttonContinue.init("  Next  ", 600, 300);
                    qFrame.init(curPage - 2);
                    clk.restart();
                }
                qFrame.show();

                int timeLimit;
                if (status == NORMAL)
                {
                    timeLimit = 30;
                }
                else if (status == EXTENDED)
                {
                    timeLimit = 10;
                }
                else
                {
                    // Error, cannot go there
                    std::cerr << "illegal status!" << std::endl;
                }

                if (clk.showCountingDown(timeLimit) == 0)
                {
                    // std::cout << "time exceeded!" << std::endl;
                    if (status == NORMAL)
                    {
                        status = EXCEED;
                    }
                    else if (status == EXTENDED)
                    {
                        status = FAILED;
                        window.close();
                        break;
                    }
                    else
                    {
                        // Error, cannot go there
                        std::cerr << "illegal status!" << std::endl;
                    }

                    tFrame.init();
                    clk.restart();
                }
                buttonContinue.show();
            }

            // summary page
            else if (curPage == 2 + qs.size())
            {
                if (pageChanged)
                {
                    pageChanged = false;
                    initMenu(getSummaryString(totalScore));
                }
                showMenu();
            }

            break;
        }
        window.display();
    }

    return 0;
}
