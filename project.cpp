#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>
#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <cmath>

using namespace std;

// ============== DATA MODELS ==================

struct SubjectInfo {
    string name;
    int total   = 0;
    int present = 0;
};

struct Student {
    string name;
    string dob;
    string address;
    string year;
    float  cgpa = 0.0f;
    vector<SubjectInfo> subjects;  // same order as global subjectNames
};

map<int, Student> DB;          // roll -> student
vector<string> subjectNames;   // defined once at start

// ============== FONT ========================

sf::Font& appFont() {
    static sf::Font font;
    static bool loaded = false;
    if (!loaded) {
        font.loadFromFile("/System/Library/Fonts/Supplemental/Arial.ttf");
        loaded = true;
    }
    return font;
}

// ============== UI HELPERS ===================

void drawCenteredText(sf::RenderWindow &win,
                      const string &text,
                      float y,
                      unsigned int size,
                      sf::Color col = sf::Color::Black)
{
    sf::Text t;
    t.setFont(appFont());
    t.setString(text);
    t.setCharacterSize(size);
    t.setFillColor(col);
    sf::FloatRect b = t.getLocalBounds();
    t.setOrigin(b.left + b.width / 2.f, b.top + b.height / 2.f);
    t.setPosition(win.getSize().x / 2.f, y);
    win.draw(t);
}

void drawLeftText(sf::RenderWindow &win,
                  const string &text,
                  float x,
                  float y,
                  unsigned int size,
                  sf::Color col = sf::Color::Black)
{
    sf::Text t;
    t.setFont(appFont());
    t.setString(text);
    t.setCharacterSize(size);
    t.setFillColor(col);
    t.setPosition(x, y);
    win.draw(t);
}

void drawCardCentered(sf::RenderWindow &win, float w, float h)
{
    float W = win.getSize().x;
    float H = win.getSize().y;
    float x = (W - w) / 2.f;
    float y = (H - h) / 2.f;

    sf::RectangleShape shadow(sf::Vector2f(w, h));
    shadow.setPosition(x + 6.f, y + 6.f);
    shadow.setFillColor(sf::Color(80, 40, 130, 80));
    win.draw(shadow);

    sf::RectangleShape card(sf::Vector2f(w, h));
    card.setPosition(x, y);
    card.setFillColor(sf::Color(250, 244, 255));
    card.setOutlineThickness(4.f);
    card.setOutlineColor(sf::Color(140, 80, 210));
    win.draw(card);
}

bool drawButtonCentered(sf::RenderWindow &win,
                        const string &label,
                        float centerY)
{
    float W = win.getSize().x;
    float bw = 360.f;
    float bh = 55.f;

    sf::RectangleShape box(sf::Vector2f(bw, bh));
    box.setOrigin(bw / 2.f, bh / 2.f);
    box.setPosition(W / 2.f, centerY);

    sf::Vector2i mp = sf::Mouse::getPosition(win);
    bool hover = box.getGlobalBounds().contains((float)mp.x, (float)mp.y);

    box.setFillColor(hover ? sf::Color(235, 210, 255) : sf::Color(220, 195, 255));
    box.setOutlineThickness(hover ? 4.f : 3.f);
    box.setOutlineColor(sf::Color(120, 60, 200));
    win.draw(box);

    sf::Text t;
    t.setFont(appFont());
    t.setString(label);
    t.setCharacterSize(24);
    t.setFillColor(hover ? sf::Color(80, 0, 150) : sf::Color(60, 0, 120));
    sf::FloatRect b = t.getLocalBounds();
    t.setOrigin(b.left + b.width / 2.f, b.top + b.height / 2.f);
    t.setPosition(W / 2.f, centerY - 3.f);
    win.draw(t);

    if (hover && sf::Mouse::isButtonPressed(sf::Mouse::Left))
        return true;
    return false;
}

