#include <iostream>    // For input/output
#include <fstream>     // For file handling
#include <string>      // For using string class
#include <vector>      // For dynamic arrays (used to store questions)
#include <ctime>       // To fetch current date/time
#include <windows.h>   // To use Sleep() for delay
using namespace std;

// Helper function to check if string contains only digits
bool isDigits(const string& s) {
    for (char c : s)
        if (!isdigit(c)) return false;
    return true;
}

// ------------------ Question Class ---------------------
class Question {
protected:
    string questionText;
    string options[4];
    char correctAnswer;

public:
    Question() : questionText(""), correctAnswer(' ') {}

    void inputQuestion() {
        cin.ignore();
        cout << "\nEnter Question: ";
        getline(cin, questionText);
        for (int i = 0; i < 4; i++) {
            cout << "Option " << char('A' + i) << ": ";
            getline(cin, options[i]);
        }
        do {
            cout << "Correct Option (A/B/C/D): ";
            cin >> correctAnswer;
            correctAnswer = toupper(correctAnswer);
            if (correctAnswer < 'A' || correctAnswer > 'D') {
                cout << "Invalid option. Try again.\n";
            }
        } while (correctAnswer < 'A' || correctAnswer > 'D');
    }

    void saveToFile(ofstream& outFile) {
        outFile << questionText << "\n";
        for (int i = 0; i < 4; i++) outFile << options[i] << "\n";
        outFile << correctAnswer << "\n";
    }

    bool loadFromFile(ifstream& inFile) {
        if (!getline(inFile, questionText)) return false;
        if (questionText.empty()) return false;
        for (int i = 0; i < 4; i++) {
            if (!getline(inFile, options[i])) return false;
        }
        inFile >> correctAnswer;
        inFile.ignore(); // discard newline after correctAnswer
        return true;
    }

    void displayQuestion(int number) {
        cout << "\nQ" << number << ": " << questionText << "\n";
        for (int i = 0; i < 4; i++) {
            cout << char('A' + i) << ". " << options[i] << "\n";
        }
    }

    bool checkAnswer(char answer) {
        return toupper(answer) == correctAnswer;
    }
};

// ------------------ Base User Class ---------------------
class User {
protected:
    string username;   // could be regNo for student or "admin" for admin
    string password;

public:
    User() : username(""), password("") {}
    virtual ~User() {}

    virtual bool login() = 0;  // pure virtual - force subclasses to implement

    void setUsername(const string& u) { username = u; }
    void setPassword(const string& p) { password = p; }
    string getUsername() const { return username; }
    string getPassword() const { return password; }
};

// ------------------ Admin Class ---------------------
class Admin : public User {
public:
    Admin() { username = "admin"; }

    bool login() override {
        string pass;
        cout << "\nEnter Admin Password: ";
        cin >> pass;
        if (pass == "admin123") {
            password = pass;
            return true;
        }
        cout << "Incorrect admin password!\n";
        return false;
    }

    void addQuestions() {
        int n;
        cout << "\nHow many questions do you want to add? ";
        cin >> n;
        if (n <= 0) {
            cout << "Invalid number of questions.\n";
            return;
        }

        ofstream outFile("questions.txt", ios::app);
        if (!outFile) {
            cout << "Error: Cannot open questions.txt for writing.\n";
            return;
        }
        for (int i = 0; i < n; i++) {
            cout << "\n--- Entering Question " << (i + 1) << " ---\n";
            Question q;
            q.inputQuestion();
            q.saveToFile(outFile);
        }
        outFile.close();
        cout << "\nQuestions saved successfully.\n";
    }
};

// ------------------ Student Class ---------------------
class Student : public User {
    string studentName;
    string studentRegNo;
    int score;
    vector<Question> questions;

    void loadQuestions() {
        ifstream inFile("questions.txt");
        if (!inFile) {
            cout << "No questions file found. Contact Admin.\n";
            return;
        }
        questions.clear();
        while (true) {
            Question q;
            if (!q.loadFromFile(inFile)) break;
            questions.push_back(q);
        }
        inFile.close();
    }

    void saveResult() {
        ofstream out("results.txt", ios::app);
        if (!out) {
            cout << "Error: Cannot open results.txt for writing.\n";
            return;
        }
        time_t now = time(0);
        char* dt = ctime(&now);
        out << studentRegNo << " " << password << "\n";
        out << "Student: " << studentName << "\n";
        out << "Score: " << score << " / " << questions.size() << "\n";
        out << "Date: " << dt;
        out << "-----------------------------\n";
        out.close();
    }

public:
    Student() : score(0) {}

    // Registration is separate from login here
    bool registerStudent() {
        cout << "\n--- Student Registration ---\n";
        cout << "Enter 6-digit Registration Number: ";
        cin >> studentRegNo;
        if (studentRegNo.length() != 6 || !isDigits(studentRegNo)) {
            cout << "Invalid Registration Number! Must be exactly 6 digits.\n";
            return false;
        }

        cout << "Enter 4-digit Password: ";
        cin >> password;
        if (password.length() != 4 || !isDigits(password)) {
            cout << "Invalid Password! Must be exactly 4 digits.\n";
            return false;
        }

        // Check if regNo already exists
        ifstream fin("students.txt");
        string reg, pass, name;
        while (fin >> reg >> pass) {
            getline(fin, name); // read rest of line
            if (reg == studentRegNo) {
                cout << "Registration Number already registered. Try login.\n";
                fin.close();
                return false;
            }
        }
        fin.close();

        cout << "Enter your Name: ";
        cin.ignore();
        getline(cin, studentName);

        // Save registration details
        ofstream fout("students.txt", ios::app);
        if (!fout) {
            cout << "Error: Cannot open students.txt for writing.\n";
            return false;
        }
        fout << studentRegNo << " " << password << " " << studentName << "\n";
        fout.close();

        cout << "Registration Successful!\n";
        username = studentRegNo; // Set username to regNo for login use
        return true;
    }

