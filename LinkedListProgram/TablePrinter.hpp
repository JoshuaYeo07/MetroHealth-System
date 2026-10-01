#ifndef TABLE_PRINTER_HPP
#define TABLE_PRINTER_HPP

#include "PatientList.hpp"
#include <iostream>
#include <iomanip>
#include <sstream>
#include <string>
#include "Console.hpp"

/**
 * @class TablePrinter
 * @brief Utilities to render formatted, aligned text-based console tables.
 * Complies with assignment guideline: "Do not use GUI, organize data neatly in text tables using formatting".
 */
class TablePrinter {
public:
    static constexpr int REPORT_WIDTH = 110;

    static void printHorizontalLine(int width = REPORT_WIDTH, char border = '-', char cross = '+') {
        std::cout << Console::DIM << cross << std::string(width - 2, border) << cross << Console::RESET << "\n";
    }

    static void printRule(char ch = '-', int width = REPORT_WIDTH) {
        std::cout << Console::DIM << std::string(width, ch) << Console::RESET << "\n";
    }

    static void printBanner(const std::string& title, int width = REPORT_WIDTH) {
        std::cout << "\n";
        printRule('=', width);
        std::cout << Console::BOLD << Console::CYAN << "  " << title << Console::RESET << "\n";
        printRule('=', width);
    }

    static void printSectionTitle(const std::string& title, int width = REPORT_WIDTH) {
        std::cout << "\n" << Console::BOLD << Console::YELLOW << title << Console::RESET << "\n";
        printRule('-', width);
    }

    // 1234.5 -> "1234.50"
    static std::string formatDecimal(double value, int precision = 2) {
        std::ostringstream out;
        out << std::fixed << std::setprecision(precision) << value;
        return out.str();
    }

    // 1234567.891 -> "1,234,567.89"
    static std::string formatMoney(double amount) {
        std::string fixed = formatDecimal(amount, 2);
        size_t dot = fixed.find('.');
        std::string whole = fixed.substr(0, dot);

        std::string grouped;
        int digits = 0;
        for (size_t i = whole.size(); i > 0; --i) {
            grouped.insert(grouped.begin(), whole[i - 1]);
            if (++digits % 3 == 0 && i > 1 && whole[i - 2] != '-') {
                grouped.insert(grouped.begin(), ',');
            }
        }
        return grouped + fixed.substr(dot);
    }

    // part / whole as a percentage string, e.g. "40.00%"
    static std::string formatPercent(double part, double whole) {
        return formatDecimal(whole > 0.0 ? (part / whole) * 100.0 : 0.0, 2) + "%";
    }

    // Truncates text with "..." so it never overflows its column
    static std::string fit(const std::string& text, size_t width) {
        if (text.size() <= width) {
            return text;
        }
        return text.substr(0, width - 3) + "...";
    }

    /**
     * @brief Prints a bounded table of Patient records by traversing the linked list from head.
     * @param list The PatientList to print
     * @param title Title of the dataset or query
     * @param maxRows Maximum rows to display (0 for all rows)
     */
    static void displayPatientTable(const PatientList& list, const std::string& title, size_t maxRows = 15) {
        std::cout << "\n";
        printRule('=');
        std::cout << Console::BOLD << Console::CYAN << "  " << title << " (Total Records: " << list.size() << ")" << Console::RESET << "\n";
        printRule('=');

        if (list.isEmpty()) {
            std::cout << Console::YELLOW << "  [Notice] No records found." << Console::RESET << "\n";
            printRule('-');
            return;
        }

        printHorizontalLine();
        std::cout << Console::BOLD << Console::YELLOW << "| " << std::left  << std::setw(12) << "Patient ID"
                  << "| " << std::right << std::setw(5)  << "Age"
                  << " | " << std::left  << std::setw(18) << "Care Type"
                  << "| " << std::right << std::setw(10) << "Stay (hrs)"
                  << "| " << std::right << std::setw(14) << "Rate (MYR/hr)"
                  << "| " << std::right << std::setw(11) << "Visits/Yr"
                  << "| " << std::right << std::setw(17) << "Total Cost (MYR)"
                  << " |" << Console::RESET << "\n";
        printHorizontalLine();

        size_t printed = 0;
        for (const PatientList::Node* current = list.getHead();
             current != nullptr && (maxRows == 0 || printed < maxRows);
             current = current->next, ++printed) {
            const Patient& p = current->data;
            std::cout << "| " << std::left  << std::setw(12) << p.patientId
                      << "| " << std::right << std::setw(5)  << p.age
                      << " | " << std::left  << std::setw(18) << p.careType
                      << "| " << std::right << std::setw(10) << p.lengthOfStay
                      << "| " << std::right << std::setw(14) << formatDecimal(p.baseCostPerHour)
                      << "| " << std::right << std::setw(11) << p.daysVisitsPerYear
                      << "| " << std::right << std::setw(17) << formatMoney(p.calculateTotalCost())
                      << " |\n";
        }
        printHorizontalLine();

        if (printed < list.size()) {
            std::cout << "  * Displaying first " << printed << " of " << list.size()
                      << " records. (Full dataset safely stored in the linked list).\n";
        }
    }
};

#endif // TABLE_PRINTER_HPP
