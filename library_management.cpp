#include <algorithm>
#include <cctype>
#include <cstdio>
#include <ctime>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <limits>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

class Book {
private:
    int id_;
    std::string title_;
    std::string author_;
    std::string publisher_;
    int totalCopies_;

public:
    Book(int id, std::string title, std::string author,
         std::string publisher, int totalCopies)
        : id_(id), title_(std::move(title)), author_(std::move(author)),
          publisher_(std::move(publisher)), totalCopies_(totalCopies) {}

    int id() const { return id_; }
    const std::string& title() const { return title_; }
    const std::string& author() const { return author_; }
    const std::string& publisher() const { return publisher_; }
    int totalCopies() const { return totalCopies_; }
};

class Member {
private:
    int id_;
    std::string name_;
    std::string phone_;

public:
    Member(int id, std::string name, std::string phone)
        : id_(id), name_(std::move(name)), phone_(std::move(phone)) {}

    int id() const { return id_; }
    const std::string& name() const { return name_; }
    const std::string& phone() const { return phone_; }
};

class Loan {
private:
    int id_;
    int bookId_;
    int memberId_;
    std::string issueDate_;
    std::string returnDate_;

public:
    Loan(int id, int bookId, int memberId, std::string issueDate,
         std::string returnDate = "-")
        : id_(id), bookId_(bookId), memberId_(memberId),
          issueDate_(std::move(issueDate)), returnDate_(std::move(returnDate)) {}

    int id() const { return id_; }
    int bookId() const { return bookId_; }
    int memberId() const { return memberId_; }
    const std::string& issueDate() const { return issueDate_; }
    const std::string& returnDate() const { return returnDate_; }
    bool isActive() const { return returnDate_ == "-"; }
    void markReturned(const std::string& date) { returnDate_ = date; }
};

class Library {
private:
    std::vector<Book> books_;
    std::vector<Member> members_;
    std::vector<Loan> loans_;

    const std::string booksFile_ = "books.txt";
    const std::string membersFile_ = "members.txt";
    const std::string loansFile_ = "loans.txt";

    static std::string currentDate() {
        const std::time_t now = std::time(nullptr);
        std::tm localTime{};
#if defined(_WIN32)
        localtime_s(&localTime, &now);
#else
        localtime_r(&now, &localTime);
#endif
        std::ostringstream output;
        output << std::put_time(&localTime, "%Y-%m-%d");
        return output.str();
    }

    static int nextId(const std::vector<Book>& records) {
        int highest = 0;
        for (const auto& record : records) highest = std::max(highest, record.id());
        return highest + 1;
    }

    static int nextId(const std::vector<Member>& records) {
        int highest = 0;
        for (const auto& record : records) highest = std::max(highest, record.id());
        return highest + 1;
    }

    static int nextLoanId(const std::vector<Loan>& records) {
        int highest = 0;
        for (const auto& record : records) highest = std::max(highest, record.id());
        return highest + 1;
    }

    Book* findBook(int id) {
        for (auto& book : books_) if (book.id() == id) return &book;
        return nullptr;
    }

    const Book* findBook(int id) const {
        for (const auto& book : books_) if (book.id() == id) return &book;
        return nullptr;
    }

    const Member* findMember(int id) const {
        for (const auto& member : members_) if (member.id() == id) return &member;
        return nullptr;
    }

    int availableCopies(int bookId) const {
        const Book* book = findBook(bookId);
        if (!book) return 0;
        int activeLoans = 0;
        for (const auto& loan : loans_) {
            if (loan.bookId() == bookId && loan.isActive()) ++activeLoans;
        }
        return std::max(0, book->totalCopies() - activeLoans);
    }

    bool saveBooks() const {
        const std::string temporary = booksFile_ + ".tmp";
        {
            std::ofstream out(temporary, std::ios::trunc);
            if (!out) return false;
            for (const auto& book : books_) {
                out << book.id() << ' ' << std::quoted(book.title()) << ' '
                    << std::quoted(book.author()) << ' '
                    << std::quoted(book.publisher()) << ' '
                    << book.totalCopies() << '\n';
            }
            if (!out) return false;
        }
        std::remove(booksFile_.c_str());
        return std::rename(temporary.c_str(), booksFile_.c_str()) == 0;
    }

    bool saveMembers() const {
        const std::string temporary = membersFile_ + ".tmp";
        {
            std::ofstream out(temporary, std::ios::trunc);
            if (!out) return false;
            for (const auto& member : members_) {
                out << member.id() << ' ' << std::quoted(member.name()) << ' '
                    << std::quoted(member.phone()) << '\n';
            }
            if (!out) return false;
        }
        std::remove(membersFile_.c_str());
        return std::rename(temporary.c_str(), membersFile_.c_str()) == 0;
    }