    bool login() override {
        cout << "\n--- Student Login ---\n";
        cout << "Enter Registration Number: ";
        cin >> studentRegNo;
        if (studentRegNo.length() != 6 || !isDigits(studentRegNo)) {
            cout << "Invalid Registration Number!\n";
            return false;
        }
        cout << "Enter Password: ";
        cin >> password;
        if (password.length() != 4 || !isDigits(password)) {
            cout << "Invalid Password!\n";
            return false;
        }

        ifstream fin("students.txt");
        string reg, pass, name;
        bool found = false;
        while (fin >> reg >> pass) {
            getline(fin, name);
            if (reg == studentRegNo && pass == password) {
                studentName = name.substr(1); // remove leading space
                found = true;
                break;
            }
        }
        fin.close();

        if (!found) {
            cout << "Incorrect Registration Number or Password.\n";
            return false;
        }
        username = studentRegNo;
        return true;
    }

    void takeExam() {
        loadQuestions();
        if (questions.empty()) {
            cout << "Test is not available currently. Please contact Admin.\n";
            return;
        }

        cout << "\nWelcome " << studentName << "! Starting the test...\n";

        score = 0;
        char answer;
        for (size_t i = 0; i < questions.size(); i++) {
            questions[i].displayQuestion((int)(i + 1));
            do {
                cout << "Your answer (A/B/C/D): ";
                cin >> answer;
                answer = toupper(answer);
                if (answer < 'A' || answer > 'D') {
                    cout << "Invalid choice. Try again.\n";
                }
            } while (answer < 'A' || answer > 'D');

            if (questions[i].checkAnswer(answer)) score++;
            Sleep(1000);
        }

        cout << "\nExam Completed!\n";
        cout << studentName << ", your score: " << score << " / " << questions.size() << "\n";

        saveResult();
    }

    void displayResultSummary() {
        cout << "\n------ RESULT SUMMARY ------\n";
        cout << "Student Name: " << studentName << "\n";
        cout << "Registration Number: " << studentRegNo << "\n";
        cout << "Score: " << score << " / " << questions.size() << "\n";
        cout << "----------------------------\n";
    }

    // Static method for viewing past results
    static void showPastResults() {
        cout << "\n--- View Result / Summary ---\n";
        string regNo, pass;
        cout << "Enter Registration Number: ";
        cin >> regNo;
        if (regNo.length() != 6 || !isDigits(regNo)) {
            cout << "Invalid Registration Number!\n";
            return;
        }
        cout << "Enter Password: ";
        cin >> pass;
        if (pass.length() != 4 || !isDigits(pass)) {
            cout << "Invalid Password!\n";
            return;
        }

        ifstream fin("results.txt");
        if (!fin) {
            cout << "No results found.\n";
            return;
        }
        string line;
        bool found = false;
        while (getline(fin, line)) {
            if (line.length() >= 11) {
                string reg = line.substr(0, 6);
                string p = line.substr(7, 4);
                if (reg == regNo && p == pass) {
                    cout << "\n" << line << "\n";
                    // Next 4 lines contain student name, score, date, separator
                    for (int i = 0; i < 4; i++) {
                        if (getline(fin, line))
                            cout << line << "\n";
                    }
                    found = true;
                    break;
                }
            }
        }
        fin.close();
        if (!found) {
            cout << "No matching results found for provided credentials.\n";
        }
    }
};

// ------------------ Student Menu ---------------------
void studentMenu() {
    Student s;
    int choice;

    do {
        cout << "\n--- Student Menu ---\n";
        cout << "1. Register\n2. Login & Take Test\n3. View Result / Summary\n4. Back to Main Menu\n";
        cout << "Enter choice: ";
        cin >> choice;

        switch (choice) {
        case 1:
            if (s.registerStudent()) {
                cout << "You can now login to take the test.\n";
            }
            break;
        case 2:
            if (s.login()) {
                s.takeExam();
                s.displayResultSummary();
            }
            break;
        case 3:
            Student::showPastResults();
            break;
        case 4:
            cout << "Returning to main menu...\n";
            break;
        default:
            cout << "Invalid choice!\n";
        }
    } while (choice != 4);
}

// ------------------ Admin Login ---------------------
void adminLogin() {
    Admin a;
    if (a.login()) {
        a.addQuestions();
    }
}

// ------------------ Main ---------------------
int main() {
    int mainChoice;

    do {
        cout << "\n----- Online Exam Portal -----\n";
        cout << "1. Admin Login\n2. Student Register / Login\n3. Exit\n";
        cout << "Enter your choice: ";
        cin >> mainChoice;

        switch (mainChoice) {
        case 1:
            adminLogin();
            break;
        case 2:
            studentMenu();
            break;
        case 3:
            cout << "Thank you for using the exam portal. Goodbye!\n";
            break;
        default:
            cout << "Invalid choice. Try again.\n";
        }
    } while (mainChoice != 3);

    return 0;
}