void drawInputCard(sf::RenderWindow &win,
                   const string &title,
                   const string &prompt,
                   const string &current)
{
    float cardW = min(640.f, win.getSize().x * 0.85f);
    float cardH = 260.f;
    drawCardCentered(win, cardW, cardH);
    float H = win.getSize().y;
    float top = (H - cardH) / 2.f;

    drawCenteredText(win, title,         top + 30.f, 26, sf::Color(60, 0, 110));
    drawCenteredText(win, prompt,        top + 75.f, 22, sf::Color(40, 0, 90));
    drawCenteredText(win, current,       top + 125.f,28, sf::Color(0, 100, 40));
    drawCenteredText(win,
        "Type and press ENTER (Backspace to correct)",
        top + 170.f, 18, sf::Color(80, 60, 130));
}

void drawMessageCard(sf::RenderWindow &win,
                     const string &title,
                     const string &msg)
{
    float cardW = min(640.f, win.getSize().x * 0.85f);
    float cardH = 220.f;
    drawCardCentered(win, cardW, cardH);
    float H = win.getSize().y;
    float top = (H - cardH)/2.f;

    drawCenteredText(win, title, top+30.f, 26, sf::Color(60,0,110));
    drawCenteredText(win, msg,   top+85.f, 22, sf::Color::Black);
    drawCenteredText(win, "Press ENTER to go back to menu",
                     top+140.f, 18, sf::Color(80,60,130));
}

// Pie chart helper palette
vector<sf::Color> pieColors() {
    return {
        {255,170,190},
        {190,205,255},
        {205,170,255},
        {255,215,160},
        {190,240,190},
        {220,210,255}
    };
}

// PIE CHART
void drawPieChart(sf::RenderWindow &win, const Student &s)
{
    if (s.subjects.empty()) return;

    vector<float> vals;
    for (auto &sub : s.subjects) {
        if (sub.total > 0) vals.push_back(100.f * sub.present / sub.total);
        else               vals.push_back(0.f);
    }

    float sum = 0.f;
    for (float v : vals) sum += v;
    if (sum <= 0.f) return;

    float cx = win.getSize().x / 2.f;
    float cy = win.getSize().y / 2.f - 20.f; // a bit higher to make space for legend
    float R  = 130.f;

    auto colors = pieColors();
    float startAngle = 0.f;
    const int seg = 60;

    for (size_t i=0; i<vals.size(); ++i) {
        float frac = vals[i] / sum;
        float span = frac * 360.f;

        sf::VertexArray fan(sf::TriangleFan, seg + 2);
        fan[0].position = {cx, cy};
        fan[0].color    = colors[i % colors.size()];

        for (int k=0;k<=seg;++k) {
            float ang = (startAngle + span * (float)k/(float)seg) * 3.14159265f / 180.f;
            float x = cx + cos(ang)*R;
            float y = cy + sin(ang)*R;
            fan[k+1].position = {x,y};
            fan[k+1].color    = colors[i%colors.size()];
        }
        win.draw(fan);
        startAngle += span;
    }

    sf::CircleShape inner(R*0.55f);
    inner.setFillColor(sf::Color(250,244,255));
    inner.setPosition(cx-inner.getRadius(), cy-inner.getRadius());
    win.draw(inner);
}

// ============== STATE MACHINE =====================

enum class Screen {
    SUBJECT_COUNT,
    SUBJECT_NAME,
    MENU,
    MSG,

    ADD_BASIC,     // roll, name, dob, address, year, cgpa
    ADD_ATTEND,    // per-subject total & present

    VIEW_DETAILS_ROLL,
    VIEW_DETAILS_SHOW,

    VIEW_ATT_ROLL,
    VIEW_ATT_SHOW,

    PIE_ROLL,
    PIE_SHOW
};

enum class AddStep {
    ROLL,
    NAME,
    DOB,
    ADDRESS,
    YEAR,
    CGPA
};

enum class AttendStep {
    TOTAL,
    PRESENT
};

// ============== MAIN =============================