    bool saveLoans() const {
        const std::string temporary = loansFile_ + ".tmp";
        {
            std::ofstream out(temporary, std::ios::trunc);
            if (!out) return false;
            for (const auto& loan : loans_) {
                out << loan.id() << ' ' << loan.bookId() << ' '
                    << loan.memberId() << ' ' << loan.issueDate() << ' '
                    << loan.returnDate() << '\n';
            }
            if (!out) return false;
        }
        std::remove(loansFile_.c_str());
        return std::rename(temporary.c_str(), loansFile_.c_str()) == 0;
    }

    void loadBooks() {
        std::ifstream in(booksFile_);
        int id = 0, copies = 0;
        std::string title, author, publisher;
        while (in >> id >> std::quoted(title) >> std::quoted(author)
                  >> std::quoted(publisher) >> copies) {
            if (id > 0 && copies > 0)
                books_.emplace_back(id, title, author, publisher, copies);
        }
    }

    void loadMembers() {
        std::ifstream in(membersFile_);
        int id = 0;
        std::string name, phone;
        while (in >> id >> std::quoted(name) >> std::quoted(phone)) {
            if (id > 0) members_.emplace_back(id, name, phone);
        }
    }

    void loadLoans() {
        std::ifstream in(loansFile_);
        int id = 0, bookId = 0, memberId = 0;
        std::string issueDate, returnDate;
        while (in >> id >> bookId >> memberId >> issueDate >> returnDate) {
            if (id > 0 && bookId > 0 && memberId > 0)
                loans_.emplace_back(id, bookId, memberId, issueDate, returnDate);
        }
    }

    static std::string toLower(std::string value) {
        std::transform(value.begin(), value.end(), value.begin(),
            [](unsigned char ch) { return static_cast<char>(std::tolower(ch)); });
        return value;
    }

    void printBook(const Book& book) const {
        std::cout << "\nBook ID: " << book.id()
                  << "\nTitle: " << book.title()
                  << "\nAuthor: " << book.author()
                  << "\nPublisher: " << book.publisher()
                  << "\nTotal copies: " << book.totalCopies()
                  << "\nAvailable copies: " << availableCopies(book.id())
                  << "\n----------------------------------------\n";
    }

public:
    Library() {
        loadBooks();
        loadMembers();
        loadLoans();
    }

    void addBook() {
        std::string title, author, publisher;
        int copies = 0;
        std::cout << "Book title: ";
        std::getline(std::cin >> std::ws, title);
        std::cout << "Author: ";
        std::getline(std::cin, author);
        std::cout << "Publisher: ";
        std::getline(std::cin, publisher);
        std::cout << "Number of copies: ";
        if (!(std::cin >> copies) || copies <= 0) {
            std::cout << "Invalid number of copies. Please enter a positive integer.\n";
            return;
        }
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

        const int id = nextId(books_);
        books_.emplace_back(id, title, author, publisher, copies);
        if (saveBooks()) std::cout << "Book added successfully. Book ID: " << id << "\n";
        else {
            books_.pop_back();
            std::cout << "Could not save book data. Please check file permissions.\n";
        }
    }

    void registerMember() {
        std::string name, phone;
        std::cout << "Member name: ";
        std::getline(std::cin >> std::ws, name);
        std::cout << "Phone/contact number: ";
        std::getline(std::cin, phone);

        const int id = nextId(members_);
        members_.emplace_back(id, name, phone);
        if (saveMembers()) std::cout << "Member registered successfully. Member ID: " << id << "\n";
        else {
            members_.pop_back();
            std::cout << "Could not save member data. Please check file permissions.\n";
        }
    }

    void displayBooks() const {
        if (books_.empty()) {
            std::cout << "No books found in the library.\n";
            return;
        }
        std::cout << "\n========== BOOK CATALOGUE ==========\n";
        for (const auto& book : books_) printBook(book);
    }

    void searchBooksByTitle() const { searchBooks("title"); }
    void searchBooksByAuthor() const { searchBooks("author"); }

    void issueBook() {
        if (books_.empty() || members_.empty()) {
            std::cout << "Add at least one book and register at least one member first.\n";
            return;
        }
        int bookId = 0, memberId = 0;
        std::cout << "Enter book ID: ";
        if (!(std::cin >> bookId)) { invalidInput(); return; }
        std::cout << "Enter member ID: ";
        if (!(std::cin >> memberId)) { invalidInput(); return; }
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

        const Book* book = findBook(bookId);
        const Member* member = findMember(memberId);
        if (!book) { std::cout << "Book ID not found.\n"; return; }
        if (!member) { std::cout << "Member ID not found.\n"; return; }
        if (availableCopies(bookId) <= 0) {
            std::cout << "No copies of this book are currently available.\n";
            return;
        }
        for (const auto& loan : loans_) {
            if (loan.bookId() == bookId && loan.memberId() == memberId && loan.isActive()) {
                std::cout << "This member already has an active loan for this book.\n";
                return;
            }
        }

        const int loanId = nextLoanId(loans_);
        loans_.emplace_back(loanId, bookId, memberId, currentDate());
        if (saveLoans())
            std::cout << "Book issued successfully. Loan ID: " << loanId << "\n";
        else {
            loans_.pop_back();
            std::cout << "Could not save loan record. Please check file permissions.\n";
        }
    }

    void returnBook() {
        int loanId = 0;
        std::cout << "Enter loan ID to return: ";
        if (!(std::cin >> loanId)) { invalidInput(); return; }
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

        for (auto& loan : loans_) {
            if (loan.id() == loanId) {
                if (!loan.isActive()) {
                    std::cout << "This loan has already been returned.\n";
                    return;
                }
                loan.markReturned(currentDate());
                if (saveLoans()) std::cout << "Book returned successfully.\n";
                else {
                    loan.markReturned("-");
                    std::cout << "Could not save return record. Please check file permissions.\n";
                }
                return;
            }
        }
        std::cout << "Loan ID not found.\n";
    }

    void displayMembers() const {
        if (members_.empty()) {
            std::cout << "No members registered.\n";
            return;
        }
        std::cout << "\n========== REGISTERED MEMBERS ==========\n";
        for (const auto& member : members_) {
            std::cout << "Member ID: " << member.id()
                      << " | Name: " << member.name()
                      << " | Contact: " << member.phone() << '\n';
        }
    }

    void displayActiveLoans() const {
        bool found = false;
        std::cout << "\n========== ACTIVE LOANS ==========\n";
        for (const auto& loan : loans_) {
            if (!loan.isActive()) continue;
            found = true;
            const Book* book = findBook(loan.bookId());
            const Member* member = findMember(loan.memberId());
            std::cout << "Loan ID: " << loan.id()
                      << " | Book: " << (book ? book->title() : "[missing book]")
                      << " | Member: " << (member ? member->name() : "[missing member]")
                      << " | Issued: " << loan.issueDate() << '\n';
        }
        if (!found) std::cout << "There are no active loans.\n";
    }

    static void invalidInput() {
        std::cout << "Invalid input. Please enter a valid value.\n";
        std::cin.clear();
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    }

private:
    void searchBooks(const std::string& field) const {
        std::string query;
        std::cout << "Enter " << field << " to search: ";
        std::getline(std::cin >> std::ws, query);
        const std::string target = toLower(query);
        bool found = false;
        for (const auto& book : books_) {
            const std::string& candidate = (field == "title") ? book.title() : book.author();
            if (toLower(candidate).find(target) != std::string::npos) {
                printBook(book);
                found = true;
            }
        }
        if (!found) std::cout << "No books matched your search.\n";
    }
};

int main() {
    Library library;
    int choice = -1;

    do {
        std::cout << "\n============================================\n"
                  << "          LIBRARY MANAGEMENT SYSTEM\n"
                  << "============================================\n"
                  << "1. Add Book\n"
                  << "2. Register Member\n"
                  << "3. Display All Books\n"
                  << "4. Search Book by Title\n"
                  << "5. Search Book by Author\n"
                  << "6. Issue Book\n"
                  << "7. Return Book\n"
                  << "8. Display Members\n"
                  << "9. Display Active Loans\n"
                  << "0. Exit\n"
                  << "--------------------------------------------\n"
                  << "Enter your choice: ";

        if (!(std::cin >> choice)) {
            if (std::cin.eof()) break;
            Library::invalidInput();
            continue;
        }
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

        switch (choice) {
            case 1: library.addBook(); break;
            case 2: library.registerMember(); break;
            case 3: library.displayBooks(); break;
            case 4: library.searchBooksByTitle(); break;
            case 5: library.searchBooksByAuthor(); break;
            case 6: library.issueBook(); break;
            case 7: library.returnBook(); break;
            case 8: library.displayMembers(); break;
            case 9: library.displayActiveLoans(); break;
            case 0: std::cout << "Exiting Library Management System. Goodbye!\n"; break;
            default: std::cout << "Invalid choice. Please select from 0 to 9.\n";
        }
    } while (choice != 0);

    return 0;
}