int main() {
    sf::RenderWindow win(sf::VideoMode(1000, 700),
                         "Student Record Management System",
                         sf::Style::Default);
    win.setFramerateLimit(60);
    sf::View view = win.getView();

    Screen screen = Screen::SUBJECT_COUNT;  // first: setup subjects
    AddStep addStep = AddStep::ROLL;
    AttendStep attendStep = AttendStep::TOTAL;

    string input;        // generic typing buffer
    string msgTitle,msgText;

    int subjectCount = 0;
    int subjectIndex = 0;

    // add student temp
    int tempRoll = -1;
    Student tempStudent;
    int attendSubIndex = 0;

    // view / pie
    int currentRoll = -1;

    while (win.isOpen()) {
        bool enterPressed = false;

        // Which input string is currently active?
        string* activeInput = nullptr;
        switch (screen) {
            case Screen::SUBJECT_COUNT:
            case Screen::SUBJECT_NAME:
            case Screen::ADD_BASIC:
            case Screen::ADD_ATTEND:
            case Screen::VIEW_DETAILS_ROLL:
            case Screen::VIEW_ATT_ROLL:
            case Screen::PIE_ROLL:
                activeInput = &input;
                break;
            default:
                activeInput = nullptr;
        }

        sf::Event event;
        while (win.pollEvent(event)) {
            if (event.type == sf::Event::Closed)
                win.close();

            if (event.type == sf::Event::Resized) {
                view.setSize(event.size.width, event.size.height);
                view.setCenter(event.size.width/2.f, event.size.height/2.f);
                win.setView(view);
            }

            if (event.type == sf::Event::KeyPressed) {
                if (event.key.code == sf::Keyboard::Enter)
                    enterPressed = true;

                if (event.key.code == sf::Keyboard::Escape &&
                    screen != Screen::SUBJECT_COUNT &&
                    screen != Screen::SUBJECT_NAME &&
                    screen != Screen::MSG)
                {
                    screen = Screen::MENU;
                    input.clear();
                }

                if (screen == Screen::MSG &&
                    event.key.code == sf::Keyboard::Enter)
                {
                    screen = Screen::MENU;
                    input.clear();
                    msgTitle.clear();
                    msgText.clear();
                }
            }

            if (activeInput &&
                event.type == sf::Event::TextEntered &&
                screen != Screen::MSG)
            {
                char32_t c = event.text.unicode;
                if (c == 8) { // backspace
                    if (!activeInput->empty()) activeInput->pop_back();
                } else if (c == 13) {
                    // ignore: handled by KeyPressed
                } else if (c >= 32 && c < 127) {
                    activeInput->push_back((char)c);
                }
            }
        }

        // ========== LOGIC (after events) ==========

        // SUBJECT SETUP FLOW
        if (screen == Screen::SUBJECT_COUNT && enterPressed) {
            try {
                subjectCount = stoi(input);
                if (subjectCount < 1) subjectCount = 1;
                subjectNames.clear();
                subjectNames.resize(subjectCount);
                subjectIndex = 0;
                input.clear();
                screen = Screen::SUBJECT_NAME;
            } catch (...) {
                input.clear();
            }
        }

        if (screen == Screen::SUBJECT_NAME && enterPressed) {
            if (!input.empty()) {
                subjectNames[subjectIndex] = input;
                input.clear();
                subjectIndex++;
                if (subjectIndex >= subjectCount) {
                    screen = Screen::MENU;
                }
            }
        }

        // ADD BASIC STUDENT FLOW
        if (screen == Screen::ADD_BASIC && enterPressed && !input.empty()) {
            try {
                switch (addStep) {
                    case AddStep::ROLL:
                        tempRoll = stoi(input);
                        tempStudent = Student();
                        input.clear();
                        addStep = AddStep::NAME;
                        break;
                    case AddStep::NAME:
                        tempStudent.name = input;
                        input.clear();
                        addStep = AddStep::DOB;
                        break;
                    case AddStep::DOB:
                        tempStudent.dob = input;
                        input.clear();
                        addStep = AddStep::ADDRESS;
                        break;
                    case AddStep::ADDRESS:
                        tempStudent.address = input;
                        input.clear();
                        addStep = AddStep::YEAR;
                        break;
                    case AddStep::YEAR:
                        tempStudent.year = input;
                        input.clear();
                        addStep = AddStep::CGPA;
                        break;
                    case AddStep::CGPA:
                        tempStudent.cgpa = stof(input);
                        input.clear();
                        // now go to per-subject attendance
                        tempStudent.subjects.clear();
                        tempStudent.subjects.resize(subjectNames.size());
                        for (size_t i=0;i<subjectNames.size();++i)
                            tempStudent.subjects[i].name = subjectNames[i];
                        attendSubIndex = 0;
                        attendStep = AttendStep::TOTAL;
                        screen = Screen::ADD_ATTEND;
                        break;
                }
            } catch (...) {
                input.clear();
            }
        }

        // ADD ATTENDANCE FLOW
        if (screen == Screen::ADD_ATTEND && enterPressed && !input.empty()) {
            try {
                SubjectInfo &sub = tempStudent.subjects[attendSubIndex];
                if (attendStep == AttendStep::TOTAL) {
                    sub.total = stoi(input);
                    input.clear();
                    attendStep = AttendStep::PRESENT;
                } else {
                    sub.present = stoi(input);
                    input.clear();
                    attendSubIndex++;
                    if (attendSubIndex >= (int)tempStudent.subjects.size()) {
                        // done, save student
                        DB[tempRoll] = tempStudent;
                        msgTitle = "Student Saved";
                        msgText  = "Student details and subject attendance stored.";
                        screen   = Screen::MSG;
                        addStep  = AddStep::ROLL;
                    } else {
                        attendStep = AttendStep::TOTAL;
                    }
                }
            } catch (...) {
                input.clear();
            }
        }

        // VIEW DETAILS / ATT ROLL input
        auto handleRollEnter = [&](Screen from, Screen to) {
            if (screen == from && enterPressed && !input.empty()) {
                try {
                    currentRoll = stoi(input);
                    input.clear();
                    if (!DB.count(currentRoll)) {
                        msgTitle = "Not Found";
                        msgText  = "No student exists with that roll.";
                        screen   = Screen::MSG;
                    } else {
                        screen = to;
                    }
                } catch (...) {
                    input.clear();
                }
            }
        };

        handleRollEnter(Screen::VIEW_DETAILS_ROLL, Screen::VIEW_DETAILS_SHOW);
        handleRollEnter(Screen::VIEW_ATT_ROLL,     Screen::VIEW_ATT_SHOW);
        handleRollEnter(Screen::PIE_ROLL,          Screen::PIE_SHOW);

        // ========== DRAWING ==========

        win.clear(sf::Color(235, 215, 255)); // bright lavender background

        // Title near top (Layout A)
        drawCenteredText(win, "STUDENT ATTENDANCE PORTAL",
                         50.f, 34, sf::Color(90, 0, 160));

        switch (screen) {
            case Screen::SUBJECT_COUNT:
                drawInputCard(win,
                    "Initial Setup",
                    "How many subjects do you want to track?",
                    input);
                break;

            case Screen::SUBJECT_NAME: {
                string prompt = "Enter name for Subject " +
                                to_string(subjectIndex+1) + " of " +
                                to_string(subjectCount) + ":";
                drawInputCard(win,
                    "Initial Setup - Subject Names",
                    prompt,
                    input);
                break;
            }

            case Screen::MENU: {
                float H = win.getSize().y;
                float baseY = H/2.f - 105.f;

                drawCardCentered(win, min(700.f, win.getSize().x*0.9f), 310.f);
                drawCenteredText(win,
                    "Choose an option (ESC inside screens returns here)",
                    baseY-60.f, 18, sf::Color(80,60,130));

                if (drawButtonCentered(win, "Add New Student", baseY)) {
                    screen = Screen::ADD_BASIC;
                    addStep = AddStep::ROLL;
                    input.clear();
                }
                if (drawButtonCentered(win, "View Student Details", baseY+70.f)) {
                    screen = Screen::VIEW_DETAILS_ROLL;
                    input.clear();
                }
                if (drawButtonCentered(win, "View Attendance Summary", baseY+140.f)) {
                    screen = Screen::VIEW_ATT_ROLL;
                    input.clear();
                }
                if (drawButtonCentered(win, "View Attendance Pie Chart", baseY+210.f)) {
                    screen = Screen::PIE_ROLL;
                    input.clear();
                }
                break;
            }

            case Screen::MSG:
                drawMessageCard(win, msgTitle, msgText);
                break;

            case Screen::ADD_BASIC: {
                string prompt;
                switch (addStep) {
                    case AddStep::ROLL:    prompt="Enter Admission / Roll Number:"; break;
                    case AddStep::NAME:    prompt="Enter Full Name of Student:"; break;
                    case AddStep::DOB:     prompt="Enter Date of Birth (YYYY-MM-DD):"; break;
                    case AddStep::ADDRESS: prompt="Enter Address:"; break;
                    case AddStep::YEAR:    prompt="Enter Year (e.g. 2nd Year):"; break;
                    case AddStep::CGPA:    prompt="Enter CGPA:"; break;
                }
                drawInputCard(win,
                    "Add New Student - Details",
                    prompt,
                    input);
                break;
            }

            case Screen::ADD_ATTEND: {
                float cardW = min(700.f, win.getSize().x*0.9f);
                float cardH = 320.f;
                drawCardCentered(win, cardW, cardH);
                float H = win.getSize().y;
                float top = (H-cardH)/2.f;

                SubjectInfo &sub = tempStudent.subjects[attendSubIndex];
                string title = "Attendance for Subject " +
                               to_string(attendSubIndex+1) + " of " +
                               to_string(tempStudent.subjects.size());
                drawCenteredText(win, title, top+30.f, 24, sf::Color(60,0,110));
                drawCenteredText(win, "Subject : " + sub.name, top+70.f, 22, sf::Color(40,0,80));

                string prompt;
                if (attendStep == AttendStep::TOTAL)
                    prompt = "Enter TOTAL classes conducted for " + sub.name + ":";
                else
                    prompt = "Enter PRESENT classes for " + sub.name + ":";

                drawCenteredText(win, prompt, top+115.f, 20, sf::Color(40,0,90));
                drawCenteredText(win, input,  top+160.f, 26, sf::Color(0,100,40));
                drawCenteredText(win,
                    "Type number and press ENTER (ESC cancels and goes to menu)",
                    top+205.f, 18, sf::Color(80,60,130));
                break;
            }

            case Screen::VIEW_DETAILS_ROLL:
                drawInputCard(win,
                    "View Student Details",
                    "Enter Roll Number:",
                    input);
                break;

            case Screen::VIEW_DETAILS_SHOW: {
                const Student &s = DB[currentRoll];

                float cardW = min(700.f, win.getSize().x*0.9f);
                float cardH = 400.f;
                drawCardCentered(win, cardW, cardH);
                float H = win.getSize().y;
                float top = (H-cardH)/2.f;

                drawCenteredText(win, "Student Profile", top+40.f, 26, sf::Color(60,0,110));
                drawCenteredText(win, "Roll : " + to_string(currentRoll), top+85.f, 20);
                drawCenteredText(win, "Name : " + s.name,                 top+115.f, 20);
                drawCenteredText(win, "DOB  : " + s.dob,                  top+145.f, 20);
                drawCenteredText(win, "Address : " + s.address,           top+175.f, 20);
                drawCenteredText(win, "Year : " + s.year,                 top+205.f, 20);
                drawCenteredText(win,
                    "CGPA : " + to_string(s.cgpa).substr(0,4),
                    top+235.f, 20, sf::Color(0,110,70));

                drawCenteredText(win,
                    "Press ESC to return to menu",
                    top+cardH-40.f, 18, sf::Color(80,60,130));
                break;
            }

            case Screen::VIEW_ATT_ROLL:
                drawInputCard(win,
                    "View Attendance Summary",
                    "Enter Roll Number:",
                    input);
                break;

            case Screen::VIEW_ATT_SHOW: {
                const Student &s = DB[currentRoll];

                int totalC=0,totalP=0;
                for (auto &sub: s.subjects) {
                    totalC += sub.total;
                    totalP += sub.present;
                }
                float overall = (totalC>0)?(100.f*totalP/totalC):0.f;

                float cardW = min(820.f, win.getSize().x*0.95f);
                float cardH = 520.f;
                drawCardCentered(win, cardW, cardH);
                float W = win.getSize().x;
                float H = win.getSize().y;
                float top = (H-cardH)/2.f;
                float left = W/2.f - cardW/2.f + 40.f;

                drawCenteredText(win, "Attendance Summary",
                                 top+40.f, 26, sf::Color(60,0,110));

                drawCenteredText(win,
                    "Roll : " + to_string(currentRoll) +
                    "   Name : " + s.name,
                    top+80.f, 20);

                drawCenteredText(win,
                    "Overall Attendance : " +
                    to_string(overall).substr(0,5) + "%",
                    top+115.f, 22, sf::Color(0,120,70));

                // Table header (ATT2 style)
                float y = top + 160.f;
                drawLeftText(win, "Subject",      left,       y, 20, sf::Color(40,0,80));
                drawLeftText(win, "Total",        left+260.f, y, 20, sf::Color(40,0,80));
                drawLeftText(win, "Present",      left+340.f, y, 20, sf::Color(40,0,80));
                drawLeftText(win, "Percent",      left+440.f, y, 20, sf::Color(40,0,80));
                y += 8.f;

                // simple horizontal line
                sf::RectangleShape line(sf::Vector2f(cardW-80.f, 2.f));
                line.setPosition(left, y+14.f);
                line.setFillColor(sf::Color(160,140,220));
                win.draw(line);
                y += 30.f;

                for (auto &sub : s.subjects) {
                    float per = (sub.total>0)?(100.f*sub.present/sub.total):0.f;
                    drawLeftText(win, sub.name,                  left,       y, 18);
                    drawLeftText(win, to_string(sub.total),      left+260.f, y, 18);
                    drawLeftText(win, to_string(sub.present),    left+340.f, y, 18);
                    drawLeftText(win, to_string(per).substr(0,5)+"%", left+440.f, y, 18);
                    y += 26.f;
                }

                drawCenteredText(win,
                    "Press ESC to return to menu",
                    top+cardH-40.f, 18, sf::Color(80,60,130));
                break;
            }

            case Screen::PIE_ROLL:
                drawInputCard(win,
                    "Attendance Pie Chart",
                    "Enter Roll Number:",
                    input);
                break;

            case Screen::PIE_SHOW: {
                const Student &s = DB[currentRoll];

                float cardW = min(820.f, win.getSize().x*0.95f);
                float cardH = 550.f;
                drawCardCentered(win, cardW, cardH);

                float H = win.getSize().y;
                float top = (H-cardH)/2.f;

                drawCenteredText(win, "Attendance Pie Chart",
                                 top+40.f, 26, sf::Color(60,0,110));

                drawCenteredText(win,
                    "Roll : " + to_string(currentRoll) +
                    "   Name : " + s.name,
                    top+80.f, 20);

                // PIE
                drawPieChart(win, s);

                // LEGEND
                auto cols = pieColors();
                float startY = top + 280.f;
                float startX = win.getSize().x / 2.f - 260.f;

                int i = 0;
                for (auto &sub : s.subjects) {
                    float per = (sub.total>0) ? (100.f*sub.present/sub.total) : 0.f;

                    sf::RectangleShape box(sf::Vector2f(18.f,18.f));
                    box.setFillColor(cols[i % cols.size()]);
                    box.setPosition(startX, startY - 14.f);
                    win.draw(box);

                    string text = sub.name + "  →  " +
                                  to_string(per).substr(0,5) + "%";
                    drawLeftText(win, text, startX+30.f, startY-16.f, 18,
                                 sf::Color::Black);

                    startY += 26.f;
                    i++;
                }

                drawCenteredText(win,
                    "Each color represents one subject and its attendance percentage",
                    top+cardH-70.f, 18, sf::Color(40,0,80));
                drawCenteredText(win,
                    "Press ESC to return to menu",
                    top+cardH-40.f, 18, sf::Color(80,60,130));
                break;
            }
        }

        win.display();
    }

    return 0;
}
